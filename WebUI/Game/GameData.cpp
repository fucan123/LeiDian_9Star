#include "Game.h"
#include "GameData.h"
#include "Emulator.h"
#include "Move.h"
#include "Talk.h"
#include "GameProc.h"
#include <My/Common/func.h>
#include <My/Win32/Peb.h>
#include <time.h>

#define MNQ_TITLE "�׵�ģ����"
#define MNQ_PNAME L"LdBoxHeadless.exe"

GameData::GameData(Game* p)
{
	m_pGame = p;
	m_pReadBuffer = new BYTE[0x10000];

	ZeroMemory(&m_DataAddr, sizeof(m_DataAddr));

	CreateShareMemory();
}

// ������Ϸ
void GameData::WatchGame()
{
	while (true) {
		int length = m_pGame->m_pEmulator->List2();
		for (int i = 0; i < length; i++) {
			MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
			if (!m || m->VBoxPid <= 0)
				continue;
			if (!m->Account || !m->Account->IsLogin || m->Account->IsGetAddr)
				continue;

			ZeroMemory(&m_DataAddr, sizeof(m_DataAddr));
			m_hGameProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m->VBoxPid);
			printf("%s ��ȡ[%s(%d)]����\n", m->Account->Name, m->Name, m->VBoxPid);
			ReadGameMemory();

			if (m_DataAddr.Player) {
				m->Process = m_hGameProcess;
				m_pGame->SetGameAddr(m->Account, &m_DataAddr);

				while (m_dwX == 0) {
					ReadCoor(nullptr, nullptr, m->Account);
					Sleep(100);
				}

				m->Account->PlayTime = time(nullptr);

				char name[16] = { 0 };
				if (ReadName(name, m->Account)) {
					if (m_dwX && m_dwY) {
						while (true) {
							Sleep(5000);
							if (m_DataAddr.MoveX)
								break;

							m_pGame->m_pGameProc->SwitchGameWnd(m->Wnd);
							m_pGame->m_pTalk->CloseLGCBox();
							m_pGame->m_pMove->OpenMap(m->Account);
							Sleep(100);
							m_pGame->m_pGameProc->Click(552, 300, 600, 350, 0xff, m->Wnd); // �����ͼ�ſ���������Ŀ�ĵ�������
							Sleep(100);
							printf("%sѰ��Ŀ�ĵص�ַ..................\n", m->Account->Name);
							ReadGameMemory();
							m_pGame->m_pTalk->CloseLGCBox();
							m_pGame->m_pMove->CloseMap(m->Wnd);
						}
						
					}
					if (m->Account->IsBig) {
						m_hWndBig = m->Wnd;
						m_hProcessBig = m_hGameProcess;

						m_pGame->m_hWndBig = m->Wnd;
						m_pGame->m_hProcessBig = m_hGameProcess;

						m_pAccoutBig = m->Account;
					}

					m_pGame->SetGameAddr(m->Account, &m_DataAddr);
					strcpy(m->Account->Role, name);
					ReadLife(nullptr, nullptr, m->Account);
					printf("%s ��ɫ����:%s ����:%d,%d Ѫ��:%d/%d %s\n", 
						m->Account->Name, m->Account->Role, m_dwX, m_dwY,
						m_dwLife, m_dwLifeMax,
						m->Account->IsBig ? "���" : "С��");

					if (m->Account->IsBig)
						goto _end_;
				}
			}
			else {
				CloseHandle(m_hGameProcess);
			}	
		}
		Sleep(8000);
	}
_end_:
	return;
}

// ��ȡ��Ϸ����
void GameData::FindGameWnd()
{
	::EnumWindows(EnumProc, (LPARAM)this);
}

// ö�ٴ���
BOOL GameData::EnumProc(HWND hWnd, LPARAM lParam)
{
	char name[32] = { 0 };
	::GetWindowTextA(hWnd, name, sizeof(name));
	if (strcmp(name, MNQ_TITLE) == 0) {
		HWND children = ::FindWindowEx(hWnd, NULL, L"RenderWindow", NULL);
		printf("%s:%08X.%08X\n", name, hWnd, children);
		bool isbig = false;
		bool set = ((GameData*)lParam)->m_pGame->SetGameWnd(children, &isbig);
		if (isbig) {
			((GameData*)lParam)->m_hWndBig = children;
		}
		return set ? FALSE : TRUE;
	}
	return TRUE;
}

// �Ƿ���ָ���������� allow=���
bool GameData::IsInArea(int x, int y, int allow, _account_* account)
{
	DWORD pos_x = 0, pos_y = 0;
	ReadCoor(&pos_x, &pos_y, account);
	if (allow == 0)
		return x == pos_x && y == pos_y;

	int cx = (int)pos_x - x;
	int cy = (int)pos_y - y;

	//printf("IsInArea:%d,%d %d,%d\n", pos_x, pos_y, cx, cy);

	return abs(cx) <= allow && abs(cy) <= allow;
}

