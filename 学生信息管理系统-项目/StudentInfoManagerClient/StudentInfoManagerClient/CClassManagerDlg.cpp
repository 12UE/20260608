// CClassManagerDlg.cpp: 实现文件
//

#include "pch.h"
#include "StudentInfoManagerClient.h"
#include "afxdialogex.h"
#include "CClassManagerDlg.h"
#include"resource.h"

// CClassManagerDlg 对话框

IMPLEMENT_DYNAMIC(CClassManagerDlg, CDialogEx)

CClassManagerDlg::CClassManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLASSDLG, pParent)
{

}

CClassManagerDlg::~CClassManagerDlg()
{
}

void CClassManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLASSQUERYLIST, m_QueryList);
	DDX_Control(pDX, IDC_EDIT2, m_QueryClassEdit);
	DDX_Control(pDX, IDC_EDIT7, m_ClassId);
	DDX_Control(pDX, IDC_EDIT1, m_ClassName);
	DDX_Control(pDX, IDC_CLASS_UPDATE_RADIO, m_UpdateRadio);
	DDX_Control(pDX, IDC_CLASS_INSERT_RADIO, m_AddRadio);
}


BEGIN_MESSAGE_MAP(CClassManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &CClassManagerDlg::OnBnClickedQuery)
	ON_BN_CLICKED(IDC_BUTTON4, &CClassManagerDlg::OnBnClickedClear)
	ON_NOTIFY(NM_RCLICK, IDC_CLASSQUERYLIST, &CClassManagerDlg::OnNMRClickQuerylist)
	ON_COMMAND(ID_POP_CLASS_DEL, &CClassManagerDlg::OnPopClassDel)
	ON_COMMAND(ID_POP_CLASS_MODIFY, &CClassManagerDlg::OnPopClassModify)
	ON_BN_CLICKED(IDC_BUTTON1, &CClassManagerDlg::OnBnClickedExecute)
	ON_BN_CLICKED(IDC_CLASS_INSERT_RADIO, &CClassManagerDlg::OnBnClickedClassInsertRadio)
	ON_BN_DOUBLECLICKED(IDC_CLASS_INSERT_RADIO, &CClassManagerDlg::OnBnDoubleclickedClassInsertRadio)
END_MESSAGE_MAP()


// CClassManagerDlg 消息处理程序

BOOL CClassManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	//班级 班级名称 学生数量
	m_QueryList.InsertColumn(0, "班级ID", LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(1, "班级名称", LVCFMT_LEFT, 100);
	m_QueryList.InsertColumn(2, "学生数量", LVCFMT_LEFT, 100);
	m_QueryList.SetExtendedStyle(
		m_QueryList.GetExtendedStyle() |
		LVS_EX_FULLROWSELECT |  // 全行选中
		LVS_EX_GRIDLINES       // 显示网格线
	);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CClassManagerDlg::OnReceiveClassInfo(PkgS2CClass* pPkg)
{
	m_QueryList.DeleteAllItems();
	if (pPkg == nullptr || pPkg->m_nCount <= 0) {
		AfxMessageBox(_T("没有查询到相关班级信息"));
		return;
	}
	for (size_t i = 0; i < pPkg->m_nCount; i++)
	{
		auto& info = pPkg->m_classInfo[i];
		m_QueryList.InsertItem(i, std::to_string(info.m_nClassID).c_str());
		m_QueryList.SetItemText(i, 1, info.m_szClassName);
		m_QueryList.SetItemText(i, 2, std::to_string(info.m_nStudentCount).c_str());
	}
}

void CClassManagerDlg::OnBnClickedQuery()
{
	CString szClassName;
	ClassQueryPacket * classQueryPacket = new ClassQueryPacket;
	m_QueryClassEdit.GetWindowText(szClassName);
	classQueryPacket->m_szClassName.Init(szClassName.GetString());
	CString szStudentName;
	m_QueryClassEdit.SetWindowText("");
	KCP::GetInstance().Send((char*)classQueryPacket, sizeof(ClassQueryPacket));
}

void CClassManagerDlg::OnBnClickedClear()
{
	m_QueryList.DeleteAllItems(); // 清空列表控件
	AfxMessageBox(_T("已清空查询结果列表"));
}

void CClassManagerDlg::OnNMRClickQuerylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CPoint point;
	GetCursorPos(&point);  // 获取屏幕坐标

	// 获取列表控件
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_CLASSQUERYLIST);
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
			if (menu.LoadMenu(IDR_CLASS_POP_MENU))
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

