#include "pch.h"
#include "CTabSheet.hpp"

IMPLEMENT_DYNAMIC(CTabSheet, CTabCtrl)

BEGIN_MESSAGE_MAP(CTabSheet, CTabCtrl)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, &CTabSheet::OnTcnSelchange)
END_MESSAGE_MAP()

CDialog* CTabSheet::GetPage(UINT nIdx)
{
	if (nIdx >= m_nTotalPages) return NULL;
	CTabNode& tab_node = m_ArrPages[nIdx];
	return tab_node.m_pDialog;
}

CString CTabSheet::GetCurrentPageTitle() const
{
	if (m_nCurrentPage < m_nTotalPages) {
		return m_ArrPages[m_nCurrentPage].m_strTitle;
	}
	return _T("");
}

BOOL CTabSheet::AddPage(LPCTSTR title, CDialog* pDialog, UINT nIDD)
{
	if (m_nTotalPages >= MAX_PAGES)
		return FALSE;

	if (!pDialog->Create(nIDD, this))
		return FALSE;
	
	pDialog->ShowWindow(SW_HIDE);

	CTabNode& tab_node = m_ArrPages[m_nTotalPages];
	tab_node.m_pDialog = pDialog;
	tab_node.m_DlgID = nIDD;
	tab_node.m_strTitle = title;

	m_nTotalPages++;
	return TRUE;
}

void CTabSheet::Show(UINT nIdx)
{
	ASSERT(nIdx < m_nTotalPages);
	DeleteAllItems();

	for (UINT idx = 0; idx < m_nTotalPages; ++idx)
	{
		CTabNode& tab_node = m_ArrPages[idx];
		if (!tab_node.m_pDialog) continue;
		InsertItem(idx, tab_node.m_strTitle);
	}

	SetRect(nIdx);
	SetCurFocus(nIdx);
}

void CTabSheet::SetRect(UINT nIdx)
{
	CRect tabRect, itemRect, clientRect;
	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);
	int nTabHeight = itemRect.Height() + 4;

	clientRect = tabRect;
	AdjustRect(FALSE, &clientRect);

	clientRect.bottom -= 2;
	clientRect.left += 2;
	clientRect.right -= 2;

	for (UINT idx = 0; idx < m_nTotalPages; ++idx)
	{
		CTabNode& tab_node = m_ArrPages[idx];
		if (!tab_node.m_pDialog) continue;

		tab_node.m_pDialog->MoveWindow(
			clientRect.left,
			clientRect.top,
			clientRect.Width(),
			clientRect.Height(),
			TRUE);

		tab_node.m_pDialog->ShowWindow(idx == nIdx ? SW_SHOW : SW_HIDE);
	}
}

void CTabSheet::OnTcnSelchange(NMHDR* pNMHDR, LRESULT* pResult)
{
	int newPageIndex = GetCurFocus();
	if (m_nCurrentPage != newPageIndex)
	{
		CTabNode& old_node = m_ArrPages[m_nCurrentPage];
		if (old_node.m_pDialog) 
			old_node.m_pDialog->ShowWindow(SW_HIDE);

		m_nCurrentPage = newPageIndex;

		CTabNode& new_node = m_ArrPages[m_nCurrentPage];
		if (new_node.m_pDialog) 
			new_node.m_pDialog->ShowWindow(SW_SHOW);
	}
	*pResult = 0;
}
