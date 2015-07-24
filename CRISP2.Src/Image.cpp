/****************************************************************************/
/* Copyright (c) 2006 Calidris.					*****************************/
/****************************************************************************/
/* Written by Peter Oleynikov, Analitex *************************************/
/****************************************************************************/

#define _WIN32_WINNT 0x0400
#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "ft32util.h"
#include "rgconfig.h"

// Added by Peter on 24 February 2006
#include "Crash.h"

#include <process.h>
#include <mmsystem.h>

#ifdef _USE_LOGGING
extern FILE *s_pLog;
#endif

// 19 Dec 2002
// Peter added in each function the check if the Object is NULL - fixes a bug,
// when CRISP2 dies with Edit Tools

// added by Peter on 22 Aug 2002
#include "MV40Live.h"

#include "..\shark49x\pcidev.h"
#include "..\shark49x\shark4.h"

HANDLE	hLiveThreadShark4 = NULL;

#define	MAXPOLY	1000	// Maximum number of vertex of polygon
static	DWORD	EditMode=0, ModeCtrl = 0;
static	int		Ex0, Ey0, Ex, Ey;
static	int		EdgeWidth = 1;
static	double	RotateAng = 90;		// Image rotation angle
static	POINT	Epoints[MAXPOLY+2], Esp[MAXPOLY+2];
static	int		Epcnt=0;

#pragma optimize ("",off)

