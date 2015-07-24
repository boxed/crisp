/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.paint * by ML *****************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "objects.h"
#include "shtools.h"
#include "commondef.h"
#include "const.h"
#include "common.h"
#include "resource.h"
#include "globals.h"
#include "rgconfig.h"

#pragma optimize ("", off)

/****************************************************************************/
/* BitMap Paint routines * by ML ********************************************/
/****************************************************************************/

HPALETTE	hMyPal=NULL;		// Handle to logical palette

static	myLOGPAL	LogPal    = { 0x300, GRAYSIZ+MAXPAL*USERSIZ+RESISIZ+1, 0 };
static	myLOGPAL	LogPalSav;

static int				pal1x[RESISIZ] =
{ 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255};

static	LPBYTE lpScrBuf = NULL;

HPEN		red_pen;			// Red pen
HPEN		blue_pen;			// Blue pen
HPEN		green_pen;			// Green pen
HPEN		grey_pen;			// Grey pen
HPEN		yellow_pen;			// Yellow pen
HPEN		magenta_pen;		// Magenta pen

#define		GetPtr(_Tx,src,x,y,stride)		((_Tx*)(((LPBYTE)src) + (stride * (y))) + x)

//static BYTE wxlt[65536];

#define	RX(a,z)	((int)(z*int((a+z-1)/z)))

/****************************************************************************/

template <typename _Tx>
void pnt_CopyX(_Tx *inSrc, int bw, int x0, int y0, LPBYTE dst,
			   int width, int height, int scale, OBJ* pObj, size_t nPixelSize)
{
	int		i, j, xc, yc;
	int		sw = R8(width);
	BYTE	b;
	WORD	w;
	DWORD	dw, dw0;
	double	dt;
	int		nDstOff;
	double	val;
	double	imax, imin;

	// qword aligned width
	int xw = R8(width);
	int nSrcStride = R4(bw * nPixelSize);
	_Tx* src = GetPtr(_Tx, inSrc, x0, y0, nSrcStride);

	if (src == NULL)
	{
		throw std::exception("failed to run pnt_CopyX: no image data");
	}
	//Trace(Format("pnt_CopyX %1% %2% %3% %4% %5%") % (int)src % (int)inSrc % x0 % y0 % nSrcStride);

	// maximum contrast?
	if( NULL != pObj )
	{
		if( pObj->maxcontr )
		{
			imax = pObj->imax;
			imin = pObj->imin;
// 			dt = 255.0 / (imax - imin);
		}
		else
		{
			imax = pObj->imaxa;
			imin = pObj->imina;
// 			dt = 255.0 / ((double)numeric_limits<_Tx>::max() + 1.0);
		}
		dt = 255.0 / (imax - imin);
	}
	else
	{
		// TODO: find the Min & Max
		imax = 255;
		imin = 0;
		dt = 255.0 / ((double)numeric_limits<_Tx>::max() + 1.0);
	}
	if( scale < 0 )
	{
		scale = -scale;
		nDstOff = sw - width;
		for(i = 0; i < height; i++)
		{
			for(j = 0; j < width; j++, dst++)
			{
				// the scaled value
				val = (src[j * scale] - imin) * dt;
				if( val <= 0.0 )		val = 0;
				else if( val > 255.0 )	val = 255;
				*dst = (_Tx)val;
			}
			src = (_Tx*)(((LPBYTE)src) + nSrcStride * scale);
			dst += nDstOff;
		}
	}
	else
	{
		xc = xw / scale;
		yc = RX(height, scale) / scale;
		nDstOff = (sw - xc) * scale;
		switch( scale )
		{
		case 1:
			nDstOff = sw - width;
			for(i = 0; i < height; i++)
			{
				for(j = 0; j < width; j++, dst++)
				{
					val = (src[j] - imin) * dt;
					if( val <= 0.0 )		val = 0;
					else if( val > 255.0 )	val = 255;
					*dst = (BYTE)val;
				}
				src = (_Tx*)(((LPBYTE)src) + nSrcStride);
				dst += nDstOff;
			}
			break;
		case 2:
// 			xc = /*width*/xw >> 1;
// 			nDstOff = (sw - xc) << 1;
// 			yc = R2(height) >> 1;
			for(i = 0; i < yc; i++)
			{
				for(j = 0; j < xc; j++, dst += 2)
				{
					val = (src[j] - imin) * dt;
					if( val <= 0.0 )		val = 0;
					else if( val > 255.0 )	val = 255;
					b = (_Tx)val;
					w = MAKEWORD(b, b);
					*(LPWORD)(dst) = *(LPWORD)(dst+sw) = w;
				}
				src = (_Tx*)(((LPBYTE)src) + nSrcStride);
				dst += nDstOff;
			}
			break;
		case 3:
//			xc = /*width*/xw / 3;
//			nDstOff = (sw - xc) * 3;
// 			yc = R3(height) / 3;
			for(i = 0; i < yc; i++)
			{
				for(j = 0; j < xc; j++, dst += 3)
				{
					val = (src[j] - imin) * dt;
					if( val <= 0.0 )		val = 0;
					else if( val > 255.0 )	val = 255;
					b = (_Tx)val;
					w = MAKEWORD(b, b);
					*(LPWORD)(dst) = w;
					*(dst+2)  = b;
					*(LPWORD)(dst+sw) = w;
					*(dst+sw+2) = b;
					*(LPWORD)(dst+(sw<<1)) = w;
					*(dst+(sw<<1)+2) = b;
				}
				dst += nDstOff;
				src = (_Tx*)(((LPBYTE)src) + nSrcStride);
			}
			break;
		case 4:
//			xc = /*width*/xw >> 2;
//			nDstOff = (sw - xc) << 2;
// 			yc = R4(height) >> 2;
			for(i = 0; i < yc; i++)
			{
				for(j = 0; j < xc; j++, dst+=4)
				{
					val = (src[j] - imin) * dt;
					if( val <= 0.0 )		val = 0;
					else if( val > 255.0 )	val = 255;
					b = (_Tx)val;
					w = MAKEWORD(b, b);
					dw = w | (w<<16);
					*(LPDWORD)(dst) = dw;
					*(LPDWORD)(dst+sw) = dw;
					*(LPDWORD)(dst+(sw<<1)) = dw;
					*(LPDWORD)(dst+(sw)*3) = dw;
				}
				dst += nDstOff;
				src = (_Tx*)(((LPBYTE)src) + nSrcStride);
			}
			break;
		case 8:
//			xc = /*width*/xw >> 3;
//			nDstOff = (sw - xc) << 3;
// 			yc = R8(height) >> 3;
			for(i = 0; i < yc; i++)
			{
				for(j = 0; j < xc; j++, dst+=8)
				{
					val = (src[j] - imin) * dt;
					if( val <= 0.0 )		val = 0;
					else if( val > 255.0 )	val = 255;
					b = (_Tx)val;
					w = MAKEWORD(b, b);
					dw = w | (w<<16);
					dw0 = dw & 0xFFFFFF00;
					*(LPDWORD)(dst)      =   0; *(LPDWORD)(dst+4)      = 0;
					*(LPDWORD)(dst+sw)   = dw0; *(LPDWORD)(dst+sw+4)   = dw;
					*(LPDWORD)(dst+sw*2) = dw0; *(LPDWORD)(dst+sw*2+4) = dw;
					*(LPDWORD)(dst+sw*3) = dw0; *(LPDWORD)(dst+sw*3+4) = dw;
					*(LPDWORD)(dst+sw*4) = dw0; *(LPDWORD)(dst+sw*4+4) = dw;
					*(LPDWORD)(dst+sw*5) = dw0; *(LPDWORD)(dst+sw*5+4) = dw;
					*(LPDWORD)(dst+sw*6) = dw0; *(LPDWORD)(dst+sw*6+4) = dw;
					*(LPDWORD)(dst+sw*7) = dw0; *(LPDWORD)(dst+sw*7+4) = dw;
				}
				dst += nDstOff;
				src = (_Tx*)(((LPBYTE)src) + nSrcStride);
			}
			break;
		}
	}
}

