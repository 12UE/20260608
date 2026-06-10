#pragma once
#include "afxdialogex.h"


// CEnrollManagerDlg 对话框

class CEnrollManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEnrollManagerDlg)

public:
	CEnrollManagerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CEnrollManagerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ENROLLDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:

	virtual BOOL OnInitDialog();
	afx_msg void OnCbnDropdownEnrollQueryCourseCombo();
	afx_msg void OnBnClickedExecute();
	afx_msg void OnBnClickedEnrollUpdateRadio();
	void OnReceiveEnrollInfo(PkgS2CEnroll* pPkg);
	CListCtrl m_QueryList;
	afx_msg void OnBnClickedQuery();
	CEdit m_QueryStudentNameEdit;
	CEdit m_QueryCourseNameEdit;
	afx_msg void OnBnClickedClear();
	afx_msg void OnNMRClickEnrollQuerylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPopEnrollModify();
	CEdit m_StudentIdEdit;
	CEdit m_CourseIdEdit;
	CEdit m_ScoreEdit;
	CButton m_UpdateRadio;
	CButton m_InsertRadio;
	afx_msg void OnPopEnrollDel();
	void RefreshList();
};
