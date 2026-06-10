#pragma once
#include "afxdialogex.h"


// CStudentManagerDlg 对话框

class CStudentManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStudentManagerDlg)

public:
	CStudentManagerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CStudentManagerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENTDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_QueryList;
	afx_msg void OnBnClickedClear();
	void OnReceiveStudentInfo(PkgS2CStudent* pPkg);
	CEdit m_QueryStudentNameEdit;
	CEdit m_QueryCourseEdit;
	afx_msg void OnBnClickedQuery();
	afx_msg void OnNMRClickQuerylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeletestu();
	afx_msg void OnUpdatestu();
	CEdit m_StudentNameEdit;
	CEdit m_CourseIDEdit;
	CEdit m_StudentId;
	afx_msg void OnBnClickedSaveRadioButton();
	afx_msg void OnBnClickedRadio2();
	CButton m_SaveRadio;
	CButton m_UpdateRadio;
	afx_msg void OnBnClickedExecute();
	afx_msg void OnBnClickedButton3();
};
