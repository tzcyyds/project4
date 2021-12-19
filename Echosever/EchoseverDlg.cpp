
// EchoseverDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Echosever.h"
#include "EchoseverDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT __cdecl WorkThreadFunction(LPVOID pParam) {
	SOCKET hsocket = (SOCKET)pParam;
	return 0;
}

UINT __cdecl ListenThreadFunction(LPVOID pParam)
{
	CEchoseverDlg* pdlg = (CEchoseverDlg*)pParam;

	WSADATA wsaData;
	SOCKADDR_IN servAdr{}, clntAdr{};
	int len = sizeof(clntAdr);
	//memset(&servAdr, 0, sizeof(servAdr));
	//memset(&clntAdr, 0, sizeof(clntAdr));
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		exit(1);
	}
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(pdlg->m_port);
	SOCKET hListenSock;
	hListenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hListenSock == INVALID_SOCKET)
	{
		exit(1);
	}
	if (bind(hListenSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
	{
		exit(1);
	}
	if (listen(hListenSock, 5) == SOCKET_ERROR)//立即返回
	{
		exit(1);
	}
	SOCKET hCommSock;
	while (1) {
		hCommSock = accept(hListenSock, (sockaddr*)&clntAdr, &len);//阻塞！
		if (hCommSock == SOCKET_ERROR)
		{
			closesocket(hListenSock);
			break;
		}
		else
		{
			AfxBeginThread(WorkThreadFunction, (LPVOID)hCommSock);
			pdlg->m_threads.AddString("work thread");
		}
	}

	closesocket(hListenSock);
	WSACleanup();
	return 0;
}
// CEchoseverDlg 对话框



CEchoseverDlg::CEchoseverDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ECHOSEVER_DIALOG, pParent)
	, m_port(9190)
	, m_cur(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEchoseverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MODE, m_mode);
	DDX_Text(pDX, IDC_EDIT1, m_port);
	DDX_Control(pDX, IDC_LIST2, m_threads);
}

BEGIN_MESSAGE_MAP(CEchoseverDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CEchoseverDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CEchoseverDlg 消息处理程序

BOOL CEchoseverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_mode.InsertString(0,"多线程模式");
	m_mode.InsertString(1,"select框架");
	m_mode.InsertString(2,"多线程+select");
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEchoseverDlg::OnPaint()
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
HCURSOR CEchoseverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CEchoseverDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(1);
	m_cur = m_mode.GetCurSel();


	switch (m_cur)
	{
	case 0://多线程,先创建一个监听线程
	{
		AfxBeginThread(ListenThreadFunction, (LPVOID)this);
		m_threads.AddString("listen thread");
	}
		break;
	case 1://select
		break;
	case 2://多线程+select
		break;
	default:
		break;
	}
}


LRESULT CEchoseverDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	switch (m_cur)
	{
	case 0://多线程
		break;
	case 1://select
		break;
	case 2://多线程+select
		break;
	default:
		break;
	}
	return CDialogEx::WindowProc(message, wParam, lParam);
}