/****************************************************************************/
/* Update selected edit tool window *****************************************/
/****************************************************************************/
void	UpdateSelect( HWND hDlg )
{
	BOOL	fc, ff, fs, fr, ffree, fundo, fin, fout, fedge, frot;
	OBJ*	O;

	O = OBJ::GetOBJ(hActive);

	// default values
	fc = ff = fs = fr = ffree = fundo = fin = fout = fedge = frot = FALSE;
	// no object...
	if( NULL != O )
	{
		if ( hActive != NULL && O->ID == OT_IMG)
		{

			fc = ff = fs = fr = ffree = TRUE;
			frot = TRUE;

			if( O->flags & OF_inedit) { fin = fout = fedge = fundo = TRUE; frot = FALSE; }

			if( O->flags & OF_wasedit) {
				fc = ff = fs = fr = ffree = fin = fout = fedge = frot = FALSE;
				fundo = TRUE;
			}
		}
	}
	// disable or enable the buttons
	EnableWindow( GetDlgItem( hDlg, IDC_IE_CCIRC),  fc);
	EnableWindow( GetDlgItem( hDlg, IDC_IE_FCIRC),  ff);
	EnableWindow( GetDlgItem( hDlg, IDC_IE_SQUARE), fs);
	EnableWindow( GetDlgItem( hDlg, IDC_IE_RECT),   fr);
	EnableWindow( GetDlgItem( hDlg, IDC_IE_FREESEL),ffree);
	EnableWindow( GetDlgItem( hDlg, IDC_IE_CUTIN),  fin);
	EnableWindow( GetDlgItem( hDlg, IDC_IE_CUTOUT), fout);
	EnableWindow( GetDlgItem( hDlg, IDC_IE_EDGE),   fedge);
	EnableWindow( GetDlgItem( hDlg, IDC_IE_RESTORE),fundo);
	EnableWindow( GetDlgItem( hDlg, IDC_IE_ROTATE), frot);

	SetDlgItemDouble( hDlg, IDC_IE_ANGLE, RotateAng, 0 );
//	SetDlgItemInt( hDlg, IDC_IE_EDGEWIDTH, EdgeWidth, FALSE );

}
/****************************************************************************/
/* Draw wire selection ******************************************************/
/****************************************************************************/
void	DrawWire( HWND hWnd, OBJ* O )
{
	HDC		hdc = GetDC( hWnd );
	double	x0, y0, x, y, x1, y1;
	int		i, orop;

	if( NULL == O )
		return;

	x = (Ex - O->xo)*(double)O->scu/(double)O->scd + (O->scu/2);
	y = (Ey - O->yo)*(double)O->scu/(double)O->scd + (O->scu/2);

	x0 = (Ex0 - O->xo)*(double)O->scu/(double)O->scd + (O->scu/2);
	y0 = (Ey0 - O->yo)*(double)O->scu/(double)O->scd + (O->scu/2);

	x1 = 2*x0-x;
	y1 = 2*y0-y;
	orop = SetROP2( hdc, R2_NOT );
	SelectObject( hdc, GetStockObject(NULL_BRUSH));

	switch (EditMode) {
		case IDC_IE_CCIRC:	Ellipse( hdc, round(x), round(y), round(x1), round(y1) );	break;
		case IDC_IE_FCIRC:	Ellipse( hdc, round(x), round(y), round(x1), round(y1) );	break;
		case IDC_IE_SQUARE:	Rectangle( hdc, round(x), round(y), round(x0), round(y0) );	break;
		case IDC_IE_RECT:    Rectangle( hdc, round(x), round(y), round(x0), round(y0) );	break;
		case IDC_IE_FREESEL:
			for( i=0; i<=Epcnt; i++) {
				Esp[i].x = round((Epoints[i].x - O->xo)*(double)O->scu/(double)O->scd + (O->scu/2));
				Esp[i].y = round((Epoints[i].y - O->yo)*(double)O->scu/(double)O->scd + (O->scu/2));
			}
			Polyline( hdc, Esp, Epcnt+1); break;
	}
	SetROP2( hdc, orop );
	ReleaseDC( hWnd, hdc );

}
/****************************************************************************/
/* Draging wire selection ***************************************************/
/****************************************************************************/
void	DragEdit( HWND hWnd, OBJ* O, LPARAM lP, WPARAM wP )
{
	int		x, y, d;

	if( NULL == O )
		return;

	DrawWire( hWnd, O );

	Ex = ((int)LOWORD(lP)) * O->scd / O->scu + O->xo;
	Ey = ((int)HIWORD(lP)) * O->scd / O->scu + O->yo;

	if (Ex<0) Ex = 0; if(Ex>=O->x) Ex = O->x-1;
	if (Ey<0) Ey = 0; if(Ey>=O->y) Ey = O->y-1;

	if ( EditMode == IDC_IE_CCIRC || EditMode == IDC_IE_FCIRC) {
		x = 2*Ex0-Ex;
		y = 2*Ey0-Ey;
		if (x<0) x = 0; if(x>=O->x) x = O->x-1;
		if (y<0) y = 0; if(y>=O->y) y = O->y-1;
		Ex = 2*Ex0 - x;
		Ey = 2*Ey0 - y;
	}
	if ( EditMode == IDC_IE_CCIRC || EditMode == IDC_IE_SQUARE) {
		x = abs(Ex-Ex0); y = abs(Ey-Ey0);
		d = __min(x, y);
		if (d) { Ex = Ex0+d*((Ex-Ex0)/x); Ey = Ey0+d*((Ey-Ey0)/y); }
		else   { Ex = Ex0; Ey = Ey0; }
	}
	if ( EditMode == IDC_IE_FREESEL ) {
		if ((wP & MK_RBUTTON) && (Epcnt>1)) Epcnt--;	// Delete one vertex
		Epoints[Epcnt].x = Ex;
		Epoints[Epcnt].y = Ey;
		if ((wP & MK_LBUTTON) && ((Epoints[Epcnt-1].x != Ex) || (Epoints[Epcnt-1].y != Ey)))
        	{ Epoints[Epcnt+1] = Epoints[Epcnt]; Epcnt++; }
		if (Epcnt>=MAXPOLY) Epcnt--;
	}
	DrawWire( hWnd, O );

}
/****************************************************************************/
void ReleaseSelection( HWND hWnd, OBJ* O )
{
	HDC		hdc;
	if( NULL == O )
		return;

	if (O->hBme)
	{
		hdc = CreateCompatibleDC(NULL);
		SelectBitmap(hdc,O->hBme);
		PatBlt( hdc, 0, 0, O->x, O->y, BLACKNESS);
		DeleteDC(hdc);
		InvalidateRect( hWnd, NULL, 0 );
	}

}
/****************************************************************************/
void	ClickEdit( HWND hWnd, OBJ* O, LPARAM lP, WPARAM wParam )
{
	if( NULL == O )
		return;

	if ( O->flags & OF_wasedit)
		return;
    if (EditMode == 0) return;
	SetCapture( hWnd );
    O->flags |= OF_indrag;
	Ex = Ex0 = (LOWORD(lP)) * O->scd / O->scu + O->xo;
	Ey = Ey0 = (HIWORD(lP)) * O->scd / O->scu + O->yo;
	Epcnt = 1;		// No points in free hand
	Epoints[0].x = Ex0;	Epoints[0].y = Ey0;
	Epoints[1] = Epoints[0];
	ModeCtrl = wParam;
	if ((wParam & (MK_SHIFT|MK_CONTROL)) == 0) ReleaseSelection(hWnd, O);

}
/****************************************************************************/
/* Add or Subtract new selection ********************************************/
/****************************************************************************/
void	UpdateMask( HWND hWnd, OBJ* pObj )
{
	int		x0, y0, x, y, x1, y1, i;
	HPEN	hp;
	HBRUSH	hb;
	HCURSOR hoc = SetCursor(hcWait);
	HDC		hdc;

	if( NULL == pObj )
		return;

	if( NULL == pObj->hBme )
	{ 					// Create new mask
		BITMAPINFO	*bb = (LPBITMAPINFO)calloc(1, sizeof(BITMAPINFO)+sizeof(RGBQUAD)*256);
		LPBYTE	bipal;

		bb->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bb->bmiHeader.biPlanes = 1;
		bb->bmiHeader.biBitCount = 8;
		bb->bmiHeader.biCompression = BI_RGB;
		bb->bmiHeader.biClrUsed  = COLORS;
		bb->bmiHeader.biClrImportant = COLORS;
		bb->bmiHeader.biSizeImage = R4(pObj->x) * pObj->y;
		bb->bmiHeader.biWidth  = R4(pObj->x);
		bb->bmiHeader.biHeight = pObj->y;
		bipal = (LPBYTE)(bb->bmiColors);
		for (i=0;i<256;i++) {
			*bipal++ = i; *bipal++ = i; *bipal++ = i;
			*bipal++ = 0;
		}
		pObj->hBme = CreateDIBSection( NULL, bb, DIB_RGB_COLORS, (LPVOID*)(&pObj->ebp), NULL,  0);
	}
	hdc = CreateCompatibleDC(NULL);
	SelectBitmap( hdc, pObj->hBme );

	pObj->flags |= OF_inedit;
	if (hSelectET) UpdateSelect( hSelectET );

	SetROP2( hdc, R2_XORPEN );
	hp = CreatePen(PS_SOLID, 1, RGB(2,2,2));
	hb = CreateSolidBrush(RGB(4,4,4));
	SelectObject( hdc, hb );
	SelectObject( hdc, hp );
	x = Ex;
	y = pObj->y - Ey - 1;

	x0 = Ex0;
	y0 = pObj->y - Ey0 - 1;

	x1 = 2*x0-x;
	y1 = 2*y0-y;

	switch( EditMode )
	{
	case IDC_IE_FCIRC:
	case IDC_IE_CCIRC:
		{
			if (x>x1) x++; else x1++;
			if (y>y1) y++; else y1++;
			Ellipse( hdc, x, y, x1, y1 );
			break;
		}
	case IDC_IE_SQUARE:
	case IDC_IE_RECT:
		{
			if (x>x0) x++; else x0++;
			if (y>y0) y++; else y0++;
			Rectangle( hdc, x, y, x0, y0 );
			break;
		}
	case IDC_IE_FREESEL:
		{
			for(i=0;i<Epcnt;i++)
			{
				Esp[i].x = Epoints[i].x;
				Esp[i].y = pObj->y - Epoints[i].y -1;
			}
			Polygon( hdc, Esp, Epcnt );
		}
	}
	memset( TMP, 0, 256);
#define	B	(BYTE)1
#define	C	(BYTE)255
	if( ModeCtrl & MK_CONTROL )
	{		// Add to selection
		TMP[  1] = B;	TMP[  2] = 0;	TMP[  3] = C;	TMP[  4] = 0;
		TMP[  5] = 0;	TMP[  6] = 0;	TMP[  7] = C;	TMP[249] = C;
		TMP[251] = 0;	TMP[253] = C;	TMP[255] = C;
	}
	else
	{							// Subtract from selection
		TMP[  1] = B;	TMP[  2] = C;	TMP[  3] = B;	TMP[  4] = B;
		TMP[  5] = B;	TMP[  6] = C;	TMP[  7] = B;	TMP[249] = C;
		TMP[251] = B;	TMP[253] = C;	TMP[255] = C;
	}
#undef	B
#undef	C
	im8Xlat( (LPBYTE)pObj->ebp, (LPBYTE)pObj->ebp, 0, (LPBYTE)TMP, R4(pObj->x), pObj->y );

	DeleteDC( hdc );
	DeleteObject( hb );
	DeleteObject( hp );
	InvalidateRect( hWnd, NULL, 0 );
	SetCursor(hoc);

}
/****************************************************************************/
/* Draw selection over Image ************************************************/
/****************************************************************************/
void	DrawMask( HWND hWnd, OBJ* O )
{
	if( NULL == O )
		return;
	InvalidateRect( hWnd, NULL, 0 );
	pntBitmapToScreen ( hWnd, O->ebp, O->x, O->y, O->xo, O->yo, O->scu, O->scd, NULL, TRUE );

}
/****************************************************************************/
template <typename _Tx>
VOID imageMulMove_t( _Tx *img, LPBYTE msk, LONG stride, LONG x, LONG y, double imdc )
{
	static const DOUBLE g_d_1_255 = 1.0 / 255.0; // the mask is scaled by 255
	
	for(int _y = 0; _y < y; _y++)
	{
		for(int _x = 0; _x < x; _x++)
		{
			img[_x] = (0 == msk[_x]) ? imdc : ((double)img[_x] - imdc) * (double)msk[_x] * g_d_1_255 + imdc;
		}
		img = (_Tx*)(((LPBYTE)img) + stride);
		msk += R4(x);
	}
}

