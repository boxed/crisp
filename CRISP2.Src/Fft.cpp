/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2.FFT * by ML *******************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "objects.h"
#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "rgconfig.h"
#include "fft.h"
#include "ft32util.h"

#include "HelperFn.h"

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

/****************************************************************************/
/* FFT window ***************************************************************/
/****************************************************************************/
static SCROLLBAR	SB0_Par={  0,  0,   0, 256, 1, 16, 0}; 	/* Parabolic control		*/
static SCROLLBAR	SB0_Lin={ 20, 20,   0, 256, 1, 16, 0}; 	/* Linear control			*/
static SCROLLBAR	SB0_Sc ={ 30, 30,   0, 128, 1, 10, 0};	/* Scale control			*/
static SCROLLBAR	SB0_DC ={  0,  0,-256, 256, 1, 16, 0};	/* Zero shift control		*/
static char	szFFT[] = "FFT";
static char	szDefaults[] = "Defaults";
//BOOL bOldFFT = TRUE;
/****************************************************************************/
void	Fc2Sc( LPDOUBLE x, LPDOUBLE y, OBJ* O)
{
	LPFFT	S = GetFFT(O);
//	int	h = (LOWORD(lParam)) * O->scd / O->scu + O->xo - S->fs/2;
//	int	k = (HIWORD(lParam)) * O->scd / O->scu + O->yo - S->fs/2;

	*x = (*x - O->xo + S->fs*.5)*(double)O->scu/(double)O->scd + (O->scu/2);
	*y = (*y - O->yo + S->fs*.5)*(double)O->scu/(double)O->scd + (O->scu/2);
}
/****************************************************************************/
static void	SaveDef( OBJ* O )
{
	LPFFT S = (LPFFT)&O->dummy;

	S->SB_DC.def = S->SB_DC.p;
	S->SB_Sc.def = S->SB_Sc.p;
	S->SB_Lin.def = S->SB_Lin.p;
	S->SB_Par.def = S->SB_Par.p;
	sprintf(TMP,"%d,%d,%d,%d", S->SB_DC.p, S->SB_Sc.p, S->SB_Lin.p, S->SB_Par.p );
	regSetData(szFFT, szDefaults, TMP, 0);

}
/****************************************************************************/
static void	LoadDef( void )
{
	int	a,b,c,d;

	sprintf(TMP,"%d,%d,%d,%d", SB0_DC.def, SB0_Sc.def, SB0_Lin.def, SB0_Par.def );
	if (regGetData(szFFT, szDefaults, TMP, sizeof(TMP)) &&
		sscanf(TMP,"%d,%d,%d,%d", &a,&b,&c,&d) == 4) {
		SB0_DC.def=a; SB0_Sc.def=b; SB0_Lin.def=c; SB0_Par.def=d;
		SB0_DC.p=a; SB0_Sc.p=b; SB0_Lin.p=c; SB0_Par.p=d;
	}
}
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
DWORD	FFT_InitMask( LPWORD lpw, UINT wWidth, UINT wHeight, UINT wLeft, UINT wTop, UINT wDim, BOOL bCircMask )
{
	WORD	sw, mw;
	WORD	dy;
	WORD	x0, x1;
	WORD	i;
	double	r, r2, y, h;
	double	ye, yof, e;
	DWORD	dwAreaOfs;
	BOOL	bMaskFit=FALSE;

	dy = wHeight - wTop;
	x0 = wLeft;
	x1 = __min(wWidth,wLeft+wDim);

	dwAreaOfs = wWidth*wTop+wLeft;
	if (bCircMask) {
		sw = 0;
		r = wDim/2;
		r2 = r*r;
		if (bMaskFit && wHeight<wDim) { // May be dy<wDim ?
			e = (double)wDim/(double)wHeight;
			yof = r*(e-1.);
		} else {
			e = 1.;
			yof = 0.;
		}
		for(y=-r+.5,i=0;y<r;y++,i++) {
			ye = y*e+yof;
			h = r2-ye*ye;
			if (h<0) h=0; else h = sqrt(h);
			x0 = (int)(r-h)+wLeft;
			x1 = (int)(r+h)+wLeft;
			x0 = __min(x0, wWidth);
			x1 = __min(x1, wWidth);
			if (i>=dy) x1=x0;
			mw = x1-x0;
			sw += x0-wLeft;
			*(lpw++) = sw;
			*(lpw++) = mw;
			sw = wDim-(x1-wLeft);
		}
		*(lpw++) = sw;
		*(lpw++) = (WORD)-1;
	} else {
		sw = 0;
		for(i=0;i<wDim;i++) {
			if (i>=dy) x1=x0;
			mw = x1-x0;
			sw += x0-wLeft;
			*(lpw++) = sw;
			*(lpw++) = mw;
			sw = wDim-(x1-wLeft);
		}
		*(lpw++) = sw;
		*(lpw++) = (WORD)-1;
	}
	return dwAreaOfs;
}
/****************************************************************************/
BOOL __stdcall fnGetImageData( UINT y, UINT cnt, LPVOID lpDst, LPARAM lParam )
{
	LPFFT	S	=(LPFFT)lParam;
	LPBYTE	dstb =(LPBYTE)lpDst, srcb=(LPBYTE)S->src;
	LPWORD	dstw =(LPWORD)lpDst, srcw=(LPWORD)S->src;

	switch(S->npix)
	{
	case PIX_BYTE:
		memcpy( lpDst, S->src+S->fs*y, cnt);
//		memcpy( S->img + y*S->fs, S->src+S->fs*y, cnt);
		break;
	case PIX_CHAR:
		memcpy( lpDst, S->src+S->fs*y, cnt);
//		memcpy( S->img + y*S->fs, S->src+S->fs*y, cnt);
		break;
	case PIX_WORD:
		memcpy( lpDst, S->src+S->fs*y*2, cnt*2);
//		for(int i=0;i<cnt;i++)
//			*(S->img + y*S->fs+i) = (BYTE)(srcw[S->fs*y+i]-30000);
		break;
	case PIX_SHORT:
		memcpy( lpDst, S->src+S->fs*y*2, cnt*2);
//		for(int i=0;i<cnt;i++)
//			*(S->img + y*S->fs+i) = (BYTE)(srcw[S->fs*y+i]-30000);
		break;
	case PIX_DWORD:
		memcpy( lpDst, S->src+S->fs*y*4, cnt*4);
		break;
	case PIX_LONG:
		memcpy( lpDst, S->src+S->fs*y*4, cnt*4);
		break;
	case PIX_FLOAT:
		memcpy( lpDst, S->src+S->fs*y*4, cnt*4);
		break;
	case PIX_DOUBLE:
		memcpy( lpDst, S->src+S->fs*y*8, cnt*8);
		break;
	}
	return TRUE;
}
/***************************************************************/
BOOL __stdcall fnSetAMAPData( UINT y, UINT cnt, LPVOID lpSrc, LPARAM lParam )
{
	LPFFT S = (LPFFT)lParam;

	memcpy( S->img + y*S->fs, lpSrc, cnt);
	return TRUE;
}
/****************************************************************************/
template <typename _Tx>
double imGetFrameDC_t(_Tx *src, int xoff, int yoff, int _stride, int area_w, LPWORD mask)
{
	typedef _Tx type;
	typedef type* pointer;
	pointer ptr = pointer(((LPBYTE)src) + yoff * _stride) + xoff;
	WORD off, cnt;
	int x, dstoff = 0;
	double sum = 0.0;
	DWORD count = 0;

	if (ptr == NULL)
	{
		throw std::exception("failed to run imGetFrameDC: no image data");
	}

	for(;;)
	{
		if( 0xFFFF == (cnt = mask[1]) )
		{
			break;
		}
		off = mask[0];
		dstoff += off;
		if( dstoff >= area_w )
		{
			dstoff -= area_w;
			ptr = pointer(((LPBYTE)ptr) + _stride);
		}
// 		memcpy(dst, ptr + dstoff, cnt * sizeof(type));
		for(x = 0; x < cnt; x++)
		{
			sum += ptr[dstoff + x];
		}
		count += cnt;
		mask += 2;
		dstoff += cnt;
	}
	return (count > 0) ? sum / count : 0.0;
}