/****************************************************************************/
static void	ChangePalette( void )
{
	HDC	hDC = GetDC( NULL );
	HPALETTE	hopal;

	hopal = SelectPalette( hDC, hMyPal, FALSE );
	AnimatePalette( hMyPal, 0, GRAYSIZ+USERSIZ*4, (LPPALETTEENTRY)LogPal.palRGBF);
	if (hopal) SelectPalette( hDC, hopal, FALSE );
	ReleaseDC( NULL, hDC );
}
/****************************************************************************/
void DrawTransparentBitmap(HDC hdc, LPBITMAPINFO bhdr, LPBYTE lpBits, int SrcX, int SrcY,
						   COLORREF cTransparentColor, HPALETTE hp)
{
	BITMAP		bm;
	COLORREF	cColor;
	HBITMAP		hBitmap, bmTempOld;
	HBITMAP		bmAndBack, bmAndObject, bmAndMem;
	HBITMAP		bmBackOld, bmObjectOld, bmMemOld;
	HDC			hdcMem, hdcBack, hdcObject, hdcTemp;
	POINT		ptSize;

	hdcTemp = CreateCompatibleDC(hdc);
	hBitmap = CreateDIBitmap( hdc, (LPBITMAPINFOHEADER)bhdr, CBM_INIT, lpBits,
							 (LPBITMAPINFO)bhdr, DIB_PAL_COLORS);

	bmTempOld = SelectBitmap(hdcTemp, hBitmap);   // Select the bitmap

	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

	ptSize.x = bm.bmWidth;            // Get width of bitmap
	ptSize.y = bm.bmHeight;           // Get height of bitmap
	DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device
	                                  // to logical points
	// Create some DCs to hold temporary data.
	hdcBack   = CreateCompatibleDC(hdc);
	hdcObject = CreateCompatibleDC(hdc);
	hdcMem    = CreateCompatibleDC(hdc);

	// Create a bitmap for each DC. DCs are required for a number of
	// GDI functions.

	// Monochrome DC
	bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

	// Monochrome DC
	bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

	bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

	// Each DC must select a bitmap object to store pixel data.
	bmBackOld   = SelectBitmap(hdcBack, bmAndBack);
	bmObjectOld = SelectBitmap(hdcObject, bmAndObject);
	bmMemOld    = SelectBitmap(hdcMem, bmAndMem);

	// Set proper mapping mode.
	SetMapMode(hdcTemp, GetMapMode(hdc));
	SelectPalette( hdcTemp, hp, FALSE );
//	RealizePalette( hdcTemp );

	// Set the background color of the source DC to the color.
	// contained in the parts of the bitmap that should be transparent
	cColor = SetBkColor(hdcTemp, cTransparentColor);

	// Create the object mask for the bitmap by performing a BitBlt()
	// from the source bitmap to a monochrome bitmap.
	BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, SrcX, SrcY, SRCCOPY);

	// Set the background color of the source DC back to the original
	// color.
	SetBkColor(hdcTemp, cColor);


	// Create the inverse of the object mask.
	BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);

	// Copy the background of the main DC to the destination.
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, 0, 0, SRCCOPY);

	// Mask out the places where the bitmap will be placed.
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

	// Mask out the transparent colored pixels on the bitmap.
	BitBlt(hdcTemp, SrcX, SrcY, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

	// XOR the bitmap with the background on the destination DC.
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, SrcX, SrcY, SRCPAINT);


	// Copy the destination to the screen.
	BitBlt(hdc, 0, 0, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY);

	// Delete the memory bitmaps.
	DeleteObject(SelectObject(hdcBack,   bmBackOld));
	DeleteObject(SelectObject(hdcObject, bmObjectOld));
	DeleteObject(SelectObject(hdcMem,    bmMemOld));
	DeleteObject(SelectObject(hdcTemp,   bmTempOld));

	// Delete the memory DCs.
	DeleteDC(hdcMem);
	DeleteDC(hdcBack);

	DeleteDC(hdcObject);
	DeleteDC(hdcTemp);
}
/****************************************************************************/
VOID pntBitmapToScreen(HWND hWnd, LPBYTE p, int w, int h, int x0, int y0,
					   int scu, int scd, OBJ* pObj, int flags)
{
	myBITMAPINFO	bi= { sizeof(BITMAPINFOHEADER), 0, 0, 1, 8, BI_RGB, 0, 0, 0, 0, 0 };
	HDC         hDC;
	HBRUSH		hbr;
	HPALETTE	hPalette=NULL;
	RECT		rw;
	int			transparent=0, uu=0, i, j, bw = R4(w);
	int			sw, scale, xw, yw;
	int			li, ls;
	int			nWidth, nHeight, SrcX, SrcY, nNumScans, ofs=0;
	LPBYTE		lpBits;
//	BOOL		b16 = FALSE;
	PAINTSTRUCT	ps;
	double		dScale;

	try
	{
	if( !((flags & 0x1000) || GetUpdateRect( hWnd, &rw, FALSE)) )
		return; // Nothing to do
	transparent = flags & 0xFF;
	uu = (flags & 0x700) >> 8;
	dScale = double(scu) / double(scd);

	bi.biBitCount = 8;
	bi.biClrUsed  = COLORS;
	bi.biClrImportant = COLORS;
	bi.biSizeImage = 0;
	if( pObj )
	{
		if( pObj->palno < 0 )
		{
			ofs = 0;
			ls = GRAYSIZ;
		}
		else
		{
			ofs = GRAYSIZ + pObj->palno*USERSIZ;
			ls = USERSIZ;
		}
		for(i = 0; i < 256; i++)
		{
//			li = (((i*ls)/256 + (pObj->bright*ls)/255 - ls/2)*pObj->contrast)/128;
			li = (( ((i*ls)>>8) + (pObj->bright*ls)/255 - ls/2)*pObj->contrast) >> 7;
			if (li<0) li=0;
			if (li>=ls) li=ls-1;
			li += ofs;
			bi.index[i] = (BYTE)li;
		}
//		if( pObj->pix == 2 ) b16 = TRUE;
	}
	else
	{
		/*
// 		ofs = 0;
// 		ls = GRAYSIZ;
		for(i = 0; i < GRAYSIZ; i++)
		{
			bi.index[i] = i;
// 			li = (( ((i*ls)>>8) + (128*ls)/255 - ls/2)*128) >> 7;
// 			if (li<0) li=0;
// 			if (li>=ls) li=ls-1;
// 			li += ofs;
// 			bi.index[i] = (BYTE)li;
		}
/*/
		j = 0;
		for(i = 0; i < RESISIZ; i++)
		{
			for( ; j < pal1x[i]; j++)
			{
				bi.index[j] = i + GRAYSIZ + (USERSIZ << 2);//4*USERSIZ;
			}
		}
//*/
	}
	if( transparent )
	{
//		for(i=0; i<256; i++) bi.index[i] = 0;
		bi.index[255] = GRAYSIZ+4*USERSIZ+RESISIZ;
	}

	GetClientRect( hWnd, &rw );
	InflateRect( &rw, -uu, -uu );
	InvalidateRect(hWnd, NULL, FALSE);
	xw = rw.right - rw.left;
	yw = rw.bottom - rw.top;

	RECT fr = rw;
	hbr = ::CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	xw = __min(__min(xw, w*dScale/*scu/scd*/), GetSystemMetrics(SM_CXSCREEN));
	yw = __min(__min(yw, h*dScale/*scu/scd*/), GetSystemMetrics(SM_CYSCREEN));

	if( (xw<=0) || (yw<=0) )
	{
		ValidateRect(hWnd, NULL);
		return;
	}

	hDC = BeginPaint( hWnd, &ps );
	hPalette = SelectPalette(hDC, hMyPal, FALSE);
	RealizePalette(hDC);

	if( IsIconic(hWnd) )
	{
//		if (b16);
//			pnt_Icon16(p, w, h, lpScrBuf, xw, yw);
//		else;
//			pnt_Icon(p, w, h, lpScrBuf, xw, yw);
		sw = R4(xw);
		bi.biWidth  = sw;
		bi.biHeight = -yw;
		nWidth = xw;
		nHeight = yw;
		SrcX = SrcY = 0;
		nNumScans = yw;
		lpBits = lpScrBuf;
	}
	else
	{
		scale = 1;
		if( scu > 1 ) scale =  scu;
		if( scd > 1 ) scale = -scd;
		PIXEL_FORMAT pix = PIX_BYTE;
		if( NULL != pObj )
		{
			pix = pObj->npix;
		}
		else
		{
			// TODO: check what to do in this case, assume 8 bit???
		}
		switch( pix )
		{
		case PIX_BYTE:
			pnt_CopyX<BYTE>(p, bw, x0, y0, lpScrBuf, xw, yw, scale, pObj, sizeof(BYTE));
			break;
		case PIX_CHAR:
			pnt_CopyX<CHAR>((LPSTR)p, bw, x0, y0, lpScrBuf, xw, yw, scale, pObj, sizeof(CHAR));
			break;
		case PIX_WORD:
			pnt_CopyX<WORD>((LPWORD)p, w, x0, y0, lpScrBuf, xw, yw, scale, pObj, sizeof(WORD));
			break;
		case PIX_SHORT:
			pnt_CopyX<SHORT>((SHORT*)p, w, x0, y0, lpScrBuf, xw, yw, scale, pObj, sizeof(SHORT));
			break;
		case PIX_DWORD:
			pnt_CopyX<DWORD>((LPDWORD)p, w, x0, y0, lpScrBuf, xw, yw, scale, pObj, sizeof(DWORD));
			break;
		case PIX_LONG:
			pnt_CopyX<LONG>((LPLONG)p, w, x0, y0, lpScrBuf, xw, yw, scale, pObj, sizeof(LONG));
			break;
		case PIX_FLOAT:
			pnt_CopyX<FLOAT>((LPFLOAT)p, w, x0, y0, lpScrBuf, xw, yw, scale, pObj, sizeof(FLOAT));
			break;
		case PIX_DOUBLE:
			pnt_CopyX<DOUBLE>((LPDOUBLE)p, w, x0, y0, lpScrBuf, xw, yw, scale, pObj, sizeof(DOUBLE));
			break;
		default:
			::OutputDebugString("pntBitmapToScreen() -> unsupported pixel format.\n");
			return;
		}
//		if (b16) pnt_Copy16( (LPWORD)p, bw, x0, y0, lpScrBuf, xw, yw, scale, pObj );
//		else     pnt_Copy  (         p, bw, x0, y0, lpScrBuf, xw, yw, scale, pObj );
		sw = R8(xw);
		bi.biWidth  = sw;
		bi.biHeight = -yw;
		nWidth = xw;
		nHeight = yw;
		SrcX = SrcY = 0;
		nNumScans = yw;
		lpBits = lpScrBuf;
	}
	if (transparent)
	{
		DrawTransparentBitmap(hDC, (LPBITMAPINFO)&bi, lpBits,
			SrcX, nNumScans-yw-SrcY, PALETTEINDEX(GRAYSIZ+4*USERSIZ), hMyPal);
	}
	else
	{
		SetDIBitsToDevice(hDC, uu, uu, nWidth, nHeight, SrcX, SrcY, 0, nNumScans, lpBits,
			(LPBITMAPINFO)&bi, DIB_PAL_COLORS);
	}
	if( hPalette ) SelectPalette(hDC, hPalette, FALSE);
	if( (flags & 0x1000) && (NULL != pObj) )
	{
		MapWindowPoints( hWnd, pObj->hWnd, (LPPOINT) &rw, 2);
		ValidateRect( pObj->hWnd, &rw );
	}
	else if( 0 == (flags & 0x8000) )
		ValidateRect( hWnd, &rw );
	if( rw.right - rw.left > w * dScale/*scu/scd*/ )
	{
		fr.left = w * dScale/*scu/scd*/;
		FillRect(hDC, &fr, hbr);
	}
	if( rw.bottom - rw.top > h * dScale/*scu/scd*/ )
	{
		fr.bottom = h * dScale/*scu/scd*/;
		FillRect(hDC, &fr, hbr);
	}
	::DeleteObject(hbr);

	EndPaint( hWnd, &ps );
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("pntBitmapToScreen() -> %s"), e.what()));
	}
}

