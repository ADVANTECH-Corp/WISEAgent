#include "SAManager.h"
#include "hlloader.h"
#include "ghloader.h"
#include "pktparser.h"
#include <cJSON.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "SAManagerLog.h"
#include "topic.h"
#include "keepalive.h"
#include <pthread.h>
#include "basequeue.h"
#include "util_path.h"
#include "util_string.h"
//#include "statequeue.h"
#include "SADataSync.h"

#define DEF_CALLBACKREQ_TOPIC			"/cagent/admin/%s/agentcallbackreq"	/*Subscribe*/
#define DEF_ACTIONACK_TOPIC				"/cagent/admin/%s/agentactionack"	/*Subscribe*/
#define DEF_AGENTCONTROL_TOPIC			"/server/admin/+/agentctrl"	/*Subscribe*/
#define DEF_EVENTNOTIFY_TOPIC			"/cagent/admin/%s/eventnotify"	/*publish*/
#define DEF_ACTIONREQ_TOPIC				"/cagent/admin/%s/agentactionreq"	/*publish*/
#define DEF_AUTOREPORT_TOPIC			"/cagent/admin/%s/%s"	/*publish*/
#define AUTOREPORT_TOPIC				"deviceinfo"

#define cagent_action_general			2001
#define general_info_spec_rep			2052
#define general_info_upload_rep			2055
#define general_event_notify_rep		2059

SALoader_Interface* g_pSALoader = NULL;
SAGeneral_Interface* g_SAGeneral = NULL;
SADataSync_Interface* g_SADataSync = NULL;
Handler_List_t g_handlerList;
PUBLISHCB g_publishCB = NULL;
SUBSCRIBECB g_subscribeCB = NULL;
CONNECTSERVERCB g_connectserverCB = NULL;
DISCONNECTCB g_disconnectCB = NULL;
SENDOSINFOCB g_sendosinfoCB = NULL;
int g_iConnStatus = 1;
LOGHANDLE g_samanagerlogger = NULL;

susiaccess_agent_conf_body_t * g_pConfig = NULL;
susiaccess_agent_profile_body_t * g_pProfile = NULL;

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
			SAManager_Uninitialize();
		}
		else // Process is terminating
		{
			// Cleanup
			SAManager_Uninitialize();
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
	SAManager_Uninitialize();
}
#endif

char * ConvertMessageToUTF8(char* wText)
{
	char * utf8RetStr = NULL;
	int tmpLen = 0;
	if(!wText)
		return utf8RetStr;

	utf8RetStr = ANSIToUTF8(wText);
	tmpLen = !utf8RetStr ? 0 : strlen(utf8RetStr);
	if(tmpLen == 1)
	{
		if(utf8RetStr) free(utf8RetStr);
		utf8RetStr = UnicodeToUTF8((wchar_t *)wText);
	}
	return utf8RetStr;
}

susiaccess_packet_body_t*  SAManager_WrapReqPacket(Handler_info const * plugin, int const enum_act, void const * const requestData, unsigned int const requestLen, bool bCust)
{
	susiaccess_packet_body_t* packet = NULL;
	bool bIsCust = false;
#if 0
	char * data = NULL;
	int length = 0;
#endif
	if(plugin == NULL)
	{
		if(requestData == NULL)
			return packet;
		else
			bIsCust = true;
	}
	else 
	{
		if(plugin->agentInfo == NULL)
			return packet;
	}

	packet = malloc(sizeof(susiaccess_packet_body_t));
	memset(packet, 0, sizeof(susiaccess_packet_body_t));

#if 0
	if(requestData)
	{
		data = ConvertMessageToUTF8((char*)requestData);
		length = strlen(data);
	}
	if(length>0)
	{
		packet->content = (char*)malloc(length+1);
		if(packet->content)
		{
			memset(packet->content, 0, length+1);
			memcpy(packet->content, data, length);
		}
		free(data);
	}
#else
	if(requestData)
	{
		packet->content = (char*)malloc(requestLen+1);
		if(packet->content)
		{
			memset(packet->content, 0, requestLen+1);
			memcpy(packet->content, requestData, requestLen);
		}
	}
#endif
	if(bIsCust)
	{
		packet->type = pkt_type_custom;
		return packet;
	}

	strcpy(packet->devId, plugin->agentInfo->devId);
	strcpy(packet->handlerName, plugin->Name);

	if(bCust)
	{
		//packet->requestID = cagent_custom_action;
		packet->requestID = plugin->ActionID;
	}
	else
	{
		packet->requestID = plugin->ActionID;
	}

	packet->cmd = enum_act;
	return packet;
}

