/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/08/08 by Scott Chang								    */
/* Modified Date: 2016/08/08 by Scott Chang									*/
/* Abstract     : HandlerKernel API definition								*/
/* Reference    : None														*/
/****************************************************************************/

#ifndef _HANDLER_KERNEL_H_
#define _HANDLER_KERNEL_H_

#include <stdbool.h>
#include "susiaccess_handler_ex_api.h"
#include "HandlerKernelEx.h"
#include "IoTMessageGenerate.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <windows.h>
#ifndef HANDLERKERNEL_API
	#define HANDLERKERNEL_API WINAPI
#endif
#else
	#define HANDLERKERNEL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* **************************************************************************************
 *  Function Name: HandlerKernel_Initialize
 *  Description: Init any objects or variables of this HandlerKernel
 *  Input : HANDLER_INFO *handler
 *  Output: None
 *  Return:  0 : Success Init Handler
 *          -1 : Fail Init Handler
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_Initialize( HANDLER_INFO *handler );

/* **************************************************************************************
 *  Function Name: HandlerKernel_Uninitialize
 *  Description: Uninit any objects or variables of this HandlerKernel
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_Uninitialize();

/* **************************************************************************************
 *  Function Name: HandlerKernel_SetCapability
 *  Description: Assign the Capability structure and set the bPublish to publish the new capability
 *  Input : MSG_CLASSIFY_T *pCapability
 *			bool bPublish
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SetCapability( MSG_CLASSIFY_T* pCapability, bool bPublish );

/* **************************************************************************************
 *  Function Name: HandlerKernel_LockCapability
 *  Description: Assign the Capability structure and set the bPublish to publish the new capability
 *  Input : MSG_CLASSIFY_T *pCapability
 *			bool bPublish
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_LockCapability();

/* **************************************************************************************
 *  Function Name: HandlerKernel_UnlockCapability
 *  Description: Assign the Capability structure and set the bPublish to publish the new capability
 *  Input : MSG_CLASSIFY_T *pCapability
 *			bool bPublish
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_UnlockCapability();

/* **************************************************************************************
 *  Function Name: HandlerKernel_AutoReportStart
 *  Description: Start Auto Report
 *  Input : char *pInQuery
 *  Output: None
 *  Return:  0 : Success
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_AutoReportStart(char *pInQuery);

/* **************************************************************************************
 *  Function Name: HandlerKernel_AutoReportStop
 *  Description: Stop Auto Report
 *  Input : char *pInQuery, if *pInQuery = NULL, then stop all upload message.
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_AutoReportStop(char *pInQuery);

/* **************************************************************************************
 *  Function Name: HandlerKernel_SetAutoReportFilter
 *  Description: Set Auto Report filter to filter out the unused sensor data. 
 *  Input : char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SetAutoReportFilter(char *pInQuery);

/* **************************************************************************************
 *  Function Name: HandlerKernel_SendAutoReportOnce
 *  Description: Send single auto report data. User can used with event trigger.
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SendAutoReportOnce();

/* **************************************************************************************
 *  Function Name: HandlerKernel_LiveReportStart
 *  Description: Start Live Report (auto upload)
 *  Input : int replyID
 *			char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_LiveReportStart(int replyID, char *pInQuery);

/* **************************************************************************************
 *  Function Name: HandlerKernel_ParseRecvCMD
 *  Description: Parse received command to command ID
 *  Input : char *pInQuery
 *  Output: int * cmdID
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_ParseRecvCMD(char *pInQuery, int * cmdID);


/* **************************************************************************************
 *  Function Name: HandlerKernel_ParseRecvCMDWithSessionID
 *  Description: Parse received command to command ID
 *  Input : char *pInQuery
 *  Output: int * cmdID
 *			char * sessionID
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_ParseRecvCMDWithSessionID(char *pInQuery, int * cmdID, char * sessionID);



/* **************************************************************************************
 *  Function Name: HandlerKernel_SetThreshold
 *  Description: Set threshold rule
 *  Input : int replyID
 *			char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_SetThreshold(int replyID, char *pInQuery);

/* **************************************************************************************
 *  Function Name: HandlerKernel_DeleteAllThreshold
 *  Description: Delete threshold rule
 *  Input : int replyID
 *			char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_DeleteAllThreshold(int replyID);

/* **************************************************************************************
 *  Function Name: HandlerKernel_StartThresholdCheck
 *  Description: Start threshold rule check thread
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_StartThresholdCheck();

/* **************************************************************************************
 *  Function Name: HandlerKernel_StopThresholdCheck
 *  Description: Stop threshold rule check thread
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_StopThresholdCheck();

/* **************************************************************************************
 *  Function Name: HandlerKernel_SetThresholdTrigger
 *  Description: register the threshold check event callback function.
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
													void *pRev));

/* **************************************************************************************
 *  Function Name: HandlerKernel_GetSensorData
 *  Description: Get Sensor Data
 *  Input : int replyID
 *			char *pInQuery
 *			on_get_sensor - a callback function in the following form:
 *               bool (*on_get_sensor)(Handler_info* handler, char* sensorname, double* value, MSG_ATTRIBUTE_T* attr, void *pRev)
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * Callback Parameters:
 *  sensorname -  sensor uri
 *  attr - target object
 *  pRev - preserved.
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernel_GetSensorData(int replyID, char* sessionID, char *pInQuery, bool (*on_get_sensor)(get_data_t* getlist, void *pRev));

/* **************************************************************************************
 *  Function Name: HandlerKernel_SetSensorData
 *  Description: Set Sensor Data
 *  Input : int replyID
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
int HANDLERKERNEL_API HandlerKernel_SetSensorData(int replyID, char* sessionID, char *pInQuery, bool (*on_set_sensor)(set_data_t* setlist, void *pRev));


#ifdef __cplusplus
}
#endif

#endif