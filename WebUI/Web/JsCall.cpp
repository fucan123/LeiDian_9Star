#include "JsCall.h"

// JsCall
JsCall::JsCall(LPDISPATCH document)
{
	SetDocument(document);
}

// ����javascrpit
void JsCall::SetDocument(LPDISPATCH document)
{
	CComQIPtr<IHTMLDocument2> spDoc = document;
	spDoc->get_Script(&m_Script);
}

// ����c++�����
void JsCall::SetCppObj(void* obj)
{
	CComVariant var = (IDispatch*)obj;
	m_Script.Invoke1(L"SetCppObj", &var, nullptr);
}

// �������
void JsCall::SetSetting(char name[], int v)
{
	Call(3, L"SetSetting", name, v);
}

// ����
void JsCall::ShowMsg(char text[], char* title, int icon)
{
	Call(4, L"ShowMsg", text, title, icon);
}

// ����Ԫ������
void JsCall::SetText(char id[], int num)
{
	char text[128];
	sprintf(text, "%d", num);
	SetText(id, text);
}

// ����Ԫ������
void JsCall::SetText(char id[], char text[])
{
	Call(3, L"SetText", id, text);
}

// ����class
void JsCall::SetClass(char id[], char cla[], char rm_cla[])
{
	Call(4, L"SetClass", id, cla, rm_cla);
}

// �˵���������
void JsCall::SetMenuDisabled(char id[], int row, int v)
{
	Call(4, L"SetMenuDisabled", id, row, v);
}

// ��ť����
void JsCall::SetBtnDisabled(char id[], int v, char* text)
{
	Call(4, L"SetBtnDisabled", id, v, text);
}

// �˵���������
void JsCall::SetMenuDisabled(wchar_t id[], int row, int v)
{
	CallW(4, L"SetMenuDisabled", id, row, v);
}

// ���һ�б��
void JsCall::AddTableRow(char id[], int col_count, int key, char* v)
{
	Call(5, L"AddTableRow", id, col_count, key, v);
}

// ���һ�б��
void JsCall::AddTableRow(char id[], int col_count, char* key, char* v)
{
	Call(5, L"AddTableRow", id, col_count, key, v);
}

// �Զ���������Դﵽ����
void JsCall::FillTableRow(char id[], int row_count, int col_count)
{
	Call(4, L"FillTableRow", id, row_count, col_count);
}

// ����ռ�
void JsCall::AddLog(char id[], char text[], char cla[])
{
	Call(4, L"AddLog", id, text, cla);
}

// ����״̬����
void JsCall::UpdateStatusText(char text[], int flag)
{
	Call(3, L"UpdateStatusText", text, flag);
}

// ���±��Ԫ����
void JsCall::UpdateTableText(char id[], int row, int col, char text[])
{
	/*
	CComVariant params[] = { text, col, row, id };
	m_Script.InvokeN(L"UpdateTableText", params, sizeof(params)/sizeof(CComVariant), nullptr);
	*/
	Call(5, L"UpdateTableText", id, row, col, text);
}

// ���±��class
void JsCall::UpdateTableClass(char id[], int row, int col, char cla[], int add)
{
	Call(6, L"UpdateTableClass", id, row, col, cla, add);
}

// �������Ų�
void JsCall::VerifyCard()
{
	m_Script.Invoke0(L"VerifyCard");
}

// �������
void JsCall::UpdateVerOk()
{
	m_Script.Invoke0(L"UpdateVerOk");
}

// �ɱ����Call
void JsCall::Call(int num, ...)
{
	va_list   arg_ptr;   //����ɱ����ָ��
	va_start(arg_ptr, num);   // iΪ���һ���̶�����
	wchar_t* function = va_arg(arg_ptr, wchar_t*);

	if (num == 1) {
		m_Script.Invoke0(function);
		return;
	}

	CComVariant* params = new CComVariant[num - 1];
	for (int i = num - 2; i >= 0; i--) {
		int v = va_arg(arg_ptr, int);
		if (v < 0x0000ffff) {
			params[i] = v;
		}
		else {
			params[i] = (char*)v;
		}
	}
	va_end(arg_ptr);        //  ��ղ���ָ��

	HRESULT r = m_Script.InvokeN(function, params, num - 1, nullptr);
}


// �ɱ����Call
void JsCall::CallW(int num, ...)
{
	va_list   arg_ptr;   //����ɱ����ָ��
	va_start(arg_ptr, num);   // iΪ���һ���̶�����
	wchar_t* function = va_arg(arg_ptr, wchar_t*);

	if (num == 1) {
		m_Script.Invoke0(function);
		return;
	}

	CComVariant* params = new CComVariant[num - 1];
	for (int i = num - 2; i >= 0; i--) {
		int v = va_arg(arg_ptr, int);
		if (v < 0x0000ffff) {
			params[i] = v;
		}
		else {
			params[i] = (wchar_t*)v;
		}
	}
	va_end(arg_ptr);        //  ��ղ���ָ��

	m_Script.InvokeN(function, params, num - 1, nullptr);
}
