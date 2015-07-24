#include "StdAfx.h"
#include <windows.h>

#include "testutils.h"
#include "eld.h"
#include "edrd.h"
#include "resource.h"
#include "globals.h"
#include "objects.h"
#include "common.h"

void RunMenuCommand(UINT cmd)
{
	::SendMessage(MainhWnd, WM_COMMAND, MAKEWPARAM(cmd, 0), 0);
}

static bool ValueClose(double inValue, double inTarget, double inTolerance)
{
	return inTarget+inTolerance > inValue && inTarget-inTolerance < inValue;
}

static void TestFailed(const TCHAR* inMessage)
{
	MessageBox(NULL, inMessage, _T("Test Failed"), MB_OK);
}

class TestFailedException : public std::exception
{
	typedef std::exception inherited;
public:
	TestFailedException(const TCHAR* inMessage)
		: inherited(inMessage)
	{
		Trace(Format("FAILED TEST: %1%") % what());
	}

	TestFailedException(const Format& inFormat)
		: inherited(inFormat.str().c_str())
	{
		Trace(Format("FAILED TEST: %1%") % what());
	}
};

static void TestRingCenter(TCHAR* inFilePath, double inExpectedX, double inExpectedY)
{
	OBJ* obj = _OpenShellFile(inFilePath);
	HWND eldForRingsWindow = test::OpenELDForRings();
	SendMessage(eldForRingsWindow, WM_COMMAND, MAKEWPARAM(IDC_EDRD_CREFINE, 0), 0);
	//BYTE_SIGNED.dm3, INT_SIGNED, INT_UNSIGNED, FLOAT_SIGNED, DOUBLE_SIGNED
	OBJ* pObj = OBJ::GetOBJ(eldForRingsWindow);
	EDRD* pEDRD = (LPEDRD) &pObj->dummy;
	if (!(ValueClose(pEDRD->xc, inExpectedX, 0.1) && ValueClose(pEDRD->yc, inExpectedY, 0.1)))
	{
		throw TestFailedException(Format("Found incorrect center on %1%") % inFilePath);
	}
	SendMessage(eldForRingsWindow, WM_CLOSE, 0, 0);
	Trace(Format("SUCCESS: found correct center on %1%") % inFilePath);
}

template <typename _Tx>
class CutImage
{
public:
	typedef void TReturn;
	void Handle(IMAGE& inImage)
	{
		Trace("data = [");
		_Tx* pDst = (_Tx*)inImage.pData;
		for(int y = 0; y < inImage.h; y++)
		{
			Trace("[");
			for(int x = 0; x < inImage.w; x++)
			{
				//if (pDst[x] > 1000)
				{
					//pDst[x] =pDst[x]/5; //log((double)pDst[x]);
					Trace(Format("%1%,") % pDst[x]);
				}
			}
			pDst = (_Tx*)(((LPBYTE)pDst) + inImage.stride);
			Trace("],");
		}
		Trace("]");
	}
};

void CtrlDown()
{
	INPUT inputs[1];
	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki.wVk = VK_CONTROL;
	inputs[0].ki.wScan = 0;
	inputs[0].ki.dwFlags = 0;
	inputs[0].ki.time = 0;
	inputs[0].ki.dwExtraInfo = 0;
	if (SendInput(1, inputs, sizeof(inputs)) == 0)
	{
		Trace("Failed to send input");
	}
}

void CtrlUp()
{
	INPUT inputs[1];
	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki.wVk = VK_CONTROL;
	inputs[0].ki.wScan = 0;
	inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
	inputs[0].ki.time = 0;
	inputs[0].ki.dwExtraInfo = 0;
	if (SendInput(1, inputs, sizeof(inputs)) == 0)
	{
		Trace("Failed to send input");
	}
}

void ClickOnImageAt(OBJ* obj, int x, int y)
{
	POINT point = {x, y};
	ClientToScreen(obj->hWnd, &point);

	double fScreenWidth    = ::GetSystemMetrics( SM_CXSCREEN )-1; 
  double fScreenHeight  = ::GetSystemMetrics( SM_CYSCREEN )-1; 
  double fx = point.x*(65535.0f/fScreenWidth);
  double fy = point.y*(65535.0f/fScreenHeight);

	INPUT inputs[1];
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dx = fx;
	inputs[0].mi.dy = fy;
	inputs[0].mi.mouseData = 0;
	inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_MOVE;
	inputs[0].mi.time = 0;
	inputs[0].mi.dwExtraInfo = 0;
	if (SendInput(1, inputs, sizeof(inputs)) == 0)
	{
		Trace("Failed to send input");
	}
}

