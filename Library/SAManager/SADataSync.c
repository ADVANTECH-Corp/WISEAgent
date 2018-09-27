#include "SADataSync.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "util_path.h"
#include "util_libloader.h"

#ifdef WIN32
#define DEF_DATASYNC_LIB_NAME	"DataSync.dll"
#else
#define DEF_DATASYNC_LIB_NAME	"libDataSync.so"
#endif

bool SADataSync_Is_Exist(const char* path)
{
	char file[MAX_PATH] = {0};
	util_path_combine(file, path, DEF_DATASYNC_LIB_NAME);
	return util_dlexist(file);
}

bool SADataSync_Load(const char* path, SADataSync_Interface* pSADataSync)
{
	bool bRet = false;
	void * hDATASYNCDLL = NULL;
	char file[MAX_PATH] = {0};
	if(!pSADataSync)
		return bRet;
	util_path_combine(file, path, DEF_DATASYNC_LIB_NAME);
	if(util_dlopen(file, &hDATASYNCDLL))
	{
		memset(pSADataSync, 0, sizeof(SADataSync_Interface));
		pSADataSync->Handler = hDATASYNCDLL;
		
		if(pSADataSync != NULL && pSADataSync->Handler != NULL)
		{
			pSADataSync->DataSync_Initialize_API = (DATASYNC_INITIALIZE)util_dlsym(pSADataSync->Handler, "DataSync_Initialize");
			pSADataSync->DataSync_Uninitialize_API = (DATASYNC_UNINITIALIZE)util_dlsym(pSADataSync->Handler, "DataSync_Uninitialize");
			pSADataSync->DataSync_SetFuncCB_API = (DATASYNC_SETFUNCCB)util_dlsym(pSADataSync->Handler, "DataSync_SetFuncCB");
			pSADataSync->DataSync_Insert_Cap_API = (DATASYNC_INSERT_CAP)util_dlsym(pSADataSync->Handler, "DataSync_Insert_Cap");
			pSADataSync->DataSync_Insert_Rep_API = (DATASYNC_INSERT_REP)util_dlsym(pSADataSync->Handler, "DataSync_Insert_Rep");
			pSADataSync->DataSync_Set_LostTime_API = (DATASYNC_SET_LOSTTIME)util_dlsym(pSADataSync->Handler, "DataSync_Set_LostTime");
			pSADataSync->DataSync_Set_ReConTime_API = (DATASYNC_SET_RECONTIME)util_dlsym(pSADataSync->Handler, "DataSync_Set_ReConTime");
		}

		bRet = true;
	}
	return bRet;
}

SADataSync_Interface* SADataSync_Initialize(char const * pWorkdir,void* pLogHandle)
{
	SADataSync_Interface * pSADataSync = NULL;

	printf("SADataSync_Initialize()\n");

	if(!pWorkdir)
		return pSADataSync;

	if(SADataSync_Is_Exist(pWorkdir))
	{
		pSADataSync = malloc(sizeof(SADataSync_Interface));
		memset(pSADataSync, 0, sizeof(SADataSync_Interface));

		if(SADataSync_Load(pWorkdir, pSADataSync))
		{
			printf("SADataSync loaded\r\n");
			pSADataSync->LogHandle = pLogHandle;
		}
	}

	return pSADataSync;
}

bool SADataSync_Uninitialize(SADataSync_Interface * pSADataSync)
{
	bool bRet = true;
	if(pSADataSync != NULL)
	{
		if(pSADataSync->Handler)
			util_dlclose(pSADataSync->Handler);
		pSADataSync->Handler = NULL;
		free(pSADataSync);
		pSADataSync=NULL;
	}
	return bRet;
}