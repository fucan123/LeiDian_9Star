#include "JsCallCpp.h"
#include "../Game/Game.h"

typedef void (JsCallCpp::*func)(DISPPARAMS *pDispParams);
#ifdef WIN32
typedef union func_addr{
	func f;
	unsigned int v;
} FuncAddr;
#else 
typedef union func_addr {
	func f;
	unsigned __int64 v;
} FuncAddr;
#endif

#define FUNC2NUM(addr, save_p) { \
    FuncAddr fa;\
	fa.f = addr;\
    *save_p = fa.v;\
}
#define NUM2FUNC(num, save_p) { \
    FuncAddr fa;\
	fa.v = num;\
    *save_p = fa.f;\
}

// 设置javascrpit
void JsCallCpp::Init(JsCall* p, Game* pGame)
{
	m_pJsCall = p;
	m_pGame = pGame;
}

// 调用 最后一个参数为功能字符, 根据它去判断不同功能
void JsCallCpp::Call(DISPPARAMS *pDispParams)
{
	int num = pDispParams->cArgs;
	int max = num - 1;
	VARIANTARG* args = pDispParams->rgvarg;
	BSTR id = args[max].bstrVal;
	printf("Call:%ws\n", id);
	if (wcscmp(id, L"start") == 0) {
		m_pGame->InstallDll();
		return;
	}
	if (wcscmp(id, L"auto_play") == 0) {
		m_pGame->AutoPlay(-1, wcscmp(args[0].bstrVal, L"停止登号") == 0);
		return;
	}
	if (wcscmp(id, L"talk") == 0) {
		m_pGame->CallTalk(args[1].bstrVal, args[0].intVal);
		return;
	}
	if (wcscmp(id, L"put_setting") == 0) {
		if (args[0].intVal)
			m_pGame->PutSetting(args[2].bstrVal, args[1].bstrVal);
		else
			m_pGame->PutSetting(args[2].bstrVal, args[1].intVal);
		return;
	}
	if (wcscmp(id, L"open_menu") == 0) {
		OpenMenu(args[1].bstrVal, args[0].intVal);
		return;
	}
	if (wcscmp(id, L"select_menu") == 0) {
		SelectMenu(args[2].bstrVal, args[1].intVal, args[0].intVal);
		return;
	}
	if (wcscmp(id, L"verify_card") == 0) {
		m_pGame->VerifyCard(args[0].bstrVal);
		return;
	}
	if (wcscmp(id, L"update_ver") == 0) {
		m_pGame->UpdateVer();
		return;
	}
}

// 点击了菜单
void JsCallCpp::OpenMenu(BSTR id, int row)
{
	//printf("OpenMenu Row:%ws=%d\n",  id, row);
	//m_pJsCall->SetMenuDisabled(pDispParams->rgvarg[1].bstrVal, 0, 1);
	if (wcscmp(id, L"table_1_menu") == 0) {
		if (m_pGame->IsLogin(m_pGame->GetAccount(row))) {
			m_pJsCall->SetMenuDisabled(id, 0, 1);
			m_pJsCall->SetMenuDisabled(id, 1, 0);
			m_pJsCall->SetMenuDisabled(id, 2, 0);
		}
		else {
			m_pJsCall->SetMenuDisabled(id, 0, 0);
			m_pJsCall->SetMenuDisabled(id, 1, 1);
			m_pJsCall->SetMenuDisabled(id, 2, 1);
		}
	}
}

// 选择菜单
void JsCallCpp::SelectMenu(BSTR id, int data_row, int menu_row)
{
	//printf("SelectMenu Row:%d-%d\n", data_row, menu_row);
	if (data_row < 0 || menu_row < 0)
		return;

	if (wcscmp(id, L"table_1_menu") == 0) {
		switch (menu_row)
		{
		case 0:
			m_pGame->OpenGame(data_row);
			break;
		case 1:
			m_pGame->CloseGame(data_row);
			break;
		case 2:
			m_pGame->SetInTeam(data_row);
			break;
		default:
			break;
		}	
	}
}

//不用实现，直接返回E_NOTIMPL
HRESULT STDMETHODCALLTYPE JsCallCpp::GetTypeInfoCount(UINT *pctinfo)
{
	return E_NOTIMPL;
}

//不用实现，直接返回E_NOTIMPL
HRESULT STDMETHODCALLTYPE JsCallCpp::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

