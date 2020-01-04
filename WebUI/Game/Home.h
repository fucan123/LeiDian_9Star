#pragma once

#include "HttpClient.h"
#include <string>

#define HOME_HOST      L"fz.myhostcpp.com"
#define HOME_GAME_FLAG "1"

class Home
{
public:
	Home();
	// 设置是否免费
	void SetFree(bool v);
	// 是否有效
	bool IsValid();
	// 充值
	bool Recharge(const char* card);
	// 验证
	bool Verify();
	// 获得此机器有效时间（秒）
	int  GetExpire();
	// 解析返回结果
	void Parse();
	// 获得返回结果中的值
	bool GetValue(char* key, char value[], int length);
	// 获得返回结果中的值
	int  GetValue(char* key, int length, int default_value=0);
	// 获得返回结果中的值
	int  GetValue(const char* key, int length, int default_value = 0);
	// 设置验证时间
	int  SetVerifyTime();
	// 设置到期时间
	int  SetEndTime();
	// 设置到期时间字符串
	void SetExpireTime_S();
	// 获得到期时间字符串
	std::string& GetExpireTime_S();
	void SetError(int code, const char* str);
	void SetError(int code, wchar_t* str);
	void SetError(int code, CString str);
	void SetError(int code, const char* str, DWORD status);
	void SetError(int code, wchar_t* str, DWORD status);
	void SetErrorCode(int code);
	void SetMsgStr(wchar_t* str);
	void SetMsgStr(CString str);
	void SetMsgStr(wchar_t* str, DWORD status);
	void SetMsgStr(const char* str, DWORD status);
	std::string& GetMsgStr();
protected:
	// 返回结果指针
	char* m_pRepsone;
	// 验证时间（秒）
	int   m_iVerifyTime;
	// 过期时间
	int m_iExpire;
	// 到期时间
	int   m_iEndTime;

	char m_MachineId[33];
	int  m_Status;
	int  m_Error;

	// 是否免费
	bool m_bFree = false;

	// 网站返回内容
	std::string m_sResult;
	// 提示内容
	std::string m_MsgStr;
	// 过期时间
	std::string m_sExpireTime;
};