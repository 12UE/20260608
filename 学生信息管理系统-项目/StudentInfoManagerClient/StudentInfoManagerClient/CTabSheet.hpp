#pragma once
#include <afxwin.h>
#include <afxcmn.h> // 包含CTabCtrl
#include <afxext.h>

#define MAX_PAGES 30

class CTabNode
{
public:
	CTabNode()
	{
		m_DlgID = 0;
		m_pDialog = NULL;
	};
	~CTabNode() {};

public:
	UINT m_DlgID;
	CDialog* m_pDialog;
	CString m_strTitle;
};

class CTabSheet : public CTabCtrl
{
	DECLARE_DYNAMIC(CTabSheet)

public:
	CTabSheet()
	{
		m_nTotalPages = 0;
		m_nCurrentPage = 0;
	}
	
	virtual ~CTabSheet()
	{
		// 自动清理资源
	}

	BOOL AddPage(LPCTSTR title, CDialog* pDialog, UINT nIDD);
	CDialog* GetPage(UINT nIdx);
	void Show(UINT nIdx = 0);
	CString GetCurrentPageTitle() const;

protected:
	DECLARE_MESSAGE_MAP()

private:
	UINT m_nTotalPages;
	UINT m_nCurrentPage;
	CTabNode m_ArrPages[MAX_PAGES];
	void SetRect(UINT nIdx);
	afx_msg void OnTcnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
};
