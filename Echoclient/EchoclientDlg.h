
// EchoclientDlg.h: 头文件
//

#pragma once
#include <random>
#include <chrono>
#include <vector>
// CEchoclientDlg 对话框
class CEchoclientDlg : public CDialogEx
{
// 构造
public:
	CEchoclientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ECHOCLIENT_DIALOG };
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
	UINT ThreadNum;
	UINT m_port;
	SOCKADDR_IN servAdr{};
	afx_msg void OnBnClickedButton1();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	DWORD m_ip;
	std::vector<double> SendRate;
	CCriticalSection critical_section;
	CListBox m_RateList;
};
