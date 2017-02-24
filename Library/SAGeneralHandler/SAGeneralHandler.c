#include "SAGeneralHandler.h"
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <cJSON.h>
#ifndef _WIN32
  #include <sys/types.h>
  #include <sys/stat.h>
#endif
#include "general_def.h"
#include "generallog.h"
#include "ftphelper.h"
#include "md5.h"
#include "profile.h"
#include "AdvPlatform.h"
#include "util_path.h"
#include "util_process.h"
#include "ghparser.h"
#include "agentupdater.h"

static Handler_info_ex  g_PluginInfo;
static HandlerSendCbf  g_sendcbf = NULL;						// Client Send information (in JSON format) to Cloud Server	
static HandlerSendCustCbf  g_sendcustcbf = NULL;			    // Client Send information (in JSON format) to Cloud Server with custom topic	
static HandlerSubscribeCustCbf g_subscribecustcbf = NULL;
static HandlerAutoReportCbf g_sendreportcbf = NULL;				// Client Send report (in JSON format) to Cloud Server with AutoReport topic
static HandlerConnectServerCbf g_connectservercbf = NULL;	
static HandlerDisconnectCbf g_disconnectcbf = NULL;	
static HandlerSendCapabilityCbf g_sendcapabilitycbf = NULL;	
static HandlerRenameCbf g_renamecbf = NULL;
static HandlerSendOSInfoCbf g_sendosinfocbf;

LOGHANDLE g_SAGeneralLogHandle = NULL;

//static pthread_t g_GetCapabilityThreadHandle = 0;
static char g_ConfigPath[MAX_PATH] = {0};

const char genreral_Topic[MAX_TOPIC_LEN] = {"general"};
const int genreral_RequestID = cagent_request_general;
const int genreral_ActionID = cagent_action_general;

Handler_List_t *g_pPL_List = NULL;
//int g_redundantServerNum = 0;
//int g_connectedFailedCnt = 0;

bool g_bEnableHeartbeat = false;
int g_iHeartbeatRate = 60;
pthread_t g_threadHeartbeat = 0;
#define DEF_HEARTBEAT_TOPIC					"/cagent/admin/%s/notify"	/*publish*/
#define DEF_HEARTBEAT_MESSAGE_JSON			"{\"hb\":{\"devID\":\"%s\"}}"
#define DEF_HEARTBEATRATE_RESPONSE_SESSION_JSON	"{\"susiCommData\":{\"commCmd\":%d,\"catalogID\":4,\"handlerName\":\"general\",\"heartbeatrate\":%d,\"sessionID\":\"%s\"}}"

#ifdef _MSC_VER
BOOL WINAPI DllMain(HINSTANCE module_handle, DWORD reason_for_call, LPVOID reserved)
{
	if (reason_for_call == DLL_PROCESS_ATTACH) // Self-explanatory
	{
		printf("DllInitializer\n");
		DisableThreadLibraryCalls(module_handle); // Disable DllMain calls for DLL_THREAD_*
		if (reserved == NULL) // Dynamic load
		{
			// Initialize your stuff or whatever
			// Return FALSE if you don't want your module to be dynamically loaded
		}
		else // Static load
		{
			// Return FALSE if you don't want your module to be statically loaded
		}
	}

	if (reason_for_call == DLL_PROCESS_DETACH) // Self-explanatory
	{
		printf("DllFinalizer\n");
		if (reserved == NULL) // Either loading the DLL has failed or FreeLibrary was called
		{
			// Cleanup
			General_Uninitialize();
		}
		else // Process is terminating
		{
			// Cleanup
			General_Uninitialize();
		}
	}
	return TRUE;
}
#else
__attribute__((constructor))
/**
 * initializer of the shared lib.
 */
static void Initializer(int argc, char** argv, char** envp)
{
    printf("DllInitializer\n");
}

__attribute__((destructor))
/** 
 * It is called when shared lib is being unloaded.
 * 
 */
static void Finalizer()
{
    printf("DllFinalizer\n");
	General_Uninitialize();
}
#endif

