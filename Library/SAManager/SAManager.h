#ifndef _SA_MANAGER_H_
#define _SA_MANAGER_H_
#include "susiaccess_def.h"
#include <stdbool.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <Windows.h>
#ifndef SAMANAGER_API
#define SAMANAGER_API WINAPI
#endif
#else
#define SAMANAGER_API
#endif


typedef void (*MESSAGERECVCB)(char* topic, susiaccess_packet_body_t *pkt, void *pRev1, void* pRev2);
typedef int (SAMANAGER_API *PUBLISHCB) (char const * topic, int qos, int retain, susiaccess_packet_body_t const * pkt);
typedef int (SAMANAGER_API *SUBSCRIBECB)(char const * topic, int qos, MESSAGERECVCB msg_recv_callback);
//typedef int (SAMANAGER_API *CONNECTSERVERCB)(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile);
typedef int (SAMANAGER_API *CONNECTSERVERCB)(char const * ip, int port, char const * mqttauth, tls_type tlstype, char const * psk);
typedef void (SAMANAGER_API *DISCONNECTCB)();
typedef bool (*SENDOSINFOCB)();

#ifdef __cplusplus
extern "C" {
#endif

	void SAMANAGER_API SAManager_Initialize(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile, void * loghandle);
	void SAMANAGER_API SAManager_Uninitialize();
	void SAMANAGER_API SAManager_SetPublishCB(PUBLISHCB func);
	void SAMANAGER_API SAManager_SetSubscribeCB(SUBSCRIBECB func);
	void SAMANAGER_API SAManager_SetConnectServerCB(CONNECTSERVERCB func);
	void SAMANAGER_API SAManager_SetDisconnectCB(DISCONNECTCB func);
	void SAMANAGER_API SAManager_SetOSInfoSendCB(SENDOSINFOCB func);
	void SAMANAGER_API SAManager_InternalSubscribe();
	void SAMANAGER_API SAManager_UpdateConnectState(int status);

#ifdef __cplusplus
}
#endif

#endif