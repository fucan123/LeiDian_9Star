
// WebUIDlg.h: 头文件
//

#pragma once
#include "Web/CWebBrowser2.h"
#include "Web/JsCall.h"
#include "Web/JsCallCpp.h"
#include "Web/WebList.h"

struct my_msg {
	char id[32];
	char text[64];
	char cla[32];
	int  value;
	int  value2;
};

#define COPY_MSG(p, i, t, c, v) { \
    ZeroMemory(p, sizeof(my_msg)); \
    strcpy(p->id, i); \
	strcpy(p->text, t); \
    if (c) strcpy(p->cla, c); \
	p->value = v; \
}

class Game;
// CWebUIDlg 对话框
class CWebUIDlg : public CDialogEx
{
// 构造
public:
	CWebUIDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WEBUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	LRESULT OnSetText(WPARAM w, LPARAM l);
	LRESULT OnAddLog(WPARAM w, LPARAM l);
	LRESULT OnUpdateStatusText(WPARAM w, LPARAM l);
	LRESULT OnUpdateTableText(WPARAM w, LPARAM l);
	LRESULT OnUpdateVerOk(WPARAM w, LPARAM l);

	HHOOK m_hk;
	static DWORD WINAPI KeyBoardHook(LPVOID);
	// 键盘钩子
	static LRESULT CALLBACK KeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	// 检查是否激活
	static DWORD WINAPI Verify(LPVOID);
	// 检查更新版本
	static DWORD WINAPI UpdateVer(LPVOID);
	// 监听
	static DWORD WINAPI Listen(LPVOID);
	// 检测模拟器
	static DWORD WINAPI CheckMNQ(LPVOID);
	// 执行流程
	static DWORD WINAPI Run(LPVOID);
public:
	Game* m_pGame;

	CWebBrowser2 m_Web;
	JsCall m_JsCall;
	JsCallCpp m_JsCallCpp;
	WebList m_Account;
	WebList m_Log;

	// 配置路径
	char m_ConfPath[255];

	DECLARE_EVENTSINK_MAP()
	void DocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT* URL);
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
};

extern CWebUIDlg* g_UI;