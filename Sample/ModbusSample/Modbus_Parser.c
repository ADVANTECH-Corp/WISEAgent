/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.														 */
/* Create Date  : 2015 by Zach Chih															     */
/* Modified Date: 2015 by Zach Chih															 */
/* Abstract     : Modbus Handler                                   													*/
/* Reference    : None																									 */
/****************************************************************************/
#include "Modbus_Parser.h"

static int cJSON_GetSensorInfoListEx(cJSON * pFatherItem, sensor_info_list sensorInfoList);
static int cJSON_GetSensorInfoEx(cJSON * pSensorInfoItem, sensor_info_t * pSensorInfo);

bool ParseReceivedData(void* data, int datalen, int * cmdID)
{
	/*{"susiCommData":{"commCmd":251,"catalogID":4,"requestID":10}}*/

	cJSON* root = NULL;
	cJSON* body = NULL;
	cJSON* target = NULL;

	if(!data) return false;
	if(datalen<=0) return false;
	root = cJSON_Parse((char *)data);
	if(!root) return false;

	body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
	if(!body)
	{
		cJSON_Delete(root);
		return false;
	}

	target = cJSON_GetObjectItem(body, AGENTINFO_CMDTYPE);
	if(target)
	{
		*cmdID = target->valueint;
	}
	cJSON_Delete(root);
	return true;
}

int Parser_PackModbusError(char * errorStr, char** outputStr)
{
	char * out = NULL;
	int outLen = 0;
	cJSON *pSUSICommDataItem = NULL;
	if(errorStr == NULL || outputStr == NULL) return outLen;
	pSUSICommDataItem = cJSON_CreateObject();

	cJSON_AddStringToObject(pSUSICommDataItem, MODBUS_ERROR_REP, errorStr);

	out = cJSON_PrintUnformatted(pSUSICommDataItem);
	outLen = strlen(out) + 1;
	*outputStr = (char *)(malloc(outLen));
	memset(*outputStr, 0, outLen);
	strcpy(*outputStr, out);
	cJSON_Delete(pSUSICommDataItem);	
	printf("%s\n",out);	
	free(out);
	return outLen;
}








bool Parser_ParseGetSensorDataReqEx(void * data, sensor_info_list siList, char * pSessionID)
{
	bool bRet = false;
	cJSON* root = NULL;
	cJSON* body = NULL;
	cJSON* params = NULL;

	if(!data || !siList || pSessionID==NULL) return bRet;

	root = cJSON_Parse((char *)data);
	if(root)
	{
		body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
		if(body)
		{
			params = cJSON_GetObjectItem(body, MODBUS_SENSOR_ID_LIST);
			if(params)
			{
				cJSON * eItem = NULL;
				eItem = cJSON_GetObjectItem(params, MODBUS_E_FLAG);
				if(eItem)
				{
					cJSON * subItem = NULL;
					cJSON * valItem = NULL;
					sensor_info_node_t * head = siList;
					sensor_info_node_t * newNode = NULL;
					int i = 0;
					int nCount = cJSON_GetArraySize(eItem);
					for(i = 0; i<nCount; i++)
					{
						subItem = cJSON_GetArrayItem(eItem, i);
						if(subItem)
						{
							valItem = cJSON_GetObjectItem(subItem, MODBUS_N_FLAG);
							if(valItem)
							{
								int len = strlen(valItem->valuestring)+1;
								newNode = (sensor_info_node_t *)malloc(sizeof(sensor_info_node_t));
								memset(newNode, 0, sizeof(sensor_info_node_t));
								newNode->sensorInfo.pathStr = (char *)malloc(len);
								memset(newNode->sensorInfo.pathStr, 0, len);
								strcpy(newNode->sensorInfo.pathStr, valItem->valuestring);
								newNode->next = head->next;
								head->next = newNode;
							}
						}
					}
					params = cJSON_GetObjectItem(body, MODBUS_SESSION_ID);
					if(params)
					{
						strcpy(pSessionID, params->valuestring);
						bRet = true;
					}
				}
			}
		}
		cJSON_Delete(root);
	}
	return bRet;
}



bool Parser_ParseSetSensorDataReqEx(void* data, sensor_info_list sensorInfoList, char * sessionID)
{
	bool bRet = false;
	cJSON* root = NULL;
	cJSON* body = NULL;
	cJSON* params = NULL;
	if(!data || sensorInfoList ==NULL || sessionID == NULL) return bRet;

	root = cJSON_Parse((char *)data);
	if(root)
	{
		body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
		if(body)
		{
			int iRet = 0;
			iRet = cJSON_GetSensorInfoListEx(body, sensorInfoList);
			if(iRet == 1)
			{
				params = cJSON_GetObjectItem(body, MODBUS_SESSION_ID);
				if(params)
				{
					strcpy(sessionID, params->valuestring);
					bRet = true;
				}
			}
		}
		cJSON_Delete(root);
	}
	return bRet;
}

