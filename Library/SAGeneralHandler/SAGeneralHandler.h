#ifndef _GENERAL_CMD_HANDLER_H
#define _GENERAL_CMD_HANDLER_H

#include "susiaccess_handler_mgmt.h"

#define cagent_request_general 1001
#define cagent_action_general 2001

typedef enum{
	SERVER_UNDEFINED = -1,
	SERVER_LOST_CONNECTION = 0,
	SERVER_AUTH_SUCCESS = 1,
	SERVER_AUTH_FAILED,
	SERVER_CONNECTION_FULL,
	SERVER_RECONNECT,
	SERVER_CONNECT_TO_MASTER,
	SERVER_CONNECT_TO_SEPCIFIC,
	SERVER_PRESERVED_MESSAGE,
	SERVER_SET_SERVER_IP_LIST,
}server_status_code_t;

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <Windows.h>
#ifndef SAGENERAL_API
#define SAGENERAL_API WINAPI
#endif
#else
#define SAGENERAL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

int SAGENERAL_API General_Initialize(HANDLER_INFO *pluginfo);
void SAGENERAL_API General_Uninitialize();
void SAGENERAL_API General_HandleRecv( char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2 );
void SAGENERAL_API General_SetPluginHandlers(Handler_List_t *pLoaderList);
void SAGENERAL_API General_OnStatusChange( HANDLER_INFO *pluginfo );
void SAGENERAL_API General_Start();
void SAGENERAL_API General_Stop();

#ifdef __cplusplus
}
#endif

#endif