#include "DeviceMessageGenerate.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAG_SUSICOMM_ROOT				"susiCommData"
#define TAG_SUSICOMM_CMD				"commCmd"
#define TAG_SUSICOMM_REQID				"requestID"
#define TAG_SUSICOMM_AGENTID			"agentID"
#define TAG_SUSICOMM_HANDLER_NAME		"handlerName"
#define TAG_SUSICOMM_TIMESTAMP			"sendTS"

#define TAG_AGENTINFO_DEVID				"devID"
#define TAG_AGENTINFO_HOSTNAME			"hostname"
#define TAG_AGENTINFO_SERIAL			"sn"
#define TAG_AGENTINFO_MAC				"mac"
#define TAG_AGENTINFO_SW_VERSION		"version"
#define TAG_AGENTINFO_TYPE				"type"
#define TAG_AGENTINFO_PRODUCT_NAME		"product"
#define TAG_AGENTINFO_MANUFACTURE		"manufacture"
#define TAG_AGENTINFO_LOGIN_ACCOUNT		"account"
#define TAG_AGENTINFO_LOGIN_PASSWORD	"password"
#define TAG_AGENTINFO_CONNECT_STATUS	"status"

#define TAG_OSINFO_ROOT					"osInfo"
#define TAG_OSINFO_AGENT_VERSION		"cagentVersion"
#define TAG_OSINFO_OS_VERSION			"osVersion"
#define TAG_OSINFO_BIOS_VERSION			"biosVersion"
#define TAG_OSINFO_PLATFROM_NAME		"platformName"
#define TAG_OSINFO_PROCESSOR_NAME		"processorName"
#define TAG_OSINFO_OS_ARCH				"osArch"
#define TAG_OSINFO_TOTAL_MEMORY			"totalPhysMemKB"
#define TAG_OSINFO_MAC_LIST				"macs"
#define TAG_OSINFO_LOCAL_IP				"IP"

#define TAG_HANDLER_LIST				"handlerlist"

#define TAG_EVENT_NOTIFY				"eventnotify"
#define TAG_EVENT_SUBTYPE				"subtype"
#define TAG_EVENT_MESSAGE				"msg"
#define TAG_EVENT_SEVERITY				"severity"
#define TAG_EVENT_HANDLER				"handler"

#define DEF_INFOACK_TOPIC				"/cagent/admin/%s/agentinfoack"		/*publish*/
#define DEF_WILLMSG_TOPIC				"/cagent/admin/%s/willmessage"		/*publish*/
#define DEF_ACTIONREQ_TOPIC				"/cagent/admin/%s/agentactionreq"	/*publish*/
#define DEF_EVENTNOTIFY_TOPIC			"/cagent/admin/%s/eventnotify"		/*publish*/

#pragma region Add_Resource

MSG_CLASSIFY_T* DEV_CreateAgentInfoBody(MSG_CLASSIFY_T* pSusiComm, susiaccess_agent_profile_body_t* profile)
{
	MSG_ATTRIBUTE_T *attr;

	if(!profile || !pSusiComm)
		return pSusiComm;

	if(pSusiComm)
	{
		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_DEVID);
		if(attr)
			MSG_SetStringValue(attr, profile->devId, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_HOSTNAME);
		if(attr)
			MSG_SetStringValue(attr, profile->hostname, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_SERIAL);
		if(attr)
			MSG_SetStringValue(attr, profile->sn, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_MAC);
		if(attr)
			MSG_SetStringValue(attr, profile->mac, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_SW_VERSION);
		if(attr)
			MSG_SetStringValue(attr, profile->version, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_TYPE);
		if(attr)
			MSG_SetStringValue(attr, profile->type, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_PRODUCT_NAME);
		if(attr)
			MSG_SetStringValue(attr, profile->product, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_MANUFACTURE);
		if(attr)
			MSG_SetStringValue(attr, profile->manufacture, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_LOGIN_ACCOUNT);
		if(attr)
			MSG_SetStringValue(attr, profile->account, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_LOGIN_PASSWORD);
		if(attr)
			MSG_SetStringValue(attr, profile->passwd, NULL);
	}
	return pSusiComm;
}

