#include "HandlerKernel.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "HandlerParser.h"
#include "HandlerThreshold.h"
#include "Log.h"
#include "triggerqueue.h"
#include "cJSON.h"

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

//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------

#define TAG_STATUSCODE				"StatusCode"
#define TAG_VALUE					"v"
#define TAG_BOOLEAN					"bv"
#define TAG_STRING					"sv"
#define TAG_DATE					"$date"
#define TAG_TIMESTAMP				"$timestamp"

typedef struct{
   void* threadHandler;
   pthread_mutex_t mux;
   unsigned int interval;
   unsigned int timeout;
   bool reqAll;
   char* reqItems;
   bool isThreadRunning;
   int replyID;
}handler_context_t;

typedef struct{
   char* reqItems;
   char sessionID[32];
   int replyID;
   void *pRev;
   bool (*on_sensor_cmd)(void* objlist, void *pRev);
}sensor_cmd_t;

Handler_info  g_HandlerInfo;
handler_context_t g_AutoReportContex;
handler_context_t g_LiveReportContex;
handler_context_t g_ThresholdChkContex;
HANDLER_THREAD_STATUS g_status = handler_status_no_init;

MSG_CLASSIFY_T *g_HandlerCapability = NULL;

thr_item_list g_ThresholdList = NULL;
typedef void (*on_threshold_triggered)(threshold_event_type type, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev);
on_threshold_triggered g_on_triggered = NULL;
//-----------------------------------------------------------------------------
// Internal Function:
//-----------------------------------------------------------------------------

void* HandlerKernel_AutoReportThread(void *args)
{
	handler_context_t *pHandlerContex = (handler_context_t *)args;
	int count = 0, interval=1000;
	int mInterval = pHandlerContex->interval;
	bool bStop = false;

	while(!bStop)
	{
		char* message = NULL;		
		pthread_mutex_lock(&pHandlerContex->mux);
		bStop = !pHandlerContex->isThreadRunning;
		if(g_HandlerCapability && !bStop)
		{
			if(pHandlerContex->reqAll)
				message = IoT_PrintData(g_HandlerCapability);
			else
				message = IoT_PrintSelectedData(g_HandlerCapability, pHandlerContex->reqItems);
		}
		pthread_mutex_unlock(&pHandlerContex->mux);

		if(bStop)
		{
			if(message)
				free(message);
			break;
		}

		if(message)
		{
			if(g_HandlerInfo.sendreportcbf)
				g_HandlerInfo.sendreportcbf(/*Handler Info*/&g_HandlerInfo, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);
			free(message);
		}

		pthread_mutex_lock(&pHandlerContex->mux);
		if(pHandlerContex->interval!=mInterval)
		{
			mInterval = pHandlerContex->interval;
		}
		pthread_mutex_unlock(&pHandlerContex->mux);
		count = mInterval*1000/interval;

		while(count > 0)
		{
			pthread_mutex_lock(&pHandlerContex->mux);
			bStop = (pHandlerContex->interval!=mInterval) || (!pHandlerContex->isThreadRunning);
			pthread_mutex_unlock(&pHandlerContex->mux);

			if(bStop)
				break;

			count--;
			usleep(interval*1000);
		}

	}
	pthread_exit(0);
	return 0;
}


void* HandlerKernel_LiveReportThread(void *args)
{
	handler_context_t *pHandlerContex = (handler_context_t *)args;
	unsigned int count = 0, interval=500;
	unsigned int mInterval = pHandlerContex->interval;
	int replyID = pHandlerContex->replyID;
	bool bStop = false;

	while(!bStop)
	{
		char* message = NULL;		
		pthread_mutex_lock(&pHandlerContex->mux);
		bStop = !pHandlerContex->isThreadRunning;
		if(g_HandlerCapability && !bStop)
		{
			if(pHandlerContex->reqAll)
				message = IoT_PrintData(g_HandlerCapability);
			else
				message = IoT_PrintSelectedData(g_HandlerCapability, pHandlerContex->reqItems);
		}
		pthread_mutex_unlock(&pHandlerContex->mux);

		if(bStop)
		{
			if(message)
				free(message);
			break;
		}

		if(message)
		{
			if(g_HandlerInfo.sendcbf)
				g_HandlerInfo.sendcbf(/*Handler Info*/&g_HandlerInfo, replyID, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);
			free(message);
		}

		pthread_mutex_lock(&pHandlerContex->mux);
		if(pHandlerContex->interval!=mInterval)
		{
			mInterval = pHandlerContex->interval;
		}
		pthread_mutex_unlock(&pHandlerContex->mux);
		count = mInterval/interval;

		while(count > 0)
		{
			pthread_mutex_lock(&pHandlerContex->mux);
			bStop = (pHandlerContex->interval!=mInterval) || (!pHandlerContex->isThreadRunning);
			pthread_mutex_unlock(&pHandlerContex->mux);

			if(bStop)
				break;

			count--;
			pthread_mutex_lock(&pHandlerContex->mux);
			if(pHandlerContex->timeout < interval)
				pHandlerContex->timeout = 0;
			else
				pHandlerContex->timeout -= interval;

			if(pHandlerContex->timeout == 0)
				bStop = true;
			pthread_mutex_unlock(&pHandlerContex->mux);
			usleep(interval*1000);
		}
	}
	pthread_mutex_lock(&pHandlerContex->mux);
	pHandlerContex->isThreadRunning = false;
	pthread_mutex_unlock(&pHandlerContex->mux);
	pthread_exit(0);
	return 0;
}

