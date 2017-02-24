#include "hlloader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "util_path.h"
#include "util_libloader.h"
#include "SAManagerLog.h"
#include "pktparser.h"

#ifdef WIN32
#define DEF_SALOADER_LIB_NAME	"SAHandlerLoader.dll"
#else
#define DEF_SALOADER_LIB_NAME	"libSAHandlerLoader.so"
#endif

void hlloader_function_load(SALoader_Interface * SALoader)
{
	if(SALoader != NULL && SALoader->Handler != NULL)
	{
		SALoader->Loader_Initialize_API = (LOADER_INITIALIZE)util_dlsym(SALoader->Handler, "Loader_Initialize");
		SALoader->Loader_Uninitialize_API = (LOADER_UNINITIALIZE)util_dlsym(SALoader->Handler, "Loader_Uninitialize");
		SALoader->Loader_GetBasicHandlerLoaderInterface_API = (LOADER_GETBASICHANDLERLLOADERIINTERFACE)util_dlsym(SALoader->Handler, "Loader_GetBasicHandlerLoaderInterface");
		SALoader->Loader_SetAgentStatus_API = (LOADER_SETAGENTSTATUS)util_dlsym(SALoader->Handler, "Loader_SetAgentStatus");
		SALoader->Loader_SetFuncCB_API = (LOADER_SETFUNCCB)util_dlsym(SALoader->Handler, "Loader_SetFuncCB");
		SALoader->Loader_GetLastHandler_API = (LOADER_GETLASTHANDLER)util_dlsym(SALoader->Handler, "Loader_GetLastHandler");
		SALoader->Loader_FindHandler_API = (LOADER_FINDHANDLER)util_dlsym(SALoader->Handler, "Loader_FindHandler");
		SALoader->Loader_FindHandlerByReqID_API = (LOADER_FINDHANDLERBYREQID)util_dlsym(SALoader->Handler, "Loader_FindHandlerByReqID");
		SALoader->Loader_LoadVirtualHandler_API = (LOADER_LOADVIRTUALHANDLER)util_dlsym(SALoader->Handler, "Loader_LoadVirtualHandler");
		SALoader->Loader_LoadHandler_API = (LOADER_LOADHANDLER)util_dlsym(SALoader->Handler, "Loader_LoadHandler");
		SALoader->Loader_AddHandler_API = (LOADER_ADDHANDLER)util_dlsym(SALoader->Handler, "Loader_AddHandler");
		SALoader->Loader_ReleaseHandler_API = (LOADER_RELEASEHANDLER)util_dlsym(SALoader->Handler, "Loader_ReleaseHandler");
		SALoader->Loader_LoadAllHandler_API = (LOADER_LOADALLHANDLER)util_dlsym(SALoader->Handler, "Loader_LoadAllHandler");
		SALoader->Loader_StartAllHandler_API = (LOADER_STARTALLHANDLER)util_dlsym(SALoader->Handler, "Loader_StartAllHandler");
		SALoader->Loader_StopAllHandler_API = (LOADER_STOPALLHANDLER)util_dlsym(SALoader->Handler, "Loader_StopAllHandler");
		SALoader->Loader_ReleaseAllHandler_API = (LOADER_RELEASEALLHANDLER)util_dlsym(SALoader->Handler, "Loader_ReleaseAllHandler");
		SALoader->Loader_ConcurrentReleaseAllHandler_API = (LOADER_RELEASEALLHANDLER)util_dlsym(SALoader->Handler, "Loader_ConcurrentReleaseAllHandler");
	}
}

bool hlloader_is_exist(char* path)
{
	char file[MAX_PATH] = {0};
	util_path_combine(file, path, DEF_SALOADER_LIB_NAME);
	return util_dlexist(file);
}

bool hlloader_load(char* path, SALoader_Interface * SALoader)
{
	bool bRet = false;
	void * hSALOADERDLL = NULL;
	char file[MAX_PATH] = {0};
	if(!SALoader)
		return bRet;
	util_path_combine(file, path, DEF_SALOADER_LIB_NAME);
	if(util_dlopen(file, &hSALOADERDLL))
	{
		memset(SALoader, 0, sizeof(SALoader_Interface));
		SALoader->Handler = hSALOADERDLL;
		hlloader_function_load(SALoader);
		bRet = true;
	}
	return bRet;
}

bool hlloader_release(SALoader_Interface * SALoader)
{
	bool bRet = true;
	if(SALoader != NULL)
	{
		if(SALoader->Handler)
			util_dlclose(SALoader->Handler);
		SALoader->Handler = NULL;
	}
	return bRet;
}

