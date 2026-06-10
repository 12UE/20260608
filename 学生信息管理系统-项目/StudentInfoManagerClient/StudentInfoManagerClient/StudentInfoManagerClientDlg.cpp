
// StudentInfoManagerClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "StudentInfoManagerClient.h"
#include "StudentInfoManagerClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CStudentInfoManagerClientDlg 对话框



PacketHeader* CStudentInfoManagerClientDlg::RecvPkg()
{
	//先收包头
	PacketHeader hdr;
	KCP::GetInstance().StickRecv((char*)&hdr, sizeof(hdr));

	char* buf = new char[sizeof(hdr) + hdr.m_length];
	if (buf == nullptr)
	{
		return nullptr;
	}
	memcpy(buf, &hdr, sizeof(hdr));

	if (hdr.m_length <= 0)
	{
		return nullptr;
	}

	//再收包体
	KCP::GetInstance().StickRecv(buf + sizeof(hdr), hdr.m_length);

	return (PacketHeader*)buf;
}

CStudentInfoManagerClientDlg::CStudentInfoManagerClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STUDENTINFOMANAGERCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStudentInfoManagerClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab);
}

BEGIN_MESSAGE_MAP(CStudentInfoManagerClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CStudentInfoManagerClientDlg 消息处理程序

BOOL CStudentInfoManagerClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_SHOW);

	// TODO: 在此添加额外的初始化代码
	m_tab.AddPage("学生管理", &m_StudentDlg, IDD_STUDENTDLG);
	m_tab.AddPage("班级管理", &m_ClassDlg, IDD_CLASSDLG);
	m_tab.AddPage("课程管理", &m_CourseDlg, IDD_COURSEDLG);
	m_tab.AddPage("选课管理", &m_EnrollDlg, IDD_ENROLLDLG);
	m_tab.AddPage("组合查询", &m_CombineDlg, IDD_COMBINEQUERY);
	m_tab.Show();
	KCP::GetInstance().Connect("192.168.3.3", 9527);
	//设置套接字响应消息
	std::thread([&]() {
		while (true)
		{
			//接收数据包
			// KCP::GetInstance().StickRecv((CHAR*)&header, sizeof(header));
			PacketHeader* pPkg = RecvPkg();
			switch (pPkg->m_PkgType) {
			case S2C_STUDENT: {
				if (pPkg->m_op == RESPONSEPKG) {
					m_StudentDlg.OnReceiveStudentInfo((PkgS2CStudent*)pPkg);
				}
				break;
			}
			case S2C_CLASS: {
				if(pPkg->m_op == RESPONSEPKG) {
					m_ClassDlg.OnReceiveClassInfo((PkgS2CClass*)pPkg);
				}
				break;
			}
			case S2C_COURSE:{
				if (pPkg->m_op == RESPONSEPKG) {
					m_CourseDlg.OnReceiveCourseInfo((PkgS2CCourse*)pPkg);
				}
				break;
			}
			case S2C_ENROLL: {
				if (pPkg->m_op == RESPONSEPKG) {
					m_EnrollDlg.OnReceiveEnrollInfo((PkgS2CEnroll*)pPkg);
				}
			}
				break;
			case S2C_COMBINE: {
				if (pPkg->m_op == RESPONSEPKG) {
					m_CombineDlg.OnReceiveCombineInfo((PkgS2CCombine*)pPkg);
				}
			}
				break;
			default:
				
				break;
			}
			
		}
	}).detach();
	//申请控制台
	AllocConsole();
	//重定向控制台输入输出
	FILE* fp;
	freopen_s(&fp, "CONIN$", "r", stdin);  // 重定向标准输入
	freopen_s(&fp, "CONOUT$", "w", stdout); // 重定向标准输出
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CStudentInfoManagerClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CStudentInfoManagerClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


