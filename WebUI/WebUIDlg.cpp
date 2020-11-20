
// WebUIDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WebUI.h"
#include "WebUIDlg.h"
#include "afxdialogex.h"
#include "Game/Game.h"
#include "Game/Emulator.h"
#include "Game/GameProc.h"
#include "Game/Home.h"
#include "Game/Driver.h"
#include "Game/DownFile.h"
#include "Game/PrintScreen.h"
#include "Web/WebList.h"
#include <My/Common/func.h>
#include <My/Common/Explode.h>
#include <My/Common/MachineID.h>
#include <My/Driver/KbdMou.h>
#include <My/Db/Sqlite.h>
#include <My/Win32/PE.h>
#include <My/Win32/Peb.h>
#include <My/Common/C.h>
#include <MsHTML.h>

#define MSG_SETTEXT      (WM_USER+100)
#define MSG_ADDLOG       (WM_USER+101)
#define MSG_UPSTATUSTEXT (WM_USER+102)
#define MSG_UPTABLETEXT  (WM_USER+103)
#define MSG_UPVEROK      (WM_USER+200)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWebUIDlg 对话框


CWebUIDlg* g_UI = nullptr;

CWebUIDlg::CWebUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WEBUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_UI = this;
}

void CWebUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPLORER1, m_Web);
}

