#include "dlfcn.h"
#include <stdio.h>

void *dlopen(const char *filename, int flags)
{
	void* lib = NULL;
	SetErrorMode(SEM_FAILCRITICALERRORS);
	lib = LoadLibrary(filename);
	SetErrorMode((UINT)NULL);
	return lib;
}

int dlclose(void *handle)
{
	BOOL bRet = FreeLibrary((HMODULE)handle);
	if(bRet == FALSE)
		return -1;
	else
		return 0;
}

void *dlsym(void *handle, const char *name)
{
	return (void*) GetProcAddress( (HMODULE)handle, name );
}

char *dlerror(void)
{
	LPSTR lpMsgBuf;
	DWORD dw = GetLastError(); 
	char* result = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) & lpMsgBuf,
		0, NULL );

	// Display the error message and exit the process
	result = (char*)malloc(strlen(lpMsgBuf)+1);
	memset(result, 0, strlen(lpMsgBuf)+1);
	sprintf(result,"%s",lpMsgBuf);

	LocalFree(lpMsgBuf);

	return result;
}