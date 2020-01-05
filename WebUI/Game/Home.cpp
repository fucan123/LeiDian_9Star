#include "Home.h"
#include <My/Common/MachineID.h>
#include <time.h>


Home::Home()
{
	m_bFree = false;

	m_iVerifyTime = 0;
	m_iExpire = 0;
	m_iEndTime = 0;
	m_pRepsone = nullptr;
	m_Status = 0;
	m_Error = 0;

	MachineID mac;
	mac.GetMachineID(m_MachineId);
	m_MachineId[32] = 0;
	//printf("机器码:%s\n", m_MachineId);
}

// 设置是否免费
void Home::SetFree(bool v)
{
	m_bFree = v;
}

bool Home::IsValid()
{
#ifdef _DEBUG
	return true;
#endif
#if 1
	if (m_bFree)
		return true;
#endif
	if (!m_iVerifyTime || !m_iExpire || !m_iEndTime)
		return false;

	int now_time = time(NULL);
	if (now_time < m_iVerifyTime) { // 当前时间比验证时间还早 可能修改了系统时间
		return false;
	}
	//printf("%d --- %d(%d)\n", now_time, m_iEndTime, m_iEndTime - now_time);
	return now_time <= m_iEndTime;
}

bool Home::Recharge(const char* card)
{
	HttpClient http;
	http.m_GB2312 = false;
	http.AddParam("card", card);
	http.AddParam("game", HOME_GAME_FLAG);
	http.AddParam("machine_id", m_MachineId);
	HTTP_STATUS status = http.Request(HOME_HOST, L"/recharge", m_sResult, HTTP_POST);
	if (status != HTTP_STATUS_OK) {
		SetError(status, "充值失败！", status);
		return false;
	}

	m_pRepsone = (char*)m_sResult.c_str();
	Parse();

	//MessageBox(NULL, m_MsgStr, L"XXX", MB_OK);
	//printf("%d, %s 内容->%s\n", m_Error, m_MsgStr.c_str(), m_pRepsone);
	
	return m_Error == 0;
}

bool Home::Verify()
{
	HttpClient http;
	http.m_GB2312 = false;
	http.AddParam("game", HOME_GAME_FLAG);
	http.AddParam("machine_id", m_MachineId);
	HTTP_STATUS status = http.Request(HOME_HOST, L"/verify", m_sResult, HTTP_POST);
	if (status != HTTP_STATUS_OK) {
		SetError(status, "验证失败！", status);
		return false;
	}

	m_pRepsone = (char*)m_sResult.c_str();
	Parse();
	time_t a;

	//MessageBox(NULL, m_MsgStr, L"XXX", MB_OK);
	//printf("%d, %s 内容->%s\n", m_Error, m_MsgStr.c_str(), m_pRepsone);

	return m_Error == 0;
}

int Home::GetExpire()
{
	return m_iExpire = GetValue("expire:", 10);
}

void Home::Parse()
{
	m_MsgStr.empty();

	char statusKey[] = "status:", msgKey[] = "msg:";
	char* statusPtr = strstr(m_pRepsone, statusKey);
	char* msgPtr = strstr(m_pRepsone, msgKey);
	if (!statusPtr || !msgPtr) {
		SetError(1, L"充值失败！！！");
		return;
	}

	statusPtr += strlen(statusKey);
	msgPtr += strlen(msgKey);

	//printf("MsgPtr:%p --- %c\n", msgPtr, *statusPtr);

	SetErrorCode(*statusPtr == '0' ? 1 : 0);
	m_MsgStr = msgPtr;
	//printf("msgPtr:%s\n", msgPtr);
	GetExpire();
	SetVerifyTime();
	SetEndTime();
	SetExpireTime_S();

	//printf("有效时间（秒）:%d\n", m_iExpire);
}

bool Home::GetValue(char* key, char value[], int length)
{
	char* ptr = strstr(m_pRepsone, key);
	if (!ptr) {
		value[0] = 0;
		return false;
	}

	ptr += strlen(key);
	for (int i = 0; i < length; i++) {
		if (ptr[i] == ' ') {
			length = i;
			break;
		}
		value[i] = ptr[i];
	}
	value[length] = 0;

	return true;
}

int Home::GetValue(char* key, int length, int default_value)
{
	return GetValue((const char*)key, length, default_value);
}

int Home::GetValue(const char* key, int length, int default_value)
{
	char* ptr = strstr(m_pRepsone, key);
	if (!ptr) {
		return default_value;
	}
	//printf("%s\n", key);
	int value = 0;
	ptr += strlen(key);
	for (int i = 0; i < length; i++) {
		if (ptr[i] == '-') {
			return 0;
		}
		if (ptr[i] == ' ') {
			break;
		}
		if (ptr[i] >= '0' && ptr[i] <= '9') {
			//printf("%c\n", ptr[i]);
			value = value * 10 + (ptr[i] - '0');
		}
	}

	return value;
}

int Home::SetVerifyTime()
{
	return m_iVerifyTime = time(NULL);
}

int Home::SetEndTime()
{
	return m_iEndTime = m_iVerifyTime + m_iExpire;
}

void Home::SetExpireTime_S()
{
	time_t expire_time = GetValue("expire_time:", 10);
	if (!expire_time) {
		expire_time = m_iEndTime;
	}

	struct tm t;
	localtime_s(&t, &expire_time);
	char str[128];
	sprintf_s(str, "%d-%d-%d %d:%d", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min);
	m_sExpireTime = str;
	//printf("expire_time:%d %s\n", expire_time, str);
}

std::string& Home::GetExpireTime_S()
{
	return m_sExpireTime;
}

void Home::SetError(int code, const char * str)
{
	SetErrorCode(code);
	SetMsgStr(str);
}

void Home::SetError(int code, wchar_t* str)
{
	SetErrorCode(code);
	SetMsgStr(str);
}

void Home::SetError(int code, CString str)
{
	SetErrorCode(code);
	SetMsgStr(str);
}

void Home::SetError(int code, const char * str, DWORD status)
{
	SetErrorCode(code);
	SetMsgStr(str, status);
}

void Home::SetError(int code, wchar_t * str, DWORD status)
{
	SetErrorCode(code);
	SetMsgStr(str, status);
}

void Home::SetErrorCode(int code)
{
	m_Error = code;
}

void Home::SetMsgStr(wchar_t* str)
{
	//m_MsgStr = str;
}

void Home::SetMsgStr(CString str)
{
	//m_MsgStr = str;
}

void Home::SetMsgStr(wchar_t* str, DWORD status)
{
	//m_MsgStr.Format(L"%ws，错误码:(%d)", str, status);
}

void Home::SetMsgStr(const char * str, DWORD status)
{
	char msg[128];
	sprintf_s(msg, "%s，错误码:(%d)", str, status);
	m_MsgStr = msg;
}

std::string& Home::GetMsgStr()
{
	return m_MsgStr;
}