void* HandlerKernel_ThresholdCheckThread(void *args)
{
	handler_context_t *pHandlerContex = (handler_context_t *)args;
	unsigned int count = 0, interval=1000;
	unsigned int mInterval = pHandlerContex->interval;
	int replyID = pHandlerContex->replyID;
	bool bStop = false;
	bool bNormal = false;

	while(!bStop)
	{
		char* message = NULL;		
		HANDLER_NOTIFY_SEVERITY severity = Severity_Debug;
		pthread_mutex_lock(&pHandlerContex->mux);
		bStop = !pHandlerContex->isThreadRunning;
		if(g_ThresholdList != NULL && !bStop)
		{
			char* buffer = calloc(1,1024);	
			if(HandlerThreshold_CheckThr(g_ThresholdList, g_HandlerCapability, &buffer, 1024, &bNormal))
			{
				if(strlen(buffer) > 0)
				{
					message = calloc(1, strlen(buffer) + 128);

					if(bNormal)
					{
						severity = Severity_Informational;
						sprintf(message,"{\"subtype\":\"THRESHOLD_CHECK_INFO\",\"thrCheckStatus\":\"%s\",\"msg\":\"%s\"}", bNormal?"True":"False", buffer); /*for custom handler*/
					}
					else
					{
						severity = Severity_Error;
						sprintf(message,"{\"subtype\":\"THRESHOLD_CHECK_ERROR\",\"thrCheckStatus\":\"%s\",\"msg\":\"%s\"}", bNormal?"True":"False", buffer); /*for custom handler*/
					}
				}
			}
			free(buffer);
		}
		pthread_mutex_unlock(&pHandlerContex->mux);

		if(bStop)
		{
			if(message)
				free(message);
			break;
		}

		if(message)
		{
			if(g_HandlerInfo.sendeventcbf && strlen(message) > 0)
				g_HandlerInfo.sendeventcbf(/*Handler Info*/&g_HandlerInfo, severity, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);
			free(message);
		}

		pthread_mutex_lock(&pHandlerContex->mux);
		if(pHandlerContex->interval!=mInterval)
		{
			mInterval = pHandlerContex->interval;
		}
		pthread_mutex_unlock(&pHandlerContex->mux);
		count = mInterval/interval;

		while(count > 0)
		{
			pthread_mutex_lock(&pHandlerContex->mux);
			bStop = (pHandlerContex->interval!=mInterval) || (!pHandlerContex->isThreadRunning);
			pthread_mutex_unlock(&pHandlerContex->mux);

			if(bStop)
				break;

			count--;
			usleep(interval*1000);
		}
	}
	pthread_mutex_lock(&pHandlerContex->mux);
	pHandlerContex->isThreadRunning = false;
	pthread_mutex_unlock(&pHandlerContex->mux);
	pthread_exit(0);
	return 0;
}

void HandlerKernel_On_Triggered(bool isNormal, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev)
{

	threshold_event_type type = thr_normal;
	
	type = isNormal?thr_normal:thr_out_of_range;

	if(g_on_triggered)
		g_on_triggered(type, sensorname, value, attr, pRev);
}

