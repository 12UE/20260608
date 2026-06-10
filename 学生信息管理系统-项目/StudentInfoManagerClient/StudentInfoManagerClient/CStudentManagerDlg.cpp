// CStudentManagerDlg.cpp: 实现文件
//

#include "pch.h"
#include "StudentInfoManagerClient.h"
#include "afxdialogex.h"
#include "CStudentManagerDlg.h"

// CStudentManagerDlg 对话框

IMPLEMENT_DYNAMIC(CStudentManagerDlg, CDialogEx)

CStudentManagerDlg::CStudentManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STUDENTDLG, pParent)
{

}

CStudentManagerDlg::~CStudentManagerDlg()
{
}

void CStudentManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_QUERYLIST, m_QueryList);
	DDX_Control(pDX, STU_QUERY_STU_NAME_EDIT, m_QueryStudentNameEdit);
	DDX_Control(pDX, STU_QUERY_CLASS_EDIT, m_QueryCourseEdit);
	DDX_Control(pDX, IDC_STU_NAME_EDIT, m_StudentNameEdit);
	DDX_Control(pDX, IDC_STU_COURSE_ID, m_CourseIDEdit);
	DDX_Control(pDX, IDC_STU_ID_EDIT, m_StudentId);
	DDX_Control(pDX, IDC_RADIO1, m_SaveRadio);
	DDX_Control(pDX, IDC_RADIO2, m_UpdateRadio);
}


