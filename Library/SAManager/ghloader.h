#ifndef _GH_LOADER_H_
#define _GH_LOADER_H_
#include "susiaccess_def.h"
#include "susiaccess_handler_mgmt.h"
#include <stdbool.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <Windows.h>
#ifndef GHLOADER_API
#define GHLOADER_API WINAPI
#endif
#else
#define GHLOADER_API
#endif

typedef int (GHLOADER_API *GENERAL_INITIALIZE)(HANDLER_INFO *pluginfo);
typedef void (GHLOADER_API *GENERAL_UNINITIALIZE)(void);
typedef void (GHLOADER_API *GENERAL_HANDLERECV)( char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2 );
typedef void (GHLOADER_API *GENERAL_SETSENDCB)(HandlerSendCbf  sendcbf);
typedef void (GHLOADER_API *GENERAL_SETPLUGINHANDLERS)(Handler_List_t *pLoaderList);
typedef void (GHLOADER_API *GENERAL_ONSTATUSCHANGE)(HANDLER_INFO *pluginfo);
typedef void (GHLOADER_API *GENERAL_START)(void);
typedef void (GHLOADER_API *GENERAL_STOP)(void);

typedef struct SAGENERAL_INTERFACE
{
	void*						Handler;               // handle of to load so library
	GENERAL_INITIALIZE			General_Initialize_API;
	GENERAL_UNINITIALIZE		General_Uninitialize_API;
	GENERAL_HANDLERECV			General_HandleRecv_API;
	GENERAL_SETSENDCB			General_SetSendCB_API;
	GENERAL_SETPLUGINHANDLERS	General_SetPluginHandlers_API;
	GENERAL_ONSTATUSCHANGE		General_OnStatusChanges_API;
	GENERAL_START				General_Start_API;
	GENERAL_STOP				General_Stop_API;

	void*						LogHandle;
}SAGeneral_Interface;
#ifdef __cplusplus
extern "C" {
#endif

bool ghloader_is_exist(char* path);
bool ghloader_load(char* path, SAGeneral_Interface * SAGeneral);
bool ghloader_release(SAGeneral_Interface * SAGeneral);
char* ghloader_get_error();

SAGeneral_Interface* ghloader_initialize(char const * pWorkdir, susiaccess_agent_conf_body_t const * pConfig, Handler_List_t *pLoaderList, Handler_Loader_Interface* pHandlerInfo, void* pLogHandle);
void ghloader_uninitialize(SAGeneral_Interface * pSAGeneral);

#ifdef __cplusplus
}
#endif

#endif