template <typename _Tx>
void imMaskCopy_t(_Tx *src, int xoff, int yoff, int _stride, int width, int height, LPFFT pFft, LPWORD mask, double dSum)
{
	typedef _Tx type;
	typedef type* pointer;
	pointer ptr = pointer(((LPBYTE)src) + yoff * _stride) + xoff;
	pointer dst = pointer(pFft->src);
	WORD off, cnt;
	int dstoff = 0;
	int _y = yoff;

	if (ptr == NULL)
	{
		throw std::exception("failed to run imGetFrameDC: no image data");
	}

	std::fill_n(dst, pFft->fs*pFft->fs, (type)dSum);
	for(;; _y++)
	{
		if( _y >= height )
		{
			break;
		}
		if( 0xFFFF == (cnt = mask[1]) )
		{
			break;
		}
		off = mask[0];
		dstoff += off;
		if( dstoff >= pFft->fs )
		{
			dstoff -= pFft->fs;
			ptr = pointer(((LPBYTE)ptr) + _stride);
		}
		dst += off;
		memcpy(dst, ptr + dstoff, cnt * sizeof(type));
		mask += 2;
		dst += cnt;
		dstoff += cnt;
	}

	BM_templ<_Tx> bmsrc(pFft->fs, pFft->fs, (pointer)pFft->src);
	pFft->ft2.Load(bmsrc);
	bmsrc.P = NULL;
}
/****************************************************************************/
VOID __cdecl DoUpdateFFT( LPVOID param )
{
	OBJ*	O = (OBJ*)param;
	HWND	hWnd = O->hWnd;
	LPFFT	S = (LPFFT)&O->dummy;
	OBJ*	pr = OBJ::GetOBJ( O->ParentWnd );		// The parent should be AREA type
	if (pr->ID != OT_AREA)
	{
		MessageBox(NULL, "parent window is not an area", "Error", MB_OK);
		return;
	}
//	WORD	wImageDC;
	DWORD	dwAreaOfs;

	S->src = O->edp;		// Input data
	S->img = O->dp;			// bitmap data
	S->fs  = O->ui;			// Dimensions
//	S->pix = IMAGE::PixFmt2Bpp(pr->npix);		// Pixel size
	S->npix = pr->npix;

	O->NI  = pr->NI;		// Project info
	O->npix = PIX_BYTE;

	BM8	bmdst(S->fs,S->fs,S->img);

	tobSetLED( ID_LIVELED, TRUE );
	try
	{
		while (O->bDoItAgain)
		{
			O->bDoItAgain = FALSE;
#ifdef USE_XB_LENGTH
			dwAreaOfs = FFT_InitMask( (LPWORD)O->ebp, pr->xb, pr->y, pr->xa, pr->ya, S->fs, pr->dummy[0] );
#else
			// TODO: check
			dwAreaOfs = FFT_InitMask( (LPWORD)O->ebp, pr->x, pr->y, pr->xa, pr->ya, S->fs, pr->dummy[0] );
#endif
			if (!bGrabberActive && O->bDoItAgain)
				continue;
			
#ifdef USE_XB_LENGTH
			int width = pr->xb;
#else
			int width = pr->stride;
#endif
			
			double dSum;
			// TODO: PIX_CHAR
			switch( S->npix )
			{
			case PIX_BYTE:
				dSum = imGetFrameDC_t((LPBYTE)pr->dp, pr->xa, pr->ya, pr->stride, S->fs, (LPWORD)O->ebp);
				imMaskCopy_t((LPBYTE)pr->dp, pr->xa, pr->ya, pr->stride, pr->x, pr->y, S, (LPWORD)O->ebp, dSum);
				break;
				
			case PIX_CHAR:
				dSum = imGetFrameDC_t((LPSTR)pr->dp, pr->xa, pr->ya, pr->stride, S->fs, (LPWORD)O->ebp);
				imMaskCopy_t((LPSTR)pr->dp, pr->xa, pr->ya, pr->stride, pr->x, pr->y, S, (LPWORD)O->ebp, dSum);
				break;
				
			case PIX_WORD:
				dSum = imGetFrameDC_t((LPWORD)pr->dp, pr->xa, pr->ya, pr->stride, S->fs, (LPWORD)O->ebp);
				imMaskCopy_t((LPWORD)pr->dp, pr->xa, pr->ya, pr->stride, pr->x, pr->y, S, (LPWORD)O->ebp, dSum);
				break;
				
			case PIX_SHORT:
				dSum = imGetFrameDC_t((SHORT*)pr->dp, pr->xa, pr->ya, pr->stride, S->fs, (LPWORD)O->ebp);
				imMaskCopy_t((SHORT*)pr->dp, pr->xa, pr->ya, pr->stride, pr->x, pr->y, S, (LPWORD)O->ebp, dSum);
				break;
				
			case PIX_DWORD:
				dSum = imGetFrameDC_t((LPDWORD)pr->dp, pr->xa, pr->ya, pr->stride, S->fs, (LPWORD)O->ebp);
				imMaskCopy_t((LPDWORD)pr->dp, pr->xa, pr->ya, pr->stride, pr->x, pr->y, S, (LPWORD)O->ebp, dSum);
				break;
				
			case PIX_LONG:
				dSum = imGetFrameDC_t((LPLONG)pr->dp, pr->xa, pr->ya, pr->stride, S->fs, (LPWORD)O->ebp);
				imMaskCopy_t((LPLONG)pr->dp, pr->xa, pr->ya, pr->stride, pr->x, pr->y, S, (LPWORD)O->ebp, dSum);
				break;
				
			case PIX_FLOAT:
				dSum = imGetFrameDC_t((LPFLOAT)pr->dp, pr->xa, pr->ya, pr->stride, S->fs, (LPWORD)O->ebp);
				imMaskCopy_t((LPFLOAT)pr->dp, pr->xa, pr->ya, pr->stride, pr->x, pr->y, S, (LPWORD)O->ebp, dSum);
				break;
				
			case PIX_DOUBLE:
				dSum = imGetFrameDC_t((LPDOUBLE)pr->dp, pr->xa, pr->ya, pr->stride, S->fs, (LPWORD)O->ebp);
				imMaskCopy_t((LPDOUBLE)pr->dp, pr->xa, pr->ya, pr->stride, pr->x, pr->y, S, (LPWORD)O->ebp, dSum);
				break;
			}
			if (!bGrabberActive && O->bDoItAgain) continue;
			S->ft2.FFT();
			if (!bGrabberActive && O->bDoItAgain) continue;
			S->ft2.iDC     = S->SB_DC.p;
			S->ft2.iScale  = S->SB_Sc.p;
			S->ft2.iLinear = S->SB_Lin.p;
			S->ft2.iParab  = S->SB_Par.p;
			S->ft2.fDoMaxVal = TRUE;
			S->ft2.DisplayFFT(bmdst);
			InvalidateRect( hWnd, NULL, 0);
			UpdateWindow( hWnd );
		}
		bmdst.P = NULL;
		imgCalcMinMax( O );
		tobSetLED( ID_LIVELED, FALSE );
	}
	catch(...)
	{
		Trace(_T("DoUpdateFFT -> unhandled exception.\n"));
	}

	O->hCalcTask = NULL;
	SendNotifyMessage( hWnd, FM_Update2, 0,0);
	InvalidateRect( hWnd, NULL, 0);
	UpdateWindow( hWnd );
}
/****************************************************************************/
BOOL	UpdateFFT( HWND hWnd )
{
	OBJ*	O = OBJ::GetOBJ(hWnd);
	LPFFT	S = (LPFFT)&O->dummy;

	if ( O->hCalcTask != 0 )
	{
		O->bDoItAgain = TRUE;
	}
	else
	{
		O->bDoItAgain = TRUE;
		O->hCalcTask = (HANDLE)_beginthread( DoUpdateFFT, 0, (LPVOID)O );
	}
	return TRUE;
}
/****************************************************************************/
BOOL CALLBACK FFTDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND ParentWnd = (HWND)GetWindowLong( hDlg, DWL_USER );
	OBJ*	O;
	LPFFT S = NULL;
	if (ParentWnd)
	{
		O = OBJ::GetOBJ(ParentWnd);
		S = (LPFFT)&O->dummy;
	}

