#pragma once
#include "afxdialogex.h"


// CCombineDlg 对话框

class CCombineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCombineDlg)

public:
	CCombineDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCombineDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMBINEQUERY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	void OnReceiveCombineInfo(PkgS2CCombine* pPkg);
	virtual BOOL OnInitDialog();
	CListCtrl m_QueryList;
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedQuery();
	CEdit m_ClassNameEdit;
	CEdit m_CourseNameEdit;
	CEdit m_StudentNameEdit;
	CEdit m_LowestScoreEdit;
	CEdit m_HighestScoreEdit;
	afx_msg void OnBnClickedButton4();
};