AGENT_SEND_STATUS SAManager_SendMessage( HANDLE const handle, int enum_act, 
									   void const * const requestData, unsigned int const requestLen, 
									   void *pRev1, void* pRev2 )
{
	AGENT_SEND_STATUS result = cagent_send_data_error;
	Handler_info* plugin = NULL;
	susiaccess_packet_body_t* packet = NULL;
	char topicStr[128] = {0};

	if(handle == NULL)
		return result;

	plugin = (Handler_info*)handle;

	if(plugin->agentInfo == NULL)
		return result;

	packet = SAManager_WrapReqPacket(plugin, enum_act, requestData, requestLen, false);

	if(packet == NULL)
	{
		SAManagerLog(g_samanagerlogger, Warning, "Request Packet is empty!");
		return result;
	}
	sprintf(topicStr, DEF_ACTIONREQ_TOPIC, plugin->agentInfo->devId);
	if(g_publishCB)
	{
		if(g_publishCB(topicStr, 0, 0, packet) == 0)
			result = cagent_success;
		else
			result = cagent_send_data_error;
	}
	else
		result = cagent_callback_null;

	if(packet->content)
		free(packet->content);
	free(packet);
	return result;
}

AGENT_SEND_STATUS SAManager_SendCustMessage( HANDLE const handle, int enum_act, char const * const topic, 
										   void const * const requestData, unsigned int const requestLen, 
										   void *pRev1, void* pRev2 )
{
	AGENT_SEND_STATUS result = cagent_send_data_error;
	Handler_info* plugin = NULL;
	susiaccess_packet_body_t* packet = NULL;

	//SAManagerLog(g_samanagerlogger, Normal, "Topic: %s, Data: %s", topic, requestData);
	//if(handle == NULL)
	//	return result;

	if(handle)
		plugin = (Handler_info*)handle;

	//if(plugin->agentInfo == NULL)
	//	return result;

	packet = SAManager_WrapReqPacket(plugin, enum_act, requestData, requestLen, true);

	if(packet == NULL)
	{
		SAManagerLog(g_samanagerlogger, Warning, "Request Packet is empty!");
		return result;
	}

	if(g_publishCB)
	{
		if(g_publishCB(topic, 0, 0, packet) == 0)
			result = cagent_success;
		else
			result = cagent_send_data_error;
	}
	else
		result = cagent_callback_null;
	if(packet->content)
		free(packet->content);
	free(packet);
	return result;
}