BEGIN_MESSAGE_MAP(CWebUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(MSG_SETTEXT, &CWebUIDlg::OnSetText)
	ON_MESSAGE(MSG_ADDLOG, &CWebUIDlg::OnAddLog)
	ON_MESSAGE(MSG_UPSTATUSTEXT, &CWebUIDlg::OnUpdateStatusText)
	ON_MESSAGE(MSG_UPTABLETEXT, &CWebUIDlg::OnUpdateTableText)
	ON_MESSAGE(MSG_UPVEROK, &CWebUIDlg::OnUpdateVerOk)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_HOTKEY()
END_MESSAGE_MAP()


// CWebUIDlg 消息处理程序

BOOL CWebUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	RegisterHotKey(m_hWnd, 1001, NULL, VK_F1);
	RegisterHotKey(m_hWnd, 1002, NULL, VK_F2);

	bool adr = AdjustPrivileges();
	::printf("提权状态结果:%d\n", adr);

	DWORD adbs[10];
	DWORD adbs_l = SGetProcessIds(L"adb.exe", adbs, sizeof(adbs) / sizeof(DWORD));
	for (int i = 0; i < adbs_l; i++) {
		char cmd[64];
		sprintf_s(cmd, "taskkill /f /t /pid %d", adbs[i]);
		system(cmd);
	}
	adbs_l = SGetProcessIds(L"conhost.exe", adbs, sizeof(adbs) / sizeof(DWORD));
	for (int i = 0; i < adbs_l; i++) {
		char cmd[64];
		sprintf_s(cmd, "taskkill /f /t /pid %d", adbs[i]);
		//system(cmd);
	}

#if 1
	AllocConsole();
	freopen("CON", "w", stdout);
#endif
#ifdef  _DEBUG
	//AllocConsole();
	//freopen("CON", "w", stdout);

	SHGetSpecialFolderPathA(0, m_ConfPath, CSIDL_DESKTOPDIRECTORY, 0);
	strcat(m_ConfPath, "\\MNQ-9Star");
#else
	pfnNtQuerySetInformationThread f = (pfnNtQuerySetInformationThread)GetNtdllProcAddress("ZwSetInformationThread");
	NTSTATUS sta = f(GetCurrentThread(), ThreadHideFromDebugger, NULL, 0);
	::printf("sta:%d\n", sta);

	GetCurrentDirectoryA(MAX_PATH, m_ConfPath);
#endif //  _DEBUG

	char title[16] = { 0 };
	RandStr(title, 6, 12, 0);
	SetWindowTextA(m_hWnd, title);

	m_pGame = new Game(&m_JsCall, "table_1", "log_ul", m_ConfPath, title, m_hWnd);

	HWND mnq = ::FindWindow(NULL, L"雷电模拟器");
	if (mnq) {
		HWND RenderWindow = ::FindWindowEx(mnq, NULL, L"RenderWindow", NULL);
		m_pGame->m_hWndBig = RenderWindow;

		DWORD tick = GetTickCount();
		PrintScreen* ps = m_pGame->m_pPrintScreen;

		HBITMAP bitmap;
		// 672, 158, 1170, 620
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 600, 500, 610, 510, 1000, false); // 聊天窗口2是否打开
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 672, 158, 1170, 620, 100, false); // 背包
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 1150, 75, 1160, 85, 100, false); // 背包, 关闭按钮前面一点
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 145, 85, 155, 95, 100, false); // 物品操作按钮 第一行
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 466, 430, 476, 440, 100, false); // 确认框 取消按钮
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 466, 505, 476, 515, 100, false); // 拆分框 取消按钮
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 1015, 578, 1025, 588, 100, false); // 物品
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 715, 240, 725, 242, 100, false); // 四百点图鉴卡
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 320, 82, 375, 145, 100, false); // 点击物品出来操作按钮旁边的图标
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 366, 466, 376, 476, 100, false); // 社交信息
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 503, 40, 513, 50, 100, false); // 多个NPC
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 350, 0, 1280, 350, 100, false); // 关闭按钮
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 855, 376, 865, 386, 100, false); // 物品使用提示框
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 30, 372, 40, 382, 100, false); // 对话框选择1
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 30, 444, 40, 454, 100, false); // 对话框选择2
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 30, 516, 40, 526, 100, false); // 对话框选择3
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 1015, 666, 1050, 680, 100, false); // 星陨
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 1005, 562, 1035, 575, 100, false); // 影魂契约
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 1166, 453, 1196, 466, 100, false); // 虚无空间
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 1060, 460, 1090, 472, 5000, false); // 最终审判
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 765, 187, 775, 197, 100, false); // 系统邀请提示框
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 60, 690, 186, 706, 100, false); // 左下角时间或电池
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 460, 50, 470, 60, 100, false); // 公共框
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 705, 215, 715, 225, 100, false); // 25星XO礼包
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 886, 355, 896, 365, 100, false); // 对话按钮
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 42, 682, 52, 692, 100, false); // 聊天窗口是否打开
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 100, 620, 110, 630, 1000, false); // 聊天窗口2是否打开
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 575, 70, 585, 80, 100, false); // 神殿练功场窗口领取经验窗口
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 925, 620, 970, 640, 100, false); // 快捷栏物品数量
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 925, 520, 970, 640, 100, false); // 快捷栏物品数量
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 926, 510, 936, 520, 100, false); // 快捷栏物品数量
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 1235, 350, 1245, 360, 100, false); // 快捷栏切换按钮
		//bitmap = ps.CopyScreenToBitmap(m_pGame->m_hWndBig, 113, 158, 620, 620, 100); // 仓库 
		//bitmap = ps.CopyScreenToBitmap(RenderWindow, 62, 162, 305, 168, 100, false); // 宠物
		//bitmap = ps.CopyScreenToBitmap(RenderWindow, 245, 150, 280, 175, 100, false); // 最后一个宠物
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 908, 602, 923, 617, 100, false); // 快捷栏上星辰之眼
		//bitmap = ps.CopyScreenToBitmap(RenderWindow, 100, 320, 110, 323, 100, false); // 对话框
		//bitmap = ps.CopyScreenToBitmap(RenderWindow, 1112, 6, 1127, 21, 100, false); // 副本
		//bitmap = ps.CopyScreenToBitmap(RenderWindow, 1205, 140, 1220, 155, 100, false); // 登录帐号图标
		//bitmap = ps.CopyScreenToBitmap(RenderWindow, 375, 235, 390, 250, 100, false); // 对话框
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 1205, 140, 1220, 155, 1000, false); // 快捷栏切换按钮
		//bitmap = ps->CopyScreenToBitmap(RenderWindow, 665, 465, 675, 475, 1000, false); // 仓库存钱按钮
		bitmap = ps->CopyScreenToBitmap(RenderWindow, 500, 380, 510, 390, 1000, false); // 仓库格子解锁取消按钮
		ps->SaveBitmapToFile(bitmap, L"C:\\Users\\fucan\\Desktop\\MNQ-9Star\\jt.bmp");
#if 0
		if (ps.CheckPixel(0xFF18191B)) {
			::printf("颜色对比成功\n");
		}
#endif

