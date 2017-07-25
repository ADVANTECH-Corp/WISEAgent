#include "HandlerKernelEx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "HandlerParser.h"
#include "HandlerThreshold.h"
#include "Log.h"
#include "triggerqueue.h"
#include "cJSON.h"

//-----------------------------------------------------------------------------
// Logger defines:
//-----------------------------------------------------------------------------
#define HandlerKernelEx_LOG_ENABLE
//#define DEF_HandlerKernelEx_LOG_MODE    (LOG_MODE_NULL_OUT)
//#define DEF_HandlerKernelEx_LOG_MODE    (LOG_MODE_FILE_OUT)
#define DEF_HandlerKernelEx_LOG_MODE    (LOG_MODE_CONSOLE_OUT|LOG_MODE_FILE_OUT)

#ifdef HandlerKernelEx_LOG_ENABLE
#define KernelHExLog(handle, level, fmt, ...)  do { if (handle != NULL)   \
	WriteLog(handle, DEF_HandlerKernelEx_LOG_MODE, level, fmt, ##__VA_ARGS__); } while(0)
#else
#define KernelHExLog(handle, level, fmt, ...)
#endif

//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------

typedef struct{
   pthread_mutex_t mux;
   unsigned int intervalMS;
   unsigned int timeoutMS;
   bool reqAll;
   char* reqItems;
   bool enable;
   int replyID;
   bool bModified;
   long long nextTime;
   long long timeout;
}handler_context_t;

typedef void (*on_threshold_triggered)(threshold_event_type type, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev);

typedef struct{
	LOGHANDLE pHandlerkernellog;
	Handler_info_ex  HandlerInfo;
	handler_context_t AutoReportContex;
	handler_context_t LiveReportContex;
	handler_context_t ThresholdChkContex;
	//handler_context_t InternelReportContex;
	HANDLER_THREAD_STATUS Status;
	MSG_CLASSIFY_T* pCapability;
	thr_item_list pThresholdList;
	void* pTriggerQueue;
	pthread_mutex_t CapabilityMux;
	
	void* threadHandler;
	bool isThreadRunning;
}handler_kernel_t;

typedef struct{
   char* reqItems;
   char sessionID[33];
   int replyID;
   void *pRev;
   bool (*on_sensor_cmd)(void* objlist, void *pRev);
   handler_kernel_t* pHandler;
}sensor_cmd_t;



//-----------------------------------------------------------------------------
// Internal Function:
//-----------------------------------------------------------------------------
bool HandlerKernelEx_CheckToExitThread(handler_kernel_t* pHandlerKernel)
{
	bool bStop = false;
	//pthread_mutex_lock(&pHandlerContex->mux);
	bStop = !pHandlerKernel->isThreadRunning;
	//pthread_mutex_unlock(&pHandlerContex->mux);
	return bStop;
}

long long HandlerKernelEx_GetTimeTick()
{
	long long tick = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tick = (long long)tv.tv_sec*1000 + (long long)tv.tv_usec/1000;
	return tick;
}

void* HandlerKernelEx_MonitorThread(void *args)
{
	handler_kernel_t* pHandlerKernel = NULL;
	int interval=1000;
	long long curTime = 0;

	if(args == NULL)
	{
		pthread_exit(0);
		return 0;
	}

	pHandlerKernel = (handler_kernel_t *)args;

	while(true)
	{
		unsigned int diffTime = 0;
		HANDLER_NOTIFY_SEVERITY severity = Severity_Debug;
		bool bAutoReport = false, bLiveReport = false, bInternal = false, bChkThreshold = false;
		
		curTime = HandlerKernelEx_GetTimeTick();

		if(HandlerKernelEx_CheckToExitThread(pHandlerKernel))
		{
			break;
		}

		pthread_mutex_lock(&pHandlerKernel->AutoReportContex.mux);
		if(pHandlerKernel->AutoReportContex.enable)
		{
			if(pHandlerKernel->AutoReportContex.nextTime == 0)
				pHandlerKernel->AutoReportContex.nextTime = curTime;
			if(pHandlerKernel->AutoReportContex.nextTime <= curTime)
				bAutoReport = true;
		}
		pthread_mutex_unlock(&pHandlerKernel->AutoReportContex.mux);
		if(pHandlerKernel->pCapability)
		{
			if(bAutoReport)
			{
				char* buff = NULL;
				pthread_mutex_lock(&pHandlerKernel->CapabilityMux);
				if(pHandlerKernel->AutoReportContex.reqAll)
					buff = IoT_PrintData(pHandlerKernel->pCapability);
				else
					buff = IoT_PrintSelectedData(pHandlerKernel->pCapability, pHandlerKernel->AutoReportContex.reqItems);
				pthread_mutex_unlock(&pHandlerKernel->CapabilityMux);
				if(pHandlerKernel->HandlerInfo.sendreportcbf && buff)
					pHandlerKernel->HandlerInfo.sendreportcbf(/*Handler Info*/&pHandlerKernel->HandlerInfo, /*message data*/buff, /*message length*/strlen(buff), /*preserved*/NULL, /*preserved*/NULL);
				free(buff);
				pHandlerKernel->AutoReportContex.nextTime += pHandlerKernel->AutoReportContex.intervalMS;
			}
		}

		if(HandlerKernelEx_CheckToExitThread(pHandlerKernel))
		{
			break;
		}

		pthread_mutex_lock(&pHandlerKernel->LiveReportContex.mux);
		if(pHandlerKernel->LiveReportContex.enable)
		{
			if(pHandlerKernel->LiveReportContex.nextTime == 0)
				pHandlerKernel->LiveReportContex.nextTime = curTime;
			if(pHandlerKernel->LiveReportContex.timeout == 0)
				pHandlerKernel->LiveReportContex.timeout = curTime + pHandlerKernel->LiveReportContex.timeoutMS;

			if(pHandlerKernel->LiveReportContex.nextTime <= curTime)
			{
				if(pHandlerKernel->LiveReportContex.timeout <= curTime)
				{
					pHandlerKernel->LiveReportContex.timeoutMS = 0;
					pHandlerKernel->LiveReportContex.timeout = 0;
					pHandlerKernel->LiveReportContex.nextTime = 0;
					pHandlerKernel->LiveReportContex.enable = false;
				}
				else
					bLiveReport = true;
			}
		}
		pthread_mutex_unlock(&pHandlerKernel->LiveReportContex.mux);
		if(pHandlerKernel->pCapability)
		{
			if(bLiveReport)
			{
				char* buff = NULL;
				pthread_mutex_lock(&pHandlerKernel->CapabilityMux);
				if(pHandlerKernel->LiveReportContex.reqAll)
					buff = IoT_PrintData(pHandlerKernel->pCapability);
				else
					buff = IoT_PrintSelectedData(pHandlerKernel->pCapability, pHandlerKernel->LiveReportContex.reqItems);
				pthread_mutex_unlock(&pHandlerKernel->CapabilityMux);
				if(pHandlerKernel->HandlerInfo.sendcbf  && buff)
					pHandlerKernel->HandlerInfo.sendcbf(/*Handler Info*/&pHandlerKernel->HandlerInfo, pHandlerKernel->LiveReportContex.replyID, /*message data*/buff, /*message length*/strlen(buff), /*preserved*/NULL, /*preserved*/NULL);
				free(buff);
				pHandlerKernel->LiveReportContex.nextTime += pHandlerKernel->LiveReportContex.intervalMS;
			}
		}

		if(HandlerKernelEx_CheckToExitThread(pHandlerKernel))
		{
			break;
		}
		
		//pthread_mutex_lock(&pHandlerKernel->InternelReportContex.mux);
		//if(pHandlerKernel->InternelReportContex.enable)
		//{
		//	if(pHandlerKernel->InternelReportContex.nextTime == 0)
		//		pHandlerKernel->InternelReportContex.nextTime = curTime;
		//	if(pHandlerKernel->InternelReportContex.nextTime <= curTime)
		//		if(pHandlerKernel->InternelReportContex.bModified)
		//			bInternal = true;
		//}
		//pthread_mutex_unlock(&pHandlerKernel->InternelReportContex.mux);
		//if(pHandlerKernel->pCapability)
		//{
		//	if(bInternal)
		//	{
		//		char* buff = NULL;
		//		pthread_mutex_lock(&pHandlerKernel->CapabilityMux);
		//		buff = IoT_PrintFlatData(pHandlerKernel->pCapability, false);
		//		pthread_mutex_unlock(&pHandlerKernel->CapabilityMux);
		//		if(pHandlerKernel->HandlerInfo.internelreportcbf  && buff)
		//			pHandlerKernel->HandlerInfo.internelreportcbf(/*Handler Info*/&pHandlerKernel->HandlerInfo, /*message data*/buff, /*message length*/strlen(buff), /*preserved*/NULL, /*preserved*/NULL);
		//		free(buff);
		//		pHandlerKernel->InternelReportContex.nextTime += pHandlerKernel->InternelReportContex.intervalMS;
		//	}
		//}

		if(HandlerKernelEx_CheckToExitThread(pHandlerKernel))
		{
			break;
		}

		pthread_mutex_lock(&pHandlerKernel->ThresholdChkContex.mux);
		if(pHandlerKernel->ThresholdChkContex.enable)
		{
			if(pHandlerKernel->ThresholdChkContex.nextTime == 0)
				pHandlerKernel->ThresholdChkContex.nextTime = curTime;
			if(pHandlerKernel->ThresholdChkContex.nextTime <= curTime)
				bChkThreshold = true;
		}
		pthread_mutex_unlock(&pHandlerKernel->ThresholdChkContex.mux);
		if(pHandlerKernel->pCapability)
		{
			if(pHandlerKernel->pThresholdList != NULL && bChkThreshold)
			{
				char* buffer = calloc(1,1024);
				char* buff = NULL;
				int buffsize = 0;
				bool bNormal = false;
				bool bResult = false;
				pthread_mutex_lock(&pHandlerKernel->CapabilityMux);
				bResult = HandlerThreshold_CheckThr(pHandlerKernel->pThresholdList, pHandlerKernel->pCapability, &buffer, 1024, &bNormal);
				pthread_mutex_unlock(&pHandlerKernel->CapabilityMux);
				if(bResult)
				{
					if(strlen(buffer) > 0)
					{
						if(bNormal)
						{
							severity = Severity_Informational;
							buffsize = strlen("{\"subtype\":\"THRESHOLD_CHECK_INFO\",\"thrCheckStatus\":\"%s\",\"msg\":\"%s\"}") + 5 + strlen(buffer);
							buff = calloc(1, buffsize+1);
							sprintf(buff,"{\"subtype\":\"THRESHOLD_CHECK_INFO\",\"thrCheckStatus\":\"%s\",\"msg\":\"%s\"}", bNormal?"True":"False", buffer); /*for custom handler*/
						}
						else
						{
							severity = Severity_Error;
							buffsize = strlen("{\"subtype\":\"THRESHOLD_CHECK_ERROR\",\"thrCheckStatus\":\"%s\",\"msg\":\"%s\"}") + 5 + strlen(buffer);
							buff = calloc(1, buffsize+1);
							sprintf(buff,"{\"subtype\":\"THRESHOLD_CHECK_ERROR\",\"thrCheckStatus\":\"%s\",\"msg\":\"%s\"}", bNormal?"True":"False", buffer); /*for custom handler*/
						}
						//sprintf(pHandlerKernel->ThresholdChkContex.msg,"{\"thrCheckStatus\":\"%s\",\"thrCheckMsg\":\"%s\"}", bNormal?"True":"False", buffer); /*original for standard handler*/
					}
				}
				free(buffer);

				if(pHandlerKernel->HandlerInfo.sendeventcbf && buff)
					pHandlerKernel->HandlerInfo.sendeventcbf(/*Handler Info*/&pHandlerKernel->HandlerInfo, severity, /*message data*/buff, /*message length*/strlen(buff), /*preserved*/NULL, /*preserved*/NULL);
				free(buff);
				pHandlerKernel->ThresholdChkContex.nextTime += pHandlerKernel->ThresholdChkContex.intervalMS;
			}
		}

		if(HandlerKernelEx_CheckToExitThread(pHandlerKernel))
		{
			break;
		}
		usleep(interval*1000);
	}

	pthread_exit(0);
	return 0;
}

void* HandlerKernelEx_ThreadGetSensor(void *args)
{
	get_data_t *root = NULL;
	handler_kernel_t* pHandlerKernel = NULL;
	sensor_cmd_t *pHandlerContex = NULL;
	int replyID = hk_get_sensors_data_rep;
	cJSON *pReqItemRoot = NULL;

	if(args == NULL)
	{
		goto GET_EXIT;
	}

	pHandlerContex = (sensor_cmd_t *)args;
	pHandlerKernel = pHandlerContex->pHandler;
	replyID = pHandlerContex->replyID;
	pReqItemRoot = cJSON_Parse(pHandlerContex->reqItems);

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
			nNode = cJSON_GetObjectItem(item, TAG_ATTR_NAME);
			if(nNode == NULL)
				continue;
			 pthread_mutex_lock(&pHandlerKernel->CapabilityMux);
			 attr = IoT_FindSensorNodeWithPath(pHandlerKernel->pCapability, nNode->valuestring);
			 pthread_mutex_unlock(&pHandlerKernel->CapabilityMux);
			 if(attr == NULL)
			 {
				 cJSON_AddStringToObject(item, TAG_STRING, STATUS_NOT_FOUND);
				 cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_NOT_FOUND);
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
					 cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_REQUEST_ERROR);
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
					 cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_SUCCESS);
				 }
				 else
				 {
					 cJSON_AddStringToObject(item, TAG_STRING, STATUS_REQUEST_ERROR);
					 cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_REQUEST_ERROR);
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
					cJSON_AddNumberToObject(item, TAG_STATUS_CODE, target->errcode);
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
					cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_FAIL);
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
			if(pHandlerKernel->HandlerInfo.sendcbf)
				pHandlerKernel->HandlerInfo.sendcbf(&pHandlerKernel->HandlerInfo, pHandlerContex->replyID, result, strlen(result), NULL, NULL);
		}
		else
		{
			KernelHExLog(pHandlerKernel->pHandlerkernellog , Error, " %s> Generate Get Sensor Data Response Fail!",  pHandlerKernel->HandlerInfo.Name);
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

void* HandlerKernelEx_ThreadSetSensor(void *args)
{
	set_data_t *root = NULL;
	handler_kernel_t* pHandlerKernel = NULL;
	sensor_cmd_t *pHandlerContex = NULL;
	int replyID = hk_set_sensors_data_rep;
	cJSON *pReqItemRoot = NULL;

	if(args == NULL)
	{
		goto SET_EXIT;
	}

	pHandlerContex = (sensor_cmd_t *)args;
	pHandlerKernel = pHandlerContex->pHandler;
	replyID = pHandlerContex->replyID;
	pReqItemRoot = cJSON_Parse(pHandlerContex->reqItems);

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
			nNode = cJSON_GetObjectItem(item, TAG_ATTR_NAME);
			if(nNode == NULL)
				continue;
			 pthread_mutex_lock(&pHandlerKernel->CapabilityMux);
			 attr = IoT_FindSensorNodeWithPath(pHandlerKernel->pCapability, nNode->valuestring);
			 pthread_mutex_unlock(&pHandlerKernel->CapabilityMux);
			 if(attr == NULL)
			 {
				 cJSON_AddStringToObject(item, TAG_STRING, STATUS_NOT_FOUND);
				 cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_NOT_FOUND);
				 cJSON_DeleteItemFromObject(item, TAG_VALUE);
				 cJSON_DeleteItemFromObject(item, TAG_BOOLEAN);
				 cJSON_DeleteItemFromObject(item, TAG_STRING);
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
					if((nValue = cJSON_GetObjectItem(item, TAG_VALUE)) != NULL)
					{
						if(attr->type != attr_type_numeric || nValue->type != cJSON_Number)
							bDiff = true;
						else
						{
							target->newtype = attr_type_numeric;
							target->v = nValue->valuedouble;
						}
						cJSON_DeleteItemFromObject(item, TAG_VALUE);
					}
					else if((nValue = cJSON_GetObjectItem(item, TAG_BOOLEAN)) != NULL)
					{
						if(attr->type != attr_type_boolean || (nValue->type != cJSON_True && nValue->type != cJSON_False))
							bDiff = true;
						else
						{
							target->newtype = attr_type_boolean;
							target->bv = nValue->type==cJSON_True?true:false;
						}
						cJSON_DeleteItemFromObject(item, TAG_BOOLEAN);
					}
					else if((nValue = cJSON_GetObjectItem(item, TAG_STRING)) != NULL)
					{
						if(attr->type != attr_type_string || nValue->type != cJSON_String)
							bDiff = true;
						else
						{
							 target->newtype = attr_type_string;
							 target->sv = calloc(1, strlen(nValue->valuestring)+1);
							 strcpy(target->sv, nValue->valuestring);
						}
						cJSON_DeleteItemFromObject(item, TAG_STRING);
					}
					if(bDiff)
					{
						free(target);
						cJSON_AddStringToObject(item, TAG_STRING, STATUS_FORMAT_ERROR);
						cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_FORMAT_ERROR);
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
					 cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_REQUEST_ERROR);
					 cJSON_DeleteItemFromObject(item, TAG_VALUE);
					 cJSON_DeleteItemFromObject(item, TAG_BOOLEAN);
					 cJSON_DeleteItemFromObject(item, TAG_STRING);
				 }
			 }
			 else
			 {
				 cJSON_AddStringToObject(item, TAG_STRING, STATUS_NOT_IMPLEMENT);
				 cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_NOT_IMPLEMENT);
				 cJSON_DeleteItemFromObject(item, TAG_VALUE);
				 cJSON_DeleteItemFromObject(item, TAG_BOOLEAN);
				 cJSON_DeleteItemFromObject(item, TAG_STRING);
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
					cJSON_AddNumberToObject(item, TAG_STATUS_CODE, target->errcode);
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
					cJSON_AddNumberToObject(item, TAG_STATUS_CODE, STATUSCODE_FAIL);
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
			if(pHandlerKernel->HandlerInfo.sendcbf)
				pHandlerKernel->HandlerInfo.sendcbf(&pHandlerKernel->HandlerInfo, pHandlerContex->replyID, result, strlen(result), NULL, NULL);
		}
		else
		{
			KernelHExLog(pHandlerKernel->pHandlerkernellog , Error, " %s> Generate Get Sensor Data Response Fail!",  pHandlerKernel->HandlerInfo.Name);
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

SET_EXIT:
	pthread_exit(0);
	return 0;
}

