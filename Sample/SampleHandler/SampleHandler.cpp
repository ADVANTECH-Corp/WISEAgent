/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/07/14 by Scott Chang									*/
/* Modified Date: 2016/07/14 by Scott Chang									*/
/* Abstract     : Sample Handler to parse json string in c:/test.txt file	*/
/*                and report to server.										*/	
/* Reference    : None														*/
/****************************************************************************/

#include "stdafx.h"
#include "susiaccess_handler_api.h"
#include "DeviceMessageGenerate.h"
#include "IoTMessageGenerate.h"
#include "FlatToIPSO.h"
#include <Log.h>
#include <stdio.h>
#include "cJSON.h"
#include "HandlerKernel.h"

//-----------------------------------------------------------------------------
// Logger defines:
//-----------------------------------------------------------------------------
#define SAMPLEHANDLER_LOG_ENABLE
//#define DEF_SAMPLEHANDLER_LOG_MODE    (LOG_MODE_NULL_OUT)
//#define DEF_SAMPLEHANDLER_LOG_MODE    (LOG_MODE_FILE_OUT)
#define DEF_SAMPLEHANDLER_LOG_MODE    (LOG_MODE_CONSOLE_OUT|LOG_MODE_FILE_OUT)

LOGHANDLE g_samolehandlerlog = NULL;

