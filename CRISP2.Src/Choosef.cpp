/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* Main File Open Dialog * by ML ********************************************/
/****************************************************************************/

#include "StdAfx.h"

//#include <dlgs.h>

#include "objects.h"
#include "shtools.h"
#include "const.h"
#include "commondef.h"
#include "common.h"
#include "globals.h"
#include "rgconfig.h"

// Added by Peter on 22 April 2008

enum { ID_LISTVIEW = lst2 };

// reverse-engineered command codes for SHELLDLL_DefView
enum LISTVIEWCMD
{
	ODM_VIEW_ICONS = 0x7029,
	ODM_VIEW_LIST  = 0x702b,
	ODM_VIEW_DETAIL= 0x702c,
	ODM_VIEW_THUMBS= 0x702d,
	ODM_VIEW_TILES = 0x702e,
};

// self-initialization message posted
const int MYWM_POSTINIT = WM_USER+1;

// End of Peters addition on 22 April 2008

static	char	szFileName [_MAX_FNAME] = "";		// Full file name
static	char	szFileTitle[_MAX_FNAME] = "";		// File title
/*--------------------------------------------------------------------------*/
static	char 	szOFNpos[] = "OFNpos";

// Added by Peter on 22 April 2008
static BOOL SetListView(HWND hDlg, LISTVIEWCMD cmd)
{
   // note that real dialog is my parent, not me
	HWND hShell = GetDlgItem(GetParent(hDlg), lst2);
	if( hShell )
	{
		//TRACE(_T("hwnd=%p.\n"), m_wndList.GetSafeHwnd());
		SendMessage(hShell, WM_COMMAND, cmd, 0);
		return TRUE;
	}
//	TRACE(_T("failed.\n"),m_wndList.GetSafeHwnd());
	return FALSE;
}
// End of Peters addition on 22 April 2008

/****************************************************************************/
UINT APIENTRY GFNDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			HANDLE image = LoadImage(hInst, "CALIDRIS", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			if (image == NULL)
			{
				Trace(_FMT(_T("Failed to load icon CALIDRIS: %s"), GetLastErrorString().c_str()));
			}
			else
			{
				SendMessage(GetParent(hDlg), WM_SETICON, (WPARAM)FALSE,	(LPARAM)image);
			}
			PostMessage(hDlg, MYWM_POSTINIT, 0, 0);
		}
		return TRUE;
	case WM_NOTIFY:
		switch(((LPOFNOTIFY)lParam)->hdr.code)
		{
		case CDN_INITDONE:
			regReadWindowPos (GetParent(hDlg), szOFNpos);
			break;
		case CDN_FILEOK:
			regWriteWindowPos(GetParent(hDlg), szOFNpos, FALSE);
			break;
		}
		break;
	// Added by Peter on 22 April 2008
	case MYWM_POSTINIT:
		SetListView(hDlg, /*ODM_VIEW_DETAIL*/ODM_VIEW_THUMBS); // this will succeed
		break;
	// End of Peters addition on 22 April 2008
	}	
	return FALSE;
}
/****************************************************************************/
BOOL fioGetFileNameDialog( HWND hWnd, LPSTR szDlgName, LPOFNAMES lpOF, const TCHAR* szDefFile, BOOL save, UINT nFIndex)
{
	BOOL			ret;
	OPENFILENAME	ofn;

	if (strlen(szDefFile))	strcpy(szFileName, szDefFile);
	else					strcpy(szFileName, lpOF->szDefFile);

	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = lpOF->szFilter;
	ofn.nFilterIndex = nFIndex;
	ofn.hInstance = NULL;

	ofn.lpstrFile= szFileName;
	ofn.nMaxFile = sizeof(szFileName);

	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = lpOF->szDefDir;
	ofn.lpstrTitle = szDlgName;				// window caption
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK
				| OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLESIZING/* | OFN_ALLOWMULTISELECT*/;

	// Added by Peter on 12 April 2007
	ofn.lpstrDefExt = NULL;//lpOF->szDefExt;
	// End of addition by Peter on 12 April 2007

	ofn.lpfnHook	= GFNDlgProc;

	if (save) ret = GetSaveFileName(&ofn);
	else	  ret = GetOpenFileName(&ofn);

	if( ret )
	{
		Trace(_FMT(_T("fioGetFileNameDialog() -> files '%s', 0x%08X"), ofn.lpstrFile, (LPVOID)ofn.lpstrFile));
		// If new file selected
		strcpy( lpOF->szDefDir, szFileName );		// Copy the current path name
		lpOF->szDefDir[ofn.nFileOffset-1] = 0;		// Cut file name
		strcpy(lpOF->szDefFile, ofn.lpstrFileTitle);
	}
	else
	{
		DWORD dwError = CommDlgExtendedError();
		if( FNERR_BUFFERTOOSMALL == dwError )
		{
		}
	}

    return ret;
}
/****************************************************************************/
LPSTR fioFileName( VOID )
{
	return szFileName;
}
/****************************************************************************/
LPSTR fioFileTitle( VOID )
{
	return szFileTitle;
}
