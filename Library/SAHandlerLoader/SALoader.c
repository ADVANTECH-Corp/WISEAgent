/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.														 */
/* Create Date  : 2014/06/18 by Eric Liang															     */
/* Modified Date: 2014/06/30 by Eric Liang															 */
/* Abstract     : Plugin Loader                                   													*/
/* Reference    : None																									 */
/****************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "SALoader.h"
#include "moduleconfig.h"
#include "loaderlog.h"
#include "util_path.h"
#include "util_libloader.h"
#include "AdvPlatform.h"

Handler_Loader_Interface* m_plugininfo = NULL;

HandlerSendCbf  g_sendcbf = NULL;						// Client Send information (in JSON format) to Cloud Server	
HandlerSendCustCbf  g_sendcustcbf = NULL;			    // Client Send information (in JSON format) to Cloud Server with custom topic	
HandlerSubscribeCustCbf  g_subscribecustcbf = NULL;			    // Client Send information (in JSON format) to Cloud Server with custom topic	
HandlerAutoReportCbf g_sendreportcbf = NULL;				// Client Send report (in JSON format) to Cloud Server with AutoReport topic
HandlerSendCapabilityCbf g_sendcapabilitycbf = NULL;
HandlerSendEventCbf g_sendnotifycbf= NULL;
HandlerConnectServerCbf g_connectservercbf = NULL;
HandlerDisconnectCbf g_disconnectcbf = NULL;

void Loader_SetAgentProfile(susiaccess_agent_profile_body_t * profile, int status);
void Loader_SetAgentInfo(cagent_agent_info_body_t * pagentinfo);
void Loader_SetBasicPluginInfo(Handler_Loader_Interface * handlerInfo);
int Handler_LoadHandler(Handler_Loader_Interface *pLoader, char const *handlerpath, char const *name);
int Handler_InitHandler( Handler_Loader_Interface *pLoader, HandlerSendCbf cbf , HandlerSendCustCbf cbfcust, HandlerSubscribeCustCbf cbfsubscribe, HandlerAutoReportCbf cbfreport, HandlerSendCapabilityCbf cbfinfo);
int Handler_GetHandlerStatus( Handler_Loader_Interface *pLoader, HANDLER_THREAD_STATUS * pOutStatus );
int Handler_StartHandler( Handler_Loader_Interface *pLoader );
int Handler_StopHandler( Handler_Loader_Interface *pLoader );
void Handler_HandlerRecv( Handler_Loader_Interface *pLoader, char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2);
int Handler_ReleaseHandler(Handler_Loader_Interface *pLoader);

void Loader_SetAgentInfo(cagent_agent_info_body_t * pagentinfo)
{
	if(!m_plugininfo) return;
	if(pagentinfo == NULL)
	{
		if(m_plugininfo->pHandlerInfo->agentInfo)
			free(m_plugininfo->pHandlerInfo->agentInfo);
		m_plugininfo->pHandlerInfo->agentInfo = NULL;
		return;
	}
	if(!m_plugininfo->pHandlerInfo->agentInfo)
	{
		m_plugininfo->pHandlerInfo->agentInfo = (cagent_agent_info_body_t*)malloc(sizeof(cagent_agent_info_body_t));
		memset( m_plugininfo->pHandlerInfo->agentInfo, 0, sizeof(cagent_agent_info_body_t));
		//printf("***Base malloc:<%d>\r\n", m_plugininfo->pHandlerInfo->agentInfo);
	}
	memcpy( m_plugininfo->pHandlerInfo->agentInfo, pagentinfo, sizeof(cagent_agent_info_body_t));
}

void Loader_SetBasicPluginInfo(Handler_Loader_Interface * pluginInfo)
{
	if(pluginInfo == NULL)
	{
		Loader_SetAgentInfo(NULL);
		free(m_plugininfo);
		m_plugininfo = NULL;
		return;
	}
	if(!m_plugininfo)
	{
		m_plugininfo = (Handler_Loader_Interface*)malloc(sizeof(Handler_Loader_Interface));
		memset( m_plugininfo, 0, sizeof(Handler_Loader_Interface));
	}
	memcpy( m_plugininfo, pluginInfo, sizeof(Handler_Loader_Interface));
	if(pluginInfo->pHandlerInfo)
	{
		m_plugininfo->pHandlerInfo = malloc(sizeof(HANDLER_INFO_EX));
		memset(m_plugininfo->pHandlerInfo, 0, sizeof(HANDLER_INFO_EX));
		memcpy( m_plugininfo->pHandlerInfo, pluginInfo->pHandlerInfo, sizeof(HANDLER_INFO_EX));
	}
}

void HANDLERLOADER_API Loader_Initialize(char const * workdir, susiaccess_agent_conf_body_t const * conf, susiaccess_agent_profile_body_t const * profile, void* loghandler)
{
	Handler_Loader_Interface pligininfo;
	SALoaderLogHandle = loghandler;
	memset(&pligininfo, 0, sizeof(Handler_Loader_Interface));
	pligininfo.pHandlerInfo = malloc(sizeof(HANDLER_INFO_EX));
	memset(pligininfo.pHandlerInfo, 0, sizeof(HANDLER_INFO_EX));
	if(pligininfo.pHandlerInfo)
	{
		pligininfo.pHandlerInfo->loghandle = loghandler;
		if(workdir)
			strncpy(pligininfo.pHandlerInfo->WorkDir, workdir, strlen(workdir)+1);
		if(conf)
		{
			strncpy(pligininfo.pHandlerInfo->ServerIP,  conf->serverIP, strlen(conf->serverIP)+1);
			pligininfo.pHandlerInfo->ServerPort = atol(conf->serverPort);
			strncpy(((HANDLER_INFO_EX*)pligininfo.pHandlerInfo)->serverAuth,  conf->serverAuth, strlen(conf->serverAuth)+1);
			((HANDLER_INFO_EX*)pligininfo.pHandlerInfo)->TLSType = conf->tlstype;
			strncpy(((HANDLER_INFO_EX*)pligininfo.pHandlerInfo)->PSK,  conf->psk, strlen(conf->psk)+1);
		}
	}

	/*pligininfo.handlerInfo.loghandle = loghandler;
	if(workdir)
		strncpy(pligininfo.handlerInfo.WorkDir, workdir, strlen(workdir)+1);
	if(conf)
	{
		strncpy(pligininfo.handlerInfo.ServerIP,  conf->serverIP, strlen(conf->serverIP)+1);
		pligininfo.handlerInfo.ServerPort = atol(conf->serverPort);
	}*/

	Loader_SetBasicPluginInfo(&pligininfo);
	
	if(profile)
	{
		cagent_agent_info_body_t pageinfo;
		strcpy(pageinfo.devId, profile->devId);
		strcpy(pageinfo.hostname, profile->hostname);
		strcpy(pageinfo.mac, profile->mac);
		strcpy(pageinfo.manufacture, profile->manufacture);
		strcpy(pageinfo.product, profile->product);
		strcpy(pageinfo.sn, profile->sn);
		strcpy(pageinfo.type, profile->type);
		strcpy(pageinfo.version, profile->version);
		pageinfo.status = AGENT_STATUS_OFFLINE;
		Loader_SetAgentInfo(&pageinfo);
	}

	if(pligininfo.pHandlerInfo)
		free(pligininfo.pHandlerInfo);
	pligininfo.pHandlerInfo = NULL;
}