bool  GeneralSendHeartbeat()
{
	char strPayloadBuff[512] = {0};
	char strTopicBuff[256] = {0};
	if( g_PluginInfo.agentInfo == NULL) return false;

	sprintf(strPayloadBuff, DEF_HEARTBEAT_MESSAGE_JSON, g_PluginInfo.agentInfo->devId);
	sprintf(strTopicBuff, DEF_HEARTBEAT_TOPIC, g_PluginInfo.agentInfo->devId);
	if(g_sendcustcbf)
	{
		if(g_sendcustcbf(NULL,0, strTopicBuff, strPayloadBuff, strlen(strPayloadBuff), NULL, NULL)==0)
			return true;
		else
			return false;
	}
	return false;
}

void* threadheartbeat(void* args)
{
	int count = 0, interval=1000;
	int iHeartbeat = g_iHeartbeatRate;

	if(iHeartbeat>0)
{
//SAGeneralLog(g_SAGeneralLogHandle, Debug, " %s> Send heartbeat!", genreral_Topic);
		GeneralSendHeartbeat();
//SAGeneralLog(g_SAGeneralLogHandle, Debug, " %s> Send heartbeat finish!", genreral_Topic);
}

	while(g_bEnableHeartbeat)
	{
		if(g_iHeartbeatRate!=iHeartbeat)
		{
//SAGeneralLog(g_SAGeneralLogHandle, Debug, "\n************** %s> heartbeat rate change from %d to %d!", genreral_Topic, iHeartbeat, g_iHeartbeatRate);
			iHeartbeat = g_iHeartbeatRate;
		}
		count = iHeartbeat*1000/interval;
		while(count > 0)
		{
			if(g_iHeartbeatRate!=iHeartbeat)
			{
//SAGeneralLog(g_SAGeneralLogHandle, Debug, "\n************** %s> heartbeat rate change from %d to %d!", genreral_Topic, iHeartbeat, g_iHeartbeatRate);
				iHeartbeat = g_iHeartbeatRate;
				count = iHeartbeat*1000/interval;
			}

			if(!g_bEnableHeartbeat)
				break;

			count--;
			usleep(interval*1000);
		}
		if(!g_bEnableHeartbeat)
			break;

		if(iHeartbeat>0)
{
//SAGeneralLog(g_SAGeneralLogHandle, Debug, " %s> Send heartbeat!", genreral_Topic);
			GeneralSendHeartbeat();
//SAGeneralLog(g_SAGeneralLogHandle, Debug, " %s> Send heartbeat finish!", genreral_Topic);
}
	}
//SAGeneralLog(g_SAGeneralLogHandle, Debug, "\n************** %s> threadheartbeat stopped!", genreral_Topic);
}

bool  GeneralSend(int cmd, char const * msg, int len, void *pRev1, void* pRev2)
{
	bool bRet = false;
	char* payload = NULL; 
	cJSON* node = cJSON_Parse(msg);
	int payloadlenth = 0;
	if(node == NULL)
	{
		payloadlenth = len+strlen("{\"result\":\"\"}"); //"msg":"XXX"
		payload = malloc(payloadlenth+1);
		memset(payload, 0, payloadlenth+1);
		sprintf(payload, "{\"result\":\"%s\"}", msg);
	}
	else
	{
		cJSON_Delete(node);
		payloadlenth = len;
		payload = malloc(payloadlenth+1);
		memset(payload, 0, payloadlenth+1);
		strcpy(payload, msg);
	}
	if(g_sendcbf)
		g_sendcbf(&g_PluginInfo, cmd, payload, payloadlenth, pRev1, pRev2);
	free(payload);
	payload = NULL;
	return bRet;
}