/****************************************************************************/
void pntCreateObjMenu( HWND hDlg )
{
HMENU	hm;
POINT	p;
	ReplyMessage( 0 );
	hm = LoadMenu(hInst, "MENUZOOM");
	GetCursorPos(&p);
	TrackPopupMenu( GetSubMenu(hm,0), TPM_LEFTALIGN, p.x, p.y, 0, hDlg, NULL );
	DestroyMenu( hm );
}
/****************************************************************************/
		char	szPalName[MAXPAL][10] ={ "Palette 1", "Palette 2", "Palette 3", "Palette 4" };
static	char	szPalNameSav[MAXPAL][10];

static	char	bMrkFlag[MAXPAL][3][USERSIZ]={0};
static	char	bMrkFlagSav[MAXPAL][3][USERSIZ];

static	char	szSign[]="CRISP_PALETTE";

static	BYTE	brightSav, contrastSav;						// Copies for Cancel
static	int		palnoSav;

#define	MRS	2				// Marker size on plot
static	int		mrkx[USERSIZ], mrky[USERSIZ];
static	int		iIndex = -1;
static	int		iPalNum;
static	int		iColNum=0;

static	PALETTEENTRY	pal1y[RESISIZ] =
{
// TriMerge Palette
	{ 128,   0, 128,   0},{  64,   0, 168,   0},{   0,   0, 204,   0},{   0,   0, 255,   0},
	{   0, 128, 192,   0},{   0, 128, 128,   0},{   0, 192, 128,   0},{   0, 255,   0,   0},
	{   0, 128,   0,   0},{ 255, 128,   0,   0},{ 192,  96,   0,   0},{ 128,  64,   0,   0},
	{ 128,   0,   0,   0},{ 171,   0,   0,   0},{ 213,   0,   0,   0},{ 255,   0,   0,   0}
};


static	SCROLLBAR sbBRI0 = { 0, 128, 0, 255, 1, 16, 0};	// Bright scroll bar defaults
static	SCROLLBAR sbCON0 = { 0, 128, 0, 255, 1, 16, 0};	// Contr. scroll bar defaults
static	SCROLLBAR sbBRI, sbCON;							// Bright and Contrast ScrBars

/****************************************************************************/

static	void	DrawColorItem( LPDRAWITEMSTRUCT lpD )
{
RECT	rc;
WORD	is;
static	myBITMAPINFO	bi= { 40, 0, 0, 1, 8, BI_RGB, 0, 0, 0, 0, 0 };
HPALETTE	hopal;
int		i, wid, xs, ys, pnum;

	if ( lpD->CtlID != IDC_CO_PALLIST ) return;
	if ( (lpD->itemAction & (ODA_DRAWENTIRE | ODA_SELECT) )==0) return;
	is  = lpD->itemState;
	pnum   = (int)lpD->itemData;
	CopyRect( &rc, &lpD->rcItem);
	wid = xs = rc.right-rc.left; ys = rc.bottom-rc.top;
	xs = (xs + 3) & 0xFFFC;

	bi.biBitCount = 8;
	bi.biClrUsed  = COLORS;
	bi.biClrImportant = COLORS;
	bi.biSizeImage = xs * ys;
	bi.biWidth  = xs; bi.biHeight = ys;

    for( i=0; i<wid; i++ ) *(lpScrBuf+i) = (BYTE)((i*255)/wid);
    for( i=1; i<ys; i++ ) memcpy(lpScrBuf+i*xs, lpScrBuf, wid );

	for(i=0; i<256; i++) bi.index[i] = (BYTE)((i*USERSIZ)/256) + USERSIZ*pnum+GRAYSIZ;

	hopal = SelectPalette(lpD->hDC, hMyPal, FALSE);
	RealizePalette(lpD->hDC);

	SetDIBitsToDevice(lpD->hDC, rc.left, rc.top, wid, ys, 0, 0, 0, ys, lpScrBuf,
	 				  (LPBITMAPINFO)&bi, DIB_PAL_COLORS);
	if (hopal) SelectPalette( lpD->hDC, hopal, FALSE);
}
/****************************************************************************/
static	void	ToggleCustomGray( HWND hDlg )
{
BOOL f = IsDlgButtonChecked( hDlg, IDC_CO_CUSTOM );

	EnableWindow( GetDlgItem( hDlg, IDC_CO_PALLIST), f);
	EnableWindow( GetDlgItem( hDlg, IDC_CO_PLOT),    f);
	EnableWindow( GetDlgItem( hDlg, IDC_CO_PALNAME), f);
	EnableWindow( GetDlgItem( hDlg, IDC_CO_EDIT),    f);
	EnableWindow( GetDlgItem( hDlg, IDC_CO_R),       f);
	EnableWindow( GetDlgItem( hDlg, IDC_CO_G),       f);
	EnableWindow( GetDlgItem( hDlg, IDC_CO_B),       f);
	if (!f) CheckDlgButton( hDlg, IDC_CO_EDIT, FALSE );
	wndChangeDlgSize( hDlg, hDlg, IsDlgButtonChecked(hDlg,IDC_CO_EDIT) ? 0:IDC_CO_PLOT, 0, "Colors", hInst);
	UpdateWindow(hDlg);
}
/****************************************************************************/
static	void	BlinkActiveMarker( HWND hDlg )
{
}
/****************************************************************************/
static	void	PaintPlot( HWND hDlg )
{
HWND 	hw	 = GetDlgItem( hDlg, IDC_CO_PLOT);
HDC 	shdc, hdc;
HBITMAP hbmp;
int 	i, xs, ys;
int		xsl, ysl;
RECT 	r;
int		x, y;
HFONT	hof;
LPBYTE	lpRGBF = (LPBYTE)LogPal.palRGBF;
HBRUSH	hwb = (HBRUSH)GetStockObject( WHITE_BRUSH ),
		hnb = (HBRUSH)GetStockObject( NULL_BRUSH ),
		hob;

	if ( ! IsDlgButtonChecked(hDlg, IDC_CO_EDIT) ) return;
	if ( iPalNum < 0) return;

	shdc = GetDC( hw );
	hdc = CreateCompatibleDC( shdc );

	lpRGBF += GRAYSIZ*4+USERSIZ*4*iPalNum;
	GetWindowRect( hw, &r );
	xs = r.right - r.left;
	ys = r.bottom - r.top;
    xsl = xs - 2 - MRS*2; ysl = ys - 2 - MRS*2;
	hbmp = CreateCompatibleBitmap( shdc, xs, ys);
	SelectObject( hdc, hbmp );
	PatBlt( hdc, 0, 0, xs, ys, BLACKNESS);

	// Grid
	SelectObject( hdc, grey_pen );
	SetTextColor( hdc, RGB(128,128,128));
	SetBkMode( hdc, TRANSPARENT );
	hof = SelectFont( hdc, hfInfo );
	_MoveTo( hdc, 0, MRS+1 );                LineTo( hdc, xs, MRS+1);
	_MoveTo( hdc, 0, MRS+1+(int)(ysl+1)/2 ); LineTo( hdc, xs, MRS+1+(int)(ysl+1)/2);
	_MoveTo( hdc, 0, MRS+1+(int)ysl );       LineTo( hdc, xs, MRS+1+(int)ysl);
	for(i=0;i<=8;i++) {
		x = i*xsl/7 + MRS+1;
		_MoveTo( hdc, x, 0 ); LineTo( hdc, x, ys );
		sprintf( TMP, "%-3d", i*32);
		TextOut(hdc, x+1, ys-MRS-wFntH, TMP, strlen(TMP));
	}
	SelectFont( hdc, hof );

	// Red chanel
	SelectObject(hdc, red_pen);
	for(i=0; i<USERSIZ; i++) {
		x = (i*xsl/(USERSIZ-1))+MRS+1;
		y = (ysl-(*(lpRGBF+i*4))*(ysl-1)/255)+MRS+1;
		if (i==0) _MoveTo( hdc, x, y);
		else      LineTo( hdc, x, y);
		if (iColNum == 0) { mrkx[i]=x; mrky[i]=y; }
	}

	// Green chanel
	SelectObject(hdc, green_pen);
	for(i=0; i<USERSIZ; i++) {
		x = (i*xsl/(USERSIZ-1))+MRS+1;
		y = (ysl-(*(lpRGBF+i*4+1))*(ysl-1)/255)+MRS+1;
		if (i==0) _MoveTo( hdc, x, y);
		else      LineTo( hdc, x, y);
		if (iColNum == 1) {	mrkx[i]=x; mrky[i]=y;}
	}

	// Blue chanel
	SelectObject(hdc, blue_pen);
	for(i=0; i<USERSIZ; i++) {
		x = (i*xsl/(USERSIZ-1))+MRS+1;
		y = (ysl-(*(lpRGBF+i*4+2))*(ysl-1)/255)+MRS+1;
		if (i==0) _MoveTo( hdc, x, y);
		else      LineTo( hdc, x, y);
		if (iColNum == 2) {	mrkx[i]=x; mrky[i]=y;}
	}
	// Markers

	switch( iColNum ) {
		case 0:	SelectObject( hdc, red_pen );   break;
		case 1:	SelectObject( hdc, green_pen ); break;
		case 2:	SelectObject( hdc, blue_pen );  break;
	}
	hob = SelectBrush( hdc, hwb );
	for(i=0;i<USERSIZ;i++) {
		if ( bMrkFlag[iPalNum][iColNum][i]) SelectBrush( hdc, hwb );	// White interior
		else								SelectBrush( hdc, hnb );	// Transparent interior
		Rectangle( hdc, mrkx[i]-MRS, mrky[i]-MRS, mrkx[i]+MRS+1, mrky[i]+MRS+1);
	}

	if (hob) SelectBrush( hdc, hob );
	BitBlt( shdc, 1, 1, xs-2, ys-2, hdc, 1, 1, SRCCOPY);
	DeleteDC( hdc );
	DeleteObject( hbmp );
	ReleaseDC( hw, shdc );
	InflateRect( &r, -1, -1);
	MapWindowPoints( NULL, hDlg, (LPPOINT)&r, 2);
	ValidateRect( hDlg, &r);
}
/****************************************************************************/
static WORD ChangeItemColors( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
HDC		hdc = (HDC)wParam;
HWND	hwnd = (HWND)lParam;
int		idc = GetDlgCtrlID(hwnd);
LRESULT	ret;
HPEN	pt = (HPEN)GetStockObject(BLACK_PEN);
COLORREF	col = RGB(0,0,0);
static	BOOL brec=FALSE;

	if (brec) return FALSE;
	brec = TRUE;
	ret = DefDlgProc(hDlg, msg, wParam, lParam);
	switch ( idc ) {
		case IDC_CO_R:
			pt = RedPen;   col = RGB(255,0,0);	break;
		case IDC_CO_G:
			pt = GreenPen; col = RGB(0,255,0);	break;
		case IDC_CO_B:
			pt = BluePen;  col = RGB(0,0,255);	break;
		default: goto xxx;
	}
	if(!IsWindowEnabled(hwnd)) {
		if ( msg == WM_CTLCOLORSTATIC ) goto xxx;
		pt = GrayPen;	col = RGB(192,192,192);
	}
	SelectObject(hdc, pt);
	SetTextColor(hdc, col);
xxx:
	brec = FALSE;
	return	ret;
}

