/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2013/05/20 by Eric Liang									*/
/* Modified Date: 2013/07/19 by Eric Liang									*/
/* Abstract       : Susi Player API interface definition   					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef _CAGENT_HANDLER_H_
#define _CAGENT_HANDLER_H_

#include "susiaccess_def.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <Windows.h>
#ifndef HANDLER_API
	#define HANDLER_API WINAPI
#endif
#else
	#include <stdlib.h>
	#define HANDLER_API
#endif

#ifdef __cplusplus 
extern "C" { 
#endif 

//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------
//


// The max length of URI
#define MAX_URI_LEN  1024

typedef enum {
   handler_fail = -1,
   handler_success = 0,               // No error. 
   handler_no_init,
   handler_callback_null,
   handler_callback_error,
   handler_no_connnect,
   handler_init_error,
} handler_result;

/* Define Plugin Handler */
#define HANDLE           void *    /**< general struct point */

typedef enum {
   cagent_success = 0,               // No error. 
   cagent_no_init, 
   cagent_callback_null,
   cagent_callback_error,
   cagent_no_connnect,
   cagent_connect_error,
   cagent_init_error,   
   cagent_network_sock_timeout = 0x10,
   cagent_network_sock_error,
   cagent_send_data_error,
} AGENT_SEND_STATUS;

typedef enum {
	handler_status_no_init = -1,               // not initialize. 
	handler_status_init,
	handler_status_start, 
	handler_status_stop, 
	handler_status_busy,						//  Internal threads no response, CAgent will restart current Handler or restart CAgent self.
} HANDLER_THREAD_STATUS;

typedef enum {
	Severity_Emergency = 0,
	Severity_Alert,
	Severity_Critical, 
	Severity_Error, 
	Severity_Warning,
	Severity_Informational,
	Severity_Debug,
} HANDLER_NOTIFY_SEVERITY;

/**SUSIAgent handler Send function point */
typedef AGENT_SEND_STATUS  (*HandlerSendCbf) ( HANDLE const handler, int enum_act, 
											  void const * const requestData, unsigned int const requestLen, 
											  void *pRev1, void* pRev2 ); 

typedef AGENT_SEND_STATUS  (*HandlerSendCustCbf) ( HANDLE const handler, int enum_act, char const * const topic, 
											  void const * const requestData, unsigned int const requestLen, 
											  void *pRev1, void* pRev2 ); 

typedef void (*HandlerCustMessageRecvCbf)(char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2);

typedef AGENT_SEND_STATUS  (*HandlerSubscribeCustCbf) ( char const * const topic, HandlerCustMessageRecvCbf recvCbf); 

typedef AGENT_SEND_STATUS  (*HandlerSendCapabilityCbf) ( HANDLE const handler, 
											      void const * const requestData, unsigned int const requestLen, 
												  void *pRev1, void* pRev2 ); 

typedef AGENT_SEND_STATUS  (*HandlerAutoReportCbf) ( HANDLE const handler, 
											  void const * const requestData, unsigned int const requestLen, 
											  void *pRev1, void* pRev2 );

typedef AGENT_SEND_STATUS  (*HandlerSendEventCbf) ( HANDLE const handler, HANDLER_NOTIFY_SEVERITY severity,
													void const * const requestData, unsigned int const requestLen, 
													void *pRev1, void* pRev2 );

/**SUSIAgent handler Recv function point */
//typedef void  (*HandlerRecvCbf) (char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2 ); 

/** Agent info struct define*/
typedef struct {
	char hostname[DEF_HOSTNAME_LENGTH];		/**< Agent host name */
	char devId[DEF_DEVID_LENGTH];			/**< Agent device id */       
	char sn[DEF_SN_LENGTH];					/**< Agent mac */
	char mac[DEF_MAC_LENGTH];				/**< Agent sn */
	char version[DEF_VERSION_LENGTH];		/**< Agent version   */
	char type[DEF_MAX_STRING_LENGTH];
	char product[DEF_MAX_STRING_LENGTH];
	char manufacture[DEF_MAX_STRING_LENGTH];
    int  status;             /**< Agent device status */
}cagent_agent_info_body_t;

