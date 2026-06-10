// CEnrollManagerDlg.cpp: 实现文件
//

#include "pch.h"
#include "StudentInfoManagerClient.h"
#include "afxdialogex.h"
#include "CEnrollManagerDlg.h"


// CEnrollManagerDlg 对话框

IMPLEMENT_DYNAMIC(CEnrollManagerDlg, CDialogEx)

CEnrollManagerDlg::CEnrollManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ENROLLDLG, pParent)
{

}

CEnrollManagerDlg::~CEnrollManagerDlg()
{
}

void CEnrollManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ENROLL_QUERYLIST, m_QueryList);
	DDX_Control(pDX, IDC_ENROLL_QUERY_STU_NAME, m_QueryStudentNameEdit);
	DDX_Control(pDX, IDC_ENROLL_QUERY_COURSENAME, m_QueryCourseNameEdit);
	DDX_Control(pDX, IDC_EDIT6, m_StudentIdEdit);
	DDX_Control(pDX, IDC_EDIT3, m_CourseIdEdit);
	DDX_Control(pDX, IDC_EDIT1, m_ScoreEdit);
	DDX_Control(pDX, IDC_ENROLL_UPDATE_RADIO, m_UpdateRadio);
	DDX_Control(pDX, IDC_ENROLL_SAVE_RADIO, m_InsertRadio);
}


BEGIN_MESSAGE_MAP(CEnrollManagerDlg, CDialogEx)
	ON_CBN_DROPDOWN(IDC_ENROLL_QUERY_COURSE_COMBO, &CEnrollManagerDlg::OnCbnDropdownEnrollQueryCourseCombo)
	ON_BN_CLICKED(IDC_BUTTON2, &CEnrollManagerDlg::OnBnClickedExecute)
	ON_BN_CLICKED(IDC_ENROLL_UPDATE_RADIO, &CEnrollManagerDlg::OnBnClickedEnrollUpdateRadio)
	ON_BN_CLICKED(IDC_BUTTON1, &CEnrollManagerDlg::OnBnClickedQuery)
	ON_BN_CLICKED(IDC_BUTTON3, &CEnrollManagerDlg::OnBnClickedClear)
	ON_NOTIFY(NM_RCLICK, IDC_ENROLL_QUERYLIST, &CEnrollManagerDlg::OnNMRClickEnrollQuerylist)
	ON_COMMAND(ID_POP_ENROLL_MODIFY, &CEnrollManagerDlg::OnPopEnrollModify)
	ON_COMMAND(ID_POP_ENROLL_DEL, &CEnrollManagerDlg::OnPopEnrollDel)
END_MESSAGE_MAP()


// CEnrollManagerDlg 消息处理程序

