#include "Game.h"
#include "Emulator.h"
#include "PrintScreen.h"
#include "Talk.h"
#include "GameProc.h"
#include <time.h>
#include <My/Common/Explode.h>
#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Common/C.h>

// ...
Emulator::Emulator(Game* p)
{
	m_pGame = p;

	m_nCount = 0;
	ZeroMemory(m_List, sizeof(m_List));
}

// 设置路径
void Emulator::SetPath(char * path)
{
	strcpy(m_chPath, path);

	strcpy(m_chDnconsole, path);
	strcat(m_chDnconsole, "\\dnconsole.exe");

	strcpy(m_chLd, path);
	strcat(m_chLd, "\\ld.exe");

	List2();
}

// 执行CMD命令并获取到结果
void Emulator::ExecCmdLd(const char* cmd, int index)
{
	char cmdline[128];
	sprintf(cmdline, "%s -s %d %s", m_chLd, index, cmd);
	system(cmdline);
}

// 执行CMD命令并获取到结果
bool Emulator::ExecCmd(const char* cmd, char* result, int size)
{
	if (!result || !size) {
		char cmdline[128];
		sprintf(cmdline, "%s %s", m_chDnconsole, cmd);
		system(cmdline);
		return true;
	}

	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		printf("CreatePipe Failed\n");
		return false;
	}

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfoA(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

	char cmdline[200];
	sprintf(cmdline, "cmd /C %s %s", m_chDnconsole, cmd);

	if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
		printf("CreateProcess failed!");
		return false;
	}
	CloseHandle(hWrite);

	DWORD bytesRead;

	Sleep(50);
	bool r = ReadFile(hRead, result, size, &bytesRead, NULL);
	CloseHandle(hRead);

	return r;
}

// 打开模拟器
MNQ* Emulator::Open(_account_* p)
{
	List2();
	int i = p->IsBig ? 0 : 1;
	for (; i < m_nCount; i++) {
		if (m_List[i].Account == nullptr) {
			MNQ* m = Open(i);
			if (m) {
				m->Account = p;
				m->Account->IsLogin = 0;
				m->Account->Mnq = m;
				m->Account->IsGetAddr = 0;

				//m_pGame->SetMNQName(m->Account, m->Name);
			}
				
			return m;
		}
	}
	return nullptr;
}

// 打开模拟器
MNQ* Emulator::Open(int index)
{
	if (index >= m_nCount)
		return nullptr;

	char cmd[128] = { 0 }, result[128] = { 0 };
	sprintf_s(cmd, "launchex --index %d --packagename \"com.nd.myht\"", index);
	ExecCmd(cmd, result, sizeof(result));

	SetStartTime(index);
	List2();
	MNQ* p = GetMNQ(index);
	if (p) {
		LOGVARN2(128, "blue", "启动模拟器:%s[%d]", p->Name, p->Index+1);
	}
	return p;
}

// 关闭模拟器
MNQ* Emulator::Close(int index)
{
	if (index >= m_nCount)
		return nullptr;

	MNQ* p = GetMNQ(index);
	if (p) {
		if (p->UiPid > 0) {
			HANDLE handle = ::OpenProcess(PROCESS_TERMINATE, FALSE, p->UiPid);
			::TerminateProcess(handle, 4);
			CloseHandle(handle);
		}
		if (p->VBoxPid > 0) {
			HANDLE handle = ::OpenProcess(PROCESS_TERMINATE, FALSE, p->VBoxPid);
			::TerminateProcess(handle, 4);
			CloseHandle(handle);
		}
		p->StartTime = 0;
		if (p->Account) {
			m_pGame->SetMNQName(p->Account, nullptr);             // 清空模拟器名字
			m_pGame->SetStatus(p->Account, ACCSTA_OFFLINE, true); // 设置离线状态

			p->Account->IsLogin = 0;
			p->Account = nullptr;
		}

		LOGVARN2(128, "red", "关闭模拟器:%s[%d]", p->Name, p->Index + 1);
		return p;
	}
	else {
		char cmd[128] = { 0 };
		sprintf_s(cmd, "quit --index %d", index);
		ExecCmd(cmd, cmd, sizeof(cmd));
		return nullptr;
	}
}