#if 1
		int count = ps->GetPixelCount(0xFFFFFFFF, 0x00101010, true);
		::printf("count:%d\n", count);
		//int yaobao = ps.LookNum(20, 0, 0, 25, 0xffffffff, 0x003A3A3A, -1, true);
		//int yao = ps.LookNum(0, 0, 95, 0, 0xffffffff, 0x003A3A3A, -1, true);
		//::printf("药包数量:%d 药数量:%d\n", yaobao, yao);

		// 756,228 账号框
		// 720,300 密码框
		// 575,405 登录
		ComPoint cp[32];
		count = ps->CompareImage(CIN_XingChen, cp, sizeof(cp)/sizeof(ComPoint));
		int count2 = ps->ComparePixel("快捷栏切换按钮", cp, sizeof(cp) / sizeof(ComPoint));
		m_pGame->m_pGameProc->Button();
		::printf("count:%d count2:%d\n", count, count2);
		count = ps->GetGrayPiexlCount(false);
		::printf("灰色值数量:%d\n", count);
		count = 0;
		for (int i = 0; i < count; i++) {
			if (i == 0) {
				Sleep(500);
				m_pGame->m_pGameProc->Click(cp[i].x, cp[i].y, 0xff, RenderWindow);
				Sleep(500);
				m_pGame->m_pGameProc->Click(180, 155, 200, 168, 0xff, RenderWindow); // 点击丢弃按钮
				Sleep(500);
				m_pGame->m_pGameProc->Click(700, 430, 750, 450, 0xff, RenderWindow); // 确定按钮
			}

			::printf("物品位置:%d,%d\n", cp[i].x, cp[i].y);
		}
#endif
		// 100,570 105,575
		ps->Release();
		::printf("截图用时:%d毫秒\n", GetTickCount() - tick);

#if 0
		//m_pGame->m_pGameProc->Keyboard('1', 0xff, RenderWindow);
		m_pGame->m_pGameProc->MouseWheel(1000, 500, 250, RenderWindow);
		Sleep(1000);
		m_pGame->m_pGameProc->MouseWheel(1000, 500, -260, RenderWindow);
#endif
#if 0
		Sleep(1000);
		int click_x = 1060, click_y = 375, mv_x = -100, mv_y = -30;
		m_pGame->m_pGameProc->Click(click_x, click_y, 0x01, RenderWindow);
		for (int i = 0; i < 2000; i += 50) {
			m_pGame->m_pGameProc->MouseMove(click_x, click_y, RenderWindow);
			Sleep(50);
		}
		m_pGame->m_pGameProc->MouseMove(click_x, click_y, mv_x, mv_y, RenderWindow);
		while (true) {
			::PostMessage(RenderWindow, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(click_x + mv_x, click_y + mv_y));
			//system("E:\\ChangZhi\\dnplayer2\\ld -s 0 input swipe 1060 375 1030 330 100");
			//system("E:\\ChangZhi\\dnplayer2\\ld -s 0 input tap 1060 370");
			Sleep(500);
		}
		//m_pGame->m_pGameProc->Click(1060, 375, 0x02, RenderWindow);
#if 1
		///for (int i = 1; i < 500;) {
			/*
			::PostMessage(RenderWindow, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(900, 500 - i));
			//::printf("%d,%d\n", 900, 500 - i);
			Sleep(MyRand(2, 7));
			i += MyRand(1, 2);
			*/

			m_pGame->m_pGameProc->MouseMove(900, 500, 0, -485, RenderWindow);
		//}
#else
		Sleep(10);
		::PostMessage(RenderWindow, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(900, 500 - 50));
		Sleep(10);
#endif
		m_pGame->m_pGameProc->Click(900, 500-100, 0x02, RenderWindow);

		bitmap = ps.CopyScreenToBitmap(RenderWindow, 672, 158, 1170, 620);
		//HBITMAP bitmap = ps.CopyScreenToBitmap(RenderWindow, 810, 290, 825, 305);
		ps.SaveBitmapToFile(bitmap, L"C:\\Users\\fucan\\Desktop\\MNQ-9Star\\jt.bmp");
#endif
		while (true) Sleep(1000);
	}

	CString strURL;//htm文件的全路径
	WCHAR chCurtPath[MAX_PATH];//当前目录

	GetCurrentDirectory(MAX_PATH, chCurtPath);//获取当前目录，并存在chCurtPath中
	strURL = "file:///";
	strURL += chCurtPath;

