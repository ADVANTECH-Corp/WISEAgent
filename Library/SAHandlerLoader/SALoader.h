#ifndef _PLUGIN_LOADER_H
#define _PLUGIN_LOADER_H

#include "susiaccess_handler_mgmt.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once

#ifndef HANDLERLOADER_API
#define HANDLERLOADER_API WINAPI
#endif
#else
#define HANDLERLOADER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HANDLERS 128

//#define IDP_MODULE   "./module/handler_sample.so" // patch

// Module Config XML Define
#ifdef WIN32
#define MODULE_CONFIG_FILE "\\module\\module_config.xml"
#else
#define MODULE_CONFIG_FILE "/module/module_config.xml"
#endif

#define MODULE_NUM "ModuleNum"
#define MODULE_NAME "ModuleName"
#define MODULE_PATH "ModulePath"

void HANDLERLOADER_API Loader_Initialize(char const * workdir, susiaccess_agent_conf_body_t const * conf, susiaccess_agent_profile_body_t const * profile, void* loghandler);
void HANDLERLOADER_API Loader_Uninitialize();
void HANDLERLOADER_API Loader_GetBasicHandlerLoaderInterface(Handler_Loader_Interface * handler);
void HANDLERLOADER_API Loader_SetAgentStatus(Handler_List_t *pLoaderList,susiaccess_agent_conf_body_t const * conf, susiaccess_agent_profile_body_t const * pProfile, int status);
void HANDLERLOADER_API Loader_SetFuncCB(Callback_Functions_t* funcs);

Handler_Loader_Interface * HANDLERLOADER_API Loader_GetLastHandler(Handler_List_t *pLoaderList);
Handler_Loader_Interface * HANDLERLOADER_API Loader_FindHandler(Handler_List_t *pLoaderList, char const *name);
Handler_Loader_Interface * HANDLERLOADER_API Loader_FindHandlerByReqID(Handler_List_t *pLoaderList, int reqID);

//bool HANDLERLOADER_API Loader_LoadVirtualHandler(Handler_List_t *pLoaderList, char *name);
int HANDLERLOADER_API Loader_LoadHandler(Handler_List_t *pLoaderList, char const *handlerpath, char const *name);
int HANDLERLOADER_API Loader_AddHandler(Handler_List_t *pLoaderList, Handler_Loader_Interface * pluginInfo);
int HANDLERLOADER_API Loader_ReleaseHandler(Handler_List_t *pLoaderList, Handler_Loader_Interface *pLoader);
int HANDLERLOADER_API Loader_LoadAllHandler(Handler_List_t *pLoaderList, char const * workdir);
void HANDLERLOADER_API Loader_StartAllHandler(Handler_List_t *pLoaderList);
void HANDLERLOADER_API Loader_StopAllHandler(Handler_List_t *pLoaderList);
void HANDLERLOADER_API Loader_ReleaseAllHandler(Handler_List_t *pLoaderList);
void HANDLERLOADER_API Loader_ConcurrentReleaseAllHandler(Handler_List_t *pLoaderList);

#ifdef __cplusplus
}
#endif

#endif // _PLUGIN_LOADER_H
