#ifndef _HL_LOADER_H_
#define _HL_LOADER_H_
#include "susiaccess_def.h"
#include "susiaccess_handler_mgmt.h"
#include <stdbool.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once

#ifndef HLLOADER_API
#define HLLOADER_API WINAPI
#endif
#else
#define HLLOADER_API
#endif

/** Action(params) enum define for SA3.0*/
typedef enum cagent_action_request{
	cagent_start_file_download = 0,
	cagent_stop_file_download,
	cagent_connect_server,
	cagent_disconnect_server,
	cagent_connect_status,
	cagent_current_version,

	//susi agent add (10--100)
	cagent_request_device_monitoring = 10,
	cagent_request_power_onoff,
	cagent_request_remote_kvm,
	cagent_request_protection,
	cagent_request_recovery,
	cagent_request_software_monitoring,
	cagent_request_global,
	cagent_request_terminal,
	cagent_request_screenshot,
	
	//QA Test and Customized  30001-31000
	cagent_qa_test = 30001,
	cagent_custom_action,
	cagent_request_custom_max
}cagent_action_request_t;

typedef void (HLLOADER_API *LOADER_INITIALIZE) (char const * workdir, susiaccess_agent_conf_body_t const * conf, susiaccess_agent_profile_body_t const * profile, void* loghandler);
typedef void (HLLOADER_API *LOADER_UNINITIALIZE) (void);
typedef void (HLLOADER_API *LOADER_GETBASICHANDLERLLOADERIINTERFACE)(Handler_Loader_Interface * handler);
typedef void (HLLOADER_API *LOADER_SETAGENTSTATUS)(Handler_List_t *pLoaderList,susiaccess_agent_conf_body_t const * conf, susiaccess_agent_profile_body_t const * pProfile, int status);
typedef void (HLLOADER_API *LOADER_SETFUNCCB)(Callback_Functions_t* funcs);

typedef Handler_Loader_Interface * (HLLOADER_API *LOADER_GETLASTHANDLER)(Handler_List_t *pLoaderList);
typedef Handler_Loader_Interface * (HLLOADER_API *LOADER_FINDHANDLER)(Handler_List_t *pLoaderList, char const *name);
typedef Handler_Loader_Interface * (HLLOADER_API *LOADER_FINDHANDLERBYREQID)(Handler_List_t *pLoaderList, int reqID);

typedef bool (HLLOADER_API *LOADER_LOADVIRTUALHANDLER)(Handler_List_t *pLoaderList,char *VirName, char *HandlerName);
typedef int (HLLOADER_API *LOADER_LOADHANDLER)(Handler_List_t *pLoaderList, char const *handlerpath, char const *name);
typedef int (HLLOADER_API *LOADER_ADDHANDLER)(Handler_List_t *pLoaderList, Handler_Loader_Interface * pluginInfo);
typedef int (HLLOADER_API *LOADER_RELEASEHANDLER)(Handler_List_t *pLoaderList, Handler_Loader_Interface *pLoader);
typedef int (HLLOADER_API *LOADER_LOADALLHANDLER)(Handler_List_t *pLoaderList, char const * workdir);
typedef void (HLLOADER_API *LOADER_STARTALLHANDLER)(Handler_List_t *pLoaderList);
typedef void (HLLOADER_API *LOADER_STOPALLHANDLER)(Handler_List_t *pLoaderList);
typedef void (HLLOADER_API *LOADER_RELEASEALLHANDLER)(Handler_List_t *pLoaderList);
typedef void (HLLOADER_API *LOADER_CONCURRENTRELEASEALLHANDLER)(Handler_List_t *pLoaderList);

typedef struct SALOADER_INTERFACE
{
	void*									Handler;               // handle of to load so library
	LOADER_INITIALIZE						Loader_Initialize_API;
	LOADER_UNINITIALIZE						Loader_Uninitialize_API;
	LOADER_GETBASICHANDLERLLOADERIINTERFACE	Loader_GetBasicHandlerLoaderInterface_API;
	LOADER_SETAGENTSTATUS					Loader_SetAgentStatus_API;
	LOADER_SETFUNCCB						Loader_SetFuncCB_API;
	LOADER_GETLASTHANDLER					Loader_GetLastHandler_API;
	LOADER_FINDHANDLER						Loader_FindHandler_API;
	LOADER_FINDHANDLERBYREQID				Loader_FindHandlerByReqID_API;
	LOADER_LOADVIRTUALHANDLER				Loader_LoadVirtualHandler_API;
	LOADER_LOADHANDLER						Loader_LoadHandler_API;
	LOADER_ADDHANDLER						Loader_AddHandler_API;
	LOADER_RELEASEHANDLER					Loader_ReleaseHandler_API;
	LOADER_LOADALLHANDLER					Loader_LoadAllHandler_API;
	LOADER_STARTALLHANDLER					Loader_StartAllHandler_API;
	LOADER_STOPALLHANDLER					Loader_StopAllHandler_API;
	LOADER_RELEASEALLHANDLER				Loader_ReleaseAllHandler_API;
	LOADER_CONCURRENTRELEASEALLHANDLER		Loader_ConcurrentReleaseAllHandler_API;

	void*									LogHandle;
}SALoader_Interface;

#ifdef __cplusplus
extern "C" {
#endif

bool hlloader_is_exist(char* path);
bool hlloader_load(char* path, SALoader_Interface * SALoader);
bool hlloader_release(SALoader_Interface * SALoader);

char* hlloader_get_error();

SALoader_Interface* hlloader_initialize(char const * pWorkdir, susiaccess_agent_conf_body_t const * pConfig, susiaccess_agent_profile_body_t const * pProfile, void* pLogHandle);
void hlloader_uninitialize(SALoader_Interface* pSALoader);
void hlloader_handler_recv(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList, char* topic, susiaccess_packet_body_t *pkt, void *pRev1, void* pRev2);
void hlloader_handler_release(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList);
void hlloader_handler_agent_status_update(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList, susiaccess_agent_conf_body_t const * pConf, susiaccess_agent_profile_body_t const * pProfile, int iStatus);
void hlloader_handler_load(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList, char * cWorkdir);
void hlloader_handler_start(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList);
void hlloader_cbfunc_set(SALoader_Interface* pSALoader, Callback_Functions_t* pFunc);
void hlloader_basic_handlerinfo_get(SALoader_Interface* pSALoader, Handler_Loader_Interface* pHandlerInfo);
void hlloader_handlerinfo_add(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList, Handler_Loader_Interface* pHandlerInfo);
void hlloader_load_virtualhandler(SALoader_Interface* pSALoader, Handler_List_t *pHandlerList,char *VirName, char *HandlerName);

#ifdef __cplusplus
}
#endif

#endif