susiaccess_packet_body_t * SAManager_WrapAutoReportPacket(Handler_info const * plugin, void const * const requestData, unsigned int const requestLen, bool bCust)
{
	susiaccess_packet_body_t* packet = NULL;

	cJSON* oproot = NULL;
	cJSON* node = NULL;
	cJSON* root = NULL;
	cJSON* pfinfoNode = NULL;
	char* buff = NULL;
	char* data = NULL;
	int length = 0;
	time_t t = time(NULL);

	if(plugin == NULL)
		return packet;
	if(plugin->agentInfo == NULL)
		return packet;

	if(requestData)
	{
		data = ConvertMessageToUTF8((char*)requestData);
	}
	else
		return packet;

	root = cJSON_CreateObject();
	pfinfoNode = cJSON_CreateObject();
	//node = cJSON_Parse((const char *)requestData);
	node = cJSON_Parse((const char *)data);
	free(data);
	
	if(node)
	{
		cJSON* chNode = node->child;
		if(chNode)
			cJSON_AddItemToObject(pfinfoNode, chNode->string, cJSON_Duplicate(chNode, true));	
	}
	cJSON_Delete(node);
	cJSON_AddItemToObject(root, "data", pfinfoNode);

	oproot = cJSON_CreateObject();
	if(oproot)
	{
		cJSON_AddNumberToObject(oproot,"$date",(unsigned long long)t*1000);
	}
	if(root)
	{		
		if(root->child)
		{
			if(root->child->child)
			{
				cJSON_AddItemToObject(root->child->child,"opTS", cJSON_Duplicate(oproot, 1));
			}
		}	
	}
	cJSON_Delete(oproot);

	buff = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	length = strlen(buff);
	
	packet = malloc(sizeof(susiaccess_packet_body_t));
	memset(packet, 0, sizeof(susiaccess_packet_body_t));
	packet->content = (char*)malloc(length+1);
	memset(packet->content, 0, length+1);
	memcpy(packet->content, buff, length);

	free(buff);

	strcpy(packet->devId, plugin->agentInfo->devId);
	strcpy(packet->handlerName, "general");  //Special case for auto report.

	packet->requestID = cagent_action_general;

	packet->cmd = general_info_upload_rep;
	//SAManagerLog(g_samanagerlogger, Normal, "Parser_CreateRequestInfo");

	return packet;
}

void SAManager_Redirect(HANDLE const handle, susiaccess_packet_body_t * packet)
{
	struct samanager_topic_entry * topicentry = NULL;
	Handler_info* plugin = NULL;
	char* buff = NULL;

	if(handle == NULL || packet == NULL)
		return;

	plugin = (Handler_info*)handle;

	if(plugin->agentInfo == NULL)
		return;

	buff = pkg_parser_packet_print(packet);
	if(buff)
	{
		topicentry = samanager_topic_find(plugin->Name);
		
		if(topicentry != NULL)
		{
			topicentry->callback_func(plugin->Name, buff, strlen(buff), NULL, NULL);
		}
		free(buff);
	}
}

AGENT_SEND_STATUS SAManager_SendAutoReport( HANDLE const handle, 
										  void const * const requestData, unsigned int const requestLen, 
										  void *pRev1, void* pRev2 )
{
	AGENT_SEND_STATUS result = cagent_send_data_error;
	Handler_info* plugin = NULL;
	susiaccess_packet_body_t* packet = NULL;
	char topicStr[128] = {0};

	if(handle == NULL)
		return result;

	plugin = (Handler_info*)handle;

	if(plugin->agentInfo == NULL)
		return result;

	packet = SAManager_WrapAutoReportPacket(plugin, requestData, requestLen, false);

	if(packet == NULL)
	{
		SAManagerLog(g_samanagerlogger, Warning, "Request Packet is empty!");
		return result;
	}

	sprintf(topicStr, DEF_AUTOREPORT_TOPIC, plugin->agentInfo->devId, AUTOREPORT_TOPIC);

	if(g_publishCB)
	{
		if(g_publishCB(topicStr, 0, 0, packet) == 0)
			result = cagent_success;
		else
			result = cagent_send_data_error;
	}
	else
		result = cagent_callback_null;

	if(g_SADataSync)
		if(g_SADataSync->DataSync_Insert_Rep_API)
			g_SADataSync->DataSync_Insert_Rep_API(handle,packet->content,topicStr, result);

	SAManager_Redirect(handle, packet);

	if(packet->content)
		free(packet->content);
	free(packet);
	return result;
}

