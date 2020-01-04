#pragma once
#include <stdio.h>
#include <Urlmon.h>
#pragma comment(lib, "urlmon.lib")

typedef void (*DownProgress)(ULONG now, ULONG max);

bool DownFile(const char* url, const char* file, DownProgress func);

class DownloadProgress : public IBindStatusCallback {
public:
	static DownProgress dpFunc;
public:
	HRESULT __stdcall QueryInterface(const IID &, void **) {
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef(void) {
		return 1;
	}
	ULONG STDMETHODCALLTYPE Release(void) {
		return 1;
	}
	HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD dwReserved, IBinding *pib) {
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE GetPriority(LONG *pnPriority) {
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE OnLowResource(DWORD reserved) {
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT hresult, LPCWSTR szError) {
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD *grfBINDF, BINDINFO *pbindinfo) {
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed) {
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID riid, IUnknown *punk) {
		return E_NOTIMPL;
	}

	virtual HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
	{
		if (dpFunc) {
			dpFunc(ulProgress, ulProgressMax);
		}
		return S_OK;
	}
};
