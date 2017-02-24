#include "util_libloader.h"
#include <stdlib.h>
#include <string.h>
#include "util_path.h"

bool util_dlexist(char* path)
{
	bool bRet = false;
	void * hSAMANAGERDLL = NULL;
	hSAMANAGERDLL = dlopen(path, RTLD_LAZY);
	if(hSAMANAGERDLL != NULL)
	{
		bRet = true;
		dlclose(hSAMANAGERDLL);
		hSAMANAGERDLL = NULL;
	}
	return bRet;
}

bool util_dlopen(char* path, void ** lib)
{
	bool bRet = false;
	void * hSAMANAGERDLL = NULL;
	
	hSAMANAGERDLL = dlopen(path, RTLD_LAZY);
	if(hSAMANAGERDLL != NULL)
	{
		bRet = true;
		*lib = hSAMANAGERDLL;
	}
	
	return bRet;
}

bool util_dlclose(void * lib)
{
	bool bRet = true;
	if(lib)
		dlclose(lib);
	return bRet;
}

char* util_dlerror()
{
	return dlerror();
}

void* util_dlsym( void * handle, const char *name )
{
	return dlsym(handle, name); 
}