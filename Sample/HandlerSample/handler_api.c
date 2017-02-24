/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/07/14 by Scott Chang									*/
/* Modified Date: 2016/07/14 by Scott Chang									*/
/* Abstract     : Sample Handler to parse json string in c:/test.txt file	*/
/*                and report to server.										*/	
/* Reference    : None														*/
/****************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "AdvPlatform.h"
#include <cJSON.h>
#include "susiaccess_handler_api.h"

#include "DeviceMessageGenerate.h"
#include "IoTMessageGenerate.h"
#include "FlatToIPSO.h"

//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------
#define cagent_request_custom 2002 
#define cagent_custom_action 30002
const char strPluginName[MAX_TOPIC_LEN] = {"HandlerSample"}; /*declare the handler name*/
const int iRequestID = cagent_request_custom; /*define the request ID for V3.0, not used on V3.1 or later*/
const int iActionID = cagent_custom_action;  /*define the action ID for V3.0, not used on V3.1 or later*/

MSG_CLASSIFY_T *g_Capability = NULL; /*the global message structure to describe the sensor data as the handelr capability*/
//-----------------------------------------------------------------------------
// Internal Prototypes:
//-----------------------------------------------------------------------------
//
typedef struct{
   void* threadHandler; // thread handle
   int interval; // time interval for file read
   bool isThreadRunning; //thread running flag
}handler_context_t;

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
static Handler_info  g_PluginInfo; //global Handler info structure
static handler_context_t g_HandlerContex;
static HANDLER_THREAD_STATUS g_status = handler_status_no_init; // global status flag.
static bool m_bAutoReprot = false;
static time_t g_monitortime;
static HandlerSendCbf  g_sendcbf = NULL;						// Client Send information (in JSON format) to Cloud Server	
static HandlerSendCustCbf  g_sendcustcbf = NULL;			    // Client Send information (in JSON format) to Cloud Server with custom topic	
static HandlerSubscribeCustCbf g_subscribecustcbf = NULL;       // Client subscribe message (in JSON format) send from Cloud Server with custom topic	
static HandlerAutoReportCbf g_sendreportcbf = NULL;				// Client Send report (in JSON format) to Cloud Server with AutoReport topic
static HandlerSendCapabilityCbf g_sendcapabilitycbf = NULL;		// Client Send capability (in JSON format) to Cloud Server
static HandlerSendEventCbf g_sendeventcbf = NULL;				// Client Send event notify (in JSON format) to Cloud Server with eventnotify topic
//-----------------------------------------------------------------------------
// Function:
//-----------------------------------------------------------------------------
void Handler_Uninitialize();

