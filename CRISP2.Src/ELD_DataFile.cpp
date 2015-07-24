/****************************************************************************/
/* Copyright (c) 1998 Calidris **********************************************/
/****************************************************************************/
/* CRISP2.ELD ***************************************************************/
/****************************************************************************/

#include "StdAfx.h"

//#include "SpinStar/typedefs.h"
#include "typedefs.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "fft.h"
#include "ft32util.h"
#include "lattice.h"
#include "xutil.h"
#include "rgconfig.h"

// Added by Peter on 23 November 2004
#include "Crash.h"
#include "HelperFn.h"

#include "OXParser.h"

#pragma warning(disable:4305)

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

#include "eld.h"
#include "xCBN\shcb.h"

#include "ELD_HOLZ.h"

using namespace std;

extern BOOL CRISP_ReadImage(OBJ* pObj, vObjectsVec &vObjects);

BOOL ELD_ParseRoot(OBJ* pObj, COXParserElement *pRoot, BOOL bParseImageOnly = TRUE);

#define TMP_End (&TMP[strlen(TMP)])

COXParserElement *FindElement(COXParserElement *pRoot, LPCTSTR pszText)
{
	COXParserObject* pObject = NULL;
	COXParserElement *pElement = NULL;
	for(int i = 0; i < pRoot->NumObjects(); i++)
	{
		pObject = pRoot->Object(i);
		if( 0 == _tcsicmp(pszText, pObject->GetText()) )
		{
			pElement = (COXParserElement*)pObject;
			break;
		}
	}
	return pElement;
}

BOOL ProcessReflection(LaueZone *pLZ, int nReflN, COXParserElement *pBase)
{
	COXAttribute *pAttr;

	if( NULL == pBase )
	{
		// TODO: report the error
		return FALSE;
	}
	if( NULL == (pAttr = pBase->FindAttribute("h")) )
	{
		return FALSE;
	}
	pLZ->h[nReflN] = atoi(pAttr->GetStringValue());
	if( NULL == (pAttr = pBase->FindAttribute("k")) )
	{
		return FALSE;
	}
	pLZ->k[nReflN] = atoi(pAttr->GetStringValue());
	if( NULL == (pAttr = pBase->FindAttribute("x")) )
	{
		return FALSE;
	}
	pLZ->x[nReflN] = strtod(pAttr->GetStringValue(), NULL);
	if( NULL == (pAttr = pBase->FindAttribute("y")) )
	{
		return FALSE;
	}
	pLZ->y[nReflN] = strtod(pAttr->GetStringValue(), NULL);
	return TRUE;
}

BOOL ProcessAxis(int nAxis, /*LPELD pEld, */LaueZone *pLZ, COXParserElement *pBase)
{
	COXAttribute *pAttr;
	CVector2d *pAxis = &pLZ->h_dir;
	if( 0 == nAxis )
	{
		*pAxis = CVector2d(10, 0);
	}
	else if( 1 == nAxis )
	{
		pAxis = &pLZ->k_dir;
		*pAxis = CVector2d(0, 10);
	}
	if( NULL == pBase )
	{
		// TODO: report the error
		return FALSE;
	}
	if( NULL == (pAttr = pBase->FindAttribute("x")) )
	{
		return FALSE;
	}
	pAxis->x = strtod(pAttr->GetStringValue(), NULL);
	if( NULL == (pAttr = pBase->FindAttribute("y")) )
	{
		return FALSE;
	}
	pAxis->y = strtod(pAttr->GetStringValue(), NULL);
	return TRUE;
}

BOOL ProcessCentre(LPELD pEld, LaueZone *pLZ, COXParserElement *pBase)
{
	COXAttribute *pAttr;

	if( NULL == pBase )
	{
		// TODO: report the error
		return FALSE;
	}
	if( NULL == (pAttr = pBase->FindAttribute("x")) )
	{
		return FALSE;
	}
	pEld->m_center.x = (pLZ->center.x = strtod(pAttr->GetStringValue(), NULL));
	if( NULL == (pAttr = pBase->FindAttribute("y")) )
	{
		return FALSE;
	}
	pEld->m_center.y = (pLZ->center.y = strtod(pAttr->GetStringValue(), NULL));
	return TRUE;
}