static void* thread_agent_get_capability(void *args)
{
	Handler_Loader_Interface *pInterfaceTmp = NULL;
	int length = 0;
	Handler_List_t *pLoaderList = (Handler_List_t *)args;
	if(pLoaderList == NULL)
	{
		pthread_exit(0);
		return 0;
	}

	if(g_sendosinfocbf)
		g_sendosinfocbf();

	pInterfaceTmp = pLoaderList->items;
	while(pInterfaceTmp)
	{
		char* tmpinfo = NULL;
		if(pInterfaceTmp->Workable == false)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}
		if(pInterfaceTmp->type != user_handler && pInterfaceTmp->type != virtual_handler)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}

		if(pInterfaceTmp->Handler_Get_Capability_API)
			length = pInterfaceTmp->Handler_Get_Capability_API(&tmpinfo);
	
		if(length>0)
		{
			if(g_sendcapabilitycbf)
				g_sendcapabilitycbf(&g_PluginInfo, tmpinfo, strlen(tmpinfo), NULL,NULL);
		}
		if(tmpinfo)
		{
			if(pInterfaceTmp->Handler_MemoryFree_API)
				pInterfaceTmp->Handler_MemoryFree_API(tmpinfo);
			else
				free(tmpinfo);
		}
			
		pInterfaceTmp = pInterfaceTmp->next;
	}
	pthread_exit(0);
	return 0;
}

bool StartAutoReport(Handler_List_t *pLoaderList, char* data)
{
	Handler_Loader_Interface *pInterfaceTmp = NULL;
	if(!pLoaderList) return false;

	pInterfaceTmp = pLoaderList->items;
	while(pInterfaceTmp)
	{
		if(pInterfaceTmp->Workable == false)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}
		if(pInterfaceTmp->type != user_handler && pInterfaceTmp->type != virtual_handler)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}
		if(pInterfaceTmp->Handler_AutoReportStart_API)
			pInterfaceTmp->Handler_AutoReportStart_API(data);
		pInterfaceTmp = pInterfaceTmp->next;
	}
	return true;
}

bool StopAutoReport(Handler_List_t *pLoaderList, char* data)
{
	Handler_Loader_Interface *pInterfaceTmp = NULL;

	if(!pLoaderList) return false;

	if(pLoaderList->total <= 0) return false;

	pInterfaceTmp = pLoaderList->items;
	while(pInterfaceTmp)
	{
		if(pInterfaceTmp->Workable == false)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}
		if(pInterfaceTmp->type != user_handler && pInterfaceTmp->type != virtual_handler)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}
		if(pInterfaceTmp->Handler_AutoReportStop_API)
			pInterfaceTmp->Handler_AutoReportStop_API(data);

		pInterfaceTmp = pInterfaceTmp->next;
	}
	return true;
}

bool SendHandlerList(Handler_List_t *pLoaderList)
{
	/*{"susiCommData":{"commCmd":124,"catalogID":4,"requestID":1001,"handlerName":"general","handlerlist":["handler1","handler2"]}}*/
	bool bRet = false;
	cJSON * root = NULL, *body = NULL; 
	char* cPayload = NULL;
	Handler_Loader_Interface *pInterfaceTmp = NULL;

	if(!pLoaderList) return bRet;

	if(pLoaderList->total <= 0) return bRet;

	body = cJSON_CreateArray();
	pInterfaceTmp = pLoaderList->items;
	while(pInterfaceTmp)
	{
		cJSON* pTempNode = NULL;
		if(pInterfaceTmp->Workable == false)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}
		if(pInterfaceTmp->type != user_handler && pInterfaceTmp->type != virtual_handler)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}
		pTempNode = cJSON_CreateString(pInterfaceTmp->pHandlerInfo->Name);
		cJSON_AddItemToArray(body, pTempNode);
		pInterfaceTmp = pInterfaceTmp->next;
	}

	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "handlerlist", body);
	cPayload = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	
	bRet = GeneralSend(glb_get_handler_list_rep, cPayload, strlen(cPayload), NULL, NULL);
	free(cPayload);
	return bRet;
}

