#pragma once
#include <Windows.h>

struct _account_;
// 模拟器信息
typedef struct mnq {
	int    Index;     // 索引
	char   Name[64];  // 名称
	HWND   WndTop;    // 顶层窗口句柄
	HWND   Wnd;       // 接受信息窗口句柄
	int    UiPid;     // UI进程ID
	int    VBoxPid;   // 真实数据进程ID
	int    Init;      // 模拟器是否已启动好
	HANDLE Process;   // 权限句柄
	int    StartTime; // 启动时间

	_account_* Account; // 对应哪个帐号, 一打开模拟器就绑定帐号
} MNQ;

class Game;
class Emulator
{
public:
	// ...
	Emulator(Game* p);
	// 设置路径
	void SetPath(char* path);
	// 执行CMD命令并获取到结果
	void ExecCmdLd(const char* cmd, int index=0);
	// 执行CMD命令并获取到结果
	bool ExecCmd(const char* cmd, char* result, int size);
	// 打开模拟器
	MNQ* Open(_account_* p);
	// 打开模拟器
	MNQ* Open(int index);
	// 关闭模拟器
	MNQ* Close(int index);
	// 关闭所有模拟器
	void CloseAll();
	// 启动游戏
	void StartGame(int index=0);
	// 关闭游戏
	void CloseGame(int index=0);
	// 等待进入游戏
	void WatchInGame(_account_* account);
	// list2命令
	int List2();
	// list2命令
	int List2(MNQ* mnq, int length);
	// 设置属性
	void Setprop(const char* key, int value, int index = 0);
	// 设置属性
	void Setprop(const char* key, const char* value, int index=0);
	// 获取属性
	bool Getprop(const char* key, char* result, int size, int index=0);
	// 获取属性
	int  Getprop(const char* key, int index=0, int d_v=0);
	// 获取这个属性的模拟器数量
	int  GetpropCount(const char* key, int v);
	// 重命名
	void ReName(const char* name, int index=0);
	// 点击
	void Tap(int x, int y, int index=0);
	// 点击
	void Tap(int x, int y, int x2, int y2, int index = 0);
	// 滑动
	void Swipe(int x, int y, int x2, int y2, int index=0, int ms=0);
	// 按键
	void Key(int key, int index=0);
	// 文字
	void Text(char* text, int index = 0);
	// 设置模拟器启动时间
	int SetStartTime(int index, int v=-1);
	// 获取模拟器数量
	int GetCount() { return m_nCount; }
	// 获取模拟器空闲数量
	int GetFreeCount();
	// 获取模拟器信息
	MNQ* GetMNQ(int index) { return index < m_nCount ? &m_List[index] : nullptr;  }
public:
	// Game类
	Game* m_pGame;
	// 模拟器路径
	char  m_chPath[255];
	// dnconsole命令
	char m_chDnconsole[255];
	// ld命令
	char m_chLd[255];
public:
	// 模拟器数量
	int  m_nCount;
	// 模拟器列表
	MNQ  m_List[10];
	// 是否获取列表中
	bool m_bIsGetList = false;
};