/****************************************************************************/
#define	cmpchg( x )	((x < 0) ? 0 : ((x > 255) ? 255 : x))
/*==========================================================================*/
static double	ys[USERSIZ], bs[USERSIZ], cs[USERSIZ], ds[USERSIZ];
/*--------------------------------------------------------------------------*/
static void	spline( int k )
{
register int i, n;
double       t;

	cs[1] = ys[0] / ds[0];
    if(k <= 1)
    {
    	bs[0] = bs[1] = cs[1]; cs[0] = cs[1] = ds[0] = ds[1] = 0.0;
    	return;
    }
/*---------------------*/
	n = k; k--;
	for(i=1; i<n; ++i)
	{
		bs[i]   = 2.0 * (ds[i-1] + ds[i]);
		cs[i+1] = ys[i] / ds[i];
		cs[i]   =  cs[i+1] - cs[i];
	}
	bs[0] = -ds[0]; bs[n] = -ds[k]; cs[0] = cs[n] = 0.0;
	for(i=1; i<n; ++i)
	{
		t = ds[i-1] / bs[i-1];
		bs[i] -= t * ds[i-1];
		cs[i] -= t * cs[i-1];
	}
	for(i=k; i>=0; --i)
	{
		cs[i] = (cs[i] - ds[i] * cs[i+1]) / bs[i];
	}
	for(i=0; i<n; ++i)
	{
		bs[i] = ys[i] / ds[i] - ds[i] * (cs[i+1] + 2.0 * cs[i]);
		ds[i] = (cs[i+1] - cs[i]) / ds[i];
		cs[i] *= 3.0;
	}
}
/*--------------------------------------------------------------------------*/
static	void	UpdatePal( HWND hDlg )
{
register int i, j;
double	     bb, cc, dd, v;
int			 dx, x, y;
LPBYTE	lpRGBF = (LPBYTE)LogPal.palRGBF;

#define	data(i)	(*(lpRGBF+i*4+iColNum))

	lpRGBF += GRAYSIZ*4+USERSIZ*4*iPalNum;

	bMrkFlag[iPalNum][iColNum][0] = bMrkFlag[iPalNum][iColNum][USERSIZ-1] = 1;

	for(j=0, i=1, x=0, y=data(0); i<USERSIZ; ++i)
		if ( bMrkFlag[iPalNum][iColNum][i] )
			{ ds[j] = (double)(i - x); ys[j] = (double)(data(i) - y); ++j; x = i; y = data(i); }
	spline(j);


	for(j=i=0; i<USERSIZ; ++i)
	{
		if( bMrkFlag[iPalNum][iColNum][i] )
			{ x = i; y = data(i); bb = bs[j]; dd = ds[j]; cc = cs[j]; ++j; }
		else
		{
	  		dx = i - x; v = (double)(dx * (bb + dx * cc)); dx = y + (int)(v+.5);
	  		data(i) = (BYTE)cmpchg(dx);
	  	}
	  	cc += dd;
	}
	ChangePalette();
}
/****************************************************************************/
static	BOOL	TestClickPlot( HWND hDlg, LPARAM lParam, BOOL set )
{
int		i, x, y;
POINT	p = {LOWORD(lParam),HIWORD(lParam)}, p1;
RECT	r;
HWND	hw = GetDlgItem( hDlg, IDC_CO_PLOT);
int		ys;

	MapWindowPoints( hDlg, hw, &p, 1);
	x = p.x; y = p.y;
	for( i=0; i<USERSIZ; i++)
		if ( (mrkx[i]-3 < x) && (mrkx[i]+3 > x) && (mrky[i]-3 < y) && (mrky[i]+3 > y) ) break;

	if (i == USERSIZ) return FALSE;
	if (set)  bMrkFlag[iPalNum][iColNum][i] = 1;
	else	 { bMrkFlag[iPalNum][iColNum][i] = 0; goto ex; }

	// Save index
	iIndex = i;

    p.x = mrkx[i]; p.y = mrky[i];
    ClientToScreen(hw, &p);
    SetCursorPos( p.x, p.y );

	GetWindowRect( hw, &r );
	ys = r.bottom - r.top;

	p.x = p1.x = mrkx[i];
	p.y = MRS; p1.y = ys-MRS;
	ClientToScreen(hw, &p); ClientToScreen(hw, &p1);
	if (p.x>p1.x) { x=p.x; p.x=p1.x; p1.x=x; }
	if (p.y>p1.y) { y=p.y; p.y=p1.y; p1.y=y; }
	SetRect( &r, p.x, p.y, p1.x+1, p1.y+1);
	ClipCursor( &r );

ex:
	UpdatePal( hDlg );
	PaintPlot( hDlg );
	return (iBPP>8);
}
/****************************************************************************/
static BOOL	DragMarker( HWND hDlg, LPARAM lParam)
{
POINT	p = {LOWORD(lParam), HIWORD(lParam)};
HWND	hw = GetDlgItem( hDlg, IDC_CO_PLOT);
RECT	r;
int		ys;
int		ysl;
LPBYTE	lpRGBF = (LPBYTE)LogPal.palRGBF;

	if ( iPalNum < 0) return FALSE;

	lpRGBF += GRAYSIZ*4+USERSIZ*4*iPalNum;

	MapWindowPoints( hDlg, hw, &p, 1);
	GetWindowRect( hw, &r );
	ys = r.bottom - r.top;
    ysl = ys - 2 - MRS*2;

	p.y = ys-p.y-MRS;
	ys = (p.y * 255 / ysl);
	if (ys<0) ys = 0; if(ys>255) ys = 255;

	*(lpRGBF+iIndex*4+iColNum) = (BYTE)ys;

	UpdatePal( hDlg );
	PaintPlot( hDlg );
	return (iBPP>8);
}
/****************************************************************************/
static	BOOL	SavePalette( LPSTR fname )
{
FILE	*out;
	if ((out = fopen( fname, "wb")) == NULL) myError(IDS_ERRCREATEFILE);
	if (   ( fwrite( szSign, 1, sizeof(szSign), out) != sizeof(szSign) )
		|| ( fwrite( szPalName, 1, sizeof(szPalName), out) != sizeof(szPalName) )
		|| ( fwrite( bMrkFlag, 1, sizeof(bMrkFlag), out) != sizeof(bMrkFlag) )
		|| ( fwrite( LogPal.palRGBF, 1, sizeof(LogPal.palRGBF), out) != sizeof(LogPal.palRGBF) ))
		{ fclose(out); return FALSE; myError(IDS_ERRWRITEFILE); }
	fclose(out);

	return TRUE;
}
/****************************************************************************/
static	BOOL	LoadPalette( LPSTR fname, BOOL f )
{
FILE	*in;

	if ((in = fopen( fname, "rb")) == NULL) {
		if (f) return myError(IDS_ERROPENFILE);
		else return FALSE;
	}
	if (   ( fread( TMP, 1, sizeof(szSign), in) != sizeof(szSign) )
		|| ( strcmp(TMP, szSign) != 0)
		|| ( fread( szPalName, 1, sizeof(szPalName), in) != sizeof(szPalName) )
		|| ( fread( bMrkFlag, 1, sizeof(bMrkFlag), in) != sizeof(bMrkFlag) )
		|| ( fread( LogPal.palRGBF, 1, sizeof(LogPal.palRGBF), in) != sizeof(LogPal.palRGBF) ))
		{
			fclose(in);
			if (f) return myError(IDS_ERRREADFILE);
			else return FALSE;
		}
	fclose(in);
	return TRUE;
}
/****************************************************************************/
BOOL CALLBACK ColorsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND ParentWnd = (HWND)GetWindowLong( hDlg, DWL_USER );
	OBJ*	O = NULL;
	BOOL	redr = FALSE;

	if (ParentWnd) O = OBJ::GetOBJ(ParentWnd);

	switch (message) {
		case WM_INITCOLORS:
		case WM_INITDIALOG:
		{
			int	i;
			if (hColors==NULL) {
				wndChangeDlgSize( hDlg, hDlg, IsDlgButtonChecked(hDlg,IDC_CO_EDIT) ? 0:IDC_CO_PLOT, 0, "Colors", hInst);
				for(i=0; i<MAXPAL; i++)
					SendDlgItemMessage(hDlg, IDC_CO_PALLIST, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)i);
				SendDlgItemMessage(hDlg, IDC_CO_PALLIST, CB_SETCURSEL, 0, 0);
				SetTimer( hDlg, 0, 500, NULL );
				regReadWindowPos( hDlg, "Colors");
			}
			if (lParam!=0 && (O = OBJ::GetOBJ((HWND)lParam))!=NULL &&
				(O->ID==OT_IMG || O->ID==OT_FFT || O->ID==OT_DMAP || O->ID==OT_IFFT) ) {
				SetWindowLong( hDlg, DWL_USER, lParam);
				sprintf(TMP,"Visualization of <%s>",O->title);
				SetWindowText(hDlg,TMP);
				sbBRI   = sbBRI0;		sbCON   = sbCON0;
				brightSav   = sbBRI.p = O->bright;
				contrastSav = sbCON.p = O->contrast;
				iPalNum = palnoSav = O->palno;
				if (iPalNum<0) iPalNum = 0;
				memcpy( &LogPalSav,   &LogPal,   sizeof(LogPal));
				memcpy( szPalNameSav, szPalName, sizeof(szPalName));
				memcpy( bMrkFlagSav,  bMrkFlag,  sizeof(bMrkFlag));

				InitScrollCtl( hDlg, IDC_CO_BRIGHTNESS,  &sbBRI);
				InitScrollCtl( hDlg, IDC_CO_CONTRAST,    &sbCON);
				CheckDlgButton( hDlg, IDC_CO_GRAY,   (O->palno<0)  );
				CheckDlgButton( hDlg, IDC_CO_CUSTOM, (O->palno>=0) );
				CheckDlgButton( hDlg, IDC_CO_MAXIMIZEC, O->maxcontr );
				CheckDlgButton( hDlg, IDC_CO_R,      (iColNum == 0) );
				CheckDlgButton( hDlg, IDC_CO_G,      (iColNum == 1) );
				CheckDlgButton( hDlg, IDC_CO_B,      (iColNum == 2) );
// 				SetDlgItemInt( hDlg, IDC_CO_FROM, O->imin, FALSE );
// 				SetDlgItemInt( hDlg, IDC_CO_TO,   O->imax, FALSE );
				if( O->imin != (int)O->imin )
				{
					SetDlgItemText( hDlg, IDC_CO_FROM, _FMT(_T("%.2f"), O->imin).c_str() );
				}
				else
				{
					SetDlgItemText( hDlg, IDC_CO_FROM, _FMT(_T("%g"), O->imin).c_str() );
				}
				if( O->imax != (int)O->imax )
				{
					SetDlgItemText( hDlg, IDC_CO_TO,   _FMT(_T("%.2f"), O->imax).c_str() );
				}
				else
				{
					SetDlgItemText( hDlg, IDC_CO_TO,   _FMT(_T("%g"), O->imax).c_str() );
				}
				SendDlgItemMessage(hDlg, IDC_CO_PALLIST, CB_SETCURSEL, iPalNum, 0);
				SetDlgItemText( hDlg, IDC_CO_PALNAME, szPalName[iPalNum]);
				EnableWindow( GetDlgItem(hDlg,IDC_CO_FROM), O->maxcontr );
				EnableWindow( GetDlgItem(hDlg,IDC_CO_TO),   O->maxcontr );
				ToggleCustomGray( hDlg );
			} else {
				SetWindowLong( hDlg, DWL_USER, 0);
				SetWindowText(hDlg,"Visualization");
				wndChangeDlgSize( hDlg, hDlg, IDC_CO_PLOT, 0, "Colors", hInst);
			}

			if (wParam) SetFocus((HWND)wParam);
			return FALSE;
		}

		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORSCROLLBAR:
			return ChangeItemColors( hDlg, message, wParam, lParam );

		case WM_HSCROLL:
			if (ParentWnd==NULL) break;
			switch (GetDlgCtrlID ( (HWND)lParam )) {
				case IDC_CO_BRIGHTNESS:	UpdateScrollCtl( hDlg, wParam,lParam, &sbBRI); break;
				case IDC_CO_CONTRAST:	UpdateScrollCtl( hDlg, wParam,lParam, &sbCON); break;
			}
			if (O) {
				O->bright   = sbBRI.p;
				O->contrast = sbCON.p;
			}
			redr = TRUE;
			break;

		case WM_PAINT:
			PaintPlot( hDlg );
			break;

		case WM_TIMER:
			BlinkActiveMarker( hDlg );
			return TRUE;

		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT) lParam;
			if (mis->CtlID != IDC_CO_PALLIST) break;
			mis->itemHeight = 15;
		}
			return TRUE;

		case WM_DRAWITEM:
			DrawColorItem( (LPDRAWITEMSTRUCT) lParam );
			break;

		case WM_RBUTTONDOWN:
			redr = TestClickPlot( hDlg, lParam, FALSE );
			break;
		case WM_LBUTTONDOWN:
			redr = TestClickPlot( hDlg, lParam, TRUE );
			break;
		case WM_MOUSEMOVE:
			if (((wParam & MK_LBUTTON) == 0 ) || iIndex<0) break;
			redr = DragMarker( hDlg, lParam );
			break;
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
			ClipCursor( NULL );
			iIndex = -1;
			if (iBPP>8) InvalidateRect( GetDlgItem( hDlg, IDC_CO_PALLIST), NULL, FALSE);
			break;

		case WM_INITMENUPOPUP:
			if (HIWORD(lParam)==0)
				CheckMenuItem((HMENU)wParam, SC_myZOOM | (O->scu<<8) | (O->scd<<4), MF_BYCOMMAND | MF_CHECKED);
			break;

		case WM_NCDESTROY:
			regWriteWindowPos( hDlg, "Colors", FALSE);
			hColors = NULL;
			if (IsWindow(MainhWnd)) InitializeMenu(MainhWnd);
			break;

		case WM_COMMAND:
			if (LOWORD(wParam)==IDCANCEL || LOWORD(wParam)==IDOK) {
				KillTimer( hDlg, 0 );
				DestroyWindow(hDlg);
				hColors = NULL;
				break;
			}

			if (ParentWnd==0) break;
			iPalNum = (int)SendDlgItemMessage( hDlg, IDC_CO_PALLIST, CB_GETCURSEL, 0, 0);

			switch (LOWORD(wParam)) {
				case IDC_CO_PALLIST:
					if ( HIWORD(wParam) == CBN_SELCHANGE) {
						O->palno = iPalNum;
						SetDlgItemText( hDlg, IDC_CO_PALNAME, szPalName[iPalNum]);
						PaintPlot( hDlg );
						redr = TRUE;
    				}
    				break;
				case IDC_CO_PALNAME:
					if ((HIWORD(wParam) == EN_CHANGE) && iPalNum>=0)
						GetDlgItemText(hDlg, IDC_CO_PALNAME, szPalName[iPalNum], 10);
					break;

				case IDC_CO_FROM:
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
// 						BOOL f; int v;
// 						v = GetDlgItemInt(hDlg, IDC_CO_FROM, &f, FALSE);
						double v;
						if( GetDlgItemDouble(hDlg, IDC_CO_FROM, &v, FALSE) )
						{
							O->imin = v;
							redr = TRUE;
						}
						//SetDlgItemInt( hDlg, IDC_CO_FROM, O->imin, FALSE );
						if( O->imin != (int)O->imin )
						{
							SetDlgItemText( hDlg, IDC_CO_FROM, _FMT(_T("%.2f"), O->imin).c_str() );
						}
						else
						{
							SetDlgItemText( hDlg, IDC_CO_FROM, _FMT(_T("%g"), O->imin).c_str() );
						}
					}
					break;

				case IDC_CO_TO:
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
// 						BOOL f; int v;
// 						v = GetDlgItemInt(hDlg, IDC_CO_TO, &f, FALSE);
						double v;
						if( GetDlgItemDouble(hDlg, IDC_CO_TO, &v, FALSE) )
						{
							O->imax = v;
							redr = TRUE;
						}
						//SetDlgItemInt( hDlg, IDC_CO_TO, O->imax, FALSE );
						if( O->imax != (int)O->imax )
						{
							SetDlgItemText( hDlg, IDC_CO_TO, _FMT(_T("%.2f"), O->imax).c_str() );
						}
						else
						{
							SetDlgItemText( hDlg, IDC_CO_TO, _FMT(_T("%g"), O->imax).c_str() );
						}
					}
					break;

				case IDC_CO_MAXIMIZEC:
					O->maxcontr = IsDlgButtonChecked( hDlg, IDC_CO_MAXIMIZEC);
					EnableWindow( GetDlgItem(hDlg,IDC_CO_FROM), O->maxcontr );
					EnableWindow( GetDlgItem(hDlg,IDC_CO_TO),   O->maxcontr );
					redr = TRUE;
					break;

				case IDC_CO_R:	iColNum = 0; goto xx;
				case IDC_CO_G:	iColNum = 1; goto xx;
				case IDC_CO_B:	iColNum = 2;
