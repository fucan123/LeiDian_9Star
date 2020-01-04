#pragma once
#include "../stdafx.h"
#include <MsHTML.h>

class JsCall
{
public:
	JsCall() { }
	// JsCall
	JsCall(LPDISPATCH docuemnt);
	// 设置javascrpit
	void SetDocument(LPDISPATCH document);

	// 设置c++类对象
	void SetCppObj(void* obj);
	// 获得设置
	void SetSetting(char name[], int v);
	// 弹框
	void ShowMsg(char text[], char* title=nullptr, int icon=0);
	// 设置元素文字
	void SetText(char id[], int num);
	// 设置元素文字
	void SetText(char id[], char text[]);
	// 设置class
	void SetClass(char id[], char cla[], char rm_cla[]);
	// 菜单禁用设置
	void SetMenuDisabled(char id[], int row, int v);
	// 按钮禁用
	void SetBtnDisabled(char id[], int v, char* text=nullptr);
	// 菜单禁用设置
	void SetMenuDisabled(wchar_t id[], int row, int v);
	// 添加一行表格
	void AddTableRow(char id[], int col_count, int key = 0, char* v=nullptr);
	// 添加一行表格
	void AddTableRow(char id[], int col_count, char* key = nullptr, char* v = nullptr);
	// 自动填充表格行以达到美观
	void FillTableRow(char id[], int row_count, int col_count);
	// 添加日记
	void AddLog(char id[], char text[], char cla[]);
	// 更新状态文字
	void UpdateStatusText(char text[], int flag=0);
	// 更新表格单元文字
	void UpdateTableText(char id[], int row, int col, char text[]);
	// 更新表格class
	void UpdateTableClass(char id[], int row, int col, char cla[], int add);
	// 弹出卡号层
	void VerifyCard();
	// 更新完成
	void UpdateVerOk();

	// 可变参数Call
	void Call(int num, ...);
	// 可变参数Call
	void CallW(int num, ...);
public:
	CComDispatchDriver m_Script;
};