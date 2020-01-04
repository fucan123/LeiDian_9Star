#include "../Web/JsCall.h"
#include "Driver.h"
#include "Game.h"
#include <My/Common/func.h>
#include <stdio.h>

// ...
Driver::Driver(Game * p)
{
	m_pJsCall = p->m_pJsCall;
	m_pGame = p;
}

// 安装Dll驱动
bool Driver::InstallDll()
{
	if (!m_bIsInstallDll) {
		wchar_t path[255];
#ifdef DEBUG
		SHGetSpecialFolderPathW(0, path, CSIDL_DESKTOPDIRECTORY, 0);
		wcscat(path, L"\\MNQ-9Star\\firenet.sys");
#else
		GetCurrentDirectoryW(MAX_PATH, path);
		wcscat(path, L"\\firenet.sys");
#endif // DEBUG

		if (!IsFileExist(path)) {
			m_pJsCall->ShowMsg("缺少必需文件:firenet.sys", "文件不存在", 2);
			LOG2("firenet.sys不存在", "red");
			m_bIsInstallDll = false;
			return false;
		}
		if (m_SysDll.Install(L"firenet_safe", L"safe fire", path)) {
			LOG2("安装驱动成功", "green");
			SetDll();
			m_bIsInstallDll = true;
			return true;
		}
		else {
			m_SysDll.UnInstall();
			LOG2("安装驱动失败, 请重试", "red");
			//MessageBox(NULL, "安装驱动失败", "提示", MB_OK);
			return false;
		}
	}
	else {
		if (m_SysDll.UnInstall()) {
			LOG2("卸载驱动成功", "green");
			m_bIsInstallDll = false;
			return true;
		}
		else {
			LOG2("卸载驱动失败", "red");
			//MessageBox(NULL, "卸载驱动失败", "提示", MB_OK);
			return false;
		}
	}

}

// 卸载
bool Driver::UnStall()
{
	return m_SysDll.UnInstall();
}

// 设置要注入的DLL
bool Driver::SetDll()
{
	BOOL	result;
	DWORD	returnLen;
	char	output;

	HANDLE	hDevice = NULL;

	PVOID	dllx64Ptr = NULL;
	PVOID	dllx86Ptr = NULL;

	ULONG	dllx64Size = 0;
	ULONG	dllx86Size = 0;

	hDevice = CreateFileA("\\\\.\\CrashDumpUpload",
		NULL,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		m_pJsCall->ShowMsg("连接驱动失败.", "提示", 2);
		return false;
	}

	DWORD pid = GetCurrentProcessId();
	result = DeviceIoControl(
		hDevice,
		IOCTL_SET_PROTECT_PID,
		&pid,
		4,
		&output,
		sizeof(char),
		&returnLen,
		NULL);
	printf("保护进程ID:%d %d\n", pid, result);

	char file[255];
#ifndef _DEBUG
	GetCurrentDirectoryA(MAX_PATH, file);
	strcat(file, "\\9Star.dll");
#else
#if 1
	strcpy(file, "C:\\Users\\fucan\\Desktop\\MNQ-9Star\\vs\\9Star.dll");
#else
	strcpy(file, "E:\\CPP\\DLL_Test\\Debug\\2Star.dll");
#endif
	printf("file:%s\n", file);
#endif
	dllx86Ptr = MyReadFile(file, &dllx86Size);
	if (dllx86Ptr == NULL) {
		LOG2("找不到文件9Star.dll", "red");
		return false;
	}
	// 1234
	// 1 2 3 4 5 6 7 8 9 10
	// 11 12 13 14 15

	result = DeviceIoControl(
		hDevice,
		IOCTL_SET_INJECT_X86DLL,
		dllx86Ptr,
		dllx86Size,
		&output,
		sizeof(char),
		&returnLen,
		NULL);

	if (dllx86Ptr)
	{
		free(dllx86Ptr);
	}
	if (result) {
		LOG2("设置dll成功", "green");
	}
	else {
		LOG2("设置dll失败", "red");
	}

	if (hDevice != NULL) {
		CloseHandle(hDevice);
	}
	return true;
}

// 读取文件
PVOID Driver::MyReadFile(const CHAR* fileName, PULONG fileSize)
{
	HANDLE fileHandle = NULL;
	DWORD readd = 0;
	PVOID fileBufPtr = NULL;

	fileHandle = CreateFileA(
		fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		*fileSize = 0;
		return NULL;
	}

	*fileSize = GetFileSize(fileHandle, NULL);

	fileBufPtr = calloc(1, *fileSize);

	if (!ReadFile(fileHandle, fileBufPtr, *fileSize, &readd, NULL))
	{
		free(fileBufPtr);
		fileBufPtr = NULL;
		*fileSize = 0;
	}

	CloseHandle(fileHandle);
	return fileBufPtr;
}