void* HandlerKernel_ThreadGetSensor(void *args)
{
	get_data_t *root = NULL;
	sensor_cmd_t *pHandlerContex = (sensor_cmd_t *)args;
	int replyID = pHandlerContex->replyID;
	cJSON *pReqItemRoot = cJSON_Parse(pHandlerContex->reqItems);

	if(pReqItemRoot)
	{
		int size = cJSON_GetArraySize(pReqItemRoot);
		int i=0;
		get_data_t *current = NULL;
		for(i=0;i<size;i++)
		{
			MSG_ATTRIBUTE_T* attr = NULL;
			cJSON* nNode = NULL;
			cJSON* item = cJSON_GetArrayItem(pReqItemRoot, i);
			if(item == NULL)
				continue;
			nNode = cJSON_GetObjectItem(item, "n");
			if(nNode == NULL)
				continue;
			 attr = IoT_FindSensorNodeWithPath(g_HandlerCapability, nNode->valuestring);
			 if(attr == NULL)
			 {
				 cJSON_AddStringToObject(item, TAG_STRING, STATUS_NOT_FOUND);
				 cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_NOT_FOUND);
			 }
			 else if(pHandlerContex->on_sensor_cmd)
			 {
				 if(strstr(attr->readwritemode,"r")>0)
				 {
					 get_data_t *target = malloc(sizeof(get_data_t));
					 memset(target, 0, sizeof(get_data_t));
					 strcpy(target->sensorname, nNode->valuestring);
					 target->attr = attr;
					 target->pNode = (void*)item;

					 if(current == NULL)
						root = current = target;
					 else
					 {
						 current->next = target;
						 current = target;
					 }
				 }
				 else
				 {
					 cJSON_AddStringToObject(item, TAG_STRING, STATUS_REQUEST_ERROR);
					 cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_REQUEST_ERROR);
				 }
			 }
			 else
			 {
				 if(strstr(attr->readwritemode,"r")>0)
				 {
					switch(attr->type)
					 {
					 case attr_type_numeric:
						 cJSON_AddNumberToObject(item, TAG_VALUE, attr->v);
						 break;
					 case attr_type_boolean:
						 cJSON_AddBoolToObject(item, TAG_BOOLEAN, attr->bv);
						 break;
					 case attr_type_string:
						 cJSON_AddStringToObject(item, TAG_STRING, attr->sv);
						 break;
					case attr_type_date:
						{
						 cJSON* pDateRoot = cJSON_CreateObject();
						 cJSON_AddItemToObject(item, TAG_VALUE, pDateRoot);
						 cJSON_AddStringToObject(pDateRoot, TAG_DATE, attr->sv);
						}
						 break;
					 case attr_type_timestamp:
						 {
						 cJSON* pDateRoot = cJSON_CreateObject();
						 cJSON_AddItemToObject(item, TAG_VALUE, pDateRoot);
						 cJSON_AddNumberToObject(pDateRoot, TAG_TIMESTAMP, attr->v);
						 }
						 break;
					 }
					 cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_SUCCESS);
				 }
				 else
				 {
					 cJSON_AddStringToObject(item, TAG_STRING, STATUS_REQUEST_ERROR);
					 cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_REQUEST_ERROR);
				 }
			 }
		}

		if(pHandlerContex->on_sensor_cmd != NULL && root != NULL)
		{
			if(pHandlerContex->on_sensor_cmd(root, pHandlerContex->pRev))
			{
				get_data_t *target = root;
				while(target)
				{
					cJSON* item = (cJSON*)target->pNode;
					MSG_ATTRIBUTE_T* attr = target->attr;
					if(target->errcode == STATUSCODE_SUCCESS)
					{
						switch(target->attr->type)
						{
							case attr_type_numeric:
								cJSON_AddNumberToObject(item, TAG_VALUE, attr->v);
								break;
							case attr_type_boolean:
								cJSON_AddBoolToObject(item, TAG_BOOLEAN, attr->bv);
								break;
							case attr_type_string:
								cJSON_AddStringToObject(item, TAG_STRING, attr->sv);
								break;
							case attr_type_date:
							{
								cJSON* pDateRoot = cJSON_CreateObject();
								cJSON_AddItemToObject(item, TAG_VALUE, pDateRoot);
								cJSON_AddStringToObject(pDateRoot, TAG_DATE, attr->sv);
							}
								break;
							case attr_type_timestamp:
							{
								cJSON* pDateRoot = cJSON_CreateObject();
								cJSON_AddItemToObject(item, TAG_VALUE, pDateRoot);
								cJSON_AddNumberToObject(pDateRoot, TAG_TIMESTAMP, attr->v);
							}
								break;
						}
					}
					else
						cJSON_AddStringToObject(item, TAG_STRING, target->errstring);
					cJSON_AddNumberToObject(item, TAG_STATUSCODE, target->errcode);
					target = target->next;
				}
			}
			else
			{
				get_data_t *target = root;
				while(target)
				{
					cJSON* item = (cJSON*)target->pNode;
					cJSON_AddStringToObject(item, TAG_STRING, STATUS_FAIL);
					cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_FAIL);
					target = target->next;
				}
			}
		}
	}
	if(root)
	{
		get_data_t *current = NULL;
		while(root)
		{
			current = root;
			root = root->next;
			free(current);
		}
	}

	if(pReqItemRoot)
	{
		char* repStr = cJSON_PrintUnformatted(pReqItemRoot);
		char* result = NULL;
		if(HandlerParser_PackSensorCMDRep(repStr, pHandlerContex->sessionID, &result))
		{
			if(g_HandlerInfo.sendcbf)
				g_HandlerInfo.sendcbf(&g_HandlerInfo, pHandlerContex->replyID, result, strlen(result), NULL, NULL);
		}
		else
		{
			KernelHLog(Error, " %s> Generate Get Sensor Data Response Fail!",  g_HandlerInfo.Name);
		}
		if(repStr)
			free(repStr);
		if(result)
			free(result);
	}
	if(pHandlerContex->reqItems)
		free(pHandlerContex->reqItems);
	free(pHandlerContex);
	cJSON_Delete(pReqItemRoot);

GET_EXIT:
	pthread_exit(0);
	return 0;
}