// 关闭所有模拟器
void Emulator::CloseAll()
{
	int i, count = List2();
	for (i = 0; i < count; i++) {
		Close(i);
	}

	DWORD pids[10];
	count = SGetProcessIds(L"dnplayer.exe", pids, sizeof(pids)/sizeof(DWORD));
	for (i = 0; i < count; i++) {
		HANDLE handle = ::OpenProcess(PROCESS_TERMINATE, FALSE, pids[i]);
		::TerminateProcess(handle, 4);
		CloseHandle(handle);
	}
}

// 启动游戏
void Emulator::StartGame(int index)
{
	printf("启动游戏APP\n");
	char cmd[128] = { 0 };
	sprintf_s(cmd, "runapp --index %d --packagename \"com.nd.myht\"", index);
	ExecCmd(cmd, nullptr, 0);
}

// 等待进入游戏
void Emulator::CloseGame(int index)
{
	printf("关闭游戏APP\n");
	char cmd[128] = { 0 };
	sprintf_s(cmd, "killapp --index %d --packagename \"com.nd.myht\"", index);
	ExecCmd(cmd, nullptr, 0);
}

// 等待进入游戏
void Emulator::WatchInGame(_account_* account)
{
	// 756,228 账号框
	// 720,300 密码框
	// 575,405 登录
	printf("%s等待启动进入模拟器...\n", account->Name);
	char log[64];
#if 1
	while (!account->Mnq) Sleep(1000);

	bool is_print = false;
	MNQ* m = account->Mnq;
	while (true) {
		//printf("获取模拟器列表(%d)\n", time(nullptr));
		//printf("获取模拟器列表完成(%d)\n", time(nullptr));

		if (!m->Init) { // 未初始化好
			if ((time(nullptr) - m->StartTime) > m_pGame->m_Setting.InitTimeOut) {
				Close(m->Index);
				Sleep(1000);
				Open(m->Index);
				break;
			}
			//printf("没有初始化好(%d)\n", time(nullptr));
			Sleep(3500);
			List2();
			continue;
		}

		if (!is_print) {
			printf("%s模拟器已启动完毕\n", account->Name);
			printf("%s等待进入游戏...\n", account->Name);
			is_print = true;
		}
		//printf("已初始化完毕(%d)\n", time(nullptr));
		//Setprop("account_index", m->Account->Index, m->Index); // 设置绑定属性
		//int v = Getprop("account_index", m->Index, -1);
		//printf("Getprop[account_index]:%d\n", v);
		
		if (!account->IsLogin) { // 还未绑定模拟器, 说明帐号还未登录进去
			// 多线程等待其他操作完毕
			while (m_pGame->m_pPrintScreen->IsLocked());
			m_pGame->m_pPrintScreen->Lock();
			bool in_login_pic = m_pGame->m_pTalk->IsInLoginPic(m->Wnd);
			//m_pGame->m_pPrintScreen->GetPixelCount(0x00, 0, true);
			Sleep(500);
			m_pGame->m_pPrintScreen->UnLock();
			
			if (in_login_pic) {
				printf("%s已进入登录界面\n", account->Name);
				m_pGame->SetStatus(m->Account, ACCSTA_LOGIN, true); // 设置登录状态

				printf("点击登录帐号图标\n");
				LOGVARP2(log, "blue", "点击登录帐号图标");
				Tap(1205, 155, m->Index);
				Sleep(1000);

				printf("点击其他账号登录\n");
				LOGVARP2(log, "blue", "点击其他账号登录");
				Tap(660, 518, m->Index);
				Sleep(1000);

				printf("双击账号框\n");
				LOGVARP2(log, "blue", "双击账号框");
				m_pGame->m_pGameProc->DBClick(520, 230, m->Wnd);
				Sleep(300);
				Key(67, m->Index); // 67为退格键
				Sleep(300);
				
				printf("输入帐号\n");
				LOGVARP2(log, "blue", "输入帐号");
				Text(m->Account->Name, m->Index);
				Sleep(300);

				printf("输入密码\n");
				LOGVARP2(log, "blue", "输入密码");
				Tap(720, 300, m->Index);
				Sleep(300);
				Text(m->Account->Password, m->Index);
				Sleep(500);

				printf("点击登录\n");
				LOGVARP2(log, "blue", "点击登录");
				Tap(575, 405, m->Index);

				account->IsLogin = 1;

				Sleep(2500);
				m_pGame->m_pGameProc->Click(510, 380, 0xff, m->Wnd); // 点击以后绑定
					
				break;
			}
		}

		Sleep(3500);
	}
#else
	while (true) {
		List2();
		for (int i = 0; i < m_nCount; i++) {
			MNQ* m = &m_List[i];
			//printf("\n%d:%s 绑定窗口:%08X UI进程ID:%d 虚拟进程ID:%d 是否初始化完毕:%d 帐号:%08X", p->Index, p->Name, p->Hwnd, p->UiPid, p->VBoxPid, p->Init, p->Account);
			if (m->Account) {   // 已绑定帐号
				if (!m_List[i].Init) { // 未初始化好
					if ((time(nullptr) - m->StartTime) > m_pGame->m_Setting.InitTimeOut) {
						Close(i);
						Sleep(1000);
						Open(i);
					}
				}
				else {
					Setprop("account_index", m->Account->Index, m->Index); // 设置绑定属性
					int v = Getprop("account_index", m->Index, -1);
					//printf("Getprop[account_index]:%d\n", v);
					//printf("已初始化完毕\n");
					if (!m_List[i].Account->IsLogin) { // 还未绑定模拟器, 说明帐号还未登录进去
						//printf("已初始化完毕2\n");
						// 截取弹框确定按钮图片
						m_pGame->m_pPrintScreen->CopyScreenToBitmap(m->Wnd, 1205, 140, 1220, 155, 0, true);
						//m_pGame->m_pPrintScreen->GetPixelCount(0x00, 0, true);
						if (m_pGame->m_pPrintScreen->CompareImage(CIN_CanLogin, nullptr, 1)) {
							m_pGame->SetStatus(m->Account, ACCSTA_LOGIN, true); // 设置登录状态

							Tap(1205, 155, m->Index);
							LOGVARP2(log, "blue", "点击登录帐号图标");
							Sleep(1000);

#if 1
							m_pGame->m_pGameProc->DBClick(520, 230, m->Wnd);
							LOGVARP2(log, "blue", "双击账号框");
							Sleep(300);
							Key(67, m->Index); // 67为退格键
							Sleep(300);
#else
							Tap(755, 230, m->Index);
							LOGVARP2(log, "blue", "点击账号框");
							Sleep(500);
							for (int i = 0; i < strlen(m->Account->Name) + 1; i++) {
								Key(67, m->Index); // 67为退格键
								Sleep(100);
							}
#endif					
							Text(m->Account->Name, m->Index);
							LOGVARP2(log, "blue", "输入帐号");
							Sleep(300);

							Tap(720, 300, m->Index);
							Sleep(300);
							Text(m->Account->Password, m->Index);
							LOGVARP2(log, "blue", "输入密码");
							Sleep(500);

							Tap(575, 405, m->Index);
							LOGVARP2(log, "blue", "点击登录");

							m_List[i].Account->IsLogin = 1;

							Sleep(2500);
							m_pGame->m_pGameProc->Click(510, 380, 0xff, m->Wnd); // 点击以后绑定
							goto _end_;
						}
					}
				}
			}
		}
		Sleep(1000);
	}
_end_:
	if (!m_pGame->AutoLogin())
		LOGVARN2(64, "blue", "已全部登入游戏");
#endif
	printf("已登录完成\n");
	return;
}