typedef struct HANDLER_INFO
{	
	char Name[MAX_TOPIC_LEN];						// The handler name
	char ServerIP[DEF_MAX_STRING_LENGTH];
	int ServerPort;
	char WorkDir[DEF_MAX_STRING_LENGTH];							
	int RequestID;
	int ActionID;
	//key_t shmKey;									// shared memory key	
	void* loghandle;								// log file handler

	//GetLibFNP  GetLibAPI;							// Get the Function Point of comman library api
	cagent_agent_info_body_t * agentInfo;			// Info of the Agent
	HandlerSendCbf  sendcbf;						// Client Send information (in JSON format) to Cloud Server	
	HandlerSendCustCbf  sendcustcbf;			    // Client Send information (in JSON format) to Cloud Server with custom topic
	HandlerSubscribeCustCbf subscribecustcbf;		// Client subscribe the custom topic to receive message from Cloud Server 
	HandlerSendCapabilityCbf sendcapabilitycbf;			// Client Send Spec.Info (in JSON format) to Cloud Server with SpecInfo topic	
	HandlerAutoReportCbf sendreportcbf;				// Client Send report (in JSON format) to Cloud Server with AutoReport topic
	HandlerSendEventCbf sendeventcbf;				// Client Send Event Notify (in JSON format) to Cloud Server with EventNotify topic	
}HANDLER_INFO, Handler_info;

/* **************************************************************************************
 *  Function Name: Handler_Initialize
 *  Description: Init any objects or variables of this handler
 *  Input :  PLUGIN_INFO *pluginfo
 *  Output: None
 *  Return:  handler_success
 *           handler_fail
 * ***************************************************************************************/
int HANDLER_API Handler_Initialize( HANDLER_INFO *handler );


/* **************************************************************************************
 *  Function Name: Handler_Get_Status
 *  Description: Get Handler Threads Status. CAgent will restart current Handler or restart CAgent self if busy.
 *  Input :  None
 *  Output: HANDLER_THREAD_STATUS pOutStatus       // cagent handler status
 *  Return:  handler_success
 *			 handler_fail
 * **************************************************************************************/
int HANDLER_API Handler_Get_Status( HANDLER_THREAD_STATUS * pOutStatus );

/* **************************************************************************************
 *  Function Name: Handler_OnStatusChange
 *  Description: CAgent status change notify.
 *  Input :  PLUGIN_INFO *pluginfo
 *  Output: None
 *  Return:  None
 * ***************************************************************************************/
void HANDLER_API Handler_OnStatusChange( HANDLER_INFO *handler );

/* **************************************************************************************
 *  Function Name: Handler_Start
 *  Description: Start handler thread
 *  Input :  None
 *  Output: None
 *  Return:  handler_success
 *           handler_fail
 * ***************************************************************************************/
int HANDLER_API Handler_Start( void );


/* **************************************************************************************
 *  Function Name: Handler_Stop
 *  Description: Stop handler thread
 *  Input :  None
 *  Output: None
 *  Return:  handler_success
 *           handler_fail
 * ***************************************************************************************/
int HANDLER_API Handler_Stop( void );

/* **************************************************************************************
 *  Function Name: Handler_Recv
 *  Description: Received Packet from Server
 *  Input : char * const topic, 
 *			void* const data, 
 *			const size_t datalen
 *  Output: void *pRev1, 
 *			void* pRev2
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_Recv( char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2  );

/* **************************************************************************************
 *  Function Name: Handler_Get_Capability
 *  Description: Get Handler Information specification. 
 *  Input :  None
 *  Output: char ** : pOutReply       // JSON Format
 *  Return:  int  : Length of the status information in JSON format
 *                :  0 : no data need to trans
 * **************************************************************************************/
int HANDLER_API Handler_Get_Capability( char ** pOutReply ); // JSON Format

/* **************************************************************************************
 *  Function Name: Handler_AutoReportStart
 *  Description: Start Auto Report
 *  Input : char *pInQuery
 *  Output: None
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_AutoReportStart(char *pInQuery);

/* **************************************************************************************
 *  Function Name: Handler_AutoReportStop
 *  Description: Stop Auto Report
 *  Input : char *pInQuery, if *pInQuery = NULL, then stop all upload message.
 *  Output: None
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_AutoReportStop(char *pInQuery);

/* **************************************************************************************
 *  Function Name: Handler_MemoryFree
 *  Description: free the mamory allocated for Handler_Get_Capability
 *  Input : char *pInData.
 *  Output: None
 *  Return: None
 * ***************************************************************************************/
void HANDLER_API Handler_MemoryFree(char *pInData);

#ifdef __cplusplus 
} 
#endif 

#endif /* _CAGENT_HANDLER_H_ */