void HANDLERLOADER_API Loader_Uninitialize()
{
	if(m_plugininfo)
	{
		if(m_plugininfo->pHandlerInfo)
		{
			if(m_plugininfo->pHandlerInfo->agentInfo)
			{
				free(m_plugininfo->pHandlerInfo->agentInfo);
				//printf("***Base free:<%d>\r\n", m_plugininfo->pHandlerInfo->Name, m_plugininfo->pHandlerInfo->agentInfo);
			}
			m_plugininfo->pHandlerInfo->agentInfo = NULL;
			free(m_plugininfo->pHandlerInfo);
		}
		m_plugininfo->pHandlerInfo = NULL;
		free(m_plugininfo);
	}
	m_plugininfo = NULL;
}

void HANDLERLOADER_API Loader_GetBasicHandlerLoaderInterface(Handler_Loader_Interface * handler)
{
	if(!handler)
		return;
	if(handler->pHandlerInfo)
	{
		if(handler->pHandlerInfo->agentInfo)
			free(handler->pHandlerInfo->agentInfo);
		free(handler->pHandlerInfo);
	}
	memset(handler, 0, sizeof(Handler_Loader_Interface));
	memcpy(handler, m_plugininfo, sizeof(Handler_Loader_Interface));
	/*if(m_plugininfo->pHandlerInfo)
	{
		handler->pHandlerInfo = malloc(sizeof(HANDLER_INFO_EX));
		if(handler->pHandlerInfo)
		{
			memset(handler->pHandlerInfo ,0, sizeof(HANDLER_INFO_EX));
			memcpy(handler->pHandlerInfo, m_plugininfo->pHandlerInfo, sizeof(HANDLER_INFO_EX));
			if(m_plugininfo->pHandlerInfo->agentInfo)
			{
				handler->pHandlerInfo->agentInfo = malloc(sizeof(cagent_agent_info_body_t));
				memset(handler->pHandlerInfo->agentInfo, 0, sizeof(cagent_agent_info_body_t));
				memcpy(handler->pHandlerInfo->agentInfo, m_plugininfo->pHandlerInfo->agentInfo, sizeof(cagent_agent_info_body_t));
			}
		}
	}*/
}

