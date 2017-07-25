#include "pktparser.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "cJSON.h"

#define BASICINFO_BODY_STRUCT			"susiCommData"
#define BASICINFO_REQID					"requestID"
#define BASICINFO_HANDLERNAME			"handlerName"
#define BASICINFO_CMDTYPE				"commCmd"
#define BASICINFO_CATALOG				"catalogID"
#define BASICINFO_AGENTID				"agentID"
#define BASICINFO_TIMESTAMP				"sendTS"

#define AGENTINFO_DEVID					"devID"
#define AGENTINFO_HOSTNAME				"hostname"
#define AGENTINFO_SN					"sn"
#define AGENTINFO_MAC					"mac"
#define AGENTINFO_VERSION				"version"
#define AGENTINFO_TYPE					"type"
#define AGENTINFO_PRODUCT				"product"
#define AGENTINFO_MANUFACTURE			"manufacture"
#define AGENTINFO_STATUS				"status"

#define GLOBAL_OS_INFO					"osInfo"
#define GLOBAL_OS_VERSION				"osVersion"
#define GLOBAL_AGENT_VERSION			"cagentVersion"
#define GLOBAL_AGENT_TYPE				"cagentType"
#define GLOBAL_BIOS_VERSION				"biosVersion"
#define GLOBAL_PLATFORM_NAME			"platformName"
#define GLOBAL_PROCESSOR_NAME			"processorName"
#define GLOBAL_OS_ARCH					"osArch"
#define GLOBAL_SYS_TOTAL_PHYS_MEM		"totalPhysMemKB"
#define GLOBAL_SYS_MACS					"macs"
#define GLOBAL_SYS_IP					"IP"

char *pkg_parser_print(PJSON item)
{
	if(item == NULL)
		return NULL;
	return cJSON_Print(item);
}

char *pkg_parser_print_unformatted(PJSON item)
{
	if(item == NULL)
		return NULL;
	return cJSON_PrintUnformatted(item);
}

void pkg_parser_free(PJSON ptr)
{
	cJSON *pAgentInfo = ptr;
	cJSON_Delete(pAgentInfo);
}

PJSON pkg_parser_packet_create(susiaccess_packet_body_t const * pPacket)
{
	/*
{"susiCommData":{"commCmd":271,"requestID":103, XXX}}
	*/
   cJSON *pReqInfoHead = NULL;
   cJSON* root = NULL;
   long tick = 0;

   if(!pPacket) return NULL;
   if(pPacket->content)
	   root = cJSON_Parse(pPacket->content);
   else
	   root = cJSON_CreateObject();

   if(!root) return NULL;
   pReqInfoHead = cJSON_CreateObject();

   cJSON_AddItemToObject(pReqInfoHead, BASICINFO_BODY_STRUCT, root);
   cJSON_AddNumberToObject(root, BASICINFO_CMDTYPE, pPacket->cmd);
   cJSON_AddNumberToObject(root, BASICINFO_REQID, pPacket->requestID);
   cJSON_AddStringToObject(root, BASICINFO_AGENTID, pPacket->devId);
   cJSON_AddStringToObject(root, BASICINFO_HANDLERNAME, pPacket->handlerName);
   //cJSON_AddNumberToObject(root, BASICINFO_CATALOG, pPacket->catalogID);

   tick = (long) time((time_t *) NULL);
   cJSON_AddNumberToObject(root, BASICINFO_TIMESTAMP, tick);
   return pReqInfoHead;
}

char* pkg_parser_packet_print(susiaccess_packet_body_t * pkt)
{
	char* pReqInfoPayload = NULL;
	PJSON ReqInfoJSON = NULL;
	if(!pkt)
		return pReqInfoPayload;
	ReqInfoJSON = pkg_parser_packet_create(pkt);
	pReqInfoPayload = pkg_parser_print_unformatted(ReqInfoJSON);
	pkg_parser_free(ReqInfoJSON);
	return pReqInfoPayload;
}

int pkg_parser_recv_message_parse(void* data, int datalen, susiaccess_packet_body_t * pkt)
{
	/*{"susiCommData":{"commCmd":251,"catalogID":4,"requestID":10}}*/

	cJSON* root = NULL;
	cJSON* body = NULL;
	cJSON* target = NULL;
	cJSON* content = NULL;

	if(!data) return false;

	if(!pkt) return false;

	memset(pkt, 0 , sizeof(susiaccess_packet_body_t));

	root = cJSON_Parse(data);
	if(!root) return false;

	body = cJSON_GetObjectItem(root, BASICINFO_BODY_STRUCT);
	if(!body)
	{
		cJSON_Delete(root);
		return false;
	}

	target = body->child;
	while (target)
	{
		if(!strcmp(target->string, BASICINFO_CMDTYPE))
			pkt->cmd = target->valueint;
		else if(!strcmp(target->string, BASICINFO_REQID))
			pkt->requestID = target->valueint;
		else if(!strcmp(target->string, BASICINFO_AGENTID))
			strcpy(pkt->devId, target->valuestring);
		else if(!strcmp(target->string, BASICINFO_HANDLERNAME))
			strcpy(pkt->handlerName, target->valuestring);
		else if(!strcmp(target->string, BASICINFO_CATALOG))
		{
			//pkt->catalogID = target->valueint;
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
		char* strcontent = cJSON_PrintUnformatted(content);
		cJSON_Delete(content);
		pkt->content = calloc(strlen(strcontent)+1, 1);
		if(pkt->content)
			strncpy(pkt->content, strcontent, strlen(strcontent));
		free(strcontent);
	}

	cJSON_Delete(root);
	return true;
}