MSG_CLASSIFY_T* DEV_CreateAgentInfo(susiaccess_agent_profile_body_t* profile)
{
	MSG_CLASSIFY_T *pRoot = MSG_CreateRoot();
	MSG_CLASSIFY_T *pSusiComm;
	long long tick = 0;
	MSG_ATTRIBUTE_T *attr;
	if(pRoot)
	{
		pSusiComm = MSG_AddJSONClassify(pRoot, TAG_SUSICOMM_ROOT, NULL, false);

		DEV_CreateAgentInfoBody(pSusiComm, profile);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_CONNECT_STATUS);
		if(attr)
			MSG_SetFloatValue(attr, 1, NULL, NULL); //connected

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_CMD);
		if(attr)
			MSG_SetFloatValue(attr, 1, NULL, NULL); //fixed to 1

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_REQID);
		if(attr)
			MSG_SetFloatValue(attr, 21, NULL, NULL); //fixed to 21

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_AGENTID);
		if(attr)
			MSG_SetStringValue(attr, profile->devId, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_HANDLER_NAME);
		if(attr)
			MSG_SetStringValue(attr, "general", NULL); //fixed to 'general'

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_TIMESTAMP);
		if(attr)
		{
			tick = (long) time((time_t *) NULL);
			MSG_SetTimestampValue(attr, tick, NULL);
		}
	}
	return pRoot;
}

char *DEV_GetAgentInfoTopic(char* devID)
{
	char *topicStr = NULL;
	if(!devID)
		return topicStr; 
	topicStr = malloc(128);
	if(topicStr)
		sprintf(topicStr, DEF_INFOACK_TOPIC, devID);
	return topicStr;
}

MSG_CLASSIFY_T* DEV_CreateWillMessage(susiaccess_agent_profile_body_t* profile)
{
	MSG_CLASSIFY_T *pRoot = MSG_CreateRoot();
	MSG_CLASSIFY_T *pSusiComm;
	long long tick = 0;
	MSG_ATTRIBUTE_T *attr;
	if(pRoot)
	{
		pSusiComm = MSG_AddJSONClassify(pRoot, TAG_SUSICOMM_ROOT, NULL, false);

		DEV_CreateAgentInfoBody(pSusiComm, profile);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_AGENTINFO_CONNECT_STATUS);
		if(attr)
			MSG_SetFloatValue(attr, 0, NULL, NULL); //disconnected

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_CMD);
		if(attr)
			MSG_SetFloatValue(attr, 1, NULL, NULL); //fixed to 1

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_REQID);
		if(attr)
			MSG_SetFloatValue(attr, 21, NULL, NULL); //fixed to 21

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_AGENTID);
		if(attr)
			MSG_SetStringValue(attr, profile->devId, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_HANDLER_NAME);
		if(attr)
			MSG_SetStringValue(attr, "general", NULL); //fixed to 'general'

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_TIMESTAMP);
		if(attr)
		{
			tick = (long long)time((time_t *) NULL);
			MSG_SetTimestampValue(attr, tick, NULL);
		}
	}
	return pRoot;
}

char *DEV_GetWillMessageTopic(char* devID)
{
	char *topicStr = NULL;
	if(!devID)
		return topicStr; 
	topicStr = malloc(128);
	if(topicStr)
		sprintf(topicStr, DEF_WILLMSG_TOPIC, devID);
	return topicStr;
}

