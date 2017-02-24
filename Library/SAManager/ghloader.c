#include "ghloader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "util_path.h"
#include "util_libloader.h"
#include "SAManagerLog.h"

#ifdef WIN32
#define DEF_SAGENERAL_LIB_NAME	"SAGeneralHandler.dll"
#else
#define DEF_SAGENERAL_LIB_NAME	"libSAGeneralHandler.so"
#endif

void ghloader_function_load(SAGeneral_Interface * SAGeneral)
{
	if(SAGeneral != NULL && SAGeneral->Handler != NULL)
	{
		SAGeneral->General_Initialize_API = (GENERAL_INITIALIZE)util_dlsym(SAGeneral->Handler, "General_Initialize");
		SAGeneral->General_Uninitialize_API = (GENERAL_UNINITIALIZE)util_dlsym(SAGeneral->Handler, "General_Uninitialize");
		SAGeneral->General_HandleRecv_API = (GENERAL_HANDLERECV)util_dlsym(SAGeneral->Handler, "General_HandleRecv");
		SAGeneral->General_SetSendCB_API = (GENERAL_SETSENDCB)util_dlsym(SAGeneral->Handler, "General_SetSendCB");
		SAGeneral->General_SetPluginHandlers_API = (GENERAL_SETPLUGINHANDLERS)util_dlsym(SAGeneral->Handler, "General_SetPluginHandlers");
		SAGeneral->General_OnStatusChanges_API = (GENERAL_ONSTATUSCHANGE)util_dlsym(SAGeneral->Handler, "General_OnStatusChange");
		SAGeneral->General_Start_API = (GENERAL_START)util_dlsym(SAGeneral->Handler, "General_Start");
		SAGeneral->General_Stop_API = (GENERAL_STOP)util_dlsym(SAGeneral->Handler, "General_Stop");
	}
}

bool ghloader_is_exist(char* path)
{
	char file[MAX_PATH] = {0};
	util_path_combine(file, path, DEF_SAGENERAL_LIB_NAME);
	return util_dlexist(file);
}

bool ghloader_load(char* path, SAGeneral_Interface * SAGeneral)
{
	bool bRet = false;
	void * hSAGENRERALDLL = NULL;
	char file[MAX_PATH] = {0};
	if(!SAGeneral)
		return bRet;
	util_path_combine(file, path, DEF_SAGENERAL_LIB_NAME);
	if(util_dlopen(file, &hSAGENRERALDLL))
	{
		memset(SAGeneral, 0, sizeof(SAGeneral_Interface));
		SAGeneral->Handler = hSAGENRERALDLL;
		ghloader_function_load(SAGeneral);
		bRet = true;
	}
	return bRet;
}

bool ghloader_release(SAGeneral_Interface * SAGeneral)
{
	bool bRet = true;
	if(SAGeneral != NULL)
	{
		if(SAGeneral->Handler)
			util_dlclose(SAGeneral->Handler);
		SAGeneral->Handler = NULL;
	}
	return bRet;
}

char* ghloader_get_error()
{
	char *error = util_dlerror();
	return error;
}

SAGeneral_Interface* ghloader_initialize(char const * pWorkdir, susiaccess_agent_conf_body_t const * pConfig, Handler_List_t *pHandlerList, Handler_Loader_Interface* pHandlerInfo, void* pLogHandle)
{
	SAGeneral_Interface * pSAGeneral = NULL;

	if(!pHandlerInfo)
		return pSAGeneral;

	if(!pWorkdir)
		return pSAGeneral;

	if(!pConfig)
		return pSAGeneral;

	if(ghloader_is_exist(pWorkdir))
	{
		pSAGeneral = malloc(sizeof(SAGeneral_Interface));
		memset(pSAGeneral, 0, sizeof(SAGeneral_Interface));
		if(ghloader_load(pWorkdir, pSAGeneral))
		{
			printf("GeneralHandler loaded\r\n");
			pSAGeneral->LogHandle = pLogHandle;
			strcpy(pHandlerInfo->Name, "general");
			if(pHandlerInfo->pHandlerInfo)
			{
				strncpy(pHandlerInfo->pHandlerInfo->ServerIP,  pConfig->serverIP, strlen(pConfig->serverIP)+1);
				pHandlerInfo->pHandlerInfo->ServerPort = atol(pConfig->serverPort);
			}
			pHandlerInfo->type = core_handler;

			if(pSAGeneral->General_Initialize_API)
			{
				pHandlerInfo->Handler_Init_API = (HANDLER_INITLIZE)pSAGeneral->General_Initialize_API;
				pSAGeneral->General_Initialize_API(pHandlerInfo->pHandlerInfo);
			}

			if(pSAGeneral->General_HandleRecv_API)
				pHandlerInfo->Handler_Recv_API = (HANDLER_RECV)pSAGeneral->General_HandleRecv_API;

			if(pSAGeneral->General_Start_API)
				pHandlerInfo->Handler_Start_API = (HANDLER_START)pSAGeneral->General_Start_API;

			if(pSAGeneral->General_Stop_API)
				pHandlerInfo->Handler_Stop_API = (HANDLER_STOP)pSAGeneral->General_Stop_API;

			if(pSAGeneral->General_OnStatusChanges_API)
				pHandlerInfo->Handler_OnStatusChange_API = (HANDLER_ONSTATUSCAHNGE)pSAGeneral->General_OnStatusChanges_API;

			if(pSAGeneral->General_SetPluginHandlers_API)
				pSAGeneral->General_SetPluginHandlers_API(pHandlerList);

			pHandlerInfo->Workable = true;
		}
		else
		{
			char *err = ghloader_get_error();
			SAManagerLog(pLogHandle, Warning, "Load GeneralHandler failed!\r\n  %s!!", err);
			free(err);
			free(pSAGeneral);
			pSAGeneral = NULL;
		}
	}
	else
	{
		char *err = ghloader_get_error();
		SAManagerLog(pLogHandle, Warning, "Cannot find GeneralHandler!\r\n  %s!!", err);
		free(err);
	}

	return pSAGeneral;
}

void ghloader_uninitialize(SAGeneral_Interface * pSAGeneral)
{
	if(pSAGeneral)
	{
		if(pSAGeneral->General_Uninitialize_API)
			pSAGeneral->General_Uninitialize_API();
		ghloader_release(pSAGeneral);
		free(pSAGeneral);
	}
}