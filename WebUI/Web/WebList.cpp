#include "WebList.h"
#include "JsCall.h"
#include <string.h>

// ...
void WebList::Init(JsCall* p, char id[])
{
	m_pJsCall = p;
	strcpy(m_Id, id);
}

// ���һ�б��
void WebList::AddRow(int key, char* v, int col_count)
{
	m_pJsCall->AddTableRow(m_Id, col_count, key, v);
}

// ���һ�б��
void WebList::AddRow(char* key, char* v, int col_count)
{
	m_pJsCall->AddTableRow(m_Id, col_count, key, v);
}

// ��������
void WebList::SetText(int row, int col, int num)
{
	char text[128];
	sprintf_s(text, "%d", num);
	SetText(row, col, text);
}

// ��������
void WebList::SetText(int row, int col, char text[])
{
	m_pJsCall->UpdateTableText(m_Id, row, col, text);
}

// ����class
void WebList::SetClass(int row, char cla[], int add)
{
	m_pJsCall->UpdateTableClass(m_Id, row, -1, cla, add);
}

// ����class
void WebList::SetClass(int row, int col, char cla[], int add)
{
	m_pJsCall->UpdateTableClass(m_Id, row, col, cla, add);
}

// ����ռ�
void WebList::AddLog(char text[], char* cla)
{
	m_pJsCall->AddLog(m_Id, text, cla);
}

// �˵���������
void WebList::SetMenuDisabled(int row, int v)
{
	char id[32];
	sprintf_s(id, "%s_menu", m_Id);
	m_pJsCall->SetMenuDisabled(id, row, v);
}

// �Զ���������Դﵽ����
void WebList::FillRow(int col_count)
{
	m_pJsCall->FillTableRow(m_Id, 0, col_count);
}