susiaccess_packet_body_t * SAManager_WrapCapabilityPacket(Handler_info const * plugin, void const * const requestData, unsigned int const requestLen, bool bCust)
{
	susiaccess_packet_body_t* packet = NULL;

	cJSON* oproot = NULL;
	cJSON* node = NULL;
	cJSON* root = NULL;
	cJSON* pfinfoNode = NULL;
	char* buff = NULL;
	char* data = NULL;
	int length = 0;
	time_t t = time(NULL);

	if(plugin == NULL)
		return packet;
	if(plugin->agentInfo == NULL)
		return packet;

	if(requestData)
	{
		data = ConvertMessageToUTF8((char*)requestData);
	}
	else
		return packet;

	root = cJSON_CreateObject();
	pfinfoNode = cJSON_CreateObject();
	//node = cJSON_Parse((const char *)requestData);
	node = cJSON_Parse((const char *)data);
	free(data);
	if(node)
	{
		cJSON* chNode = node->child;
		if(chNode)
			cJSON_AddItemToObject(pfinfoNode, chNode->string, cJSON_Duplicate(chNode, true));	
	}
	cJSON_Delete(node);
	cJSON_AddItemToObject(root, "infoSpec", pfinfoNode);

	oproot = cJSON_CreateObject();
	if(oproot)
	{
		cJSON_AddNumberToObject(oproot,"$date",(unsigned long long)t*1000);
	}
	if(root)
	{
		if(root->child)
			if(root->child->child)
				cJSON_AddItemToObject(root->child->child,"opTS",oproot);
	}

	buff = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	length = strlen(buff);

	packet = malloc(sizeof(susiaccess_packet_body_t));
	memset(packet, 0, sizeof(susiaccess_packet_body_t));
	packet->content = (char*)malloc(length+1);
	memset(packet->content, 0, length+1);
	memcpy(packet->content, buff, length);

	free(buff);

	strcpy(packet->devId, plugin->agentInfo->devId);
	strcpy(packet->handlerName, "general");  //Special case for auto report.

	packet->requestID = cagent_action_general;

	packet->cmd = general_info_spec_rep;
	//SAManagerLog(g_samanagerlogger, Normal, "Parser_CreateRequestInfo");

	return packet;
}

AGENT_SEND_STATUS SAManager_SendCapability( HANDLE const handle, 
										 void const * const requestData, unsigned int const requestLen, 
										 void *pRev1, void* pRev2 )
{
	AGENT_SEND_STATUS result = cagent_send_data_error;
	Handler_info* plugin = NULL;
	susiaccess_packet_body_t* packet = NULL;
	char topicStr[128] = {0};

	if(handle == NULL)
		return result;

	plugin = (Handler_info*)handle;

	if(plugin->agentInfo == NULL)
		return result;
	
	packet = SAManager_WrapCapabilityPacket(handle, requestData, requestLen, false);

	if(packet == NULL)
	{
		SAManagerLog(g_samanagerlogger, Warning, "Request Packet is empty!");
		return result;
	}
	sprintf(topicStr, DEF_ACTIONREQ_TOPIC, plugin->agentInfo->devId);

	if(g_publishCB)
	{
		if(g_publishCB(topicStr, 0, 0, packet) == 0)
			result = cagent_success;
		else
			result = cagent_send_data_error;
	}
	else
		result = cagent_callback_null;

	if(g_SADataSync)
		if(g_SADataSync->DataSync_Insert_Cap_API)
			g_SADataSync->DataSync_Insert_Cap_API(handle,packet->content,topicStr, result);

	if(packet->content)
		free(packet->content);
	free(packet);
	return result;
}