// list2命令
int Emulator::List2()
{
	while (m_bIsGetList);
	//printf("Emulator::List2()\n");
	m_bIsGetList = true;
	m_nCount = List2(m_List, sizeof(m_List) / sizeof(MNQ));
	m_bIsGetList = false;
	return m_nCount;
}

// list2命令
int Emulator::List2(MNQ* mnq, int length)
{
	int count = 0;
	char result[1024] = { 0 };
	if (ExecCmd("list2", result, sizeof(result))) {
		//printf("结果:\n--------------------\n%s--------------------\n", result);
		Explode line("\n", result);
		for (int i = 0; i < line.GetCount(); i++) {
			Explode arr(",", line[i]);
			if (arr.GetCount() == 7) {
				MNQ* m = &mnq[count];
				int oldInit = m->Init;

				m->Index = count;
				m->WndTop = (HWND)arr.GetValue2Int(2);
				m->Wnd = (HWND)arr.GetValue2Int(3);
				m->UiPid = arr.GetValue2Int(5);
				m->VBoxPid = arr.GetValue2Int(6);
				m->Init = arr.GetValue2Int(4);
				strcpy(m->Name, arr[1]);

				if (!m->WndTop) {
					//mnq[count].StartTime = 0;
					//mnq[count].Account = nullptr;
				}

				if (m->VBoxPid > 0 && !m->Account) { // 是否已绑定了帐号
					//printf("get account_index\n");
					int account_index = 0;// Getprop("account_index", count, -1);
					m->Account = m_pGame->GetAccount(account_index);
					//printf("xxx:%d\n", account_index);
					if (m->Account) {
						m->Account->IsLogin = 1;
						m->Account->Mnq = m;
					}
				}
				if (oldInit && !m->Init) { // 说明模拟器退出了
					if (m->Account) { // 设置帐号退出
						printf("%s 设置为已下线\n", m->Account->Name);
						m_pGame->SetStatus(m->Account, ACCSTA_OFFLINE, true);
						m->Account->IsLogin = 0;
						m->Account->Mnq = nullptr;
						m->Account = nullptr;
					}
				}

				if (++count >= length)
					return length;
			}
		}
		//printf("\n");
	}
	return count;
}

