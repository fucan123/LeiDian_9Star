#include "JsCall.h"

// JsCall
JsCall::JsCall(LPDISPATCH document)
{
	SetDocument(document);
}

// 设置javascrpit
void JsCall::SetDocument(LPDISPATCH document)
{
	CComQIPtr<IHTMLDocument2> spDoc = document;
	spDoc->get_Script(&m_Script);
}

// 设置c++类对象
void JsCall::SetCppObj(void* obj)
{
	CComVariant var = (IDispatch*)obj;
	m_Script.Invoke1(L"SetCppObj", &var, nullptr);
}

// 获得设置
void JsCall::SetSetting(char name[], int v)
{
	Call(3, L"SetSetting", name, v);
}

// 弹框
void JsCall::ShowMsg(char text[], char* title, int icon)
{
	Call(4, L"ShowMsg", text, title, icon);
}

// 设置元素文字
void JsCall::SetText(char id[], int num)
{
	char text[128];
	sprintf(text, "%d", num);
	SetText(id, text);
}

// 设置元素文字
void JsCall::SetText(char id[], char text[])
{
	Call(3, L"SetText", id, text);
}

// 设置class
void JsCall::SetClass(char id[], char cla[], char rm_cla[])
{
	Call(4, L"SetClass", id, cla, rm_cla);
}

// 菜单禁用设置
void JsCall::SetMenuDisabled(char id[], int row, int v)
{
	Call(4, L"SetMenuDisabled", id, row, v);
}

// 按钮禁用
void JsCall::SetBtnDisabled(char id[], int v, char* text)
{
	Call(4, L"SetBtnDisabled", id, v, text);
}

// 菜单禁用设置
void JsCall::SetMenuDisabled(wchar_t id[], int row, int v)
{
	CallW(4, L"SetMenuDisabled", id, row, v);
}

// 添加一行表格
void JsCall::AddTableRow(char id[], int col_count, int key, char* v)
{
	Call(5, L"AddTableRow", id, col_count, key, v);
}

// 添加一行表格
void JsCall::AddTableRow(char id[], int col_count, char* key, char* v)
{
	Call(5, L"AddTableRow", id, col_count, key, v);
}

// 自动填充表格行以达到美观
void JsCall::FillTableRow(char id[], int row_count, int col_count)
{
	Call(4, L"FillTableRow", id, row_count, col_count);
}

// 添加日记
void JsCall::AddLog(char id[], char text[], char cla[])
{
	Call(4, L"AddLog", id, text, cla);
}

// 更新状态文字
void JsCall::UpdateStatusText(char text[], int flag)
{
	Call(3, L"UpdateStatusText", text, flag);
}

// 更新表格单元文字
void JsCall::UpdateTableText(char id[], int row, int col, char text[])
{
	/*
	CComVariant params[] = { text, col, row, id };
	m_Script.InvokeN(L"UpdateTableText", params, sizeof(params)/sizeof(CComVariant), nullptr);
	*/
	Call(5, L"UpdateTableText", id, row, col, text);
}

// 更新表格class
void JsCall::UpdateTableClass(char id[], int row, int col, char cla[], int add)
{
	Call(6, L"UpdateTableClass", id, row, col, cla, add);
}

// 弹出卡号层
void JsCall::VerifyCard()
{
	m_Script.Invoke0(L"VerifyCard");
}

// 更新完成
void JsCall::UpdateVerOk()
{
	m_Script.Invoke0(L"UpdateVerOk");
}

// 可变参数Call
void JsCall::Call(int num, ...)
{
	va_list   arg_ptr;   //定义可变参数指针
	va_start(arg_ptr, num);   // i为最后一个固定参数
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
	va_end(arg_ptr);        //  清空参数指针

	HRESULT r = m_Script.InvokeN(function, params, num - 1, nullptr);
}


// 可变参数Call
void JsCall::CallW(int num, ...)
{
	va_list   arg_ptr;   //定义可变参数指针
	va_start(arg_ptr, num);   // i为最后一个固定参数
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
	va_end(arg_ptr);        //  清空参数指针

	m_Script.InvokeN(function, params, num - 1, nullptr);
}