#ifdef _MSC_VER
BOOL WINAPI DllMain(HINSTANCE module_handle, DWORD reason_for_call, LPVOID reserved)
{
	if (reason_for_call == DLL_PROCESS_ATTACH) // Self-explanatory
	{
		printf("DllInitializer\r\n");
		DisableThreadLibraryCalls(module_handle); // Disable DllMain calls for DLL_THREAD_*
		if (reserved == NULL) // Dynamic load
		{
			// Initialize your stuff or whatever
			// Return FALSE if you don't want your module to be dynamically loaded
		}
		else // Static load
		{
			// Return FALSE if you don't want your module to be statically loaded
			return FALSE;
		}
	}

	if (reason_for_call == DLL_PROCESS_DETACH) // Self-explanatory
	{
		printf("DllFinalizer\r\n");
		if (reserved == NULL) // Either loading the DLL has failed or FreeLibrary was called
		{
			// Cleanup
			Handler_Uninitialize();
		}
		else // Process is terminating
		{
			// Cleanup
			Handler_Uninitialize();
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
    printf("DllInitializer\r\n");
}

__attribute__((destructor))
/** 
 * It is called when shared lib is being unloaded.
 * 
 */
static void Finalizer()
{
    printf("DllFinalizer\r\n");
	Handler_Uninitialize();
}
#endif

static char buff[1024]={0};
const char* FileRead()
{
	/*read string form file: test.txt*/
	FILE *fpSrc = fopen("c:/test.txt", "r");
	if(fpSrc)
	{
		fgets(buff, 1024, fpSrc);
		fclose(fpSrc);
	}
	fpSrc = NULL;
	return buff;
}

bool ParseReceivedData(MSG_CLASSIFY_T *pGroup)
{
	/*Parse the JSON string to Message Structure*/
	/*Data String: {"<tag>":<Number>, "<tag>":true, "<tag>":false, "<tag>":null, "<tag>":"<string>"}*/
	char* data = (char *)FileRead();
	if(!data) return false;
	if(strlen(data)<=0) return false;	
	return transfer_parse_json(data, pGroup);
}


MSG_CLASSIFY_T * CreateCapability()
{
	/*Generate the Handler Capability to describe the sensor data*/
	MSG_CLASSIFY_T* myCapability = IoT_CreateRoot((char*)strPluginName);
	MSG_CLASSIFY_T* myGroup = IoT_AddGroup(myCapability, strPluginName);

	ParseReceivedData(myGroup);

	return myCapability;
}

void* SampleHandlerReportThread(void *args)
{
	/*thread to read file and update sensor data repeatedly.*/

	handler_context_t *pHandlerContex = (handler_context_t *)args;
	int mInterval = pHandlerContex->interval * 1000;

	if(!g_Capability)
		g_Capability = CreateCapability();

	while(pHandlerContex->isThreadRunning)
	{

		if(g_Capability)
		{
			char* message = NULL;
			MSG_CLASSIFY_T *myGroup = IoT_FindGroup(g_Capability, strPluginName);
			if(myGroup)
				ParseReceivedData(myGroup);
			message = IoT_PrintData(g_Capability);

			if(g_sendreportcbf)
				g_sendreportcbf(/*Handler Info*/&g_PluginInfo, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);

			free(message);
		}
		usleep(mInterval*1000);
	}
	pthread_exit(0);
	return 0;
}

/* **************************************************************************************
 *  Function Name: Handler_Initialize
 *  Description: Init any objects or variables of this handler
 *  Input :  PLUGIN_INFO *pluginfo
 *  Output: None
 *  Return:  0  : Success Init Handler
 *              -1 : Fail Init Handler
 * ***************************************************************************************/
int HANDLER_API Handler_Initialize( HANDLER_INFO *pluginfo )
{
	if( pluginfo == NULL )
		return handler_fail;

	// 1. Topic of this handler
	snprintf( pluginfo->Name, sizeof(pluginfo->Name), "%s", strPluginName );
	pluginfo->RequestID = iRequestID;
	pluginfo->ActionID = iActionID;
	printf(" %s> Initialize", strPluginName);
	// 2. Copy agent info 
	memcpy(&g_PluginInfo, pluginfo, sizeof(HANDLER_INFO));
	g_PluginInfo.agentInfo = pluginfo->agentInfo;

	// 3. Callback function -> Send JSON Data by this callback function
	g_sendcbf = g_PluginInfo.sendcbf = pluginfo->sendcbf;
	g_sendcustcbf = g_PluginInfo.sendcustcbf = pluginfo->sendcustcbf;
	g_subscribecustcbf = g_PluginInfo.subscribecustcbf = pluginfo->subscribecustcbf;
	g_sendreportcbf = g_PluginInfo.sendreportcbf = pluginfo->sendreportcbf;
	g_sendcapabilitycbf =g_PluginInfo.sendcapabilitycbf = pluginfo->sendcapabilitycbf;
	g_sendeventcbf = g_PluginInfo.sendeventcbf = pluginfo->sendeventcbf;

	g_HandlerContex.threadHandler = NULL;
	g_HandlerContex.isThreadRunning = false;
	g_status = handler_status_no_init;

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: Handler_Uninitialize
 *  Description: Release the objects or variables used in this handler
 *  Input :  None
 *  Output: None
 *  Return:  void
 * ***************************************************************************************/
void Handler_Uninitialize()
{
	/*Stop read text file thread*/
	if(g_HandlerContex.threadHandler)
	{
		g_HandlerContex.isThreadRunning = false;
		pthread_join(g_HandlerContex.threadHandler, NULL);
		g_HandlerContex.threadHandler = NULL;
	}
	g_sendcbf = NULL;
	g_sendcustcbf = NULL;
	g_sendreportcbf = NULL;
	g_sendcapabilitycbf = NULL;
	g_subscribecustcbf = NULL;
	g_sendeventcbf = NULL;
	/*Release Capability Message Structure*/
	if(g_Capability)
	{
		IoT_ReleaseAll(g_Capability);
		g_Capability = NULL;
	}
}

/* **************************************************************************************
 *  Function Name: Handler_Get_Status
 *  Description: Get Handler Threads Status. CAgent will restart current Handler or restart CAgent self if busy.
 *  Input :  None
 *  Output: char * : pOutStatus       // cagent handler status
 *  Return:  handler_success  : Success Init Handler
 *			 handler_fail : Fail Init Handler
 * **************************************************************************************/
int HANDLER_API Handler_Get_Status( HANDLER_THREAD_STATUS * pOutStatus )
{
	int iRet = handler_fail; 
	printf(" %s> Get Status", strPluginName);
	if(!pOutStatus) return iRet;
	/*user need to implement their thread status check function*/
	*pOutStatus = g_status;

	iRet = handler_success;
	return iRet;
}


/* **************************************************************************************
 *  Function Name: Handler_OnStatusChange
 *  Description: Agent can notify handler the status is changed.
 *  Input :  PLUGIN_INFO *pluginfo
 *  Output: None
 *  Return:  None
 * ***************************************************************************************/
void HANDLER_API Handler_OnStatusChange( HANDLER_INFO *pluginfo )
{
	char message[256] = {0};
	char* msg = NULL;
	MSG_CLASSIFY_T* evt = NULL;
	printf(" %s> Update Status", strPluginName);
	if(pluginfo)
		memcpy(&g_PluginInfo, pluginfo, sizeof(HANDLER_INFO));
	else
	{
		memset(&g_PluginInfo, 0, sizeof(HANDLER_INFO));
		snprintf( g_PluginInfo.Name, sizeof( g_PluginInfo.Name), "%s", strPluginName );
		g_PluginInfo.RequestID = iRequestID;
		g_PluginInfo.ActionID = iActionID;
	}
	sprintf(message, "Handler %s on connected", strPluginName);
	/*Sample code to generate and send event message*/
	evt = DEV_CreateEventNotify("message", message);
	msg = DEV_PrintUnformatted(evt);
	DEV_ReleaseDevice(evt);
	if(g_sendeventcbf)
		g_sendeventcbf(&g_PluginInfo, Severity_Informational, msg, strlen(msg), NULL, NULL);
	free(msg);
}


/* **************************************************************************************
 *  Function Name: Handler_Start
 *  Description: Start Running
 *  Input :  None
 *  Output: None
 *  Return:  0  : Success Init Handler
 *              -1 : Fail Init Handler
 * ***************************************************************************************/
int HANDLER_API Handler_Start( void )
{
	printf("> %s Start", strPluginName);

	g_status = handler_status_start;
	return handler_success;
}

/* **************************************************************************************
 *  Function Name: Handler_Stop
 *  Description: Stop the handler
 *  Input :  None
 *  Output: None
 *  Return:  0  : Success Init Handler
 *              -1 : Fail Init Handler
 * ***************************************************************************************/
int HANDLER_API Handler_Stop( void )
{
	printf("> %s Stop", strPluginName);

	g_status = handler_status_stop;
	return handler_success;
}

/* **************************************************************************************
 *  Function Name: Handler_Recv
 *  Description: Receive Packet from MQTT Server
 *  Input : char * const topic, 
 *			void* const data, 
 *			const size_t datalen
 *  Output: void *pRev1, 
 *			void* pRev2
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_Recv(char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2  )
{
	printf(" >Recv Topic [%s] Data %s", topic, (char*) data );
}

/* **************************************************************************************
 *  Function Name: Handler_AutoReportStart
 *  Description: Start Auto Report
 *  Input : char *pInQuery
 *  Output: None
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_AutoReportStart(char *pInQuery)
{
	/*TODO: Parsing received command
	*input data format: 
	* {"susiCommData":{"catalogID":4,"autoUploadIntervalSec":30,"requestID":1001,"requestItems":["all"],"commCmd":2053}}
	*
	* "autoUploadIntervalSec":30 means report sensor data every 30 sec.
	* "requestItems":["all"] defined which handler or sensor data to report. 
	*/
	printf("> %s Start Report", strPluginName);
	/*create thread to report sensor data*/
	g_HandlerContex.interval = 1;
	g_HandlerContex.isThreadRunning = true;
	if (pthread_create(&g_HandlerContex.threadHandler, NULL, SampleHandlerReportThread, &g_HandlerContex) != 0)
	{
		g_HandlerContex.isThreadRunning = false;
		printf("start thread failed!\n");	
	}
	;
}

/* **************************************************************************************
 *  Function Name: Handler_AutoReportStop
 *  Description: Stop Auto Report
 *  Input : None
 *  Output: None
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_AutoReportStop(char *pInQuery)
{
	/*TODO: Parsing received command*/
	printf("> %s Stop Report", strPluginName);

	if(g_HandlerContex.threadHandler)
	{
		g_HandlerContex.isThreadRunning = false;
		pthread_join(g_HandlerContex.threadHandler, NULL);
		g_HandlerContex.threadHandler = NULL;
	}
}

/* **************************************************************************************
 *  Function Name: Handler_Get_Capability
 *  Description: Get Handler Information specification. 
 *  Input :  None
 *  Output: char ** : pOutReply       // JSON Format
 *  Return:  int  : Length of the status information in JSON format
 *                :  0 : no data need to trans
 * **************************************************************************************/
int HANDLER_API Handler_Get_Capability( char ** pOutReply ) // JSON Format
{
	char* result = NULL;
	int len = 0;

	printf("> %s Get Capability", strPluginName);

	if(!pOutReply) return len;

	if(!g_Capability)
		g_Capability = CreateCapability();

	result = IoT_PrintCapability(g_Capability);

	len = strlen(result);
	*pOutReply = (char *)malloc(len + 1);
	memset(*pOutReply, 0, len + 1);
	strcpy(*pOutReply, result);
	free(result);
	return len;
}

/* **************************************************************************************
 *  Function Name: Handler_MemoryFree
 *  Description: free the memory allocated for Handler_Get_Capability
 *  Input : char *pInData.
 *  Output: None
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_MemoryFree(char *pInData)
{
	printf("> %s Free Allocated Memory", strPluginName);

	if(pInData)
	{
		free(pInData);
		pInData = NULL;
	}
	return;
}