susiaccess_packet_body_t * SAManager_WrapEventNotifyPacket(Handler_info const * plugin, HANDLER_NOTIFY_SEVERITY severity, void const * const requestData, unsigned int const requestLen, bool bCust)
{
	char* data = NULL; 
	susiaccess_packet_body_t* packet = NULL;

	cJSON* node = NULL;
	cJSON* root = NULL;
	cJSON* pfinfoNode = NULL;
	char* buff = NULL;
	int length = 0;

	if(plugin == NULL)
		return packet;
	if(plugin->agentInfo == NULL)
		return packet;

	if(requestData)
	{
		data = ConvertMessageToUTF8((char*)requestData);
	}
	else
		return packet;

	root = cJSON_CreateObject();
	pfinfoNode = cJSON_CreateObject();
	//node = cJSON_Parse((const char *)requestData);
	node = cJSON_Parse((const char *)data);
	free(data);
	if(node)
	{
		cJSON* chNode = node->child;
		while(chNode)
		{
			cJSON_AddItemToObject(pfinfoNode, chNode->string, cJSON_Duplicate(chNode, true));	
			chNode = chNode->next;
		}
		cJSON_AddNumberToObject(pfinfoNode, "severity", severity);
		cJSON_AddStringToObject(pfinfoNode, "handler", plugin->Name);
	}
	cJSON_Delete(node);
	cJSON_AddItemToObject(root, "eventnotify", pfinfoNode);
	buff = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	length = strlen(buff);

	packet = malloc(sizeof(susiaccess_packet_body_t));
	memset(packet, 0, sizeof(susiaccess_packet_body_t));
	packet->content = (char*)malloc(length+1);
	memset(packet->content, 0, length+1);
	memcpy(packet->content, buff, length);

	free(buff);

	strcpy(packet->devId, plugin->agentInfo->devId);
	strcpy(packet->handlerName, "general");  //Special case for event notify.

	packet->requestID = cagent_action_general;

	packet->cmd = general_event_notify_rep;
	//SAManagerLog(g_samanagerlogger, Normal, "Parser_CreateRequestInfo");

	return packet;
}

AGENT_SEND_STATUS SAManager_SendEventNotify( HANDLE const handle, HANDLER_NOTIFY_SEVERITY severity,
										 void const * const requestData, unsigned int const requestLen, 
										 void *pRev1, void* pRev2 )
{
	AGENT_SEND_STATUS result = cagent_send_data_error;
	Handler_info* plugin = NULL;
	susiaccess_packet_body_t* packet = NULL;
	char topicStr[128] = {0};

	if(handle == NULL)
		return result;

	plugin = (Handler_info*)handle;

	if(plugin->agentInfo == NULL)
		return result;

	packet = SAManager_WrapEventNotifyPacket(handle, severity, requestData, requestLen, false);

	if(packet == NULL)
	{
		SAManagerLog(g_samanagerlogger, Warning, "Request Packet is empty!");
		return result;
	}
	sprintf(topicStr, DEF_EVENTNOTIFY_TOPIC, plugin->agentInfo->devId);
	if(g_publishCB)
	{
		if(g_publishCB(topicStr, 0, 0, packet) == 0)
			result = cagent_success;
		else
			result = cagent_send_data_error;
	}
	else
		result = cagent_callback_null;

	if(packet->content)
		free(packet->content);
	free(packet);
	return result;
}

AGENT_SEND_STATUS SAManager_ConnectServer(char const * ip, int port, char const * mqttauth, tls_type tlstype, char const * psk)
{
	AGENT_SEND_STATUS result = cagent_send_data_error;
/*
	susiaccess_agent_conf_body_t * pConfig = NULL;
	susiaccess_agent_profile_body_t * pProfile = NULL;

	pConfig = malloc(sizeof(susiaccess_agent_conf_body_t));
	if(pConfig)
	{
		memset(pConfig, 0, sizeof(susiaccess_agent_conf_body_t));
		if(g_pConfig)
			memcpy(pConfig, g_pConfig, sizeof(susiaccess_agent_conf_body_t));

		if(ip)
		{
			strncpy(pConfig->serverIP, ip, strlen(ip)+1);
		}

		if(port>0)
		{
			sprintf(pConfig->serverPort, "%d", port);
		}

		if(mqttauth)
		{
			strncpy(pConfig->serverAuth, mqttauth, strlen(mqttauth)+1);
		}

		pConfig->tlstype = tlstype;

		if(psk)
		{
			strncpy(pConfig->psk, psk, strlen(psk)+1);
		}
	}

	if(g_pProfile)
	{
		pProfile = malloc(sizeof(susiaccess_agent_profile_body_t));
		if(pProfile)
		{
			memset(pProfile, 0, sizeof(susiaccess_agent_profile_body_t));
			memcpy(pProfile, g_pProfile, sizeof(susiaccess_agent_profile_body_t));
		}
	}
*/
	if(g_connectserverCB)
	{
	
		//if(g_connectserverCB(pConfig, pProfile) == 0)
		if(g_connectserverCB(ip, port, mqttauth, tlstype, psk) == 0)
			result = cagent_success;
		else
			result = cagent_connect_error;
	}
	else
		result = cagent_callback_null;
/*
	if(pConfig)
		free(pConfig);

	if(pProfile)
		free(pProfile);
*/
	return result;
}