#ifdef  _DEBUG
	strURL += L"/../html/static/index.html";
#else
	strURL += L"/html/static/index.html";
#endif

	//system("D:\\ChangZhi\\dnplayer2\\dnconsole.exe launchex --index 0 --packagename \"com.nd.myht\"");

	CDialog *pDlg = (CDialog *)GetDlgItem(IDC_EXPLORER1);
	CRect rect(0, 0, 800, 750);
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE);
	GetClientRect(rect);
	pDlg->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE);

	::printf("Navigate\n");
	m_Web.Navigate(strURL, NULL, NULL, NULL, NULL);
	::printf("Navigate End...\n");

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CWebUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWebUIDlg::OnPaint()
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
HCURSOR CWebUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BEGIN_EVENTSINK_MAP(CWebUIDlg, CDialogEx)
	ON_EVENT(CWebUIDlg, IDC_EXPLORER1, 259, CWebUIDlg::DocumentCompleteExplorer1, VTS_DISPATCH VTS_PVARIANT)
END_EVENTSINK_MAP()


void CWebUIDlg::DocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT* URL)
{
	// TODO: 在此处添加消息处理程序代码
	//m_pGame = new Game(&m_JsCall, "table_1", "log_ul", m_ConfPath, m_hWnd);
	m_pGame->m_funcUpdateVer = UpdateVer;

	m_JsCallCpp.Init(&m_JsCall, m_pGame);
	m_JsCall.SetDocument(m_Web.get_Document());
	m_Account.Init(&m_JsCall, "table_1");
	m_Log.Init(&m_JsCall, "log_ul");

	m_pGame->ReadConf();
	m_pGame->UpdateFBCountText(0, false);
	m_pGame->UpdateFBTimeLongText(0, 0);

	m_JsCall.SetCppObj(&m_JsCallCpp);

	CreateThread(NULL, NULL, Verify, m_Web.get_Document(), NULL, NULL);
	CreateThread(NULL, NULL, Listen, m_Web.get_Document(), NULL, NULL);
	CreateThread(NULL, NULL, CheckMNQ, m_Web.get_Document(), NULL, NULL);
	CreateThread(NULL, NULL, Run,    m_Web.get_Document(), NULL, NULL);
	//CreateThread(NULL, NULL, KeyBoardHook, m_Web.get_Document(), NULL, NULL);

	SetTimer(2, 8000, NULL);
	/*
	LPDISPATCH l;

	HRESULT r = spDoc->get_Script(&m_Script);
	::printf("r:%d\n", r);

	CComVariant var1 = 10, var2 = '0', varRet = 0;
	r = m_Script.Invoke2(L"Add", &var1, &var2, &varRet);
	::printf("r:%08X %d %d\n", r, varRet.intVal, GetLastError());

	CComVariant v1 = "table_1", v2 = 1, v3 = 0, v4 = 123;
	CComVariant params[] = { "123", "0", 0, "table_1"};
	m_Script.InvokeN(L"UpdateTableText", params, 4, &varRet);
	CComVariant params2[] = { 1, "blue", "0", "0", "table_1" };
	m_Script.InvokeN(L"UpdateTableClass", params2, 5, &varRet);
	*/
	HRESULT hr;
	//IDispatch *pDisp = this->m_Web2.GetDocument();
	pDisp = this->m_Web.get_Document();              //m_Web 就是你的IE控件生成的对象
	IHTMLDocument2 *pDocument = NULL;
	IHTMLElement*   pEl;
	IHTMLBodyElement * pBodyEl;
	hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDocument);
	if (SUCCEEDED(pDocument->get_body(&pEl)))
	{
		if (SUCCEEDED(pEl->QueryInterface(IID_IHTMLBodyElement, (void**)&pBodyEl)))
		{
			pBodyEl->put_scroll(L"no");//去滚动条
		}
		IHTMLStyle   *phtmlStyle;
		pEl->get_style(&phtmlStyle);

		if (phtmlStyle != NULL)
		{
			phtmlStyle->put_overflow(L"hidden");
			phtmlStyle->put_border(L"none");//   去除边框     
			phtmlStyle->Release();
			pEl->Release();
		}
	}
}

