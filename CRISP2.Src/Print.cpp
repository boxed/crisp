/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.Printing * by SK, ML **********************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "rgconfig.h"
#include "myprint.h"
#include "scaling.h"

	// Error return codes for DIBPrint().  Error are returned in a bitfield,
	//  since more than one error can ocurr during the print.
#define ERR_PRN_NONE			0x0000	// No error -- everything's A-OK!
#define ERR_PRN_NODC			0x0001	// Couldn't get printer's DC.
#define ERR_PRN_BITBLT			0x0002	// Error in call to BitBlt().
#define ERR_PRN_STARTDOC		0x0004	// Error in STARTDOC escape.
#define ERR_PRN_SETABORTPROC	0x0008	// Error in SETABORTPROC escape.
#define ERR_PRN_NEWFRAME		0x0010	// Error in NEWFRAME or EndPage().
#define ERR_PRN_ENDDOC			0x0020	// Error in ENDDOC or EndDoc().
#define ERR_PRN_MEMORY			0x0040	// Memory Allocation error
#define ERR_PRN_TEXTOUT			0x0080	// Memory Allocation error

// Globals for this module.
static HWND hDlgAbort    = NULL;        // Handle to abort dialog box.
static BOOL bAbort       = FALSE;       // Abort a print operation?
static LPSTR lpDocName 	 = NULL;		// Document name

static char		szPrinter[] = "Printer";
static char		szDevNames[] = "DevNames";
static char		szFooter[] = "Footer";
static char		szMargins[] = "Margins";
static char		szAlign[] = "Align";
static char		szPatRem[] = "PatRem";

static char		szPrDriver[64]="", szPrDevice[64]="", szPrOutput[64]="";
static char		szPrFooter[128] = "*CRISP2* image";

static WORD	wLeft   = 200;			// Printing offsets
static WORD	wRight  = 200;
static WORD	wTop    = 150;
static WORD	wBottom = 150;
static BOOL	bCentX = FALSE, bCentY = FALSE;
static BOOL bNegative = FALSE;

#define	PATREM	4
static WORD wPatIndx = 2;		// Medium
static WORD wPatRem[PATREM]={16,3,2,1};
static LPSTR szPatRemNam[PATREM] = {"Off","Low","Medium","High"};

	// User defined messages.
#define MYWM_CHANGEPCT        WM_USER+1000   // Sent to abort dialog to change % done text
#define MYWM_CHANGECMT        WM_USER+1001   // Sent to abort dialog to change % done text

// Macros
#define ChangePrintPercent(nPct)    SendMessage(hDlgAbort, MYWM_CHANGEPCT, (WPARAM)nPct, 0)
#define ChangePrintComment(cmt)    SendMessage(hDlgAbort, MYWM_CHANGECMT, 0, (LPARAM)(LPSTR)cmt)