template <typename _Tx>
DOUBLE imageCountFrameDC_t( _Tx *img, LPBYTE msk, LONG stride, LONG x, LONG y, int mbt )
{
	DOUBLE value = 0.0;
	DWORD count = 0;
	for(int _y = 0; _y < y; _y++)
	{
		for(int _x = 0; _x < x; _x++)
		{
			if( mbt == msk[_x] )
			{
				value += img[_x];
				count++;
			}
		}
		img = (_Tx*)(((LPBYTE)img) + stride);
		msk += R4(x);
	}
	return (0 == count) ? 0.0 : (_Tx)(value / count);
}

void	CutImage( HWND hWnd, OBJ* O, BOOL outside )
{
	LPBYTE	bim;
	double	frdc;
	HCURSOR	hcur = SetCursor(hcWait);
	//int		i;

	if( NULL == O )
		return;

	memset( TMP, 0, 256);
	if (outside)
	{
		TMP[  1] = -1;	TMP[  2] = -1;	TMP[  3] = -1;	TMP[  4] = -1;
		TMP[  5] = -1;	TMP[  6] = -1;	TMP[  7] = -1;	TMP[249] = -1;
		TMP[251] = -1;	TMP[253] = -1;	TMP[255] = -1;
	}
	else
	{
		TMP[  0] = -1;
	}

	// Mean value on edges
	switch( O->npix )
	{
	case PIX_CHAR: frdc = imageCountFrameDC_t<CHAR>( (LPSTR)O->dp, O->ebp, O->stride, O->x, O->y, 255 ); break;
	case PIX_BYTE: frdc = imageCountFrameDC_t<BYTE>( (LPBYTE)O->dp, O->ebp, O->stride, O->x, O->y, 255 ); break;
	case PIX_SHORT: frdc = imageCountFrameDC_t<SHORT>( (SHORT*)O->dp, O->ebp, O->stride, O->x, O->y, 255 ); break;
	case PIX_WORD: frdc = imageCountFrameDC_t<WORD>( (LPWORD)O->dp, O->ebp, O->stride, O->x, O->y, 255 ); break;
	case PIX_LONG: frdc = imageCountFrameDC_t<LONG>( (LPLONG)O->dp, O->ebp, O->stride, O->x, O->y, 255 ); break;
	case PIX_DWORD: frdc = imageCountFrameDC_t<DWORD>( (LPDWORD)O->dp, O->ebp, O->stride, O->x, O->y, 255 ); break;
	case PIX_FLOAT: frdc = imageCountFrameDC_t<FLOAT>( (FLOAT*)O->dp, O->ebp, O->stride, O->x, O->y, 255 ); break;
	case PIX_DOUBLE: frdc = imageCountFrameDC_t<DOUBLE>( (DOUBLE*)O->dp, O->ebp, O->stride, O->x, O->y, 255 ); break;
	}

	// TODO: check what is this function doing
	im8Xlat( O->ebp, O->ebp, 0, (LPBYTE)TMP, R4(O->x), O->y );

	if ( EdgeWidth > 1 )
	{											// Apply filter
		bim = O->ebp;
//		for (i=0; i<bb->biHeight; i++, bim+=bb->biWidth )
//			Img_Filter(bim, (int)bb->biWidth, 1, EdgeWidth);

// 		bim = O->ebp;
//		for (i=0; i<bb->biWidth;  i++, bim++ )
//			Img_Filter(bim, (int)bb->biHeight, (int)bb->biWidth, EdgeWidth);
	}

	if( NULL == O->edp )
	{
		int nSize = O->stride * O->y;
		O->edp = (LPBYTE)malloc( nSize );
		memcpy( O->edp, O->dp, nSize );
	}
	bim = O->ebp;

	switch( O->npix )
	{
	case PIX_CHAR:   imageMulMove_t<CHAR>((LPSTR)O->dp, bim, O->stride, O->x, O->y, frdc); break;
	case PIX_BYTE:   imageMulMove_t<BYTE>((LPBYTE)O->dp, bim, O->stride, O->x, O->y, frdc); break;
	case PIX_SHORT:  imageMulMove_t<SHORT>((SHORT*)O->dp, bim, O->stride, O->x, O->y, frdc); break;
	case PIX_WORD:   imageMulMove_t<WORD>((LPWORD)O->dp, bim, O->stride, O->x, O->y, frdc); break;
	case PIX_LONG:   imageMulMove_t<LONG>((LPLONG)O->dp, bim, O->stride, O->x, O->y, frdc); break;
	case PIX_DWORD:  imageMulMove_t<DWORD>((LPDWORD)O->dp, bim, O->stride, O->x, O->y, frdc); break;
	case PIX_FLOAT:  imageMulMove_t<FLOAT>((FLOAT*)O->dp, bim, O->stride, O->x, O->y, frdc); break;
	case PIX_DOUBLE: imageMulMove_t<DOUBLE>((DOUBLE*)O->dp, bim, O->stride, O->x, O->y, frdc); break;
	}

    O->flags &= ~OF_inedit;
	O->flags |=  OF_wasedit;
	InvalidateRect( hWnd, NULL, 0 );
	objInformChildren( hWnd, FM_Update, 0, 0);
	SetCursor( hcur );

}
/****************************************************************************/
static	VOID RotateImage( HWND hWnd, double angle )
{
	OBJ* O = OBJ::GetOBJ(hWnd);
	LPBYTE dst, lpSrc;
	LPDOUBLE lpD;
	LPLONG	lpW;
	int		x0, y0, x1, y1, i, j, srcw, dstw;
	double	nx0, nx1, nx2, nx3;
	double	ny0, ny1, ny2, ny3;
	double	ox0, ox1, ox2, ox3;
	double	oy0, oy1, oy2, oy3;
	double	dx, dy;
	double	si, co;
	double	fx1, fy1;
	HCURSOR	hc = SetCursor(hcWait);

	if( NULL == O )
		return;

	int neg = 0;

	x0 = O->x;
	y0 = O->y;

	si = sin(-angle/180.*M_PI);
	co = cos(-angle/180.*M_PI);
	if( fabs(si) < 1e-9 )
	{
		si = 0; co = (co < 0) ? -1 : 1;
	}
	else if( fabs(co) < 1e-9 )
	{
		co = 0; si = (si < 0) ? -1 : 1;
	}
	// center
	dx = x0/2.; dy = y0/2.;

	ox0 = -dx; ox1 = dx-1; ox2 = dx-1; ox3 = -dx;
	oy0 = -dy; oy1 = -dy;  oy2 = dy-1; oy3 =  dy-1;

	nx0 = (ox0*co-oy0*si);
	ny0 = (ox0*si+oy0*co);
	nx1 = (ox1*co-oy1*si);
	ny1 = (ox1*si+oy1*co);
	nx2 = (ox2*co-oy2*si);
	ny2 = (ox2*si+oy2*co);
	nx3 = (ox3*co-oy3*si);
	ny3 = (ox3*si+oy3*co);

	ox0 = __min(nx0,(__min(nx1,(__min(nx2,nx3)))));
	oy0 = __min(ny0,(__min(ny1,(__min(ny2,ny3)))));
	ox1 = __max(nx0,(__max(nx1,(__max(nx2,nx3)))));
	oy1 = __max(ny0,(__max(ny1,(__max(ny2,ny3)))));

	fx1 = ox1-ox0+1;
	fy1 = oy1-oy0+1;

	si = sin(angle/180.*M_PI);
	co = cos(angle/180.*M_PI);

	if( fabs(si) < 1e-9 )
	{
		si = 0; co = (co < 0) ? -1 : 1;
	}
	else if( fabs(co) < 1e-9 )
	{
		co = 0; si = (si < 0) ? -1 : 1;
	}

	nx0 = ox0; ny0 = oy0;
	nx1 = ox1; ny1 = oy0;
	nx2 = ox1; ny2 = oy1;
	nx3 = ox0; ny3 = oy1;

	ox0 = (nx0*co-ny0*si)+dx;
	oy0 = (nx0*si+ny0*co)+dy;
	ox1 = (nx1*co-ny1*si)+dx;
	oy1 = (nx1*si+ny1*co)+dy;
	ox2 = (nx2*co-ny2*si)+dx;
	oy2 = (nx2*si+ny2*co)+dy;
	ox3 = (nx3*co-ny3*si)+dx;
	oy3 = (nx3*si+ny3*co)+dy;

	x1 = round(fx1);
	y1 = round(fy1);

	dx = (ox3-ox0)/(y1-1);
	dy = (oy3-oy0)/(y1-1);

	// srcw = R4(x0);
	// dstw = R4(x1);
	srcw = O->CalculateStride();//R4(x0 * IMAGE::PixFmt2Bpp(O->npix));
	dstw = x1;

	O->x = x1;
	O->y = y1;
	O->xas = O->x;
	O->yas = O->y;
#ifdef USE_XB_LENGTH
	O->xb = R4(x1);
#else
	O->stride = /*R4( x1 * IMAGE::PixFmt2Bpp(O->npix));*/O->CalculateStride();
#endif

	lpW = (LPLONG)  malloc( sizeof(long)  * dstw);
	lpD = (LPDOUBLE)malloc( sizeof(double)* dstw);
	dst = (LPBYTE)  malloc( O->stride * y1);

	if (lpW==NULL || lpD==NULL || dst==NULL) goto ex;

	lpSrc = O->dp;

	for(i=0; i<y1; i++ )
	{
		memset( lpW, 0, sizeof(long)*dstw);
		memset( lpD, 0, sizeof(double)*dstw);
		switch( O->npix )
		{
		case PIX_BYTE:
			{
				LPBYTE lpDst = (LPBYTE)(((LPBYTE)dst) + i * O->stride);
				im8GetProfile ( (LPBYTE)lpSrc, srcw, x0, y0, neg, ox0,oy0,ox1,oy1, dstw, lpD, lpW);
				for(j=0;j<dstw;j++)
				{
					if (lpW[j]) lpDst[j] = round(lpD[j]);
					else        lpDst[j] = 1;
				}
// 				lpDst += dstw;
				break;
			}
		case PIX_CHAR:
			{
				LPSTR lpDst = (LPSTR)(((LPBYTE)dst) + i * O->stride);
				im8sGetProfile ( (LPSTR)lpSrc, srcw, x0, y0, neg, ox0,oy0,ox1,oy1, dstw, lpD, lpW);
				for(j=0;j<dstw;j++)
				{
					if (lpW[j]) lpDst[j] = round(lpD[j]);
					else        lpDst[j] = 1;
				}
// 				lpDst += dstw;
				break;
			}
		case PIX_WORD:
			{
				LPWORD lpDst = (LPWORD)(((LPBYTE)dst) + i * O->stride);
				im16GetProfile( (LPWORD)lpSrc, srcw, x0, y0, neg, ox0,oy0,ox1,oy1, dstw, lpD, lpW);
				for(j=0;j<dstw;j++)
				{
					if (lpW[j]) lpDst[j] = round(lpD[j]);
					else        lpDst[j] = 1;
				}
// 				lpDst += dstw;
				break;
			}
		case PIX_SHORT:
			{
				SHORT *lpDst = (SHORT*)(((LPBYTE)dst) + i * O->stride);
				im16sGetProfile( (SHORT*)lpSrc, srcw, x0, y0, neg, ox0,oy0,ox1,oy1, dstw, lpD, lpW);
				for(j=0;j<dstw;j++)
				{
					if (lpW[j]) lpDst[j] = round(lpD[j]);
					else        lpDst[j] = 1;
				}
// 				lpDst += dstw;
				break;
			}
		case PIX_DWORD:
			{
				LPDWORD lpDst = (LPDWORD)(((LPBYTE)dst) + i * O->stride);
				im32GetProfile( (LPDWORD)lpSrc, srcw, x0, y0, neg, ox0,oy0,ox1,oy1, dstw, lpD, lpW);
				for(j=0;j<dstw;j++)
				{
					if (lpW[j]) lpDst[j] = round(lpD[j]);
					else        lpDst[j] = 1;
				}
//				lpDst += dstw;
				break;
			}
		case PIX_LONG:
			{
				LPLONG lpDst = (LPLONG)(((LPBYTE)dst) + i * O->stride);
				im32sGetProfile( (LPLONG)lpSrc, srcw, x0, y0, neg, ox0,oy0,ox1,oy1, dstw, lpD, lpW);
				for(j=0;j<dstw;j++)
				{
					if (lpW[j]) lpDst[j] = round(lpD[j]);
					else        lpDst[j] = 1;
				}
// 				lpDst += dstw;
				break;
			}
		case PIX_FLOAT:
			{
				LPFLOAT lpDst = (LPFLOAT)(((LPBYTE)dst) + i * O->stride);
				imFGetProfile( (LPFLOAT)lpSrc, srcw, x0, y0, neg, ox0,oy0,ox1,oy1, dstw, lpD, lpW);
				for(j=0;j<dstw;j++)
				{
					if (lpW[j]) lpDst[j] = float(lpD[j]);
					else        lpDst[j] = 1;
				}
// 				lpDst += dstw;
				break;
			}
		case PIX_DOUBLE:
			{
				LPDOUBLE lpDst = (LPDOUBLE)(((LPBYTE)dst) + i * O->stride);
				imDGetProfile( (LPDOUBLE)lpSrc, srcw, x0, y0, neg, ox0,oy0,ox1,oy1, dstw, lpD, lpW);
				for(j=0;j<dstw;j++)
				{
					if (lpW[j]) lpDst[j] = double(lpD[j]);
					else        lpDst[j] = 1;
				}
// 				lpDst += dstw;
				break;
			}
		default:
			break;
		}
		ox0+=dx; ox1+=dx;
		oy0+=dy; oy1+=dy;
	}
	free( O->dp );
 	O->dp = dst;
// 	O->x = x1;
// 	O->y = y1;
// 	O->xas = O->x;
// 	O->yas = O->y;
	wndSetZoom( hWnd, O->scu, O->scd);
	objInformChildren( hWnd, FM_Update, 0, 0);
	if( O->edp )
	{
		free(O->edp);
		O->edp = NULL;
	}
	if (O->hBme)
	{
		DeleteBitmap( O->hBme );
		O->ebp = NULL;
		O->hBme = NULL;
	}
ex:
	if (lpW) free( lpW );
	if (lpD) free( lpD );
	SetCursor( hc );

}
/****************************************************************************/
static VOID _cdecl LiveProc( LPVOID lpv )
{
	try
	{
	OBJ*	O = OBJ::GetOBJ((HWND) lpv);
	int		bHalf=0;
	LPBYTE	pBf0, pBf1;
	RECT	rc = {0, 0, 0, 0};
	char	tmp[100];
	DWORD	t0, nf;
	UINT	fmt;
	HCURSOR hoc;

	if (O == NULL)
		return;

	rc.right = O->x;
	rc.bottom = O->y;
	fmt = (O->npix == PIX_WORD) ? S4_SF_R16 : S4_SF_R8;

//	if (!s4HookLiveBuffer(O->dp, O->xb*O->pix, &rc, (O->pix==2)?S4_SF_R16:S4_SF_R8, O->xb*O->y*O->pix))
#ifdef USE_XB_LENGTH
	if (!g_pfnS4HOOKLIVEBUFFER(O->dp, O->xb*IMAGE::PixFmt2Bpp(O->npix), &rc,
		fmt, O->xb*O->y*IMAGE::PixFmt2Bpp(O->npix)))
#else
	if( !g_pfnS4HOOKLIVEBUFFER(O->dp, O->stride, &rc, fmt, O->stride*O->y) )
#endif
	{
		MessageBeep(0xFFFF);
		return;
	}
	hwndLive = (HWND)lpv;
	g_pfnS4SELECTLIVEBUFFER( bHalf );
	bHalf = !bHalf;
	g_pfnS4LIVEBUFFERREADY();
	g_pfnS4START();
	pBf0 = (LPBYTE)O->dp;
#ifdef USE_XB_LENGTH
	pBf1 = pBf0 + O->xb*O->y*IMAGE::PixFmt2Bpp(O->npix);
#else
	pBf1 = pBf0 + O->stride*O->y;
#endif

	// ADDED BY PETER on 2003 Jan 30
	O->maxcontr = TRUE;

	t0 = timeGetTime();
	nf=0;
	while(bGrabberActive)
	{
		while(g_pfnS4GETSTATUS() & SH4_S_LBBUSY)
		{
			Sleep(1);
			if (!bGrabberActive)
				goto exit;
		}
		if (!bInformChilds)
		{
			nf++;
			if (nf==10)
			{
				t0 = timeGetTime()-t0;
				if (t0)
				{
					sprintf(tmp,"%.1f fps", 1000.*10./(double)t0);
					tobSetInfoText( ID_DDB, tmp );
				}
				t0 = timeGetTime();
				nf=0;
			}
			g_pfnS4SELECTLIVEBUFFER( bHalf ); bHalf = !bHalf;
			O->dp = (bHalf ? pBf1 : pBf0);
			if (O->maxcontr)
			{
				imgCalcMinMax( O );
			}
//			SendNotifyMessage(hwndLive,FM_Update,0,0);
			PostMessage(hwndLive,FM_Update,0,0);
		}
		g_pfnS4LIVEBUFFERREADY();
		Sleep(0);
	}
exit:
	// removed by Peter on 1 Oct 2001
	if (!bAbrupt)
	{
		DWORD t0;
		g_pfnS4SELECTLIVEBUFFER( 0 );
		g_pfnS4LIVEBUFFERREADY();
		hoc=SetCursor(hcWait);
		GetAsyncKeyState(VK_ESCAPE);

		while(g_pfnS4GETSTATUS() & SH4_S_LBBUSY)
		{
			if (GetAsyncKeyState(VK_ESCAPE))
				break;
		}
		g_pfnS4STOP();

		t0 = timeGetTime();
		while(( g_pfnS4GETSTATUS() &(SH4_S_GDONE|SH4_S_DDONE)) != (SH4_S_GDONE|SH4_S_DDONE))
		{
			if (GetAsyncKeyState(VK_ESCAPE)) break;		// Eacape pressed
			if ((timeGetTime() - t0 ) > 30000 ) break;	// more than 30 sec timeout
		}
		O->dp = pBf0;
		imgCalcMinMax( O );
		O->maxcontr = TRUE;
		SetCursor(hoc);
		sprintf( O->title, "Grabbed image #%d", O->imageserial );
		sprintf(TMP, "%s - %dx%dx%d (%1d:%1d)",
			O->title,
			O->x,
			O->y,
			8*IMAGE::PixFmt2Bpp(O->npix),
			O->scu,
			O->scd);
		SetWindowText( hwndLive, TMP );
		InvalidateRect( hwndLive, NULL, FALSE );
	}
	else
	{
		g_pfnS4STOP();
	}
	g_pfnS4HOOKLIVEBUFFER(NULL, 0, NULL, 0, 0);
	O->flags &= ~OF_grabberON;	// Clear grabberON flag
	hwndLive = NULL;
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("LiveWndProc() -> %s"), e.what()));
	}
}
/****************************************************************************/
static void DoLiveShark4( HWND hWnd )
{
	hLiveThreadShark4 = (HANDLE)_beginthread( LiveProc,  0, (LPVOID)hWnd );
	if (hLiveThreadShark4 == (HANDLE)-1)
	{
		hLiveThreadShark4 = NULL;
		return;
	}
//	::SetThreadPriority(hLiveThreadShark4, THREAD_PRIORITY_LOWEST);

}