LRESULT CWebUIDlg::OnSetText(WPARAM w, LPARAM l) 
{
	my_msg* msg = (my_msg*)w;
	//::printf("OnSetText:%s\n", msg->id);
	m_JsCall.SetText(msg->id, msg->text);
	if (l)
		delete msg;
	return 0;
}
LRESULT CWebUIDlg::OnAddLog(WPARAM w, LPARAM l)
{
	my_msg* msg = (my_msg*)w;
	//::printf("OnAddLog:%08X\n", msg);
	m_JsCall.AddLog(msg->id, msg->text, msg->cla);
	if (l) {
		//::printf("delete msg\n");
		delete msg;
	}
		
	return 0;
}

LRESULT CWebUIDlg::OnUpdateStatusText(WPARAM w, LPARAM l)
{
	my_msg* msg = (my_msg*)w;
	m_JsCall.UpdateStatusText(msg->text, msg->value);
	if (l)
		delete msg;
	return 0;
}

LRESULT CWebUIDlg::OnUpdateTableText(WPARAM w, LPARAM l)
{
	my_msg* msg = (my_msg*)w;
	m_pGame->m_pAccoutCtrl->SetText(msg->value, msg->value2, msg->text);
	if (l)
		delete msg;
	return 0;
}

LRESULT CWebUIDlg::OnUpdateVerOk(WPARAM w, LPARAM l)
{
	m_JsCall.UpdateVerOk();
	return 0;
}

DWORD WINAPI CWebUIDlg::KeyBoardHook(LPVOID)
{
	g_UI->m_hk = SetWindowsHookExW(WH_KEYBOARD_LL, KeyBoardHookProc, NULL, 0);
	if (g_UI->m_hk)
		::printf(("HOOK成功\n"));

#if 1
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (GetMessage(&msg, 0, 0, 0)) {
		DispatchMessage(&msg);
	}
#endif
	::printf("HOOK结束\n");
	UnhookWindowsHookEx(g_UI->m_hk);

	return 0;
}

LRESULT CWebUIDlg::KeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *Key_Info = (KBDLLHOOKSTRUCT*)lParam;
	if (HC_ACTION == nCode) {
		if (WM_KEYDOWN == wParam) { //如果按键为按下状态
			if (g_UI->m_pGame->m_pBig && g_UI->m_pGame->m_pBig->Mnq) {
				HWND hWndTop = ::GetForegroundWindow();
				//::printf("%08x %08X %08X\n", hWndTop, g_UI->m_pGame->m_pBig->Mnq->WndTop, g_UI->m_pGame->m_pBig->Mnq->Wnd);
				if (hWndTop == g_UI->m_pGame->m_pBig->Mnq->WndTop || hWndTop == g_UI->m_pGame->m_pBig->Mnq->Wnd) {
					if (Key_Info->vkCode == 'C') {
						g_UI->m_pGame->m_pGameProc->m_bPause = false;
						::printf("游戏继续\n");
					}
					if (Key_Info->vkCode == 'P') {
						g_UI->m_pGame->m_pGameProc->m_bPause = true;
						::printf("游戏暂停\n");
					}
				}
			}
		}
	}
	LRESULT result = CallNextHookEx(g_UI->m_hk, nCode, wParam, lParam);
	return result;
}

// 检查是否激活
DWORD __stdcall CWebUIDlg::Verify(LPVOID p)
{
#if 1
	my_msg* msg = new my_msg;
	COPY_MSG(msg, "id", "正在激活...", 0, 4);
	PostMessageA(g_UI->m_hWnd, MSG_UPSTATUSTEXT, (WPARAM)msg, NULL);
	Sleep(100);

	COPY_MSG(msg, "log_ul", "验证卡号...", 0, 0);
	PostMessageA(g_UI->m_hWnd, MSG_ADDLOG, (WPARAM)msg, NULL);
	Sleep(100);
	if (g_UI->m_pGame->m_pHome->Verify()) {
		COPY_MSG(msg, "id", "激活成功.", 0, 2);
		PostMessageA(g_UI->m_hWnd, MSG_UPSTATUSTEXT, (WPARAM)msg, NULL);
		Sleep(100);

		COPY_MSG(msg, "card_date", (char*)g_UI->m_pGame->m_pHome->GetExpireTime_S().c_str(), nullptr, 0);
		PostMessageA(g_UI->m_hWnd, MSG_SETTEXT, (WPARAM)msg, NULL);
		Sleep(100);

		COPY_MSG(msg, "log_ul", "验证成功", 0, 0);
		PostMessageA(g_UI->m_hWnd, MSG_ADDLOG, (WPARAM)msg, NULL);
	}
	else {
		COPY_MSG(msg, "id", "未激活.", 0, 3);
		PostMessageA(g_UI->m_hWnd, MSG_UPSTATUSTEXT, (WPARAM)msg, NULL);
		Sleep(100);

		COPY_MSG(msg, "card_date", "未激活", 0, 0);
		PostMessageA(g_UI->m_hWnd, MSG_SETTEXT, (WPARAM)msg, NULL);
		Sleep(100);

		COPY_MSG(msg, "log_ul", "卡号未激活", 0, 0);
		PostMessageA(g_UI->m_hWnd, MSG_ADDLOG, (WPARAM)msg, NULL);
	}
#endif
	return 0;
}