void HANDLERLOADER_API Loader_SetAgentStatus(Handler_List_t *pLoaderList, susiaccess_agent_conf_body_t const * conf, susiaccess_agent_profile_body_t const * pProfile, int status)
{
	Handler_Loader_Interface *pInterfaceTmp = NULL;
	if(m_plugininfo)
	{
		if(m_plugininfo->pHandlerInfo)
		{
			if(conf)
			{
				strncpy(m_plugininfo->pHandlerInfo->ServerIP,  conf->serverIP, strlen(conf->serverIP)+1);
				m_plugininfo->pHandlerInfo->ServerPort = atol(conf->serverPort);
			}
			if(m_plugininfo->pHandlerInfo->agentInfo)
			{
				m_plugininfo->pHandlerInfo->agentInfo->status = status;
				if(pProfile)
					strncpy(m_plugininfo->pHandlerInfo->agentInfo->hostname, pProfile->hostname, sizeof(m_plugininfo->pHandlerInfo->agentInfo->hostname));
			}
		}
	}
	
	if(!pLoaderList) return;
	pInterfaceTmp = pLoaderList->items;
	while(pInterfaceTmp)
	{
		if(pInterfaceTmp->pHandlerInfo)
		{
			if(conf)
			{
				strncpy(pInterfaceTmp->pHandlerInfo->ServerIP,  conf->serverIP, strlen(conf->serverIP)+1);
				pInterfaceTmp->pHandlerInfo->ServerPort = atol(conf->serverPort);
			}
			if(pInterfaceTmp->pHandlerInfo->agentInfo)
			{
				pInterfaceTmp->pHandlerInfo->agentInfo->status = status;
				if(pProfile)
					strncpy(pInterfaceTmp->pHandlerInfo->agentInfo->hostname, pProfile->hostname, sizeof(pInterfaceTmp->pHandlerInfo->agentInfo->hostname));
			}

			if(pInterfaceTmp->type!=virtual_handler)
				if(pInterfaceTmp->Handler_OnStatusChange_API)
					pInterfaceTmp->Handler_OnStatusChange_API(pInterfaceTmp->pHandlerInfo);
		}
		pInterfaceTmp = pInterfaceTmp->next;
	}
}

void HANDLERLOADER_API Loader_SetFuncCB(Callback_Functions_t* funcs)
{
	g_sendcbf = funcs->sendcbf;
	g_sendcustcbf = funcs->sendcustcbf;
	g_subscribecustcbf = funcs->subscribecustcbf;
	g_sendreportcbf = funcs->sendreportcbf;
	g_sendcapabilitycbf = funcs->sendcapabilitycbf;
	g_sendnotifycbf = funcs->sendevnetcbf;
	g_connectservercbf =  funcs->connectservercbf;
	g_disconnectcbf = funcs->disconnectcbf;
	if(m_plugininfo)
	{
		if(m_plugininfo->pHandlerInfo)
		{
			HANDLER_INFO_EX *pHandler = (HANDLER_INFO_EX *)m_plugininfo->pHandlerInfo;

			pHandler->sendcbf = funcs->sendcbf;
			pHandler->sendcustcbf = funcs->sendcustcbf;
			pHandler->subscribecustcbf = funcs->subscribecustcbf;
			pHandler->sendreportcbf = funcs->sendreportcbf;
			pHandler->sendcapabilitycbf = funcs->sendcapabilitycbf;
			pHandler->sendeventcbf = funcs->sendevnetcbf;
			pHandler->connectservercbf = funcs->connectservercbf;
			pHandler->disconnectcbf = funcs->disconnectcbf;
			pHandler->renamecbf = funcs->renamecbf;
			pHandler->sendosinfocbf = funcs->sendosinfocbf;
			pHandler->addvirtualhandlercbf = funcs->addvirtualhandlercbf;
		}
	}
}

Handler_Loader_Interface * HANDLERLOADER_API Loader_GetLastHandler(Handler_List_t *pLoaderList)
{
	Handler_Loader_Interface *pLoader = NULL;
	if(!pLoaderList) return pLoader;
	pLoader = pLoaderList->items;
	if(!pLoader) return pLoader;
	while(pLoader->next)
	{
		pLoader = pLoader->next;
	}
	return pLoader;
}