MSG_CLASSIFY_T* DEV_CreateOSInfo(susiaccess_agent_profile_body_t* profile)
{
	MSG_CLASSIFY_T *pRoot = MSG_CreateRoot();
	MSG_CLASSIFY_T *pSusiComm, *pOSInfo = NULL;
	long long tick = 0;
	MSG_ATTRIBUTE_T *attr;
	if(pRoot)
	{
		pSusiComm = MSG_AddJSONClassify(pRoot, TAG_SUSICOMM_ROOT, NULL, false);

		pOSInfo = MSG_AddJSONClassify(pSusiComm, TAG_OSINFO_ROOT, NULL, false);

		attr = MSG_AddJSONAttribute(pOSInfo, TAG_OSINFO_AGENT_VERSION);
		if(attr)
			MSG_SetStringValue(attr, profile->version, NULL);

		attr = MSG_AddJSONAttribute(pOSInfo, TAG_OSINFO_OS_VERSION);
		if(attr)
			MSG_SetStringValue(attr, profile->osversion, NULL);

		attr = MSG_AddJSONAttribute(pOSInfo, TAG_OSINFO_BIOS_VERSION);
		if(attr)
			MSG_SetStringValue(attr, profile->biosversion, NULL);

		attr = MSG_AddJSONAttribute(pOSInfo, TAG_OSINFO_PLATFROM_NAME);
		if(attr)
			MSG_SetStringValue(attr, profile->platformname, NULL);

		attr = MSG_AddJSONAttribute(pOSInfo, TAG_OSINFO_PROCESSOR_NAME);
		if(attr)
			MSG_SetStringValue(attr, profile->processorname, NULL);

		attr = MSG_AddJSONAttribute(pOSInfo, TAG_OSINFO_OS_ARCH);
		if(attr)
			MSG_SetStringValue(attr, profile->osarchitect, NULL);
		
		attr = MSG_AddJSONAttribute(pOSInfo, TAG_OSINFO_TOTAL_MEMORY);
		if(attr)
			MSG_SetDoubleValue(attr, profile->totalmemsize, NULL, NULL); //fixed to 116

		attr = MSG_AddJSONAttribute(pOSInfo, TAG_OSINFO_MAC_LIST);
		if(attr)
			MSG_SetStringValue(attr, profile->maclist, NULL);

		attr = MSG_AddJSONAttribute(pOSInfo, TAG_OSINFO_LOCAL_IP);
		if(attr)
			MSG_SetStringValue(attr, profile->localip, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_CMD);
		if(attr)
			MSG_SetFloatValue(attr, 116, NULL, NULL); //fixed to 116

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_REQID);
		if(attr)
			MSG_SetFloatValue(attr, 161, NULL, NULL); //fixed to 16

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_AGENTID);
		if(attr)
			MSG_SetStringValue(attr, profile->devId, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_HANDLER_NAME);
		if(attr)
			MSG_SetStringValue(attr, "general", NULL); //fixed to 'general'

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_TIMESTAMP);
		if(attr)
		{
			tick = (long long)time((time_t *) NULL);
			MSG_SetTimestampValue(attr, tick, NULL);
		}
	}
	return pRoot;
}

char *DEV_GetActionReqTopic(char* devID)
{
	char *topicStr = NULL;
	if(!devID)
		return topicStr; 
	topicStr = malloc(128);
	if(topicStr)
		sprintf(topicStr, DEF_ACTIONREQ_TOPIC, devID);
	return topicStr;
}

MSG_CLASSIFY_T* DEV_CreateHandlerList(char* devID, char** handldelist, int count)
{
	MSG_CLASSIFY_T *pRoot = MSG_CreateRoot();
	MSG_CLASSIFY_T *pSusiComm, *pHandlerList = NULL;
	long long tick = 0;
	int i=0;
	MSG_ATTRIBUTE_T *attr;
	if(pRoot)
	{
		pSusiComm = MSG_AddJSONClassify(pRoot, TAG_SUSICOMM_ROOT, NULL, false);

		pHandlerList = MSG_AddJSONClassify(pSusiComm, TAG_HANDLER_LIST, NULL, true);
		for(i=0; i<count; i++)
		{
			attr = MSG_AddJSONAttribute(pHandlerList, handldelist[i]);
			MSG_SetStringValue(attr, handldelist[i], NULL); //fixed to 'general'
		}

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_CMD);
		if(attr)
			MSG_SetFloatValue(attr, 124, NULL, NULL); //fixed to 124

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_REQID);
		if(attr)
			MSG_SetFloatValue(attr, 1001, NULL, NULL); //fixed to 1001

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_AGENTID);
		if(attr)
			MSG_SetStringValue(attr, devID, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_HANDLER_NAME);
		if(attr)
			MSG_SetStringValue(attr, "general", NULL); //fixed to 'general'

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_TIMESTAMP);
		if(attr)
		{
			tick = (long long)time((time_t *) NULL);
			MSG_SetTimestampValue(attr, tick, NULL);
		}
	}
	return pRoot;
}

