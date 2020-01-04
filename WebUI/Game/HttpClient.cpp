#include "HttpClient.h"

HTTP_STATUS HttpClient::Request(const WCHAR* host, const WCHAR* path, std::string& result, int type)
{
	result = "";

	HTTP_STATUS status = 0;
	CInternetSession session(NULL);
	session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 15000);
	try {
		//printf("%s%s\n", HOST_NAME, path);
		//MessageBox(NULL, m_SendParam, L"发送的参数", MB_OK);
		CHttpConnection* pServer;
		if (host) {
			pServer = session.GetHttpConnection(host, (INTERNET_PORT)80);
		}
		else {
			pServer = session.GetHttpConnection(HOST_NAME, (INTERNET_PORT)80);
		}
		
		INTERNET_FLAG_SECURE;
		CHttpFile* pFile = pServer->OpenRequest(type, path);
		CString strHeaders = _T("Content-Type: application/x-www-form-urlencoded"); // 请求头
		
		if (!m_Cookie.IsEmpty()) {
			CString cookie;
			if (m_Cookie.Find(L"Cookie") == 0) {
				cookie += m_Cookie;
			}
			else {
				cookie = L"Cookie: ";
				cookie += m_Cookie;
			}
			
			pFile->AddRequestHeaders(cookie);
		}

		if (m_SendParam.IsEmpty()) {
			pFile->SendRequest(NULL, 0, 0, 0);
		}
		else {
			USES_CONVERSION;
			char* param = T2A(m_SendParam.GetBuffer(0));
			//printf("参数:%s\n", param);
			pFile->SendRequest(strHeaders, param, strlen(param));
		}
		
		//printf("param:%s\n", param);
		// 开始发送请求
		//printf("发送请求！\n");
		
		pFile->QueryInfoStatusCode(status);
		//printf("HTTP CODE:%d %s\n", status, result.c_str());
		if (status == HTTP_STATUS_OK) {
			CString newline;
			while (pFile->ReadString(newline)) { // 循环读取每行内容 
				std::string str;
				if (m_GB2312) {
					//printf("转成m_GB2312\n");
					str = UTF8ToGB((const char*)newline.GetBuffer());
				}
				else {
					str = (const char*)newline.GetBuffer();
					//printf("%s\n", newline.GetBuffer());
				}
				//printf("sss: %s\n", str.c_str());
				result += str;
			}
			//printf("内容:%s结束\n", result.c_str()); // 显示返回内容 
		}

		if (pFile)
			delete pFile;
		if (pServer)
			delete pServer;
	}
	catch (CInternetException* pEx) {
		TCHAR szErr[1024];
		CString strInfo;
		pEx->GetErrorMessage(szErr, 1024);
		MessageBox(NULL, szErr, L"oo", MB_OK);
		printf("错误:服务器未响应！！！\n"); // 显示异常信息 
		status = 0;
	}
	session.Close();
	return status;
}

void HttpClient::SetCookie(char * cookie)
{
	m_Cookie = cookie;
}

void HttpClient::SetCookie(wchar_t * cookie)
{
	m_Cookie = cookie;
}

void HttpClient::AddParam(const CHAR * content)
{
	m_SendParam = content;
}

void HttpClient::AddParam(CHAR * content)
{
	m_SendParam = content;
}

void HttpClient::AddParam(WCHAR * content)
{
	m_SendParam = content;
}

void HttpClient::AddParam(CString & content)
{
	m_SendParam = content;
}

void HttpClient::AddParam(const CHAR * key, const CHAR * value)
{
	m_SendParam += key;
	m_SendParam += L"=";
	m_SendParam += value;
	m_SendParam += L"&";
}

void HttpClient::AddParam(WCHAR* key, CHAR* value)
{
	m_SendParam += key;
	m_SendParam += L"=";
	m_SendParam += value;
	m_SendParam += L"&";
}

void HttpClient::AddParam(WCHAR* key, WCHAR* value)
{
	m_SendParam += key;
	m_SendParam += L"=";
	m_SendParam += value;
	m_SendParam += L"&";
}

void HttpClient::AddParam(WCHAR* key, CString& value)
{
	m_SendParam += key;
	m_SendParam += L"=";
	m_SendParam += value;
	m_SendParam += L"&";
}

void HttpClient::ResetParam()
{
	m_SendParam.Empty();
}

std::string HttpClient::UTF8ToGB(const char * str)
{
	std::string result;
	WCHAR *strSrc;
	CHAR *szRes;

	//获得临时变量的大小
	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	strSrc = new WCHAR[i + 1];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

	//获得临时变量的大小
	i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new CHAR[i + 1];
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

	result = szRes;
	delete[]strSrc;
	delete[]szRes;

	return result;
}
