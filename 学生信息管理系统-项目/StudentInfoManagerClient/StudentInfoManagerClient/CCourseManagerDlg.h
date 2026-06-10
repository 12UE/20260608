#pragma once
#include "afxdialogex.h"


// CCourseManagerDlg 对话框

class CCourseManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCourseManagerDlg)

public:
	CCourseManagerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCourseManagerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COURSEDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void OnReceiveCourseInfo(PkgS2CCourse* pPkg);
	CListCtrl m_QueryList;
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnNMRClickCourseQuerylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPopCourseDel();
	CEdit m_CourseNameEdit;
	afx_msg void OnUpdatestu();
	CButton m_InsertRadio;
	CButton m_updateRadio;
	afx_msg void OnBnClickedExecute();
	CEdit m_CourseIdEdit;
	afx_msg void OnBnClickedClassSaveRadio();
	afx_msg void OnPopCourseModify();
};