// 设置属性
void Emulator::Setprop(const char * key, int value, int index)
{
	char v[16];
	sprintf_s(v, "%d", value);
	Setprop(key, v, index);
}

// 设置属性
void Emulator::Setprop(const char * key, const char * value, int index)
{
#if 0
	char name[128];
	sprintf_s(name, "%s:%s|", key, value);
	ReName(name, index);
#else
	char cmd[128] = { 0 };
	sprintf_s(cmd, "setprop --index %d --key \"%s\" --value \"%s\"", index, key, value);
	ExecCmd(cmd, nullptr, 0);
#endif
}

// 获取属性
bool Emulator::Getprop(const char * key, char * result, int size, int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "getprop --index %d --key \"%s\"", index, key);
	return ExecCmd(cmd, result, sizeof(result));
}

// 获取属性
int Emulator::Getprop(const char * key, int index, int d_v)
{
	char result[128] = { 0 };
	if (!Getprop(key, result, sizeof(result), index))
		return d_v;

	trim(result);
	return result[0] >= '0'&&result[0] <= '9' ? atoi(result) : d_v;
}

// 获取这个属性的模拟器数量
int Emulator::GetpropCount(const char* key, int v)
{
	int count = 0;
	List2();
	for (int i = 0; i < m_nCount; i++) {
		if (Getprop(key, m_List[i].Index) == v)
			count++;
	}
	return count;
}

// 重命名
void Emulator::ReName(const char * name, int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "rename --index %d --title \"%s\"", index, name);
	ExecCmd(cmd, nullptr, 0);
}

// 点击
void Emulator::Tap(int x, int y, int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "input tap %d %d", x, y);
	ExecCmdLd(cmd, index);
}

// 点击
void Emulator::Tap(int x, int y, int x2, int y2, int index)
{
	Tap(MyRand(x, x2), MyRand(y, y2), index);
}

// 滑动
void Emulator::Swipe(int x, int y, int x2, int y2, int index, int ms)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "input swipe %d %d %d %d %d", x, y, x2, y2, ms);
	ExecCmdLd(cmd, index);
}

// 按键
void Emulator::Key(int key, int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "input keyevent %d", key);
	ExecCmdLd(cmd, index);
}

void Emulator::Text(char* text, int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "input text %s", text);
	ExecCmdLd(cmd, index);
}

// 设置模拟器启动时间
int Emulator::SetStartTime(int index, int v)
{
	MNQ* p = GetMNQ(index);
	if (p == nullptr)
		return false;

	p->StartTime = v == -1 ? time(nullptr) : v;
	return p->StartTime;
}

// 获取模拟器空闲数量
int Emulator::GetFreeCount()
{
	int free_count = 0;
	int count = List2();
	for (int i = 0; i < count; i++) {
		if (!GetMNQ(i)->Account)
			free_count++;
	}
	return free_count;
}
