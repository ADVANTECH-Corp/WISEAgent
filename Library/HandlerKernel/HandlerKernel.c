#include "HandlerKernel.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "Log.h"


//-----------------------------------------------------------------------------
// Logger defines:
//-----------------------------------------------------------------------------
#define HANDLERKERNEL_LOG_ENABLE
//#define DEF_HANDLERKERNEL_LOG_MODE    (LOG_MODE_NULL_OUT)
//#define DEF_HANDLERKERNEL_LOG_MODE    (LOG_MODE_FILE_OUT)
#define DEF_HANDLERKERNEL_LOG_MODE    (LOG_MODE_CONSOLE_OUT|LOG_MODE_FILE_OUT)

LOGHANDLE g_handlerkernellog = NULL;

#ifdef HANDLERKERNEL_LOG_ENABLE
#define KernelHLog(level, fmt, ...)  do { if (g_handlerkernellog != NULL)   \
	WriteLog(g_handlerkernellog, DEF_HANDLERKERNEL_LOG_MODE, level, fmt, ##__VA_ARGS__); } while(0)
#else
#define KernelHLog(level, fmt, ...)
#endif

typedef void (*on_threshold_triggered)(threshold_event_type type, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev);
on_threshold_triggered g_on_triggered = NULL;

void* g_pHandlerKernel = NULL;
//-----------------------------------------------------------------------------
// Internal Function:
//-----------------------------------------------------------------------------

void HandlerKernel_On_Triggered(bool isNormal, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev)
{

	threshold_event_type type = thr_normal;
	
	type = isNormal?thr_normal:thr_out_of_range;

	if(g_on_triggered)
		g_on_triggered(type, sensorname, value, attr, pRev);
}

//-----------------------------------------------------------------------------
// API Function:
//-----------------------------------------------------------------------------

/* **************************************************************************************
 *  Function Name: Handler_Initialize
 *  Description: Init any objects or variables of this handler
 *  Input : HANDLER_INFO *handler
 *  Output: None
 *  Return:  0 : Success Init Handler
 *          -1 : Fail Init Handler
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_Initialize( HANDLER_INFO *handler )
{
	if( handler == NULL )
		return handler_fail;
	g_handlerkernellog = handler->loghandle;
	// 1. Topic of this handler
	KernelHLog(Debug, " %s> Initialize",  handler->Name);
	g_pHandlerKernel = HandlerKernelEx_Initialize(handler);
	if(g_pHandlerKernel == NULL)
		return handler_fail;

	HandlerKernelEx_SetThresholdTrigger(g_pHandlerKernel, HandlerKernel_On_Triggered);

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: Handler_Uninitialize
 *  Description: Uninit any objects or variables of this handler
 *  Input : None
 *  Output: None
 *  Return:  0  : Success
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_Uninitialize()
{
	if(g_pHandlerKernel == NULL)
		return handler_fail;

	HandlerKernelEx_Uninitialize(g_pHandlerKernel);
	g_handlerkernellog = NULL;

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_SetCapability
 *  Description: Assign the Capability structure and set the bPublish to publish the new capability
 *  Input : MSG_CLASSIFY_T *pCapability
 *			bool bPublish
 *  Output: None
 *  Return:  0 : Success
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SetCapability( MSG_CLASSIFY_T* pCapability, bool bPublish )
{
	if( pCapability == NULL )
		return handler_fail;

	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_SetCapability(g_pHandlerKernel, pCapability, bPublish);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_LockCapability
 *  Description: Assign the Capability structure and set the bPublish to publish the new capability
 *  Input : MSG_CLASSIFY_T *pCapability
 *			bool bPublish
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_LockCapability()
{
	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_LockCapability(g_pHandlerKernel);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_UnlockCapability
 *  Description: Assign the Capability structure and set the bPublish to publish the new capability
 *  Input : MSG_CLASSIFY_T *pCapability
 *			bool bPublish
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_UnlockCapability()
{
	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_UnlockCapability(g_pHandlerKernel);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_AutoReportStart
 *  Description: Start Auto Report
 *  Input : char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_AutoReportStart(char *pInQuery)
{
	if( pInQuery == NULL )
		return handler_fail;

	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_AutoReportStart(g_pHandlerKernel, pInQuery);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_AutoReportStop
 *  Description: Stop Auto Report
 *  Input : char *pInQuery, preserved.
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_AutoReportStop(char *pInQuery)
{
	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_AutoReportStop(g_pHandlerKernel, pInQuery);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_SetAutoReportFilter
 *  Description: Set Auto Report filter to filter out the unused sensor data. 
 *  Input : char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SetAutoReportFilter(char *pInQuery)
{
	if( pInQuery == NULL )
		return handler_fail;
	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_SetAutoReportFilter(g_pHandlerKernel, pInQuery);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_SendAutoReportOnce
 *  Description: Send single auto report data. User can used with event trigger.
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SendAutoReportOnce()
{
	if(g_pHandlerKernel == NULL)
		return handler_fail;
	return HandlerKernelEx_SendAutoReportOnce(g_pHandlerKernel);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_LiveReportStart
 *  Description: Start Live Report (auto upload)
 *  Input : int replyID
 *			char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_LiveReportStart(int replyID, char *pInQuery)
{
	if(g_pHandlerKernel == NULL)
		return handler_fail;
	if( pInQuery == NULL )
		return handler_fail;

	return HandlerKernelEx_LiveReportStart(g_pHandlerKernel, replyID, pInQuery);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_ParseRecvCMD
 *  Description: Parse received command to command ID
 *  Input : char *pInQuery
 *  Output: int * cmdID
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_ParseRecvCMD(char *pInQuery, int * cmdID)
{
	if( pInQuery == NULL || cmdID == NULL)
		return handler_fail;

	return HandlerKernelEx_ParseRecvCMDWithSessionID(pInQuery, cmdID, NULL);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_ParseRecvCMDWithSessionID
 *  Description: Parse received command to command ID
 *  Input : char *pInQuery
 *  Output: int * cmdID
 *			char * sessionID
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_ParseRecvCMDWithSessionID(char *pInQuery, int * cmdID, char * sessionID)
{
	if( pInQuery == NULL || cmdID == NULL)
		return handler_fail;

	return HandlerKernelEx_ParseRecvCMDWithSessionID(pInQuery, cmdID, sessionID);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_SetThreshold
 *  Description: Set threshold rule
 *  Input : int replyID
 *			char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SetThreshold(int replyID, char *pInQuery)
{
	if( pInQuery == NULL)
		return handler_fail;

	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_SetThreshold(g_pHandlerKernel, replyID, pInQuery);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_DeleteAllThreshold
 *  Description: Delete threshold rule
 *  Input : int replyID
 *			char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_DeleteAllThreshold(int replyID)
{
	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_DeleteAllThreshold(g_pHandlerKernel, replyID);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_StartThresholdCheck
 *  Description: Start threshold rule check thread
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_StartThresholdCheck()
{
	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_StartThresholdCheck(g_pHandlerKernel);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_StopThresholdCheck
 *  Description: Stop threshold rule check thread
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_StopThresholdCheck()
{
	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_StopThresholdCheck(g_pHandlerKernel);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_SetThresholdTrigger
 *  Description:  register the threshold check event callback function.
 *  Input : on_trigger - a callback function in the following form:
 *               void callback(HANDLE const handler, threshold_event_type tyep, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev)
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * Callback Parameters:
 *  type -  threshold trigger type
 *  sensorname -  triggered sensor uri
 *  value - triggered value
 *  attr - triggered object
 *  pRev - preserved.
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SetThresholdTrigger(void (*on_triggered)(threshold_event_type type,
													char* sensorname, double value, MSG_ATTRIBUTE_T* attr,
													void *pRev))
{
	g_on_triggered = on_triggered;
	
	return handler_success;
}


/* **************************************************************************************
 *  Function Name: HandlerKernel_GetSensorData
 *  Description: Get Sensor Data
 *  Input : int replyID
 *			char* sessionID
 *			char *pInQuery
 *			on_get_sensor - a callback function in the following form:
 *               bool (*on_get_sensor)(Handler_info* handler, char* sensorname, double* value, MSG_ATTRIBUTE_T* attr, void *pRev)
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * Callback Parameters:
 *  sensorname -  sensor uri
 *  value - return value
 *  attr - target object
 *  pRev - preserved.
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_GetSensorData(int replyID, char* sessionID, char *pInQuery, bool (*on_get_sensor)(get_data_t* getlist, void *pRev))
{
	if( pInQuery == NULL )
		return handler_fail;

	if(g_pHandlerKernel == NULL)
		return handler_fail;
	
	return HandlerKernelEx_GetSensorData(g_pHandlerKernel, replyID, sessionID, pInQuery, on_get_sensor);
}

/* **************************************************************************************
 *  Function Name: HandlerKernel_SetSensorData
 *  Description: Set Sensor Data
 *  Input : int replyID
 *			char* sessionID
 *			char *pInQuery
 *			on_set_sensor - a callback function in the following form:
 *               bool (*on_get_sensor)(Handler_info* handler, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev)
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * Callback Parameters:
 *  sensorname -  sensor uri
 *  value - received value
 *  attr - target object
 *  pRev - preserved.
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SetSensorData(int replyID, char* sessionID, char *pInQuery, bool (*on_set_sensor)(set_data_t* setlist, void *pRev))
{
	if( pInQuery == NULL )
		return handler_fail;

	if(g_pHandlerKernel == NULL)
		return handler_fail;

	return HandlerKernelEx_SetSensorData(g_pHandlerKernel, replyID, sessionID, pInQuery, on_set_sensor);
}