//	return FALSE;

	switch (message)
	{
		case WM_INITDIALOG:
			// ADDED BY PETER ON 29 JUNE 2004
			// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
			// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
//			O = (OBJ*)lParam;
//			S = (LPEDRD) &O->dummy;
			O = OBJ::GetOBJ((HWND)lParam);
			S = (LPFFT)&O->dummy;
			// REMOVED BY PETER ON 06 JULY 2005
			// because this leads to un-updatable FFT window
	//		InitializeObjectDialog(hDlg, O);

//			O = OBJ::GetOBJ((HWND)lParam);
//			S = (LPFFT)&O->dummy;
//			SetWindowLong( hDlg, GWL_EXSTYLE, GetWindowLong( hDlg, GWL_EXSTYLE) | WS_EX_TOOLWINDOW );
//			wndChangeDlgSize( hDlg, hDlg, 0, 0, "FFT", hInst);
			SetWindowLong( hDlg, DWL_USER, lParam);
			InitSliderCtl( hDlg, IDC_FT_PAR, &S->SB_Par);
			InitSliderCtl( hDlg, IDC_FT_LIN, &S->SB_Lin);
			InitSliderCtl( hDlg, IDC_FT_DC,  &S->SB_DC);
			InitSliderCtl( hDlg, IDC_FT_SC,  &S->SB_Sc);
//			CheckDlgButton( hDlg, IDC_FT_LOG,  S->flag & FF_LOG );
			CheckDlgButton( hDlg, IDC_FT_RESL, S->flag & FF_RES );
			wndPlaceToolWindow( (HWND)lParam, hDlg );
//			SetDlgItemText(hDlg, IDC_FT_RESL, "Status in 1/\302");
#ifdef USE_FONT_MANAGER
			SendDlgItemMessage(hDlg, IDC_FT_RESL_TEXT, WM_SETFONT, (WPARAM)FontManager::GetFont(FM_NORMAL_FONT), 0);
#else
			SendDlgItemMessage(hDlg, IDC_FT_RESL_TEXT, WM_SETFONT, (WPARAM)hfNormal, 0);
#endif
			TextToDlgItemW(hDlg, IDC_FT_RESL_TEXT, "Status in 1/%c", ANGSTR_CHAR);
			return TRUE;

		case WM_HSCROLL:
			switch (GetDlgCtrlID ( (HWND)lParam ))
			{
				case IDC_FT_PAR: UpdateSliderCtl ( hDlg, wParam,lParam, &S->SB_Par); break;
				case IDC_FT_LIN: UpdateSliderCtl ( hDlg, wParam,lParam, &S->SB_Lin); break;
				case IDC_FT_DC:  UpdateSliderCtl ( hDlg, wParam,lParam, &S->SB_DC);  break;
				case IDC_FT_SC:  UpdateSliderCtl ( hDlg, wParam,lParam, &S->SB_Sc);  break;
			}
//			ReplyMessage( TRUE );
//			if(wParam == SB_THUMBTRACK) return TRUE;
//			if (!GetAsyncKeyState(VK_LBUTTON)) {
				{
					BM8		bmdst(S->fs,S->fs,S->img);
					S->ft2.iDC     = S->SB_DC.p;
					S->ft2.iScale  = S->SB_Sc.p;
					S->ft2.iLinear = S->SB_Lin.p;
					S->ft2.iParab  = S->SB_Par.p;
					S->ft2.fDoMaxVal = FALSE;
					S->ft2.DisplayFFT(bmdst);
					bmdst.P = NULL;
//					S->ft2.fDoMaxVal = TRUE;
				}
				imgCalcMinMax( O );
				InvalidateRect( ParentWnd, NULL, 0);
				UpdateWindow( ParentWnd );
//			}
			return TRUE;

		case WM_DESTROY:
				O->hDlg = 0;
				break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					O->hDlg = 0;
					DestroyWindow( hDlg );
					return TRUE;

				case IDC_FT_RESL:
					if (IsDlgButtonChecked( hDlg, IDC_FT_RESL )) S->flag |=  FF_RES;
					else										 S->flag &= ~FF_RES;
					return TRUE;

				case IDC_FT_DEF:
					S->SB_DC.p  = S->SB_DC.def;  S->SB_Sc.p  = S->SB_Sc.def;
					S->SB_Lin.p = S->SB_Lin.def; S->SB_Par.p = S->SB_Par.def;
					InitSliderCtl( hDlg, IDC_FT_PAR, &S->SB_Par);
					InitSliderCtl( hDlg, IDC_FT_LIN, &S->SB_Lin);
					InitSliderCtl( hDlg, IDC_FT_DC,  &S->SB_DC);
					InitSliderCtl( hDlg, IDC_FT_SC,  &S->SB_Sc);
					goto xx;
/*
				case IDC_FT_LOG:
					if (IsDlgButtonChecked( hDlg, IDC_FT_LOG )) S->flag |=  FF_Log;
					else										S->flag &= ~FF_Log;
					EnableWindow( GetDlgItem(hDlg,IDC_FT_PAR), ! (S->flag & FF_Log) );
					EnableWindow( GetDlgItem(hDlg,IDC_FT_LIN), ! (S->flag & FF_Log) );
*/
xx:
				{
					BM8		bmdst(S->fs,S->fs,S->img);
					S->ft2.iDC     = S->SB_DC.p;
					S->ft2.iScale  = S->SB_Sc.p;
					S->ft2.iLinear = S->SB_Lin.p;
					S->ft2.iParab  = S->SB_Par.p;
					S->ft2.DisplayFFT(bmdst);
					bmdst.P = NULL;
				}
					imgCalcMinMax( O );
					InvalidateRect( ParentWnd, NULL, 0);
					return TRUE;

				case IDC_FT_SAVEDEF:
					SaveDef( O );
					return TRUE;
			}
			break;
		case WM_ACTIVATE:
			if (ParentWnd && LOWORD(wParam)==WA_CLICKACTIVE) ActivateObj( ParentWnd );
			break;

	}
	return FALSE;
}
/****************************************************************************/
BOOL	InitFFTObject( HWND hWnd, OBJ* O )
{
	int		scdx=1, scdy=1;
	OBJ*	pr  = OBJ::GetOBJ(O->ParentWnd);
//	LPFFT	S = (LPFFT)&O->dummy;
	LPFFT	S = GetFFT(O);

	strcpy(O->title, "FFT");

	LoadDef();
	S->fs     = O->ui;	/* Dimensions	*/
	S->SB_DC  = SB0_DC;	/* Init scroll bars structures with default values */
	S->SB_Sc  = SB0_Sc;
	S->SB_Lin = SB0_Lin;
	S->SB_Par = SB0_Par;
	S->flag   = 0;
//	S->pix = IMAGE::PixFmt2Bpp(pr->npix);
	S->npix = pr->npix;
	S->ft2 = FT2R(S->fs);
	S->ft2.iDC     = S->SB_DC.p;
	S->ft2.iScale  = S->SB_Sc.p;
	S->ft2.iLinear = S->SB_Lin.p;
	S->ft2.iParab  = S->SB_Par.p;

	O->npix = PIX_BYTE;
	O->imin = 0;
	O->imax = 255;

	O->x  = S->fs;
#ifdef USE_XB_LENGTH
	O->xb = S->fs;
#else
	O->stride = R4(S->fs);
#endif
	O->y  = S->fs;

	O->bp  = O->dp  = NULL;	// To avoid freeing of parent memory which handles was copied
	O->ebp = O->edp = NULL;

// FFT bitmap memory
	O->dp = (LPBYTE)calloc(1, (S->fs)*(S->fs));
	if ( ! O->dp ) return FALSE;
// Mask table
	O->ebp = (LPBYTE)calloc(1, S->fs*4*sizeof(WORD));
	if ( ! O->ebp ) return FALSE;
// Cut image data
	O->edp = (LPBYTE)calloc(1, S->fs*S->fs*IMAGE::PixFmt2Bpp(S->npix));
	if ( ! O->edp ) return FALSE;

	PostMessage( hWnd, FM_Update, 0, 0);

	return	TRUE;
}
/****************************************************************************/
LRESULT CALLBACK FFTWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
	OBJ*	O = OBJ::GetOBJ(hWnd);
	LPFFT	S = (LPFFT)&O->dummy;

	switch(message)
	{
	case WM_CREATE:
		if ( ! InitFFTObject( hWnd, O )) return -1;
		wndSetInitialWindowSize( hWnd );
		UpdateFFT( hWnd );
		break;

	case WM_DESTROY:
		while(O->hCalcTask != 0)
			O = OBJ::GetOBJ(hWnd);
		S->ft2.FT2R::~FT2R();
		if ( O->dp  ) free( O->dp ); O->dp = NULL;		// FFT bitmap
		if ( O->ebp ) free( O->ebp ); O->ebp = NULL;	// Mask
		if ( O->edp ) free( O->edp ); O->edp = NULL;	// Cut of image
		if ( O->hDlg ) DestroyWindow( O->hDlg );
		break;

	case WM_NCDESTROY:
		objFreeObj( hWnd );
		goto CallDCP;

	case FM_Update:
		UpdateFFT( hWnd );
    	break;

	case FM_Update2:
		objInformChildren( hWnd, FM_Update, (WPARAM)O, lParam);
		break;

	case FM_UpdateNI:
		O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
    	objInformChildren( hWnd, FM_UpdateNI, wParam, lParam);
    	break;
	case WM_PAINT:
//		if ( bLive && hWnd==ImagehWnd) goto CallDCP;
//		bDoRealizePal = TRUE;
		pntBitmapToScreen( hWnd, O->dp, S->fs, S->fs, O->xo, O->yo, O->scu, O->scd, O, 0 );
		objInformChildren( hWnd, FM_ParentPaint, (WPARAM)O, 0 );
		break;

	case WM_SETCURSOR:
		if (LOWORD(lParam)==HTCLIENT) {
			SetCursor( hcFFT );
			return TRUE;
		}
		goto CallDCP;

	case WM_LBUTTONUP:
		ReleaseCapture();
		objInformChildren( hWnd, FM_ParentLBUTTONUP, (WPARAM)O, 0 );
		break;

	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
	case WM_MOUSEMOVE:
	{
//		double a,p;
		int	h = (LOWORD(lParam)) * O->scd / O->scu + O->xo - S->fs/2;
		int	k = (HIWORD(lParam)) * O->scd / O->scu + O->yo - S->fs/2;
		ReportCoordinates( lParam, hWnd );
		if (wParam & (MK_LBUTTON) && (GetAsyncKeyState(VK_LBUTTON)&0x8000))
			objInformChildren( hWnd, FM_ParentInspect, (WPARAM)O, MAKELONG(h,k));
		break;
	}
		break;

	case WM_HSCROLL:
		wndUpdateScrollBars( hWnd, wParam, lParam, SB_HORZ);
		objInformChildren( hWnd, FM_ParentSize, (WPARAM)O, 0);
		break;

	case WM_VSCROLL:
		wndUpdateScrollBars( hWnd, wParam, lParam, SB_VERT);
		objInformChildren( hWnd, FM_ParentSize, (WPARAM)O, 0);
		break;

	case WM_NCCALCSIZE:
		if ( O && wParam ) wndTrackWindow( hWnd, O, (LPRECT)lParam );
		goto CallDCP;

	case WM_SIZE:
		// Update new window size and inform childs only in case of "normal" sizes
		if (wParam == SIZENORMAL) {
			O->xw = LOWORD(lParam);
			O->yw = HIWORD(lParam);
			O->xo = (O->x - O->xw * O->scd / O->scu)/2;	// Centering picture
			O->yo = (O->y - O->yw * O->scd / O->scu)/2;
			wndInitScrollBars( hWnd );
			objInformChildren( hWnd, FM_ParentSize, (WPARAM)O, 0);
		}
		break;

	case WM_GETMINMAXINFO:
		// Max track size
		if ( O ) {
			RECT r;
			SetRect(&r, 0, 0, O->x * O->scu / O->scd, O->y * O->scu / O->scd);
			AdjustWindowRectEx( &r, GetWindowLong(hWnd,GWL_STYLE), FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
			((LPPOINT)lParam)[4].x = r.right-r.left;
			((LPPOINT)lParam)[4].y = r.bottom-r.top;
		}
		break;

	case WM_RBUTTONUP:
		wndCreateObjMenu(hWnd, TRUE, TRUE, FALSE, lParam);
		break;

/*
	case WM_INITMENU:
		InitColorMenu( O );
		break;

	case WM_SETFOCUS:
		if ( Mhwnd ) SetFocus( Mhwnd );
		break;
*/
	case WM_ERASEBKGND:
		return FALSE;

	case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS wp = (LPWINDOWPOS)lParam;
			// Setting this bit prevents(?) BitBlt while moving
			wp->flags |= SWP_NOCOPYBITS;
		}
		goto CallDCP;

	case WM_SYSCOMMAND:
	case WM_COMMAND:
		if (wndDefSysMenuProc(hWnd, wParam, lParam)) break;
		if (LOWORD(wParam)==IDM_OM_TUNE)
		{
			if  (O->hDlg)  BringWindowToTop(O->hDlg);
			else O->hDlg = CreateDialogParam(hInst, "FFT", MainhWnd, FFTDlgProc, (LPARAM)hWnd);
			break;
		}
		goto CallDCP;

	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
	case WM_QUERYOPEN:
		ActivateObj(hWnd);
		goto CallDCP;

	case WM_INITMENUPOPUP:
		if (HIWORD(lParam)==0)
			CheckMenuItem((HMENU)wParam, SC_myZOOM | (O->scu<<8) | (O->scd<<4), MF_BYCOMMAND | MF_CHECKED);
		break;

	default:
CallDCP:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	}
   	catch (std::exception& e)
	{
		Trace(_FMT(_T("FFTWndProc() -> message '%s': '%s'"), message, e.what()));
	}
	return FALSE;
}