bool Parser_ParseAutoReportCmd(char * cmdJsonStr, unsigned int * intervalTimeS, char * repFilter)
{
	/*{"susiCommData":{"catalogID":4,"autoUploadIntervalSec":30,"requestID":1001,"requestItems":["HWM"],"commCmd":2053,"type":"WSN"}}*/
   bool bRet = false;
	cJSON* root = NULL;
	cJSON* body = NULL;
	cJSON* target = NULL;

	if(cmdJsonStr == NULL || NULL == intervalTimeS || repFilter == NULL) return false;

	root = cJSON_Parse(cmdJsonStr);
	if(root)
	{
		body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
		if(body)
		{
			target = cJSON_GetObjectItem(body, MODBUS_AUTOREP_INTERVAL_SEC);
			if(target)
			{
				*intervalTimeS = target->valueint;
				target = cJSON_GetObjectItem(body, MODBUS_AUTOREP_REQ_ITEMS);
				if(target)
				{
					char * tmpJsonStr = NULL;
					int len = 0;
					tmpJsonStr = cJSON_PrintUnformatted(target);
					len = strlen(tmpJsonStr) + 1;
					strcpy(repFilter, tmpJsonStr);	
					free(tmpJsonStr);
					bRet = true;
				}
			}
		}
		cJSON_Delete(root);
	}
	return bRet;
}

bool Parser_ParseAutoUploadCmd(char * cmdJsonStr, unsigned int * intervalTimeMs, unsigned int * continueTimeMs, char * repFilter)
{
	bool bRet = false;
	cJSON* root = NULL;
	cJSON* body = NULL;
	cJSON* target = NULL;

	if(cmdJsonStr == NULL || NULL == intervalTimeMs || continueTimeMs == NULL || repFilter == NULL) return false;

	root = cJSON_Parse(cmdJsonStr);
	if(root)
	{
		body = cJSON_GetObjectItem(root, AGENTINFO_BODY_STRUCT);
		if(body)
		{
			target = cJSON_GetObjectItem(body, MODBUS_AUTOUPLOAD_INTERVAL_MS);
			if(target)
			{
				*intervalTimeMs = target->valueint;
				target = cJSON_GetObjectItem(body, MODBUS_AUTOUPLOAD_CONTINUE_MS);
				if(target)
				{
					*continueTimeMs = target->valueint;
					target = cJSON_GetObjectItem(body, MODBUS_AUTOREP_REQ_ITEMS);
					if(target)
					{
						char * tmpJsonStr = NULL;
						int len = 0;
						tmpJsonStr = cJSON_PrintUnformatted(target);
						len = strlen(tmpJsonStr) + 1;
						strcpy(repFilter, tmpJsonStr);	
						free(tmpJsonStr);
						bRet = true;
					}
				}
			}
		}
		cJSON_Delete(root);
	}
	return bRet;
}







//-----------------------------------------------------------------cJSON
static int cJSON_GetSensorInfoEx(cJSON * pSensorInfoItem, sensor_info_t * pSensorInfo)
{
	int iRet = 0;
	if(!pSensorInfoItem|| !pSensorInfo) return iRet;
	{
		cJSON * pSubItem = NULL;
		pSubItem = cJSON_GetObjectItem(pSensorInfoItem, MODBUS_N_FLAG);
		if(pSubItem)
		{
			int len = strlen(pSubItem->valuestring) +1;
			pSensorInfo->pathStr = (char *)malloc(len);
			memset(pSensorInfo->pathStr, 0, len);
			strcpy(pSensorInfo->pathStr, pSubItem->valuestring);
			iRet = 1;
		}
	}
	return iRet;
}
static int cJSON_GetSensorInfoListEx(cJSON * pFatherItem, sensor_info_list sensorInfoList)
{
	int iRet = 0, iFlag = 0;
	if(pFatherItem == NULL || sensorInfoList == NULL) return iRet;
	{
		cJSON * sensorIDListItem = NULL;
		sensorIDListItem = cJSON_GetObjectItem(pFatherItem, MODBUS_SENSOR_ID_LIST);
		if(sensorIDListItem)
		{
			cJSON * eItem = NULL;
			eItem = cJSON_GetObjectItem(sensorIDListItem, MODBUS_E_FLAG);
			if(eItem)
			{
				int i = 0;
				int nCount = cJSON_GetArraySize(eItem);
				for(i = 0; i < nCount; i++)
				{
					cJSON *pParamItem = cJSON_GetArrayItem(eItem, i);
					if(pParamItem)
					{
						char * jsonStr = NULL;
						int iRet = 0;
						sensor_info_node_t * pSensorInfoNode = NULL;
						pSensorInfoNode = (sensor_info_node_t *)malloc(sizeof(sensor_info_node_t));
						memset(pSensorInfoNode, 0, sizeof(sensor_info_node_t));
						jsonStr = cJSON_PrintUnformatted(pParamItem);
						if(jsonStr)
						{
							int len = strlen(jsonStr) +1;
							pSensorInfoNode->sensorInfo.jsonStr = (char *)malloc(len);
							memset(pSensorInfoNode->sensorInfo.jsonStr, 0, len);
							strcpy(pSensorInfoNode->sensorInfo.jsonStr, jsonStr);
							free(jsonStr);
							iRet = cJSON_GetSensorInfoEx(pParamItem, &(pSensorInfoNode->sensorInfo));
							if(!iRet)
							{
								if(pSensorInfoNode)
								{
									free(pSensorInfoNode);
									pSensorInfoNode = NULL;
								}
								continue;
							}
						}
						pSensorInfoNode->next = sensorInfoList->next;
						sensorInfoList->next = pSensorInfoNode;
					}
				}
				iRet = 1;
			}
		}
	}
	return iRet;
}
//---------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------Modbus_General_Node_Parser-------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------
//Parse Sensor Node in JSON
bool Modbus_General_Node_Parser(sensor_info_node_t *curNode,WISE_Sensor *sensors,int count)
{
	char *delim = "/";
	char *str=NULL;
	char *temp;
	int i=0;
    
	temp=(char *)calloc(strlen(curNode->sensorInfo.pathStr)+1,sizeof(char));
	strcpy(temp,curNode->sensorInfo.pathStr);
	str = strtok(temp,delim);
	while (str != NULL)
	{	
		if(i==0)
			strcpy(sensors[count].handler,str);
		else if(i==1)
			strcpy(sensors[count].type,str);
		else	
			strcpy(sensors[count].name,str);
		str = strtok (NULL, delim);
		i++;
	}
	
	free(temp);
	return true;
}

