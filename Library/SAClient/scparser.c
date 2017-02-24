#include "scparser.h"
#include <stdlib.h>
#include <cJSON.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include "AdvPlatform.h" //for strdup wrapping
#include "util_string.h"

#define BASICINFO_BODY_STRUCT	"susiCommData"
#define BASICINFO_REQID			"requestID"
#define BASICINFO_HANDLERNAME	"handlerName"
#define BASICINFO_CMDTYPE		"commCmd"
#define BASICINFO_CATALOG		"catalogID"
#define BASICINFO_AGENTID		"agentID"
#define BASICINFO_TIMESTAMP		"sendTS"

#define AGENTINFO_DEVID			"devID"
#define AGENTINFO_HOSTNAME		"hostname"
#define AGENTINFO_SN			"sn"
#define AGENTINFO_MAC			"mac"
#define AGENTINFO_VERSION		"version"
#define AGENTINFO_TYPE			"type"
#define AGENTINFO_PRODUCT		"product"
#define AGENTINFO_MANUFACTURE	"manufacture"
#define AGENTINFO_STATUS		"status"
#define AGENTINFO_USERNAME		"account"
#define AGENTINFO_PASSWORD		"passwd"

#define GLOBAL_OS_INFO                   "osInfo"
#define GLOBAL_OS_VERSION                "osVersion"
#define GLOBAL_AGENT_VERSION             "cagentVersion"
#define GLOBAL_AGENT_TYPE             "cagentType"
#define GLOBAL_BIOS_VERSION              "biosVersion"
#define GLOBAL_PLATFORM_NAME             "platformName"
#define GLOBAL_PROCESSOR_NAME            "processorName"
#define GLOBAL_OS_ARCH                   "osArch"
#define GLOBAL_SYS_TOTAL_PHYS_MEM        "totalPhysMemKB"
#define GLOBAL_SYS_MACS					 "macs"
#define GLOBAL_SYS_IP					 "IP"

#define AGNET_INFO_CMD				1 
#define cagent_agent_info			21
#define cagent_global				109
#define glb_get_init_info_rep		116
#ifdef WIN32
	#if _MSC_VER < 1900
struct timespec {
	time_t   tv_sec;        /* seconds */
	long     tv_nsec;       /* nanoseconds */
};
	#else 
#include <time.h> // VS2015 or WIN_IOT
	#endif // WIN_IOT
#endif
char *scparser_utf8toansi(const char* str)
{
	int len = 0;
	char *strOutput = NULL;
	if(!IsUTF8(str))
	{
		len = strlen(str)+1;
		strOutput = (char *)malloc(len);
		memcpy(strOutput, str, len);
		
	}
	else
	{
		char * tempStr=UTF8ToANSI(str);
		len = strlen(tempStr)+1;
		strOutput = (char *)malloc(len);
		memcpy(strOutput, tempStr, len);
		free(tempStr);
		tempStr = NULL;
	}
	return strOutput;	
}

char * scparser_ansitoutf8(char* wText)
{
	char * utf8RetStr = NULL;
	int tmpLen = 0;
	if(!wText)
		return utf8RetStr;
	if(!IsUTF8(wText))
	{
		utf8RetStr = ANSIToUTF8(wText);
		tmpLen = !utf8RetStr ? 0 : strlen(utf8RetStr);
		if(tmpLen == 1)
		{
			if(utf8RetStr) free(utf8RetStr);
			utf8RetStr = UnicodeToUTF8((wchar_t *)wText);
		}
	}
	else
	{
		tmpLen = strlen(wText)+1;
		utf8RetStr = (char *)malloc(tmpLen);
		memcpy(utf8RetStr, wText, tmpLen);
	}
	return utf8RetStr;
}

char *scparser_print(PJSON item)
{
	if(item == NULL)
		return NULL;
	return cJSON_Print(item);
}

char *scparser_unformatted_print(PJSON item)
{
	if(item == NULL)
		return NULL;
	return cJSON_PrintUnformatted(item);
}

void scparser_free(PJSON ptr)
{
	cJSON *pAgentInfo = ptr;
	cJSON_Delete(pAgentInfo);
}

