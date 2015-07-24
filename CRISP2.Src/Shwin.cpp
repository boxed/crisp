/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Window management * by ML ************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "objects.h"
#include "globals.h"
#include "shtools.h"
#include "const.h"

#pragma optimize("",off)

/****************************************************************************/
VOID WINAPI wndPlaceToolWindow(HWND hWnd, HWND hDlg )
{
	RECT ro, rd, rm;
	int	x, y;

	GetWindowRect( hWnd, &ro ); MapWindowPoints( NULL, MDIhWnd, (LPPOINT)&ro, 2 );
	GetWindowRect( hDlg, &rd );
	GetClientRect( MDIhWnd, &rm );
	if (ro.bottom+(rd.bottom-rd.top) <= rm.bottom) { // under the main
		x = ro.left; y=ro.bottom;
	} else if (ro.right+(rd.right-rd.left) <= rm.right) { // right to the main
		x = ro.right; y=ro.top;
	} else return;
	rd.left = x; rd.top = y;
	MapWindowPoints( MDIhWnd, NULL, (LPPOINT)&rd, 1 );
	SetWindowPos ( hDlg, 0, rd.left, rd.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
/****************************************************************************/
VOID WINAPI wndSetWindowSize(HWND hWnd, int x, int y, BOOL bMenu )
{
RECT r;

	SetRect(&r, 0, 0, x, y);
	AdjustWindowRectEx( &r, GetWindowLong(hWnd,GWL_STYLE), bMenu, GetWindowLong(hWnd,GWL_EXSTYLE));
	SetWindowPos ( hWnd, 0, 0, 0, r.right-r.left, r.bottom-r.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}
/****************************************************************************/
VOID WINAPI wndSetWindowPos(HWND hWnd, int x, int y )
{
RECT	swp;
int		width, height;

 	GetWindowRect(hWnd, &swp);
 	width = swp.right-swp.left;
 	height = swp.bottom-swp.top;

	if ( x==-1 || y==-1 ) {
		x = (GetSystemMetrics(SM_CXSCREEN) - width)/2;
		y = (GetSystemMetrics(SM_CYSCREEN) - height)/2;
	}
 	MoveWindow(hWnd, x, y, width, height, FALSE);
}
/****************************************************************************/
VOID WINAPI wndGetWindowPos(HWND hWnd, LPINT x, LPINT y )
{
RECT	swp;

 	GetWindowRect(hWnd, &swp);
	*x = swp.left;
	*y = swp.top;
}
/****************************************************************************/
VOID WINAPI wndCenter(HWND hWnd, HWND Mhwnd, int top)
{
POINT		pt;
RECT		swp;
int			iwidth, iheight;
int			cxscreen = GetSystemMetrics( SM_CXSCREEN );
int			cyscreen = GetSystemMetrics( SM_CYSCREEN );

 	GetWindowRect(hWnd, &swp);
 	iwidth  = swp.right - swp.left;
 	iheight = swp.bottom - swp.top;
 	GetClientRect(Mhwnd, &swp);
 	pt.x = (swp.right  - swp.left) / 2;
 	pt.y = (swp.bottom - swp.top)  / 2;
 	ClientToScreen(Mhwnd, &pt);
 	pt.x -= (iwidth  / 2);
 	pt.y -= (iheight / 2);
    if(top) pt.y = pt.y + top;
    if (pt.x < 0) pt.x = 0; if( pt.x+iwidth  > cxscreen ) pt.x = cxscreen - iwidth;
    if (pt.y < 0) pt.y = 0; if( pt.y+iheight > cyscreen ) pt.y = cyscreen - iheight;
 	MoveWindow(hWnd, pt.x, pt.y, iwidth, iheight, FALSE);
}
/****************************************************************************/
/* MoveChWindow	*************************************************************/
/****************************************************************************/
VOID WINAPI wndMoveChiWindow(HWND hWnd, int x, int y, BOOL f)
{
RECT       swp;

 	GetWindowRect(hWnd, &swp);
 	if (x<0) x=0;
 	if (y<0) y=0;
 	if (x > GetSystemMetrics(SM_CXSCREEN) - (swp.right-swp.left))
 		x = GetSystemMetrics(SM_CXSCREEN) - (swp.right-swp.left);
 	if (y > GetSystemMetrics(SM_CYSCREEN) - (swp.bottom-swp.top))
 		y = GetSystemMetrics(SM_CYSCREEN) - (swp.bottom-swp.top);
 	MoveWindow(hWnd, x, y, swp.right-swp.left, swp.bottom-swp.top, f);
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/***************************************************************************/
VOID WINAPI wndUpdateScrollBars( HWND hWnd, WPARAM wParam, LPARAM lParam, int sb)
{
	int		pos, st=4, pg=16;
	int		xs, ys;
	RECT	r;
	OBJ* O = OBJ::GetOBJ( hWnd );

	GetClientRect( hWnd, &r );
	xs = O->x - (r.right  * O->scd + O->scu-1) / O->scu;
	ys = O->y - (r.bottom * O->scd + O->scu-1) / O->scu;
	if ( sb == SB_HORZ ) {
		pos =  GetScrollPos( hWnd, SB_HORZ);//O->xo;
		switch( LOWORD(wParam) ){
		  case SB_THUMBTRACK   :
		  case SB_THUMBPOSITION: pos = HIWORD(wParam);  break;
	      case SB_LINEDOWN     : pos += st;				break;
	      case SB_LINEUP       : pos -= st;				break;
	      case SB_PAGEDOWN     : pos += pg; 	  		break;
	      case SB_PAGEUP       : pos -= pg; 	  		break;
	      case SB_TOP          : pos =  0;	 	  		break;
	      case SB_BOTTOM	   : pos =  xs; 			break;
	      case 0xFFFF      	   : pos =  O->xo;
								 SetScrollPos(hWnd, SB_HORZ, pos, 1);
	      						 break;
	    }
		if (LOWORD(wParam) != SB_ENDSCROLL && LOWORD(wParam)!=0xFFFF) {
			MapWindowPoints(hWnd,NULL,(LPPOINT)&r,2);
			r.top=r.bottom+1;
			r.bottom += GetSystemMetrics(SM_CYHSCROLL);
			ClipCursor(&r);
		} else ClipCursor( NULL );

	    if(pos < 0) pos = 0; if(pos > xs) pos = xs;
		if (pos != O->xo) {
			ScrollWindowEx( hWnd, O->xo-pos, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);
			O->xo = pos;
			O->clickpoint.x = O->xo + (O->x-xs)/2;
			SetScrollPos(hWnd, SB_HORZ, pos, 1);
		}
	} else {
		pos = GetScrollPos( hWnd, SB_VERT); //ys - O->yo;
		switch( LOWORD(wParam) ){
		  case SB_THUMBTRACK   :
		  case SB_THUMBPOSITION: pos = HIWORD(wParam);  break;
	      case SB_LINEDOWN     : pos += st;				break;
	      case SB_LINEUP       : pos -= st;				break;
	      case SB_PAGEDOWN     : pos += pg; 	  		break;
	      case SB_PAGEUP       : pos -= pg; 	  		break;
	      case SB_TOP          : pos =  0;	 	  		break;
	      case SB_BOTTOM	   : pos =  ys; 			break;
	      case 0xFFFF      	   : pos =  O->yo;
								 SetScrollPos(hWnd, SB_VERT, pos, 1);
	      						 break;
	    }
		if (LOWORD(wParam) != SB_ENDSCROLL && LOWORD(wParam)!=0xFFFF) {
			MapWindowPoints(hWnd,NULL,(LPPOINT)&r,2);
			r.left=r.right+1;
			r.right += GetSystemMetrics(SM_CXVSCROLL);
			ClipCursor(&r);
		} else ClipCursor( NULL );


	    if(pos < 0) pos = 0; if(pos > ys) pos = ys;
		if (pos !=  O->yo) {
			ScrollWindowEx( hWnd, 0, O->yo-pos, NULL, NULL, NULL, NULL, SW_INVALIDATE);
			O->yo = pos;
			O->clickpoint.y = O->yo + (O->y-ys)/2;
			SetScrollPos(hWnd, SB_VERT, pos, 1);
		}
	}
}
/****************************************************************************/
VOID WINAPI wndInitScrollBars( HWND hWnd )
{
OBJ* O = OBJ::GetOBJ(hWnd);
int		mx, my;

	mx = O->x - (O->xw * O->scd + O->scu-1) / O->scu;
	my = O->y - (O->yw * O->scd + O->scu-1) / O->scu;
	if (mx>0) {
		SetScrollRange( hWnd, SB_HORZ, 0, mx, FALSE);
		wndUpdateScrollBars( hWnd, 0xFFFF, 0, SB_HORZ );
	} else { O->xo = 0; }
	if (my>0) {
		SetScrollRange( hWnd, SB_VERT, 0, my, FALSE);
		wndUpdateScrollBars( hWnd, 0xFFFF, 0, SB_VERT );
	} else { O->yo = 0; }
}
/****************************************************************************/
VOID WINAPI wndTrackWindow( HWND hWnd, OBJ* O, LPRECT r )
{
int		xs, ys, xsd, ysd;
DWORD	style, style0, exstyle;
RECT	rr;

	xs = r->right  - r->left;
	ys = r->bottom - r->top;

	SetRect( &rr, 0, 0, O->x * O->scu / O->scd, O->y * O->scu / O->scd);
	style0 = style = GetWindowLong( hWnd, GWL_STYLE );
	exstyle = GetWindowLong( hWnd, GWL_EXSTYLE );
	style &= ~(WS_HSCROLL | WS_VSCROLL);
	AdjustWindowRectEx(&rr, style, FALSE, exstyle);
	style = style0;
	xsd = rr.right-rr.left;
	ysd = rr.bottom-rr.top;

	if ( xs < xsd ) { style |=  WS_HSCROLL; ysd += GetSystemMetrics(SM_CYHSCROLL); }
	else		    style &= ~WS_HSCROLL;
	if ( ys < ysd ) { style |=  WS_VSCROLL; xsd += GetSystemMetrics(SM_CXVSCROLL); }
	else			style &= ~WS_VSCROLL;
	if ( xs < xsd ) style |=  WS_HSCROLL;
	else		    style &= ~WS_HSCROLL;
	if ( style0 != style ) {
		SetWindowLong( hWnd, GWL_STYLE, style );
		InvalidateRect( hWnd, NULL, TRUE );
	}
}
/***************************************************************************/
/* SetDlgItemDouble ********************************************************/
/***************************************************************************/
VOID WINAPI SetDlgItemDouble( HWND hDlg, int idc, double x, int w )
{
char	xxx[30];
HWND	hw = GetDlgItem(hDlg,idc);
RECT	r;
HDC		hdc;
SIZE	ts;
int		i,k,l;

	if (w==0) {
		sprintf(xxx, "%g", x);
	} else if (w>0) {
		GetClientRect(hw, &r);
		hdc = GetDC( hw );
		GetTextExtentPoint32( hdc, " ", strlen(" "), &ts );
		for(i=0;i<=w;i++) {
			k = sprintf( xxx, "%-*.*f", w,i,x );
			if(strchr(xxx,' ')) *(strchr(xxx,' ')) = 0;
			l = strlen(xxx);
			GetTextExtentPoint32( hdc, xxx, strlen(xxx), &ts );
			if (ts.cx-8>r.right) break;	// This -8 is a kinda strange, but without it does not work
		}
		if (i) i--;
		ReleaseDC(hw, hdc);
		sprintf( xxx, "%-*.*f", w,i, x );
	} else {
		sprintf( xxx, "%-.*f", -w, x );
	}
	if(strchr(xxx,' ')) *(strchr(xxx,' ')) = 0;
	SetDlgItemText( hDlg, idc, xxx );
}
/****************************************************************************/
/* GetDlgItemFloat **********************************************************/
/****************************************************************************/
BOOL WINAPI GetDlgItemDouble(HWND hDlg, int IDC, double *result, int w)
{
char * p;
	if (SendDlgItemMessage(hDlg, IDC, EM_GETMODIFY,0,0)) {
		SendDlgItemMessage(hDlg, IDC, EM_SETMODIFY, FALSE, 0 );
		GetDlgItemText(hDlg, IDC, TMP, 100);
		p = TMP; if (*p==0) goto err;
		while(*p && *p!=' ') {
			if (!isdigit(*p) && *p!='.' && *p!='e' && *p!='E' && *p!='-' && *p!='+') goto err;
			p++;
		}
		*result = atof(TMP);
		SetDlgItemDouble( hDlg, IDC, *result, w );
		return TRUE;
	}
	return FALSE;
err:
	SetDlgItemDouble( hDlg, IDC, *result, w );
	return FALSE;
}
/***************************************************************************/
/* UpdateSliderCtl *********************************************************/
/***************************************************************************/
VOID WINAPI UpdateSliderCtl( HWND hWnd, WPARAM wParam, LPARAM lParam, LPSCROLLBAR s)
{
register int pos;
//RECT	r;

	pos = s->p;					/* Prev position	*/
	switch( LOWORD(wParam) )
	{
	  case TB_THUMBTRACK   :
	  case TB_THUMBPOSITION: pos = HIWORD(wParam) + s->mi;  break;
      case TB_LINEDOWN     : pos += s->st;					break;
      case TB_LINEUP       : pos -= s->st;					break;
      case TB_PAGEDOWN     : pos += s->pg;   				break;
      case TB_PAGEUP       : pos -= s->pg;   				break;
      case TB_TOP          : pos =  s->mi;   				break;
      case TB_BOTTOM	   : pos =  s->ma; 					break;
    }
//	if (LOWORD(wParam) != TB_ENDTRACK && LOWORD(wParam)!=0xFFFF) {
//		GetWindowRect( (HWND)lParam, &r );
//		ClipCursor(&r);
//	} else ClipCursor( NULL );

    if(pos < s->mi) pos = s->mi; if(pos >= s->ma) pos = s->ma;
	SendMessage((HWND)lParam, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)(pos - s->mi));
	s->p = pos;
	if ( s->idc ) SetDlgItemInt( hWnd, s->idc, s->p, TRUE);
}
/****************************************************************************/
/* InitScrollCtl ************************************************************/
/****************************************************************************/
VOID WINAPI InitSliderCtl( HWND hWnd, int id, LPSCROLLBAR s)
{
HWND	h;

	h = GetDlgItem( hWnd, id );
	SendMessage( h, TBM_SETRANGE, (WPARAM)FALSE, (LPARAM)MAKELONG(0, s->ma-s->mi));
	SendMessage( h, TBM_SETPOS,   (WPARAM)TRUE,  (LPARAM)(s->p-s->mi));
	if ( s->idc ) SetDlgItemInt( hWnd, s->idc, s->p, TRUE);
}
/***************************************************************************/
/* UpdateScrollCtl *********************************************************/
/***************************************************************************/
VOID WINAPI UpdateScrollCtl( HWND hWnd, WPARAM wParam, LPARAM lParam, LPSCROLLBAR s)
{
register int pos;
RECT	r;

	pos = s->p;					/* Prev position	*/
	switch( LOWORD(wParam) )
	{
	  case SB_THUMBTRACK   :
	  case SB_THUMBPOSITION: pos = HIWORD(wParam) + s->mi;  break;
      case SB_LINEDOWN     : pos += s->st;					break;
      case SB_LINEUP       : pos -= s->st;					break;
      case SB_PAGEDOWN     : pos += s->pg;   				break;
      case SB_PAGEUP       : pos -= s->pg;   				break;
      case SB_TOP          : pos =  s->mi;   				break;
      case SB_BOTTOM	   : pos =  s->ma; 					break;
    }
	if (LOWORD(wParam) != SB_ENDSCROLL && LOWORD(wParam)!=0xFFFF) {
		GetWindowRect( (HWND)lParam, &r );
		ClipCursor(&r);
	} else ClipCursor( NULL );

    if(pos < s->mi) pos = s->mi; if(pos >= s->ma) pos = s->ma;
	SetScrollPos((HWND)lParam, SB_CTL, pos - s->mi, 1);
	s->p = pos;
	if ( s->idc ) SetDlgItemInt( hWnd, s->idc, s->p, TRUE);
}
/****************************************************************************/
/* InitScrollCtl ************************************************************/
/****************************************************************************/
VOID WINAPI InitScrollCtl( HWND hWnd, int id, LPSCROLLBAR s)
{
HWND	h;

	h = GetDlgItem( hWnd, id );
	SetScrollRange( h, SB_CTL, 0, s->ma - s->mi, 0);
	SetScrollPos  ( h, SB_CTL, s->p - s->mi, 1);
	if ( s->idc ) SetDlgItemInt( hWnd, s->idc, s->p, TRUE);
}
/****************************************************************************/
/****************************************************************************/
#define CLIENTSTRIP WS_MINIMIZE|WS_MAXIMIZE|WS_CAPTION|WS_BORDER \
					|WS_DLGFRAME|WS_SYSMENU|WS_POPUP|WS_THICKFRAME|DS_MODALFRAME


typedef struct {
	WORD   dlgVer;
	WORD   signature;
	DWORD  helpID;
	DWORD  exStyle;
	DWORD  style;
	WORD   cDlgItems;
	short  x;
	short  y;
	short  cx;
	short  cy;
//	sz_Or_Ord menu;         // name or ordinal of a menu resource
//	sz_Or_Ord windowClass;  // name or ordinal of a window class
//	WCHAR  title[titleLen]; // title string of the dialog box
//	short  pointsize;       // only if DS_SETFONT flag is set
//	short  weight;          // only if DS_SETFONT flag is set
//	short  bItalic;         // only if DS_SETFONT flag is set
//	WCHAR  font[fontLen];   // typeface name, if DS_SETFONT is set
} DLGTEMPLATEEX, *LP_DLGTEMPLATEEX;

typedef DLGTEMPLATE *LP_DLGTEMPLATE;
/****************************************************************************/
/****************************************************************************/
HWND WINAPI wndCreateClientControls(char *pTemplateName, HINSTANCE hInst, DLGPROC lpNew, HWND hWparent)
{
RECT			rCli,rPar,rDlg;
int				dxDlg,dyDlg,dyExtra,dtXold,dtYold;
HRSRC			hRes;
HGLOBAL			hMem;
LP_DLGTEMPLATE	lpDlg;
DWORD			styleold,style;
HWND			hNew;

	if (!IsWindow(hWparent))
		return 0;
	if (IsZoomed(hWparent))
		ShowWindow(hWparent,SW_RESTORE);

	/* Get access to data structure of dialog box containing layout of controls */
	if (!(hRes=FindResource(hInst,(LPSTR)pTemplateName,RT_DIALOG)))
		return 0;
	if (!(hMem=LoadResource(hInst,hRes)))
		return 0;
	if (!(lpDlg=(LP_DLGTEMPLATE)LockResource(hMem)))
		return 0;

    /* Change dialog box data structure so it can be used as a window in client area */
	styleold     = lpDlg->style;
	style        = lpDlg->style&(CLIENTSTRIP);
	lpDlg->style = lpDlg->style^style;
	lpDlg->style = lpDlg->style | WS_CHILD | WS_CLIPSIBLINGS;
	dtXold       = lpDlg->x;
	dtYold       = lpDlg->y;
	lpDlg->x     = 0;
	lpDlg->y     = 0;

	if (!(hNew = CreateDialogIndirectParam(hInst, lpDlg, hWparent, lpNew, (LPARAM)hWparent)))
		return 0;

    /* Restore dialog box data structure. */
	lpDlg->style =styleold;
	lpDlg->x     = dtXold;
	lpDlg->y     = dtYold;

	UnlockResource(hMem);
	FreeResource(hMem);

    /* Move and size window in client area and main window */
	GetClientRect(hWparent,&rCli);
	GetWindowRect(hWparent,&rPar);
	MapWindowPoints( NULL, GetParent(hWparent), (LPPOINT)&rPar, 2 );
	GetWindowRect(hNew,&rDlg);
	dxDlg=(rDlg.right -rDlg.left)-(rCli.right -rCli.left);
	dyDlg=(rDlg.bottom-rDlg.top) -(rCli.bottom-rCli.top);
/*
	MYMoveWindow(hWparent, rPar.left, rPar.top,
				 (rPar.right -rPar.left)+dxDlg,
				 (rPar.bottom-rPar.top) +dyDlg,
				 TRUE);
*/
	MoveWindow(hNew,0,0,
                     (rDlg.right -rDlg.left),
                     (rDlg.bottom-rDlg.top),
                     TRUE);
    GetClientRect(hWparent,&rCli);

    /* Compensate size if menu bar is more than one line. */
	if ((rDlg.bottom-rDlg.top)>(rCli.bottom-rCli.top))
	{
		dyExtra=(rDlg.bottom-rDlg.top)-(rCli.bottom-rCli.top);
/*
		MYMoveWindow(hWparent,rPar.left,rPar.top,
                   (rPar.right -rPar.left)+dxDlg,
                   (rPar.bottom-rPar.top) +dyDlg + dyExtra,
                   TRUE);
*/
	}

	ShowWindow(hNew,SW_SHOW);
	return hNew;
}
/****************************************************************************/
VOID WINAPI wndChangeDlgSize(HWND hWnd, HWND hDlg, int idw, int idh, LPSTR lpName, HINSTANCE hInst)
{
HGLOBAL			hMem;
HRSRC			hRes;
static LP_DLGTEMPLATE		lpDlg;
static LP_DLGTEMPLATEEX	lpDlgEx;
RECT			rw, rh, rf;
int				xs, ys, oxs, oys;


	GetWindowRect(hWnd, (LPRECT)&rf);
	oxs = rf.right - rf.left;
	oys = rf.bottom - rf.top;

	if (lpName == NULL) return;	// To set full size -- Name must be specified
	// Get access to data structure of dialog box containing layout of controls
	if (!(hRes=FindResource(hInst,lpName,RT_DIALOG)))	return;
	if (!(hMem=LoadResource(hInst,hRes)))				return;
	if (!(lpDlg=(LP_DLGTEMPLATE)LockResource(hMem)))	return;
	lpDlgEx = (LP_DLGTEMPLATEEX)lpDlg;
	if (lpDlgEx->dlgVer==1 && lpDlgEx->signature==0xFFFF) {
		rh.left = rh.top = 0;
		rh.right  = lpDlgEx->cx;
		rh.bottom = lpDlgEx->cy;
	} else {
		rh.left = rh.top = 0;
		rh.right  = lpDlg->cx;
		rh.bottom = lpDlg->cy;
	}
	UnlockResource(hMem);
	FreeResource(hMem);
	MapDialogRect( hDlg, &rh );
	AdjustWindowRectEx(&rh, GetWindowLong(hWnd, GWL_STYLE), FALSE, GetWindowLong(hWnd, GWL_EXSTYLE));

	xs = rh.right-rh.left; ys = rh.bottom-rh.top;

	if (idw) {
		GetWindowRect(GetDlgItem(hDlg, idw), (LPRECT)&rw);
		xs = rw.left - rf.left;// + GetSystemMetrics(SM_CXDLGFRAME)*2;
	}
	if(idh) {
		GetWindowRect(GetDlgItem(hDlg, idh), (LPRECT)&rh);
		ys = rh.top - rf.top;
	}
	if ((oxs != xs) || (oys != ys))
	SetWindowPos(hWnd, (HWND)NULL, 0, 0, xs, ys,
					SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOZORDER);
}