char* hlloader_get_error()
{
	char *error = util_dlerror();
	return error;
}

SALoader_Interface* hlloader_initialize(char const * pWorkdir, susiaccess_agent_conf_body_t const * pConfig, susiaccess_agent_profile_body_t const * pProfile, void* pLogHandle)
{
	SALoader_Interface* pSALoader = NULL;

	if(! pWorkdir)
		return pSALoader;

	if(! pConfig)
		return pSALoader;

	if(! pProfile)
		return pSALoader;

	if(hlloader_is_exist(pWorkdir))
	{
		pSALoader = malloc(sizeof(SALoader_Interface));
		memset(pSALoader, 0, sizeof(SALoader_Interface));
		if(hlloader_load(pWorkdir, pSALoader))
		{
			printf("HandlerLoader loaded\r\n");
			pSALoader->LogHandle = pLogHandle;
			if(pSALoader->Loader_Initialize_API)
				pSALoader->Loader_Initialize_API(pWorkdir, pConfig, pProfile, pLogHandle);
		}
		else
		{
			char *err = hlloader_get_error();
			SAManagerLog(pLogHandle, Warning, "Load HandlerLoader failed!\r\n  %s!!", err);
			free(err);
			free(pSALoader);
			pSALoader = NULL;
		}
	}
	else
	{
		char *err = hlloader_get_error();
		SAManagerLog(pLogHandle, Warning, "Cannot find HandlerLoader!\r\n  %s!!", err);
		free(err);
	}
	return pSALoader;
}

void hlloader_uninitialize(SALoader_Interface* pSALoader)
{
	if(pSALoader)
	{
		if(pSALoader->Loader_Uninitialize_API)
			pSALoader->Loader_Uninitialize_API();
		hlloader_release(pSALoader);
		free(pSALoader);
	}
}

void hlloader_handler_recv(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList, char* topic, susiaccess_packet_body_t *pkt, void *pRev1, void* pRev2)
{
	Handler_Loader_Interface* handler = NULL;
	char* pReqInfoPayload = NULL;
	
	if(!pSALoader)
		return;

	pReqInfoPayload = pkg_parser_packet_print(pkt);

	if(pSALoader->Loader_FindHandler_API)
	{
		/*Support V3.1 Version */
		handler = pSALoader->Loader_FindHandler_API(pHandlerList, pkt->handlerName);
	}
	if(handler != NULL)
	{
		if( handler->Handler_Recv_API )
			handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
	}

	/*Support V3.0 or Older Version */
	else{
		int reqID = pkt->requestID;
		if(pSALoader)
		{
			/*Support V3.0 Version */
			handler = pSALoader->Loader_FindHandlerByReqID_API(pHandlerList, reqID);
		}
		if(handler != NULL)
		{
			if( handler->Handler_Recv_API )
				handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
		}
		/*Support Older Version */
		else if(reqID == cagent_request_device_monitoring)
		{
			if(pSALoader)
			{
				handler = pSALoader->Loader_FindHandler_API(pHandlerList, "device_monitoring");
				if(handler != NULL)
				{
					if( handler->Handler_Recv_API )
						handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
				}
				else
				{
					SAManagerLog(pSALoader->LogHandle, Warning, "Cannot find handler: %s", "device_monitoring" );
				}
			}
		}
		else if(reqID == cagent_request_power_onoff)
		{
			if(pSALoader)
			{
				handler = pSALoader->Loader_FindHandler_API(pHandlerList, "power_onoff");
				if(handler != NULL)
				{
					if( handler->Handler_Recv_API )
						handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
				}
				else
				{
					SAManagerLog(pSALoader->LogHandle, Warning, "Cannot find handler: %s", "power_onoff" );
				}
			}
		}
		else if(reqID == cagent_request_remote_kvm)
		{
			if(pSALoader)
			{
				handler = pSALoader->Loader_FindHandler_API(pHandlerList, "remote_kvm");
				if(handler != NULL)
				{
					if( handler->Handler_Recv_API )
						handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
				}
				else
				{
					SAManagerLog(pSALoader->LogHandle, Warning, "Cannot find handler: %s", "remote_kvm" );
				}
			}
		}
		else if(reqID == cagent_request_protection)
		{
			if(pSALoader)
			{
				handler = pSALoader->Loader_FindHandler_API(pHandlerList, "protection");
				if(handler != NULL)
				{
					if( handler->Handler_Recv_API )
						handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
				}
				else
				{
					SAManagerLog(pSALoader->LogHandle, Warning, "Cannot find handler: %s", "protection" );
				}
			}
		}
		else if(reqID == cagent_request_recovery)
		{
			if(pSALoader)
			{
				handler = pSALoader->Loader_FindHandler_API(pHandlerList, "recovery");
				if(handler != NULL)
				{
					if( handler->Handler_Recv_API )
						handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
				}
				else
				{
					SAManagerLog(pSALoader->LogHandle, Warning, "Cannot find handler: %s", "recovery" );
				}
			}
		}
		else if(reqID == cagent_request_software_monitoring)
		{
			if(pSALoader)
			{
				handler = pSALoader->Loader_FindHandler_API(pHandlerList, "software_monitoring");
				if(handler != NULL)
				{
					if( handler->Handler_Recv_API )
						handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
				}
				else
				{
					SAManagerLog(pSALoader->LogHandle, Warning, "Cannot find handler: %s", "software_monitoring" );
				}
			}
		}
		else if(reqID == cagent_request_global)
		{
			if(pSALoader)
			{
				handler = pSALoader->Loader_FindHandler_API(pHandlerList, "general");
				if(handler != NULL)
				{
					if( handler->Handler_Recv_API )
						handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
				}
				else
				{
					SAManagerLog(pSALoader->LogHandle, Warning, "Cannot find handler: %s", "global" );
				}
			}
		}
		else if(reqID == cagent_request_terminal)
		{
			if(pSALoader)
			{
				handler = pSALoader->Loader_FindHandler_API(pHandlerList, "terminal");
				if(handler != NULL)
				{
					if( handler->Handler_Recv_API )
						handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
				}
				else
				{
					SAManagerLog(pSALoader->LogHandle, Warning, "Cannot find handler: %s", "terminal" );
				}
			}
		}
		else if(reqID == cagent_request_screenshot)
		{
			if(pSALoader)
			{
				handler = pSALoader->Loader_FindHandler_API(pHandlerList, "screenshot");
				if(handler != NULL)
				{
					if( handler->Handler_Recv_API )
						handler->Handler_Recv_API(topic, pReqInfoPayload, strlen(pReqInfoPayload), pRev1, pRev2);
				}
				else
				{
					SAManagerLog(pSALoader->LogHandle, Warning, "Cannot find handler: %s", "screenshot" );
				}
			}
		}
	}
	free(pReqInfoPayload);
}

