#include "smloader.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "util_path.h"
#include "util_libloader.h"
#include "SAClientLog.h"
#include "eventqueue.h"

#ifdef WIN32
#define DEF_SAMANAGER_LIB_NAME	"SAManager.dll"
#else
#define DEF_SAMANAGER_LIB_NAME	"libSAManager.so"
#endif

SAManager_Interface* g_SAManager = NULL;
int g_status = AGENT_STATUS_OFFLINE;

void smloader_function_load(SAManager_Interface * SAManager)
{
	if(SAManager != NULL && SAManager->Handler != NULL)
	{
		SAManager->SAManager_Initialize_API = (SAMANAGER_INITIALIZE)dlsym(SAManager->Handler, "SAManager_Initialize");
		SAManager->SAManager_Uninitialize_API = (SAMANAGER_UNINITIALIZE)dlsym(SAManager->Handler, "SAManager_Uninitialize");
		SAManager->SAManager_SetPublishCB_API = (SAMANAGER_SETPBUBLISHCB)dlsym(SAManager->Handler, "SAManager_SetPublishCB");
		SAManager->SAManager_SetSubscribeCB_API = (SAMANAGER_SETSUBSCRIBECB)dlsym(SAManager->Handler, "SAManager_SetSubscribeCB");
		SAManager->SAManager_SetConnectServerCB_API = (SAMANAGER_SETCONNECTSERVERCB)dlsym(SAManager->Handler, "SAManager_SetConnectServerCB");
		SAManager->SAManager_SetDisconnectCB_API = (SAMANAGER_SETDISCONNECTCB)dlsym(SAManager->Handler, "SAManager_SetDisconnectCB");
		SAManager->SAManager_SetOSInfoSendCB_API = (SAMANAGER_SETOSINFOSENDCB)dlsym(SAManager->Handler, "SAManager_SetOSInfoSendCB");
		SAManager->SAManager_InternalSubscribe_API = (SAMANAGER_INTERNALSUBSCRIBE)dlsym(SAManager->Handler, "SAManager_InternalSubscribe");
		SAManager->SAManager_UpdateConnectState_API = (SAMANAGER_UPDATECONNECTSTATE)dlsym(SAManager->Handler, "SAManager_UpdateConnectState");
	}
}

bool smloader_is_exist(char* path)
{
	char file[MAX_PATH] = {0};
	util_path_combine(file, path, DEF_SAMANAGER_LIB_NAME);
	return util_dlexist(file);
}

bool smloader_load(char* path, SAManager_Interface * SAManager)
{
	bool bRet = false;
	void * hSAMANAGERDLL = NULL;
	char file[MAX_PATH] = {0};
	if(!SAManager)
		return bRet;
	util_path_combine(file, path, DEF_SAMANAGER_LIB_NAME);
	if(util_dlopen(file, &hSAMANAGERDLL))
	{
		memset(SAManager, 0, sizeof(SAManager_Interface));
		SAManager->Handler = hSAMANAGERDLL;
		smloader_function_load(SAManager);
		bRet = true;
	}
	return bRet;
}

bool smloader_release(SAManager_Interface * SAManager)
{
	bool bRet = true;
	if(SAManager != NULL)
	{
		if(SAManager->Handler)
			util_dlclose(SAManager->Handler);
		SAManager->Handler = NULL;
	}
	return bRet;
}

char* smloader_get_error()
{
	char *error = util_dlerror();
	return error;
}

void smloader_connect_status_update_cb(int status)
{
	if(g_SAManager)
	{
		if(g_SAManager->SAManager_UpdateConnectState_API)
			g_SAManager->SAManager_UpdateConnectState_API(status);
	}
}

void smloader_init(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile, void * loghandle)
{
	if(smloader_is_exist(profile->workdir))
	{
		g_SAManager = malloc(sizeof(SAManager_Interface));
		if(smloader_load(profile->workdir, g_SAManager))
		{
			g_SAManager->logHandle = loghandle;
			if(g_SAManager->SAManager_Initialize_API)
				g_SAManager->SAManager_Initialize_API(config, profile, loghandle);

			evtqueue_init(1, smloader_connect_status_update_cb);
		}
		else
		{
			char* err = smloader_get_error();
			SAClientLog(loghandle, Warning, "Load SAManager failed! %s!", err);
			free(err);
		}
	}
	else
	{
		char* err = smloader_get_error();
		SAClientLog(loghandle, Warning, "Cannot find SAManager! %s!", err);
		free(err);
	}
}

void smloader_uninit()
{
	if(g_SAManager)
	{
		evtqueue_uninit();

		if(g_SAManager->SAManager_Uninitialize_API)
			g_SAManager->SAManager_Uninitialize_API();
		if(g_SAManager->SAManager_SetPublishCB_API)
			g_SAManager->SAManager_SetPublishCB_API(NULL);
		if(g_SAManager->SAManager_SetSubscribeCB_API)
			g_SAManager->SAManager_SetSubscribeCB_API(NULL);

		if(g_SAManager->SAManager_SetConnectServerCB_API)
			g_SAManager->SAManager_SetConnectServerCB_API(NULL);
		if(g_SAManager->SAManager_SetDisconnectCB_API)
			g_SAManager->SAManager_SetDisconnectCB_API(NULL);

		if(g_SAManager->SAManager_SetOSInfoSendCB_API)
			g_SAManager->SAManager_SetOSInfoSendCB_API(NULL);

		smloader_release(g_SAManager);
		free(g_SAManager);
		g_SAManager = NULL;
	}
}

void smloader_callback_set(PUBLISHCBFN fn_publish, SUBSCRIBECBFN fn_subscribe, CONNECTSERVERCBFN fn_connserver, DISCONNECTCBFN fn_disconnect)
{
	if(!g_SAManager)
		return;
	if(g_SAManager->SAManager_SetPublishCB_API)
		g_SAManager->SAManager_SetPublishCB_API(fn_publish);

	if(g_SAManager->SAManager_SetSubscribeCB_API)
		g_SAManager->SAManager_SetSubscribeCB_API(fn_subscribe);

	if(g_SAManager->SAManager_SetConnectServerCB_API)
		g_SAManager->SAManager_SetConnectServerCB_API(fn_connserver);

	if(g_SAManager->SAManager_SetDisconnectCB_API)
		g_SAManager->SAManager_SetDisconnectCB_API(fn_disconnect);
}

void smloader_osinfo_send_set(SENDOSINFOCBFN fn_sendosinfo)
{
	if(!g_SAManager)
		return;

	if(g_SAManager->SAManager_SetOSInfoSendCB_API)
		g_SAManager->SAManager_SetOSInfoSendCB_API(fn_sendosinfo);
}

void smloader_connect_status_update(int status)
{
	/*if(g_status == status)
		return;
	g_status = status;*/
	evtqueue_clear();
	if(!evtqueue_push(status))
		SAClientLog(g_SAManager->logHandle, Warning, "Cannot push Event Status: %d!", status);
}

void smloader_internal_subscribe()
{
	if(g_SAManager)
		{
			if(g_SAManager->SAManager_InternalSubscribe_API)
				g_SAManager->SAManager_InternalSubscribe_API();
		}
}
