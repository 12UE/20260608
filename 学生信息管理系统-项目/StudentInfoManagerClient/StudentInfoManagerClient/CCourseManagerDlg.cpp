// CCourseManagerDlg.cpp: 实现文件
//

#include "pch.h"
#include "StudentInfoManagerClient.h"
#include "afxdialogex.h"
#include "CCourseManagerDlg.h"
#include"resource.h"

// CCourseManagerDlg 对话框

IMPLEMENT_DYNAMIC(CCourseManagerDlg, CDialogEx)

CCourseManagerDlg::CCourseManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COURSEDLG, pParent)
{

}

CCourseManagerDlg::~CCourseManagerDlg()
{
}

void CCourseManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COURSE_QUERYLIST, m_QueryList);
	DDX_Control(pDX, IDC_EDIT2, m_CourseNameEdit);
	DDX_Control(pDX, IDC_CLASS_SAVE_RADIO, m_InsertRadio);
	DDX_Control(pDX, IDC_CLASS_UPDATE_RADIO, m_updateRadio);
	DDX_Control(pDX, IDC_EDIT5, m_CourseIdEdit);
}


BEGIN_MESSAGE_MAP(CCourseManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON3, &CCourseManagerDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &CCourseManagerDlg::OnBnClickedClear)
	ON_NOTIFY(NM_RCLICK, IDC_COURSE_QUERYLIST, &CCourseManagerDlg::OnNMRClickCourseQuerylist)
	ON_COMMAND(ID_POP_COURSE_DEL, &CCourseManagerDlg::OnPopCourseDel)
	ON_COMMAND(ID_UPDATESTU, &CCourseManagerDlg::OnUpdatestu)
	ON_BN_CLICKED(IDC_BUTTON1, &CCourseManagerDlg::OnBnClickedExecute)
	ON_BN_CLICKED(IDC_CLASS_SAVE_RADIO, &CCourseManagerDlg::OnBnClickedClassSaveRadio)
	ON_COMMAND(ID_POP_COURSE_MODIFY, &CCourseManagerDlg::OnPopCourseModify)
END_MESSAGE_MAP()


// CCourseManagerDlg 消息处理程序

BOOL CCourseManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	//课程ID 课程名称 选课学生数
	m_QueryList.InsertColumn(0, _T("课程ID"), LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(1, _T("课程名称"), LVCFMT_LEFT, 200);
	m_QueryList.InsertColumn(2, _T("选课学生数"), LVCFMT_LEFT, 100);
	m_QueryList.SetExtendedStyle(
		m_QueryList.GetExtendedStyle() |
		LVS_EX_FULLROWSELECT |  // 全行选中
		LVS_EX_GRIDLINES       // 显示网格线
	);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CCourseManagerDlg::OnReceiveCourseInfo(PkgS2CCourse* pPkg)
{
	m_QueryList.DeleteAllItems(); // 清空列表控件
	if (pPkg == nullptr || pPkg->m_nCount <= 0) {
		AfxMessageBox(_T("没有查询到相关课程信息"));
		return;
	}
	for (size_t i = 0; i < pPkg->m_nCount; i++)
	{
		auto& info = pPkg->m_courseInfo[i];
		m_QueryList.InsertItem(i, std::to_string(info.m_nCourseID).c_str());
		m_QueryList.SetItemText(i, 1, info.m_szCourseName);
		m_QueryList.SetItemText(i, 2, std::to_string(info.m_nStudentCount).c_str());
	}
}

void CCourseManagerDlg::OnBnClickedButton3()
{
	ClientCourseQueryPacket* ClientCourseQueryheader = new ClientCourseQueryPacket;
	CString szCourseName;
	m_CourseNameEdit.GetWindowText(szCourseName);
	ClientCourseQueryheader->m_szCourseName.Init(szCourseName.GetString());
	KCP::GetInstance().Send(ClientCourseQueryheader, sizeof(ClientCourseQueryPacket));
}

void CCourseManagerDlg::OnBnClickedClear()
{
	m_QueryList.DeleteAllItems(); // 清空列表控件
	AfxMessageBox(_T("已清空查询结果列表"));
}

void CCourseManagerDlg::OnNMRClickCourseQuerylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CPoint point;
	GetCursorPos(&point);  // 获取屏幕坐标

	// 获取列表控件
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_COURSE_QUERYLIST);
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
			if (menu.LoadMenu(IDR_COURSE_POP_MENU))
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