Handler_Loader_Interface * HANDLERLOADER_API Loader_FindHandler(Handler_List_t *pLoaderList, char const *name)
{
	Handler_Loader_Interface *pLoader = NULL;
	if(!pLoaderList) return pLoader;
	pLoader = pLoaderList->items;
	while(pLoader)
	{
		if(pLoader->pHandlerInfo)
		{	
			if(!strcmp(pLoader->pHandlerInfo->Name, name))
				return pLoader;
		}
		pLoader = pLoader->next;
	}
	return NULL;
}

Handler_Loader_Interface * HANDLERLOADER_API Loader_FindHandlerByReqID(Handler_List_t *pLoaderList, int reqID)
{
	Handler_Loader_Interface *pLoader = NULL;
	if(!pLoaderList) return pLoader;
	pLoader = pLoaderList->items;
	while(pLoader)
	{
		if(pLoader->pHandlerInfo)
		{
			if(pLoader->pHandlerInfo->RequestID == reqID)
				return pLoader;
		}
		pLoader = pLoader->next;
	}
	return NULL;
}

int HANDLERLOADER_API Loader_AddHandler(Handler_List_t *pLoaderList, Handler_Loader_Interface * pluginInfo)
{
	Handler_Loader_Interface *pLastLoader = NULL;
	Handler_Loader_Interface *pClone = NULL;
	if(!pLoaderList || !pluginInfo) return -1;

	if(!pluginInfo->pHandlerInfo)  return -1;

	if(Loader_FindHandler(pLoaderList, pluginInfo->pHandlerInfo->Name) != NULL)
		return -1;

	pClone = malloc(sizeof(Handler_Loader_Interface));
	memset(pClone, 0, sizeof(Handler_Loader_Interface));
	memcpy(pClone, pluginInfo, sizeof(Handler_Loader_Interface));
	if(pluginInfo->pHandlerInfo)
	{
		pClone->pHandlerInfo = malloc(sizeof(HANDLER_INFO_EX));
		memset(pClone->pHandlerInfo, 0, sizeof(HANDLER_INFO_EX));
		memcpy(pClone->pHandlerInfo, pluginInfo->pHandlerInfo, sizeof(HANDLER_INFO_EX));

		if(pluginInfo->pHandlerInfo->agentInfo)
		{
			pClone->pHandlerInfo->agentInfo = malloc(sizeof(cagent_agent_info_body_t));
			memset(pClone->pHandlerInfo->agentInfo, 0, sizeof(cagent_agent_info_body_t));
			memcpy(pClone->pHandlerInfo->agentInfo, pluginInfo->pHandlerInfo->agentInfo, sizeof(cagent_agent_info_body_t));
			//printf("***%s malloc:<%d>\r\n", pluginInfo->pHandlerInfo->Name, pClone->pHandlerInfo->agentInfo);
		}
	}
	pLastLoader = Loader_GetLastHandler(pLoaderList);
	if(pLastLoader == NULL)
	{
		pLoaderList->items = pClone;
	}
	else
	{
		pLastLoader->next = pClone;
		pClone->prev = pLastLoader;
	}
	pLoaderList->total++;
	return pLoaderList->total;
}
bool Handler_LoadVirtualHandler(Handler_List_t *pLoaderList,Handler_Loader_Interface *pLoader,char *VirName, char *HandlerName)
{
	void* handler = NULL;

	if(pLoader->pHandlerInfo)
	{
		if(pLoader->pHandlerInfo->agentInfo)
		{
			free(pLoader->pHandlerInfo->agentInfo);
			//printf("***%s free:<%d>\r\n", pLoader->pHandlerInfo->Name, pLoader->pHandlerInfo->agentInfo);
		}
		free(pLoader->pHandlerInfo);
		pLoader->pHandlerInfo = NULL;
	}

	memcpy(pLoader, m_plugininfo, sizeof(Handler_Loader_Interface));

	if((handler=Loader_FindHandler(pLoaderList, HandlerName)) != NULL)
	{
		memcpy(pLoader, (Handler_Loader_Interface *)handler, sizeof(Handler_Loader_Interface));
		pLoader->next=NULL;
		pLoader->prev=NULL;
		//strcpy(pLoader->pHandlerInfo->Name,name);
		if(m_plugininfo->pHandlerInfo)
		{
			pLoader->pHandlerInfo = malloc(sizeof(HANDLER_INFO_EX));
			memset(pLoader->pHandlerInfo, 0, sizeof(HANDLER_INFO_EX));
			memcpy(pLoader->pHandlerInfo, m_plugininfo->pHandlerInfo, sizeof(HANDLER_INFO_EX));
			if(m_plugininfo->pHandlerInfo->agentInfo)
			{
				pLoader->pHandlerInfo->agentInfo = malloc(sizeof(cagent_agent_info_body_t));
				memset(pLoader->pHandlerInfo->agentInfo, 0, sizeof(cagent_agent_info_body_t));
				memcpy(pLoader->pHandlerInfo->agentInfo, m_plugininfo->pHandlerInfo->agentInfo, sizeof(cagent_agent_info_body_t));
				//printf("***%s malloc:<%d>\r\n", name, pLoader->pHandlerInfo->agentInfo);
			}
			snprintf( pLoader->pHandlerInfo->Name, sizeof( pLoader->pHandlerInfo->Name ), "%s", VirName );
		}
		snprintf( pLoader->Name, sizeof( pLoader->Name ), "%s", VirName );
		SALoaderLog(Normal, "Load Virtual %s Successful\n", pLoader->Name );
		pLoader->type=virtual_handler;
	}


	return true;
}
bool HANDLERLOADER_API Loader_LoadVirtualHandler(Handler_List_t *pLoaderList, char *VirName, char *HandlerName)
{

	Handler_Loader_Interface *pLoader=NULL;

	printf("VirName : %s\n", VirName);

	if(!pLoaderList) return false;

	if(Loader_FindHandler(pLoaderList, VirName) != NULL)
		return false;

	pLoader = (Handler_Loader_Interface *)malloc(sizeof(Handler_Loader_Interface));
	memset(pLoader, 0, sizeof(Handler_Loader_Interface));
	if(Handler_LoadVirtualHandler(pLoaderList,pLoader, VirName, HandlerName) == true)
	{
			Handler_Loader_Interface *pLastLoader = Loader_GetLastHandler(pLoaderList);
			if(pLastLoader == NULL)
			{
				pLoaderList->items = pLoader;
			}
			else
			{
				pLastLoader->next = pLoader;
				pLoader->prev = pLastLoader;
			}
			pLoaderList->total++;
	}
	
	return true;
}
int HANDLERLOADER_API Loader_LoadHandler(Handler_List_t *pLoaderList, char const *handlerpath, char const *name)
{
	Handler_Loader_Interface *pLoader = NULL;
	if(!pLoaderList) return false;
	pLoader = (Handler_Loader_Interface *)malloc(sizeof(Handler_Loader_Interface));
	memset(pLoader, 0, sizeof(Handler_Loader_Interface));
	if(Handler_LoadHandler(pLoader, handlerpath, name) == true)
	{
		if(Handler_InitHandler( pLoader, g_sendcbf, g_sendcustcbf, g_subscribecustcbf, g_sendreportcbf, g_sendcapabilitycbf) == false)
		{
			Handler_ReleaseHandler(pLoader);
			free(pLoader);
			pLoader = NULL;
		}
		else
		{
			Handler_Loader_Interface *pLastLoader = Loader_GetLastHandler(pLoaderList);
			if(pLastLoader == NULL)
			{
				pLoaderList->items = pLoader;
			}
			else
			{
				pLastLoader->next = pLoader;
				pLoader->prev = pLastLoader;
			}
			pLoaderList->total++;
			return true;
		}
	}
	free(pLoader);
	pLoader = NULL;
	return false;
}

