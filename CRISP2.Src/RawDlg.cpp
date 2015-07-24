// RawDlg.cpp : implementation file
//

#include "StdAfx.h"

#if defined USE_MFC

#include "stdafx.h"

#include "RawDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RawDlg dialog


RawDlg::RawDlg(CWnd* pParent /*=NULL*/)
	: CDialog(RawDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(RawDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void RawDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RawDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(RawDlg, CDialog)
	//{{AFX_MSG_MAP(RawDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RawDlg message handlers

#endif // _MFC_VER