void ServercontrolAction(General_Ctrl_Msg *pRespMsg)
{
	SAGeneralLog(g_SAGeneralLogHandle, Normal, " %s> Server Response: %s", genreral_Topic, pRespMsg->msg);

	switch (pRespMsg->statuscode)
	{
	case SERVER_LOST_CONNECTION:
	case SERVER_AUTH_SUCCESS:
	case SERVER_PRESERVED_MESSAGE:
		break;
	case SERVER_AUTH_FAILED:
	case SERVER_CONNECTION_FULL:
		{
			if(g_disconnectcbf)
				g_disconnectcbf();
		}
		break;
	case SERVER_RECONNECT:
		{
			if(g_connectservercbf)
				g_connectservercbf(g_PluginInfo.ServerIP, g_PluginInfo.ServerPort, g_PluginInfo.serverAuth, g_PluginInfo.TLSType, g_PluginInfo.PSK);
		}
		break;
	case SERVER_CONNECT_TO_MASTER:
		{
			//connect to master
			//if(g_connectservercbf)
			//	g_connectservercbf(master_ip, master_port, master_auth);
		}
		break;
	case SERVER_CONNECT_TO_SEPCIFIC:
		{
			//connect to server
			if(g_connectservercbf)
				g_connectservercbf(pRespMsg->serverIP, pRespMsg->serverPort, pRespMsg->serverAuth, g_PluginInfo.TLSType, g_PluginInfo.PSK);
		}
		break;
	default:
		break;
	}
}

int SAGENERAL_API General_Initialize(HANDLER_INFO *pluginfo)
{
	Handler_info_ex* tmpinfo = NULL;
	if( pluginfo == NULL )
		return handler_fail;

	tmpinfo = (Handler_info_ex*)pluginfo;
	g_SAGeneralLogHandle = tmpinfo->loghandle;

	// 1. Topic of this handler
	snprintf( tmpinfo->Name, sizeof(tmpinfo->Name), "%s", genreral_Topic );
	SAGeneralLog(g_SAGeneralLogHandle, Normal, " %s> Initialize", genreral_Topic);
	tmpinfo->RequestID = genreral_RequestID;
	tmpinfo->ActionID = genreral_ActionID;
	
	// 2. Copy agent info 
	memcpy(&g_PluginInfo, tmpinfo, sizeof(Handler_info_ex));

	// 3. Callback function -> Send JSON Data by this callback function
	g_sendcbf = g_PluginInfo.sendcbf = tmpinfo->sendcbf;
	g_sendcustcbf = g_PluginInfo.sendcustcbf = tmpinfo->sendcustcbf;
	g_subscribecustcbf = g_PluginInfo.subscribecustcbf = tmpinfo->subscribecustcbf;
	g_sendreportcbf = g_PluginInfo.sendreportcbf = tmpinfo->sendreportcbf;
	g_sendcapabilitycbf = g_PluginInfo.sendcapabilitycbf = tmpinfo->sendcapabilitycbf;
	g_connectservercbf = g_PluginInfo.connectservercbf = tmpinfo->connectservercbf;	
	g_disconnectcbf = g_PluginInfo.disconnectcbf = tmpinfo->disconnectcbf;
	g_renamecbf = g_PluginInfo.renamecbf = tmpinfo->renamecbf;
	g_sendosinfocbf = g_PluginInfo.sendosinfocbf = tmpinfo->sendosinfocbf;
	
	util_path_combine(g_ConfigPath, tmpinfo->WorkDir, DEF_CONFIG_FILE_NAME);
	
	g_sendcbf = tmpinfo->sendcbf;
	return handler_success;
}

void SAGENERAL_API General_Uninitialize()
{
	/*if(g_GetCapabilityThreadHandle)
	{
		pthread_join(g_GetCapabilityThreadHandle, NULL);
		g_GetCapabilityThreadHandle = 0;
	}*/
	if(g_threadHeartbeat)
	{
		g_bEnableHeartbeat = false;
		pthread_join(g_threadHeartbeat, NULL);
		g_threadHeartbeat = 0;
	}

	StopAutoReport(g_pPL_List, NULL);
	g_sendcbf = NULL;
	g_SAGeneralLogHandle = NULL;

	updater_update_stop();

	g_pPL_List = NULL;
}