int HANDLERLOADER_API Loader_ReleaseHandler(Handler_List_t *pLoaderList, Handler_Loader_Interface *pLoader)
{
	Handler_Loader_Interface *pPrevLoader = NULL, *pNextLoader = NULL;
	Handler_Loader_Interface *pCurLoader = Loader_FindHandler(pLoaderList, pLoader->Name);
	if(!pCurLoader) return false;
	pNextLoader = pCurLoader->next;
	pPrevLoader = pCurLoader->prev;

	if(Handler_ReleaseHandler(pCurLoader) == true)
	{
		pPrevLoader->next = pNextLoader;
		pNextLoader->prev = pPrevLoader;
		pLoaderList->total--;

		free(pCurLoader);
		pCurLoader = NULL;
		return true;
	}
	return false;
}

int HANDLERLOADER_API Loader_LoadAllHandler(Handler_List_t *pLoaderList, char const * workdir)
{
	int iNum = 0;
	int i = 0;
	char sValue[128] = {0};
	char sItemName[32]={0};
	char stModuleName[64]={0};
	char configFile[MAX_PATH]={0};
	char moduleFile[MAX_PATH]={0};
	
	if(!pLoaderList) return 0;
	if(workdir != NULL)
	{
		util_path_combine(configFile, workdir, MODULE_CONFIG_FILE);
	}
	else
	{
		strcpy(configFile, MODULE_CONFIG_FILE);
	}

	if(module_get(configFile, MODULE_NUM, sValue, sizeof(sValue))>0) 
	{	
		iNum = atoi(sValue);
		if( iNum > 0 ) 
		{
			for( i=0; i < iNum; i++ ) 
			{
				memset(sItemName,0,sizeof(sItemName));
				memset(sValue,0,sizeof(sValue));
				memset(stModuleName,0,sizeof(stModuleName));
				// 1. Enable?
				snprintf( sItemName, sizeof( sItemName ), "ModuleEnable%d",i+1 );
				if( module_get(configFile, sItemName, sValue, sizeof(sValue)) > 0 ){				
					 if( strcasecmp(sValue,"true") )
						 continue;
				}
				memset(sItemName,0,sizeof(sItemName));
				memset(sValue,0,sizeof(sValue));
				// 2. Name
				snprintf( sItemName, sizeof( sItemName ), "%s%d",MODULE_NAME,i+1 );
				module_get(configFile, sItemName, stModuleName, sizeof(stModuleName));
				memset(sItemName,0,sizeof(sItemName));
				// 3. Path
				snprintf( sItemName, sizeof( sItemName ), "%s%d",MODULE_PATH,i+1 );
				if( module_get(configFile, sItemName, sValue, sizeof(sValue)) > 0 ) 
				{
					memset(moduleFile,0,sizeof(moduleFile));
					if(workdir != NULL)
					{
						util_path_combine(moduleFile, workdir, sValue);
					}
					else
					{
						strcpy(moduleFile, sValue);
					}
					if(Loader_LoadHandler(pLoaderList, moduleFile, stModuleName) == false)
					{
						SALoaderLog(Error, "Can't Load Module Path %s\n", moduleFile );
					}					
				}	// End of cfg_get ( MODULE_PATH )
				else
				{
					SALoaderLog(Error, "Can't Find Module Path %s\n", moduleFile );
				}
			}	// End of For 
		}	// End of cfg_get ( MODULE_NUM )
		else {
			SALoaderLog(Error, "Module XML is not any handler module need to load\n");
		}
	} else {
		SALoaderLog(Error, "Can't not load Module XML Conifg %s\n", MODULE_CONFIG_FILE );
	}
	return pLoaderList->total;
}