void* HandlerKernel_ThreadSetSensor(void *args)
{
	set_data_t *root = NULL;
	sensor_cmd_t *pHandlerContex = (sensor_cmd_t *)args;
	int replyID = pHandlerContex->replyID;
	cJSON *pReqItemRoot = cJSON_Parse(pHandlerContex->reqItems);

	if(pReqItemRoot)
	{
		int size = cJSON_GetArraySize(pReqItemRoot);
		int i=0;
		set_data_t *current = NULL;
		for(i=0;i<size;i++)
		{
			MSG_ATTRIBUTE_T* attr = NULL;
			cJSON* nNode = NULL;
			cJSON* item = cJSON_GetArrayItem(pReqItemRoot, i);
			if(item == NULL)
				continue;
			nNode = cJSON_GetObjectItem(item, "n");
			if(nNode == NULL)
				continue;
			 attr = IoT_FindSensorNodeWithPath(g_HandlerCapability, nNode->valuestring);
			 if(attr == NULL)
			 {
				 cJSON_AddStringToObject(item, TAG_STRING, STATUS_NOT_FOUND);
				 cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_NOT_FOUND);
				 cJSON_DeleteItemFromObject(item, "v");
				 cJSON_DeleteItemFromObject(item, "bv");
				 cJSON_DeleteItemFromObject(item, "sv");
			 }
			 else if(pHandlerContex->on_sensor_cmd)
			 {
				  if(strstr(attr->readwritemode,"w")>0)
				  {
					bool bDiff = false;
					cJSON* nValue = NULL;
					set_data_t *target = malloc(sizeof(set_data_t));
					memset(target, 0, sizeof(set_data_t));
					strcpy(target->sensorname, nNode->valuestring);
					target->attr = attr;
					target->pNode = (void*)item;
					if((nValue = cJSON_GetObjectItem(item, "v")) != NULL)
					{
						if(attr->type != attr_type_numeric || nValue->type != cJSON_Number)
							bDiff = true;
						else
						{
							target->newtype = attr_type_numeric;
							target->v = nValue->valuedouble;
						}
						cJSON_DeleteItemFromObject(item, "v");
					}
					else if((nValue = cJSON_GetObjectItem(item, "bv")) != NULL)
					{
						if(attr->type != attr_type_boolean || (nValue->type != cJSON_True && nValue->type != cJSON_False))
							bDiff = true;
						else
						{
							target->newtype = attr_type_boolean;
							target->bv = nValue->type==cJSON_True?true:false;
						}
						cJSON_DeleteItemFromObject(item, "bv");
					}
					else if((nValue = cJSON_GetObjectItem(item, "sv")) != NULL)
					{
						if(attr->type != attr_type_string || nValue->type != cJSON_String)
							bDiff = true;
						else
						{
							 target->newtype = attr_type_string;
							 target->sv = calloc(1, strlen(nValue->valuestring)+1);
							 strcpy(target->sv, nValue->valuestring);
						}
						cJSON_DeleteItemFromObject(item, "sv");
					}
					if(bDiff)
					{
						free(target);
						cJSON_AddStringToObject(item, TAG_STRING, STATUS_FORMAT_ERROR);
						cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_FORMAT_ERROR);
					}
					else
					{
						if(current == NULL)
							root = current = target;
						else
						{
							current->next = target;
							current = target;
						}
					}
				 }
				 else
				 {
					 cJSON_AddStringToObject(item, TAG_STRING, STATUS_REQUEST_ERROR);
					 cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_REQUEST_ERROR);
					 cJSON_DeleteItemFromObject(item, "v");
					 cJSON_DeleteItemFromObject(item, "bv");
					 cJSON_DeleteItemFromObject(item, "sv");
				 }
			 }
			 else
			 {
				 cJSON_AddStringToObject(item, TAG_STRING, STATUS_NOT_IMPLEMENT);
				 cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_NOT_IMPLEMENT);
				 cJSON_DeleteItemFromObject(item, "v");
				 cJSON_DeleteItemFromObject(item, "bv");
				 cJSON_DeleteItemFromObject(item, "sv");
			 }
		}

		if(pHandlerContex->on_sensor_cmd != NULL && root != NULL)
		{
			if(pHandlerContex->on_sensor_cmd(root, pHandlerContex->pRev))
			{
				set_data_t *target = root;
				while(target)
				{
					cJSON* item = (cJSON*)target->pNode;
					cJSON_AddStringToObject(item, TAG_STRING, target->errstring);
					cJSON_AddNumberToObject(item, TAG_STATUSCODE, target->errcode);
					target = target->next;
				}
			}
			else
			{
				set_data_t *target = root;
				while(target)
				{
					cJSON* item = (cJSON*)target->pNode;
					cJSON_AddStringToObject(item, TAG_STRING, STATUS_FAIL);
					cJSON_AddNumberToObject(item, TAG_STATUSCODE, STATUSCODE_FAIL);
					target = target->next;
				}
			}
		}
	}

	if(root)
	{
		set_data_t *current = NULL;
		while(root)
		{
			current = root;
			root = root->next;

			if(current->newtype == attr_type_string)
			{
				if(current->sv)
					free(current->sv);
			}

			free(current);
		}
	}

	if(pReqItemRoot)
	{
		char* repStr = cJSON_PrintUnformatted(pReqItemRoot);
		char* result = NULL;
		if(HandlerParser_PackSensorCMDRep(repStr, pHandlerContex->sessionID, &result))
		{
			if(g_HandlerInfo.sendcbf)
				g_HandlerInfo.sendcbf(&g_HandlerInfo, pHandlerContex->replyID, result, strlen(result), NULL, NULL);
		}
		else
		{
			KernelHLog(Error, " %s> Generate Get Sensor Data Response Fail!",  g_HandlerInfo.Name);
		}
		if(repStr)
			free(repStr);
		if(result)
			free(result);
	}
	if(pHandlerContex->reqItems)
		free(pHandlerContex->reqItems);
	free(pHandlerContex);
	cJSON_Delete(pReqItemRoot);

