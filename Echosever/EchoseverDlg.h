
// EchoseverDlg.h: 头文件
//

#pragma once


// CEchoseverDlg 对话框
class CEchoseverDlg : public CDialogEx
{
// 构造
public:
	CEchoseverDlg(CWnd* pParent = nullptr);	// 标准构造函数
	~CEchoseverDlg();	// 析构函数
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ECHOSEVER_DIALOG };
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
	afx_msg void OnBnClickedButton1();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	CComboBox m_mode;
	UINT m_port;
	UINT m_cur;
	CListBox m_threads;

	SOCKADDR_IN servAdr{};
	WSADATA wsaData;
	SOCKET hServSock;
};

inline void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
	int i;
	for (i = idx; i < total; i++)
		hSockArr[i] = hSockArr[i + 1];
}
inline void CompressEvents(WSAEVENT hEventArr[], int idx, int total)
{
	int i;
	for (i = idx; i < total; i++)
		hEventArr[i] = hEventArr[i + 1];
}
inline void ErrorHandling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}