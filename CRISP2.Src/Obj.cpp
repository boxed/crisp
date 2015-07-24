/****************************************************************************/
/* Copyright (c) 2006 Calidris **********************************************/
/****************************************************************************/
/* Written by Peter Oleynikov  **********************************************/
/****************************************************************************/
/* CRISP2.OBJ ***************************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "objects.h"
#include "InfoWindow.h"
#include "globals.h"

using namespace std;

class ObjectHolder
{
	ObjectHolder() {}
	ObjectHolder(const ObjectHolder &) {}
	ObjectHolder& operator = (const ObjectHolder &) {}
public:
	typedef std::map<HWND, OBJ*> ObjectMap;
	typedef ObjectMap::iterator ObjectMapIt;

	~ObjectHolder()
	{
// 		Map().clear();
	}
	static ObjectHolder &instance()
	{
		static ObjectHolder _ObjectHolder;
		return _ObjectHolder;
	}
	ObjectMap &Map()
	{
		static ObjectMap m_map;
		return m_map;
	}
};

// OBJ* GetOBJ(HWND hWnd)
// {
// 	return ((OBJ*)GetWindowLong( hWnd, GWL_USERDATA));
// }

// void SetOBJ(HWND hWnd, OBJ* pObj)
// {
// 	SetWindowLong(hWnd, GWL_USERDATA, (LONG)pObj);
// }

OBJ* OBJ::GetOBJ(HWND hWnd)
{
	OBJ* pObj = NULL;
	ObjectHolder::ObjectMapIt it = ObjectHolder::instance().Map().find(hWnd);
	if( it != ObjectHolder::instance().Map().end() )
	{
		pObj = it->second;
	}
//	return it->second;
//	pObj = (OBJ*)::GetWindowLong( hWnd, GWL_USERDATA);
	return pObj;
}

void OBJ::SetOBJ(HWND hWnd, OBJ* pObj)
{
	ObjectHolder::instance().Map()[hWnd] = pObj;
//	SetWindowLong(hWnd, GWL_USERDATA, (LONG)pObj);
}

OBJ::OBJ()
{
	ID = 0;
	ParentWnd = NULL;				// myParent HWND
	hWnd = NULL;				// Self hWnd
	hTree = NULL;			// Handle in Navigator window
	bDialog = FALSE;			// This object is a dialog

	// consider a BYTE image
	npix = PIX_BYTE;
	
	imin = imina = 0;
	imax = imaxa = 255;

	maxcontr = TRUE;

	bright = contrast = 128;

	palno = -1;

	pfnPreActivate = NULL;

	hbit = NULL;				// BITMAP handle
	bp = NULL;					// BitMap mem ptr
	dp = NULL;					// Data mem ptr
	dp000 = NULL;				// Original Data mem ptr, used while grabbing
	ebp = NULL;				// Edit BitMap mem ptr
	edp = NULL;				// Edit Data mem ptr
	hBme = NULL;				// Edit BITMAP handle

	title[0] = 0;	// Image window title
	fname[0] = 0;	// Image file path
	extra[0] = 0;	// extra file path
	sysmenu = NULL;			// System menu copy
	hDlg = NULL;				// Dialog handle
	hBigPlot = NULL;			// Big Plot window
	PlotPaint = NULL;	// Plot paint function

	ZeroMemory(&dummy, sizeof(dummy));
	m_vChildren.reserve(1000);
	infoWindows = NULL;
}

OBJ::~OBJ()
{
	m_vChildren.clear();
	if (infoWindows != NULL)
	{
		for (InfoWindows::iterator i = infoWindows->begin(); i != infoWindows->end(); i++)
		{
			DestroyWindow(i->second);
		}
		delete infoWindows;
	}
}

OBJ &OBJ::operator = (const OBJ &object)
{
	ID = object.ID;					// Object type
	ParentWnd = object.ParentWnd;				// myParent HWND
	hWnd = object.hWnd;				// Self hWnd
	hTree = object.hTree;			// Handle in Navigator window
	bDialog = object.bDialog;			// This object is a dialog

	pfnPreActivate = object.pfnPreActivate;

// 	m_vChildren.resize(object.m_vChildren.size());
// 	std::copy(object.m_vChildren.begin(), object.m_vChildren.end(), m_vChildren.begin());

	x = object.x;
	y = object.y;					// Data size
	//	float	x, y;
	xw = object.xw;
	yw = object.yw;					// Window client area size
	//	float	xw, yw;				// Window client area size
	xo = object.xo;
	yo = object.yo;					// Window offsets
	xa = object.xa;
	ya = object.ya;					// Area position
	xas = object.xas;
	yas = object.yas;				// Area size
	//	float	xas,yas;			// Area size
	scd = object.scd;				// Scale down (zoom out)
	scu = object.scu;				// Scale up (zoom in)
#ifdef USE_XB_LENGTH
	xb = object.xb;					// Image buffer width
#else
	stride = object.stride;			// Image buffer width
#endif
	hbit = object.hbit;				// BITMAP handle
	bp = object.bp;					// BitMap mem ptr
	dp = object.dp;					// Data mem ptr
	dp000 = object.dp000;				// Original Data mem ptr, used while grabbing
	ebp = object.ebp;				// Edit BitMap mem ptr
	edp = object.edp;				// Edit Data mem ptr
	hBme = object.hBme;				// Edit BITMAP handle
	npix = object.npix;				// Pixel size in bytes
	imin = object.imin;
	imax = object.imax;				// 5% thresh. min/max intensity in image
	imina = object.imina;
	imaxa = object.imaxa;			// Absolute min/max intensity in image
	maxcontr = object.maxcontr;		// Maximize contrast
	flags = object.flags;			// Object flags

	strcpy(title, object.title);	// Image window title
	strcpy(fname, object.fname);	// Image file path
	strcpy(extra, object.extra);	// extra file path
	sysmenu = object.sysmenu;		// System menu copy
	hDlg = object.hDlg;				// Dialog handle
	hBigPlot = object.hBigPlot;		// Big Plot window
	PlotPaint = object.PlotPaint;	// Plot paint function
	bright = object.bright;			// Brightness (Gray mode)
	contrast = object.contrast;		// Contrast (Gray mode)
	palno = object.palno;			// Palette no
	memcpy(&NI, &object.NI, sizeof(NI));	// Project Info struct
	imageserial = object.imageserial;		// Image serial number
	clickpoint = object.clickpoint;	// Click point coordinates

	hCalcTask = object.hCalcTask;	// Calculation thread handle
	bDoItAgain = object.bDoItAgain;	// Something has been changed, so do it again

	ui = object.ui;					// Simple user int
	memcpy(dummy, object.dummy, sizeof(dummy));		// Dummy place for communication packages

	nMouseMessage = object.nMouseMessage;		// shows in case of image which message was for the mouse
	return *this;
}

POSITION OBJ::FindFirstChild(int nID)
{
	OBJ* pObj;
	POSITION pos = NULL;
	HwndVecCit cit;
	for(cit = m_vChildren.begin(); cit != m_vChildren.end(); ++cit)
	{
		// check the window Handle
		if( (NULL != *cit) && (TRUE == ::IsWindow(*cit)) )
		{
			pObj = GetOBJ(*cit);
			if( (NULL != pObj) && (nID == pObj->ID) )
			{
 				pos = (POSITION)*cit;
				break;
			}
		}
	}
	return pos;
}

OBJ *OBJ::FindNextChild(POSITION &CurPos, int nID)
{
	OBJ* pObj = NULL;
	if( NULL == CurPos )
	{
		return NULL;
	}
	HwndVecIt it = *(HwndVecIt*)&CurPos;
	HwndVecIt first = std::find(m_vChildren.begin(), m_vChildren.end(), *it);
// 	for( ;it < m_vChildren.end(); ++it)
	for( it = first; it < m_vChildren.end(); ++it )
	{
		if( (NULL != *it) && (NULL != ::IsWindow(*it)) )
		{
			pObj = GetOBJ(*(HwndVecIt)it);
			if( NULL != pObj && (nID == pObj->ID) )
			{
				it++;
 				CurPos = (POSITION)*it;
				break;
			}
			else
			{
				pObj = NULL;
				CurPos = NULL;
			}
		}
	}
/*
	if( NULL != CurPos )
	{
		pObj = GetOBJ(*(HwndVecIt)CurPos);
		if( (it >= m_vChildren.begin()) && (it < m_vChildren.end()) )
		{
			it++;
			CurPos = (POSITION)it;
		}
	}
	if( (HwndVecIt)CurPos == m_vChildren.end() )
	{
		CurPos = NULL;
	}
*/
	return pObj;
}

