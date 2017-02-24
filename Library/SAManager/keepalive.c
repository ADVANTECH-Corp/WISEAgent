#include "keepalive.h"
#include "SAManagerLog.h"
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "NamedPipeClient.h"

#ifdef WIN32
#define DEF_PIPE_NAME              "\\\\.\\pipe\\SAWatchdogCommPipe"
#else
#define DEF_PIPE_NAME              "/tmp/SAWatchdogFifo"
#endif
#define DEF_COMM_ID                (1)
#define DEF_KEEPALIVE_INTERVAL_S   (2)

#define DEF_MAX_SELF_CPU_USAGE     70
#define DEF_MAX_SELF_MEM_USAGE     80000   //KB

#define DEF_KEEPALIVE_TIME_S         3
#define DEF_KEEPALIVE_TRY_TIMES      3

typedef enum WatchObjType
{
	FORM_PROCESS,
	NO_FORM_PROCESS,
	WIN_SERVICE,
}WATCHOBJTYPE;

typedef enum WatchCmdKey{
	START_WATCH,
	KEEPALIVE,
	BUSY_WAIT,
	STOP_WATCH,
}WATCHCMDKEY;

typedef union WatchParams{
	unsigned long busyWaitTimeS;
	struct {
		WATCHOBJTYPE  objType;
		unsigned long watchPID;
	}starWatchInfo;
}WATCHPARAMS;

typedef struct WatchMessage{
	unsigned long  commID;
	WATCHCMDKEY  commCmd;
	WATCHPARAMS  commParams;
}WATCHMSG, *PWATCHMSG;

struct kepalive_ctx{
	pthread_t		threadHandler;
	bool			isThreadRunning;
	Handler_List_t	*pHandlerList;
};

LOGHANDLE g_keepalivelogger = NULL;
struct kepalive_ctx g_kepalivectx;

typedef struct handler_countdown
{    
	char name[128];
	int count;
	int limit;
	struct handler_countdown *prev;
	struct handler_countdown *next;
} handler_countdown_st;

static handler_countdown_st *handlers_countdown = NULL;

handler_countdown_st * GetLastHandlerCD()
{
	handler_countdown_st *handlers = handlers_countdown;
	handler_countdown_st *target = NULL;
	//printf("Find Last\n"); 
	while(handlers != NULL)
	{
		//printf("Topic Name: %s\n", topic->name);
		target = handlers;
		handlers = handlers->next;
	}
	return target;
}

handler_countdown_st * AddhHandlerCD(char const * name, int limit)
{
	handler_countdown_st *handler = NULL;
	
	if (name == NULL)
		return NULL;
	
	handler = (handler_countdown_st *)malloc(sizeof(handler_countdown_st));

	strncpy(handler->name, name, strlen(name)+1);
	handler->count = 0;
	handler->limit = limit;
	handler->next = NULL;	
	handler->prev = NULL;	

	if(handlers_countdown == NULL)
	{
		handlers_countdown = handler;
	} else {
		handler_countdown_st *lastone = GetLastHandlerCD();
		//printf("Last Topic Name: %s\n", lasttopic->name);
		lastone->next = handler;
		handler->prev = lastone;
	}
	return handler;
}

void RemoveHandlerCD(char* name)
{
	handler_countdown_st *handler = handlers_countdown;
	handler_countdown_st *target = NULL;
	//printf("Remove Topic\n");
	while(handler != NULL)
	{
		//printf("Topic Name: %s\n", topic->name);
		if(strcmp(handler->name, name) == 0)
		{
			if(handlers_countdown == handler)
				handlers_countdown = handler->next;
			if(handler->prev != NULL)
				handler->prev->next = handler->next;
			if(handler->next != NULL)
				handler->next->prev = handler->prev;
			target = handler;
			break;
		}
		handler = handler->next;
	}
	if(target!=NULL)
	{
		free(target);
		target = NULL;
	}
}

handler_countdown_st * FindHandlerCD(char const * name)
{
	handler_countdown_st *handler = handlers_countdown;
	handler_countdown_st *target = NULL;

	//printf("Find Topic\n");
	while(handler != NULL)
	{
		//printf("Topic Name: %s\n", topic->name);
		if(strcmp(handler->name, name) == 0)
		{
			target = handler;
			break;
		}
		handler = handler->next;
	}
	return target;
}

void CheckHandlerStatus(struct kepalive_ctx* ctx)
{
	Handler_Loader_Interface *pInterfaceTmp = NULL;
	if(!ctx)
		return;
	pInterfaceTmp = ctx->pHandlerList->items;
	while(pInterfaceTmp)
	{
		HANDLER_THREAD_STATUS pOutStatus;
		bool bRestart = false;
		if(pInterfaceTmp->Workable == false)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}
		if(pInterfaceTmp->type != user_handler)
		{
			pInterfaceTmp = pInterfaceTmp->next;
			continue;
		}

		if(!ctx->isThreadRunning)
			break;

		if(pInterfaceTmp->Handler_Get_Status_API)
		{
			handler_result result = pInterfaceTmp->Handler_Get_Status_API(&pOutStatus);
			if(result == handler_success)
			{
				handler_countdown_st *phandlercd = FindHandlerCD(pInterfaceTmp->Name);
				if(pOutStatus == handler_status_busy)
				{
					if(!phandlercd)
					{
						phandlercd = AddhHandlerCD(pInterfaceTmp->Name, 3);
					}
					phandlercd->count++;
					SAManagerLog(g_keepalivelogger, Warning, "Handler %s is busy count %d!", pInterfaceTmp->Name, phandlercd->count);
					if(phandlercd->count >= phandlercd->limit)
					{
						bRestart = true;
						RemoveHandlerCD(pInterfaceTmp->Name);
					}
				}
				else if(phandlercd)
				{
					RemoveHandlerCD(pInterfaceTmp->Name);
				}
			}
		}
		
		if(!ctx->isThreadRunning)
			break;

		if(bRestart)
		{
			if(pInterfaceTmp->Handler_Stop_API)
			{
				SAManagerLog(g_keepalivelogger, Warning, "Stop Handler %s!", pInterfaceTmp->Name);
				pInterfaceTmp->Handler_Stop_API();
			}

			if(!ctx->isThreadRunning)
				break;

			if(pInterfaceTmp->Handler_Start_API)
			{
				SAManagerLog(g_keepalivelogger, Warning, "Restart Handler %s!", pInterfaceTmp->Name);
				pInterfaceTmp->Handler_Start_API();
			}
		}
		pInterfaceTmp = pInterfaceTmp->next;
	}
}