PJSON scparser_agentinfo_create(susiaccess_agent_profile_body_t const * pProfile, int status)
{
	/*
{"susiCommData":{"commCmd":1,"requestID":21,"devID":"00004437E646AC6D","hostname":"WIN-A0F7V3LOTLL","sn":"00004437E646AC6D","version":"1.0.0","account":"admin","passwd":"admin","status":1}}
	*/
   cJSON *pAgentInfo = NULL;

   if(!pProfile) return NULL;
   pAgentInfo = cJSON_CreateObject();
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_DEVID, pProfile->devId);
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_HOSTNAME, pProfile->hostname);
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_SN, pProfile->sn);
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_MAC, pProfile->mac);
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_VERSION, pProfile->version);
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_TYPE, pProfile->type);
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_PRODUCT, pProfile->product);
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_MANUFACTURE, pProfile->manufacture);
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_USERNAME, pProfile->account);
   cJSON_AddStringToObject(pAgentInfo, AGENTINFO_PASSWORD, pProfile->passwd);
   cJSON_AddNumberToObject(pAgentInfo, AGENTINFO_STATUS, status);

   return pAgentInfo;
}

char* scparser_agentinfo_print(susiaccess_agent_profile_body_t const * pProfile, int status)
{
	PJSON AgentInfoJSON = NULL;
	PJSON BaseInfoJSON = NULL;
	char* pAgentInfoPayload;
	susiaccess_packet_body_t packet;

	BaseInfoJSON = scparser_agentinfo_create(pProfile, status);
	memset(&packet, 0, sizeof(susiaccess_packet_body_t));
	strcpy(packet.devId, pProfile->devId);
	strcpy(packet.handlerName, "general");
	packet.requestID = cagent_agent_info;
	packet.cmd = AGNET_INFO_CMD;
	packet.content = scparser_unformatted_print(BaseInfoJSON);
	scparser_free(BaseInfoJSON);
	
	AgentInfoJSON = scparser_packet_create(&packet);
	pAgentInfoPayload = scparser_unformatted_print(AgentInfoJSON);
	scparser_free(AgentInfoJSON);
	free(packet.content);
	return pAgentInfoPayload;
}

PJSON scparser_osinfo_create(susiaccess_agent_profile_body_t const * pProfile)
{
/*
{"osInfo":{"cagentVersion":"1.0.0","osVersion":"Windows 7 Service Pack 1","biosVersion":"","platformName":"","processorName":"","osArch":"X64","totalPhysMemKB":8244060,"macs":"84:8F:69:CB:12:96;C4:17:FE:DB:F4:F5;00:50:56:C0:00:01;00:50:56:C0:00:08","IP":"172.22.12.24"}}
*/
	cJSON *OSInfoHead = NULL;
  	cJSON * pOSInfoItem = NULL;

	if(!pProfile) return OSInfoHead;

	pOSInfoItem = cJSON_CreateObject();

	cJSON_AddStringToObject(pOSInfoItem, GLOBAL_AGENT_VERSION, pProfile->version);
	cJSON_AddStringToObject(pOSInfoItem, GLOBAL_AGENT_TYPE, pProfile->type);
	cJSON_AddStringToObject(pOSInfoItem, GLOBAL_OS_VERSION, pProfile->osversion);
	cJSON_AddStringToObject(pOSInfoItem, GLOBAL_BIOS_VERSION, pProfile->biosversion);
	cJSON_AddStringToObject(pOSInfoItem, GLOBAL_PLATFORM_NAME, pProfile->platformname);
	cJSON_AddStringToObject(pOSInfoItem, GLOBAL_PROCESSOR_NAME, pProfile->processorname);
	cJSON_AddStringToObject(pOSInfoItem, GLOBAL_OS_ARCH, pProfile->osarchitect);
	cJSON_AddNumberToObject(pOSInfoItem, GLOBAL_SYS_TOTAL_PHYS_MEM, pProfile->totalmemsize);
	cJSON_AddStringToObject(pOSInfoItem, GLOBAL_SYS_MACS, pProfile->maclist);
	cJSON_AddStringToObject(pOSInfoItem, GLOBAL_SYS_IP, pProfile->localip);

	OSInfoHead = cJSON_CreateObject();
	cJSON_AddItemToObject(OSInfoHead, GLOBAL_OS_INFO, pOSInfoItem);

	return OSInfoHead;
}

char* scparser_osinfo_print(susiaccess_agent_profile_body_t const * pProfile)
{
	PJSON AgentInfoJSON = NULL;
	PJSON OSInfoJSON = NULL;
	char* pAgentInfoPayload;
	susiaccess_packet_body_t packet;

	OSInfoJSON = scparser_osinfo_create(pProfile);
	memset(&packet, 0, sizeof(susiaccess_packet_body_t));
	strcpy(packet.devId, pProfile->devId);
	strcpy(packet.handlerName, "general");
	packet.requestID = cagent_global;
	packet.cmd = glb_get_init_info_rep;
	packet.content = scparser_unformatted_print(OSInfoJSON);
	scparser_free(OSInfoJSON);

	AgentInfoJSON = scparser_packet_create(&packet);
	pAgentInfoPayload = scparser_unformatted_print(AgentInfoJSON);
	scparser_free(AgentInfoJSON);
	free(packet.content);
	return pAgentInfoPayload;
}

