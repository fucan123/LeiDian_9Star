#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>

BOOL CheckDllInProcess(DWORD dwPID, char* szDllPath);
BOOL InjectDll(DWORD dwPID, char* szDllPath);

int main(int argc, char** argv)
{
	char path[MAX_PATH], floder[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);
	sprintf_s(floder, "%s\\files\\opengl_ps.dll", path);
	printf("��ǰ�ļ���:%s %s\n", floder, argv[1]);
	//sprintf_s(floder, "E:\\CPP\\OpenglPrintScreen\\Release\\opengl_ps.dll");
	if (argc == 2) {
		int pid = atoi(argv[1]);
		if (InjectDll(pid, floder)) {
			printf("ע��ɹ�\n");
		}
	}
	else if (argc >= 5) {
		int pid = atoi(argv[1]);
		int addr = atoi(argv[2]);
		int x = atoi(argv[3]);
		int y = atoi(argv[4]);

		HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (h) {
			DWORD write_len = 0;
			DWORD data[] = { x, y };

			DWORD old_p;
			//VirtualProtect((PVOID)m_DataAddr.MoveX, sizeof(data), PAGE_READWRITE, &old_p);

			if (!WriteProcessMemory(h, (PVOID)addr, data, sizeof(data), NULL)) {
				::printf("WriteXY->�޷�д��Ŀ������(%d) %08X\n", GetLastError(), addr);
			}
		}
	}
	else {
		printf("WriteXY->��������\n");
	}

	//system("pause");
	return 0;
}

BOOL InjectDll(DWORD dwPID, char* szDllPath)
{
	if (CheckDllInProcess(dwPID, szDllPath))
		return TRUE;

	HANDLE                  h_token;
	HANDLE                  hProcess = NULL; // ����Ŀ����̵ľ��
	LPVOID                  pRemoteBuf = NULL; // Ŀ����̿��ٵ��ڴ����ʼ��ַ
	DWORD                   dwBufSize = (DWORD)(strlen(szDllPath) + 1) * sizeof(char); // ���ٵ��ڴ�Ĵ�С
	LPTHREAD_START_ROUTINE  pThreadProc = NULL; // loadLibreayW��������ʼ��ַ
	HMODULE                 hMod = NULL; // kernel32.dllģ��ľ��
	BOOL                    bRet = FALSE;
	HANDLE                  rt = NULL;

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID))) { // ��Ŀ����̣���þ��
		printf("InjectDll() : OpenProcess(%d) failed!!! [%d]\n",
			dwPID, GetLastError());
		goto INJECTDLL_EXIT;
	}
	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize,
		MEM_COMMIT, PAGE_READWRITE);
	if (pRemoteBuf == NULL) { //��Ŀ����̿ռ俪��һ���ڴ�
		printf("InjectDll() : VirtualAllocEx() failed!!! [%d]\n",
			GetLastError());
		goto INJECTDLL_EXIT;
	}
	if (!WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllPath, dwBufSize, NULL)) { // �򿪱ٵ��ڴ渴��dll��·��
		printf("InjectDll() : WriteProcessMemory() failed!!! [%d]\n",
			GetLastError());
		goto INJECTDLL_EXIT;
	}
	hMod = GetModuleHandle("kernel32.dll"); // ��ñ�����kernel32.dll��ģ����
	if (hMod == NULL) {
		printf("InjectDll() : GetModuleHandle(\"kernel32.dll\") failed!!! [%d]\n",
			GetLastError());
		goto INJECTDLL_EXIT;
	}
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryA"); // ���LoadLibraryW��������ʼ��ַ
	if (pThreadProc == NULL) {
		printf("InjectDll() : GetProcAddress(\"LoadLibraryA\") failed!!! [%d]\n",
			GetLastError());
		goto INJECTDLL_EXIT;
	}
	rt = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);
	printf("����->ID: %d, %s(%d) LoadLibraryW: %08x --- rt: %08X\n", dwPID, szDllPath, dwBufSize, pThreadProc, rt);
	if (!rt) { // ִ��Զ���߳�
		printf("InjectDll() : MyCreateRemoteThread() failed!!!\n");
		goto INJECTDLL_EXIT;
	}
	printf("Last Error:%d %s\n", GetLastError(), szDllPath);
	Sleep(100);
INJECTDLL_EXIT:
	bRet = CheckDllInProcess(dwPID, szDllPath); // ȷ�Ͻ��
	if (pRemoteBuf)
		VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
	if (hProcess)
		CloseHandle(hProcess);
	return bRet;
}

BOOL CheckDllInProcess(DWORD dwPID, char* szDllPath)
{
	BOOL                    bMore = FALSE;
	HANDLE                  hSnapshot = INVALID_HANDLE_VALUE;
	MODULEENTRY32           me = { sizeof(me), };

	if (INVALID_HANDLE_VALUE ==
		(hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID))) { // ��ý��̵Ŀ���
		return FALSE;
	}
	bMore = Module32First(hSnapshot, &me); // ���������ڵõ�����ģ��
	for (; bMore; bMore = Module32Next(hSnapshot, &me)) {
		//_tprintf(L"%ws %ws\n", me.szModule, me.szExePath);
		if (!strcmp(me.szModule, szDllPath) || !strcmp(me.szExePath, szDllPath)) { //ģ������·���������
			CloseHandle(hSnapshot);
			return TRUE;
		}
		me.modBaseAddr;
	}
	CloseHandle(hSnapshot);
	return FALSE;
}