void  HandlerKernelEx_OnAttributeChanged( void* attribute, void* pRev1)
{
	handler_kernel_t* pHandlerKernel = NULL;
	if(attribute==NULL || pRev1 == NULL)
		return;
	pHandlerKernel = (handler_kernel_t*)pRev1;
	//pthread_mutex_lock(&pHandlerKernel->InternelReportContex.mux);
	//pHandlerKernel->InternelReportContex.bModified = true;
	//pthread_mutex_unlock(&pHandlerKernel->InternelReportContex.mux);
}

//-----------------------------------------------------------------------------
// API Function:
//-----------------------------------------------------------------------------

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_Initialize
 *  Description: Init any objects or variables of this handler
 *  Input : HANDLER_INFO *handler
 *  Output: None
 *  Return:  0 : Success Init Handler
 *          -1 : Fail Init Handler
 * ***************************************************************************************/
void* HANDLERKERNEL_API HandlerKernelEx_Initialize( HANDLER_INFO *handler )
{
	handler_kernel_t* pHandlerKernel = NULL;
	if( handler == NULL )
		return NULL;

	pHandlerKernel = (handler_kernel_t*) calloc(1, sizeof(handler_kernel_t));

	pHandlerKernel->pHandlerkernellog = handler->loghandle;
	// 1. Topic of this handler
	KernelHExLog(pHandlerKernel->pHandlerkernellog, Debug, " %s> Initialize",  handler->Name);
	memset(&pHandlerKernel->HandlerInfo, 0, sizeof(struct HANDLER_INFO_EX));

	// 2. Copy agent info 
	memcpy(&pHandlerKernel->HandlerInfo, handler, sizeof(struct HANDLER_INFO_EX));

	// 3. Init variable
	pHandlerKernel->Status = handler_status_init;

	memset(&pHandlerKernel->AutoReportContex, 0, sizeof(handler_context_t));
	pHandlerKernel->AutoReportContex.intervalMS = 1000;
	//pthread_mutex_init(&pHandlerKernel->AutoReportContex.mux, NULL);

	memset(&pHandlerKernel->LiveReportContex, 0, sizeof(handler_context_t));
	pHandlerKernel->LiveReportContex.replyID = hk_auto_upload_rep;
	pHandlerKernel->LiveReportContex.intervalMS = 1000;
	pHandlerKernel->LiveReportContex.timeoutMS = 0;
	//pthread_mutex_init(&pHandlerKernel->LiveReportContex.mux, NULL);

	memset(&pHandlerKernel->ThresholdChkContex, 0, sizeof(handler_context_t));
	pHandlerKernel->ThresholdChkContex.intervalMS = 1000;
	//pthread_mutex_init(&pHandlerKernel->ThresholdChkContex.mux, NULL);

	/*memset(&pHandlerKernel->InternelReportContex, 0, sizeof(handler_context_t));
	pHandlerKernel->InternelReportContex.intervalMS = 1000;*/

	pthread_mutex_init(&pHandlerKernel->CapabilityMux, NULL);

	//pthread_mutex_init(&pHandlerKernel->InternelReportContex.mux, NULL);

	if(pHandlerKernel->pTriggerQueue)
		pHandlerKernel->pTriggerQueue = triggerqueue_init(10);
	else
		KernelHExLog(pHandlerKernel->pHandlerkernellog , Error, " %s> Trigger Queue is Exist!",  pHandlerKernel->HandlerInfo.Name);

	pHandlerKernel->isThreadRunning = true;
	if (pthread_create(&pHandlerKernel->threadHandler, NULL, HandlerKernelEx_MonitorThread, pHandlerKernel) != 0)
	{
		KernelHExLog(pHandlerKernel->pHandlerkernellog, Warning, " %s> start monitor thread failed!", pHandlerKernel->HandlerInfo.Name);
		pHandlerKernel->isThreadRunning = false;
		pHandlerKernel->threadHandler = NULL;
	}

	return pHandlerKernel;
}

