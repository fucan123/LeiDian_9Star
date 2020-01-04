#pragma once

#include <afxinet.h>
#include <string>

#define HTTP_POST 0
#define HTTP_GET  1

#define HOST_NAME  L"www.myhostcpp.com"
#define HOST_PORT  80
#define HTTPS_PORT 443

typedef DWORD HTTP_STATUS;

class HttpClient
{
public:
	HttpClient() { }
	HTTP_STATUS Request(const WCHAR* host, const WCHAR* path, std::string& result, int type= HTTP_GET);
	void SetCookie(char* cookie);
	void SetCookie(wchar_t* cookie);
	void AddParam(const CHAR* content);
	void AddParam(CHAR* content);
	void AddParam(WCHAR* content);
	void AddParam(CString& content);
	void AddParam(const CHAR* key, const CHAR* value);
	void AddParam(WCHAR* key, CHAR* value);
	void AddParam(WCHAR* key, WCHAR* value);
	void AddParam(WCHAR* key, CString& value);
	void ResetParam();
	std::string UTF8ToGB(const char* str);
public:
	// ÊÇ·ñ×ª³ÉGB2312
	bool m_GB2312 = false;

	CString m_Cookie;
	CString m_SendParam;
	CInternetSession* m_pSession;
	CHttpFile* m_pHttpFile;
};