//Parse Sensor Path
bool Modbus_General_Paths_Parser(char *paths,WISE_Sensor *sensors,int count)
{
	char *delim = "/";
	char *str=NULL;
	char *temp;
	int i=0;

	temp=(char *)calloc(strlen(paths)+1,sizeof(char));
	strcpy(temp,paths);
	str = strtok(temp,delim);
	while (str != NULL)
	{	
		if(i==0)
			strcpy(sensors[count].handler,str);
		else if(i==1)
			strcpy(sensors[count].type,str);
		else
			strcpy(sensors[count].name,str);
		str = strtok (NULL, delim);
		i++;
	}

	free(temp);

	return true;
}

//Check Format of "Set" Command
bool Modbus_Parser_Set_FormatCheck(sensor_info_node_t *curNode,bool *bVal,double *dVal,char *sVal)
{

	int vCount=0;
	int nCount=0;
	cJSON *root = cJSON_Parse(curNode->sensorInfo.jsonStr);


	if(!root)
	{	printf("get root failed!\n");
		return false;
	}
	else
	{
				cJSON *name = cJSON_GetObjectItem(root,"n");
				cJSON *bv = cJSON_GetObjectItem(root,"bv");
				cJSON *sv = cJSON_GetObjectItem(root,"sv");
				cJSON *v = cJSON_GetObjectItem(root,"v");
				if(!name)
				{	printf("no name!\n");
					cJSON_Delete(root);
					return false;
				}
				else
				{
					if(name->type==cJSON_String)
					{
						nCount++;
					}
					else
					{
						printf("name->type fail\n");
						cJSON_Delete(root);
						return false;
					}

				}	
				
				if(!bv)
				{	printf("no bv!\n");
				}
				else
				{
					if(bv->type==cJSON_True)
					{
						printf("bv->type : true\n");						
						*bVal=true;
					}
					else if(bv->type==cJSON_False)
					{	

						printf("bv->type : false\n");
						*bVal=false;
					}
					else if(bv->type!=cJSON_True&&bv->type!=cJSON_False)
					{
						printf("bv->type error!!\n");
						cJSON_Delete(root);
						return false;
					}
					vCount++;
				}


				
				if(!v)
				{	printf("no v!\n");

				}
				else
				{
					if(v->type==cJSON_Number)
					{

						printf("v->type : %lf\n",v->valuedouble);
						*dVal=v->valuedouble;	
					}
					else
					{
						printf("v->type error!!\n");
						cJSON_Delete(root);
						return false;
					}
					vCount++;
				}



				if(!sv)
				{	printf("no sv!\n");

				}
				else
				{
					if(sv->type==cJSON_String)
					{	

						printf("sv->type : %s\n",sv->valuestring);
						strcpy(sVal,sv->string);
					}
					else
					{
						printf("sv->type error\n");
						cJSON_Delete(root);
						return false;
					}
					vCount++;
				}
				
				if(nCount!=1||vCount!=1)
				{
					cJSON_Delete(root);
					return false;
				}
				
				cJSON_Delete(root);
	}

	return true;
}