void hlloader_handler_release(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList)
{
	if(!pSALoader)
		return;
	if(pSALoader->Loader_ConcurrentReleaseAllHandler_API)
			pSALoader->Loader_ConcurrentReleaseAllHandler_API(pHandlerList);
}

void hlloader_handler_agent_status_update(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList, susiaccess_agent_conf_body_t const * pConf, susiaccess_agent_profile_body_t const * pProfile, int iStatus)
{
	if(!pSALoader)
		return;
	if(pSALoader->Loader_SetAgentStatus_API)
		pSALoader->Loader_SetAgentStatus_API(pHandlerList, pConf, pProfile, iStatus);
}

void hlloader_handler_load(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList, char * cWorkdir)
{
	if(!pSALoader)
		return;
	if(pSALoader->Loader_LoadAllHandler_API)
		pSALoader->Loader_LoadAllHandler_API(pHandlerList, cWorkdir);
}

void hlloader_handler_start(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList)
{
	if(!pSALoader)
		return;
	if(pSALoader->Loader_StartAllHandler_API)
		pSALoader->Loader_StartAllHandler_API(pHandlerList);
}

void hlloader_cbfunc_set(SALoader_Interface* pSALoader, Callback_Functions_t* pFunc)
{
	if(!pSALoader)
		return;
	if(pSALoader->Loader_SetFuncCB_API)
		pSALoader->Loader_SetFuncCB_API(pFunc);
}

void hlloader_basic_handlerinfo_get(SALoader_Interface* pSALoader, Handler_Loader_Interface* pHandlerInfo)
{
	if(!pSALoader)
		return;
	if(pSALoader->Loader_GetBasicHandlerLoaderInterface_API)
		pSALoader->Loader_GetBasicHandlerLoaderInterface_API(pHandlerInfo);
}

void hlloader_handlerinfo_add(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList, Handler_Loader_Interface* pHandlerInfo)
{
	if(!pSALoader)
		return;
	if(pSALoader->Loader_AddHandler_API)
		pSALoader->Loader_AddHandler_API(pHandlerList, pHandlerInfo);
}
void hlloader_load_virtualhandler(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList,char *VirName, char *HandlerName)
{
	if(!pSALoader)
		return;
	if(pSALoader->Loader_LoadVirtualHandler_API)
		pSALoader->Loader_LoadVirtualHandler_API(pHandlerList, VirName, HandlerName);
}