GET_EXIT:
	pthread_exit(0);
	return 0;
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
	memset(&g_HandlerInfo, 0, sizeof(HANDLER_INFO));

	// 2. Copy agent info 
	memcpy(&g_HandlerInfo, handler, sizeof(HANDLER_INFO));

	// 3. Init variable
	g_status = handler_status_init;

	memset(&g_AutoReportContex, 0, sizeof(handler_context_t));
	g_AutoReportContex.interval = 1000;
	pthread_mutex_init(&g_AutoReportContex.mux, NULL);

	memset(&g_LiveReportContex, 0, sizeof(handler_context_t));
	g_LiveReportContex.replyID = hk_auto_upload_rep;
	g_LiveReportContex.interval = 1000;
	pthread_mutex_init(&g_LiveReportContex.mux, NULL);

	memset(&g_ThresholdChkContex, 0, sizeof(handler_context_t));
	g_ThresholdChkContex.interval = 1000;
	pthread_mutex_init(&g_ThresholdChkContex.mux, NULL);

	triggerqueue_init(10, HandlerKernel_On_Triggered);

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
	memset(&g_HandlerInfo, 0, sizeof(HANDLER_INFO));

	//pthread_mutex_lock(&g_AutoReportContex.mux);
	if(g_AutoReportContex.isThreadRunning)
	{
		g_AutoReportContex.isThreadRunning = false;
		pthread_cancel(g_AutoReportContex.threadHandler);
	}
	//pthread_mutex_unlock(&g_AutoReportContex.mux);


	//pthread_mutex_lock(&g_LiveReportContex.mux);
	if(g_LiveReportContex.isThreadRunning)
	{
		g_LiveReportContex.isThreadRunning = false;
		pthread_cancel(g_LiveReportContex.threadHandler);
	}
	//pthread_mutex_unlock(&g_LiveReportContex.mux);

	//pthread_mutex_lock(&g_ThresholdChkContex.mux);
	if(g_ThresholdChkContex.isThreadRunning)
	{
		g_ThresholdChkContex.isThreadRunning = false;
		pthread_cancel(g_ThresholdChkContex.threadHandler);
	}
	//pthread_mutex_unlock(&g_ThresholdChkContex.mux);

	triggerqueue_uninit();

	g_status = handler_status_no_init;
	
	pthread_mutex_lock(&g_ThresholdChkContex.mux);
		pthread_mutex_lock(&g_AutoReportContex.mux);
			pthread_mutex_lock(&g_LiveReportContex.mux);
				g_HandlerCapability = NULL;
			pthread_mutex_unlock(&g_LiveReportContex.mux);
		pthread_mutex_unlock(&g_AutoReportContex.mux);
		HandlerThreshold_DestroyThrList(g_ThresholdList);
	pthread_mutex_unlock(&g_ThresholdChkContex.mux);

	pthread_mutex_destroy(&g_AutoReportContex.mux);
	if(g_AutoReportContex.reqItems)
		free(g_AutoReportContex.reqItems);
	g_AutoReportContex.reqItems = NULL;

	pthread_mutex_destroy(&g_LiveReportContex.mux);
	if(g_LiveReportContex.reqItems)
		free(g_LiveReportContex.reqItems);
	g_LiveReportContex.reqItems = NULL;

	pthread_mutex_destroy(&g_ThresholdChkContex.mux);
	if(g_ThresholdChkContex.reqItems)
		free(g_ThresholdChkContex.reqItems);
	g_ThresholdChkContex.reqItems = NULL;

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
	char *strCapability = NULL;
	if( pCapability == NULL )
		return handler_fail;

	pthread_mutex_lock(&g_ThresholdChkContex.mux);
		pthread_mutex_lock(&g_AutoReportContex.mux);
			pthread_mutex_lock(&g_LiveReportContex.mux);
				g_HandlerCapability = pCapability;
				strCapability = IoT_PrintCapability(g_HandlerCapability);
			pthread_mutex_unlock(&g_LiveReportContex.mux);
		pthread_mutex_unlock(&g_AutoReportContex.mux);
	pthread_mutex_unlock(&g_ThresholdChkContex.mux);

	if(strCapability)
	{
		if(bPublish && g_HandlerInfo.sendcapabilitycbf)
			g_HandlerInfo.sendcapabilitycbf(&g_HandlerInfo, strCapability, strlen(strCapability), NULL, NULL);
		free(strCapability);
	}

	return handler_success;
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
	unsigned int interval = 0;
	char* reqItems = NULL;
	bool bReqAll = false;
	if( pInQuery == NULL )
		return handler_fail;

	/*create thread to report sensor data*/
	if(!HandlerParser_ParseAutoReportCmd(pInQuery, g_HandlerInfo.Name, &interval, &reqItems, &bReqAll))
	{
		KernelHLog(Warning, " %s> Auto Report CMD Parser Failed: %s", g_HandlerInfo.Name, pInQuery); 
		return handler_fail;
	}

	pthread_mutex_lock(&g_AutoReportContex.mux);
	g_AutoReportContex.interval = interval;
	g_AutoReportContex.reqAll = bReqAll;
	
	if(g_AutoReportContex.reqItems)
		free(g_AutoReportContex.reqItems);
	if(reqItems)
	{
		g_AutoReportContex.reqItems = (char*)calloc(1, strlen(reqItems)+1);
		strcpy(g_AutoReportContex.reqItems, reqItems);
		free(reqItems);
	}
	pthread_mutex_unlock(&g_AutoReportContex.mux);

	pthread_mutex_lock(&g_AutoReportContex.mux);
	if(!g_AutoReportContex.isThreadRunning)
	{
		g_AutoReportContex.isThreadRunning = true;
		if (pthread_create(&g_AutoReportContex.threadHandler, NULL, HandlerKernel_AutoReportThread, &g_AutoReportContex) != 0)
		{
			g_AutoReportContex.isThreadRunning = false;
			KernelHLog(Warning, " %s> start thread failed!", g_HandlerInfo.Name);	
		}
		else
		{
			pthread_detach(g_AutoReportContex.threadHandler);
			g_AutoReportContex.threadHandler = NULL;
		}
	}
	pthread_mutex_unlock(&g_AutoReportContex.mux);

	return handler_success;
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
	bool bStopThread = true;
	//if( pInQuery == NULL )
	//	bStopThread = true;

	if(bStopThread)
	{
		pthread_mutex_lock(&g_AutoReportContex.mux);
		if(g_AutoReportContex.isThreadRunning)
		{
			g_AutoReportContex.isThreadRunning = false;
			//pthread_join(g_AutoReportContex.threadHandler, NULL);
			//g_AutoReportContex.threadHandler = NULL;
		}
		pthread_mutex_unlock(&g_AutoReportContex.mux);
	}

	return handler_success;
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
	unsigned int interval = 0;
	char* reqItems = NULL;
	bool bReqAll = false;
	if( pInQuery == NULL )
		return handler_fail;

	/*create thread to report sensor data*/
	if(!HandlerParser_ParseAutoReportCmd(pInQuery, g_HandlerInfo.Name, &interval, &reqItems, &bReqAll))
	{
		KernelHLog(Warning, " %s> Auto Report CMD Parser Failed: %s", g_HandlerInfo.Name, pInQuery); 
		return handler_fail;
	}

	pthread_mutex_lock(&g_AutoReportContex.mux);
	g_AutoReportContex.reqAll = bReqAll;
	
	if(g_AutoReportContex.reqItems)
		free(g_AutoReportContex.reqItems);
	if(reqItems)
	{
		g_AutoReportContex.reqItems = (char*)calloc(1, strlen(reqItems)+1);
		strcpy(g_AutoReportContex.reqItems, reqItems);
		free(reqItems);
	}
	pthread_mutex_unlock(&g_AutoReportContex.mux);

	return handler_success;
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
	int iRet = cagent_send_data_error;
	char* message = NULL;		
	pthread_mutex_unlock(&g_AutoReportContex.mux);
	if(g_HandlerCapability)
	{
		if(g_AutoReportContex.reqAll)
			message = IoT_PrintData(g_HandlerCapability);
		else
			message = IoT_PrintSelectedData(g_HandlerCapability, g_AutoReportContex.reqItems);
	}
	pthread_mutex_unlock(&g_AutoReportContex.mux);

	if(message)
	{
		if(g_HandlerInfo.sendreportcbf)
			g_HandlerInfo.sendreportcbf(/*Handler Info*/&g_HandlerInfo, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);
		free(message);
	}
	if(iRet == cagent_success)
		return handler_success;
	else
		return handler_fail;
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
	unsigned int interval = 0;
	unsigned int timeout = 0;
	char* reqItems = NULL;
	bool bReqAll = false;
	if( pInQuery == NULL )
		return handler_fail;

	/*create thread to report sensor data*/
	if(!HandlerParser_ParseAutoUploadCmd(pInQuery, g_HandlerInfo.Name, &interval, &timeout, &reqItems, &bReqAll))
	{
		KernelHLog(Warning, " %s> Live Report CMD Parser Failed: %s", g_HandlerInfo.Name, pInQuery); 
		return handler_fail;
	}

	pthread_mutex_lock(&g_LiveReportContex.mux);
	g_LiveReportContex.replyID = replyID;
	g_LiveReportContex.interval = interval;
	g_LiveReportContex.timeout = timeout;
	g_LiveReportContex.reqAll = bReqAll;
	
	if(g_LiveReportContex.reqItems)
		free(g_LiveReportContex.reqItems);
	if(reqItems)
	{
		g_LiveReportContex.reqItems = (char*)calloc(1, strlen(reqItems)+1);
		strcpy(g_LiveReportContex.reqItems, reqItems);
		free(reqItems);
	}
	pthread_mutex_unlock(&g_LiveReportContex.mux);

	pthread_mutex_lock(&g_LiveReportContex.mux);
	if(!g_LiveReportContex.isThreadRunning)
	{
		g_LiveReportContex.isThreadRunning = true;
		if (pthread_create(&g_LiveReportContex.threadHandler, NULL, HandlerKernel_LiveReportThread, &g_LiveReportContex) != 0)
		{
			g_LiveReportContex.isThreadRunning = false;
			KernelHLog(Warning, " %s> start thread failed!", g_HandlerInfo.Name);	
		}
		else
		{
			pthread_detach(g_LiveReportContex.threadHandler);
			g_LiveReportContex.threadHandler = NULL;
		}
	}
	pthread_mutex_unlock(&g_LiveReportContex.mux);

	return handler_success;
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

	if(HandlerParser_ParseReceivedCMD(pInQuery, strlen(pInQuery), cmdID, NULL))
		return handler_success;
	else
		return handler_fail;
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

	if(HandlerParser_ParseReceivedCMD(pInQuery, strlen(pInQuery), cmdID, sessionID))
		return handler_success;
	else
		return handler_fail;
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
	int iRet = handler_fail;
	char * repJsonStr = NULL;
	char repMsg[256] = {0};
	thr_item_list tmpThrItemList = NULL;
	char* buffer = NULL;	
	char* message = NULL;
	HANDLER_NOTIFY_SEVERITY severity = Severity_Debug;

	if( pInQuery == NULL)
		return handler_fail;

	tmpThrItemList = HandlerThreshold_CreateThrList();
	if(!HandlerParser_ParseThrInfo(pInQuery, tmpThrItemList, triggerqueue_push))
	{
		sprintf(repMsg, "%s", "Threshold apply failed!");
		iRet = handler_fail;
		goto SET_THRESHOLD_EXIT;
	}

	pthread_mutex_lock(&g_ThresholdChkContex.mux);
	if(g_ThresholdList == NULL)
		g_ThresholdList = HandlerThreshold_CreateThrList();

	buffer = calloc(1,1024);	
	if(HandlerThreshold_UpdateThrInfoList(g_ThresholdList, tmpThrItemList, &buffer, 1024))
	{
		if(strlen(buffer) > 0)
		{
			bool bNormal = false;
			HandlerThreshold_IsThrItemListNormal(g_ThresholdList, &bNormal);
			message = calloc(1, strlen(buffer) + 128);
			sprintf(message,"{\"subtype\":\"THRESHOLD_CHECK_INFO\",\"thrCheckStatus\":\"%s\",\"msg\":\"%s\"}", bNormal?"True":"False", buffer); /*for custom handler*/
			//sprintf(message,"{\"thrCheckStatus\":\"True\",\"thrCheckMsg\":\"%s\"}", buffer); /*original for standard handler*/
		}
	}
	free(buffer);
	pthread_mutex_unlock(&g_ThresholdChkContex.mux);

	sprintf(repMsg, "%s", "Threshold rule apply OK!");
	iRet = handler_success;

