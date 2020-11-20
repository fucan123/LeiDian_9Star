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
	printf("当前文件夹:%s %s\n", floder, argv[1]);
	//sprintf_s(floder, "E:\\CPP\\OpenglPrintScreen\\Release\\opengl_ps.dll");
	if (argc == 2) {
		int pid = atoi(argv[1]);
		if (InjectDll(pid, floder)) {
			printf("注入成功\n");
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
				::printf("WriteXY->无法写入目的坐标(%d) %08X\n", GetLastError(), addr);
			}
		}
	}
	else {
		printf("WriteXY->参数错误\n");
	}

	//system("pause");
	return 0;
}

BOOL InjectDll(DWORD dwPID, char* szDllPath)
{
	if (CheckDllInProcess(dwPID, szDllPath))
		return TRUE;

	HANDLE                  h_token;
	HANDLE                  hProcess = NULL; // 保存目标进程的句柄
	LPVOID                  pRemoteBuf = NULL; // 目标进程开辟的内存的起始地址
	DWORD                   dwBufSize = (DWORD)(strlen(szDllPath) + 1) * sizeof(char); // 开辟的内存的大小
	LPTHREAD_START_ROUTINE  pThreadProc = NULL; // loadLibreayW函数的起始地址
	HMODULE                 hMod = NULL; // kernel32.dll模块的句柄
	BOOL                    bRet = FALSE;
	HANDLE                  rt = NULL;

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID))) { // 打开目标进程，获得句柄
		printf("InjectDll() : OpenProcess(%d) failed!!! [%d]\n",
			dwPID, GetLastError());
		goto INJECTDLL_EXIT;
	}
	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize,
		MEM_COMMIT, PAGE_READWRITE);
	if (pRemoteBuf == NULL) { //在目标进程空间开辟一块内存
		printf("InjectDll() : VirtualAllocEx() failed!!! [%d]\n",
			GetLastError());
		goto INJECTDLL_EXIT;
	}
	if (!WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllPath, dwBufSize, NULL)) { // 向开辟的内存复制dll的路径
		printf("InjectDll() : WriteProcessMemory() failed!!! [%d]\n",
			GetLastError());
		goto INJECTDLL_EXIT;
	}
	hMod = GetModuleHandle("kernel32.dll"); // 获得本进程kernel32.dll的模块句柄
	if (hMod == NULL) {
		printf("InjectDll() : GetModuleHandle(\"kernel32.dll\") failed!!! [%d]\n",
			GetLastError());
		goto INJECTDLL_EXIT;
	}
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryA"); // 获得LoadLibraryW函数的起始地址
	if (pThreadProc == NULL) {
		printf("InjectDll() : GetProcAddress(\"LoadLibraryA\") failed!!! [%d]\n",
			GetLastError());
		goto INJECTDLL_EXIT;
	}
	rt = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);
	printf("进程->ID: %d, %s(%d) LoadLibraryW: %08x --- rt: %08X\n", dwPID, szDllPath, dwBufSize, pThreadProc, rt);
	if (!rt) { // 执行远程线程
		printf("InjectDll() : MyCreateRemoteThread() failed!!!\n");
		goto INJECTDLL_EXIT;
	}
	printf("Last Error:%d %s\n", GetLastError(), szDllPath);
	Sleep(100);
INJECTDLL_EXIT:
	bRet = CheckDllInProcess(dwPID, szDllPath); // 确认结果
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
		(hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID))) { // 获得进程的快照
		return FALSE;
	}
	bMore = Module32First(hSnapshot, &me); // 遍历进程内得的所有模块
	for (; bMore; bMore = Module32Next(hSnapshot, &me)) {
		//_tprintf(L"%ws %ws\n", me.szModule, me.szExePath);
		if (!strcmp(me.szModule, szDllPath) || !strcmp(me.szExePath, szDllPath)) { //模块名或含路径的名相符
			CloseHandle(hSnapshot);
			return TRUE;
		}
		me.modBaseAddr;
	}
	CloseHandle(hSnapshot);
	return FALSE;
}