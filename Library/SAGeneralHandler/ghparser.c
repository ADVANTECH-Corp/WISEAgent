#include "ghparser.h"
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

#include "util_path.h"

#define AGENTINFO_BODY_STRUCT	"susiCommData"
#define AGENTINFO_CMDTYPE		"commCmd"
#define AGENTINFO_AUTOREPORT	"autoReport"
#define AGENTINFO_REPORTDATALEN	"reportDataLength"
#define AGENTINFO_REPORTDATA	"reportData"
#define AGENTINFO_SESSION_ID	"sessionID"

#define GBL_UPDATE_PARMAS                "params"
#define GBL_UPDATE_USERNAME              "userName"
#define GBL_UPDATE_PASSWORD              "pwd"
#define GBL_UPDATE_PORT                  "port"
#define GBL_UPDATE_PATH                  "path"
#define GBL_UPDATE_MD5                   "md5"

#define GBL_RENAME_DEVNAME				 "devName"

#define GBL_SERVER_RESPONSE				 "response"
#define GBL_SERVER_STATUSCODE			 "statuscode"
#define GBL_SERVER_RESPONSEMESSAGE		 "msg"
#define GBL_SERVER_SERVER_NODE			 "server"
#define GBL_SERVER_SERVER_ADDRESS		 "address"
#define GBL_SERVER_SERVER_PORT			 "port"
#define GBL_SERVER_SERVER_AUTH			 "auth"
#define GBL_SERVER_SERVER_IP_LIST		 "serverIPList"
#define GBL_SERVER_N_FLAG				 "n"

#define GBL_HEARTBEAT_RATE				"heartbeatrate"

bool ParseUpdateCMD(void* data, int datalen, download_params_t *pDownloadParams)
{
	/*{"susiCommData":{"commCmd":111,"catalogID":4,"requestID":16,"params":{"userName":"sa30Read","pwd":"sa30Read","port":2121,"path":"/upgrade/SA30Agent_V3.0.15.exe","md5":"758C9D0A8654A93D09F375D33E262507"}}}*/
	cJSON * root = NULL, *body = NULL, *pSubItem = NULL; 
	cJSON *pDownloadParamsItem = NULL;
	bool bRet = false;
	if(!data) return false;
	if(datalen<=0) return false;
	if(!pDownloadParams) return false;

	root = cJSON_Parse(data);
	if(!root) return false;

	body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
	if(!body)
	{
		cJSON_Delete(root);
		return false;
	}

	pDownloadParamsItem = cJSON_GetObjectItem(body, GBL_UPDATE_PARMAS);
	if(pDownloadParams && pDownloadParamsItem)
	{
		pSubItem = cJSON_GetObjectItem(pDownloadParamsItem, GBL_UPDATE_USERNAME);
		if(pSubItem)
		{
			strcpy(pDownloadParams->ftpuserName, pSubItem->valuestring);
			pSubItem = cJSON_GetObjectItem(pDownloadParamsItem, GBL_UPDATE_PASSWORD);
			if(pSubItem)
			{
				strcpy(pDownloadParams->ftpPassword, pSubItem->valuestring);
				pSubItem = cJSON_GetObjectItem(pDownloadParamsItem, GBL_UPDATE_PORT);
				if(pSubItem)
				{
					pDownloadParams->port = pSubItem->valueint;
					pSubItem = cJSON_GetObjectItem(pDownloadParamsItem, GBL_UPDATE_PATH);
					if(pSubItem)
					{
						strcpy(pDownloadParams->installerPath, pSubItem->valuestring);
						pSubItem = cJSON_GetObjectItem(pDownloadParamsItem, GBL_UPDATE_MD5);
						if(pSubItem)
						{
							strcpy(pDownloadParams->md5, pSubItem->valuestring);
							bRet = true;
						}
					}
				}
			}
		}
	}
	cJSON_Delete(root);
	return bRet;
}

bool ParseReceivedCMD(void* data, int datalen, int * cmdID, char* pSessionID)
{
	/*{"susiCommData":{"commCmd":251,"catalogID":4,"requestID":10, "sessionID":"XXX"}}*/

	cJSON* root = NULL;
	cJSON* body = NULL;
	cJSON* target = NULL;

	if(!data) return false;
	if(datalen<=0) return false;
	//MonitorLog(g_loghandle, Normal, " %s>Parser_ParseReceivedData [%s]\n", MyTopic, data );
	root = cJSON_Parse(data);
	if(!root) return false;

	body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
	if(!body)
	{
		cJSON_Delete(root);
		return false;
	}

	target = cJSON_GetObjectItem(body, AGENTINFO_CMDTYPE);
	if(target)
	{
		*cmdID = target->valueint;
	}

	target = cJSON_GetObjectItem(body, AGENTINFO_SESSION_ID);
	if(target)
	{
		if(pSessionID)
			strcpy(pSessionID, target->valuestring);
	}

	cJSON_Delete(root);
	return true;
}

