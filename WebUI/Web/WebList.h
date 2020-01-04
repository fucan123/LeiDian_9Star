#pragma once

class JsCall;
class WebList
{
public:
	// ...
	void Init(JsCall* p, char id[]);
	// 添加一行表格
	void AddRow(int key=0, char* v=nullptr, int col_count=0);
	// 添加一行表格
	void AddRow(char* key, char* v = nullptr, int col_count = 0);
	// 更新文字
	void SetText(int row, int col, int num);
	// 更新文字
	void SetText(int row, int col, char text[]);
	// 设置class
	void SetClass(int row, char cla[], int add = 1);
	// 设置class
	void SetClass(int row, int col, char cla[], int add = 1);
	// 添加日记
	void AddLog(char text[], char* cla=nullptr);
	// 菜单禁用设置
	void SetMenuDisabled(int row, int v);
	// 自动填充表格行以达到美观
	void FillRow(int col_count=0);
public:
	// 元素ID
	char m_Id[32];
	// JsCall指针
	JsCall* m_pJsCall;
};