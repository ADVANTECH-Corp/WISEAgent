#include "wrapper.h"
#include "dlfcn.h"
#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <direct.h>

void* ADVPLAT_CALL dlopen(const char *filename, int flag) {
#ifdef UNICODE
	WCHAR wsz[256];
	mbstowcs(wsz, filename, sizeof(wsz));
	LPCWSTR p = wsz;
#else
	CHAR wsz[256];
	snprintf(wsz, sizeof(wsz), "%s", filename);
	LPCSTR p = wsz;
#endif
	return (void *)LoadLibrary(p);
}

void* ADVPLAT_CALL dlsym(void *handle, const char *symbol) {
	HINSTANCE instance = (HINSTANCE)handle;
	char name[256];
	sprintf(name, "%s", symbol);
	return GetProcAddress(instance, name);
}



char* ADVPLAT_CALL dlerror(void) {
	return NULL;
}


int ADVPLAT_CALL dlclose(void *handle) {
	return 0;
}