/* **************************************************************************************
 *  Function Name: Handler_Uninitialize
 *  Description: Uninit any objects or variables of this handler
 *  Input : None
 *  Output: None
 *  Return:  0  : Success
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_Uninitialize(void* pHandle)
{
	handler_kernel_t* pHandlerKernel = NULL;
	if( pHandle == NULL)
		return handler_fail;
	pHandlerKernel = (handler_kernel_t*)pHandle;

	if(pHandlerKernel->pTriggerQueue)
	{
		triggerqueue_uninit(pHandlerKernel->pTriggerQueue);
		pHandlerKernel->pTriggerQueue = NULL;
	}

	if(pHandlerKernel->threadHandler)
	{
		pHandlerKernel->isThreadRunning = false;
		pthread_cancel(pHandlerKernel->threadHandler);
		pthread_join(pHandlerKernel->threadHandler, NULL);
		pHandlerKernel->threadHandler = NULL;
	}

	pHandlerKernel->Status = handler_status_no_init;
	
	pthread_mutex_lock(&pHandlerKernel->CapabilityMux);
	/*pthread_mutex_lock(&pHandlerKernel->ThresholdChkContex.mux);
		pthread_mutex_lock(&pHandlerKernel->AutoReportContex.mux);
			pthread_mutex_lock(&pHandlerKernel->LiveReportContex.mux);*/
				pHandlerKernel->pCapability = NULL;
			/*pthread_mutex_unlock(&pHandlerKernel->LiveReportContex.mux);
		pthread_mutex_unlock(&pHandlerKernel->AutoReportContex.mux);
		HandlerThreshold_DestroyThrList(pHandlerKernel->pThresholdList);
	pthread_mutex_unlock(&pHandlerKernel->ThresholdChkContex.mux);*/
	pthread_mutex_unlock(&pHandlerKernel->CapabilityMux);

	pthread_mutex_destroy(&pHandlerKernel->CapabilityMux);

	//pthread_mutex_destroy(&pHandlerKernel->InternelReportContex.mux);
	/*if(pHandlerKernel->InternelReportContex.reqItems)
		free(pHandlerKernel->InternelReportContex.reqItems);
	pHandlerKernel->InternelReportContex.reqItems = NULL;*/

	//pthread_mutex_lock(&pHandlerKernel->ThresholdChkContex.mux);
	HandlerThreshold_DestroyThrList(pHandlerKernel->pThresholdList);
	//pthread_mutex_unlock(&pHandlerKernel->ThresholdChkContex.mux);

	//pthread_mutex_destroy(&pHandlerKernel->AutoReportContex.mux);
	if(pHandlerKernel->AutoReportContex.reqItems)
		free(pHandlerKernel->AutoReportContex.reqItems);
	pHandlerKernel->AutoReportContex.reqItems = NULL;

	//pthread_mutex_destroy(&pHandlerKernel->LiveReportContex.mux);
	if(pHandlerKernel->LiveReportContex.reqItems)
		free(pHandlerKernel->LiveReportContex.reqItems);
	pHandlerKernel->LiveReportContex.reqItems = NULL;

	//pthread_mutex_destroy(&pHandlerKernel->ThresholdChkContex.mux);
	if(pHandlerKernel->ThresholdChkContex.reqItems)
		free(pHandlerKernel->ThresholdChkContex.reqItems);
	pHandlerKernel->ThresholdChkContex.reqItems = NULL;

	pHandlerKernel->pCapability = NULL;
	pHandlerKernel->pHandlerkernellog = NULL;
	free(pHandlerKernel);

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_SetCapability
 *  Description: Assign the Capability structure and set the bPublish to publish the new capability
 *  Input : MSG_CLASSIFY_T *pCapability
 *			bool bPublish
 *  Output: None
 *  Return:  0 : Success
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_SetCapability(void* pHandle, MSG_CLASSIFY_T* pCapability, bool bPublish )
{
	char *strCapability = NULL;
	handler_kernel_t* pHandlerKernel = NULL;

	if( pCapability == NULL || pHandle == NULL)
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	/*if(pHandlerKernel->InternelReportContex.enable)
	{
		pHandlerKernel->InternelReportContex.enable = false;
	}*/

	pthread_mutex_lock(&pHandlerKernel->CapabilityMux);
	//pthread_mutex_lock(&pHandlerKernel->ThresholdChkContex.mux);
	//	pthread_mutex_lock(&pHandlerKernel->AutoReportContex.mux);
	//		pthread_mutex_lock(&pHandlerKernel->LiveReportContex.mux);
				pHandlerKernel->pCapability = pCapability;
				//IoT_SetDataChangeCallback(pCapability, HandlerKernelEx_OnAttributeChanged, pHandle);
				strCapability = IoT_PrintCapability(pHandlerKernel->pCapability);
	//		pthread_mutex_unlock(&pHandlerKernel->LiveReportContex.mux);
	//	pthread_mutex_unlock(&pHandlerKernel->AutoReportContex.mux);
	//pthread_mutex_unlock(&pHandlerKernel->ThresholdChkContex.mux);
	pthread_mutex_unlock(&pHandlerKernel->CapabilityMux);

	/*if(pHandlerKernel->HandlerInfo.internelreportcbf)
	{
		pHandlerKernel->InternelReportContex.reqAll = true;
		pHandlerKernel->InternelReportContex.enable = true;
	}*/

	if(strCapability)
	{
		if(bPublish && pHandlerKernel->HandlerInfo.sendcapabilitycbf)
			pHandlerKernel->HandlerInfo.sendcapabilitycbf(&pHandlerKernel->HandlerInfo, strCapability, strlen(strCapability), NULL, NULL);
		free(strCapability);
	}

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_LockCapability
 *  Description: Assign the Capability structure and set the bPublish to publish the new capability
 *  Input : MSG_CLASSIFY_T *pCapability
 *			bool bPublish
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_LockCapability(void* pHandle)
{
	handler_kernel_t* pHandlerKernel = NULL;

	if(pHandle == NULL)
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*) pHandle;

	pthread_mutex_lock(&pHandlerKernel->CapabilityMux);
	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_UnlockCapability
 *  Description: Assign the Capability structure and set the bPublish to publish the new capability
 *  Input : MSG_CLASSIFY_T *pCapability
 *			bool bPublish
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_UnlockCapability(void* pHandle )
{
	handler_kernel_t* pHandlerKernel = NULL;

	if(pHandle == NULL)
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*) pHandle;

	pthread_mutex_unlock(&pHandlerKernel->CapabilityMux);
	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_AutoReportStart
 *  Description: Start Auto Report
 *  Input : char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_AutoReportStart(void* pHandle, char *pInQuery)
{
	unsigned int interval = 0;
	char* reqItems = NULL;
	bool bReqAll = false;
	handler_kernel_t* pHandlerKernel = NULL;

	if( pInQuery == NULL || pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	/*create thread to report sensor data*/
	if(!HandlerParser_ParseAutoReportCmd(pInQuery, pHandlerKernel->HandlerInfo.Name, &interval, &reqItems, &bReqAll))
	{
		KernelHExLog(pHandlerKernel->pHandlerkernellog, Warning, " %s> Auto Report CMD Parser Failed: %s", pHandlerKernel->HandlerInfo.Name, pInQuery); 
		return handler_fail;
	}

	//pthread_mutex_lock(&pHandlerKernel->AutoReportContex.mux);
	pHandlerKernel->AutoReportContex.intervalMS = interval*1000;
	pHandlerKernel->AutoReportContex.reqAll = bReqAll;
	pHandlerKernel->AutoReportContex.enable = true;
	if(pHandlerKernel->AutoReportContex.reqItems)
		free(pHandlerKernel->AutoReportContex.reqItems);
	pHandlerKernel->AutoReportContex.reqItems = NULL;
	if(reqItems)
	{
		pHandlerKernel->AutoReportContex.reqItems = (char*)calloc(1, strlen(reqItems)+1);
		strcpy(pHandlerKernel->AutoReportContex.reqItems, reqItems);
		free(reqItems);
	}
	//pthread_mutex_unlock(&pHandlerKernel->AutoReportContex.mux);
	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_AutoReportStop
 *  Description: Stop Auto Report
 *  Input : char *pInQuery, preserved.
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_AutoReportStop(void* pHandle, char *pInQuery)
{
	bool bStopThread = true;
	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;
	//if( pInQuery == NULL )
	//	bStopThread = true;

	if(bStopThread)
	{
		pHandlerKernel->AutoReportContex.enable = false;		
	}

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_SetAutoReportFilter
 *  Description: Set Auto Report filter to filter out the unused sensor data. 
 *  Input : char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_SetAutoReportFilter(void* pHandle, char *pInQuery)
{
	unsigned int interval = 0;
	char* reqItems = NULL;
	bool bReqAll = false;
	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	if( pInQuery == NULL )
		return handler_fail;

	/*create thread to report sensor data*/
	if(!HandlerParser_ParseAutoReportCmd(pInQuery, pHandlerKernel->HandlerInfo.Name, &interval, &reqItems, &bReqAll))
	{
		KernelHExLog(pHandlerKernel->pHandlerkernellog, Warning, " %s> Auto Report CMD Parser Failed: %s", pHandlerKernel->HandlerInfo.Name, pInQuery); 
		return handler_fail;
	}

	//pthread_mutex_lock(&pHandlerKernel->AutoReportContex.mux);
	pHandlerKernel->AutoReportContex.reqAll = bReqAll;
	
	if(pHandlerKernel->AutoReportContex.reqItems)
		free(pHandlerKernel->AutoReportContex.reqItems);
	pHandlerKernel->AutoReportContex.reqItems = NULL;
	if(reqItems)
	{
		pHandlerKernel->AutoReportContex.reqItems = (char*)calloc(1, strlen(reqItems)+1);
		strcpy(pHandlerKernel->AutoReportContex.reqItems, reqItems);
		free(reqItems);
	}
	//pthread_mutex_unlock(&pHandlerKernel->AutoReportContex.mux);

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_SendAutoReportOnce
 *  Description: Send single auto report data. User can used with event trigger.
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_SendAutoReportOnce(void* pHandle)
{
	int iRet = cagent_send_data_error;
	char* message = NULL;
	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	//pthread_mutex_unlock(&pHandlerKernel->AutoReportContex.mux);
	if(pHandlerKernel->pCapability)
	{
		if(pHandlerKernel->AutoReportContex.reqAll)
			message = IoT_PrintData(pHandlerKernel->pCapability);
		else
			message = IoT_PrintSelectedData(pHandlerKernel->pCapability,pHandlerKernel->AutoReportContex.reqItems);
	}
	//pthread_mutex_unlock(&pHandlerKernel->AutoReportContex.mux);

	if(message)
	{
		if(pHandlerKernel->HandlerInfo.sendreportcbf)
			pHandlerKernel->HandlerInfo.sendreportcbf(/*Handler Info*/&pHandlerKernel->HandlerInfo, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);
		free(message);
	}
	if(iRet == cagent_success)
		return handler_success;
	else
		return handler_fail;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_LiveReportStart
 *  Description: Start Live Report (auto upload)
 *  Input : int replyID
 *			char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_LiveReportStart(void* pHandle, int replyID, char *pInQuery)
{
	unsigned int interval = 0;
	unsigned int timeout = 0;
	char* reqItems = NULL;
	bool bReqAll = false;
	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	if( pInQuery == NULL )
		return handler_fail;

	/*create thread to report sensor data*/
	if(!HandlerParser_ParseAutoUploadCmd(pInQuery, pHandlerKernel->HandlerInfo.Name, &interval, &timeout, &reqItems, &bReqAll))
	{
		KernelHExLog(pHandlerKernel->pHandlerkernellog, Warning, " %s> Live Report CMD Parser Failed: %s", pHandlerKernel->HandlerInfo.Name, pInQuery); 
		return handler_fail;
	}

	pHandlerKernel->LiveReportContex.replyID = replyID;
	pHandlerKernel->LiveReportContex.intervalMS = interval;
	pHandlerKernel->LiveReportContex.timeoutMS = timeout;
	pHandlerKernel->LiveReportContex.reqAll = bReqAll;
	pHandlerKernel->LiveReportContex.enable = true;

	if(pHandlerKernel->LiveReportContex.reqItems)
		free(pHandlerKernel->LiveReportContex.reqItems);
		pHandlerKernel->LiveReportContex.reqItems = NULL;
	if(reqItems)
	{
		pHandlerKernel->LiveReportContex.reqItems = (char*)calloc(1, strlen(reqItems)+1);
		strcpy(pHandlerKernel->LiveReportContex.reqItems, reqItems);
		free(reqItems);
	}

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_ParseRecvCMDWithSessionID
 *  Description: Parse received command to command ID
 *  Input : char *pInQuery
 *  Output: int * cmdID
 *			char * sessionID
 *  Return:  0 : Success 
 *          -1 : Fail 
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_ParseRecvCMDWithSessionID(char *pInQuery, int * cmdID, char * sessionID)
{
	if( pInQuery == NULL || cmdID == NULL)
		return handler_fail;

	if(HandlerParser_ParseReceivedCMD(pInQuery, strlen(pInQuery), cmdID, sessionID))
		return handler_success;
	else
		return handler_fail;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_SetThreshold
 *  Description: Set threshold rule
 *  Input : int replyID
 *			char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_SetThreshold(void* pHandle, int replyID, char *pInQuery)
{
	int iRet = handler_fail;
	char * repJsonStr = NULL;
	char repMsg[256] = {0};
	thr_item_list tmpThrItemList = NULL;
	char* buffer = NULL;	
	char* message = NULL;
	HANDLER_NOTIFY_SEVERITY severity = Severity_Debug;
	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	if( pInQuery == NULL)
		return handler_fail;

	tmpThrItemList = HandlerThreshold_CreateThrList();
	tmpThrItemList->thrItemInfo.pTriggerQueue = pHandlerKernel->pTriggerQueue;
	if(!HandlerParser_ParseThrInfo(pInQuery, tmpThrItemList, triggerqueue_push))
	{
		sprintf(repMsg, "%s", "Threshold apply failed!");
		iRet = handler_fail;
		goto SET_THRESHOLD_EXIT;
	}

	//pthread_mutex_lock(&pHandlerKernel->ThresholdChkContex.mux);
	if(pHandlerKernel->pThresholdList == NULL)
	{
		pHandlerKernel->pThresholdList = HandlerThreshold_CreateThrList();
		pHandlerKernel->pThresholdList->thrItemInfo.pTriggerQueue = pHandlerKernel->pTriggerQueue;
	}

	buffer = calloc(1,1024);	
	if(HandlerThreshold_UpdateThrInfoList(pHandlerKernel->pThresholdList, tmpThrItemList, &buffer, 1024))
	{
		if(strlen(buffer) > 0)
		{
			bool bNormal = false;
			HandlerThreshold_IsThrItemListNormal(pHandlerKernel->pThresholdList, &bNormal);
			message = calloc(1, strlen(buffer) + 128);
			sprintf(message,"{\"subtype\":\"THRESHOLD_CHECK_INFO\",\"thrCheckStatus\":\"%s\",\"msg\":\"%s\"}", bNormal?"True":"False", buffer); /*for custom handler*/
			//sprintf(message,"{\"thrCheckStatus\":\"True\",\"thrCheckMsg\":\"%s\"}", buffer); /*original for standard handler*/
		}
	}
	free(buffer);
	//pthread_mutex_unlock(&pHandlerKernel->ThresholdChkContex.mux);

	sprintf(repMsg, "%s", "Threshold rule apply OK!");
	iRet = handler_success;

SET_THRESHOLD_EXIT:
	HandlerThreshold_DestroyThrList(tmpThrItemList);
	if(HandlerParser_PackSetThrRep(repMsg, &repJsonStr))
	{
		if(pHandlerKernel->HandlerInfo.sendcbf)
			pHandlerKernel->HandlerInfo.sendcbf(&pHandlerKernel->HandlerInfo, replyID, repJsonStr, strlen(repJsonStr), NULL, NULL);
	}
	if(repJsonStr)free(repJsonStr);	

	if(message)
	{
		if(pHandlerKernel->HandlerInfo.sendeventcbf && strlen(message) > 0)
			pHandlerKernel->HandlerInfo.sendeventcbf(/*Handler Info*/&pHandlerKernel->HandlerInfo, Severity_Informational, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);
		free(message);
	}

	return iRet;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_DeleteAllThreshold
 *  Description: Delete threshold rule
 *  Input : int replyID
 *			char *pInQuery
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_DeleteAllThreshold(void* pHandle, int replyID)
{
	char * repJsonStr = NULL;
	char repMsg[256] = {0};
	char* buffer = NULL;	
	char* message = NULL;
	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	//pthread_mutex_lock(&pHandlerKernel->ThresholdChkContex.mux);
	buffer = calloc(1,1024);
	HandlerThreshold_WhenDelThrCheckNormal(pHandlerKernel->pThresholdList, &buffer, 1024);
	if(strlen(buffer) > 0)
	{
		int len = strlen(buffer) + 70;
		message = calloc(1, len);
		sprintf(message,"{\"subtype\":\"THRESHOLD_CHECK_INFO\",\"thrCheckStatus\":\"True\",\"msg\":\"%s\"}", buffer); /*for custom handler*/
		//sprintf(message,"{\"thrCheckStatus\":\"True\",\"thrCheckMsg\":\"%s\"}", buffer); /*original for standard handler*/
	}
	free(buffer);
	HandlerThreshold_DestroyThrList(pHandlerKernel->pThresholdList);
	pHandlerKernel->pThresholdList = NULL;
	//pthread_mutex_unlock(&pHandlerKernel->ThresholdChkContex.mux);

	sprintf(repMsg, "%s", "Delete all threshold successfully!");

	if(HandlerParser_PackDelAllThrRep(repMsg, &repJsonStr))
	{
		if(pHandlerKernel->HandlerInfo.sendcbf)
			pHandlerKernel->HandlerInfo.sendcbf(&pHandlerKernel->HandlerInfo, replyID, repJsonStr, strlen(repJsonStr), NULL, NULL);
	}
	if(repJsonStr) free(repJsonStr);	

	if(message)
	{
		if(pHandlerKernel->HandlerInfo.sendeventcbf && strlen(message) > 0)
			pHandlerKernel->HandlerInfo.sendeventcbf(/*Handler Info*/&pHandlerKernel->HandlerInfo, Severity_Informational, /*message data*/message, /*message length*/strlen(message), /*preserved*/NULL, /*preserved*/NULL);
		free(message);
	}

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_StartThresholdCheck
 *  Description: Start threshold rule check thread
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_StartThresholdCheck(void* pHandle)
{
	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	if(pHandlerKernel->pThresholdList == NULL)
		return handler_fail;

	pHandlerKernel->ThresholdChkContex.enable = true;

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_StopThresholdCheck
 *  Description: Stop threshold rule check thread
 *  Input : None
 *  Output: None
 *  Return:  0 : Success 
 *          -1 : Fail
 * ***************************************************************************************/
int HANDLERKERNEL_API HandlerKernelEx_StopThresholdCheck(void* pHandle)
{
	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	if(pHandlerKernel->pThresholdList == NULL)
		return handler_fail;

	pHandlerKernel->ThresholdChkContex.enable = false;

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_SetThresholdTrigger
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
int HANDLERKERNEL_API HandlerKernelEx_SetThresholdTrigger(void* pHandle, void (*on_triggered)(bool isnormal,
													char* sensorname, double value, MSG_ATTRIBUTE_T* attr,
													void *pRev))
{
	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	triggerqueue_setcb(pHandlerKernel->pTriggerQueue, on_triggered);
	
	return handler_success;
}


/* **************************************************************************************
 *  Function Name: HandlerKernelEx_GetSensorData
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
int HANDLERKERNEL_API HandlerKernelEx_GetSensorData(void* pHandle, int replyID, char* sessionID, char *pInQuery, bool (*on_get_sensor)(get_data_t* getlist, void *pRev))
{
	pthread_t threadctx=0;
	sensor_cmd_t* getcmd = NULL;

	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	if( pInQuery == NULL )
		return handler_fail;

	getcmd = (sensor_cmd_t*)malloc(sizeof(sensor_cmd_t));
	strcpy(getcmd->sessionID, sessionID);
	getcmd->replyID = replyID;
	getcmd->on_sensor_cmd = on_get_sensor;
	getcmd->pHandler = pHandlerKernel;
	if(!HandlerParser_ParseSensorDataCmd(pInQuery, &getcmd->reqItems))
	{
		KernelHExLog(pHandlerKernel->pHandlerkernellog, Warning, " %s> Get Sensor CMD Parser Failed: %s", pHandlerKernel->HandlerInfo.Name, pInQuery); 
		free(getcmd);
		return handler_fail;
	}
	
	if (pthread_create(&threadctx, NULL, HandlerKernelEx_ThreadGetSensor, getcmd) != 0)
	{
		free(getcmd);
		KernelHExLog(pHandlerKernel->pHandlerkernellog, Warning, " %s> start get sensor data thread failed!", pHandlerKernel->HandlerInfo.Name);	
	}
	else
	{
		pthread_detach(threadctx);
		threadctx = NULL;
	}

	return handler_success;
}

/* **************************************************************************************
 *  Function Name: HandlerKernelEx_SetSensorData
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
int HANDLERKERNEL_API HandlerKernelEx_SetSensorData(void* pHandle, int replyID, char* sessionID, char *pInQuery, bool (*on_set_sensor)(set_data_t* setlist, void *pRev))
{
	pthread_t threadctx=0;
	sensor_cmd_t* setcmd = NULL;

	handler_kernel_t* pHandlerKernel = NULL;

	if( pHandle == NULL )
		return handler_fail;

	pHandlerKernel = (handler_kernel_t*)pHandle;

	if( pInQuery == NULL )
		return handler_fail;

	setcmd = (sensor_cmd_t*)malloc(sizeof(sensor_cmd_t));
	strcpy(setcmd->sessionID, sessionID);
	setcmd->replyID = replyID;
	setcmd->on_sensor_cmd = on_set_sensor;
	setcmd->pHandler = pHandlerKernel;
	if(!HandlerParser_ParseSensorDataCmd(pInQuery, &setcmd->reqItems))
	{
		KernelHExLog(pHandlerKernel->pHandlerkernellog, Warning, " %s> Get Sensor CMD Parser Failed: %s", pHandlerKernel->HandlerInfo.Name, pInQuery); 
		free(setcmd);
		return handler_fail;
	}

	if (pthread_create(&threadctx, NULL, HandlerKernelEx_ThreadSetSensor, setcmd) != 0)
	{
		free(setcmd);
		KernelHExLog(pHandlerKernel->pHandlerkernellog, Warning, " %s> start get sensor data thread failed!", pHandlerKernel->HandlerInfo.Name);	
	}
	else
	{
		pthread_detach(threadctx);
		threadctx = NULL;
	}
	return handler_success;
}