void PaintImage(HWND hWnd, OBJ* pObj)
{
	pntBitmapToScreen( hWnd, pObj->dp, pObj->x, pObj->y, pObj->xo, pObj->yo, pObj->scu, pObj->scd, pObj, FALSE);
}

/****************************************************************************/
LRESULT CALLBACK ImageWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		OBJ*	pObj = OBJ::GetOBJ(hWnd);
		int xPos, yPos;

#ifdef _USE_LOGGING
		if( NULL != s_pLog )
		{
			fprintf(s_pLog, "msg: IMG   %8X(%d)\n", message, message);
		}
#endif
		if( NULL == pObj )
			goto CallDCP;
//			return -1;

		switch (message)
		{
		case WM_CREATE:
			if ( bGrabberActive && (pObj->flags&OF_grabberON)!=0 )
			{
				// changed by Peter on 22 Aug 2002
				if( GB_USE_SHARK4 == nUseGrabber )
				{
					DoLiveShark4( hWnd );
				}
				else if( GB_USE_MV40 == nUseGrabber )
				{
					DoLiveMV40( hWnd );
				}
			}
			wndSetInitialWindowSize( hWnd );
			break;

		case WM_NCDESTROY:
	//		if (pObj->dp000) free( pObj->dp000 ); { pObj->dp000=NULL; pObj->dp=NULL; }
			// Changed by Peter on 03 Feb. 2003. Pointer dp sometimes is equal to dp000!
			if( pObj->dp000 == pObj->dp )
			{
				pObj->dp000 = NULL;
			}
			// Changed by Peter on 4 Oct. 2002. Delete correct
			if (pObj->dp000) { free( pObj->dp000 ); pObj->dp000 = NULL; }
			if (pObj->dp) { free( pObj->dp ); pObj->dp=NULL; }
			if (pObj->hBme)
			{
				DeleteBitmap(pObj->hBme);
				pObj->ebp = NULL;
				pObj->hBme = NULL;
			}
			objFreeObj( hWnd );
			goto CallDCP;

		case FM_Update:
			InvalidateRect(hWnd,NULL,FALSE);
			UpdateWindow(hWnd);
			objInformChildren( hWnd, FM_Update, 0, 0);
    		break;

		case WM_MOUSEMOVE:
			pObj->nMouseMessage = message;
			if ( pObj->flags & OF_indrag ) DragEdit( hWnd, pObj, lParam, wParam);
			ReportCoordinates( lParam, hWnd );
			if ((wParam & (MK_LBUTTON | MK_MBUTTON)))
			{
				objInformChildren( hWnd, FM_ParentInspect, wParam, lParam);
			}
			break;

		case WM_LBUTTONDOWN:
			pObj->nMouseMessage = message;
			if ( pObj->flags & OF_indrag ) DragEdit( hWnd, pObj, lParam, wParam); // New point in free hand
			else ClickEdit( hWnd, pObj, lParam, wParam );	// New wire select
			objInformChildren( hWnd, FM_ParentInspect, wParam, lParam);
			break;

		case WM_LBUTTONUP:
			pObj->nMouseMessage = message;
			if ( (pObj->flags & OF_indrag) && (EditMode != IDC_IE_FREESEL) )
			{
				// If in drag, and not free hand selection, then update mask
				ReleaseCapture();
				pObj->flags &= ~OF_indrag;
				UpdateMask( hWnd, pObj );
			}
			break;

		case WM_LBUTTONDBLCLK:
			pObj->nMouseMessage = message;
			if (( pObj->flags & OF_indrag) && (EditMode == IDC_IE_FREESEL) )
			{
				// If in drag, and free hand selection, then update mask
				ReleaseCapture();
				pObj->flags &= ~OF_indrag;
				UpdateMask( hWnd, pObj );
			}
			break;

		case WM_RBUTTONDOWN:
			pObj->nMouseMessage = message;
			if ((pObj->flags & OF_indrag) && (Epcnt>1)) // Delete one vertex from polyline
				DragEdit( hWnd, pObj, lParam, wParam);
			break;

		case WM_RBUTTONUP:
			pObj->nMouseMessage = message;
			wndCreateObjMenu(hWnd, FALSE, TRUE, TRUE, lParam);
			break;

		case WM_MBUTTONDOWN:
			pObj->nMouseMessage = message;
			if ((wParam & (MK_LBUTTON | MK_MBUTTON)))
				objInformChildren( hWnd, FM_ParentInspect, wParam, lParam);
			break;

		case WM_MOUSEWHEEL:
	//			int fwKeys = GET_KEYSTATE_WPARAM(wParam);
//			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			pObj->nMouseMessage = message;
			xPos = GET_X_LPARAM(lParam);
			yPos = GET_Y_LPARAM(lParam);
			break;

		case WM_HSCROLL:
			wndUpdateScrollBars( hWnd, wParam, lParam, SB_HORZ);
			objInformChildren( hWnd, FM_ParentSize, (WPARAM)pObj, 0);
			break;

		case WM_VSCROLL:
			wndUpdateScrollBars( hWnd, wParam, lParam, SB_VERT);
			objInformChildren( hWnd, FM_ParentSize, (WPARAM)pObj, 0);
			break;

		case WM_NCCALCSIZE:
			if ( pObj && wParam ) wndTrackWindow( hWnd, pObj, (LPRECT)lParam );
			goto CallDCP;

		case WM_SIZE:
			// Update new window size in case of "normal" sizes
			DefWindowProc(hWnd, message, wParam, lParam);
			if ((wParam == SIZENORMAL)||(wParam == SIZEFULLSCREEN)) {
				pObj->xw = LOWORD(lParam);
				pObj->yw = HIWORD(lParam);
				wndSetScrollOrigin( hWnd, TRUE );
				wndInitScrollBars( hWnd );
				objInformChildren( hWnd, FM_ParentSize, (WPARAM)pObj, 0);
			}
			break;

		case WM_ERASEBKGND:
			return 1;
//		return FALSE;

		case WM_GETMINMAXINFO:
			// Max track size
			if ( pObj )
			{
				RECT r;
				SetRect(&r, 0, 0, pObj->x * pObj->scu / pObj->scd, pObj->y * pObj->scu / pObj->scd);
				AdjustWindowRectEx( &r, GetWindowLong(hWnd,GWL_STYLE), FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
				((LPPOINT)lParam)[4].x = r.right-r.left;
				((LPPOINT)lParam)[4].y = r.bottom-r.top;
			}
			break;

		case WM_SYSCOMMAND:
			if (wndDefSysMenuProc(hWnd, wParam, lParam)) break;
			goto CallDCP;

		case WM_CLOSE:
			if (hWnd == hwndLive)
			{
				if(bGrabberActive == FALSE)	// the grabber is busy
				{
					// changed by Peter on 22 Aug 2002
					if( GB_USE_SHARK4 == nUseGrabber )
					{
						TerminateThread(hLiveThreadShark4, -1);
						hLiveThreadShark4 = NULL;
					}
					else if( GB_USE_MV40 == nUseGrabber )
					{
						TerminateMV40Capturing();
					}
					hwndLive = NULL;
				}
				else
				{
					sprintf(TMP, "To close 'Live image' window:\r\n"
						" - release 'GRAB' button on the toolbar to stop capturing process;\r\n"
						" - close the window in usual way.");
					MessageBox(NULL, TMP, "CRISP warning", MB_OK);
					return 0;
				}
			}
			goto CallDCP;

		case WM_COMMAND:
			if (wndDefSysMenuProc(hWnd, wParam, lParam)) break;
			goto CallDCP;

		case WM_MOUSEACTIVATE:
		case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
		case WM_QUERYOPEN:
			ActivateObj(hWnd);
			goto CallDCP;

/*
		case MYWM_QUERYNEWPALETTE:
			{
			HDC hDC;
			HPALETTE hOldPal;
			int nColorsChanged;

			hDC     = GetDC (MainhWnd);
			if (I->hPal==NULL) hOldPal = SelectPalette (hDC, hMyPal,  FALSE);
			else               hOldPal = SelectPalette (hDC, I->hPal, FALSE);
			nColorsChanged = RealizePalette (hDC);

			if (nColorsChanged) InvalidateRect (hWnd, NULL, FALSE);

			if (hOldPal) SelectPalette (hDC, hOldPal, FALSE);

			ReleaseDC (MainhWnd, hDC);

			return (nColorsChanged != 0);
			}
*/
		case WM_PALETTECHANGED:
			if (hWnd==(HWND)wParam) break;
			{
				HDC hDC;
				HPALETTE hOldPal;
				hDC     = GetDC (hWnd);
				hOldPal = SelectPalette (hDC, pntGetPal(), TRUE);

				RealizePalette (hDC);
				UpdateColors (hDC);

				if (hOldPal) SelectPalette (hDC, hOldPal, FALSE);

				ReleaseDC (hWnd, hDC);
			}
			break;

		case WM_PAINT:
		// Start by PETER on 21 Aug 2001
			if( ::IsIconic(hWnd) == FALSE )		// non-iconic window
			{
				PaintImage(hWnd, pObj);
				if ( pObj->flags & OF_inedit ) DrawMask( hWnd, pObj );
				if ( pObj->flags & OF_indrag ) DrawWire( hWnd, pObj );
				objInformChildren( hWnd, FM_ParentPaint, (WPARAM)pObj, 0 );
			}
			else
			{
//				OutputDebugString("Drawing in iconized window\n");
			}
			break;

		case WM_WINDOWPOSCHANGING:
			{
				LPWINDOWPOS wp = (LPWINDOWPOS)lParam;
				// Setting this bit prevents(?) BitBlt while moving
				wp->flags |= SWP_NOCOPYBITS;
			}
			goto CallDCP;

		case WM_INITMENUPOPUP:
			if (HIWORD(lParam)==0)
				CheckMenuItem((HMENU)wParam, SC_myZOOM | (pObj->scu<<8) | (pObj->scd<<4), MF_BYCOMMAND | MF_CHECKED);
			break;

		default:
	CallDCP:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("ImageWndProc() -> message '%s': '%s'"), message, e.what()));
	}
	return FALSE;
}
/****************************************************************************/
/* Image edit tool selection window *****************************************/
/****************************************************************************/
BOOL CALLBACK SelectETDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ* O=NULL;

	if (hActive) O = OBJ::GetOBJ(hActive);

	switch (message)
	{
		case WM_INITDIALOG:
			regReadWindowPos( hDlg, "SelectET");
			SendDlgItemMessage( hDlg, IDC_IE_CCIRC, BM_SETIMAGE, IMAGE_BITMAP,
				(LPARAM)LoadImage(hInst, "BMCCIRC", IMAGE_BITMAP, 0,0, LR_DEFAULTCOLOR));
			SendDlgItemMessage( hDlg, IDC_IE_FCIRC, BM_SETIMAGE, IMAGE_BITMAP,
				(LPARAM)LoadImage(hInst, "BMFCIRC", IMAGE_BITMAP, 0,0, LR_DEFAULTCOLOR));
			SendDlgItemMessage( hDlg, IDC_IE_RECT, BM_SETIMAGE, IMAGE_BITMAP,
				(LPARAM)LoadImage(hInst, "BMRECT", IMAGE_BITMAP, 0,0, LR_DEFAULTCOLOR));
			SendDlgItemMessage( hDlg, IDC_IE_SQUARE, BM_SETIMAGE, IMAGE_BITMAP,
				(LPARAM)LoadImage(hInst, "BMSQUARE", IMAGE_BITMAP, 0,0, LR_DEFAULTCOLOR));
			SendDlgItemMessage( hDlg, IDC_IE_FREESEL, BM_SETIMAGE, IMAGE_BITMAP,
				(LPARAM)LoadImage(hInst, "BMFREESEL", IMAGE_BITMAP, 0,0, LR_DEFAULTCOLOR));
			EditMode = IDC_IE_CCIRC;
			CheckDlgButton( hDlg, EditMode, TRUE );
			UpdateSelect( hDlg );
			return FALSE;

		case WM_INITINFO:
			UpdateSelect( hDlg );
			break;

		case WM_NCDESTROY:
			regWriteWindowPos( hDlg, "SelectET", FALSE);
			break;

		case WM_ACTIVATE:
			if (LOWORD(wParam)==WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
				UpdateSelect( hDlg );
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_IE_CCIRC:
			case IDC_IE_FCIRC:
			case IDC_IE_SQUARE:
			case IDC_IE_RECT:
			case IDC_IE_FREESEL:
				EditMode = LOWORD(wParam);
				break;

			case IDC_IE_CUTIN:
				CutImage( hActive, OBJ::GetOBJ(hActive) , TRUE);
				goto CutCont;
			case IDC_IE_CUTOUT:
				CutImage( hActive, OBJ::GetOBJ(hActive) , FALSE);
CutCont:
				UpdateSelect( hDlg );
				objInformChildren( hActive, FM_Update, 0, 0);
				break;

			case IDC_IE_RESTORE:
				O->flags &= ~OF_inedit;
				ReleaseSelection(hActive, OBJ::GetOBJ(hActive));
				if (O->edp!=NULL)
				{
#ifdef USE_XB_LENGTH
					memcpy( O->dp, O->edp, O->xb * O->y * IMAGE::PixFmt2Bpp(O->npix) );
#else
					memcpy( O->dp, O->edp, O->stride * O->y );
#endif
					InvalidateRect( hActive, NULL, FALSE );
					O->flags &= ~OF_wasedit;
				}
				UpdateSelect( hDlg );
				break;

//			case IDC_IE_EDGE:
//				DialogBox(hInst, "Edge", hWnd, EdgeDlgProc );
//				return TRUE;

			case IDC_IE_ROTATE:
				GetDlgItemDouble( hDlg, IDC_IE_ANGLE, &RotateAng, 0 );
				if (O->hBme)
				{
					DeleteBitmap(O->hBme);
					O->ebp = NULL;
					O->hBme = NULL;
				}
				RotateImage( hActive, RotateAng );
				UpdateSelect( hDlg );
				return TRUE;

			case IDOK:
			case IDCANCEL:
				if (hActive) ReleaseSelection(hActive, OBJ::GetOBJ(hActive));
				DestroyWindow(hDlg);
				hSelectET = NULL;
				EditMode = 0;
				return TRUE;
			}
			break;
	}
	return FALSE;
}