void* threat_keepalive(void* args)
{
	struct kepalive_ctx* ctx = (struct kepalive_ctx*)args;
//#ifdef WIN32
	PIPECLINETHANDLE pipeClientHandle = NULL;
	WATCHMSG watchMsg;
//#endif
	bool isLogConnectFail = true;
	bool bRet = true;
	while(ctx->isThreadRunning)
	{
//#ifdef WIN32
		pipeClientHandle = NamedPipeClientConnect(DEF_PIPE_NAME, DEF_COMM_ID, NULL);
		if(pipeClientHandle)
		{
			usleep(1000*1000); //On CentOS, NamedPipeClient Connect need more time
			SAManagerLog(g_keepalivelogger, Normal, "NamedPipe: %s, CommID: %d, IPC Connect successfully!", DEF_PIPE_NAME, DEF_COMM_ID);
			isLogConnectFail = true;
			memset(&watchMsg, 0, sizeof(WATCHMSG));
			watchMsg.commCmd = START_WATCH;
			watchMsg.commID = DEF_COMM_ID;
			watchMsg.commParams.starWatchInfo.objType = WIN_SERVICE;
			watchMsg.commParams.starWatchInfo.watchPID = getpid();
			bRet = NamedPipeClientSend(pipeClientHandle, (char *)&watchMsg, sizeof(WATCHMSG));
			if(!bRet)
			{
				SAManagerLog(g_keepalivelogger, Error, "NamedPipe: %s, CommID: %d, Start watch failed!", DEF_PIPE_NAME, DEF_COMM_ID);
				goto done;	
			}
			usleep(1000*1000); //On CentOS, NamedPipeClient Send Packet need more time
			while(ctx->isThreadRunning)
			{
//#endif
				CheckHandlerStatus(ctx);
//#ifdef WIN32
				//memset(&watchMsg, 0, sizeof(WATCHMSG));
				watchMsg.commCmd = KEEPALIVE;
				watchMsg.commID = DEF_COMM_ID;
				bRet = NamedPipeClientSend(pipeClientHandle, (char *)&watchMsg, sizeof(WATCHMSG));
				if(!bRet)
				{
					//SAManagerLog(g_keepalivelogger, Error, "NamedPipe: %s, CommID: %d, Send keepalive failed!", DEF_PIPE_NAME, DEF_COMM_ID);
					goto done;
				}
//#endif
				usleep(DEF_KEEPALIVE_INTERVAL_S*1000*1000);
//#ifdef WIN32
			}
			//memset(&watchMsg, 0, sizeof(WATCHMSG));
			watchMsg.commCmd = STOP_WATCH;
			watchMsg.commID = DEF_COMM_ID;
			bRet = NamedPipeClientSend(pipeClientHandle, (char *)&watchMsg, sizeof(WATCHMSG));
			if(!bRet) SAManagerLog(g_keepalivelogger, Error, "NamedPipe: %s, CommID: %d, Stop watch failed!", DEF_PIPE_NAME, DEF_COMM_ID);
			else SAManagerLog(g_keepalivelogger, Normal, "NamedPipe: %s, CommID: %d, Stop watch successfully!", DEF_PIPE_NAME, DEF_COMM_ID);       
		}
		else
		{
			if(isLogConnectFail)
			{//Continuous failure log only once
				SAManagerLog(g_keepalivelogger,Error, "NamedPipe: %s, CommID: %d, IPC Connect failed!", DEF_PIPE_NAME, DEF_COMM_ID);
				isLogConnectFail = false;
			}
		}		
		
	done:
		if(pipeClientHandle)NamedPipeClientDisconnect(pipeClientHandle);

		usleep(1000*1000);
//#endif
	}
	
	pthread_exit(0);
	return 0;
}

void keepalive_initialize(Handler_List_t *pLoaderList, void * logger)
{
	g_keepalivelogger = logger;
	
	if(g_kepalivectx.threadHandler)
	{
		pthread_join(g_kepalivectx.threadHandler, NULL);
		g_kepalivectx.threadHandler = 0;
	}
	
	memset(&g_kepalivectx, 0, sizeof(struct kepalive_ctx));

	g_kepalivectx.isThreadRunning = true;
	if(pthread_create(&g_kepalivectx.threadHandler, NULL, threat_keepalive, &g_kepalivectx)==0)
		g_kepalivectx.pHandlerList = pLoaderList;
}

void keepalive_uninitialize()
{
	if(g_kepalivectx.threadHandler)
	{
		g_kepalivectx.isThreadRunning = false;
		pthread_join(g_kepalivectx.threadHandler, NULL);
		g_kepalivectx.threadHandler = 0;
	}
}