AGENT_SEND_STATUS SAManager_Disconnect()
{
	AGENT_SEND_STATUS result = cagent_callback_null;
	if(g_disconnectCB)
		g_disconnectCB();
	else
		result = cagent_callback_null;
	return result;
}

AGENT_SEND_STATUS SAManager_Rename(char const * name)
{
	if(g_pProfile)
		strncpy(g_pProfile->hostname, name, sizeof(g_pProfile->hostname));
	/*if(g_pSALoader)
	{
		hlloader_handler_agent_status_update(g_pSALoader, &g_handlerList, g_pConfig, g_pProfile, g_iConnStatus);
	}*/
	return cagent_success;
}

AGENT_SEND_STATUS SAManager_SendOSInfo()
{
	bool bRet = false;
	if(g_sendosinfoCB)
		bRet = g_sendosinfoCB();
	return bRet?cagent_success:cagent_send_data_error;
}

AGENT_SEND_STATUS SAManager_AddVirtualHandler(char *VirName, char *HandlerName)
{
	hlloader_load_virtualhandler(g_pSALoader, &g_handlerList,VirName,HandlerName);
}

void SAManager_RecvInternalCommandReq(char* topic, susiaccess_packet_body_t *pkt, void *pRev1, void* pRev2)
{
	if(g_pSALoader)
	{
		hlloader_handler_recv(g_pSALoader,&g_handlerList, topic, pkt, pRev1, pRev2);
	}
}

void SAManager_CustMessageRecv(char* topic, susiaccess_packet_body_t *pkt, void *pRev1, void* pRev2)
{
	struct samanager_topic_entry * topicentry = samanager_topic_find(topic);
	
	if(topicentry != NULL)
	{
		char* pReqInfoPayload = NULL;
		int length = 0;
		if(pkt->type == pkt_type_custom)
			pReqInfoPayload = strdup(pkt->content);
		else
			pReqInfoPayload = pkg_parser_packet_print(pkt);
		length = strlen(pReqInfoPayload);
		topicentry->callback_func(topic, pReqInfoPayload, length, NULL, NULL);
		free(pReqInfoPayload);
	}
}

AGENT_SEND_STATUS SAManager_SubscribeCustTopic(char const * const topic, HandlerCustMessageRecvCbf recvCbf)
{
	struct samanager_topic_entry *topicentry = NULL;
	if(!g_subscribeCB)
		return cagent_callback_null;

	if(g_subscribeCB(topic, 1, SAManager_CustMessageRecv) == 0/*saclient_success*/)
	{
		topicentry = samanager_topic_find(topic);
		if(topicentry == NULL)
		{
			samanager_topic_add(topic, (samanager_topic_msg_cb_func_t)recvCbf);
		}	
		else
			topicentry->callback_func = (samanager_topic_msg_cb_func_t)recvCbf;
		SAManagerLog(g_samanagerlogger, Debug, "Subscribe Custom Topic: %s", topic);
	}
	else
		SAManagerLog(g_samanagerlogger, Warning, "Subscribe Custom Topic Fail: %s", topic);
	return cagent_success;
}