void SAGENERAL_API General_HandleRecv( char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2 )
{
	int cmdID = 0;
	char cSessionID[64] = {0};
	int respID = 0;
	char cResponse[512] = {0};
	SAGeneralLog(g_SAGeneralLogHandle, Normal, " %s>Recv Topic [%s] Data %s", genreral_Topic, topic, (char*) data );

	if(!ParseReceivedCMD(data, datalen, &cmdID, cSessionID))
		return;
	switch (cmdID)
	{
	case general_info_spec_req:
		{
			if(g_pPL_List)
			{
				/*if(g_GetCapabilityThreadHandle)
				{
					pthread_join(g_GetCapabilityThreadHandle, NULL);
					g_GetCapabilityThreadHandle = 0;
				}*/
				pthread_t getCapabilityThreadHandle = 0;
				if(pthread_create(&getCapabilityThreadHandle, NULL, thread_agent_get_capability, g_pPL_List)==0)
					pthread_detach(getCapabilityThreadHandle);
			}
		}
		break;
	case general_start_auto_upload_req:
		{
			StartAutoReport(g_pPL_List, data);
			
			if(strlen(cSessionID)>0)
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\",\"sessionID\":\"%s\"}", "SUCCESS", cSessionID);
			else
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\"}", "SUCCESS");

			respID = general_start_auto_upload_rep;
		}
		break;
	case general_stop_auto_upload_req:
		{
			StopAutoReport(g_pPL_List, data);
			if(strlen(cSessionID)>0)
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\",\"sessionID\":\"%s\"}", "SUCCESS", cSessionID);
			else
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\"}", "SUCCESS");
			respID = general_stop_auto_upload_rep;
		}
		break;
	case glb_update_cagent_req:
		{
			char repMsg[256];
		
			if(!updater_update_start(g_PluginInfo.ServerIP, data, datalen, repMsg, GeneralSend, g_SAGeneralLogHandle))
			{
				if(strlen(repMsg)>0)
				{
					if(strlen(cSessionID)>0)
						snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\",\"sessionID\":\"%s\"}", repMsg, cSessionID);
					else
						snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\"}", repMsg);
					respID = glb_update_cagent_rep;
				}
			}
		}
		break;
	case glb_get_init_info_rep:
		{
			if(g_sendosinfocbf)
				g_sendosinfocbf();
		}
		break;
	case glb_update_cagent_stop_req:
		{
			updater_update_stop();
			if(strlen(cSessionID)>0)
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\",\"sessionID\":\"%s\"}", "Update cagent stop success!", cSessionID);
			else
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\"}", "Update cagent stop success!");
			respID = glb_update_cagent_stop_rep;
			break;
		}
	case glb_update_cagent_retry_req:
		{
			char repMsg[256];
			updater_update_retry(g_PluginInfo.ServerIP, data, datalen, repMsg, GeneralSend, g_SAGeneralLogHandle);
			if(strlen(repMsg)>0)
			{
				if(strlen(cSessionID)>0)
					snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\",\"sessionID\":\"%s\"}", repMsg, cSessionID);
				else
					snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\"}", repMsg);
				respID = glb_update_cagent_retry_rep;
			}
			break;
		}
	case glb_get_handler_list_req:
		{
			SendHandlerList(g_pPL_List);
			break;
		}
	case glb_cagent_rename_req:
		{
			char name[DEF_HOSTNAME_LENGTH] = {0};
			bool bRename = false;
			//SAGeneralLog(Normal, " %s> Rename: %s", genreral_Topic, data);
			if(ParseRenameCMD(data, datalen, name))
				bRename = profile_set(g_ConfigPath, "DeviceName", name);
			if(bRename)
			{
				if(g_renamecbf)
					g_renamecbf(name);
			}
			if(strlen(cSessionID)>0)
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\",\"sessionID\":\"%s\"}", bRename?"SUCCESS":"FALSE", cSessionID);
			else
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\"}", bRename?"SUCCESS":"FALSE");
			respID = glb_cagent_rename_rep;
			break;
		}
	case glb_server_control_req:
		{
			int statuscode = SERVER_UNDEFINED;
			char *message = NULL;
			General_Ctrl_Msg pRespMsg;
			memset(&pRespMsg,0,sizeof(General_Ctrl_Msg));
			ParseServerCtrl(data, datalen, g_PluginInfo.WorkDir, &pRespMsg);
			ServercontrolAction(&pRespMsg);
			if(pRespMsg.msg)
				free(pRespMsg.msg);
			break;
		}
	case glb_cagent_heartbeatratequery_req:
		{
			if(strlen(cSessionID)>0)
				snprintf(cResponse, sizeof(cResponse), "{\"heartbeatrate\":%d,\"sessionID\":\"%s\"}", g_iHeartbeatRate, cSessionID);
			else
				snprintf(cResponse, sizeof(cResponse), "{\"heartbeatrate\":%d}", g_iHeartbeatRate);
			respID = glb_cagent_heartbeatratequery_rep;
		}
		break;
	case glb_cagent_heartbeatrateupdte_req:
		{
			int heartbeatrate = 60;
			char status[8]={0};
			if(ParseHeartbeatRateUpdateCMD(data, datalen, &heartbeatrate))
			{
				strcpy(status, "SUCCESS");
				g_iHeartbeatRate = heartbeatrate;
			}
			else
				strcpy(status, "FALSE");
			if(strlen(cSessionID)>0)
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\",\"sessionID\":\"%s\"}", status, cSessionID);
			else
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\"}", status);
			respID = glb_cagent_heartbeatrateupdte_rep;
		}
		break;
	default:
		{
			if(strlen(cSessionID)>0)
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\",\"sessionID\":\"%s\"}", "Unknown cmd!", cSessionID);
			else
				snprintf(cResponse, sizeof(cResponse), "{\"result\":\"%s\"}", "Unknown cmd!");
			respID = glb_error_rep;
			break;
		}
	}

	if(respID>0 && strlen(cResponse)>0)
		GeneralSend(respID, cResponse, strlen(cResponse), NULL, NULL);
}

