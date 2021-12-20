
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

#define WM_SOCK WM_USER + 100// 自定义消息，为避免冲突，最好100以上
#define BUF_SIZE 100
#define MY_MAXSOCK 20

UINT __cdecl WorkThread0(LPVOID pParam) {
	SOCKET hsocket = (SOCKET)pParam;
	//阻塞收发
	return 0;
}

UINT __cdecl ListenThread0(LPVOID pParam)
{
	CEchoseverDlg* pdlg = (CEchoseverDlg*)pParam;
	//阻塞监听，节省CPU
	SOCKET hCommSock;
	SOCKADDR_IN clntAdr{};
	int len = sizeof(clntAdr);
	
	while (1) {
		hCommSock = accept(pdlg->hServSock, (sockaddr*)&clntAdr, &len);//阻塞！
		if (hCommSock == SOCKET_ERROR)
		{
			closesocket(pdlg->hServSock);
			break;
		}
		else //多线程，每个socket一个thread
		{
			AfxBeginThread(WorkThread0, (LPVOID)hCommSock);
			pdlg->m_threads.AddString("work thread0");
		}
	}
	//closesocket(pdlg->hServSock);
	return 0;
}
UINT __cdecl ListenThread2(LPVOID pParam) {
	CEchoseverDlg* pdlg = (CEchoseverDlg*)pParam;

	
	SOCKET hClntSock = 0;
	SOCKADDR_IN clntAdr{};
	int clntAdrLen = sizeof(clntAdr);

	SOCKET hSockArr[MY_MAXSOCK]{};
	WSAEVENT hEventArr[MY_MAXSOCK]{};//原来是WSA_MAXIMUM_WAIT_EVENTS，64个,想改小点
	WSAEVENT newEvent;
	WSANETWORKEVENTS netEvents;

	int numOfClntSock = 0;
	int strLen, i;
	int posInfo, startIdx;

	char msg[BUF_SIZE];

	newEvent = WSACreateEvent();
	if (WSAEventSelect(pdlg->hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
		ErrorHandling("WSAEventSelect() error");

	hSockArr[numOfClntSock] = pdlg->hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while (1)
	{
		posInfo = WSAWaitForMultipleEvents(
			numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);
		startIdx = posInfo - WSA_WAIT_EVENT_0;
		if (startIdx == 0) //因为只注册了FD_ACCEPT，说明需要accept，单独处理
		{
			if (numOfClntSock == MY_MAXSOCK) 
			{
				//如果溢出，开其它线程继续接收连接，把S_socket用0代替，取消注册，但不能删除，只让它在此线程中它沉默
				//但允许处理数据
				AfxBeginThread(ListenThread2, pParam);
				pdlg->m_threads.AddString("listen thread2");

				WSAEventSelect(pdlg->hServSock, newEvent, 0);//取消注册
				WSACloseEvent(hEventArr[0]);//关闭事件
				hSockArr[0] = 0;
				hEventArr[0] = 0;
			}
			else 
			{	//如果不溢出，就正常接收
				WSAEnumNetworkEvents(
					hSockArr[0], hEventArr[0], &netEvents);

				if (netEvents.lNetworkEvents & FD_ACCEPT)
				{
					if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						TRACE("Accept Error");
						break;
					}
					hClntSock = accept(
						hSockArr[0], (SOCKADDR*)&clntAdr, &clntAdrLen);
					newEvent = WSACreateEvent();
					WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

					hEventArr[numOfClntSock] = newEvent;
					hSockArr[numOfClntSock] = hClntSock;
					numOfClntSock++;
					TRACE("connected new client...");
				}
			}
			startIdx = 1;//1
		}
		for (i = startIdx; i < numOfClntSock; i++)
		{
			int sigEventIdx =
				WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);
			if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
			{
				continue;
			}
			else
			{
				sigEventIdx = i;
				WSAEnumNetworkEvents(
					hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);
				
				if (netEvents.lNetworkEvents & FD_READ)
				{
					if (netEvents.iErrorCode[FD_READ_BIT] != 0)
					{
						TRACE("Read Error");
						break;
					}
					strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);
					send(hSockArr[sigEventIdx], msg, strLen, 0);
				}

				if (netEvents.lNetworkEvents & FD_CLOSE)
				{
					if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0)
					{
						TRACE("Close Error");
						break;
					}
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);

					numOfClntSock--;//一般情况下不会溢出的
					CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
					CompressEvents(hEventArr, sigEventIdx, numOfClntSock);
				}
			}
		}
	}
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

CEchoseverDlg::~CEchoseverDlg()
{
	closesocket(hServSock);
	WSACleanup();
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

	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(m_port);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hServSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("INVALID_SOCKET");
	if (bind(hServSock, (SOCKADDR*)&(servAdr), sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	switch (m_cur)
	{
	case 0://多线程,先创建一个监听线程
	{
		AfxBeginThread(ListenThread0, (LPVOID)this);
		m_threads.AddString("listen thread0");
	}
		break;
	case 1://select
	{
		WSAAsyncSelect(hServSock, m_hWnd, WM_SOCK, FD_ACCEPT | FD_READ | FD_CLOSE);
	}
		break;
	case 2://多线程+select  开启多个线程，每个线程里用slect，获得更大的服务量！
	{
		AfxBeginThread(ListenThread2, (LPVOID)this);
		m_threads.AddString("listen thread2");
	}
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
	case 0://多线程，不处理
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
