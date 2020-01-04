#pragma once
#include <devioctl.h>
#include <My/Driver/Sys.h>

#define IOCTL_SET_INJECT_X86DLL \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_SET_INJECT_X64DLL \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_GET_THREADINFO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_PROTECT_PID \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x100, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

class JsCall;
class Game;
class Driver
{
public:
	// ...
	Driver(Game* p);
	// 安装Dll驱动
	bool InstallDll();
	// 卸载
	bool UnStall();
	// 设置要注入的DLL
	bool SetDll();
	// 读取文件
	PVOID MyReadFile(const CHAR* fileName, PULONG fileSize);
public:
	JsCall* m_pJsCall;
	Game* m_pGame;

	bool  m_bIsInstallDll = false;
	Sys m_SysDll;
};