HwndVecIt OBJ::FindByWindow(HWND hWnd)
{
	HwndVecIt it = m_vChildren.end();

	if( (NULL != hWnd) && (TRUE == ::IsWindow(hWnd)) )
	{
		it = find_if(m_vChildren.begin(), m_vChildren.end(), bind2nd(equal_to<HWND>(), hWnd));
	}

	return it;
}

void OBJ::HandleWarningMessage(const TCHAR* inMessage, bool inShowMessage, int inCommandOnClick)
{
	if (inShowMessage)
	{
		AddWarningMessage(inMessage, inCommandOnClick);
	}
	else
	{
		RemoveWarningMessage(inMessage);
	}
}

void OBJ::RemoveWarningMessage(const TCHAR* inMessage)
{
	_tstd::string message = _FMT(_T("Warning: %s"), inMessage);
	if (GetInfoWindows().find(message) != GetInfoWindows().end())
	{
		HWND hwnd = GetInfoWindows()[message];
		GetInfoWindows().erase(GetInfoWindows().find(message));
		DestroyWindow(hwnd);
	}
}

void OBJ::AddWarningMessage(const TCHAR* inMessage, int inCommandOnClick)
{
	_tstd::string message = _FMT(_T("Warning: %s"), inMessage);
	if (GetInfoWindows().find(message) == GetInfoWindows().end())
	{
		GetInfoWindows()[message] = CreateInfoWindow(message.c_str(), inCommandOnClick, (GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) == WS_CHILD);
		UpdateInfoWindowsPos();
	}
}

void OBJ::UpdateInfoWindowsPos()
{
	RECT r;
	GetWindowRect(hWnd, &r);
	POINT p = {r.left, r.bottom};
	if (GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)
	{
		ScreenToClient(MDIhWnd, &p);
	}
	int lastBottom = p.y;
	for (InfoWindows::iterator i = GetInfoWindows().begin(); i != GetInfoWindows().end(); i++)
	{
		RECT infoRect;
		GetWindowRect(i->second, &infoRect);
		int height = 34;
		MoveWindow(i->second, p.x, lastBottom, r.right-r.left, height, TRUE);
		SetWindowPos(i->second, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		lastBottom += height;
	}
}

OBJ::InfoWindows& OBJ::GetInfoWindows()
{
	if (infoWindows == NULL)
	{
		infoWindows = new InfoWindows;
	}
	return *infoWindows;
}

OBJ* GetBaseObject(OBJ* inObject)
{
	if (inObject == NULL)
	{
		return NULL;
	}

	if (inObject->ID != OT_IMG && inObject->ParentWnd != NULL)
	{
		OBJ* obj = OBJ::GetOBJ(inObject->ParentWnd);
		if (obj != NULL)
		{
			return GetBaseObject(obj);
		}
	}

	return inObject;
}
