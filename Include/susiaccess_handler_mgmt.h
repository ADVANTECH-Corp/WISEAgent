#ifndef _CAGENT_HANDLER_MGMT_H_
#define _CAGENT_HANDLER_MGMT_H_

#include "susiaccess_handler_ex_api.h"

typedef handler_result (HANDLER_API *HANDLER_INITLIZE) ( HANDLER_INFO *handler );
typedef handler_result (HANDLER_API *HANDLER_GET_STATUS) ( HANDLER_THREAD_STATUS * pOutStatus );
typedef handler_result (HANDLER_API *HANDLER_ONSTATUSCAHNGE) ( HANDLER_INFO *handler );
typedef handler_result (HANDLER_API *HANDLER_START) ( void );
typedef handler_result (HANDLER_API *HANDLER_STOP) ( void );
typedef void (HANDLER_API *HANDLER_RECV) (char * const topic, void* const data, const size_t datalen, void *pRev1, void* pRev2 ); 
typedef handler_result (HANDLER_API *HANDLER_GET_CAPABILITY) ( char ** pOutReply );
typedef void (HANDLER_API *HANDLER_AUTOREPORT_START)(char *pInQuery);
typedef void (HANDLER_API *HANDLER_AUTOREPORT_STOP)(char *pInQuery);
typedef void (HANDLER_API *HANDLER_MEMORYFREE)(char *pInData);

typedef enum{
	unknown_handler = 0,
	user_handler,
	core_handler,
	virtual_handler,
}hander_type;

typedef struct HANDLER_LOADER_INTERFACE
{
	char						Name[MAX_TOPIC_LEN];
	HANDLER_INFO				*pHandlerInfo;
	int							Workable;
	hander_type					type;
	void*						Handler;               // handle of to load so library
	HANDLER_INITLIZE			Handler_Init_API;
	HANDLER_GET_STATUS			Handler_Get_Status_API;
	HANDLER_ONSTATUSCAHNGE		Handler_OnStatusChange_API;
	HANDLER_START				Handler_Start_API;
	HANDLER_STOP				Handler_Stop_API;
	HANDLER_RECV				Handler_Recv_API;
	HANDLER_GET_CAPABILITY		Handler_Get_Capability_API;
	HANDLER_AUTOREPORT_START	Handler_AutoReportStart_API;
	HANDLER_AUTOREPORT_STOP		Handler_AutoReportStop_API;
	HANDLER_MEMORYFREE			Handler_MemoryFree_API;

	struct HANDLER_LOADER_INTERFACE *prev;
	struct HANDLER_LOADER_INTERFACE *next;
}Handler_Loader_Interface;

typedef struct Handler_List_t{
	int total;
	Handler_Loader_Interface* items;
}Handler_List_t;

typedef struct Callback_Functions_t{
	HandlerSendCbf  sendcbf;
	HandlerSendCustCbf  sendcustcbf;
	HandlerSubscribeCustCbf  subscribecustcbf;
	HandlerAutoReportCbf sendreportcbf;
	HandlerSendCapabilityCbf sendcapabilitycbf;
	HandlerSendEventCbf sendevnetcbf;
	HandlerConnectServerCbf connectservercbf;
	HandlerDisconnectCbf disconnectcbf;
	HandlerRenameCbf renamecbf;
	HandlerSendOSInfoCbf sendosinfocbf;
	HandlerAddVirtualHandlerCbf addvirtualhandlercbf;
}Callback_Functions_t;

#endif /* _CAGENT_HANDLER_MGMT_H_ */