void HANDLERLOADER_API Loader_StartAllHandler(Handler_List_t *pLoaderList)
{
	Handler_Loader_Interface *pInterfaceTmp = NULL;
	if(!pLoaderList) return;
	pInterfaceTmp = pLoaderList->items;
	while(pInterfaceTmp)
	{
		if(pInterfaceTmp->type!=virtual_handler)
			Handler_StartHandler( pInterfaceTmp );
		pInterfaceTmp = pInterfaceTmp->next;
	}
}

void HANDLERLOADER_API Loader_StopAllHandler(Handler_List_t *pLoaderList)
{
	Handler_Loader_Interface *pInterfaceTmp = NULL;
	if(!pLoaderList) return;
	pInterfaceTmp = pLoaderList->items;
	while(pInterfaceTmp)
	{
		if(pInterfaceTmp->type!=virtual_handler)
			Handler_StopHandler( pInterfaceTmp );
		pInterfaceTmp = pInterfaceTmp->next;
	}
}

void HANDLERLOADER_API Loader_ReleaseAllHandler(Handler_List_t *pLoaderList)
{
	Handler_Loader_Interface *pInterfaceTmp = NULL, *pInterfaceCur = NULL;
	if(!pLoaderList) return;
	pInterfaceTmp = Loader_GetLastHandler(pLoaderList);
	while(pInterfaceTmp)
	{
		pInterfaceCur = pInterfaceTmp;
		pInterfaceTmp = pInterfaceTmp->prev;
		if(pInterfaceCur->Workable)
		{
			if(pInterfaceCur->Handler)
				Handler_ReleaseHandler( pInterfaceCur );
		}
		if(pInterfaceCur->pHandlerInfo)
		{
			if(pInterfaceCur->pHandlerInfo->agentInfo)
				free(pInterfaceCur->pHandlerInfo->agentInfo);
			pInterfaceCur->pHandlerInfo->agentInfo = NULL;
			free(pInterfaceCur->pHandlerInfo);
		}
		pInterfaceCur->pHandlerInfo = NULL;
		free(pInterfaceCur);
		pInterfaceCur = NULL;
		pLoaderList->total--;
	}
	if(pLoaderList->total == 0)
		pLoaderList->items = NULL;
}

