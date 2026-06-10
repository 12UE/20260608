
// StudentInfoManagerClientDlg.h: 头文件
//

#pragma once


// CStudentInfoManagerClientDlg 对话框
class CStudentInfoManagerClientDlg : public CDialogEx
{
private:
	PacketHeader* RecvPkg();
// 构造
public:
	CStudentInfoManagerClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENTINFOMANAGERCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()
public:
	CTabSheet          m_tab;
	CClassManagerDlg   m_ClassDlg;
	CCourseManagerDlg  m_CourseDlg;
	CStudentManagerDlg m_StudentDlg;
	CEnrollManagerDlg  m_EnrollDlg;
	CCombineDlg        m_CombineDlg;
};