// 检查更新版本
DWORD __stdcall CWebUIDlg::UpdateVer(LPVOID)
{
#define DOWNURL  "http://www.myhostcpp.com/2Star"

	::printf("检查更新版本\n");
	CString path;
	path.Format(L"/2Star/ver.ini?%d", time(nullptr));
	std::string result;
	HttpClient http;
	http.Request(NULL, path.GetBuffer(), result);
	::printf("%s\n", result.c_str());
	Explode explode("|", result.c_str());
	if (explode.GetCount() != 5) {
		::MessageBox(NULL, L"检查失败, 请重试.", L"检查更新", MB_OK);
		PostMessageA(g_UI->m_hWnd, MSG_UPVEROK, NULL, NULL);
		return 0;
	}

	std::string ver;
	char ver_file[255];
	sprintf_s(ver_file, "%s\\ver.ini", g_UI->m_ConfPath);
	ifstream fr(ver_file, fstream::in);
	getline(fr, ver);
	if (!fr.is_open()) {
		::printf("没有ver:%s\n", ver_file);
		ver = "1|1|1|1|1";
	}

	Explode test("|", ver.c_str());
	if (test.GetCount() != 5) {
		ver = "1|1|1|1|1";
	}

	Explode arr("|", ver.c_str());

	my_msg* msg = new my_msg;
	bool update = false;
	CString csMsg = L"";
	char url[256];
	if (strcmp(arr[1], explode[1]) != 0) {
		update = true;
		csMsg = L"更新完成, 停止再启动后生效.";
		::printf("下载2Star.dll\n");
		COPY_MSG(msg, "id", "下载2Star.dll...", NULL, 4);
		PostMessageA(g_UI->m_hWnd, MSG_UPSTATUSTEXT, (WPARAM)msg, NULL);
		Sleep(100);
		sprintf_s(url, "%s/2Star.dll?t=%d", DOWNURL, time(nullptr));
		DownFile(url, "2Star.dll", NULL);
	}
	if (strcmp(arr[2], explode[2]) != 0) {
		update = true;
		csMsg = L"更新完成, 关闭此程序, 重启生效.";

		::printf("下载index.html\n");
		COPY_MSG(msg, "id", "下载index.html...", NULL, 4);
		PostMessageA(g_UI->m_hWnd, MSG_UPSTATUSTEXT, (WPARAM)msg, NULL);
		Sleep(100);
		sprintf_s(url, "%s/html/static/index.html?t=%d", DOWNURL, time(nullptr));
		DownFile(url, "html/static/index.html", NULL);

		::printf("下载main.css\n");
		COPY_MSG(msg, "id", "下载main.css...", NULL, 4);
		PostMessageA(g_UI->m_hWnd, MSG_UPSTATUSTEXT, (WPARAM)msg, NULL);
		Sleep(100);
		sprintf_s(url, "%s/html/static/main.css?t=%d", DOWNURL, time(nullptr));
		DownFile(url, "html/static/main.css", NULL);

		::printf("下载main.js\n");
		COPY_MSG(msg, "id", "下载main.js...", NULL, 4);
		PostMessageA(g_UI->m_hWnd, MSG_UPSTATUSTEXT, (WPARAM)msg, NULL);
		Sleep(100);
		sprintf_s(url, "%s/html/static/main.js?t=%d", DOWNURL, time(nullptr));
		DownFile(url, "html/static/main.js", NULL);
	}
	if (strcmp(arr[3], explode[3]) != 0) {
	}
	if (strcmp(arr[4], explode[4]) != 0) {
		update = true;
		csMsg = L"更新完成, 关闭此程序, 重启生效.";
		::printf("下载driver_inject_x64.sys\n");
		COPY_MSG(msg, "id", "下载driver_inject_x64.sys...", NULL, 4);
		PostMessageA(g_UI->m_hWnd, MSG_UPSTATUSTEXT, (WPARAM)msg, NULL);
		Sleep(100);
		sprintf_s(url, "%s/driver_inject_x64.sys?t=%d", DOWNURL, time(nullptr));
		DownFile(url, "driver_inject_x64.sys", NULL);
	}
	if (strcmp(arr[0], explode[0]) != 0) {
		char param[128];
		sprintf_s(param, "2Star.exe %s/2Star.exe?t=%d", DOWNURL, time(nullptr));
		
		fr.close();

		ofstream fw;
		fw.open(ver_file);
		fw << result;
		fw.close();

		ShellExecuteA(NULL, "open", "down.exe", param, g_UI->m_ConfPath, SW_SHOWNORMAL);
		ExitProcess(0);
		return 0;
	}

	if (update) {
		::MessageBox(NULL, csMsg, L"检查更新", MB_OK);
	}
	
	PostMessageA(g_UI->m_hWnd, MSG_UPVEROK, NULL, NULL);
	Sleep(100);

	fr.close();

	ofstream fw;
	fw.open(ver_file);
	fw << result;
	fw.close();

	return 0;
}