BOOL CEnrollManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//学生姓名 班级 课程名称 成绩
	m_QueryList.InsertColumn(0, _T("学生姓名"), LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(1, _T("学生ID"), LVCFMT_LEFT, 50);
	m_QueryList.InsertColumn(2, _T("班级"), LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(3, _T("课程名称"), LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(4, _T("课程ID"), LVCFMT_LEFT, 50);
	m_QueryList.InsertColumn(5, _T("成绩"), LVCFMT_LEFT, 100);
	m_QueryList.SetExtendedStyle(
		m_QueryList.GetExtendedStyle() |
		LVS_EX_FULLROWSELECT |  // 全行选中
		LVS_EX_GRIDLINES       // 显示网格线
	);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CEnrollManagerDlg::OnCbnDropdownEnrollQueryCourseCombo()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CEnrollManagerDlg::OnBnClickedExecute()
{
	if (m_InsertRadio.GetCheck() == BST_CHECKED) {
		// 插入选课信息
		CString szStudentId, szCourseId, szScore;
		m_StudentIdEdit.GetWindowText(szStudentId);
		m_CourseIdEdit.GetWindowText(szCourseId);
		if(szStudentId.IsEmpty() || szCourseId.IsEmpty()) {
			AfxMessageBox(_T("学生ID和课程ID不能为空"));
			return;
		}
		EnrollInsert enrollInsert;
		enrollInsert.m_szStudentId.Init(szStudentId.GetString());
		enrollInsert.m_szCourseId.Init(szCourseId.GetString());
		m_ScoreEdit.GetWindowText(szScore);
		enrollInsert.m_szScore.Init(szScore.GetString());
		KCP::GetInstance().Send((char*)&enrollInsert, sizeof(enrollInsert));
		m_StudentIdEdit.SetWindowTextA(""); // 清空输入框
		m_CourseIdEdit.SetWindowTextA(""); // 清空输入框
		m_ScoreEdit.SetWindowTextA(""); // 清空输入框
		RefreshList(); // 刷新列表
		AfxMessageBox(_T("选课信息已插入"));
	}
	if (m_UpdateRadio.GetCheck() == BST_CHECKED) {
		//更新选课信息
		CString szStudentId, szCourseId, szScore;
		m_StudentIdEdit.GetWindowText(szStudentId);
		m_CourseIdEdit.GetWindowText(szCourseId);
		m_ScoreEdit.GetWindowText(szScore);
		if (szStudentId.IsEmpty() || szCourseId.IsEmpty() || szScore.IsEmpty()) {
			AfxMessageBox(_T("学生ID、课程ID和成绩不能为空"));
			return;
		}
		EnrollUpdate updatepack;
		updatepack.m_szCourseId.Init(szCourseId.GetString());
		updatepack.m_szStudentId.Init(szStudentId.GetString());
		updatepack.m_szScore.Init(szScore.GetString());
		KCP::GetInstance().Send((char*)&updatepack, sizeof(updatepack));
		m_StudentIdEdit.SetWindowTextA(""); // 清空输入框
		m_CourseIdEdit.SetWindowTextA(""); // 清空输入框
		m_ScoreEdit.SetWindowTextA(""); // 清空输入框
		RefreshList(); // 刷新列表
		AfxMessageBox(_T("选课信息已更新"));
	}
}

void CEnrollManagerDlg::OnBnClickedEnrollUpdateRadio()
{
	// TODO: 在此添加控件通知处理程序代码

}

void CEnrollManagerDlg::OnReceiveEnrollInfo(PkgS2CEnroll* pPkg)
{
	m_QueryList.DeleteAllItems(); // 清空列表控件
	if (pPkg == nullptr || pPkg->m_nCount <= 0) {
		AfxMessageBox(_T("没有查询到相关选课信息"));
		return;
	}
	for (size_t i = 0; i < pPkg->m_nCount; i++)
	{
		auto& info = pPkg->m_enrollInfo[i];
		m_QueryList.InsertItem(i, info.m_szStudentName);
		m_QueryList.SetItemText(i,1, info.m_szStudentID);
		m_QueryList.SetItemText(i, 2, info.m_szClassName);
		m_QueryList.SetItemText(i, 3, info.m_szCourseName);
		m_QueryList.SetItemText(i, 4, info.m_szCourseID);

		m_QueryList.SetItemText(i, 5, info.m_szScore);
	}
}

void CEnrollManagerDlg::OnBnClickedQuery()
{
	// TODO: 在此添加控件通知处理程序代码
	EnrollQueryPacket enrollQueryPacket;
	CString szStuName;
	m_QueryStudentNameEdit.GetWindowText(szStuName);
	enrollQueryPacket.m_szStuName.Init(szStuName.GetString());
	CString szCourseName;
	m_QueryCourseNameEdit.GetWindowText(szCourseName);
	enrollQueryPacket.m_szCourseName.Init(szCourseName.GetString());
	KCP::GetInstance().Send((char*)&enrollQueryPacket, sizeof(enrollQueryPacket));
	m_QueryStudentNameEdit.SetWindowTextA(""); // 清空查询输入框
	m_QueryCourseNameEdit.SetWindowTextA(""); // 清空查询输入框
}

void CEnrollManagerDlg::OnBnClickedClear()
{
	m_QueryList.DeleteAllItems(); // 清空列表控件
	AfxMessageBox(_T("已清空查询结果列表"));
}

void CEnrollManagerDlg::OnNMRClickEnrollQuerylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CPoint point;
	GetCursorPos(&point);  // 获取屏幕坐标

	// 获取列表控件
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_ENROLL_QUERYLIST);
	if (pListCtrl)
	{
		// 将屏幕坐标转换为控件客户区坐标
		CPoint ptClient = point;
		pListCtrl->ScreenToClient(&ptClient);

		// 进行点击测试，确定是否在有效的列表项上点击
		UINT flags = 0;
		int nItem = pListCtrl->HitTest(ptClient, &flags);

		// 如果在有效的列表项上点击，则选中该项
		if (nItem >= 0)
		{
			pListCtrl->SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

			// 加载弹出菜单
			CMenu menu;
			if (menu.LoadMenu(IDR_ENROLL_POP_MENU))
			{
				// 获取子菜单 - 这是关键步骤！
				CMenu* pPopup = menu.GetSubMenu(0);
				if (pPopup)
				{
					// 设置菜单项默认字体和大小
					NONCLIENTMETRICS ncm;
					ncm.cbSize = sizeof(NONCLIENTMETRICS);
					SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

					CFont menuFont;
					menuFont.CreateFontIndirect(&ncm.lfMenuFont);

					// 使用子菜单显示弹出菜单
					pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
						point.x, point.y, this);
				}
			}
		}
	}
	*pResult = 0;
}

void CEnrollManagerDlg::OnPopEnrollModify()
{
	// TODO: 在此添加命令处理程序代码
	int nSelected = m_QueryList.GetSelectionMark();
	if (nSelected == -1) {
		AfxMessageBox(_T("请选择一个选课信息进行修改"));
		return;
	}
	CString szStudentId = m_QueryList.GetItemText(nSelected, 1);
	CString szCourseId = m_QueryList.GetItemText(nSelected, 4);
	CString szScore = m_QueryList.GetItemText(nSelected, 5);
	m_StudentIdEdit.SetWindowText(szStudentId);
	m_CourseIdEdit.SetWindowText(szCourseId);
	m_ScoreEdit.SetWindowText(szScore);
	m_UpdateRadio.SetCheck(BST_CHECKED); // 设置更新单选按钮为选中状态
}

void CEnrollManagerDlg::OnPopEnrollDel()
{
	int nSelected = m_QueryList.GetSelectionMark();
	if (nSelected == -1) {
		AfxMessageBox(_T("请选择一个选课信息进行修改"));
		return;
	}
	
	EnrollDelete enrollDelete;
	enrollDelete.m_szStudentId.Init(m_QueryList.GetItemText(nSelected, 1).GetString());
	enrollDelete.m_szCourseId.Init(m_QueryList.GetItemText(nSelected, 4).GetString());
	KCP::GetInstance().Send((char*)&enrollDelete, sizeof(enrollDelete));
	RefreshList(); // 刷新列表
	m_QueryList.DeleteItem(nSelected); // 删除选中的行
}

void CEnrollManagerDlg::RefreshList()
{
	EnrollQueryPacket enrollQueryPacket;
	enrollQueryPacket.m_szStuName.Init("");
	enrollQueryPacket.m_szCourseName.Init("");
	KCP::GetInstance().Send((char*)&enrollQueryPacket, sizeof(enrollQueryPacket));
}
