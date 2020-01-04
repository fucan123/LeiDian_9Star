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

// ����javascrpit
void JsCallCpp::Init(JsCall* p, Game* pGame)
{
	m_pJsCall = p;
	m_pGame = pGame;
}

// ���� ���һ������Ϊ�����ַ�, ������ȥ�жϲ�ͬ����
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
		m_pGame->AutoPlay(-1, wcscmp(args[0].bstrVal, L"ֹͣ�Ǻ�") == 0);
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

// ����˲˵�
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

// ѡ��˵�
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

//����ʵ�֣�ֱ�ӷ���E_NOTIMPL
HRESULT STDMETHODCALLTYPE JsCallCpp::GetTypeInfoCount(UINT *pctinfo)
{
	return E_NOTIMPL;
}

//����ʵ�֣�ֱ�ӷ���E_NOTIMPL
HRESULT STDMETHODCALLTYPE JsCallCpp::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

//JavaScript�����������ķ���ʱ����ѷ��������ŵ�rgszNames�У�������Ҫ�����ַ������ⶨһ��Ψһ������ID����rgDispId���ظ���
//ͬ��JavaScript��ȡ������������ʱ������������ŵ�rgszNames�У�������Ҫ������������ⶨһ��Ψһ������ID����rgDispId���ظ���
//������JavaScript�����Invoke���������ID��Ϊ�������ݽ���
HRESULT STDMETHODCALLTYPE JsCallCpp::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
	//rgszNames�Ǹ��ַ������顣cNamesָ������������м����ַ��������費��1���ַ�����������
	if (cNames != 1)
		return E_NOTIMPL;

	//printf("����:%ws\n", rgszNames[0]);
	//�����ַ�����ShowMessageBox��˵��JavaScript�ڵ�������������ShowMessageBox�������ҾͰ����ⶨ��IDͨ��rgDispId������
	/*if (wcscmp(rgszNames[0], L"Call") == 0) {
		FUNC2NUM(&JsCallCpp::Call, rgDispId);
		return S_OK;
	}*/
	*rgDispId = 1;
	return S_OK;
}

//JavaScriptͨ��GetIDsOfNames�õ��ҵĶ���ķ�����ID�󡣻����Invoke��dispIdMember���Ǹղ��Ҹ����������Լ��ⶨ��ID
//wFlagsָ��JavaScript���ҵĶ������ʲô���飡

//������DISPATCH_METHOD��˵��JavaScript�ڵ����������ķ������ȷ�cpp_object.ShowMessageBox();
//������DISPATCH_PROPERTYGET��˵��JavaScript�ڻ�ȡ�����������ԣ��ȷ�var n = cpp_object.num;
//������DISPATCH_PROPERTYPUT��˵��JavaScript�ڸĶ������������ԣ��ȷ�cpp_object.num = 10;
//������DISPATCH_PROPERTYPUTREF��˵��JavaScript��ͨ�����øĶ����������ϸ��Ҳ����
//��ʾ�������벢û���漰��wFlags�Ͷ������Ե�ʹ�á���Ҫ���������о���ʹ�÷�����һ����
//pDispParams����JavaScript�����ҵĶ���ķ���ʱ���ݽ����ą�����������һ�����鱣����ȫ������
//pDispParams->cArgs�����������ж��ٸ�����
//pDispParams->rgvarg���Ǳ����Ņ��������飬��ʹ��[]�±����L�ʡ�ÿһ����������VARIANT���ͣ��ܹ�����������͵�ֵ
//��ϸ��ʲô������VARIANT::vt���ƶϣ���������ˡ�VARIANT�ⶫ����Ҷ���
//pVarResult�������Ǹ�JavaScript�ķ���ֵ
//�������ù�
HRESULT STDMETHODCALLTYPE JsCallCpp::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
	WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	//ͨ��ID�Ҿ�֪��JavaScript������ĸ�����
	//printf("dispIdMember:%d\n", dispIdMember);
	if (dispIdMember == 1) {
		this->Call(pDispParams);
		return S_OK;
	}

	// �������ҵ������� ��Ϊ0��ʼ
	if (dispIdMember == 1) {
		//����Ƿ������һ������
		if (pDispParams->cArgs != 1)
			return E_NOTIMPL;
		//�����������Ƿ����ַ�������
		if (pDispParams->rgvarg[0].vt != VT_BSTR)
			return E_NOTIMPL;

		//���ĵ���
		return S_OK;
	}
		
	return E_NOTIMPL;
}

//JavaScript�õ����Ǵ��ݸ�����ָ�����������������ǵĶ�����ʲô�����������QueryInterface��ѯ�����ǡ�����ʲô��������
//����ͨ��riid����������ʲô���������������ʵ������ǲ���IID_IDispatch�������ǲ���IID_IUnknownʱ�����ǲŸɿ϶��Ļش���S_OK
//�������ǵĶ���̳���IDispatch����IDispatch�ּ̳���IUnknown�����ǽ���ʵ�����������ӿڣ����Խ������������ش�����ѯ��
HRESULT STDMETHODCALLTYPE JsCallCpp::QueryInterface(REFIID riid, void **ppvObject)
{
	//printf("javascriptѯ�ʶ�����ʲô\n");
	if (riid == IID_IDispatch || riid == IID_IUnknown) {
		//�Եģ�����һ��IDispatch�������Լ�(this)������
		*ppvObject = static_cast<IDispatch*>(this);
		return S_OK;
	}

	return E_NOINTERFACE;
}

//����֪��COM����ʹ�����ü�������������������ڣ����ǵ�CJsCallCppDlg������������ھ��������������������
//�ҵ����������Ҫ��JavaScript���ܣ����Լ���ܡ������Ҳ���ʵ��AddRef()��Release()��������дһЩ��


//��Ҫreturn 1;return 2;return 3;return 4;return 5;���ܹ�
ULONG STDMETHODCALLTYPE JsCallCpp::AddRef()
{
	return 3;
}

//ͬ�ϡ�����˵��
//���⻰����Ȼ������Ҫnew��һ��c++���������Ӹ�JavaScript���ܣ������Ҫʵ��AddRef()��Release()�������ü�������ʱdelete this;
ULONG STDMETHODCALLTYPE JsCallCpp::Release()
{
	return 3;
}