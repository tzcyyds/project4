
// EchoclientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Echoclient.h"
#include "EchoclientDlg.h"
#include "afxdialogex.h"

constexpr auto BUF_SIZE = 256;
constexpr auto MAX_TIMES = 4096;
constexpr auto MAX_THREAD_NUM = 64;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT __stdcall WorkThreadFunction(LPVOID pParam) {
	CEchoclientDlg* pdlg = (CEchoclientDlg*)pParam;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		exit(1);
	}
	SOCKET hCommSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hCommSock == INVALID_SOCKET)
	{
		exit(1);
	}

	if (connect(hCommSock, 
		(SOCKADDR*)&(pdlg->servAdr), 
		sizeof(pdlg->servAdr)) == SOCKET_ERROR)//隐式绑定，连接服务器
	{
		exit(1);
	}
	std::default_random_engine e;
	char msg[BUF_SIZE];
	int sendLen = 0, recvLen = 0;
	for (size_t i = 0; i < MAX_TIMES; i++)
	{
		using namespace std::chrono;
		sprintf_s(msg, "%d", e());
		//假定随机数小于BUF_SIZE，可以一次发完
		sendLen = send(hCommSock, msg, sizeof(msg), 0);
		auto t1 = steady_clock::now();
		if (sendLen <= 0) {
			TRACE("send error");
		}
		//但一次不一定可以收完
		int tmp = 0;
		do
		{
			tmp = recv(hCommSock, msg, sizeof(msg), 0);
			if (tmp > 0)recvLen += tmp;
			else TRACE("waiting");
		} while (recvLen < sendLen);
		//记录时间和流量
		auto t2 = steady_clock::now();
		auto time_span = duration_cast<microseconds>(t2 - t1);//精确到微秒级
		double s_MB = sendLen * static_cast<double>(8) / 1024 / 1024;
		double m_rate = s_MB / (double(time_span.count()) * microseconds::period::num / microseconds::period::den);
		pdlg->critical_section.Lock();
		//pdlg->SendRate.push_back(m_rate);
		char ratestr[20];//这里的缓冲应该足够大，
		sprintf_s(ratestr, "%.5f MB/s", m_rate);
		pdlg->m_RateList.AddString((LPCTSTR)ratestr);
		pdlg->critical_section.Unlock();

	}
	closesocket(hCommSock);
	WSACleanup();
	return 0;
}

// CEchoclientDlg 对话框



CEchoclientDlg::CEchoclientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ECHOCLIENT_DIALOG, pParent)
	, ThreadNum(30)
	, m_port(9190)
	, m_ip(0x7f000001)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEchoclientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TNUM, ThreadNum);
	DDX_Text(pDX, IDC_PORT, m_port);
	DDX_IPAddress(pDX, IDC_IPADDRESS1, m_ip);
	DDX_Control(pDX, IDC_LIST2, m_RateList);
}

BEGIN_MESSAGE_MAP(CEchoclientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CEchoclientDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CEchoclientDlg 消息处理程序

BOOL CEchoclientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEchoclientDlg::OnPaint()
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
HCURSOR CEchoclientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CEchoclientDlg::OnBnClickedButton1()
{
	UpdateData(TRUE);
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(m_ip);
	servAdr.sin_port = htons(m_port);
	SendRate.clear();
	HANDLE handle[MAX_THREAD_NUM]{};
	for (size_t i = 0; i < ThreadNum; i++)
	{
		handle[i] = (HANDLE)
			_beginthreadex(NULL, 0, WorkThreadFunction, (LPVOID)this, 0, NULL);
		//AfxBeginThread(WorkThreadFunction, );
	}
	return;
}


LRESULT CEchoclientDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CDialogEx::WindowProc(message, wParam, lParam);
}