bool ParseRenameCMD(void* data, int datalen, char* pNewName)
{
	/*{"susiCommData":{"devName":"pc-test1","commCmd":113,"requestID":1001,"agentID":"","handlerName":"","sendTS":1434447015}}*/
	cJSON * root = NULL, *body = NULL, *pSubItem = NULL; 
	bool bRet = false;
	if(!data) return false;
	if(datalen<=0) return false;
	if(!pNewName) return false;

	root = cJSON_Parse(data);
	if(!root) return false;

	body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
	if(!body)
	{
		cJSON_Delete(root);
		return false;
	}

	pSubItem = cJSON_GetObjectItem(body, GBL_RENAME_DEVNAME);
	if(pSubItem)
	{
		strcpy(pNewName, pSubItem->valuestring);
		bRet = true;
	}
	cJSON_Delete(root);
	return bRet;
}

int ParseServerCtrl(void* data, int datalen, char* workdir, GENERAL_CTRL_MSG *pMessage)
{
	/*{"susiCommData":{"commCmd":125,"handlerName":"general","catalogID":4,"response":{"statuscode":0,"msg":"Server losyconnection"}}}*/
	cJSON *root = NULL, *body = NULL, *pSubItem = NULL, *pTarget = NULL, *pServer = NULL,*pServerIPList = NULL; 
	bool bRet = false;
	if(!data) return false;
	if(datalen<=0) return false;
	if(!pMessage) return false;

	root = cJSON_Parse(data);
	if(!root) return false;

	body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
	if(!body)
	{
		cJSON_Delete(root);
		return false;
	}

	pSubItem = cJSON_GetObjectItem(body, GBL_SERVER_RESPONSE);
	if(pSubItem)
	{
		pTarget = cJSON_GetObjectItem(pSubItem, GBL_SERVER_STATUSCODE);
		if(pTarget)
		{
			pMessage->statuscode = pTarget->valueint;
		}

		pTarget = cJSON_GetObjectItem(pSubItem, GBL_SERVER_RESPONSEMESSAGE);
		if(pTarget)
		{
			if(pTarget->valuestring)
			{
				if(strlen(pTarget->valuestring)<=0)
					pMessage->msg = NULL;
				else
				{
					pMessage->msg = malloc(strlen(pTarget->valuestring)+1);
					memset(pMessage->msg, 0, strlen(pTarget->valuestring)+1);
					strcpy(pMessage->msg, pTarget->valuestring);
				}
			}
		}

		pServer = cJSON_GetObjectItem(pSubItem, GBL_SERVER_SERVER_NODE);
		if(pServer)
		{
			pTarget = cJSON_GetObjectItem(pServer, GBL_SERVER_SERVER_ADDRESS);
			if(pTarget)
			{
				strcpy(pMessage->serverIP, pTarget->valuestring);
			}

			pTarget = cJSON_GetObjectItem(pServer, GBL_SERVER_SERVER_PORT);
			if(pTarget)
			{
				pMessage->serverPort = pTarget->valueint;
			}

			pTarget = cJSON_GetObjectItem(pServer, GBL_SERVER_SERVER_AUTH);
			if(pTarget)
			{
				strcpy(pMessage->serverAuth, pTarget->valuestring);
			}
		}
		pServerIPList = cJSON_GetObjectItem(pSubItem, GBL_SERVER_SERVER_IP_LIST);
		if(pServerIPList)
		{
			cJSON * subItem = NULL;
			cJSON * valItem = NULL;
			int i = 0;
			FILE *fp = NULL;
			int nCount = cJSON_GetArraySize(pServerIPList);
			char filepath[MAX_PATH] = {0};
			util_path_combine(filepath, workdir, DEF_SERVER_IP_LIST_FILE);
			if(fp=fopen(filepath,"w+"))
			{
				for(i = 0; i<nCount; i++)
				{
					subItem = cJSON_GetArrayItem(pServerIPList, i);
					if(subItem)
					{
						valItem = cJSON_GetObjectItem(subItem, GBL_SERVER_N_FLAG);
						if(valItem)
						{
							fputs(valItem->valuestring,fp);
							fputc('\n',fp);
						}
					}
				}
			}
			fclose(fp);
		}
		bRet = true;
	}
	cJSON_Delete(root);
	return bRet;
}

bool ParseHeartbeatRateUpdateCMD(void* data, int datalen, int* heartbeatrate)
{
	/*{"susiCommData":{"commCmd":129,"handlerName":"general","heartbeatrate":5,"sessionID":"123465"}}*/
	cJSON * root = NULL, *body = NULL, *pSubItem = NULL; 
	bool bRet = false;
	*heartbeatrate=0;
	if(!data) return false;
	if(datalen<=0) return false;
	if(!heartbeatrate) return false;

	root = cJSON_Parse(data);
	if(!root) return false;

	body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
	if(!body)
	{
		cJSON_Delete(root);
		return false;
	}

	pSubItem = cJSON_GetObjectItem(body, GBL_HEARTBEAT_RATE);
	if(pSubItem)
	{
		*heartbeatrate = pSubItem->valueint;
		bRet = true;
	}
	cJSON_Delete(root);
	return bRet;
}