// �Ƿ���ָ���������� allow=���
bool GameData::IsNotInArea(int x, int y, int allow, _account_* account)
{
	ReadCoor(nullptr, nullptr, account);
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	return abs(cx) > allow || abs(cy) > allow;
}

// �Ƿ������
bool GameData::IsInShenDian(_account_* account)
{
	return IsInArea(60, 60, 30, account);
}

// �Ƿ��ڸ����ſ�
bool GameData::IsInFBDoor(_account_* account)
{
	return IsInArea(865, 500, 50, account);
}

// ��ȡ�����׵�ַ
bool GameData::FindPlayerAddr()
{
	//printf("FindPlayerAddr\n");
	// 4:0x072CD498 4:0x00 4:0xFFFFFFFF 4:0x3F800000 4:0x00010001 4:0x000 4:0x072C4678 4:0x9A078100

	// 4:0x00000000 4:0x0000DECE 4:0x00000000 4:0x00000001 4:0x00000000 4:0x00000030 4:0x00000000 4:0x0000DECE 4:0x00000000 4:0x00000001 4:0x00000000 4:0x00000030
	DWORD codes[] = {
		0x072CE498, 0x00000000, 0xFFFFFFFF, 0x3F800000,
		0x00010001, 0x00000011, 0x00000011, 0x00000011,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_DataAddr.Player = address;
		m_DataAddr.Name = m_DataAddr.Player + NAME_OFFSET;
		m_DataAddr.CoorX = m_DataAddr.Player + X_OFFSET;
		m_DataAddr.CoorY = m_DataAddr.Player + Y_OFFSET;
		m_DataAddr.Life = m_DataAddr.Player + LIFE_OFFSET;
		m_DataAddr.LifeMax = m_DataAddr.Player + LIFE_MAX_OFFSET;

		LOGVARN2(32, "blue", "�����׵�ַ:%08X", m_DataAddr.Player);
		::printf("�����׵�ַ:%08X\n", m_DataAddr.Player);
	}

	return address != 0;
}

// ��ȡĿ�ĵ������ַ
bool GameData::FindMoveCoorAddr()
{
	// 4:0x00000000 4:0x00000000 4:0x00000000 4:0x07328EF4 4:0x07328EF4
	DWORD codes[] = {
		0x00000011, 0x00000011, 0x00000000, 0x00000000,
		0x073330A0, 0x07333098, 0x00000000, 0x00000022,
		0x00000022, 0x00000022, 0x00000011, 0x00000000,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_DataAddr.MoveX = address + 0x30;
		m_DataAddr.MoveY = m_DataAddr.MoveX + 4;

		LOGVARN2(32, "blue", "Ŀ�ĵ������ַ:%08X", m_DataAddr.MoveX);
		::printf("Ŀ�ĵ������ַ:%08X\n", m_DataAddr.MoveX);
	}

	return address != 0;
}

// ��ȡ��ɫ
bool GameData::ReadName(char* name, _account_* account)
{
	account = account ? account : m_pAccoutBig;
	if (!ReadMemory((PVOID)account->Addr.Name, name, 16, account)) {
		::printf("�޷���ȡ��ɫ����(%d)\n", GetLastError());
		return false;
	}
	return true;
}

// ��ȡ����
bool GameData::ReadCoor(DWORD * x, DWORD * y, _account_* account)
{
	account = account ? account : m_pAccoutBig;
	if (!account->Addr.CoorX || !account->Addr.CoorY)
		return false;

	DWORD pos_x = 0, pos_y = 0;
	if (!ReadDwordMemory(account->Addr.CoorX, pos_x, account)) {
		::printf("�޷���ȡ����X(%d) %08X\n", GetLastError(), account->Addr.CoorX);
		return false;
	}
	if (!ReadDwordMemory(account->Addr.CoorY, pos_y, account)) {
		::printf("�޷���ȡ����Y(%d) %08X\n", GetLastError(), account->Addr.CoorY);
		return false;
	}

	m_dwX = pos_x;
	m_dwY = pos_y;
	if (x) {
		*x = pos_x;
	}
	if (y) {
		*y = pos_y;
	}
	return true;
}

// ��ȡ����ֵ
DWORD GameData::ReadLife(DWORD* life, DWORD* life_max, _account_* account)
{
	account = account ? account : m_pAccoutBig;
	ReadDwordMemory(account->Addr.Life, m_dwLife, account);
	ReadDwordMemory(account->Addr.LifeMax, m_dwLifeMax, account);

	if (life) {
		*life = m_dwLife;
	}
	if (life_max) {
		*life_max = m_dwLifeMax;
	}
	return m_DataAddr.Life;
}

