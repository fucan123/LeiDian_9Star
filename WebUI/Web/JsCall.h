#pragma once
#include "../stdafx.h"
#include <MsHTML.h>

class JsCall
{
public:
	JsCall() { }
	// JsCall
	JsCall(LPDISPATCH docuemnt);
	// ����javascrpit
	void SetDocument(LPDISPATCH document);

	// ����c++�����
	void SetCppObj(void* obj);
	// �������
	void SetSetting(char name[], int v);
	// ����
	void ShowMsg(char text[], char* title=nullptr, int icon=0);
	// ����Ԫ������
	void SetText(char id[], int num);
	// ����Ԫ������
	void SetText(char id[], char text[]);
	// ����class
	void SetClass(char id[], char cla[], char rm_cla[]);
	// �˵���������
	void SetMenuDisabled(char id[], int row, int v);
	// ��ť����
	void SetBtnDisabled(char id[], int v, char* text=nullptr);
	// �˵���������
	void SetMenuDisabled(wchar_t id[], int row, int v);
	// ���һ�б��
	void AddTableRow(char id[], int col_count, int key = 0, char* v=nullptr);
	// ���һ�б��
	void AddTableRow(char id[], int col_count, char* key = nullptr, char* v = nullptr);
	// �Զ���������Դﵽ����
	void FillTableRow(char id[], int row_count, int col_count);
	// ����ռ�
	void AddLog(char id[], char text[], char cla[]);
	// ����״̬����
	void UpdateStatusText(char text[], int flag=0);
	// ���±��Ԫ����
	void UpdateTableText(char id[], int row, int col, char text[]);
	// ���±��class
	void UpdateTableClass(char id[], int row, int col, char cla[], int add);
	// �������Ų�
	void VerifyCard();
	// �������
	void UpdateVerOk();

	// �ɱ����Call
	void Call(int num, ...);
	// �ɱ����Call
	void CallW(int num, ...);
public:
	CComDispatchDriver m_Script;
};