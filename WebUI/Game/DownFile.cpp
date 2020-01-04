#include "DownFile.h"

DownProgress DownloadProgress::dpFunc = nullptr;

bool DownFile(const char* url, const char * file, DownProgress func)
{
	DownloadProgress progress;
	progress.dpFunc = func;
	HRESULT hr = URLDownloadToFileA(NULL, url, file, NULL, static_cast<IBindStatusCallback*>(&progress));
	printf("%s %d\n", url , GetLastError());
	return SUCCEEDED(hr);
}