void CCourseManagerDlg::OnPopCourseDel()
{
	int nItem = m_QueryList.GetSelectionMark();
	if(nItem== -1) {
		AfxMessageBox(_T("请先选择要删除的课程信息！"));
		return;
	}
	CourseDelete courseDeletePacket;
	courseDeletePacket.m_nCourseID = _ttoi(m_QueryList.GetItemText(nItem, 0)); // 将CString转换为整数
	KCP::GetInstance().Send((char*)&courseDeletePacket, sizeof(courseDeletePacket));
	ClientCourseQueryPacket ClientCourseQueryheader;
	ClientCourseQueryheader.m_szCourseName.Init("");//无条件搜索
	KCP::GetInstance().Send(&ClientCourseQueryheader, sizeof(ClientCourseQueryheader));
	m_QueryList.DeleteItem(nItem);// 删除选中的行
	AfxMessageBox(_T("课程已删除"));
}

void CCourseManagerDlg::OnUpdatestu()
{
	
}

void CCourseManagerDlg::OnBnClickedExecute()
{
	CString courseName, courseId;
	m_CourseNameEdit.GetWindowText(courseName);
	m_CourseIdEdit.GetWindowText(courseId);
	if (courseName.IsEmpty()) {
		AfxMessageBox(_T("课程名称不能为空！"));
		return;
	}
	//清空输入
	m_CourseNameEdit.SetWindowTextA("");
	m_CourseIdEdit.SetWindowTextA("");
	if (m_InsertRadio.GetCheck() == BST_CHECKED) {
		// 保存模式
		if (!courseId.IsEmpty()) {
			AfxMessageBox(_T("保存模式下不需要输入课程ID！"));
			return;
		}
		CourseAdd courseAddPacket;
		courseAddPacket.m_szCourseName.Init(courseName.GetString());
		KCP::GetInstance().Send((char*)&courseAddPacket, sizeof(courseAddPacket));
		ClientCourseQueryPacket Clientheader;
		Clientheader.m_szCourseName.Init("");
		KCP::GetInstance().Send(&Clientheader, sizeof(Clientheader));
		AfxMessageBox(_T("课程信息已添加"));
	}
	else if (m_updateRadio.GetCheck() == BST_CHECKED) {
		CourseUpdate courseUpdatePacket;
		courseUpdatePacket.m_nCourseID = _ttoi(courseId); // 将CString转换为整数
		courseUpdatePacket.m_szCourseName.Init(courseName.GetString());
		KCP::GetInstance().Send((char*)&courseUpdatePacket, sizeof(courseUpdatePacket));
		ClientCourseQueryPacket Clientheader;
		Clientheader.m_szCourseName.Init("");
		KCP::GetInstance().Send(&Clientheader, sizeof(Clientheader));
		AfxMessageBox(_T("课程信息已更新"));
	}
	else {
		AfxMessageBox(_T("请选择操作类型！"));
	}
}

void CCourseManagerDlg::OnBnClickedClassSaveRadio()
{
	m_CourseIdEdit.SetWindowTextA("");
}

void CCourseManagerDlg::OnPopCourseModify()
{
	// TODO: 在此添加命令处理程序代码
	int nItem = m_QueryList.GetSelectionMark();
	if (nItem == -1) {
		AfxMessageBox(_T("请先选择要删除的课程信息！"));
		return;
	}
	CString courseId = m_QueryList.GetItemText(nItem, 0);
	CString courseName = m_QueryList.GetItemText(nItem, 1);
	m_CourseNameEdit.SetWindowText(courseName);
	m_CourseIdEdit.SetWindowText(courseId);
	m_updateRadio.SetCheck(BST_CHECKED);
	m_InsertRadio.SetCheck(BST_UNCHECKED);
}