#ifdef SAMPLEHANDLER_LOG_ENABLE
#define SampleHLog(level, fmt, ...)  do { if (g_samolehandlerlog != NULL)   \
	WriteLog(g_samolehandlerlog, DEF_SAMPLEHANDLER_LOG_MODE, level, fmt, ##__VA_ARGS__); } while(0)
#else
#define SampleHLog(level, fmt, ...)
#endif

//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------
#define cagent_request_custom 2102 /*define the request ID for V3.0, not used on V3.1 or later*/
#define cagent_custom_action 31002 /*define the action ID for V3.0, not used on V3.1 or later*/

const char strHandlerName[MAX_TOPIC_LEN] = {"TextHandler"}; /*declare the handler name*/

MSG_CLASSIFY_T *g_Capability = NULL; /*the global message structure to describe the sensor data as the handelr capability*/
//-----------------------------------------------------------------------------
// Internal Prototypes:
//-----------------------------------------------------------------------------
//
typedef struct{
   void* threadHandler; // thread handle
   int interval;		// time interval for file read
   bool isThreadRunning;//thread running flag
}handler_context_t;

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
static Handler_info  g_HandlerInfo; //global Handler info structure
static handler_context_t g_HandlerContex;
static HANDLER_THREAD_STATUS g_status = handler_status_no_init; // global status flag.
static HandlerSendCbf  g_sendcbf = NULL;						// Client Send information (in JSON format) to Cloud Server	
//-----------------------------------------------------------------------------
// Function:
//-----------------------------------------------------------------------------
void Handler_Uninitialize();

#ifdef _MSC_VER
BOOL WINAPI DllMain(HINSTANCE module_handle, DWORD reason_for_call, LPVOID reserved)
{
	if (reason_for_call == DLL_PROCESS_ATTACH) // Self-explanatory
	{
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
/*Open text file*/
const char* FileRead()
{
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
	/*Data String: {"<tag>":<Number>, "<tag>":true, "<tag>":false, "<tag>":null, "<tag>":"<string>"}*/
	char* data = (char *)FileRead();
	if(!data) return false;
	if(strlen(data)<=0) return false;	
	return transfer_parse_json(data, pGroup);
}

/*Create Capability Message Structure to describe sensor data*/
MSG_CLASSIFY_T * CreateCapability()
{
	MSG_CLASSIFY_T* myCapability = IoT_CreateRoot((char*)strHandlerName);
	MSG_CLASSIFY_T*myGroup = IoT_AddGroup(myCapability, "Monitor");
	MSG_ATTRIBUTE_T* attr = NULL;
	/*Open text file and parse the JSON string into Message Structure*/
	ParseReceivedData(myGroup);
	return myCapability;
}

static DWORD WINAPI SampleHandlerReportThread(void *args)
{
	/*thread to read text file repeatedly.*/
	handler_context_t *pHandlerContex = (handler_context_t *)args;
	int mInterval = pHandlerContex->interval * 1000;

	if(!g_Capability)
	{
		g_Capability = CreateCapability();
		HandlerKernel_SetCapability(g_Capability, true);
	}

	while(pHandlerContex->isThreadRunning)
	{
	
		if(g_Capability)
		{
			MSG_CLASSIFY_T *myGroup = IoT_FindGroup(g_Capability, "Monitor");
			if(myGroup)
				ParseReceivedData(myGroup);
		}
		Sleep(mInterval);
	}
    return 0;
}

/*callback function to handle threshold rule check event*/
void on_threshold_triggered(threshold_event_type type, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev)
{
	SampleHLog(Debug, " %s> threshold triggered:[%d, %s, %f]", g_HandlerInfo.Name, type, sensorname, value);
}

/*callback function to handle get sensor data event*/
bool on_get_sensor(get_data_t* objlist, void *pRev)
{
	get_data_t *current = objlist;
	if(objlist == NULL) return false;

	while(current)
	{
		current->errcode = STATUSCODE_SUCCESS;
		strcpy(current->errstring, STATUS_SUCCESS);

		switch(current->attr->type)
		{
		case attr_type_numeric:
			SampleHLog(Debug, " %s> get: %s value:%d", g_HandlerInfo.Name, current->sensorname, current->attr->v);
		 break;
		case attr_type_boolean:
			SampleHLog(Debug, " %s> get: %s value:%s", g_HandlerInfo.Name, current->sensorname, current->attr->bv?"true":"false");
		 break;
		case attr_type_string:
			SampleHLog(Debug, " %s> get: %s value:%s", g_HandlerInfo.Name, current->sensorname, current->attr->sv);
		 break;
		case attr_type_date:
			SampleHLog(Debug, " %s> get: %s value:Date:%s", g_HandlerInfo.Name, current->sensorname, current->attr->sv);
		 break;
		case attr_type_timestamp:
		 SampleHLog(Debug, " %s> get: %s value:Timestamp:%d", g_HandlerInfo.Name, current->sensorname, current->attr->v);
		 break;
		}

		current = current->next;
	}
	return true;
}

/*callback function to handle set sensor data event*/
bool on_set_sensor(set_data_t* objlist, void *pRev)
{
	set_data_t *current = objlist;
	if(objlist == NULL) return false;
	while(current)
	{
		current->errcode = STATUSCODE_SUCCESS;
		strcpy(current->errstring, STATUS_SUCCESS);

		switch(current->newtype)
		{
		case attr_type_numeric:
			SampleHLog(Debug, " %s> set: %s value:%d", g_HandlerInfo.Name, current->sensorname, current->v);
		 break;
		case attr_type_boolean:
			SampleHLog(Debug, " %s> set: %s value:%s", g_HandlerInfo.Name, current->sensorname, current->bv?"true":"false");
		 break;
		case attr_type_string:
			SampleHLog(Debug, " %s> set: %s value:%s", g_HandlerInfo.Name, current->sensorname, current->sv);
		 break;
		}

		current = current->next;
	}

	return true;
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
	sprintf_s( pluginfo->Name, sizeof(pluginfo->Name), "%s", strHandlerName );
	pluginfo->RequestID = cagent_request_custom;
	pluginfo->ActionID = cagent_custom_action;
	g_samolehandlerlog = pluginfo->loghandle;
	SampleHLog(Debug, " %s> Initialize", strHandlerName);
	// 2. Copy agent info 
	memcpy(&g_HandlerInfo, pluginfo, sizeof(HANDLER_INFO));
	g_HandlerInfo.agentInfo = pluginfo->agentInfo;

	// 3. Callback function -> Send JSON Data by this callback function

	g_HandlerContex.threadHandler = NULL;
	g_HandlerContex.isThreadRunning = false;
	g_status = handler_status_no_init;
	
	return HandlerKernel_Initialize(pluginfo);
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
		WaitForSingleObject(g_HandlerContex.threadHandler, INFINITE);
		CloseHandle(g_HandlerContex.threadHandler);
		g_HandlerContex.threadHandler = NULL;
	}
	HandlerKernel_Uninitialize();
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
	SampleHLog(Debug, " %s> Get Status", strHandlerName);
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
	SampleHLog(Debug, " %s> Update Status", strHandlerName);
	if(pluginfo)
		memcpy(&g_HandlerInfo, pluginfo, sizeof(HANDLER_INFO));
	else
	{
		memset(&g_HandlerInfo, 0, sizeof(HANDLER_INFO));
		sprintf_s( g_HandlerInfo.Name, sizeof( g_HandlerInfo.Name), "%s", strHandlerName );
		g_HandlerInfo.RequestID = cagent_request_custom;
		g_HandlerInfo.ActionID = cagent_custom_action;
	}
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
	SampleHLog(Debug, "> %s Start", strHandlerName);
	/*Create thread to read text file*/
	g_HandlerContex.interval = 1;
	g_HandlerContex.isThreadRunning = true;
	g_HandlerContex.threadHandler = CreateThread(NULL, 0, SampleHandlerReportThread, &g_HandlerContex, 0, NULL);

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
	SampleHLog(Debug, "> %s Stop", strHandlerName);

	/*Stop text file read thread*/
	if(g_HandlerContex.threadHandler)
	{
		g_HandlerContex.isThreadRunning = false;
		WaitForSingleObject(g_HandlerContex.threadHandler, INFINITE);
		CloseHandle(g_HandlerContex.threadHandler);
		g_HandlerContex.threadHandler = NULL;
	}

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
	int cmdID = 0;
	char sessionID[33] = {0};
	printf(" >Recv Topic [%s] Data %s\n", topic, (char*) data );
	
	/*Parse Received Command*/
	if(HandlerKernel_ParseRecvCMDWithSessionID((char*)data, &cmdID, sessionID) != handler_success)
		return;
	switch(cmdID)
	{
	case hk_auto_upload_req:
		/*start live report*/
		HandlerKernel_LiveReportStart(hk_auto_upload_rep, (char*)data);
		break;
	case hk_set_thr_req:
		/*Stop threshold check thread*/
		HandlerKernel_StopThresholdCheck();
		/*setup threshold rule*/
		HandlerKernel_SetThreshold(hk_set_thr_rep,(char*) data);
		/*register the threshold check callback function to handle trigger event*/
		HandlerKernel_SetThresholdTrigger(on_threshold_triggered);
		/*Restart threshold check thread*/
		HandlerKernel_StartThresholdCheck();
		break;
	case hk_del_thr_req:
		/*Stop threshold check thread*/
		HandlerKernel_StopThresholdCheck();
		/*clear threshold check callback function*/
		HandlerKernel_SetThresholdTrigger(NULL);
		/*Delete all threshold rules*/
		HandlerKernel_DeleteAllThreshold(hk_del_thr_rep);
		break;
	case hk_get_sensors_data_req:
		/*Get Sensor Data with callback function*/
		HandlerKernel_GetSensorData(hk_get_sensors_data_rep, sessionID, (char*)data, on_get_sensor);
		break;
	case hk_set_sensors_data_req:
		/*Set Sensor Data with callback function*/
		HandlerKernel_SetSensorData(hk_set_sensors_data_rep, sessionID, (char*)data, on_set_sensor);
		break;
	default:
		{
			/* Send command not support reply message*/
			char repMsg[32] = {0};
			int len = 0;
			sprintf( repMsg, "{\"errorRep\":\"Unknown cmd!\"}" );
			len= strlen( "{\"errorRep\":\"Unknown cmd!\"}" ) ;
			if ( g_sendcbf ) g_sendcbf( & g_HandlerInfo, hk_error_rep, repMsg, len, NULL, NULL );
		}
		break;
	}
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
	SampleHLog(Debug, "> %s Start Report", strHandlerName);
	/*create thread to report sensor data*/
	HandlerKernel_AutoReportStart(pInQuery);
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
	SampleHLog(Debug, "> %s Stop Report", strHandlerName);

	HandlerKernel_AutoReportStop(pInQuery);
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

	SampleHLog(Debug, "> %s Get Capability", strHandlerName);

	if(!pOutReply) return len;

	/*Create Capability Message Structure to describe sensor data*/
	if(!g_Capability)
	{
		g_Capability = CreateCapability();
		HandlerKernel_SetCapability(g_Capability, false);
	}
	/*generate capability JSON string*/
	result = IoT_PrintCapability(g_Capability);

	/*create buffer to store the string*/
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
	SampleHLog(Debug, "> %s Free Allocated Memory", strHandlerName);

	if(pInData)
	{
		free(pInData);
		pInData = NULL;
	}
	return;
}