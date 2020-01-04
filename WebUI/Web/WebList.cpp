#include "WebList.h"
#include "JsCall.h"
#include <string.h>

// ...
void WebList::Init(JsCall* p, char id[])
{
	m_pJsCall = p;
	strcpy(m_Id, id);
}

// 添加一行表格
void WebList::AddRow(int key, char* v, int col_count)
{
	m_pJsCall->AddTableRow(m_Id, col_count, key, v);
}

// 添加一行表格
void WebList::AddRow(char* key, char* v, int col_count)
{
	m_pJsCall->AddTableRow(m_Id, col_count, key, v);
}

// 更新文字
void WebList::SetText(int row, int col, int num)
{
	char text[128];
	sprintf_s(text, "%d", num);
	SetText(row, col, text);
}

// 更新文字
void WebList::SetText(int row, int col, char text[])
{
	m_pJsCall->UpdateTableText(m_Id, row, col, text);
}

// 设置class
void WebList::SetClass(int row, char cla[], int add)
{
	m_pJsCall->UpdateTableClass(m_Id, row, -1, cla, add);
}

// 设置class
void WebList::SetClass(int row, int col, char cla[], int add)
{
	m_pJsCall->UpdateTableClass(m_Id, row, col, cla, add);
}

// 添加日记
void WebList::AddLog(char text[], char* cla)
{
	m_pJsCall->AddLog(m_Id, text, cla);
}

// 菜单禁用设置
void WebList::SetMenuDisabled(int row, int v)
{
	char id[32];
	sprintf_s(id, "%s_menu", m_Id);
	m_pJsCall->SetMenuDisabled(id, row, v);
}

// 自动填充表格行以达到美观
void WebList::FillRow(int col_count)
{
	m_pJsCall->FillTableRow(m_Id, 0, col_count);
}