#ifndef _SA_SAMGRLOADER_H_
#define _SA_SAMGRLOADER_H_
#include <stdbool.h>
#include "Log.h"
#include "susiaccess_def.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <windows.h>
#ifndef SAMGRLOADER_API
#define SAMGRLOADER_API WINAPI
#endif
#else
#define SAMGRLOADER_API
#endif

typedef void (*MESSAGERECVCBFN)(char* topic, susiaccess_packet_body_t *pkt, void *pRev1, void* pRev2);
typedef int (SAMGRLOADER_API *PUBLISHCBFN) (char const * topic, int qos, int retain, susiaccess_packet_body_t const * pkt);
typedef int (SAMGRLOADER_API *SUBSCRIBECBFN)(char const * topic, int qos, MESSAGERECVCBFN msg_recv_callback);
//typedef int (SAMGRLOADER_API *CONNECTSERVERCBFN)(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile);
typedef int (SAMGRLOADER_API *CONNECTSERVERCBFN)(char const * ip, int port, char const * mqttauth, tls_type tlstype, char const * psk);
typedef void (SAMGRLOADER_API *DISCONNECTCBFN)();
typedef bool (*SENDOSINFOCBFN)();

typedef void (SAMGRLOADER_API *SAMANAGER_INITIALIZE)(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile, void * loghandle);
typedef void (SAMGRLOADER_API *SAMANAGER_UNINITIALIZE)();
typedef void (SAMGRLOADER_API *SAMANAGER_SETPBUBLISHCB)(PUBLISHCBFN func);
typedef void (SAMGRLOADER_API *SAMANAGER_SETSUBSCRIBECB)(SUBSCRIBECBFN func);
typedef void (SAMGRLOADER_API *SAMANAGER_SETCONNECTSERVERCB)(CONNECTSERVERCBFN func);
typedef void (SAMGRLOADER_API *SAMANAGER_SETDISCONNECTCB)(DISCONNECTCBFN func);
typedef void (SAMGRLOADER_API *SAMANAGER_SETOSINFOSENDCB)(SENDOSINFOCBFN func);
typedef void (SAMGRLOADER_API *SAMANAGER_INTERNALSUBSCRIBE)();
typedef void (SAMGRLOADER_API *SAMANAGER_UPDATECONNECTSTATE)(int status);

typedef struct SAMANAGER_INTERFACE
{
	void*								Handler;               // handle of to load so library
	SAMANAGER_INITIALIZE				SAManager_Initialize_API;
	SAMANAGER_UNINITIALIZE				SAManager_Uninitialize_API;
	SAMANAGER_SETPBUBLISHCB				SAManager_SetPublishCB_API;
	SAMANAGER_SETSUBSCRIBECB			SAManager_SetSubscribeCB_API;
	SAMANAGER_SETCONNECTSERVERCB		SAManager_SetConnectServerCB_API;
	SAMANAGER_SETDISCONNECTCB			SAManager_SetDisconnectCB_API;
	SAMANAGER_SETOSINFOSENDCB			SAManager_SetOSInfoSendCB_API;
	SAMANAGER_INTERNALSUBSCRIBE			SAManager_InternalSubscribe_API;
	SAMANAGER_UPDATECONNECTSTATE		SAManager_UpdateConnectState_API;
	LOGHANDLE							logHandle;
}SAManager_Interface;

#ifdef __cplusplus
extern "C" {
#endif

void smloader_init(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile, void * loghandle);

void smloader_uninit();

void smloader_callback_set(PUBLISHCBFN fn_publish, SUBSCRIBECBFN fn_subscribe, CONNECTSERVERCBFN fn_connserver, DISCONNECTCBFN fn_disconnect);

void smloader_osinfo_send_set(SENDOSINFOCBFN fn_sendosinfo);

void smloader_connect_status_update(int status);

void smloader_internal_subscribe();

#ifdef __cplusplus
}
#endif

#endif