SET_THRESHOLD_EXIT:
	HandlerThreshold_DestroyThrList(tmpThrItemList);
	if(HandlerParser_PackSetThrRep(repMsg, &repJsonStr))
	{
		if(g_HandlerInfo.sendcbf)
			g_HandlerInfo.sendcbf(&g_HandlerInfo, replyID, repJsonStr, strlen(repJsonStr), NULL, NULL);
	}
	if(repJsonStr)free(repJsonStr);	

	if(message)
	{
		if(g_HandlerInfo.sendeventcbf && strlen(message) > 0)
			g_HandlerInfo.sendeventcbf(/*Handler Info*/&g_HandlerInfo, Severity_Informational, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);
		free(message);
	}

	return iRet;
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
	char * repJsonStr = NULL;
	char repMsg[256] = {0};
	char* buffer = NULL;	
	char* message = NULL;
	pthread_mutex_lock(&g_ThresholdChkContex.mux);
	buffer = calloc(1,1024);
	HandlerThreshold_WhenDelThrCheckNormal(g_ThresholdList, &buffer, 1024);
	if(strlen(buffer) > 0)
	{
		message = calloc(1, strlen(buffer) + 64);
		sprintf(message,"{\"subtype\":\"THRESHOLD_CHECK_INFO\",\"thrCheckStatus\":\"True\",\"msg\":\"%s\"}", buffer); /*for custom handler*/
		//sprintf(message,"{\"thrCheckStatus\":\"True\",\"thrCheckMsg\":\"%s\"}", buffer); /*original for standard handler*/
	}
	free(buffer);
	HandlerThreshold_DestroyThrList(g_ThresholdList);
	g_ThresholdList = NULL;
	pthread_mutex_unlock(&g_ThresholdChkContex.mux);

	sprintf(repMsg, "%s", "Delete all threshold successfully!");

	if(HandlerParser_PackDelAllThrRep(repMsg, &repJsonStr))
	{
		if(g_HandlerInfo.sendcbf)
			g_HandlerInfo.sendcbf(&g_HandlerInfo, replyID, repJsonStr, strlen(repJsonStr), NULL, NULL);
	}
	if(repJsonStr) free(repJsonStr);	

	if(message)
	{
		if(g_HandlerInfo.sendeventcbf && strlen(message) > 0)
			g_HandlerInfo.sendeventcbf(/*Handler Info*/&g_HandlerInfo, Severity_Informational, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);
		free(message);
	}

	return handler_success;
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
	if(g_ThresholdList == NULL)
		return handler_fail;

	pthread_mutex_lock(&g_ThresholdChkContex.mux);
	if(!g_ThresholdChkContex.isThreadRunning)
	{
		g_ThresholdChkContex.isThreadRunning = true;
		if (pthread_create(&g_ThresholdChkContex.threadHandler, NULL, HandlerKernel_ThresholdCheckThread, &g_ThresholdChkContex) != 0)
		{
			g_ThresholdChkContex.isThreadRunning = false;
			KernelHLog(Warning, " %s> start thread failed!", g_HandlerInfo.Name);	
		}
		else
		{
			pthread_detach(g_ThresholdChkContex.threadHandler);
			g_ThresholdChkContex.threadHandler = NULL;
		}
	}
	pthread_mutex_unlock(&g_ThresholdChkContex.mux);

	return handler_success;
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
	if(g_ThresholdList == NULL)
		return handler_fail;

	pthread_mutex_lock(&g_ThresholdChkContex.mux);
	if(g_ThresholdChkContex.isThreadRunning)
	{
		g_ThresholdChkContex.isThreadRunning = false;
	}
	pthread_mutex_unlock(&g_ThresholdChkContex.mux);

	return handler_success;
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
	pthread_t threadctx=0;
	sensor_cmd_t* getcmd = NULL;

	if( pInQuery == NULL )
		return handler_fail;

	getcmd = (sensor_cmd_t*)malloc(sizeof(sensor_cmd_t));
	strcpy(getcmd->sessionID, sessionID);
	getcmd->replyID = replyID;
	getcmd->on_sensor_cmd = on_get_sensor;
	if(!HandlerParser_ParseSensorDataCmd(pInQuery, &getcmd->reqItems))
	{
		KernelHLog(Warning, " %s> Get Sensor CMD Parser Failed: %s", g_HandlerInfo.Name, pInQuery); 
		free(getcmd);
		return handler_fail;
	}
	
	if (pthread_create(&threadctx, NULL, HandlerKernel_ThreadGetSensor, getcmd) != 0)
	{
		free(getcmd);
		KernelHLog(Warning, " %s> start get sensor data thread failed!", g_HandlerInfo.Name);	
	}
	else
	{
		pthread_detach(threadctx);
		threadctx = NULL;
	}

	return handler_success;
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
	pthread_t threadctx=0;
	sensor_cmd_t* setcmd = NULL;

	if( pInQuery == NULL )
		return handler_fail;

	setcmd = (sensor_cmd_t*)malloc(sizeof(sensor_cmd_t));
	strcpy(setcmd->sessionID, sessionID);
	setcmd->replyID = replyID;
	setcmd->on_sensor_cmd = on_set_sensor;
	if(!HandlerParser_ParseSensorDataCmd(pInQuery, &setcmd->reqItems))
	{
		KernelHLog(Warning, " %s> Get Sensor CMD Parser Failed: %s", g_HandlerInfo.Name, pInQuery); 
		free(setcmd);
		return handler_fail;
	}

	if (pthread_create(&threadctx, NULL, HandlerKernel_ThreadSetSensor, setcmd) != 0)
	{
		free(setcmd);
		KernelHLog(Warning, " %s> start get sensor data thread failed!", g_HandlerInfo.Name);	
	}
	else
	{
		pthread_detach(threadctx);
		threadctx = NULL;
	}
	return handler_success;
}