// SymmListBox.cpp : implementation file
//

#include "stdafx.h"

#include "Spinningstar/SpaceGroups.h"
#include "Spinningstar/SpinningStar.h"
#include "Spinningstar/SymmListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSymmListBox

CSymmListBox::CSymmListBox()
{
	m_nBestChoises[0] = -1;
	m_nBestChoises[1] = -1;
}

CSymmListBox::~CSymmListBox()
{
}

BEGIN_MESSAGE_MAP(CSymmListBox, CListCtrl)
	//{{AFX_MSG_MAP(CSymmListBox)
	ON_MESSAGE(LVM_DELETEALLITEMS, OnDeleteAllItems)
	ON_MESSAGE(LVM_INSERTITEM, OnInsertItem)
	ON_MESSAGE(LVM_SETITEM, OnSetItem)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSymmListBox message handlers

void CSymmListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	LOGFONT lf;
	CFont *pFont = GetFont();
	if( pFont )
	{
		pFont->GetLogFont(&lf);
		if( lf.lfHeight < 0 )
			lpMeasureItemStruct->itemHeight = -lf.lfHeight;
		else
			lpMeasureItemStruct->itemHeight = lf.lfHeight;
	}
}

void CSymmListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CString sText;
	int i, iColWidth, iOffset;
	int iItem = (int)lpDIS->itemID;

	if( (iItem < 0) || (iItem >= m_vItemsList.size()) )
		return; 

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	CRect rect = lpDIS->rcItem;

	if( iItem == m_nBestChoises[0] )
	{
		CBrush brush(RGB(255, 0, 0));
		rect.right = rect.left + GetColumnWidth(0) + GetColumnWidth(1);
		pDC->FillRect(rect, &brush);
	}
//	if( iItem == m_nBestChoises[1] )
//	{
//		CBrush brush(RGB(0, 255, 0));
//		rect.right = rect.left + GetColumnWidth(0);
//		pDC->FillRect(rect, &brush);
//		rect.left = rect.left + GetColumnWidth(0) + GetColumnWidth(1);
//		rect.right = rect.left + GetColumnWidth(2);
//		pDC->FillRect(rect, &brush);
//	}
	// Setup the text format.
	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
	if( GetStyle() & LBS_USETABSTOPS )
		nFormat |= DT_EXPANDTABS;
	
	// Calculate the rectangle size before drawing the text.
	iOffset = 0;
	for(i = 0; i < 3; i++)
	{
		iColWidth = GetColumnWidth(i);
		if( 0 == i )
		{
			sText = m_vItemsList[iItem].sSymm;
		}
		else if( 1 == i )
		{
			sText = m_vItemsList[iItem].sRA1;
		}
		else if( 2 == i )
		{
			sText = m_vItemsList[iItem].sRA2;
		}
		rect.left = 0;
		pDC->DrawText(sText, &rect, nFormat | DT_CALCRECT);
		if( rect.Width() > iColWidth )
		{
			rect.right = rect.left + iColWidth;
		}
		rect.OffsetRect(iOffset, 0);
		pDC->DrawText(sText, &rect, nFormat);
		iOffset += iColWidth;
	}
}

LRESULT CSymmListBox::OnInsertItem(WPARAM wParam, LPARAM lParam)
{
	LVITEM* pLVItemSrc = reinterpret_cast<LVITEM*>(lParam);
	int     nColumn    = pLVItemSrc->iSubItem;
	LRESULT lResult = 0;
	int iItem = pLVItemSrc->iItem;

	if( m_vItemsList.size() <= iItem )
	{
		m_vItemsList.resize(iItem+1);
	}
	switch( pLVItemSrc->iSubItem )
	{
	case 0:
		m_vItemsList[iItem].sSymm = pLVItemSrc->pszText;
		break;
	case 1:
		m_vItemsList[iItem].sRA1 = pLVItemSrc->pszText;
		break;
	case 2:
		m_vItemsList[iItem].sRA2 = pLVItemSrc->pszText;
		break;
	default:
		TRACE(_T("CSymmListBox::OnInsertItem() -> Wrong subitem number (%d). Must be 0...2.\n"),
			pLVItemSrc->iSubItem);
	}

	lResult = DefWindowProc(LVM_INSERTITEM, wParam, lParam);

	return lResult;
}

LRESULT CSymmListBox::OnSetItem(WPARAM wParam, LPARAM lParam)
{
	LVITEM* pLVItemSrc = reinterpret_cast<LVITEM*>(lParam);
	int     nColumn    = pLVItemSrc->iSubItem;
	LRESULT lResult = 0;
	int iItem = pLVItemSrc->iItem;

	if( m_vItemsList.size() <= iItem )
	{
		m_vItemsList.resize(iItem+1);
	}
	switch( pLVItemSrc->iSubItem )
	{
	case 0:
		m_vItemsList[iItem].sSymm = pLVItemSrc->pszText;
		break;
	case 1:
		m_vItemsList[iItem].sRA1 = pLVItemSrc->pszText;
		break;
	case 2:
		m_vItemsList[iItem].sRA2 = pLVItemSrc->pszText;
		break;
	default:
		TRACE(_T("CSymmListBox::OnInsertItem() -> Wrong subitem number (%d). Must be 0...2.\n"),
			pLVItemSrc->iSubItem);
	}

	lResult = DefWindowProc(LVM_SETITEM, wParam, lParam);

	return lResult;
}

LRESULT CSymmListBox::OnDeleteAllItems(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;

	m_vItemsList.clear();

	lResult = DefWindowProc(LVM_DELETEALLITEMS, wParam, lParam);

	return lResult;
}

void CSymmListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rc, subrc;
	int iCol, iRow, iColWidth, iOffset;

	iOffset = 0;
	for(iCol = 0; iCol < 2; iCol++)
	{
		iColWidth = GetColumnWidth(iCol);
		if( point.x <= iOffset+iColWidth )
		{
			break;
		}
		iOffset += iColWidth;
	}
	for(iRow = 0; iRow < GetItemCount(); iRow++)
	{
		GetItemRect(iRow, rc, LVIR_BOUNDS);
		if( TRUE == rc.PtInRect(point) )
		{
			break;
		}
	}
//	iCol-1, iRow+1
	if( 0 == iCol )		// 1st choice
	{
		m_nBestChoises[0] = iRow;
		m_nBestChoises[1] = iRow;
		::PostMessage(GetParent()->GetSafeHwnd(), WMU_SET_BEST_CHOICE,
			(WPARAM)iCol, (LPARAM)iRow);
	}
	else if( 1 == iCol )
	{
		m_nBestChoises[0] = iRow;
		m_nBestChoises[1] = iRow;
		::PostMessage(GetParent()->GetSafeHwnd(), WMU_SET_BEST_CHOICE,
			(WPARAM)iCol, (LPARAM)iRow);
	}

	RedrawWindow();

	CListCtrl::OnLButtonDown(nFlags, point);
}
