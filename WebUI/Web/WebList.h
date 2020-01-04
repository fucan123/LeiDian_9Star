#pragma once

class JsCall;
class WebList
{
public:
	// ...
	void Init(JsCall* p, char id[]);
	// ���һ�б��
	void AddRow(int key=0, char* v=nullptr, int col_count=0);
	// ���һ�б��
	void AddRow(char* key, char* v = nullptr, int col_count = 0);
	// ��������
	void SetText(int row, int col, int num);
	// ��������
	void SetText(int row, int col, char text[]);
	// ����class
	void SetClass(int row, char cla[], int add = 1);
	// ����class
	void SetClass(int row, int col, char cla[], int add = 1);
	// ����ռ�
	void AddLog(char text[], char* cla=nullptr);
	// �˵���������
	void SetMenuDisabled(int row, int v);
	// �Զ���������Դﵽ����
	void FillRow(int col_count=0);
public:
	// Ԫ��ID
	char m_Id[32];
	// JsCallָ��
	JsCall* m_pJsCall;
};