void SAGENERAL_API General_SetPluginHandlers(Handler_List_t *pLoaderList)
{
	g_pPL_List = pLoaderList;
}

void SAGENERAL_API General_OnStatusChange( HANDLER_INFO *pluginfo )
{
SAGeneralLog(g_SAGeneralLogHandle, Debug, " %s> Update Status: %d", genreral_Topic, pluginfo->agentInfo->status);
	if(pluginfo)
		memcpy(&g_PluginInfo, pluginfo, sizeof(HANDLER_INFO));
	else
	{
		memset(&g_PluginInfo, 0, sizeof(HANDLER_INFO));
		snprintf( g_PluginInfo.Name, sizeof( g_PluginInfo.Name), "%s", genreral_Topic );
		g_PluginInfo.RequestID = genreral_RequestID;
		g_PluginInfo.ActionID = genreral_ActionID;
	}
	if(g_threadHeartbeat)
	{
//SAGeneralLog(g_SAGeneralLogHandle, Debug, "\n************** %s> Join threadheartbeat!", genreral_Topic);
		g_bEnableHeartbeat = false;
		pthread_join(g_threadHeartbeat, NULL);
		g_threadHeartbeat = 0;
	}

	if(pluginfo->agentInfo->status == AGENT_STATUS_ONLINE)
	{
		if(g_threadHeartbeat  == 0)
		{
//SAGeneralLog(g_SAGeneralLogHandle, Debug, "\n************** %s> Create threadheartbeat!", genreral_Topic);
			g_bEnableHeartbeat = true;
			if(pthread_create(&g_threadHeartbeat, NULL, threadheartbeat, NULL)!=0)
			{
				SAGeneralLog(g_SAGeneralLogHandle, Warning, " %s> Create threadheartbeat Failed!", genreral_Topic);
				g_threadHeartbeat = 0;
			}
		}	
		else
		{
//SAGeneralLog(g_SAGeneralLogHandle, Warning, "\n************** %s> threadheartbeat haven't restart!", genreral_Topic);		
		}
	}
}

void SAGENERAL_API General_Start()
{
	
}

void SAGENERAL_API General_Stop()
{

}