void SAMANAGER_API SAManager_Initialize(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile, void * loghandle)
{
	char workdir[MAX_PATH] = {0};
	Handler_Loader_Interface GlobalPlugin;
	g_samanagerlogger = loghandle;

	/*
	g_pConfig = malloc(sizeof(susiaccess_agent_conf_body_t));
	memset(g_pConfig, 0, sizeof(susiaccess_agent_conf_body_t));
	memcpy(g_pConfig, config, sizeof(susiaccess_agent_conf_body_t));
	g_pProfile = malloc(sizeof(susiaccess_agent_profile_body_t));
	memset(g_pProfile, 0, sizeof(susiaccess_agent_profile_body_t));
	memcpy(g_pProfile, profile, sizeof(susiaccess_agent_profile_body_t));
    */

	g_pConfig = config;
	g_pProfile = profile;

	if(strlen(profile->workdir)>0)
		strcpy(workdir, profile->workdir);
	else
		util_module_path_get(workdir);

	memset(&g_handlerList, 0, sizeof(Handler_List_t));

	keepalive_initialize(&g_handlerList, workdir, g_samanagerlogger);
	/*Load Handler Loader*/	
	g_pSALoader = hlloader_initialize(workdir, config, profile, loghandle);
	if(g_pSALoader)
	{
		Callback_Functions_t functions;
		memset(&functions, 0, sizeof(Callback_Functions_t));
		functions.sendcbf = SAManager_SendMessage;
		functions.sendcustcbf = SAManager_SendCustMessage;
		functions.subscribecustcbf = SAManager_SubscribeCustTopic;
		functions.sendreportcbf = SAManager_SendAutoReport;
		functions.sendcapabilitycbf = SAManager_SendCapability;
		functions.sendevnetcbf = SAManager_SendEventNotify;
		functions.connectservercbf = SAManager_ConnectServer;
		functions.disconnectcbf = SAManager_Disconnect;
		functions.renamecbf = SAManager_Rename;
		functions.sendosinfocbf = SAManager_SendOSInfo;
		functions.addvirtualhandlercbf = SAManager_AddVirtualHandler;
		hlloader_cbfunc_set(g_pSALoader, &functions);
	}
	/*get initialized Handler_Loader_Interface*/
	memset(&GlobalPlugin, 0, sizeof(Handler_Loader_Interface));
	if(g_pSALoader)
	{
		hlloader_basic_handlerinfo_get(g_pSALoader, &GlobalPlugin);
	}

	g_SAGeneral = ghloader_initialize(workdir, config, &g_handlerList, &GlobalPlugin, loghandle);

	if(g_pSALoader)
	{
		if(g_SAGeneral)
		{
			hlloader_handlerinfo_add(g_pSALoader, &g_handlerList, &GlobalPlugin);
		}
		hlloader_handler_load(g_pSALoader, &g_handlerList, workdir);

		g_SADataSync=SADataSync_Initialize(workdir,loghandle);
		if(g_SADataSync)
			if(g_SADataSync->DataSync_Initialize_API)
				g_SADataSync->DataSync_Initialize_API(workdir,&g_handlerList,loghandle);

		hlloader_handler_start(g_pSALoader, &g_handlerList);
	}
}

void SAMANAGER_API SAManager_Uninitialize()
{
	struct samanager_topic_entry *iter_topic = NULL;
	struct samanager_topic_entry *tmp_topic = NULL;

	keepalive_uninitialize();

	iter_topic = samanager_topic_first();
	while(iter_topic != NULL)
	{
		tmp_topic = iter_topic->next;
		samanager_topic_remove(iter_topic->name);
		iter_topic = tmp_topic;
	}

	/*Release Handler Loader*/	
	if(g_pSALoader)
	{
		hlloader_handler_release(g_pSALoader, &g_handlerList);
		hlloader_uninitialize(g_pSALoader);
		g_pSALoader = NULL;
	}
	
	if(g_SAGeneral)
	{
		ghloader_uninitialize(g_SAGeneral);
		g_SAGeneral = NULL;
	}
/*
	if(g_pConfig)
	{
		free(g_pConfig);
		g_pConfig = NULL;
	}

	if(g_pProfile)
	{
		free(g_pProfile);
		g_pProfile = NULL;
	}
*/

	if(g_SADataSync)
		if(g_SADataSync->DataSync_Uninitialize_API)
			g_SADataSync->DataSync_Uninitialize_API();
	if(g_SADataSync)
		if(SADataSync_Uninitialize(g_SADataSync))
			g_SADataSync=NULL;
}

