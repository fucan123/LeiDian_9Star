#pragma once
#include "../stdafx.h"
#include "JsCall.h"
#include <MsHTML.h>

class Game;
class JsCallCpp: public IDispatch
{
public:
	// 设置javascrpit
	void Init(JsCall* p, Game* pGame);
	// 调用 最后一个参数为功能字符, 根据它去判断不同功能
	void Call(DISPPARAMS *pDispParams);
	// 打开了菜单
	void OpenMenu(BSTR id, int row);
	// 选择菜单
	void SelectMenu(BSTR id, int data_row, int menu_row);
public:
	JsCall* m_pJsCall;
	Game* m_pGame;
public:
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo);
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
	virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
};