PJSON scparser_packet_create(susiaccess_packet_body_t const * pPacket)
{
	/*
{"susiCommData":{"commCmd":271,"requestID":103, XXX}}
	*/
   cJSON *pReqInfoHead = NULL;
   cJSON* root = NULL;
   long long tick = 0;

   if(!pPacket) return NULL;
   if(pPacket->content)
   {
	   char* strContent = scparser_ansitoutf8(pPacket->content);
	   root = cJSON_Parse(strContent);
	   free(strContent);
	   strContent = NULL;
   }
   if(pPacket->type == pkt_type_custom)
	   return root;

   if(!root)
	   root = cJSON_CreateObject();

   
   pReqInfoHead = cJSON_CreateObject();

   cJSON_AddItemToObject(pReqInfoHead, BASICINFO_BODY_STRUCT, root);
   cJSON_AddNumberToObject(root, BASICINFO_CMDTYPE, pPacket->cmd);
   cJSON_AddNumberToObject(root, BASICINFO_REQID, pPacket->requestID);
   cJSON_AddStringToObject(root, BASICINFO_AGENTID, pPacket->devId);
   cJSON_AddStringToObject(root, BASICINFO_HANDLERNAME, pPacket->handlerName);

   {
		struct timespec time;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		tick = (long long)tv.tv_sec*1000 + (long long)tv.tv_usec/1000;
		cJSON_AddNumberToObject(root, BASICINFO_TIMESTAMP, tick);
   }
   return pReqInfoHead;
}

char* scparser_packet_print(susiaccess_packet_body_t const * pPacket)
{
	PJSON pPacketJSON = NULL;
	char* pPayload = NULL;

	pPacketJSON = scparser_packet_create(pPacket);

	pPayload = scparser_unformatted_print(pPacketJSON);
	
	scparser_free(pPacketJSON);

	return pPayload;
}

int scparser_message_parse(void* data, int datalen, susiaccess_packet_body_t * pkt)
{
	/*{"susiCommData":{"commCmd":251,"catalogID":4,"requestID":10}}*/
	//char* strInput = NULL;
	cJSON* root = NULL;
	cJSON* body = NULL;
	cJSON* target = NULL;
	cJSON* content = NULL;

	if(!data) return false;

	if(!pkt) return false;

	memset(pkt, 0 , sizeof(susiaccess_packet_body_t));

	//strInput = scparser_utf8toansi(data);

	//root = cJSON_Parse(strInput);
	root = cJSON_Parse(data);
	
	//free(strInput);
	//strInput = NULL;
	
	if(!root) return false;

	body = cJSON_GetObjectItem(root, BASICINFO_BODY_STRUCT);
	if(!body)
	{
		pkt->content = strdup((char*)data);
		pkt->type = pkt_type_custom;
		cJSON_Delete(root);
		return true;
	}

	target = body->child;
	while (target)
	{
		if(!strcmp(target->string, BASICINFO_CMDTYPE))
			pkt->cmd = target->valueint;
		else if(!strcmp(target->string, BASICINFO_REQID))
			pkt->requestID = target->valueint;
		else if(!strcmp(target->string, BASICINFO_AGENTID))
			strncpy(pkt->devId, target->valuestring, sizeof(pkt->devId));
		else if(!strcmp(target->string, BASICINFO_HANDLERNAME))
			strncpy(pkt->handlerName, target->valuestring, sizeof(pkt->handlerName));
		else if(!strcmp(target->string, BASICINFO_CATALOG))
		{

		}
		else
		{
			if(!content)
				content = cJSON_CreateObject();

			cJSON_AddItemToObject(content, target->string, cJSON_Duplicate(target,true));
		}
		target = target->next;
	}

	if(content)
	{
		char* strInput = NULL;
		char* strcontent = cJSON_PrintUnformatted(content);
		cJSON_Delete(content);
		strInput = scparser_utf8toansi(strcontent);
		pkt->content = strdup(strInput);
		free(strcontent);
		free(strInput);
		strInput = NULL;
	}

	cJSON_Delete(root);
	return true;
}