void SAMANAGER_API SAManager_SetPublishCB(PUBLISHCB func)
{
	g_publishCB = func;
	if(g_SADataSync)
		if(g_SADataSync->DataSync_SetFuncCB_API)
			g_SADataSync->DataSync_SetFuncCB_API(g_publishCB);
}

void SAMANAGER_API SAManager_SetSubscribeCB(SUBSCRIBECB func)
{
	g_subscribeCB = func;
}

void SAMANAGER_API SAManager_SetConnectServerCB(CONNECTSERVERCB func)
{
	g_connectserverCB = func;
}

void SAMANAGER_API SAManager_SetDisconnectCB(DISCONNECTCB func)
{
	g_disconnectCB = func;
}

void SAMANAGER_API SAManager_SetOSInfoSendCB(SENDOSINFOCB func)
{
	g_sendosinfoCB = func;
}

void SAMANAGER_API SAManager_InternalSubscribe()
{
	/* Add Topic Callback*/
	char topicStr[128] = {0};
	if(!g_pProfile)
		return;
	sprintf(topicStr, DEF_CALLBACKREQ_TOPIC, g_pProfile->devId);
	if(g_subscribeCB)
		g_subscribeCB(topicStr, 1, SAManager_RecvInternalCommandReq);

	memset(topicStr, 0, sizeof(topicStr));
	sprintf(topicStr, DEF_ACTIONACK_TOPIC, g_pProfile->devId);
	if(g_subscribeCB)
		g_subscribeCB(topicStr, 1, SAManager_RecvInternalCommandReq);

	/* Add Server Broadcast Topic Callback*/
	if(g_subscribeCB)
		g_subscribeCB(DEF_AGENTCONTROL_TOPIC, 2, SAManager_RecvInternalCommandReq);
}

void SAMANAGER_API SAManager_UpdateConnectState(int status)
{
	//if(g_iConnStatus == status)  /*Server redundancy need to count how many times connect failed!*/
	//	return;

	time_t t=time(NULL);
	if(g_iConnStatus==1 && status!=1)
		if(g_SADataSync)
			if(g_SADataSync->DataSync_Set_LostTime_API)
				g_SADataSync->DataSync_Set_LostTime_API((unsigned long long)t);
	

	if(g_iConnStatus!=1 && status==1) // 0 -> 1 or 2 -> 1
	{
		if(g_SADataSync)
		{
			if(g_SADataSync->DataSync_Set_ReConTime_API)
				g_SADataSync->DataSync_Set_ReConTime_API((unsigned long long)t);
		}
	}	


	g_iConnStatus = status;

	/*if(conf)
	{
		if(!g_pConfig)
		{
			g_pConfig = malloc(sizeof(susiaccess_agent_conf_body_t));
		}
		memset(g_pConfig, 0, sizeof(susiaccess_agent_conf_body_t));
		memcpy(g_pConfig, conf, sizeof(susiaccess_agent_conf_body_t));

	}

	if(profile)
	{
		if(!g_pProfile)
		{
			g_pProfile = malloc(sizeof(susiaccess_agent_profile_body_t));
		}
		memset(g_pProfile, 0, sizeof(susiaccess_agent_profile_body_t));
		memcpy(g_pProfile, profile, sizeof(susiaccess_agent_profile_body_t));
	}*/


	if(g_pSALoader)
	{
		hlloader_handler_agent_status_update(g_pSALoader, &g_handlerList, g_pConfig, g_pProfile, status);
	}
}