BEGIN_MESSAGE_MAP(CStudentManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &CStudentManagerDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_BUTTON4, &CStudentManagerDlg::OnBnClickedQuery)
	ON_NOTIFY(NM_RCLICK, IDC_QUERYLIST, &CStudentManagerDlg::OnNMRClickQuerylist)
	ON_COMMAND(ID_DELETESTU, &CStudentManagerDlg::OnDeletestu)
	ON_COMMAND(ID_UPDATESTU, &CStudentManagerDlg::OnUpdatestu)
	ON_BN_CLICKED(IDC_RADIO1, &CStudentManagerDlg::OnBnClickedSaveRadioButton)
	ON_BN_CLICKED(IDC_RADIO2, &CStudentManagerDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_BUTTON1, &CStudentManagerDlg::OnBnClickedExecute)
	ON_BN_CLICKED(IDC_BUTTON3, &CStudentManagerDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CStudentManagerDlg 消息处理程序

BOOL CStudentManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	//学号 学生姓名 所属班级
	m_QueryList.InsertColumn(0, "学生ID", LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(1, "学生姓名", LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(2, "所属班级", LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(3, "班级ID", LVCFMT_LEFT, 100);
	m_QueryList.SetExtendedStyle(
		m_QueryList.GetExtendedStyle() |
		LVS_EX_FULLROWSELECT |  // 全行选中
		LVS_EX_GRIDLINES       // 显示网格线
	);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CStudentManagerDlg::OnBnClickedClear()
{
	m_QueryList.DeleteAllItems(); // 清空列表控件
	AfxMessageBox(_T("已清空查询结果列表"));
	// TODO: 在此添加控件通知处理程序代码
}
struct QueryResult {
	std::vector<std::string> columnNames;
	std::vector<std::vector<std::string>> rows;
	bool success;
	std::string errorMessage;

	QueryResult() : success(true) {}
};

void CStudentManagerDlg::OnReceiveStudentInfo(PkgS2CStudent* pPkgStudent) {
	m_QueryList.DeleteAllItems(); // 清空列表控件
	if( pPkgStudent == nullptr || pPkgStudent->m_nCount <= 0) {
		AfxMessageBox(_T("没有查询到相关学生信息"));
		return;
	}
	for (size_t i = 0; i < pPkgStudent->m_nCount; i++)
	{
		auto &info = pPkgStudent->m_stuInfo[i];
		m_QueryList.InsertItem(i, std::to_string(info.m_nStudentID).c_str());
		m_QueryList.SetItemText(i, 1, info.m_szStudentName);
		m_QueryList.SetItemText(i, 2, info.m_szClassName);
		m_QueryList.SetItemText(i, 3, std::to_string(info.m_nClassID).c_str());

	}

	
}
void CStudentManagerDlg::OnBnClickedQuery()
{
	ClientStudentQueryPacket Clientheader;
	CString szStuName;

	m_QueryStudentNameEdit.GetWindowText(szStuName);
	m_QueryStudentNameEdit.SetWindowText("");
	Clientheader.m_szStuName.Init(szStuName.GetString());
	CString szCourse;
	m_QueryCourseEdit.GetWindowText(szCourse);
	m_QueryCourseEdit.SetWindowText("");
	Clientheader.m_szCourseName.Init(szCourse.GetString());
	KCP::GetInstance().Send(&Clientheader,sizeof(Clientheader));
	
	// TODO: 在此添加控件通知处理程序代码
}

void CStudentManagerDlg::OnNMRClickQuerylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CPoint point;
	GetCursorPos(&point);  // 获取屏幕坐标

	// 获取列表控件
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_QUERYLIST);
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
			if (menu.LoadMenu(IDR_STU_POP_MENU))
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

void CStudentManagerDlg::OnDeletestu()
{
	int nItem = m_QueryList.GetSelectionMark();
	if (nItem == -1) {
		AfxMessageBox(_T("请先选择要删除的学生信息！"));
		return;
	}
	CString studentID = m_QueryList.GetItemText(nItem, 0);
	m_QueryList.DeleteItem(nItem); // 删除选中的行
	StudentDelete studentDeletePacket;
	studentDeletePacket.m_nStuID = _ttoi(studentID); // 将CString转换为整数
	KCP::GetInstance().Send((char*)&studentDeletePacket, sizeof(studentDeletePacket));
	AfxMessageBox(_T("学生信息已删除"));
}

void CStudentManagerDlg::OnUpdatestu()
{
	// TODO: 在此添加命令处理程序代码
	int nItem = m_QueryList.GetSelectionMark();
	if (nItem == -1) {
		AfxMessageBox(_T("请先选择要删除的学生信息！"));
		return;
	}

	CString studentId = m_QueryList.GetItemText(nItem, 0);
	CString studentName = m_QueryList.GetItemText(nItem, 1);
	CString courseID = m_QueryList.GetItemText(nItem, 3);
	m_StudentNameEdit.SetWindowText(studentName);
	m_CourseIDEdit.SetWindowText(courseID);
	m_StudentId.SetWindowText(studentId);
	m_UpdateRadio.SetCheck(BST_CHECKED);
	m_SaveRadio.SetCheck(BST_UNCHECKED);
}

void CStudentManagerDlg::OnBnClickedSaveRadioButton()
{
	// TODO: 在此添加控件通知处理程序代码
	m_StudentId.EnableWindow(TRUE);
	m_StudentNameEdit.EnableWindow(TRUE);
	m_CourseIDEdit.EnableWindow(TRUE);
	m_StudentId.SetWindowTextA("");
}

void CStudentManagerDlg::OnBnClickedRadio2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_StudentId.EnableWindow(FALSE);

}

void CStudentManagerDlg::OnBnClickedExecute()
{
	CString studentName, courseID, studentID;
	m_StudentNameEdit.GetWindowText(studentName);
	m_CourseIDEdit.GetWindowText(courseID);
	m_StudentId.GetWindowText(studentID);
	if (studentName.IsEmpty() || courseID.IsEmpty()) {
		AfxMessageBox(_T("学生姓名和课程ID不能为空！"));
		return;
	}
	
	//清空输入
	m_StudentNameEdit.SetWindowTextA("");
	m_CourseIDEdit.SetWindowTextA("");
	m_StudentId.SetWindowTextA("");
	if (m_SaveRadio.GetCheck() == BST_CHECKED) {
		// 保存模式
		if (!studentID.IsEmpty()) {
			AfxMessageBox(_T("保存模式下不需要输入学生ID！"));
			return;
		}
		StudentAdd studentAddPacket;
		studentAddPacket.m_szStuName.Init(studentName.GetString());
		studentAddPacket.m_CourseID = _ttoi(courseID); // 将CString转换为整数
		KCP::GetInstance().Send((char*)&studentAddPacket, sizeof(studentAddPacket));
		ClientStudentQueryPacket Clientheader;
		Clientheader.m_szStuName.Init("");
		Clientheader.m_szCourseName.Init("");
		KCP::GetInstance().Send(&Clientheader, sizeof(Clientheader));
		AfxMessageBox(_T("学生信息已保存"));
		m_SaveRadio.SetCheck(BST_UNCHECKED);
		return;
	}
	else if (m_UpdateRadio.GetCheck() == BST_CHECKED) {
		 // 更新模式
		if (studentID.IsEmpty()) {
			AfxMessageBox(_T("更新模式下需要输入学生ID！"));
			return;
		}
		StudentUpdate studentUpdatePacket;
		studentUpdatePacket.m_szStuName.Init(studentName.GetString());
		studentUpdatePacket.m_CourseID = _ttoi(courseID); // 将CString转换为整数
		studentUpdatePacket.m_nStuID = _ttoi(studentID); // 将CString转换为整数
		KCP::GetInstance().Send((char*)&studentUpdatePacket, sizeof(studentUpdatePacket));
		ClientStudentQueryPacket Clientheader;
		Clientheader.m_szStuName.Init("");
		Clientheader.m_szCourseName.Init("");
		KCP::GetInstance().Send(&Clientheader, sizeof(Clientheader));
		AfxMessageBox("学生信息更新完毕");
		m_UpdateRadio.SetCheck(BST_UNCHECKED);
		return;
	}
	else {
		AfxMessageBox(_T("请选择操作模式！"));
		return;
	}
	
}

void CStudentManagerDlg::OnBnClickedButton3()
{
	//清空数据库
	
}