xx:					PaintPlot( hDlg );
					break;

				case IDC_CO_EDIT:
					wndChangeDlgSize( hDlg, hDlg,
									  IsDlgButtonChecked(hDlg,IDC_CO_EDIT) ? 0:IDC_CO_PLOT, 0, "Colors", hInst);
					break;

				case IDC_CO_GDEFAULT:
					O->bright   = sbBRI.p = sbBRI.def;
					O->contrast = sbCON.p = sbCON.def;
					InitScrollCtl( hDlg, IDC_CO_BRIGHTNESS,  &sbBRI);
					InitScrollCtl( hDlg, IDC_CO_CONTRAST,    &sbCON);
					imgCalcMinMax( O );
// 					SetDlgItemInt( hDlg, IDC_CO_FROM, O->imin, FALSE );
// 					SetDlgItemInt( hDlg, IDC_CO_TO,   O->imax, FALSE );
					if( O->imin != (int)O->imin )
					{
						SetDlgItemText( hDlg, IDC_CO_FROM, _FMT(_T("%.2f"), O->imin).c_str() );
					}
					else
					{
						SetDlgItemText( hDlg, IDC_CO_FROM, _FMT(_T("%g"), O->imin).c_str() );
					}
					if( O->imax != (int)O->imax )
					{
						SetDlgItemText( hDlg, IDC_CO_TO,   _FMT(_T("%.2f"), O->imax).c_str() );
					}
					else
					{
						SetDlgItemText( hDlg, IDC_CO_TO,   _FMT(_T("%g"), O->imax).c_str() );
					}
					redr = TRUE;
					break;

				case IDC_CO_GRAY:
					if (IsDlgButtonChecked(hDlg,IDC_CO_GRAY) && (O->palno >= 0))
					{
						O->palno = -1;
						ToggleCustomGray( hDlg );
						redr = TRUE;
					}
					break;

				case IDC_CO_CUSTOM:
					if (IsDlgButtonChecked(hDlg,IDC_CO_CUSTOM) && (O->palno < 0)) {
						O->palno = iPalNum;
						ToggleCustomGray( hDlg );
						redr = TRUE;
					}
					break;

				case IDC_CO_CSAVE:
					if (!fioGetFileNameDialog( hDlg, "Save Palette File", &ofNames[OFN_PALETTE], "", TRUE))
						return FALSE;
					SavePalette( fioFileName() );
					break;
				case IDC_CO_CLOAD:
					if (!fioGetFileNameDialog( hDlg, "Load Palette File", &ofNames[OFN_PALETTE], "", FALSE))
						return FALSE;
					redr = LoadPalette( fioFileName(), TRUE );
					SetDlgItemText( hDlg, IDC_CO_PALNAME, szPalName[iPalNum]);
					UpdatePal( hDlg );
					PaintPlot( hDlg );
					break;

				case IDC_CO_HELP:
					WinHelp(hDlg, "crispd.hlp", HELP_KEY, (DWORD)(LPSTR)"Colors");
					break;

				case IDC_CO_RESTORE:
					O->bright   = brightSav;
					O->contrast = contrastSav;
					O->palno = palnoSav;
					memcpy( &LogPal,   &LogPalSav,   sizeof(LogPal));
					memcpy( szPalName, szPalNameSav, sizeof(szPalName));
					memcpy( bMrkFlag,  bMrkFlagSav,  sizeof(bMrkFlag));
					UpdatePal( hDlg );
					if (O->hbit) { DeleteObject( O->hbit ); O->hbit = NULL; }
					InvalidateRect( ParentWnd, NULL, FALSE );
					break;
			}
			break;
	}
	if (redr && ParentWnd) {
		if (O->hbit) { DeleteObject( O->hbit ); O->hbit = NULL; }
		InvalidateRect( ParentWnd, NULL, FALSE );
	}
	return (FALSE);
}
/****************************************************************************/
BOOL pntInit( void )
{
int		i, j;
BYTE	flg = PC_RESERVED;

	if ((lpScrBuf = (LPBYTE)calloc(1, GetSystemMetrics(SM_CXSCREEN)*GetSystemMetrics(SM_CYSCREEN))) == NULL)
		return FALSE;

	strcpy( TMP, ofNames[OFN_PALETTE].szDefDir );
	if (strlen(TMP)) strcat( TMP, "\\" );
	strcat( TMP, "default.pal" );
	if ( ! LoadPalette(TMP, FALSE) ) {

		i=0;
		for(j=0; j<GRAYSIZ; i++, j++ ) {
			LogPal.palRGBF[i*4]   = (char)((255*j) / GRAYSIZ);
			LogPal.palRGBF[i*4+1] = (char)((255*j) / GRAYSIZ);
			LogPal.palRGBF[i*4+2] = (char)((255*j) / GRAYSIZ);
			LogPal.palRGBF[i*4+3] = flg;
	    }
	    for(j=0; j<USERSIZ; i++, j++ ) {
			LogPal.palRGBF[i*4]   = (char)((255*j) / USERSIZ);
			LogPal.palRGBF[i*4+1] = (char)((128*j) / USERSIZ);
			LogPal.palRGBF[i*4+2] = (char)((255*j) / USERSIZ);
			LogPal.palRGBF[i*4+3] = flg;
	    }
	    for(j=0; j<USERSIZ; i++, j++ ) {
			LogPal.palRGBF[i*4]   = (char)((190*j) / USERSIZ);
			LogPal.palRGBF[i*4+1] = (char)((255*j) / USERSIZ);
			LogPal.palRGBF[i*4+2] = (char)((100*j) / USERSIZ);
			LogPal.palRGBF[i*4+3] = flg;
	    }
	    for(j=0; j<USERSIZ; i++, j++ ) {
			LogPal.palRGBF[i*4]   = (char)((100*j) / USERSIZ);
			LogPal.palRGBF[i*4+1] = (char)((255*j) / USERSIZ);
			LogPal.palRGBF[i*4+2] = (char)((190*j) / USERSIZ);
			LogPal.palRGBF[i*4+3] = flg;
	    }
	    for(j=0; j<USERSIZ; i++, j++ ) {
			LogPal.palRGBF[i*4]   = (char)(( 90*j) / USERSIZ);
			LogPal.palRGBF[i*4+1] = (char)((255*j) / USERSIZ);
			LogPal.palRGBF[i*4+2] = (char)((140*j) / USERSIZ);
			LogPal.palRGBF[i*4+3] = flg;
		}
		for(j=0; j<RESISIZ; i++, j++) {
			LogPal.palRGBF[i*4]     = pal1y[j].peRed;
			LogPal.palRGBF[i*4+1]   = pal1y[j].peGreen;
			LogPal.palRGBF[i*4+2]   = pal1y[j].peBlue;
			LogPal.palRGBF[i*4+3]   = 0;
	    }
		bMrkFlag[0][0][0] = bMrkFlag[0][0][USERSIZ-1] = 1;
		bMrkFlag[1][0][0] = bMrkFlag[1][0][USERSIZ-1] = 1;
		bMrkFlag[2][0][0] = bMrkFlag[2][0][USERSIZ-1] = 1;
		bMrkFlag[3][0][0] = bMrkFlag[3][0][USERSIZ-1] = 1;

		bMrkFlag[0][1][0] = bMrkFlag[0][1][USERSIZ-1] = 1;
		bMrkFlag[1][1][0] = bMrkFlag[1][1][USERSIZ-1] = 1;
		bMrkFlag[2][1][0] = bMrkFlag[2][1][USERSIZ-1] = 1;
		bMrkFlag[3][1][0] = bMrkFlag[3][1][USERSIZ-1] = 1;

		bMrkFlag[0][2][0] = bMrkFlag[0][2][USERSIZ-1] = 1;
		bMrkFlag[1][2][0] = bMrkFlag[1][2][USERSIZ-1] = 1;
		bMrkFlag[2][2][0] = bMrkFlag[2][2][USERSIZ-1] = 1;
		bMrkFlag[3][2][0] = bMrkFlag[3][2][USERSIZ-1] = 1;
	}


	i = GRAYSIZ+MAXPAL*USERSIZ + RESISIZ;
	LogPal.palRGBF[i*4]     = 255;
	LogPal.palRGBF[i*4+1]   = 0;
	LogPal.palRGBF[i*4+2]   = 0;
	LogPal.palRGBF[i*4+3]   = 0;

	hMyPal = CreatePalette( (LPLOGPALETTE) &LogPal );

	red_pen		= CreatePen(PS_SOLID, 1, RGB(255,  0,  0));
	green_pen	= CreatePen(PS_SOLID, 1, RGB(  0,255,  0));
	blue_pen	= CreatePen(PS_SOLID, 1, RGB(  0,  0,255));
	grey_pen	= CreatePen(PS_SOLID, 1, RGB(128,128,128));
	yellow_pen	= CreatePen(PS_SOLID, 1, RGB(255,255,  0));
	magenta_pen	= CreatePen(PS_SOLID, 1, RGB(255,0,255));

	return TRUE;
}
/****************************************************************************/
VOID pntClose( void )
{
	if (lpScrBuf) free( lpScrBuf );
	if (hMyPal) DeleteObject( hMyPal );

	if (red_pen) DeleteObject( red_pen );
	if (green_pen) DeleteObject( green_pen );
	if (blue_pen) DeleteObject( blue_pen );
	if (grey_pen) DeleteObject( grey_pen );
	if (yellow_pen) DeleteObject( yellow_pen );
	if (magenta_pen) DeleteObject( magenta_pen );
}
/****************************************************************************/
LPBYTE WINAPI pntCreateXLAT( LPBYTE buf )
{
PALETTEENTRY pe[256];
int			 ii[256], i, ic, in[256];
int			 r, g, b, l, y;
HDC			hDC = GetDC( NULL );

#define CDIF	4
	GetSystemPaletteEntries( hDC, 0, 256, pe );
	memset(ii, 0, 256*2);
	for (i=0;i<256;i++) {
		r = pe[i].peRed; g = pe[i].peGreen; b = pe[i].peBlue;
		if ((abs(r - g) < CDIF) && (abs(b - g) < CDIF) && (abs(r - b) < CDIF)) {
			y = (r+g+b)/3;
			ii[y] = i | 0x100;
		}
	}
	ic=0;
	for(i=0;i<256;i++) if(ii[i]&0x100) { in[ic] = i; ii[ic] = ii[i]&0xFF; ic++; }
	memset(buf, ii[0], in[1]/2);
	for(i=1;i<ic-1;i++) {
		l = (in[i]-in[i-1])/2+in[i-1];
		r = (in[i+1]-in[i])/2+in[i];
		memset(&buf[l], ii[i], r-l);
	}
	l = (in[i]-in[i-1])/2+in[i-1];
	r = 256;
	memset(&buf[l], ii[ic-1], r-l);

	ReleaseDC( NULL, hDC );
	return buf;
}
/****************************************************************************/
__declspec(dllexport) HPALETTE pntGetPal( VOID )
{
	return hMyPal;
}
/****************************************************************************/
/*
__declspec(dllexport) void pnt_Copy(LPBYTE src, int bw, int x0, int y0, LPBYTE dst,
									int xw, int yw, int scale, OBJ* O)
{
	int		i,j, xc, yc;
	int		sw = R8(xw);
	BYTE	b;
	WORD	w;
	DWORD	dw, dw0;
	double	dt;

	xw = R8(xw);
	src+=bw*y0+x0;

	if (O && O->maxcontr)
	{
		dt = 255./(O->imax-O->imin);
		for(i=0;i<O->imin;i++) wxlt[i] = 0;
		for(i=O->imin;i<=O->imax;i++)
		{
			wxlt[i]=(BYTE)((i-O->imin)*dt);
		}
		for(i=O->imax;i<256;i++) wxlt[i] = 255;
		if( scale < 0)
		{
			scale = -scale;
			for(i=0;i<yw;i++) {
				for(j=0;j<xw;j++) {
					*dst = wxlt[*src];
					src+=scale; dst++;
				}
				src+=(bw-xw)*scale; dst+=sw-xw;
			}
		}
		else switch( scale )
		{
			case 1:
				for(i=0;i<yw;i++)
				{
					for(j=0;j<xw;j++)
					{
						*dst = wxlt[*src];
						src++; dst++;
					}
					src+=bw-xw; dst+=sw-xw;
				}
				break;
			case 2:
				xc = xw/2; yc = R2(yw)/2;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8);
						*(LPWORD)(dst) = w;
						*(LPWORD)(dst+sw) = w;
						src++; dst+=2;
					}
					dst+=sw*2-xc*2; src+=bw-xc;
				}
				break;
			case 3:
				xc = xw/3; yc = yw/3;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8);
						*(LPWORD)(dst) = w;
						*(dst+2)  = b;
						*(LPWORD)(dst+sw) = w;
						*(dst+sw+2) = b;
						*(LPWORD)(dst+sw*2) = w;
						*(dst+sw*2+2) = b;
						src++; dst+=3;
					}
					dst+=sw*3-xc*3;	src+=bw-xc;
				}
				break;
			case 4:
				xc = xw/4; yc = R4(yw)/4;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
						*(LPDWORD)(dst) = dw;
						*(LPDWORD)(dst+sw) = dw;
						*(LPDWORD)(dst+sw*2) = dw;
						*(LPDWORD)(dst+sw*3) = dw;
						src++; dst+=4;
					}
					dst+=sw*4-xc*4; src+=bw-xc;
				}
				break;
			case 8:
				xc = xw/8; yc = R8(yw)/8;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
						dw0 = dw & 0xFFFFFF00;
						*(LPDWORD)(dst)      =   0; *(LPDWORD)(dst+4)      = 0;
						*(LPDWORD)(dst+sw)   = dw0; *(LPDWORD)(dst+sw+4)   = dw;
						*(LPDWORD)(dst+sw*2) = dw0; *(LPDWORD)(dst+sw*2+4) = dw;
						*(LPDWORD)(dst+sw*3) = dw0; *(LPDWORD)(dst+sw*3+4) = dw;
						*(LPDWORD)(dst+sw*4) = dw0; *(LPDWORD)(dst+sw*4+4) = dw;
						*(LPDWORD)(dst+sw*5) = dw0; *(LPDWORD)(dst+sw*5+4) = dw;
						*(LPDWORD)(dst+sw*6) = dw0; *(LPDWORD)(dst+sw*6+4) = dw;
						*(LPDWORD)(dst+sw*7) = dw0; *(LPDWORD)(dst+sw*7+4) = dw;
						src++; dst+=8;
					}
					dst+=sw*8-xc*8; src+=bw-xc;
				}
				break;
		}
	} else {
		if (scale<0) {
			scale = -scale;
			for(i=0;i<yw;i++) {
				for(j=0;j<xw;j++) {
					*dst = *src;
					src+=scale; dst++;
				}
				src+=(bw-xw)*scale; dst+=sw-xw;
			}
		} else switch( scale ) {
		case 1:
			for(i=0;i<yw;i++) {
				memcpy( dst, src, xw );
				src+=bw; dst+=sw;
			}
			break;
		case 2:
			xc = xw/2; yc = R2(yw)/2;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = *src; w = b | (b<<8);
					*(LPWORD)(dst) = w;
					*(LPWORD)(dst+sw) = w;
					src++; dst+=2;
				}
				dst+=sw*2-xc*2; src+=bw-xc;
			}
			break;
		case 3:
			xc = xw/3; yc = yw/3;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = *src; w = b | (b<<8);
					*(LPWORD)(dst) = w;
					*(dst+2)  = b;
					*(LPWORD)(dst+sw) = w;
					*(dst+sw+2) = b;
					*(LPWORD)(dst+sw*2) = w;
					*(dst+sw*2+2) = b;
					src++; dst+=3;
				}
				dst+=sw*3-xc*3;	src+=bw-xc;
			}
			break;
		case 4:
			xc = xw/4; yc = R4(yw)/4;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = *src; w = b | (b<<8); dw = w | (w<<16);
					*(LPDWORD)(dst) = dw;
					*(LPDWORD)(dst+sw) = dw;
					*(LPDWORD)(dst+sw*2) = dw;
					*(LPDWORD)(dst+sw*3) = dw;
					src++; dst+=4;
				}
				dst+=sw*4-xc*4; src+=bw-xc;
			}
			break;
		case 8:
			xc = xw/8; yc = R8(yw)/8;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = *src; w = b | (b<<8); dw = w | (w<<16);
					dw0 = dw & 0xFFFFFF00;
					*(LPDWORD)(dst)      =   0; *(LPDWORD)(dst+4)      = 0;
					*(LPDWORD)(dst+sw)   = dw0; *(LPDWORD)(dst+sw+4)   = dw;
					*(LPDWORD)(dst+sw*2) = dw0; *(LPDWORD)(dst+sw*2+4) = dw;
					*(LPDWORD)(dst+sw*3) = dw0; *(LPDWORD)(dst+sw*3+4) = dw;
					*(LPDWORD)(dst+sw*4) = dw0; *(LPDWORD)(dst+sw*4+4) = dw;
					*(LPDWORD)(dst+sw*5) = dw0; *(LPDWORD)(dst+sw*5+4) = dw;
					*(LPDWORD)(dst+sw*6) = dw0; *(LPDWORD)(dst+sw*6+4) = dw;
					*(LPDWORD)(dst+sw*7) = dw0; *(LPDWORD)(dst+sw*7+4) = dw;
					src++; dst+=8;
				}
				dst+=sw*8-xc*8; src+=bw-xc;
			}
			break;
		}
	}
}
/****************************************************************************/
/*
__declspec(dllexport) void pnt_Copy16(LPWORD src, int bw, int x0, int y0, LPBYTE dst,
									  int xw, int yw, int scale, OBJ* O )
{
int		i,j, xc, yc;
int		sw = R8(xw);
BYTE	b;
WORD	w;
DWORD	dw, dw0;
double	dt;

	xw = R8(xw);
	src+=bw*y0+x0;

	if (O && O->maxcontr) {
		dt = 255./(O->imax-O->imin);
		for(i=0;i<O->imin;i++) wxlt[i] = 0;
		for(i=O->imin;i<=O->imax;i++) {
			wxlt[i]=(BYTE)((i-O->imin)*dt);
		}
		for(i=O->imax;i<65536;i++) wxlt[i] = 255;
		if (scale<0) {
			scale = -scale;
			for(i=0;i<yw;i++) {
				for(j=0;j<xw;j++) {
					*dst = wxlt[*src];
					src+=scale; dst++;
				}
				src+=(bw-xw)*scale; dst+=sw-xw;
			}
		} else switch( scale ) {
			case 1:
				for(i=0;i<yw;i++) {
					for(j=0;j<xw;j++) {
						*dst = wxlt[*src];
						src++; dst++;
					}
					src+=bw-xw; dst+=sw-xw;
				}
				break;
			case 2:
				xc = xw/2; yc = R2(yw)/2;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8);
						*(LPWORD)(dst) = w;
						*(LPWORD)(dst+sw) = w;
						src++; dst+=2;
					}
					dst+=sw*2-xc*2; src+=bw-xc;
				}
				break;
			case 3:
				xc = xw/3; yc = yw/3;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8);
						*(LPWORD)(dst) = w;
						*(dst+2)  = b;
						*(LPWORD)(dst+sw) = w;
						*(dst+sw+2) = b;
						*(LPWORD)(dst+sw*2) = w;
						*(dst+sw*2+2) = b;
						src++; dst+=3;
					}
					dst+=sw*3-xc*3;	src+=bw-xc;
				}
				break;
			case 4:
				xc = xw/4; yc = R4(yw)/4;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
						*(LPDWORD)(dst) = dw;
						*(LPDWORD)(dst+sw) = dw;
						*(LPDWORD)(dst+sw*2) = dw;
						*(LPDWORD)(dst+sw*3) = dw;
						src++; dst+=4;
					}
					dst+=sw*4-xc*4; src+=bw-xc;
				}
				break;
			case 8:
				xc = xw/8; yc = R8(yw)/8;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
						dw0 = dw & 0xFFFFFF00;
						*(LPDWORD)(dst)      =   0; *(LPDWORD)(dst+4)      = 0;
						*(LPDWORD)(dst+sw)   = dw0; *(LPDWORD)(dst+sw+4)   = dw;
						*(LPDWORD)(dst+sw*2) = dw0; *(LPDWORD)(dst+sw*2+4) = dw;
						*(LPDWORD)(dst+sw*3) = dw0; *(LPDWORD)(dst+sw*3+4) = dw;
						*(LPDWORD)(dst+sw*4) = dw0; *(LPDWORD)(dst+sw*4+4) = dw;
						*(LPDWORD)(dst+sw*5) = dw0; *(LPDWORD)(dst+sw*5+4) = dw;
						*(LPDWORD)(dst+sw*6) = dw0; *(LPDWORD)(dst+sw*6+4) = dw;
						*(LPDWORD)(dst+sw*7) = dw0; *(LPDWORD)(dst+sw*7+4) = dw;
						src++; dst+=8;
					}
					dst+=sw*8-xc*8; src+=bw-xc;
				}
				break;
		}
	} else {
		dt = 255./65536.;
		for(i=0;i<65536;i++) {
			wxlt[i]=(BYTE)(i*dt);
		}
		if (scale<0) {
			scale = -scale;
			for(i=0;i<yw;i++) {
				for(j=0;j<xw;j++) {
					*dst = wxlt[*src];
					src+=scale; dst++;
				}
				src+=(bw-xw)*scale; dst+=sw-xw;
			}
		} else switch( scale ) {
		case 1:
			for(i=0;i<yw;i++) {
				for(j=0;j<xw;j++) dst[j]=wxlt[src[j]];
				src+=bw; dst+=sw;
			}
			break;
		case 2:
			xc = xw/2; yc = R2(yw)/2;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = wxlt[*src]; w = b | (b<<8);
					*(LPWORD)(dst) = w;
					*(LPWORD)(dst+sw) = w;
					src++; dst+=2;
				}
				dst+=sw*2-xc*2; src+=bw-xc;
			}
			break;
		case 3:
			xc = xw/3; yc = yw/3;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = wxlt[*src]; w = b | (b<<8);
					*(LPWORD)(dst) = w;
					*(dst+2)  = b;
					*(LPWORD)(dst+sw) = w;
					*(dst+sw+2) = b;
					*(LPWORD)(dst+sw*2) = w;
					*(dst+sw*2+2) = b;
					src++; dst+=3;
				}
				dst+=sw*3-xc*3;	src+=bw-xc;
			}
			break;
		case 4:
			xc = xw/4; yc = R4(yw)/4;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
					*(LPDWORD)(dst) = dw;
					*(LPDWORD)(dst+sw) = dw;
					*(LPDWORD)(dst+sw*2) = dw;
					*(LPDWORD)(dst+sw*3) = dw;
					src++; dst+=4;
				}
				dst+=sw*4-xc*4; src+=bw-xc;
			}
			break;
		case 8:
			xc = xw/8; yc = R8(yw)/8;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
					dw0 = dw & 0xFFFFFF00;
					*(LPDWORD)(dst)      =   0; *(LPDWORD)(dst+4)      = 0;
					*(LPDWORD)(dst+sw)   = dw0; *(LPDWORD)(dst+sw+4)   = dw;
					*(LPDWORD)(dst+sw*2) = dw0; *(LPDWORD)(dst+sw*2+4) = dw;
					*(LPDWORD)(dst+sw*3) = dw0; *(LPDWORD)(dst+sw*3+4) = dw;
					*(LPDWORD)(dst+sw*4) = dw0; *(LPDWORD)(dst+sw*4+4) = dw;
					*(LPDWORD)(dst+sw*5) = dw0; *(LPDWORD)(dst+sw*5+4) = dw;
					*(LPDWORD)(dst+sw*6) = dw0; *(LPDWORD)(dst+sw*6+4) = dw;
					*(LPDWORD)(dst+sw*7) = dw0; *(LPDWORD)(dst+sw*7+4) = dw;
					src++; dst+=8;
				}
				dst+=sw*8-xc*8; src+=bw-xc;
			}
			break;
		}
	}
}
*/
/****************************************************************************/
/*
__declspec(dllexport) void pnt_Copy32(LPDWORD src, int bw, int x0, int y0, LPBYTE dst,
									  int xw, int yw, int scale, OBJ* O)
{
	int		i,j, xc, yc;
	int		sw = R8(xw);
	BYTE	b;
	WORD	w;
	DWORD	dw, dw0;
	double	dt;

	xw = R8(xw);
	src+=bw*y0+x0;

	if (O && O->maxcontr)
	{
		dt = 255.0 / (O->imax - O->imin);
		for(i=0;i<O->imin;i++) wxlt[i] = 0;
		for(i=O->imin;i<=O->imax;i++)
		{
			wxlt[i]=(BYTE)((i-O->imin)*dt);
		}
		for(i=O->imax;i<65536;i++) wxlt[i] = 255;
		if( scale < 0)
		{
			scale = -scale;
			for(i=0;i<yw;i++)
			{
				for(j=0;j<xw;j++)
				{
					*dst = wxlt[*src];
					src+=scale; dst++;
				}
				src+=(bw-xw)*scale; dst+=sw-xw;
			}
		}
		else switch( scale )
		{
			case 1:
				for(i=0;i<yw;i++)
				{
					for(j=0;j<xw;j++)
					{
						*dst = wxlt[*src];
						src++; dst++;
					}
					src+=bw-xw; dst+=sw-xw;
				}
				break;
			case 2:
				xc = xw/2; yc = R2(yw)/2;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8);
						*(LPWORD)(dst) = w;
						*(LPWORD)(dst+sw) = w;
						src++; dst+=2;
					}
					dst+=sw*2-xc*2; src+=bw-xc;
				}
				break;
			case 3:
				xc = xw/3; yc = yw/3;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8);
						*(LPWORD)(dst) = w;
						*(dst+2)  = b;
						*(LPWORD)(dst+sw) = w;
						*(dst+sw+2) = b;
						*(LPWORD)(dst+sw*2) = w;
						*(dst+sw*2+2) = b;
						src++; dst+=3;
					}
					dst+=sw*3-xc*3;	src+=bw-xc;
				}
				break;
			case 4:
				xc = xw/4; yc = R4(yw)/4;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
						*(LPDWORD)(dst) = dw;
						*(LPDWORD)(dst+sw) = dw;
						*(LPDWORD)(dst+sw*2) = dw;
						*(LPDWORD)(dst+sw*3) = dw;
						src++; dst+=4;
					}
					dst+=sw*4-xc*4; src+=bw-xc;
				}
				break;
			case 8:
				xc = xw/8; yc = R8(yw)/8;
				for(i=0;i<yc;i++) {
					for(j=0;j<xc;j++) {
						b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
						dw0 = dw & 0xFFFFFF00;
						*(LPDWORD)(dst)      =   0; *(LPDWORD)(dst+4)      = 0;
						*(LPDWORD)(dst+sw)   = dw0; *(LPDWORD)(dst+sw+4)   = dw;
						*(LPDWORD)(dst+sw*2) = dw0; *(LPDWORD)(dst+sw*2+4) = dw;
						*(LPDWORD)(dst+sw*3) = dw0; *(LPDWORD)(dst+sw*3+4) = dw;
						*(LPDWORD)(dst+sw*4) = dw0; *(LPDWORD)(dst+sw*4+4) = dw;
						*(LPDWORD)(dst+sw*5) = dw0; *(LPDWORD)(dst+sw*5+4) = dw;
						*(LPDWORD)(dst+sw*6) = dw0; *(LPDWORD)(dst+sw*6+4) = dw;
						*(LPDWORD)(dst+sw*7) = dw0; *(LPDWORD)(dst+sw*7+4) = dw;
						src++; dst+=8;
					}
					dst+=sw*8-xc*8; src+=bw-xc;
				}
				break;
		}
	}
	else
	{
		dt = 255./65536.;
		for(i=0;i<65536;i++)
		{
			wxlt[i]=(BYTE)(i*dt);
		}
		if (scale<0)
		{
			scale = -scale;
			for(i=0;i<yw;i++) {
				for(j=0;j<xw;j++) {
					*dst = wxlt[*src];
					src+=scale; dst++;
				}
				src+=(bw-xw)*scale; dst+=sw-xw;
			}
		}
		else switch( scale )
		{
		case 1:
			for(i=0;i<yw;i++) {
				for(j=0;j<xw;j++) dst[j]=wxlt[src[j]];;
				src+=bw; dst+=sw;
			}
			break;
		case 2:
			xc = xw/2; yc = R2(yw)/2;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = wxlt[*src]; w = b | (b<<8);
					*(LPWORD)(dst) = w;
					*(LPWORD)(dst+sw) = w;
					src++; dst+=2;
				}
				dst+=sw*2-xc*2; src+=bw-xc;
			}
			break;
		case 3:
			xc = xw/3; yc = yw/3;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = wxlt[*src]; w = b | (b<<8);
					*(LPWORD)(dst) = w;
					*(dst+2)  = b;
					*(LPWORD)(dst+sw) = w;
					*(dst+sw+2) = b;
					*(LPWORD)(dst+sw*2) = w;
					*(dst+sw*2+2) = b;
					src++; dst+=3;
				}
				dst+=sw*3-xc*3;	src+=bw-xc;
			}
			break;
		case 4:
			xc = xw/4; yc = R4(yw)/4;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
					*(LPDWORD)(dst) = dw;
					*(LPDWORD)(dst+sw) = dw;
					*(LPDWORD)(dst+sw*2) = dw;
					*(LPDWORD)(dst+sw*3) = dw;
					src++; dst+=4;
				}
				dst+=sw*4-xc*4; src+=bw-xc;
			}
			break;
		case 8:
			xc = xw/8; yc = R8(yw)/8;
			for(i=0;i<yc;i++) {
				for(j=0;j<xc;j++) {
					b = wxlt[*src]; w = b | (b<<8); dw = w | (w<<16);
					dw0 = dw & 0xFFFFFF00;
					*(LPDWORD)(dst)      =   0; *(LPDWORD)(dst+4)      = 0;
					*(LPDWORD)(dst+sw)   = dw0; *(LPDWORD)(dst+sw+4)   = dw;
					*(LPDWORD)(dst+sw*2) = dw0; *(LPDWORD)(dst+sw*2+4) = dw;
					*(LPDWORD)(dst+sw*3) = dw0; *(LPDWORD)(dst+sw*3+4) = dw;
					*(LPDWORD)(dst+sw*4) = dw0; *(LPDWORD)(dst+sw*4+4) = dw;
					*(LPDWORD)(dst+sw*5) = dw0; *(LPDWORD)(dst+sw*5+4) = dw;
					*(LPDWORD)(dst+sw*6) = dw0; *(LPDWORD)(dst+sw*6+4) = dw;
					*(LPDWORD)(dst+sw*7) = dw0; *(LPDWORD)(dst+sw*7+4) = dw;
					src++; dst+=8;
				}
				dst+=sw*8-xc*8; src+=bw-xc;
			}
			break;
		}
	}
}
*/