//JavaScript调用这个对象的方法时，会把方法名，放到rgszNames中，我们须要给这种方法名拟定一个唯一的数字ID。用rgDispId传回给它
//同理JavaScript存取这个对象的属性时。会把属性名放到rgszNames中，我们须要给这个属性名拟定一个唯一的数字ID，用rgDispId传回给它
//紧接着JavaScript会调用Invoke。并把这个ID作为參数传递进来
HRESULT STDMETHODCALLTYPE JsCallCpp::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
	//rgszNames是个字符串数组。cNames指明这个数组中有几个字符串。假设不是1个字符串。忽略它
	if (cNames != 1)
		return E_NOTIMPL;

	//printf("函数:%ws\n", rgszNames[0]);
	//假设字符串是ShowMessageBox。说明JavaScript在调用我这个对象的ShowMessageBox方法。我就把我拟定的ID通过rgDispId告诉它
	/*if (wcscmp(rgszNames[0], L"Call") == 0) {
		FUNC2NUM(&JsCallCpp::Call, rgDispId);
		return S_OK;
	}*/
	*rgDispId = 1;
	return S_OK;
}

//JavaScript通过GetIDsOfNames拿到我的对象的方法的ID后。会调用Invoke。dispIdMember就是刚才我告诉它的我自己拟定的ID
//wFlags指明JavaScript对我的对象干了什么事情！

//假设是DISPATCH_METHOD，说明JavaScript在调用这个对象的方法。比方cpp_object.ShowMessageBox();
//假设是DISPATCH_PROPERTYGET。说明JavaScript在获取这个对象的属性，比方var n = cpp_object.num;
//假设是DISPATCH_PROPERTYPUT。说明JavaScript在改动这个对象的属性，比方cpp_object.num = 10;
//假设是DISPATCH_PROPERTYPUTREF，说明JavaScript在通过引用改动这个对象，详细我也不懂
//演示样例代码并没有涉及到wFlags和对象属性的使用。须要的请自行研究，使用方法是一样的
//pDispParams就是JavaScript调用我的对象的方法时传递进来的參数，里面有一个数组保存着全部參数
//pDispParams->cArgs就是数组中有多少个參数
//pDispParams->rgvarg就是保存着參数的数组，请使用[]下标来訪问。每一个參数都是VARIANT类型，能够保存各种类型的值
//详细是什么类型用VARIANT::vt来推断，不多解释了。VARIANT这东西大家都懂
//pVarResult就是我们给JavaScript的返回值
//其他不用管
HRESULT STDMETHODCALLTYPE JsCallCpp::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
	WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	//通过ID我就知道JavaScript想调用哪个方法
	//printf("dispIdMember:%d\n", dispIdMember);
	if (dispIdMember == 1) {
		this->Call(pDispParams);
		return S_OK;
	}

	// 参数从右到左算起 右为0开始
	if (dispIdMember == 1) {
		//检查是否仅仅有一个參数
		if (pDispParams->cArgs != 1)
			return E_NOTIMPL;
		//检查这个參数是否是字符串类型
		if (pDispParams->rgvarg[0].vt != VT_BSTR)
			return E_NOTIMPL;

		//放心调用
		return S_OK;
	}
		
	return E_NOTIMPL;
}

//JavaScript拿到我们传递给它的指针后，由于它不清楚我们的对象是什么东西，会调用QueryInterface来询问我们“你是什么鬼东西？”
//它会通过riid来问我们是什么东西。仅仅有它问到我们是不是IID_IDispatch或我们是不是IID_IUnknown时，我们才干肯定的回答它S_OK
//由于我们的对象继承于IDispatch。而IDispatch又继承于IUnknown，我们仅仅实现了这两个接口，所以仅仅能这样来回答它的询问
HRESULT STDMETHODCALLTYPE JsCallCpp::QueryInterface(REFIID riid, void **ppvObject)
{
	//printf("javascript询问对象是什么\n");
	if (riid == IID_IDispatch || riid == IID_IUnknown) {
		//对的，我是一个IDispatch，把我自己(this)交给你
		*ppvObject = static_cast<IDispatch*>(this);
		return S_OK;
	}

	return E_NOINTERFACE;
}

//我们知道COM对象使用引用计数来管理对象生命周期，我们的CJsCallCppDlg对象的生命周期就是整个程序的生命周期
//我的这个对象不须要你JavaScript来管，我自己会管。所以我不用实现AddRef()和Release()。这里乱写一些。


//你要return 1;return 2;return 3;return 4;return 5;都能够
ULONG STDMETHODCALLTYPE JsCallCpp::AddRef()
{
	return 3;
}

//同上。不多说了
//题外话：当然假设你要new出一个c++对象来并扔给JavaScript来管，你就须要实现AddRef()和Release()，在引用计数归零时delete this;
ULONG STDMETHODCALLTYPE JsCallCpp::Release()
{
	return 3;
}