BOOL ProcessCell(LPELD pEld, LaueZone *pLZ, COXParserElement *pBase)
{
	COXAttribute *pAttr;

	if( NULL == pBase )
	{
		// TODO: report the error
		return FALSE;
	}
	if( NULL == (pAttr = pBase->FindAttribute("a")) )
	{
		return FALSE;
	}
	pLZ->a_length = strtod(pAttr->GetStringValue(), NULL);
	if( NULL == (pAttr = pBase->FindAttribute("b")) )
	{
		return FALSE;
	}
	pLZ->b_length = strtod(pAttr->GetStringValue(), NULL);
	if( NULL == (pAttr = pBase->FindAttribute("c")) )
	{
		return FALSE;
	}
	pLZ->c_length = strtod(pAttr->GetStringValue(), NULL);
	if( NULL == (pAttr = pBase->FindAttribute("gamma")) )
	{
		return FALSE;
	}
	pLZ->gamma = strtod(pAttr->GetStringValue(), NULL);
	return TRUE;
}

LPELD FindEld(OBJ* pObj)
{
	LPELD pEld = NULL;
	if( NULL == pObj )
	{
		return NULL;
	}
	POSITION pos = pObj->FindFirstChild(OT_ELD);
	OBJ* pTObj = pObj->FindNextChild(pos, OT_ELD);
	if( NULL != pTObj )
	{
		return (LPELD)pTObj->dummy;
	}
//	for(int i = 0; i < MAXCHI; i++)
//	{
//		if( NULL != pObj->Chi[i] )
//		{
//			OBJ* pTObj = GetOBJ(pObj->Chi[i]);
//			if( (NULL != pTObj) && (OT_ELD == pTObj->ID) )
//			{
//				return (LPELD)pTObj->dummy;
//			}
//		}
//	}
	return NULL;
}

BOOL ProcessLaueZone(LPELD pEld, COXParserElement *pBase, int nLaueIndex, BOOL bAuto = TRUE)
{
	COXParserElement *pElement;
	if( NULL == pEld )
	{
		return FALSE;
	}
	if( (nLaueIndex > 0) && (nLaueIndex < ELD_MAX_LAUE_CIRCLES) )
	{
		// TODO: report the error
		return FALSE;
	}
	LaueZone *pLZ = pEld->m_vLaueZones[nLaueIndex];
	if( NULL == pLZ )
	{
		return FALSE;
	}
	pElement = FindElement(pBase, _T("refl1"));
	if( NULL != pElement )
	{
		ProcessReflection(pLZ, 0, pElement);
	}
	pElement = FindElement(pBase, _T("refl2"));
	if( NULL != pElement )
	{
		ProcessReflection(pLZ, 1, pElement);
	}
	pElement = FindElement(pBase, _T("refl3"));
	if( NULL != pElement )
	{
		ProcessReflection(pLZ, 2, pElement);
	}
	if( NULL != (pElement = FindElement(pBase, _T("h_axis"))) )
	{
		ProcessAxis(0, /*pEld, */pLZ, pElement);
	}
	if( NULL != (pElement = FindElement(pBase, _T("k_axis"))) )
	{
		ProcessAxis(1, /*pEld, */pLZ, pElement);
	}
	if( NULL != (pElement = FindElement(pBase, _T("center"))) )
	{
		ProcessCentre(pEld, pLZ, pElement);
	}
	if( NULL != (pElement = FindElement(pBase, _T("cell"))) )
	{
		ProcessCell(pEld, pLZ, pElement);
	}
	// set Auto flag
	pLZ->bAuto = bAuto;
	return TRUE;
}