// ���������ڴ�
void GameData::CreateShareMemory()
{
	m_hShareMap = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024, L"Share_Write_XY");
	if (!m_hShareMap) {
		m_pShareBuffer = nullptr;
		printf("CreateFileMappingʧ��\n");
		return;
	}
	// ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬�������������
	m_pShareBuffer = (ShareWriteXYData*)::MapViewOfFile(m_hShareMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	// ��ʼ��
	ZeroMemory(m_pShareBuffer, 1024);
}

// д��Ŀ�ĵ�
void GameData::WriteMoveCoor(DWORD x, DWORD y, _account_* account)
{
	account = account ? account : m_pAccoutBig;
	if (!account || !account->Mnq)
		return;

	DWORD old[2] = { 0, 0 };
	if (ReadMemory((PVOID)account->Addr.MoveX, old, sizeof(old))) {
		//printf("old:%d,%d\n", old[0], old[1]);
		if (old[0] == x && old[1] == y)
			return;
	}

	SIZE_T write_len = 0;
	DWORD data[] = { x, y };

	DWORD old_p;
	//VirtualProtect((PVOID)m_DataAddr.MoveX, sizeof(data), PAGE_READWRITE, &old_p);
	
	if (1 && WriteProcessMemory(account->Mnq->Process, (PVOID)account->Addr.MoveX, data, sizeof(data), &write_len)) {
		//printf("WriteMoveCoor����:%d\n", write_len);
	}
	else {
		//::printf("�޷�д��Ŀ������(%d) %08X\n", GetLastError(), account->Addr.MoveX);
		if (!m_pShareBuffer) {
			printf("�޷���������\n");
			return;
		}

		if (!m_bInDll) {
			char cmd[128];
			sprintf_s(cmd, "%s\\win7\\\WriteXY.exe %d", m_pGame->m_chPath, account->Mnq->VBoxPid);
			printf("����WriteXY����ע��DLL:%s\n", cmd);
			system(cmd);
			m_bInDll = true;
			Sleep(100);
		}

		// ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬�������������
		m_pShareBuffer = (ShareWriteXYData*)::MapViewOfFile(m_hShareMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		m_pShareBuffer->AddrX = account->Addr.MoveX;
		m_pShareBuffer->AddrY = account->Addr.MoveY;
		m_pShareBuffer->X = x;
		m_pShareBuffer->Y = y;
		m_pShareBuffer->Flag = 1; // Dllд��
		printf("�ȴ�д�����...\n");
		while (m_pShareBuffer->Flag == 1) { // �ȴ�Dllд�����
			//printf("m_pShareBuffer->Flag:%d\n", m_pShareBuffer->Flag);
			Sleep(10);
		}
		printf("�Ѿ����д��!!!\n");
	}
}

// ����������
DWORD GameData::SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length, DWORD step)
{
	if (length == 0 || save_length == 0)
		return 0;

	DWORD count = 0;
	for (DWORD i = 0; i < m_dwReadSize; i += step) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // �����Լ�
			//::printf("���������Լ�:%08X\n", codes);
			return 0;
		}

		DWORD* dw = (DWORD*)(m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // �����
				result = true;
			}
			else if (codes[j] == 0x22) { // ��Ҫ��ֵ��Ϊ0
				if (dw[j] == 0) {
					result = false;
					break;
				}
			}
			else if (((codes[j] & 0xffff0000) == 0x12340000)) { // ��2�ֽ����
				if ((dw[j] & 0x0000ffff) != (codes[j] & 0x0000ffff)) {
					result = false;
					break;
				}
				else {
					//::printf("%08X\n", dw[j]);
				}
			}
			else if (((codes[j] & 0x0000ffff) == 0x00001234)) { // ��2�ֽ����
				if ((dw[j] & 0xffff0000) != (codes[j] & 0xffff0000)) {
					result = false;
					break;
				}
			}
			else {
				if ((codes[j] & 0xff00) == 0x1100) { // �Ƚ�������ַ��ֵ��� ���8λΪ�Ƚ�����
					//::printf("%X:%X %08X:%08X\n", j, codes[j] & 0xff, dw[j], dw[codes[j] & 0xff]);
					if (dw[j] != dw[codes[j] & 0xff]) {
						result = false;
						break;
					}
				}
				else if (dw[j] != codes[j]) { // ������ֵ�����
					result = false;
					break;
				}
			}
		}

		if (result) {
			save[count++] = addr;
			//::printf("��ַ:%08X   %08X\n", addr, codes);
			if (count == save_length)
				break;
		}
	}

	return count;
}

