#ifndef SA_DATA_SYNC_H
#define SA_DATA_SYNC_H

#include "susiaccess_def.h"
#include "susiaccess_handler_mgmt.h"
#include "SAManager.h"
#include <stdbool.h>


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <Windows.h>
#ifndef SADATASYNC_API
#define SADATASYNC_API WINAPI
#endif
#else
#define SADATASYNC_API
#endif

//typedef int (SADATASYNC_API *PUBLISHCB) (char const * topic, int qos, int retain, susiaccess_packet_body_t const * pkt);

typedef bool (SADATASYNC_API *DATASYNC_INITIALIZE)(char * pWorkdir,Handler_List_t *pLoaderList,void* pLogHandle);
typedef void (SADATASYNC_API *DATASYNC_UNINITIALIZE)();
typedef void (SADATASYNC_API *DATASYNC_SETFUNCCB)(PUBLISHCB g_publishCB);
typedef void (SADATASYNC_API *DATASYNC_INSERT_CAP)(void* const handle,char *cap,char *captopic, int result);
typedef void (SADATASYNC_API *DATASYNC_INSERT_REP)(void* const handle,char *rep,char *reptopic, int result);
typedef void (SADATASYNC_API *DATASYNC_SET_LOSTTIME)(unsigned long long losttime);
typedef void (SADATASYNC_API *DATASYNC_SET_RECONTIME)(unsigned long long recontime);




typedef struct SADATASYNC_INTERFACE
{
	void*						Handler;               // handle of to load so library
	DATASYNC_INITIALIZE			DataSync_Initialize_API;
	DATASYNC_UNINITIALIZE		DataSync_Uninitialize_API;
	DATASYNC_SETFUNCCB			DataSync_SetFuncCB_API;
	DATASYNC_INSERT_CAP			DataSync_Insert_Cap_API;
	DATASYNC_INSERT_REP			DataSync_Insert_Rep_API;
	DATASYNC_SET_LOSTTIME		DataSync_Set_LostTime_API;
	DATASYNC_SET_RECONTIME		DataSync_Set_ReConTime_API;
	void*						LogHandle;
}SADataSync_Interface;

#ifdef __cplusplus
extern "C" {
#endif

bool SADataSync_Is_Exist(const char* path);
bool SADataSync_Load(const char* path, SADataSync_Interface* pSADataSync);
SADataSync_Interface* SADataSync_Initialize(char const * pWorkdir,void* pLogHandle);
bool SADataSync_Uninitialize(SADataSync_Interface * pSADataSync);

#ifdef __cplusplus
}
#endif


#endif