void test::RunUnitTests()
{
	try
	{
		/*
		// ELD test 1
		OBJ* obj = _OpenShellFile("z:\\Calidris\\downloaded files\\Precession hk0.xif");
		HWND eldWindow = test::OpenELD();
		ELD* pEld = test::ELDFromWindow(eldWindow);
		pEld->m_pCurLZ[0].x[0] = 10.; 
		pEld->m_pCurLZ[0].y[0] = 10.;
		pEld->m_pCurLZ[0].x[1] = 10.; 
		pEld->m_pCurLZ[0].y[1] = 10.; 
		pEld->m_pCurLZ[0].x[2] = 10.; 
		pEld->m_pCurLZ[0].y[2] = 10.; 

		test::RunManualRefinement(
			eldWindow,
			195., 230., 10, 0,
			315., 246., 0, 10,
			270., 125., 15, 15);

		test::RunAutomaticRefinement(eldWindow);

		// TODO: validate output: center, a axis, b axis, successful shapefit on all good spots
		*/

		//OBJ* obj = _OpenShellFile("z:\\Desktop\\Crisp test images\\INT_SIGNED.dm3");
		/*TestRingCenter(_T("z:\\Desktop\\Crisp test images\\BYTE_SIGNED.dm3"),		457.933, 466.067);
		TestRingCenter(_T("z:\\Desktop\\Crisp test images\\BYTE_UNSIGNED.dm3"),		457.933, 466.067);
		TestRingCenter(_T("z:\\Desktop\\Crisp test images\\SHORT_UNSIGNED.dm3"),	457.933, 466.067);
		TestRingCenter(_T("z:\\Desktop\\Crisp test images\\SHORT_SIGNED.dm3"),		457.933, 466.067);
		TestRingCenter(_T("z:\\Desktop\\Crisp test images\\INT_UNSIGNED.dm3"),		457.933, 466.067);
		TestRingCenter(_T("z:\\Desktop\\Crisp test images\\INT_SIGNED.dm3"),		457.933, 466.067);
		TestRingCenter(_T("z:\\Desktop\\Crisp test images\\FLOAT_SIGNED.dm3"),		457.933, 466.067);
		TestRingCenter(_T("z:\\Desktop\\Crisp test images\\DOUBLE_SIGNED.dm3"),		457.933, 466.067);*/
		
		//OBJ* obj = _OpenShellFile("z:\\Desktop\\FFT-overflow.tif");
		//HWND eldWindow = test::OpenELD();
		//ELD* pEld = test::ELDFromWindow(eldWindow);
		//ProcessImage<CutImage>(pEld->m_image);
		//RunMenuCommand(IDM_A_AR1K);
		//RunMenuCommand(IDM_C_FFT);
		//RunMenuCommand(IDM_A_IFFT);

		/*OBJ* obj = _OpenShellFile("C:\\Program Files\\Calidris\\CRISP\\Sample Images\\ELD\\BANB55CA.PCX");
		RunMenuCommand(IDM_A_AR512);
		RunMenuCommand(IDM_C_FFT);
		RunMenuCommand(IDM_A_IFFT);
		RunMenuCommand(IDM_A_AR512);
		RunMenuCommand(IDM_C_FFT);*/

		/*
		// run elliptical distortion detection
		OBJ* obj = _OpenShellFile("C:\\Program Files\\Calidris\\CRISP\\Sample Images\\ELD\\Al-rings.tif");
		HWND wnd = test::OpenELDForRings();
		HWND b = GetDlgItem(wnd, IDC_EDRD_ELLIPTICAL_DISTORTION);
		Button_SetCheck(b, BST_CHECKED);
		
		CtrlDown();
		ClickOnImageAt(obj, 383, 192);
		CtrlUp();
		*/
	}
	catch (TestFailedException& x)
	{
		TestFailed(x.what());
	}
}

ELD* test::ELDFromWindow(HWND eldWindow)
{
	OBJ* eldObj = OBJ::GetOBJ(eldWindow);
	if (eldObj == NULL)
	{
		MessageBox(NULL, "Failed to get ELD from window", "Test Error", MB_OK);
	}
	return (LPELD) &eldObj->dummy;
}

HWND test::OpenELD()
{
	RunMenuCommand(IDM_E_ELD);
	return GetWindow(MDIhWnd, GW_CHILD);
}

HWND test::OpenELDForRings()
{
	RunMenuCommand(IDM_E_EDRD);
	return GetWindow(MDIhWnd, GW_CHILD);
}

void test::RunAutomaticRefinement(HWND eldWindow)
{
	CheckDlgButton(eldWindow, IDC_ELD_AUTO, TRUE);

	SendMessage(eldWindow, WM_COMMAND, MAKEWPARAM(IDC_ELD_REFINE, 0), 0);
}

void test::RunManualRefinement(
	HWND eldWindow,
	double x1, double y1,
	int h1, int k1,
	double x2, double y2,
	int h2, int k2,
	double x3, double y3,
	int h3, int k3)
{
	ELD* pEld = ELDFromWindow(eldWindow);
	Trace(Format("test::RunManualRefinement %1%") % (int)pEld);

	pEld->m_pCurLZ[0].x[0] = x1; pEld->m_pCurLZ[0].y[0] = y1;
	pEld->m_pCurLZ[0].x[1] = x2; pEld->m_pCurLZ[0].y[1] = y2; 
	pEld->m_pCurLZ[0].x[2] = x3; pEld->m_pCurLZ[0].y[2] = y3; 

	SetDlgItemInt(eldWindow, IDC_ELD_H1, h1, TRUE ); SetDlgItemInt(eldWindow, IDC_ELD_K1, k1, TRUE );
	SetDlgItemInt(eldWindow, IDC_ELD_H2, h2, TRUE ); SetDlgItemInt(eldWindow, IDC_ELD_K2, k2, TRUE );
	SetDlgItemInt(eldWindow, IDC_ELD_H3, h3, TRUE ); SetDlgItemInt(eldWindow, IDC_ELD_K3, k3, TRUE );

	CheckDlgButton(eldWindow, IDC_ELD_AUTO, FALSE);

	SendMessage(eldWindow, WM_COMMAND, MAKEWPARAM(IDC_ELD_REFINE, 0), 0);
}