// ����������
DWORD GameData::SearchByteCode(BYTE * codes, DWORD length)
{
	if (length == 0)
		return 0;

	for (DWORD i = 0; i < m_dwReadSize; i++) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // �����Լ�
			//::printf("���������Լ�:%08X\n", codes);
			return 0;
		}

		BYTE* data = (m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // �����
				result = true;
			}
			else if (codes[j] == 0x22) { // ��Ҫ��ֵ��Ϊ0
				if (data[j] == 0) {
					result = false;
					break;
				}
			}
			else if (codes[j] != data[j]) {
				result = false;
				break;
			}
		}

		if (result)
			return addr;
	}

	return 0;
}

// ��ȡ���ֽ�����
bool GameData::ReadDwordMemory(DWORD addr, DWORD& v, _account_* account)
{
	return ReadMemory((PVOID)addr, &v, 4, account);
}

// ��ȡ�ڴ�
bool GameData::ReadMemory(PVOID addr, PVOID save, DWORD length, _account_* account)
{
	if (!account)
		account =  m_pGame->m_pBig;
	if (!account || !account->Mnq)
		return false;

	SIZE_T dwRead = 0;
	bool result = ReadProcessMemory(account->Mnq->Process, addr, save, length, &dwRead);
	//::printf("ReadProcessMemory:%d %08X %d Read:%d ��ֵ:%d(%08X)\n", GetLastError(), addr, result, dwRead, *(DWORD*)save, *(DWORD*)save);

	if (!result || dwRead != length) {
		::printf("ReadProcessMemory����:%d %08X %d\n", GetLastError(), addr, result);
		if (GetLastError() == 6) {
			m_hProcessBig = GetCurrentProcess();
			return ReadProcessMemory(m_hProcessBig, addr, save, length, NULL);
		}
	}

	return result;
}


// ��ȡ��Ϸ�ڴ�
bool GameData::ReadGameMemory(DWORD flag)
{
	m_bIsReadEnd = false;

	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	DWORD_PTR MaxPtr = 0x70000000; // ����ȡ�ڴ��ַ
	DWORD_PTR max = 0;


	DWORD ms = GetTickCount();
	DWORD_PTR ReadAddress = 0x00000000;
	ULONG count = 0, failed = 0;
	//::printf("fuck\n");
	while (ReadAddress < MaxPtr)
	{
		SIZE_T r = VirtualQueryEx(m_hGameProcess, (LPCVOID)ReadAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
		SIZE_T rSize = 0;

		//::printf("r:%d\n", r);
		//::printf("ReadAddress:%08X - %08X-----%X\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
		if (r == 0) {
			::printf("r:%d -- %p\n", (int)r, ReadAddress);
			break;
		}

		if (mbi.Type != MEM_PRIVATE && mbi.Protect != PAGE_READWRITE) {
			//::printf("%p-%p ===����, ��С:%d %08X %08X\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize, mbi.Type, mbi.Protect);
			goto _c;
		}
		else {
			DWORD pTmpReadAddress = ReadAddress;
			DWORD dwOneReadSize = 0x1000; // ÿ�ζ�ȡ����
			DWORD dwReadSize = 0x00;      // �Ѷ�ȡ����
			while (dwReadSize < mbi.RegionSize) {
				m_dwReadBase = pTmpReadAddress;
				m_dwReadSize = (dwReadSize + dwOneReadSize) <= mbi.RegionSize
					? dwOneReadSize : mbi.RegionSize - dwReadSize;

				if (ReadProcessMemory(m_hGameProcess, (LPVOID)pTmpReadAddress, m_pReadBuffer, m_dwReadSize, NULL)) {
					//::printf("flag:%08X %p-%p\n", flag, ReadAddress, ReadAddress + mbi.RegionSize);

					if (flag & 0x01) {
						DWORD find_num = 0;
						if (!m_DataAddr.Player) {
							if (FindPlayerAddr())
								find_num++;
						}
						if (!m_DataAddr.MoveX) {
							if (FindMoveCoorAddr())
								find_num++;
						}
						if (find_num == 2)
							flag = 0;
					}
					if (!flag) {
						printf("������ȫ���ҵ�\n");
						goto end;
					}
				}
				else {
					//::printf("%p-%p === ��ȡʧ��, ��С:%d, ������:%d\n", pTmpReadAddress, pTmpReadAddress + m_dwReadSize, (int)m_dwReadSize, GetLastError());
				}

				dwReadSize += m_dwReadSize;
				pTmpReadAddress += m_dwReadSize;
			}

			count++;
		}
	_c:
		// ��ȡ��ַ����
		ReadAddress += mbi.RegionSize;
		//Sleep(8);
		//::printf("%016X---�ڴ��С2:%08X\n", ReadAddress, mbi.RegionSize);
		// ɨ0x10000000�ֽ��ڴ� ����100����
	}
end:
	DWORD ms2 = GetTickCount();
	::printf("��ȡ���ڴ���ʱ:%d����\n", ms2 - ms);
	m_bIsReadEnd = true;
	return true;
}