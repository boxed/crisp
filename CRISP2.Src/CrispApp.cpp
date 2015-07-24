// CrispApp.cpp: implementation of the CCrispApp class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#if defined USE_MFC

#include "resource.h"

#include "CrispApp.h"
#include "MainFrm.h"

#include "setinp.h"
#include "..\shark49x\pcidev.h"
#include "..\shark49x\shark4.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CCrispApp, CWinApp)
	//{{AFX_MSG_MAP(CCrispApp)
//	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
//	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
//	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
//	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCrispApp::CCrispApp()
{

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCrispApp object

CCrispApp theApp;

extern HWND MainhWnd;

BOOL CCrispApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CMainFrame *pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

//	m_pMainWnd = new CWnd;
//	m_pMainWnd->Attach(	MainhWnd );

/*
	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CNLfit2DDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CNLfit2DView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
*/
	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}
/*
// App command to run the dialog
void CCrispApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
*/

extern 	HINSTANCE hInst;
extern void LoadConfig();
extern BOOL	bShark4;

extern BOOL InitApplication(HINSTANCE hInst, HANDLE hPrev, int *pCmdShow, LPTSTR lpCmdLine);

BOOL CCrispApp::InitApplication() 
{
	hInst = ::AfxGetInstanceHandle();
	static	WORD ccww = 0x137F;
	int nCmdShow = 0;

	_asm {
		fldcw	[ccww];
	}
	LoadConfig();
	::InitApplication(hInst, NULL, &nCmdShow, ::GetCommandLine());

	if (bShark4)
	{
		siInitInputs(); 
		siSetSharkInput(); 
	}

	return CWinApp::InitApplication();
}

int CCrispApp::Run() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CWinApp::Run();
}

extern LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CCrispApp::PreTranslateMessage(MSG* pMsg) 
{
	::MainWndProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
	return TRUE;
//	return CWinApp::PreTranslateMessage(pMsg);
}

#endif // _MFC_VER