void CClassManagerDlg::OnPopClassDel()
{
	int nSelected = m_QueryList.GetSelectionMark();
	if (nSelected == -1) {
		AfxMessageBox("选择一个项目");
		return;
	}
	ClassDelete DeleteInfo;
	CString classID = m_QueryList.GetItemText(nSelected, 0);
	DeleteInfo.m_nClassID = _ttoi(classID); // 将CString转换为整数
	KCP::GetInstance().Send((char*)&DeleteInfo, sizeof(DeleteInfo));
	m_QueryList.DeleteItem(nSelected); // 删除选中的行
	AfxMessageBox(_T("班级信息已删除"));
	// TODO: 在此添加命令处理程序代码
}

void CClassManagerDlg::OnPopClassModify()
{
	// TODO: 在此添加命令处理程序代码
	int nSelected = m_QueryList.GetSelectionMark();
	if (nSelected == -1) {
		AfxMessageBox("选择一个项目");
		return;
	}
	CString classID = m_QueryList.GetItemText(nSelected, 0);
	CString className = m_QueryList.GetItemText(nSelected, 1);
	m_ClassId.SetWindowText(classID);
	m_ClassName.SetWindowText(className);
	m_UpdateRadio.SetCheck(BST_CHECKED); // 设置更新单选按钮为选中状态
	m_AddRadio.SetCheck(BST_UNCHECKED); // 确保添加单选按钮未选中

}

void CClassManagerDlg::OnBnClickedExecute()
{
	CString szClassName;
	m_ClassName.GetWindowText(szClassName);
	if (szClassName.IsEmpty()) {
		AfxMessageBox(_T("班级名称不能为空"));
		return;
	}
	if( m_UpdateRadio.GetCheck() == BST_CHECKED) {
		// 更新模式
		CString szClassId;
		m_ClassId.GetWindowTextA(szClassId);
		if (szClassId.IsEmpty()) {
			AfxMessageBox("更新模式下不能为空");
			return;
		}
		ClassUpdate* classupdate = new ClassUpdate;
		classupdate->m_nClassID = _ttoi(szClassId);
		classupdate->m_ClassName.Init(szClassName.GetString());
		KCP::GetInstance().Send((char*)classupdate, sizeof(ClassUpdate));
		//更细列表
		ClassQueryPacket* classQueryPacket = new ClassQueryPacket;
		classQueryPacket->m_szClassName.Init("");
		KCP::GetInstance().Send((char*)classQueryPacket, sizeof(ClassQueryPacket));
		AfxMessageBox(_T("班级信息已更新"));
	} else if (m_AddRadio.GetCheck() == BST_CHECKED) {
		// 添加模式
		ClassAdd* addPacket=new ClassAdd();
		addPacket->m_szClassName.Init(szClassName.GetString());
		KCP::GetInstance().Send((char*)addPacket, sizeof(ClassAdd));
		//更细列表
		ClassQueryPacket* classQueryPacket = new ClassQueryPacket;
		classQueryPacket->m_szClassName.Init("");
		KCP::GetInstance().Send((char*)classQueryPacket, sizeof(ClassQueryPacket));
		m_AddRadio.SetCheck(BST_UNCHECKED);
		AfxMessageBox(_T("班级信息已添加"));
	}
	
}

void CClassManagerDlg::OnBnClickedClassInsertRadio()
{


	// 清除相关编辑框内容
	m_ClassId.SetWindowTextA("");

	// 确保另一个单选按钮未被选中（可选）
	m_UpdateRadio.SetCheck(BST_UNCHECKED);
}

void CClassManagerDlg::OnBnDoubleclickedClassInsertRadio()
{
	// TODO: 在此添加控件通知处理程序代码
	m_AddRadio.SetCheck(BST_UNCHECKED); // 设置添加单选按钮为选中状态
}