static void* HandlerReleaseThread(void*  args)
{
	Handler_Loader_Interface *pInterfaceCur = (Handler_Loader_Interface *)args;

	if(pInterfaceCur)
	{
		if(pInterfaceCur->type!=virtual_handler)
			Handler_StopHandler( pInterfaceCur );

		if(pInterfaceCur->Workable)
		{
			if(pInterfaceCur->Handler)
				Handler_ReleaseHandler( pInterfaceCur );
		}
		
		if(pInterfaceCur->pHandlerInfo)
		{
			if(pInterfaceCur->pHandlerInfo->agentInfo)
			{
				free(pInterfaceCur->pHandlerInfo->agentInfo);
				//printf("***%s free:<%d>\r\n", pInterfaceCur->pHandlerInfo->Name, pInterfaceCur->pHandlerInfo->agentInfo);
			}
			pInterfaceCur->pHandlerInfo->agentInfo = NULL;
			free(pInterfaceCur->pHandlerInfo);
		}
		pInterfaceCur->pHandlerInfo = NULL;
		free(pInterfaceCur);
		pInterfaceCur = NULL;

		
	}
	pthread_exit(0);
	return 0;
}

void HANDLERLOADER_API Loader_ConcurrentReleaseAllHandler(Handler_List_t *pLoaderList)
{
	Handler_Loader_Interface *pInterfaceTmp = NULL, *pInterfaceCur = NULL;
	int total = pLoaderList->total;
	void **arrThreadHandler = malloc(sizeof(void *) * total);
	int i=0;
	if(!pLoaderList) return;
	memset(arrThreadHandler, 0, sizeof(void *) * total); 
	pInterfaceTmp = Loader_GetLastHandler(pLoaderList);
	while(pInterfaceTmp)
	{
		
		pInterfaceCur = pInterfaceTmp;
		pInterfaceTmp = pInterfaceTmp->prev;

		if (pthread_create((pthread_t *)&arrThreadHandler[i], NULL, HandlerReleaseThread, pInterfaceCur) != 0)
		{
			SALoaderLog(Error, " %s> start handler release thread failed!", pInterfaceCur->Name);	
		}
		i++;

	}


	for (i = 0; i < total; i++) 
	{
		if(arrThreadHandler[i])
		{
			pthread_join((pthread_t)arrThreadHandler[i], NULL);
			arrThreadHandler[i] = NULL;
		}
		pLoaderList->total--;
	}
	free(arrThreadHandler);

	if(pLoaderList->total == 0)
		pLoaderList->items = NULL;
}


int Handler_LoadHandler(Handler_Loader_Interface *pLoader, char const *handlerpath, char const *name )
{
	void* handler = NULL;
	if( pLoader == NULL ) return false;

	// 1. Load DLL
	if(!util_dlopen((char *) handlerpath, &handler))
	{
		char* err = util_dlerror();
		SALoaderLog(Error, "Failed to load %s: %s\n", handlerpath, err);
		free(err);
		return false;
	}
	if(pLoader->pHandlerInfo)
	{
		if(pLoader->pHandlerInfo->agentInfo)
		{
			free(pLoader->pHandlerInfo->agentInfo);
			//printf("***%s free:<%d>\r\n", pLoader->pHandlerInfo->Name, pLoader->pHandlerInfo->agentInfo);
		}
		free(pLoader->pHandlerInfo);
		pLoader->pHandlerInfo = NULL;
	}

	memcpy(pLoader, m_plugininfo, sizeof(Handler_Loader_Interface));
		
	pLoader->Handler = handler;
	// 2. Load API Symbol from Handler Library
	pLoader->Handler_Init_API				= (HANDLER_INITLIZE) util_dlsym( pLoader->Handler, "Handler_Initialize");
	pLoader->Handler_Get_Status_API			= (HANDLER_GET_STATUS) util_dlsym( pLoader->Handler, "Handler_Get_Status");	
	pLoader->Handler_OnStatusChange_API		= (HANDLER_ONSTATUSCAHNGE) util_dlsym( pLoader->Handler, "Handler_OnStatusChange");
	pLoader->Handler_Start_API				= (HANDLER_START) util_dlsym( pLoader->Handler, "Handler_Start");
	pLoader->Handler_Stop_API				= (HANDLER_STOP) util_dlsym( pLoader->Handler, "Handler_Stop");
	pLoader->Handler_Recv_API				= (HANDLER_RECV) util_dlsym( pLoader->Handler, "Handler_Recv");
	pLoader->Handler_Get_Capability_API		= (HANDLER_GET_CAPABILITY) util_dlsym( pLoader->Handler, "Handler_Get_Capability");
	pLoader->Handler_AutoReportStart_API	= (HANDLER_AUTOREPORT_START) util_dlsym( pLoader->Handler, "Handler_AutoReportStart");
	pLoader->Handler_AutoReportStop_API		= (HANDLER_AUTOREPORT_STOP) util_dlsym( pLoader->Handler, "Handler_AutoReportStop");
	pLoader->Handler_MemoryFree_API			= (HANDLER_MEMORYFREE) util_dlsym( pLoader->Handler, "Handler_MemoryFree");

	// 3. Check All Function are OK
	if(  pLoader->Handler_Init_API == NULL || pLoader->Handler_Get_Status_API == NULL ||
		pLoader->Handler_OnStatusChange_API == NULL || pLoader->Handler_Start_API  == NULL ||
		pLoader->Handler_Stop_API == NULL )
	{
		util_dlclose(pLoader->Handler);
		pLoader->Handler = handler = NULL;
		SALoaderLog(Error, "Failed: Not All Handler Function Found on %s\n", handlerpath);
		return false;
	}
	if(m_plugininfo->pHandlerInfo)
	{
		pLoader->pHandlerInfo = malloc(sizeof(HANDLER_INFO_EX));
		memset(pLoader->pHandlerInfo, 0, sizeof(HANDLER_INFO_EX));
		memcpy(pLoader->pHandlerInfo, m_plugininfo->pHandlerInfo, sizeof(HANDLER_INFO_EX));
		if(m_plugininfo->pHandlerInfo->agentInfo)
		{
			pLoader->pHandlerInfo->agentInfo = malloc(sizeof(cagent_agent_info_body_t));
			memset(pLoader->pHandlerInfo->agentInfo, 0, sizeof(cagent_agent_info_body_t));
			memcpy(pLoader->pHandlerInfo->agentInfo, m_plugininfo->pHandlerInfo->agentInfo, sizeof(cagent_agent_info_body_t));
			//printf("***%s malloc:<%d>\r\n", name, pLoader->pHandlerInfo->agentInfo);
		}
		snprintf( pLoader->pHandlerInfo->Name, sizeof( pLoader->pHandlerInfo->Name ), "%s", name );
	}
	snprintf( pLoader->Name, sizeof( pLoader->Name ), "%s", name );
	SALoaderLog(Normal, "Load %s Successful\n", pLoader->Name );
	pLoader->type = user_handler;
	
	return true;
}