// Function prototypes.
BOOL CALLBACK	PrintAbortProc	(HDC hDC, int code);
BOOL CALLBACK	PrintAbortDlg	(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK	xPageSetupDlg	(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

HDC		GetDefaultPrinterDC (void);
BOOL	TranslatePrintRect  (HDC hDC, LPRECT lpPrnRect, UINT cxDIB, UINT cyDIB);

// Export from prn_subr.asm
extern "C" {
void	prn_DitherL			( LPBYTE dst, LPBYTE src, LPWORD err, int cnt, int prem );
void	prn_DitherR			( LPBYTE dst, LPBYTE src, LPWORD err, int cnt, int prem );
void	prn_RGB2Gray		( LPBYTE dst, LPBYTE src, int cnt );
}


#pragma optimize ("",off)

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void memc16(LPBYTE dst, LPWORD src, UINT pcnt, LPBYTE xlt)
{
UINT i;

	for(i=0;i<pcnt;i++,dst++,src++) *dst = xlt[*src];
}
void memc8(LPBYTE dst, LPBYTE src, UINT pcnt, LPBYTE xlt)
{
UINT i;

	for(i=0;i<pcnt;i++,dst++,src++) *dst = xlt[*src];
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static HBITMAP RasterizeBitmap(HDC hDC, LPRECT lpR, LPBYTE lpSrc, int iWidth, int iHeight, int iBW, int ps)
{
int		ys, xs, nPct, y, xbw, xis, ls;
LPBYTE	lpX=NULL, lpD=NULL, lpS=NULL;
LPWORD	lpE=NULL;
HBITMAP	hbmp=NULL, hobmp=NULL, hsbmp=NULL, hsobmp=NULL;
HDC		hMemDC, hSrcDC;
BOOL	err = FALSE;
LPBYTE	xlt;

	if ((xlt=(LPBYTE)malloc(65536)) == NULL ) return NULL;

	if (ps==1) {
		if (bNegative)	for(y=0;y<256;y++) xlt[y]=y^255;
		else			for(y=0;y<256;y++) xlt[y]=y;
	} else if (ps==2) {
		if (bNegative)	for(y=0;y<65536;y++) xlt[y]=(y>>8)^255;
		else			for(y=0;y<65536;y++) xlt[y]=y>>8;
	}

	xs = lpR->right - lpR->left;
	ys = lpR->bottom - lpR->top;
		
	xbw = R8(__max(xs, iWidth));         
	xis = R8(xs);
	lpX = (LPBYTE)malloc(xbw);		// One bpp
	lpS = (LPBYTE)malloc(xbw+1);	// Source for scaler
	lpD = (LPBYTE)malloc(xbw*2);	// Dest for scaler
	lpE = (LPWORD)calloc( (xbw+2)*4, 1);// Dithering error buffer
	hbmp = CreateBitmap( xs, ys, 1, 1, NULL );
	hsbmp = CreateBitmap( xs, 1, 1, 1, NULL );
	hMemDC = CreateCompatibleDC( hDC );
	hSrcDC = CreateCompatibleDC( hDC );
	
	if ( hbmp==NULL || lpX==NULL || lpS==NULL || lpD==NULL || lpE==NULL || hMemDC==NULL ||
		 hsbmp==NULL || hSrcDC==NULL) {
		goto ex;
	}

	ChangePrintComment("Rasterizing");

	hobmp  = SelectBitmap( hMemDC, hbmp );
	hsobmp  = SelectBitmap( hSrcDC, hsbmp );
//	PatBlt( hMemDC, 0, 0, lpR->right-lpR->left, lpR->bottom-lpR->top, BLACKNESS);
	PatBlt( hMemDC, 0, 0, lpR->right-lpR->left, lpR->bottom-lpR->top, WHITENESS);

	sc1Init( xs, ys, iWidth, iHeight );
	switch(ps){
		case 3: prn_RGB2Gray( lpS, lpSrc, iWidth); break;
		case 2: memc16(lpS, (LPWORD)lpSrc, iWidth, xlt); break;
		case 1: memc8 (lpS,         lpSrc, iWidth, xlt); break;
	}
	lpSrc+=iBW; ls=1;
		
	for(y=0; y<ys; y++ ) {
		while( !sc1Line (lpD, lpS)) {
			if (ls<iHeight) {
				switch(ps){
					case 3: prn_RGB2Gray( lpS, lpSrc, iWidth); break;
					case 2: memc16(lpS, (LPWORD)lpSrc, iWidth, xlt); break;
					case 1: memc8 (lpS,         lpSrc, iWidth, xlt); break;
				}
				lpSrc+=iBW;
				ls++;
			} else {
				nPct=ls;			
			}
		}
		if (y&1) prn_DitherL( lpX, lpD, lpE, xis, wPatRem[wPatIndx]);
		else     prn_DitherR( lpX, lpD, lpE, xis, wPatRem[wPatIndx]);
		SetBitmapBits( hsbmp, (xs+7)/8, lpX );

		if ( ! BitBlt( hMemDC, 0, y, xs, 1,	hSrcDC, 0, 0, SRCCOPY )) {
			err = TRUE;
			break;
		}
		// Change percentage of print shown in abort dialog.
		nPct = MulDiv (y+1, 100, ys);
		ChangePrintPercent (nPct);
		PrintAbortProc(hDC, 0);
		if (bAbort) break;
	}

	sc1Free();
	if (hobmp) SelectObject(hMemDC, hobmp);
	if (hsobmp) SelectObject(hSrcDC, hsobmp);
ex:                                    
	if (lpD) free( lpD );
	if (lpS) free( lpS );
	if (lpX) free( lpX );
	if (lpE) free( lpE );
	if (hMemDC) DeleteDC( hMemDC );
	if (hSrcDC) DeleteDC( hSrcDC );
	if (hsbmp) DeleteObject( hsbmp );

//	if (!err) 
		return hbmp;
//	else
//		{ DeleteObject(hbmp); return NULL; }

}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static WORD PrintBitmapBand( HDC hDC, HDC hsDC, LPRECT rO, LPRECT rB )
{
RECT	rI;
WORD	wErr = ERR_PRN_NONE, nPct;

	nPct = MulDiv(rB->bottom, 100, GetDeviceCaps (hDC, VERTRES));
	if (nPct>100) nPct=100;
	ChangePrintPercent( nPct );
	if ( ! IntersectRect( &rI, rO, rB ) ) return wErr;
	if ( ! BitBlt( hDC, rI.left, rI.top, rI.right-rI.left, rI.bottom-rI.top,
				   hsDC, rI.left-rO->left,rI.top-rO->top, SRCCOPY) )
		wErr |= ERR_PRN_BITBLT;
	return wErr;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static WORD PrintTextBand( HDC hDC, LPRECT rO, LPRECT rB, LPSTR lpName )
{
RECT	rI, rt;
WORD	wErr = ERR_PRN_NONE;
int		xt, yt;
SIZE	ts;

	if (strlen(lpName)) {
		GetTextExtentPoint32( hDC, lpName, strlen(lpName), &ts);
		xt = (rO->left + rO->right - ts.cx)/2;
		yt = rO->bottom+ts.cy;
		SetRect( &rt, xt, yt, xt+ts.cx, yt+ts.cy);
		if ( IntersectRect( &rI, rB, &rt ) && ! TextOut(hDC, xt, yt, lpName, strlen(lpName)) )
				wErr |= ERR_PRN_TEXTOUT;
	}

	if (strlen(szPrFooter)) {
		GetTextExtentPoint32( hDC, szPrFooter, strlen(szPrFooter), &ts);
		xt = (rO->left + rO->right - ts.cx)/2;
		yt = GetDeviceCaps (hDC, VERTRES)-ts.cy;
		SetRect( &rt, xt, yt, xt+ts.cx, yt+ts.cy);
		if ( IntersectRect( &rI, rB, &rt ) )
			if ( ! TextOut(hDC, xt, yt, szPrFooter, strlen(szPrFooter)) )
				wErr |= ERR_PRN_TEXTOUT;
	}
	return wErr;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
VOID prnPrintImage (LPBYTE lpBits, int iWidth, int iHeight, int iBufWidth, LPSTR  lpName, int ps)
{
	HDC				hPrnDC=NULL, hMemDC=NULL;
	WORD			wErr = ERR_PRN_NONE;
	DOCINFO			DocInfo;
	int				i;
	RECT			rO={0,0,iWidth,iHeight}, rB;
	HBITMAP			hBmp=NULL, hoBmp = NULL;
	BOOL			bBanding = FALSE;

	bAbort      = FALSE;

	if (strlen(szPrDriver)*strlen(szPrDevice)*strlen(szPrOutput)) 
		hPrnDC = CreateDC(szPrDriver, szPrDevice, szPrOutput, NULL);
	else
		hPrnDC = GetDefaultPrinterDC ();

	i = NEXTBAND;
	bBanding = Escape(hPrnDC, QUERYESCSUPPORT, sizeof(int), (LPSTR)&i, NULL );
	
	if (hPrnDC && TranslatePrintRect (hPrnDC, &rO, iWidth, iHeight))
	{
		SetStretchBltMode (hPrnDC, COLORONCOLOR);

		lpDocName = lpName;
		// Initialize the abort procedure.  Then STARTDOC.
		hDlgAbort   = CreateDialog(hInst, "PrintAbort", MainhWnd, PrintAbortDlg);

		// SetAbortProc
		if (SetAbortProc (hPrnDC, PrintAbortProc) < 0) {
			wErr |= ERR_PRN_SETABORTPROC;
			goto ex;
		}

		ShowWindow( hDlgAbort, SW_NORMAL );
		UpdateWindow( hDlgAbort );
		EnableWindow( MainhWnd, FALSE );
		
		// Rasterizing bitmap
//		if (bColor) iBufWidth *= 3;
		hBmp = RasterizeBitmap(hPrnDC, &rO, lpBits, iWidth, iHeight, iBufWidth, ps);
		if (hBmp==NULL) {
			wErr |= ERR_PRN_MEMORY;
			goto ex;
		}
		hMemDC = CreateCompatibleDC( hPrnDC );
		if (hMemDC==NULL) {
			wErr |= ERR_PRN_MEMORY;
			goto ex;
		}

		SelectObject( hMemDC, hBmp );		

		ChangePrintComment("Printing");
		ChangePrintPercent( 0 );
		// StartDoc
		DocInfo.cbSize      = sizeof (DOCINFO);
		DocInfo.lpszDocName = "SHT Print Engine";
		DocInfo.lpszOutput  = NULL;
		DocInfo.lpszDatatype= NULL;
		DocInfo.fwType      = 0;
		if (StartDoc (hPrnDC, &DocInfo) < 0) {
			wErr |= ERR_PRN_STARTDOC;
			goto ex;
		}

/*		if (bBanding && FALSE) {
			while ( Escape(hPrnDC, NEXTBAND, 0, 0, (LPSTR)&rB ) > 0 &&
				   ! IsRectEmpty( &rB )) {
				PrintAbortProc(hPrnDC, 0);
				if (bAbort) break;
				wErr |= PrintBitmapBand( hPrnDC, hMemDC, &rO, &rB);
				wErr |= PrintTextBand  ( hPrnDC, &rO, &rB, lpName);
			}
		} else {
*/
			StartPage( hPrnDC );

			SetRect( &rB, 0, 0, GetDeviceCaps (hPrnDC, HORZRES), GetDeviceCaps (hPrnDC, VERTRES));
			wErr |= PrintBitmapBand( hPrnDC, hMemDC, &rO, &rB);
			wErr |= PrintTextBand  ( hPrnDC, &rO, &rB, lpName);

			// Non-banding devices need the NEWFRAME or EndPage() call.
			if (!bAbort && EndPage(hPrnDC) < 0) wErr |= ERR_PRN_NEWFRAME;
//		}

		// End the print operation.  Only send the ENDDOC if
		//   we didn't abort or error.
		i = GetLastError();

		if (bAbort) AbortDoc(hPrnDC);
		else if (EndDoc(hPrnDC) < 0) wErr |= ERR_PRN_ENDDOC;

		// All done, clean up.
ex:
		EnableWindow( MainhWnd, TRUE );
		DestroyWindow (hDlgAbort);
		
	} else wErr |= ERR_PRN_NODC;

	// Error reporting
	if (bAbort) {
		if (LoadString(hInst, IDS_PE_ABORTED, TMP, sizeof(TMP)))
			MessageBox( MainhWnd, TMP, "Print", MB_ICONSTOP | MB_OK );
	} else 
	if (wErr) {
		i=IDS_PE_NODC;
		while (wErr) {
			if (wErr&1) {
				if (LoadString(hInst, i, TMP, sizeof(TMP)))
					MessageBox( MainhWnd, TMP, "Print", MB_ICONSTOP | MB_OK );
				break;
			}
			wErr=wErr>1;
			i++;
		}
		
	} 

	if (hoBmp) SelectObject(hMemDC, hoBmp);
	if (hPrnDC)	DeleteDC(hPrnDC);
	if (hMemDC) DeleteDC(hMemDC);
	if (hBmp) DeleteObject(hBmp);
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static BOOL TranslatePrintRect (HDC hDC, LPRECT lpR, UINT cxDIB, UINT cyDIB)
{
int cxPage, cyPage, cxInch, cyInch;
double	dXmm, dYmm;
int dx, dy;

	if (!hDC)
		return FALSE;
	
	cxPage = GetDeviceCaps (hDC, HORZRES);
	cyPage = GetDeviceCaps (hDC, VERTRES);
	cxInch = GetDeviceCaps (hDC, LOGPIXELSX);
	cyInch = GetDeviceCaps (hDC, LOGPIXELSY);
	dXmm = cxInch/254.;
	dYmm = cyInch/254.;

	dx = cxPage - (int)((wLeft+wRight)*dXmm+.5);	// X extent of printing Rect
	dy = cyPage - (int)((wTop+wBottom)*dYmm+.5);	// X extent of printing Rect

	if ((dx<16) || (dy<8)) return FALSE;
	
	if (cxDIB*dy > cyDIB*dx) {
	// Fit Horizontaly
		lpR->left = (int)(wLeft*dXmm+.5);
		lpR->right = lpR->left+dx;
		dy = (int)(dx*dXmm*cyDIB/(double)cxDIB/dYmm);
		if (bCentY) lpR->top = (cyPage-dy)/2;
		else        lpR->top = (int)(wTop*dYmm+.5);
		lpR->bottom = lpR->top+dy;
	} else {
	// Fit Verticaly
		lpR->top =  (int)(wTop*dYmm+.5);
		lpR->bottom = lpR->top+dy;
		dx = (int)(dy*dYmm*cxDIB/(double)cyDIB/dXmm);
		if (bCentX) lpR->left = (cxPage-dx)/2;
		else        lpR->left = (int)(wLeft*dXmm+.5);
		lpR->right = lpR->left+dx;
	}
	return TRUE;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
HDC GetDefaultPrinterDC (void)
{
	PRINTDLG pd;
	
	pd.lStructSize          = sizeof (pd);
	pd.hwndOwner            = NULL;
	pd.hDevMode             = NULL;
	pd.hDevNames            = NULL;
	pd.hDC                  = NULL;
	pd.Flags                = PD_RETURNDC | PD_RETURNDEFAULT;
	pd.nFromPage            = 0;
	pd.nToPage              = 0;
	pd.nMinPage             = 0;
	pd.nMaxPage             = 0;
	pd.nCopies              = 0;
	pd.hInstance            = NULL;
	pd.lCustData            = 0;
	pd.lpfnPrintHook        = NULL;
	pd.lpfnSetupHook        = NULL;
	pd.lpPrintTemplateName  = NULL;
	pd.lpSetupTemplateName  = NULL;
	pd.hPrintTemplate       = NULL;
	pd.hSetupTemplate       = NULL;

	if (PrintDlg (&pd))
		return pd.hDC;
	else
		return NULL;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
VOID	prnPrintSetup( HWND hWnd )
{
PRINTDLG	pd;
LPDEVNAMES	lpn=NULL;
LPSTR		lps;
WORD		siz;

	siz = strlen(szPrDriver)+strlen(szPrDevice)+strlen(szPrOutput);
	if (siz) {
		siz = sizeof(DEVNAMES)+300;
		lpn = (LPDEVNAMES)GlobalAllocPtr(  GMEM_MOVEABLE | GMEM_ZEROINIT, siz );
		lps = ((LPSTR) lpn) + sizeof(DEVNAMES);
		lpn->wDriverOffset = sizeof(DEVNAMES);
		lpn->wDeviceOffset = sizeof(DEVNAMES)+100;
		lpn->wOutputOffset = sizeof(DEVNAMES)+200;
		strcpy( lps,     szPrDriver );
		strcpy( lps+100, szPrDevice );
		strcpy( lps+200, szPrOutput );
//		lpn->wDefault = DN_DEFAULTPRN;
	}

	memset(&pd,0,sizeof(pd));					
	pd.lStructSize   = sizeof (pd);
	pd.Flags         = PD_PRINTSETUP;
	pd.hwndOwner = hWnd;
	if (lpn) pd.hDevNames = GlobalPtrHandle( lpn );
	if (PrintDlg (&pd)) {
		if (pd.hDevNames) {
			lpn = (LPDEVNAMES)GlobalLock( pd.hDevNames );
			lps = (LPSTR) lpn;
			strcpy( szPrDriver, lps + lpn->wDriverOffset);
			strcpy( szPrDevice, lps + lpn->wDeviceOffset);
			strcpy( szPrOutput, lps + lpn->wOutputOffset);
			GlobalUnlock( pd.hDevNames );
		}
/*		if (lpn->wDefault & DN_DEFAULTPRN) {
			szPrDriver[0]=0;
			szPrDevice[0]=0;
			szPrOutput[0]=0;
		}*/
 	} else if (CommDlgExtendedError() == PDERR_PRINTERNOTFOUND && pd.hDevNames) {
		szPrDriver[0]=0;
		szPrDevice[0]=0;
		szPrOutput[0]=0;
 	}
 
	if (pd.hDevNames) GlobalFree( pd.hDevNames );
	if (pd.hDevMode) GlobalFree( pd.hDevMode );
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
BOOL CALLBACK PrintAbortProc(HDC hDC, int code)
{
	MSG msg;
	
	bAbort |= (code != 0);
	
	while (!bAbort && PeekMessage (&msg, 0, 0, 0, PM_REMOVE))
		if (!IsDialogMessage (hDlgAbort, &msg)) {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	return !bAbort;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
BOOL CALLBACK PrintAbortDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			if (lpDocName) SetDlgItemText( hDlg, IDC_PRN_TEXT1, lpDocName);
			wndCenter( hDlg, MainhWnd, 0 );
			SetFocus(hDlg);
			return TRUE;
			
		case WM_COMMAND:
			bAbort = TRUE;
			return TRUE;
			
		case MYWM_CHANGEPCT:
		{
			char szBuf[20];
			wsprintf (szBuf, "%3d%% done", wParam);
			SetDlgItemText (hDlg, IDC_PRN_TEXT2, szBuf);
			return TRUE;
		}
		case MYWM_CHANGECMT:
			SetDlgItemText (hDlg, IDC_PRN_TEXT0, (LPSTR)lParam);
			return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
VOID	prnLoadConfig( void )
{
	regGetDataDef( szPrinter, szDevNames, "", TMP, sizeof(TMP));
	if (strlen(TMP) > 3 ) {
		LPSTR lp2 = strchr(TMP,','), lp3 = strrchr(TMP,',');
		if (lp2 && lp3) {
			*(lp2++) = 0; *(lp3++) = 0;
			strcpy( szPrDriver, TMP );
			strcpy( szPrDevice, lp2 );
			strcpy( szPrOutput, lp3 );
		}
	}
	regGetDataDef(szPrinter, szFooter, szPrFooter, szPrFooter, sizeof(szPrFooter));
	regGetDataDef(szPrinter, szMargins, "", TMP, sizeof(TMP));
	if (7 <= strlen(TMP)) {
		sscanf( TMP, "%d,%d,%d,%d", &wLeft, &wRight, &wTop, &wBottom );
	}
	regGetDataDef(szPrinter, szAlign, "", TMP, sizeof(TMP));
	if (3 <= strlen(TMP)) {
		sscanf( TMP, "%d,%d", &bCentX, &bCentY );
	}
	wPatIndx = regGetInt( szPrinter, szPatRem, wPatIndx );
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
VOID	prnSaveConfig( void )
{
	sprintf(TMP, "%s,%s,%s", szPrDriver,szPrDevice,szPrOutput); 
	regSetData( szPrinter, szDevNames, TMP, 0);
	regSetData( szPrinter, szFooter, szPrFooter, 0);
	sprintf(TMP, "%d,%d,%d,%d", wLeft,wRight,wTop,wBottom); regSetData( szPrinter, szMargins, TMP, 0);
	sprintf(TMP,"%d,%d",bCentX,bCentY);	regSetData( szPrinter, szAlign, TMP, 0);
	sprintf(TMP,"%d",wPatIndx);	regSetData( szPrinter, szPatRem, TMP, 0);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
BOOL prnPageSetup( HWND hWnd, BOOL bNeg )
{
	bNegative = bNeg;
	return DialogBox( hInst, "PageSetup", hWnd, xPageSetupDlg );
}                      

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static VOID UpdateNumbersPS( HWND hDlg )
{
	if (bCentX) wRight = wLeft;
	if (bCentY) wBottom = wTop;
	sprintf( TMP, "%5.2f", wLeft/100. );	SetDlgItemText( hDlg, IDC_PS_LEFT, TMP );
	sprintf( TMP, "%5.2f", wRight/100. );	SetDlgItemText( hDlg, IDC_PS_RIGHT, TMP );
	sprintf( TMP, "%5.2f", wTop/100. );		SetDlgItemText( hDlg, IDC_PS_TOP, TMP );
	sprintf( TMP, "%5.2f", wBottom/100. );	SetDlgItemText( hDlg, IDC_PS_BOTTOM, TMP );
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static VOID EnablePS( HWND hDlg )
{
	EnableWindow(GetDlgItem( hDlg, IDC_PS_RIGHT),  !bCentX);
	EnableWindow(GetDlgItem( hDlg, IDC_PS_BOTTOM), !bCentY);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static VOID InitPS( HWND hDlg )
{
int i;
HWND hw = GetDlgItem( hDlg, IDC_PS_PREM );

//	SendDlgItemMessage( hDlg, IDC_PS_PRINTERNAME, WM_SETFONT, (WPARAM)hfSmall, 0 );
	if (strlen(szPrDevice)) {
		sprintf( TMP, "%s on %s", szPrDevice, szPrOutput );
		SetDlgItemText( hDlg, IDC_PS_PRINTERNAME, TMP );
	} else 
		SetDlgItemText( hDlg, IDC_PS_PRINTERNAME, "Default" );
	SetDlgItemText( hDlg, IDC_PS_FOOTER,		szPrFooter );
	CheckDlgButton( hDlg, IDC_PS_CENTX, 		bCentX );	                             
	CheckDlgButton( hDlg, IDC_PS_CENTY, 		bCentY );	                             
	CheckDlgButton( hDlg, IDC_PS_NEGATIVE, 		bNegative );	                             
	EnablePS( hDlg );
	UpdateNumbersPS( hDlg );	                             
	ComboBox_ResetContent( hw );
	for (i=0;i<PATREM;i++) 
		ComboBox_InsertString(hw, -1, (LPSTR)szPatRemNam[i]);
	ComboBox_SetCurSel( hw, wPatIndx );
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static BOOL ScanfPS( LPWORD lpw, HWND hw)
{
float	r;

	if (SendMessage(hw, EM_GETMODIFY, 0, 0)) {
		SendMessage(hw, EM_SETMODIFY, 0, 0);
		if (GetWindowText( hw, TMP, sizeof(TMP)) && 1==sscanf(TMP,"%f",&r))
			{ *lpw = (int)(r*100.+.5); return TRUE; }
	}
	return FALSE;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
BOOL CALLBACK xPageSetupDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static	int wL, wR, wT, wB, wPi;

	switch (msg) {
		case WM_INITDIALOG:
			InitPS( hDlg );
			wndCenter( hDlg, MainhWnd, 0 );
			SetFocus(hDlg);
			wL = wLeft; wR = wRight; wT = wTop; wB = wBottom;
			wPi = wPatIndx;
			return TRUE;
			
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
					wLeft = wL; wRight = wR; wTop = wT; wBottom = wB;
					wPatIndx = wPi;
					EndDialog( hDlg, FALSE );
					break;
				case IDOK:
					EndDialog( hDlg, TRUE );
					break;

				case IDC_PS_LEFT:
					if (HIWORD(wParam)==EN_KILLFOCUS && ScanfPS(&wLeft, (HWND)lParam))
						goto upd1;
					break;
				case IDC_PS_RIGHT:
					if (HIWORD(wParam)==EN_KILLFOCUS && ScanfPS(&wRight, (HWND)lParam))
						goto upd1;
					break;
				case IDC_PS_TOP:
					if (HIWORD(wParam)==EN_KILLFOCUS && ScanfPS(&wTop, (HWND)lParam))
						goto upd1;
					break;
				case IDC_PS_BOTTOM:
					if (HIWORD(wParam)==EN_KILLFOCUS && ScanfPS(&wBottom, (HWND)lParam))
						goto upd1;
					break;

				case IDC_PS_FOOTER:
					if (HIWORD(wParam)==EN_KILLFOCUS)
						GetDlgItemText( hDlg, IDC_PS_FOOTER, szPrFooter, sizeof(szPrFooter) );
					break;

				case IDC_PS_CENTX:
					bCentX = IsDlgButtonChecked( hDlg, IDC_PS_CENTX );
					goto upd;
					
				case IDC_PS_CENTY:
					bCentY = IsDlgButtonChecked( hDlg, IDC_PS_CENTY );
upd:				EnablePS( hDlg );
upd1:				UpdateNumbersPS( hDlg );	                             
					break;

				case IDC_PS_NEGATIVE:
					bNegative = IsDlgButtonChecked( hDlg, IDC_PS_NEGATIVE );
					break;
				case IDC_PS_SETUP:
					prnPrintSetup( hDlg );
					InitPS( hDlg );
					break;

				case IDC_PS_PREM:
					if (HIWORD(lParam)==CBN_SELCHANGE)
						wPatIndx = ComboBox_GetCurSel( (HWND)LOWORD(lParam) );
					break;
				default:	break;
			}
			return TRUE;
			
	}
	return FALSE;
}