void CreateEventBody(MSG_CLASSIFY_T* pRoot, char* subtype, char* message)
{
	MSG_ATTRIBUTE_T *attr = NULL;
	if(!pRoot)
		return;

	if(subtype)
	{
		attr = MSG_AddJSONAttribute(pRoot, TAG_EVENT_SUBTYPE);
		if(attr)
			MSG_SetStringValue(attr, subtype, NULL); 
	}

	if(message)
	{
		attr = MSG_AddJSONAttribute(pRoot, TAG_EVENT_MESSAGE);
		if(attr)
			MSG_SetStringValue(attr, message, NULL); 
	}
}

MSG_CLASSIFY_T* DEV_CreateEventNotify(char* subtype, char* message)
{
	MSG_CLASSIFY_T *pRoot = MSG_CreateRoot();
	if(!pRoot)
		return pRoot;

	CreateEventBody(pRoot, subtype, message);

	return pRoot;
}

MSG_CLASSIFY_T* DEV_CreateFullEventNotify(char* devID, int severity, char* handler, char* subtype, char* message)
{
	MSG_CLASSIFY_T *pRoot = MSG_CreateRoot();
	MSG_CLASSIFY_T *pSusiComm, *pEventNotify = NULL;
	long long tick = 0;

	MSG_ATTRIBUTE_T *attr;
	if(pRoot)
	{
		pSusiComm = MSG_AddJSONClassify(pRoot, TAG_SUSICOMM_ROOT, NULL, false);

		pEventNotify = MSG_AddJSONClassify(pSusiComm, TAG_EVENT_NOTIFY, NULL, false);

		CreateEventBody(pEventNotify, subtype, message);

		attr = MSG_AddJSONAttribute(pEventNotify, TAG_EVENT_SEVERITY);
		if(attr)
			MSG_SetDoubleValue(attr, severity, NULL, NULL);

		if(handler)
		{
			attr = MSG_AddJSONAttribute(pEventNotify, TAG_EVENT_HANDLER);
			if(attr)
				MSG_SetStringValue(attr, handler, NULL); 
		}

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_CMD);
		if(attr)
			MSG_SetFloatValue(attr, 2059, NULL, NULL); //fixed to 2059

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_REQID);
		if(attr)
			MSG_SetFloatValue(attr, 2001, NULL, NULL); //fixed to 2001

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_AGENTID);
		if(attr)
			MSG_SetStringValue(attr, devID, NULL);

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_HANDLER_NAME);
		if(attr)
			MSG_SetStringValue(attr, "general", NULL); //fixed to 'general'

		attr = MSG_AddJSONAttribute(pSusiComm, TAG_SUSICOMM_TIMESTAMP);
		if(attr)
		{
			tick = (long long)time((time_t *) NULL);
			MSG_SetTimestampValue(attr, tick, NULL);
		}
	}
	return pRoot;
}

char *DEV_GetEventNotifyTopic(char* devID)
{
	char *topicStr = NULL;
	if(!devID)
		return topicStr; 
	topicStr = malloc(128);
	if(topicStr)
		sprintf(topicStr, DEF_EVENTNOTIFY_TOPIC, devID);
	return topicStr;
}

#pragma endregion Add_Resource
#pragma region Release_Resource
void DEV_ReleaseDevice(MSG_CLASSIFY_T* node)
{
	MSG_ReleaseRoot(node);
}
#pragma endregion Release_Resource

#pragma region Find_Resource

#pragma endregion Find_Resource

#pragma region Generate_JSON 
char *DEV_PrintUnformatted(MSG_CLASSIFY_T* node)
{
	return MSG_PrintUnformatted(node);
}
#pragma endregion Generate_JSON 