// 监听
DWORD __stdcall CWebUIDlg::Listen(LPVOID p)
{
	g_UI->m_pGame->Listen(12379);
	g_UI->m_JsCall.AddLog("log_ul", "停止服务监听", "blue");
	return 0;
}

// 检测模拟器
DWORD __stdcall CWebUIDlg::CheckMNQ(LPVOID)
{
	// TODO: Add extra validation here
	MNQ mnq[10];
	int count = g_UI->m_pGame->m_pEmulator->GetCount();

	for (int i = 0; i < count; i++) {
		MNQ* p = g_UI->m_pGame->m_pEmulator->GetMNQ(i);
		::printf("\n%d:%s 绑定窗口:%08X UI进程ID:%d 虚拟进程ID:%d 是否初始化完毕:%d",
		    p->Index, p->Name, p->Wnd, p->UiPid, p->VBoxPid, p->Init);
	}
	::printf("\n");

	//g_UI->m_pGame->m_pEmulator->Setprop("account", 123, 0);
	//int v = g_UI->m_pGame->m_pEmulator->Getprop("account", 0, -1);
	//::printf("v:%d\n", v);
	//g_UI->m_pGame->m_pEmulator->Swipe(10, 10, 10, 300);
	//bool open = g_UI->m_pGame->m_pEmulator->OpenMNQ(0);
	//::printf("open:%d\n", open);
	return 0;
}

// 执行流程
DWORD __stdcall CWebUIDlg::Run(LPVOID)
{
	g_UI->m_pGame->Run();
	return 0;
}

void CWebUIDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pGame->m_pDriver->UnStall();
	UnregisterHotKey(m_hWnd, 1001);
	UnregisterHotKey(m_hWnd, 1002);

	CDialogEx::OnClose();
}


void CWebUIDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1) {
	}
	if (nIDEvent == 2) { // 用于检查登录是否超时
		int count = m_pGame->CheckLoginTimeOut();
		//::printf("登录超时的帐号数量:%d\n", count);
	}

	CDialogEx::OnTimer(nIDEvent);
}


BOOL CWebUIDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类
	return CDialogEx::PreCreateWindow(cs);
}


void CWebUIDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	HWND hWndTop = ::GetForegroundWindow();
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nHotKeyId)
	{
	case 1001:
		if (hWndTop == m_pGame->m_pBig->Mnq->WndTop || hWndTop == m_pGame->m_pBig->Mnq->Wnd) {
			m_pGame->m_pGameProc->m_bPause = true;
			::printf("游戏暂停\n");
		}
		break;
	case 1002:
		if (hWndTop == m_pGame->m_pBig->Mnq->WndTop || hWndTop == m_pGame->m_pBig->Mnq->Wnd) {
			m_pGame->m_pGameProc->m_bPause = false;
			::printf("游戏继续\n");
		}
		break;
	}

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}
