#include "Game.h"
#include "Emulator.h"
#include "PrintScreen.h"
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

// ����·��
void Emulator::SetPath(char * path)
{
	strcpy(m_chPath, path);

	strcpy(m_chDnconsole, path);
	strcat(m_chDnconsole, "\\dnconsole.exe");

	strcpy(m_chLd, path);
	strcat(m_chLd, "\\ld.exe");

	List2();
}

// ִ��CMD�����ȡ�����
void Emulator::ExecCmdLd(const char* cmd, int index)
{
	char cmdline[128];
	sprintf(cmdline, "%s -s %d %s", m_chLd, index, cmd);
	system(cmdline);
}

// ִ��CMD�����ȡ�����
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

// ��ģ����
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

// ��ģ����
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
		LOGVARN2(128, "blue", "����ģ����:%s[%d]", p->Name, p->Index+1);
	}
	return p;
}

// �ر�ģ����
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
			m_pGame->SetMNQName(p->Account, nullptr);             // ���ģ��������
			m_pGame->SetStatus(p->Account, ACCSTA_OFFLINE, true); // ��������״̬

			p->Account->IsLogin = 0;
			p->Account = nullptr;
		}

		LOGVARN2(128, "red", "�ر�ģ����:%s[%d]", p->Name, p->Index + 1);
		return p;
	}
	else {
		char cmd[128] = { 0 };
		sprintf_s(cmd, "quit --index %d", index);
		ExecCmd(cmd, cmd, sizeof(cmd));
		return nullptr;
	}
}

// �ر�����ģ����
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

// ������Ϸ
void Emulator::StartGame(int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "runapp --index %d --packagename \"com.nd.myht\"", index);
	ExecCmd(cmd, nullptr, 0);
}

// �ȴ�������Ϸ
void Emulator::CloseGame(int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "killapp --index %d --packagename \"com.nd.myht\"", index);
	ExecCmd(cmd, nullptr, 0);
}