int Handler_InitHandler( Handler_Loader_Interface *pLoader, HandlerSendCbf cbf , HandlerSendCustCbf cbfcust, HandlerSubscribeCustCbf cbfsubscribe, HandlerAutoReportCbf cbfreport, HandlerSendCapabilityCbf cbfinfo)
{
	int len;
	//pLoader->pluginHandlerInfo.sendcbf = cbf;

	if( pLoader->Handler_Init_API( pLoader->pHandlerInfo ) == handler_fail )
	{
		SALoaderLog(Error, "Initialize Handler Failed: %s\n", pLoader->Name );
		return false;
	}
	else
	{
		if( ( len = strlen(pLoader->pHandlerInfo->Name) ) == 0 )
			SALoaderLog(Warning, "Handler %s have not set the Topic\n", pLoader->Name );

		pLoader->Workable = true;
		return true;
	}
	return false;
}

int Handler_GetHandlerStatus( Handler_Loader_Interface *pLoader, HANDLER_THREAD_STATUS * pOutStatus )
{
	if( pLoader == NULL ) return false;

	if(pLoader->type!=virtual_handler)
	{
		if( pLoader->Handler_Get_Status_API )
			if(pLoader->Handler_Get_Status_API(pOutStatus ) == handler_success)
				return true;
			else
				return false;
		else
			return false;
	}
	return true;
}

int Handler_StartHandler( Handler_Loader_Interface *pLoader )
{
	if( pLoader == NULL ) return false;

	if( pLoader->Handler_Start_API )
		if(pLoader->Handler_Start_API() == handler_success)
			return true;
		else
			return false;
	else
		return false;
}

int Handler_StopHandler( Handler_Loader_Interface *pLoader )
{
	if( pLoader == NULL ) return false;

	if( pLoader->Handler_Stop_API )
		if(pLoader->Handler_Stop_API() == handler_success)
			return true;
		else
			return false;
	else
		return false;
}

void Handler_HandlerRecv( Handler_Loader_Interface *pLoader, char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2)
{
	if( pLoader == NULL ) return;

	if( pLoader->Handler_Recv_API )
		pLoader->Handler_Recv_API(topic, data, datalen, pRev1, pRev2);
}
 
int Handler_ReleaseHandler(Handler_Loader_Interface *pLoader)
{
	if(!pLoader->Handler)
		return true;
	if(pLoader->type!=virtual_handler)
	{
		if(util_dlclose(pLoader->Handler) == 0)
		{
			pLoader->Handler = NULL;
			SALoaderLog(Normal, "Release %s Successful\n", pLoader->Name );
			return true;
		}
		else
			return false;
	}

	return true;
}
