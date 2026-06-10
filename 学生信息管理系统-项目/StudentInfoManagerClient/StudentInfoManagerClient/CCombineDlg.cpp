// CCombineDlg.cpp: 实现文件
//

#include "pch.h"
#include "StudentInfoManagerClient.h"
#include "afxdialogex.h"
#include "CCombineDlg.h"


// CCombineDlg 对话框

IMPLEMENT_DYNAMIC(CCombineDlg, CDialogEx)

CCombineDlg::CCombineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMBINEQUERY, pParent)
{

}

CCombineDlg::~CCombineDlg()
{
}

void CCombineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_QUERYLIST, m_QueryList);
	DDX_Control(pDX, IDC_EDIT7, m_ClassNameEdit);
	DDX_Control(pDX, IDC_EDIT8, m_CourseNameEdit);
	DDX_Control(pDX, IDC_EDIT2, m_StudentNameEdit);
	DDX_Control(pDX, IDC_EDIT1, m_LowestScoreEdit);
	DDX_Control(pDX, IDC_EDIT4, m_HighestScoreEdit);
}


BEGIN_MESSAGE_MAP(CCombineDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &CCombineDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_BUTTON1, &CCombineDlg::OnBnClickedQuery)
	ON_BN_CLICKED(IDC_BUTTON4, &CCombineDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CCombineDlg 消息处理程序

void CCombineDlg::OnReceiveCombineInfo(PkgS2CCombine* pPkg)
{
	m_QueryList.DeleteAllItems(); // 清空列表控件
	if(pPkg == nullptr || pPkg->m_nCount <= 0) {
		AfxMessageBox(_T("没有查询到相关组合信息"));
		return;
	}
	for(int i = 0; i < pPkg->m_nCount; i++)
	{
		CombineInfo& info = pPkg->m_combineInfo[i];
		int nIndex = m_QueryList.InsertItem(i, info.m_szClassName.m_szStr);
		m_QueryList.SetItemText(nIndex, 1, info.m_szClassId.m_szStr);
		m_QueryList.SetItemText(nIndex, 2, info.m_szCourseName.m_szStr);
		m_QueryList.SetItemText(nIndex, 3, info.m_szCourseId.m_szStr);
		m_QueryList.SetItemText(nIndex, 4, info.m_szStudentName.m_szStr);
		m_QueryList.SetItemText(nIndex, 5, info.m_szStudentId.m_szStr);
		m_QueryList.SetItemText(nIndex, 6, info.m_szScore.m_szStr);
		
	}

}

BOOL CCombineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//学生姓名 班级 课程名称
	m_QueryList.InsertColumn(0, _T("班级"), LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(1, _T("班级ID"), LVCFMT_LEFT, 50);
	m_QueryList.InsertColumn(2, _T("课程名称"), LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(3, _T("课程ID"), LVCFMT_LEFT, 50);
	m_QueryList.InsertColumn(4, _T("学生姓名"), LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(5, _T("学生ID"), LVCFMT_LEFT, 50);
	m_QueryList.InsertColumn(6, _T("分数"), LVCFMT_LEFT, 100);
	m_QueryList.SetExtendedStyle(
		m_QueryList.GetExtendedStyle() |
		LVS_EX_FULLROWSELECT |  // 全行选中
		LVS_EX_GRIDLINES       // 显示网格线
	);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CCombineDlg::OnBnClickedClear()
{
	m_QueryList.DeleteAllItems(); // 清空列表控件
	AfxMessageBox(_T("已清空查询结果列表"));
}

void CCombineDlg::OnBnClickedQuery()
{
	CString szClassName, szCourseName, szStuName, szLowestScore, szHighestScore;
	m_ClassNameEdit.GetWindowText(szClassName);
	m_CourseNameEdit.GetWindowText(szCourseName);
	m_StudentNameEdit.GetWindowText(szStuName);
	m_LowestScoreEdit.GetWindowText(szLowestScore);
	m_HighestScoreEdit.GetWindowText(szHighestScore);
	CombineQueryPacket combineQueryPacket;
	combineQueryPacket.m_szClassName.Init(szClassName.GetString());
	combineQueryPacket.m_szCourseName.Init(szCourseName.GetString());
	combineQueryPacket.m_szStuName.Init(szStuName.GetString());
	combineQueryPacket.m_szLowestScore.Init(szLowestScore.GetString());
	combineQueryPacket.m_szHighestScore.Init(szHighestScore.GetString());
	KCP::GetInstance().Send((char*)&combineQueryPacket, sizeof(combineQueryPacket));
	m_ClassNameEdit.SetWindowTextA(""); // 清空查询输入框
	m_CourseNameEdit.SetWindowTextA(""); // 清空查询输入框
	m_StudentNameEdit.SetWindowTextA(""); // 清空查询输入框
	m_LowestScoreEdit.SetWindowTextA(""); // 清空查询输入框
	m_HighestScoreEdit.SetWindowTextA(""); // 清空查询输入框
}

void CCombineDlg::OnBnClickedButton4()
{
	int nRet = ::MessageBoxA(NULL, "是否清空所有信息？数据不可还原", "提示", MB_YESNO | MB_ICONQUESTION);
	if (nRet == IDYES) {
		StudentClear studentClearPacket;
		KCP::GetInstance().Send((char*)&studentClearPacket, sizeof(studentClearPacket));
		AfxMessageBox(_T("数据已清空"));
		m_QueryList.DeleteAllItems(); // 清空列表控件
	}
	else {
		return; // 用户选择不清空
	}
}