// �ȴ�������Ϸ
void Emulator::WatchInGame(_account_* account)
{
	// 756,228 �˺ſ�
	// 720,300 �����
	// 575,405 ��¼
	printf("%s�ȴ���������ģ����...\n", account->Name);
	char log[64];
#if 1
	while (!account->Mnq) Sleep(1000);

	bool is_print = false;
	MNQ* m = account->Mnq;
	while (true) {
		printf("��ȡģ�����б�(%d)\n", time(nullptr));
		List2();
		//printf("��ȡģ�����б����(%d)\n", time(nullptr));

		if (!m->Init) { // δ��ʼ����
			if ((time(nullptr) - m->StartTime) > m_pGame->m_Setting.InitTimeOut) {
				Close(m->Index);
				Sleep(1000);
				Open(m->Index);
			}
			//printf("û�г�ʼ����(%d)\n", time(nullptr));
			Sleep(3500);
			continue;
		}

		if (!is_print) {
			printf("%sģ�������������\n", account->Name);
			printf("%s�ȴ�������Ϸ...\n", account->Name);
			is_print = true;
		}
		//printf("�ѳ�ʼ�����(%d)\n", time(nullptr));
		//Setprop("account_index", m->Account->Index, m->Index); // ���ð�����
		//int v = Getprop("account_index", m->Index, -1);
		//printf("Getprop[account_index]:%d\n", v);
		
		if (!account->IsLogin) { // ��δ��ģ����, ˵���ʺŻ�δ��¼��ȥ
			// ���̵߳ȴ������������
			while (m_pGame->m_pPrintScreen->IsLocked());
			m_pGame->m_pPrintScreen->Lock();
			// ��ȡ����ȷ����ťͼƬ
			m_pGame->m_pPrintScreen->CopyScreenToBitmap(m->Wnd, 1205, 140, 1220, 155, 0, true);
			int compare_count = m_pGame->m_pPrintScreen->ComparePixel("��¼��ť", nullptr, 1);
			//m_pGame->m_pPrintScreen->GetPixelCount(0x00, 0, true);
			Sleep(500);
			m_pGame->m_pPrintScreen->UnLock();
			
			if (compare_count > 0) {
				printf("%s�ѽ����¼����\n", account->Name);
				m_pGame->SetStatus(m->Account, ACCSTA_LOGIN, true); // ���õ�¼״̬

				printf("�����¼�ʺ�ͼ��\n");
				LOGVARP2(log, "blue", "�����¼�ʺ�ͼ��");
				Tap(1205, 155, m->Index);
				Sleep(1000);

				printf("˫���˺ſ�\n");
				LOGVARP2(log, "blue", "˫���˺ſ�");
				m_pGame->m_pGameProc->DBClick(520, 230, m->Wnd);
				Sleep(300);
				Key(67, m->Index); // 67Ϊ�˸��
				Sleep(300);
				
				printf("�����ʺ�\n");
				LOGVARP2(log, "blue", "�����ʺ�");
				Text(m->Account->Name, m->Index);
				Sleep(300);

				printf("��������\n");
				LOGVARP2(log, "blue", "��������");
				Tap(720, 300, m->Index);
				Sleep(300);
				Text(m->Account->Password, m->Index);
				Sleep(500);

				printf("�����¼\n");
				LOGVARP2(log, "blue", "�����¼");
				Tap(575, 405, m->Index);

				account->IsLogin = 1;

				Sleep(2500);
				m_pGame->m_pGameProc->Click(510, 380, 0xff, m->Wnd); // ����Ժ��
					
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
			//printf("\n%d:%s �󶨴���:%08X UI����ID:%d �������ID:%d �Ƿ��ʼ�����:%d �ʺ�:%08X", p->Index, p->Name, p->Hwnd, p->UiPid, p->VBoxPid, p->Init, p->Account);
			if (m->Account) {   // �Ѱ��ʺ�
				if (!m_List[i].Init) { // δ��ʼ����
					if ((time(nullptr) - m->StartTime) > m_pGame->m_Setting.InitTimeOut) {
						Close(i);
						Sleep(1000);
						Open(i);
					}
				}
				else {
					Setprop("account_index", m->Account->Index, m->Index); // ���ð�����
					int v = Getprop("account_index", m->Index, -1);
					//printf("Getprop[account_index]:%d\n", v);
					//printf("�ѳ�ʼ�����\n");
					if (!m_List[i].Account->IsLogin) { // ��δ��ģ����, ˵���ʺŻ�δ��¼��ȥ
						//printf("�ѳ�ʼ�����2\n");
						// ��ȡ����ȷ����ťͼƬ
						m_pGame->m_pPrintScreen->CopyScreenToBitmap(m->Wnd, 1205, 140, 1220, 155, 0, true);
						//m_pGame->m_pPrintScreen->GetPixelCount(0x00, 0, true);
						if (m_pGame->m_pPrintScreen->CompareImage(CIN_CanLogin, nullptr, 1)) {
							m_pGame->SetStatus(m->Account, ACCSTA_LOGIN, true); // ���õ�¼״̬

							Tap(1205, 155, m->Index);
							LOGVARP2(log, "blue", "�����¼�ʺ�ͼ��");
							Sleep(1000);

#if 1
							m_pGame->m_pGameProc->DBClick(520, 230, m->Wnd);
							LOGVARP2(log, "blue", "˫���˺ſ�");
							Sleep(300);
							Key(67, m->Index); // 67Ϊ�˸��
							Sleep(300);
#else
							Tap(755, 230, m->Index);
							LOGVARP2(log, "blue", "����˺ſ�");
							Sleep(500);
							for (int i = 0; i < strlen(m->Account->Name) + 1; i++) {
								Key(67, m->Index); // 67Ϊ�˸��
								Sleep(100);
							}
#endif					
							Text(m->Account->Name, m->Index);
							LOGVARP2(log, "blue", "�����ʺ�");
							Sleep(300);

							Tap(720, 300, m->Index);
							Sleep(300);
							Text(m->Account->Password, m->Index);
							LOGVARP2(log, "blue", "��������");
							Sleep(500);

							Tap(575, 405, m->Index);
							LOGVARP2(log, "blue", "�����¼");

							m_List[i].Account->IsLogin = 1;

							Sleep(2500);
							m_pGame->m_pGameProc->Click(510, 380, 0xff, m->Wnd); // ����Ժ��
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
		LOGVARN2(64, "blue", "��ȫ��������Ϸ");
#endif

	return;
}

// list2����
int Emulator::List2()
{
	while (m_bIsGetList);
	//printf("Emulator::List2()\n");
	m_bIsGetList = true;
	m_nCount = List2(m_List, sizeof(m_List) / sizeof(MNQ));
	m_bIsGetList = false;
	return m_nCount;
}

// list2����
int Emulator::List2(MNQ* mnq, int length)
{
	int count = 0;
	char result[1024] = { 0 };
	if (ExecCmd("list2", result, sizeof(result))) {
		//printf("���:\n--------------------\n%s--------------------\n", result);
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

				if (m->VBoxPid > 0 && !m->Account) { // �Ƿ��Ѱ����ʺ�
					//printf("get account_index\n");
					int account_index = 0;// Getprop("account_index", count, -1);
					m->Account = m_pGame->GetAccount(account_index);
					//printf("xxx:%d\n", account_index);
					if (m->Account) {
						m->Account->IsLogin = 1;
						m->Account->Mnq = m;
					}
				}
				if (oldInit && !m->Init) { // ˵��ģ�����˳���
					if (m->Account) { // �����ʺ��˳�
						printf("%s ����Ϊ������\n", m->Account->Name);
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

// ��������
void Emulator::Setprop(const char * key, int value, int index)
{
	char v[16];
	sprintf_s(v, "%d", value);
	Setprop(key, v, index);
}

// ��������
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

// ��ȡ����
bool Emulator::Getprop(const char * key, char * result, int size, int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "getprop --index %d --key \"%s\"", index, key);
	return ExecCmd(cmd, result, sizeof(result));
}

// ��ȡ����
int Emulator::Getprop(const char * key, int index, int d_v)
{
	char result[128] = { 0 };
	if (!Getprop(key, result, sizeof(result), index))
		return d_v;

	trim(result);
	return result[0] >= '0'&&result[0] <= '9' ? atoi(result) : d_v;
}

// ��ȡ������Ե�ģ��������
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

// ������
void Emulator::ReName(const char * name, int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "rename --index %d --title \"%s\"", index, name);
	ExecCmd(cmd, nullptr, 0);
}

// ���
void Emulator::Tap(int x, int y, int index)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "input tap %d %d", x, y);
	ExecCmdLd(cmd, index);
}

// ���
void Emulator::Tap(int x, int y, int x2, int y2, int index)
{
	Tap(MyRand(x, x2), MyRand(y, y2), index);
}

// ����
void Emulator::Swipe(int x, int y, int x2, int y2, int index, int ms)
{
	char cmd[128] = { 0 };
	sprintf_s(cmd, "input swipe %d %d %d %d %d", x, y, x2, y2, ms);
	ExecCmdLd(cmd, index);
}

// ����
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

// ����ģ��������ʱ��
int Emulator::SetStartTime(int index, int v)
{
	MNQ* p = GetMNQ(index);
	if (p == nullptr)
		return false;

	p->StartTime = v == -1 ? time(nullptr) : v;
	return p->StartTime;
}

// ��ȡģ������������
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