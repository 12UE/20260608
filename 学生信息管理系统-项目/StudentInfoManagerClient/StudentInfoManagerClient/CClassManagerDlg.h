#pragma once
#include "afxdialogex.h"


// CClassManagerDlg 对话框

class CClassManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClassManagerDlg)

public:
	CClassManagerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CClassManagerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLASSDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_QueryList;
	virtual BOOL OnInitDialog();
	void OnReceiveClassInfo(PkgS2CClass* pPkg);
	afx_msg void OnBnClickedQuery();
	CEdit m_QueryClassEdit;
	afx_msg void OnBnClickedClear();
	afx_msg void OnNMRClickQuerylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPopClassDel();
	afx_msg void OnPopClassModify();
	CEdit m_ClassId;
	CEdit m_ClassName;
	afx_msg void OnBnClickedExecute();
	CButton m_UpdateRadio;
	CButton m_AddRadio;
	afx_msg void OnBnClickedClassInsertRadio();
	afx_msg void OnBnDoubleclickedClassInsertRadio();
};