BOOL ProcessIndexing(OBJ* pObj, COXParserElement *pBase)
{
	BOOL bRet = FALSE;
	COXAttribute *pAttr;
	COXParserObject *pObject;
	COXParserElement *pElement;
	int i, nLaueIndex;
	LPELD pEld = FindEld(pObj);

	if( NULL == pEld )
	{
		return FALSE;
	}
	// TODO: go over all objects and check each for the "LaueZone"
	for(i = 0; i < pBase->NumObjects(); i++)
	{
		pObject = pBase->Object(i);
		if( 0 == _tcsicmp(_T("LaueZone"), pObject->GetText()) )
		{
			pElement = (COXParserElement*)pObject;
			if( NULL != pElement )
			{
				BOOL bAuto = TRUE;
				pAttr = pElement->FindAttribute("auto");
				if( NULL != pAttr )
				{
					if( 0 == _tcsicmp(pAttr->GetStringValue(), _T("true")) )
					{
						bAuto = TRUE;
					}
					else if( 0 == _tcsicmp(pAttr->GetStringValue(), _T("false")) )
					{
						bAuto = FALSE;
					}
				}
				pAttr = pElement->FindAttribute("index");
				if( NULL != pAttr )
				{
					nLaueIndex = pAttr->GetIntValue();
				}
				bRet = ProcessLaueZone(pEld, pElement, nLaueIndex, bAuto);
			}
		}
	}
	extern BOOL InitELDDialog(HWND hDlg, OBJ* O, LPELD lpEld);
	extern void EnableRefine(OBJ* pObj, LaueZone *pLZ);
	extern void DrawELD(OBJ* pObj);
	extern void RepaintWindow(HWND hWnd);

	InitELDDialog( pEld->m_pObj->hWnd, pEld->m_pObj, pEld );
	EnableWindow(GetDlgItem(pObj->hWnd, IDC_ELD_EXTRACT), FALSE);
	EnableRefine( pEld->m_pObj, pEld->m_pCurLZ );
	RepaintWindow(pObj->hWnd);
	DrawELD(pEld->m_pObj);
	return bRet;
}

BOOL PreActivateELDObject(OBJ* pObj, LPVOID pExtraData)
{
	COXParser parser;
	COXParserElement *pRoot;
	BOOL bRet = FALSE;

	::SendMessage(MainhWnd, WM_COMMAND, MAKEWPARAM(IDM_E_ELD, 0), 0);

	if( NULL == pObj )
	{
		goto Error_Exit;
	}
	if( FALSE == parser.ParseFile(pObj->extra) )
	{
		goto Error_Exit;
	}
	pRoot = parser.Root();
	if( pRoot )
	{
		bRet = ELD_ParseRoot(pObj, pRoot, FALSE);
	}
Error_Exit:
	return bRet;
}

BOOL ProcessImagePath(OBJ* pObj, COXParserElement *pRoot)
{
	char drv[_MAX_DRIVE], dir[_MAX_DIR];
	COXParserObject* pObject;
	vObjectsVec vObjects;
	int i;

	for(i = 0; i < pRoot->NumObjects(); i++)
	{
		pObject = pRoot->Object(i);
		if( !pObject ) continue;
		_splitpath(pObj->fname, drv, dir, NULL, NULL);
		_makepath(pObj->fname, drv, dir, NULL, NULL);
		// remember the path
		strcpy(pObj->title, pObject->GetText());
		strcat(pObj->fname, pObj->title);
		BOOL bRet = CRISP_ReadImage(pObj, vObjects);
		if( TRUE == bRet )
		{
			pObj->pfnPreActivate = &PreActivateELDObject;
			return bRet;
		}
		else
		{
			MessageBox(NULL, _T("Cannot open IMAGE file!"), _T("ELD error"), MB_OK);
			break;
		}
	}
	return FALSE;
}

BOOL ELD_ParseRoot(OBJ* pObj, COXParserElement *pRoot, BOOL bParseImageOnly /*= TRUE*/)
{
	BOOL bRet = FALSE;
	if( NULL == pRoot )
	{
		return FALSE;
	}
	try
	{
		COXParserElement *pELD = FindElement(pRoot, _T("ELD"));
		COXParserElement *pElement;
		if( NULL != pELD )
		{
			if( TRUE == bParseImageOnly )
			{
				pElement = FindElement(pELD, _T("ImagePath"));
				if( NULL != pElement )
					bRet = ProcessImagePath(pObj, pElement);
			}
			else
			{
				// ---
				pElement = FindElement(pELD, _T("Indexing"));
				if( NULL != pElement )
					bRet = ProcessIndexing(pObj, pElement);
			}
		}
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("ELD_ParseRoot() -> %s"), e.what()));
	}
	return bRet;
}

BOOL ELD_ReadEldData(OBJ* pObj)
{
	BOOL bRet = FALSE;
	COXParser parser;
	COXParserElement *pRoot;

	if( NULL == pObj )
	{
		goto Error_Exit;
	}
	if( FALSE == parser.ParseFile(pObj->extra) )
	{
		goto Error_Exit;
	}
	pRoot = parser.Root();
	if( pRoot )
	{
		bRet = ELD_ParseRoot(pObj, pRoot, TRUE);
	}
Error_Exit:
	return bRet;
}
