#include "MsgGenerator.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "AdvPlatform.h"
#include <math.h>
#include <time.h>
#include <sys/time.h>

long long MSG_GetTimeTick()
{
	long long tick = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tick = (long long)tv.tv_sec*1000 + (long long)tv.tv_usec/1000;
	return tick;
}

#pragma region Add_Resource
MSG_CLASSIFY_T* MSG_CreateRoot()
{
	MSG_CLASSIFY_T *pMsg = malloc(sizeof(MSG_CLASSIFY_T));
	if(pMsg)
	{
		memset(pMsg, 0, sizeof(MSG_CLASSIFY_T));
		pMsg->type = class_type_root;
	}
	return pMsg;
}

MSG_CLASSIFY_T* MSG_CreateRootEx(AttributeChangedCbf onchanged, void* pRev1)
{
	MSG_CLASSIFY_T *pMsg = malloc(sizeof(MSG_CLASSIFY_T));
	if(pMsg)
	{
		memset(pMsg, 0, sizeof(MSG_CLASSIFY_T));
		pMsg->type = class_type_root;
		pMsg->on_datachanged = onchanged;
		pMsg->pRev1 = pRev1;
	}
	return pMsg;
}

MSG_CLASSIFY_T* MSG_AddClassify(MSG_CLASSIFY_T *pNode, char const* name, char const* version, bool bArray, bool isIoT)
{
	MSG_CLASSIFY_T *pCurNode = NULL;
	if(!pNode || !name)
		return pNode;

	pCurNode = malloc(sizeof(MSG_CLASSIFY_T));
	if(pCurNode)
	{
		memset(pCurNode, 0, sizeof(MSG_CLASSIFY_T));
		pCurNode->type = bArray ? class_type_array : class_type_object;
		if(name)
		{
			strncpy(pCurNode->classname, name, strlen(name));
		}

		if(version)
		{
			strncpy(pCurNode->version, version, strlen(version));
		}

		pCurNode->bIoTFormat = isIoT;

		if(!pNode->sub_list)
		{
			pNode->sub_list = pCurNode;
		}
		else
		{
			MSG_CLASSIFY_T *pLastNode = pNode->sub_list;
			while(pLastNode->next)
			{
				pLastNode = pLastNode->next;
			}
			pLastNode->next = pCurNode;

			pCurNode->on_datachanged = pNode->on_datachanged;
			pCurNode->pRev1 = pNode->pRev1;
		}
	}
	return pCurNode;
}

MSG_ATTRIBUTE_T* MSG_AddAttribute(MSG_CLASSIFY_T* pClass, char const* attrname, bool isSensorData)
{
	MSG_ATTRIBUTE_T* curAttr = NULL;
	MSG_ATTRIBUTE_T* lastAttr = NULL;
	if(!pClass)
		return curAttr;
	curAttr = malloc(sizeof(MSG_ATTRIBUTE_T));
	if(curAttr)
	{
		memset(curAttr, 0, sizeof(MSG_ATTRIBUTE_T));

		if(attrname)
		{
			strncpy(curAttr->name, attrname, strlen(attrname));
		}
		curAttr->bSensor = isSensorData;

		if(pClass->attr_list == NULL)
		{	
			
			pClass->attr_list = curAttr;
		
		}
		else
		{
			lastAttr = pClass->attr_list;
			while(lastAttr->next)
			{	
				lastAttr = lastAttr->next;
			}
			lastAttr->next = curAttr;
			curAttr->on_datachanged = pClass->on_datachanged;
			curAttr->pRev1 = pClass->pRev1;
		}
	}
	return curAttr;
}
#pragma endregion Add_Resource

#pragma region Release_Resource
void ReleaseAttribute(MSG_ATTRIBUTE_T* attr)
{
	if(!attr)
		return;

	if((attr->type == attr_type_date || attr->type == attr_type_string) && !attr->bNull)
	{
		if(attr->sv)
		{
			free(attr->sv);
		}
		attr->sv = NULL;
		attr->on_datachanged = NULL;
		attr->pRev1 = NULL;
	}

	{
		EXT_ATTRIBUTE_T* extattr = attr->extra;
		while(extattr)
		{
			EXT_ATTRIBUTE_T* extnext = extattr->next;
			if(extattr->type == attr_type_date || extattr->type == attr_type_string)
			{
				if(extattr->sv)
				{
					free(extattr->sv);
				}
				attr->sv = NULL;
			}
			free(extattr);
			extattr = extnext;
		}
	}
	
	free(attr);
}

void ReleaseClassify(MSG_CLASSIFY_T* classify)
{
	MSG_ATTRIBUTE_T* curAttr = NULL;
	MSG_ATTRIBUTE_T* nxtAttr = NULL;

	MSG_CLASSIFY_T* curSubtype = NULL;
	MSG_CLASSIFY_T* nxtSubtype = NULL;

	if(!classify)
		return;
	curAttr = classify->attr_list;
	while (curAttr)
	{
		nxtAttr = curAttr->next;
		ReleaseAttribute(curAttr);
		curAttr = nxtAttr;
	}

	curSubtype = classify->sub_list;
	while (curSubtype)
	{
		nxtSubtype = curSubtype->next;
		ReleaseClassify(curSubtype);
		curSubtype = nxtSubtype;
	}

	classify->on_datachanged = NULL;
	classify->pRev1 = NULL;
	free(classify);
}

bool MSG_DelAttribute(MSG_CLASSIFY_T* pNode, char* name, bool isSensorData)
{
	MSG_ATTRIBUTE_T* curAttr = NULL;
	MSG_ATTRIBUTE_T* nxtAttr = NULL;
	if(!pNode || !name)
		return false;
	curAttr = pNode->attr_list;
	if(curAttr->bSensor == isSensorData)
	{
		if(!strcmp(curAttr->name, name))
		{
			pNode->attr_list = curAttr->next;
			ReleaseAttribute(curAttr);
			return true;
		}
	}

	nxtAttr = curAttr->next;
	while (nxtAttr)
	{
		if(nxtAttr->bSensor == isSensorData)
		{
			if(!strcmp(nxtAttr->name, name))
			{
				curAttr->next = nxtAttr->next;
				ReleaseAttribute(nxtAttr);
				return true;
			}
		}
		curAttr = nxtAttr;
		nxtAttr = curAttr->next;
	}
	return false;
}

bool MSG_DelClassify(MSG_CLASSIFY_T* pNode, char* name)
{
	MSG_CLASSIFY_T* curClass = NULL;
	MSG_CLASSIFY_T* nxtClass = NULL;
	if(!pNode || !name)
		return false;
	curClass = pNode->sub_list;
	if(!strcmp(curClass->classname, name))		
	{
		pNode->sub_list = curClass->next;
		ReleaseClassify(curClass);
		return true;
	}

	nxtClass = curClass->next;
	while (nxtClass)
	{
		if(!strcmp(nxtClass->classname, name))
		{
			curClass->next = nxtClass->next;
			ReleaseClassify(nxtClass);
			return true;
		}
		curClass = nxtClass;
		nxtClass = curClass->next;
	}
	return false;
}

void MSG_ReleaseRoot(MSG_CLASSIFY_T* classify)
{
	ReleaseClassify(classify);
}
#pragma endregion Release_Resource

#pragma region Find_Resource
MSG_CLASSIFY_T* MSG_FindClassifyWithoutPrefix(MSG_CLASSIFY_T* pNode, char const* name)
{
	char* prefix = NULL;
	char* index = 0;
	MSG_CLASSIFY_T* curClass = NULL;
	MSG_CLASSIFY_T* nxtClass = NULL;
	if(!pNode || !name)
		return curClass;
	curClass = pNode->sub_list;
	while (curClass)
	{
		nxtClass = curClass->next;

		index = strstr(curClass->classname, "-");
		if(index == 0)
		{
			curClass = nxtClass;
			continue;
		}

		prefix = calloc(1, strlen(index));
		strncpy(prefix, index+1, strlen(index)-1);


		if(!strcmp(prefix, name))
		{
			free(prefix);
			return curClass;
		}
		free(prefix);
		curClass = nxtClass;
	}
	return NULL;
}

MSG_CLASSIFY_T* MSG_FindClassifyWithPrefix(MSG_CLASSIFY_T* pNode, char const* name)
{
	char* prefix = NULL;
	char* index = 0;
	MSG_CLASSIFY_T* curClass = NULL;
	MSG_CLASSIFY_T* nxtClass = NULL;
	if(!pNode || !name)
		return curClass;
	curClass = pNode->sub_list;
	while (curClass)
	{
		nxtClass = curClass->next;

		index = strstr(curClass->classname, "-");
		if(index == 0)
		{
			curClass = nxtClass;
			continue;
		}

		prefix = calloc(1, (index - curClass->classname + 1));
		strncpy(prefix, curClass->classname, (index - curClass->classname));


		if(!strcmp(prefix, name))
		{
			free(prefix);
			return curClass;
		}
		free(prefix);
		curClass = nxtClass;
	}
	return MSG_FindClassifyWithoutPrefix(pNode, name);
}

MSG_CLASSIFY_T* MSG_FindClassify(MSG_CLASSIFY_T* pNode, char const* name)
{
	MSG_CLASSIFY_T* curClass = NULL;
	MSG_CLASSIFY_T* nxtClass = NULL;
	if(!pNode || !name)
		return curClass;
	curClass = pNode->sub_list;
	while (curClass)
	{
		nxtClass = curClass->next;
		if(!strcmp(curClass->classname, name))
			return curClass;
		curClass = nxtClass;
	}
	return MSG_FindClassifyWithPrefix(pNode, name);
}

MSG_ATTRIBUTE_T* MSG_FindAttribute(MSG_CLASSIFY_T* root, char const* senname, bool isSensorData)
{
	MSG_ATTRIBUTE_T* curAttr = NULL;
	MSG_ATTRIBUTE_T* nxtAttr = NULL;
	if(!root || !senname)
		return curAttr;
	curAttr = root->attr_list;
	while (curAttr)
	{
		nxtAttr = curAttr->next;
		if(curAttr->bSensor == isSensorData)
		{
			if(!strcmp(curAttr->name, senname))
				return curAttr;
		}
		curAttr = nxtAttr;
	}
	return NULL;
}

bool MSG_SetFloatValue(MSG_ATTRIBUTE_T* attr, float value, char* readwritemode, char *unit)
{
	return MSG_SetDoubleValue(attr, value, readwritemode, unit);
}

bool MSG_SetFloatValueWithMaxMin(MSG_ATTRIBUTE_T* attr, float value, char* readwritemode, float max, float min, char *unit)
{
	return MSG_SetDoubleValueWithMaxMin(attr, value, readwritemode, max, min, unit);
}

bool MSG_SetDoubleValue(MSG_ATTRIBUTE_T* attr, double value, char* readwritemode, char *unit)
{
	bool bNotify = false;
	if(!attr)
		return false;
	if(attr->type != attr_type_numeric)
		bNotify = true;
	else if(attr->v != value)
		bNotify = true;

	if((attr->type == attr_type_date || attr->type == attr_type_string) && !attr->bNull)
	{
		if(attr->sv)
			free(attr->sv);
		attr->sv = NULL;
	}
	attr->v = value;
	attr->type = attr_type_numeric;
	attr->bRange = false;
	attr->bNull = false;
	if(readwritemode)
		strncpy(attr->readwritemode, readwritemode, strlen(readwritemode));
	if(unit)
		strncpy(attr->unit, unit, strlen(unit));

	if(bNotify)
		if(attr->on_datachanged)
			attr->on_datachanged(attr, attr->pRev1);
	return true;
}

bool MSG_SetDoubleValueWithMaxMin(MSG_ATTRIBUTE_T* attr, double value, char* readwritemode, double max, double min, char *unit)
{
	bool bNotify = false;
	if(!attr)
		return false;
	if(attr->type != attr_type_numeric)
		bNotify = true;
	else if(attr->v != value)
		bNotify = true;

	if((attr->type == attr_type_date || attr->type == attr_type_string) && !attr->bNull)
	{
		if(attr->sv)
			free(attr->sv);
		attr->sv = NULL;
	}
	attr->v = value;
	attr->type = attr_type_numeric;
	attr->bRange = true;
	attr->bNull = false;
	attr->max = max;
	attr->min = min;
	if(readwritemode)
		strncpy(attr->readwritemode, readwritemode, strlen(readwritemode));
	if(unit)
		strncpy(attr->unit, unit, strlen(unit));

	if(bNotify)
		if(attr->on_datachanged)
			attr->on_datachanged(attr, attr->pRev1);
	return true;
}

bool MSG_SetBoolValue(MSG_ATTRIBUTE_T* attr, bool bvalue, char* readwritemode)
{
	bool bNotify = false;
	if(!attr)
		return false;
	if(attr->type != attr_type_boolean)
		bNotify = true;
	else if(attr->bv != bvalue)
		bNotify = true;

	if((attr->type == attr_type_date || attr->type == attr_type_string) && !attr->bNull)
	{
		if(attr->sv)
			free(attr->sv);
		attr->sv = NULL;
	}
	attr->bv = bvalue;
	attr->type = attr_type_boolean;
	if(readwritemode)
		strncpy(attr->readwritemode, readwritemode, strlen(readwritemode));
	attr->bRange = false;
	attr->bNull = false;

	if(bNotify)
		if(attr->on_datachanged)
			attr->on_datachanged(attr, attr->pRev1);
	return true;
}

bool MSG_SetStringValue(MSG_ATTRIBUTE_T* attr, char *svalue, char* readwritemode)
{
	int length = 0;
	bool bNotify = false;
	if(!attr)
		return false;
	if(attr->type != attr_type_string)
		bNotify = true;
	else if(strcmp(attr->sv,svalue)!=0)
		bNotify = true;
	
	if((attr->type == attr_type_date || attr->type == attr_type_string) && !attr->bNull)
	{
		if(attr->sv)
			free(attr->sv);
		attr->sv = NULL;
	}

	if(svalue)
	{
		length = strlen(svalue);
		attr->sv = calloc(1, length+1);
		strncpy(attr->sv, svalue, strlen(svalue));
		attr->bNull = false;
	}
	else
		attr->bNull = true;

	attr->type = attr_type_string;
	if(readwritemode)
		strncpy(attr->readwritemode, readwritemode, strlen(readwritemode));
	attr->bRange = false;

	if(bNotify)
		if(attr->on_datachanged)
			attr->on_datachanged(attr, attr->pRev1);
	return true;
}

bool MSG_SetTimestampValue(MSG_ATTRIBUTE_T* attr, unsigned int value, char* readwritemode)
{
	bool bNotify = false;
	if(!attr)
		return false;
	if(attr->type != attr_type_timestamp)
		bNotify = true;
	else if(attr->v != value)
		bNotify = true;

	if((attr->type == attr_type_date || attr->type == attr_type_string) && !attr->bNull)
	{
		if(attr->sv)
			free(attr->sv);
		attr->sv = NULL;
	}
	attr->v = value;
	attr->type = attr_type_timestamp;
	if(readwritemode)
		strncpy(attr->readwritemode, readwritemode, strlen(readwritemode));
	attr->bRange = false;
	attr->bNull = false;

	if(bNotify)
		if(attr->on_datachanged)
			attr->on_datachanged(attr, attr->pRev1);
	return true;
}

bool MSG_SetDateValue(MSG_ATTRIBUTE_T* attr, char *svalue, char* readwritemode)
{
	int length = 0;
	bool bNotify = false;
	if(!attr)
		return false;
	if(attr->type != attr_type_date)
		bNotify = true;
	else if(strcmp(attr->sv,svalue)!=0)
		bNotify = true;

	if((attr->type == attr_type_date || attr->type == attr_type_string) && !attr->bNull)
	{
		if(attr->sv)
			free(attr->sv);
		attr->sv = NULL;
	}

	if(svalue)
	{
		length = strlen(svalue);
		attr->sv = calloc(1, length+1);
		strncpy(attr->sv, svalue, strlen(svalue));
		attr->bNull = false;
	}
	else
		attr->bNull = true;

	attr->type = attr_type_date;
	if(readwritemode)
		strncpy(attr->readwritemode, readwritemode, strlen(readwritemode));
	attr->bRange = false;

	if(bNotify)
		if(attr->on_datachanged)
			attr->on_datachanged(attr, attr->pRev1);
	return true;
}

bool MSG_SetNULLValue(MSG_ATTRIBUTE_T* attr, char* readwritemode)
{
	int length = 0;
	bool bNotify = false;
	if(!attr)
		return false;
	if(attr->type != attr_type_string)
		bNotify = true;
	else if(!attr->bNull)
		bNotify = true;

	if((attr->type == attr_type_date || attr->type == attr_type_string) && !attr->bNull)
	{
		if(attr->sv)
			free(attr->sv);
		attr->sv = NULL;
	}
	attr->bNull = true;

	attr->type = attr_type_string;
	if(readwritemode)
		strncpy(attr->readwritemode, readwritemode, strlen(readwritemode));
	attr->bRange = false;
	
	if(bNotify)
		if(attr->on_datachanged)
			attr->on_datachanged(attr, attr->pRev1);
	return true;
}
#pragma endregion Find_Resource

#pragma region Generate_JSON 
bool MatchFilterString(char* target, char** filter, int filterlength)
{
	int i=0;
	if(!filter)
		return true;

	if(!target)
		return false;

	for(i=0; i<filterlength; i++)
	{
		if(!strcasecmp(target, filter[i]))
			return true;
	}
	return false;
}

bool AddJSONAttribute(cJSON *pClass, MSG_ATTRIBUTE_T *attr_list, char** filter, int length)
{
	cJSON* pAttr = NULL;
	cJSON* pENode = NULL;
	MSG_ATTRIBUTE_T* curAttr = NULL;
	MSG_ATTRIBUTE_T* nxtAttr = NULL;

	if(!pClass || !attr_list)
		return false;

	curAttr = attr_list;
	while (curAttr)
	{
		nxtAttr = curAttr->next;

		if(curAttr->bSensor)
		{	
			if(!pENode)
			{
				pENode = cJSON_CreateArray();
				cJSON_AddItemToObject(pClass, TAG_E_NODE, pENode);
			}
			pAttr = cJSON_CreateObject();
			cJSON_AddItemToArray(pENode, pAttr);
			if(MatchFilterString(TAG_ATTR_NAME, filter, length))
				cJSON_AddStringToObject(pAttr, TAG_ATTR_NAME, curAttr->name);
			switch (curAttr->type)
			{
			case attr_type_numeric:
				{
					if(MatchFilterString(TAG_VALUE, filter, length))
					{
						if(curAttr->bNull)
							cJSON_AddNullToObject(pAttr, TAG_VALUE);
						else
							cJSON_AddNumberToObject(pAttr, TAG_VALUE, curAttr->v);
					}
					if(curAttr->bRange)
					{
						if(MatchFilterString(TAG_MAX, filter, length))
							cJSON_AddNumberToObject(pAttr, TAG_MAX, curAttr->max);
						if(MatchFilterString(TAG_MIN, filter, length))
							cJSON_AddNumberToObject(pAttr, TAG_MIN, curAttr->min);
					}
					if(strlen(curAttr->readwritemode)>0)
						if(MatchFilterString(TAG_ASM, filter, length))
							cJSON_AddStringToObject(pAttr, TAG_ASM, curAttr->readwritemode);
	

					if(strlen(curAttr->unit)>0)
					{
						if(MatchFilterString(TAG_UNIT, filter, length))
							cJSON_AddStringToObject(pAttr, TAG_UNIT, curAttr->unit);
					}
				}
				break;
			case attr_type_boolean:
				{
					if(MatchFilterString(TAG_BOOLEAN, filter, length))
					{
						if(curAttr->bNull)
							cJSON_AddNullToObject(pAttr, TAG_BOOLEAN);
						else if(curAttr->bv)
							cJSON_AddTrueToObject(pAttr, TAG_BOOLEAN);
						else
							cJSON_AddFalseToObject(pAttr, TAG_BOOLEAN);
					}
					if(strlen(curAttr->readwritemode)>0)
						if(MatchFilterString(TAG_ASM, filter, length))
							cJSON_AddStringToObject(pAttr, TAG_ASM, curAttr->readwritemode);
				}
				break;
			case attr_type_string:
				{
					if(curAttr->bNull || strlen(curAttr->sv)==0)
					{
						if(MatchFilterString(TAG_VALUE, filter, length))
							cJSON_AddNullToObject(pAttr, TAG_STRING);
					}
					else if(strlen(curAttr->sv)>0)
					{
						if(MatchFilterString(TAG_STRING, filter, length))
						{
							cJSON_AddStringToObject(pAttr, TAG_STRING, curAttr->sv);
						}
					}
					if(strlen(curAttr->readwritemode)>0)
						if(MatchFilterString(TAG_ASM, filter, length))
							cJSON_AddStringToObject(pAttr, TAG_ASM, curAttr->readwritemode);
				}
				break;
			case  attr_type_date:
				{
					if(curAttr->bNull || strlen(curAttr->sv)==0)
					{
						if(MatchFilterString(TAG_VALUE, filter, length))
							cJSON_AddNullToObject(pAttr, TAG_VALUE);
					}
					else if(strlen(curAttr->sv)>0)
					{
						if(MatchFilterString(TAG_VALUE, filter, length))
						{
							cJSON* pDateRoot = cJSON_CreateObject();
							cJSON_AddItemToObject(pAttr, TAG_VALUE, pDateRoot);
							cJSON_AddStringToObject(pDateRoot, TAG_DATE, curAttr->sv);
						}
					}
					if(strlen(curAttr->readwritemode)>0)
						if(MatchFilterString(TAG_ASM, filter, length))
							cJSON_AddStringToObject(pAttr, TAG_ASM, curAttr->readwritemode);
				}
				break;
			case  attr_type_timestamp:
				{
					if(MatchFilterString(TAG_VALUE, filter, length))
					{
						if(curAttr->bNull)
							cJSON_AddNullToObject(pAttr, TAG_VALUE);
						else 
						{
							cJSON* pDateRoot = cJSON_CreateObject();
							cJSON_AddItemToObject(pAttr, TAG_VALUE, pDateRoot);
							cJSON_AddNumberToObject(pDateRoot, TAG_TIMESTAMP, curAttr->v);
						}
					}
					if(strlen(curAttr->readwritemode)>0)
						if(MatchFilterString(TAG_ASM, filter, length))
							cJSON_AddStringToObject(pAttr, TAG_ASM, curAttr->readwritemode);
				}
				break;
			default:
				{
				}
				break;
			}

			{
				EXT_ATTRIBUTE_T* extattr = curAttr->extra;
				while(extattr)
				{
					switch (extattr->type)
					{
					case attr_type_numeric:
						cJSON_AddNumberToObject(pAttr, extattr->name, extattr->v);
						break;
					case attr_type_boolean:
						if(extattr->bv)
							cJSON_AddTrueToObject(pAttr, extattr->name);
						else
							cJSON_AddFalseToObject(pAttr, extattr->name);
						break;
					case attr_type_string:
						cJSON_AddStringToObject(pAttr, extattr->name, extattr->sv);
						break;
					case  attr_type_date:
						{
							cJSON* pDateRoot = cJSON_CreateObject();
							cJSON_AddItemToObject(pAttr, extattr->name, pDateRoot);
							cJSON_AddStringToObject(pDateRoot, TAG_DATE, extattr->sv);
						}
						break;
					case  attr_type_timestamp:
						{
							cJSON* pDateRoot = cJSON_CreateObject();
							cJSON_AddItemToObject(pAttr, extattr->name, pDateRoot);
							cJSON_AddNumberToObject(pDateRoot, TAG_TIMESTAMP, extattr->v);
						}
						break;
					default:
						{
						}
						break;
					}
					extattr = extattr->next;
				}
						
			}
		}
		else
		{	
			switch (curAttr->type)
			{
			case attr_type_numeric:
				{
					if(MatchFilterString(curAttr->name, filter, length))
					{
						if(curAttr->bNull)
							cJSON_AddNullToObject(pClass, curAttr->name);
						else 
							cJSON_AddNumberToObject(pClass, curAttr->name, curAttr->v);
					}
				}
				break;
			case attr_type_boolean:
				{
					if(MatchFilterString(curAttr->name, filter, length))
					{
						if(curAttr->bNull)
							cJSON_AddNullToObject(pClass, curAttr->name);
						else if(curAttr->bv)
							cJSON_AddTrueToObject(pClass, curAttr->name);
						else
							cJSON_AddFalseToObject(pClass, curAttr->name);
					}
				}
				break;
			case attr_type_string:
				{
					if(curAttr->bNull || strlen(curAttr->sv)==0)
					{
						if(MatchFilterString(TAG_VALUE, filter, length))
							cJSON_AddNullToObject(pClass, curAttr->name);
					}
					else if(strlen(curAttr->sv)>0)
					{
						if(MatchFilterString(curAttr->name, filter, length))
						{
							cJSON_AddStringToObject(pClass, curAttr->name, curAttr->sv);
						}
					}
				}
				break;
			case  attr_type_date:
				{
					if(curAttr->bNull || strlen(curAttr->sv)==0)
					{
						if(MatchFilterString(curAttr->name, filter, length))
							cJSON_AddNullToObject(pClass, curAttr->name);
					}
					else if(strlen(curAttr->sv)>0)
					{
						if(MatchFilterString(curAttr->name, filter, length))
						{
							cJSON* pDateRoot = cJSON_CreateObject();
							cJSON_AddItemToObject(pClass, curAttr->name, pDateRoot);
							cJSON_AddStringToObject(pDateRoot, TAG_DATE, curAttr->sv);
						}
					}
				}
				break;
			case  attr_type_timestamp:
				{
					if(MatchFilterString(curAttr->name, filter, length))
					{

						if(curAttr->bNull)
							cJSON_AddNullToObject(pClass, curAttr->name);
						else 
						{
							cJSON* pDateRoot = cJSON_CreateObject();
							cJSON_AddItemToObject(pClass, curAttr->name, pDateRoot);
							cJSON_AddNumberToObject(pDateRoot, TAG_TIMESTAMP, curAttr->v);
						}
					}
				}
				break;
			default:
				{
				}
				break;
			}
		}

		curAttr = nxtAttr;
	};

	return true;
}

bool AddJSONClassify(cJSON *pRoot, MSG_CLASSIFY_T* msg, char** filter, int length)
{
	cJSON* pClass = NULL;

	MSG_CLASSIFY_T* curClass = NULL;
	MSG_CLASSIFY_T* nxtClass = NULL;

	if(!pRoot || !msg)
		return false;

	if(msg->type == class_type_root)
		pClass = pRoot;
	else
	{
		if(msg->type == class_type_array)
			pClass = cJSON_CreateArray();
		else
		{
			pClass = cJSON_CreateObject();
			if(msg->bIoTFormat)
			{
				if(MatchFilterString(TAG_BASE_NAME, filter, length))
					cJSON_AddStringToObject(pClass, TAG_BASE_NAME, msg->classname);

				if(MatchFilterString(TAG_VERSION, filter, length))
					if(strlen(msg->version)>0)
						cJSON_AddStringToObject(pClass, TAG_VERSION, msg->version);
			}
		}

		if(pRoot->type == cJSON_Array)
			cJSON_AddItemToArray(pRoot, pClass);
		else
			cJSON_AddItemToObject(pRoot, msg->classname, pClass);
	}

	//if(msg->type != class_type_array)
	//{
		if(msg->attr_list)
		{
			AddJSONAttribute(pClass, msg->attr_list, filter, length);
		}
	//}

	if(msg->sub_list)
	{
		curClass = msg->sub_list;
		while (curClass)
		{
			nxtClass = curClass->next;

			AddJSONClassify(pClass, curClass, filter, length);

			curClass = nxtClass;
		};
	}
/*	
	if(msg->next)
	{
		curClass = msg->next;
		while (curClass)
		{
			nxtClass = curClass->next;

			AddJSONClassify(pClass, curClass);

			curClass = nxtClass;
		};
	}
*/	
	return true;
}

bool AddSingleJSONAttribute(cJSON *pClass, MSG_ATTRIBUTE_T *attr, char** filter, int length)
{
	cJSON* pAttr = NULL;
	cJSON* pENode = NULL;

	if(!pClass || !attr)
		return false;

	if(attr->bSensor)
	{	
		pENode = cJSON_GetObjectItem(pClass, TAG_E_NODE);
		if(!pENode)
		{
			pENode = cJSON_CreateArray();
			cJSON_AddItemToObject(pClass, TAG_E_NODE, pENode);
		}


		{
			int size = cJSON_GetArraySize(pENode);
			int i=0;
			for(i=0; i<size;i++)
			{
				cJSON* pNode = cJSON_GetArrayItem(pENode, i);
				if(pNode)
				{
					pNode = cJSON_GetObjectItem(pNode, TAG_ATTR_NAME);
					if(pNode)
					{
						if(strcmp(pNode->valuestring, attr->name) == 0)
							return true;
					}

				}
			}
		}

		pAttr = cJSON_CreateObject();
		cJSON_AddItemToArray(pENode, pAttr);
		if(MatchFilterString(TAG_ATTR_NAME, filter, length))
			cJSON_AddStringToObject(pAttr, TAG_ATTR_NAME, attr->name);
		switch (attr->type)
		{
		case attr_type_numeric:
			{
				if(MatchFilterString(TAG_VALUE, filter, length))
				{
					if(attr->bNull)
						cJSON_AddNullToObject(pAttr, TAG_VALUE);
					else
						cJSON_AddNumberToObject(pAttr, TAG_VALUE, attr->v);
				}
				if(attr->bRange)
				{
					if(MatchFilterString(TAG_MAX, filter, length))
						cJSON_AddNumberToObject(pAttr, TAG_MAX, attr->max);
					if(MatchFilterString(TAG_MIN, filter, length))
						cJSON_AddNumberToObject(pAttr, TAG_MIN, attr->min);
				}
				if(strlen(attr->readwritemode)>0)
					if(MatchFilterString(TAG_ASM, filter, length))
						cJSON_AddStringToObject(pAttr, TAG_ASM, attr->readwritemode);

				if(strlen(attr->unit)>0)
				{
					if(MatchFilterString(TAG_UNIT, filter, length))
						cJSON_AddStringToObject(pAttr, TAG_UNIT, attr->unit);
				}
			}
			break;
		case attr_type_boolean:
			{
				if(MatchFilterString(TAG_BOOLEAN, filter, length))
				{
					if(attr->bNull)
						cJSON_AddNullToObject(pAttr, TAG_BOOLEAN);
					else if(attr->bv)
						cJSON_AddTrueToObject(pAttr, TAG_BOOLEAN);
					else
						cJSON_AddFalseToObject(pAttr, TAG_BOOLEAN);
				}
				if(strlen(attr->readwritemode)>0)
					if(MatchFilterString(TAG_ASM, filter, length))
						cJSON_AddStringToObject(pAttr, TAG_ASM, attr->readwritemode);
			}
			break;
		case attr_type_string:
			{
				if(attr->bNull)
				{
					if(MatchFilterString(TAG_VALUE, filter, length))
						cJSON_AddNullToObject(pAttr, TAG_STRING);
				}
				else if(strlen(attr->sv)>0)
				{
					if(MatchFilterString(TAG_STRING, filter, length))
					{
						cJSON_AddStringToObject(pAttr, TAG_STRING, attr->sv);
					}
				}
				if(strlen(attr->readwritemode)>0)
					if(MatchFilterString(TAG_ASM, filter, length))
						cJSON_AddStringToObject(pAttr, TAG_ASM, attr->readwritemode);
			}
			break;
		case  attr_type_date:
			{
				if(attr->bNull)
				{
					if(MatchFilterString(TAG_VALUE, filter, length))
						cJSON_AddNullToObject(pAttr, TAG_VALUE);
				}
				else if(strlen(attr->sv)>0)
				{
					if(MatchFilterString(TAG_VALUE, filter, length))
					{
						cJSON* pDateRoot = cJSON_CreateObject();
						cJSON_AddItemToObject(pAttr, TAG_VALUE, pDateRoot);
						cJSON_AddStringToObject(pDateRoot, TAG_DATE, attr->sv);
					}
				}
				if(strlen(attr->readwritemode)>0)
					if(MatchFilterString(TAG_ASM, filter, length))
						cJSON_AddStringToObject(pAttr, TAG_ASM, attr->readwritemode);
			}
			break;
		case  attr_type_timestamp:
			{
				if(MatchFilterString(TAG_VALUE, filter, length))
				{
					if(attr->bNull)
						cJSON_AddNullToObject(pAttr, TAG_VALUE);
					else 
					{
						cJSON* pDateRoot = cJSON_CreateObject();
						cJSON_AddItemToObject(pAttr, TAG_VALUE, pDateRoot);
						cJSON_AddNumberToObject(pDateRoot, TAG_TIMESTAMP, attr->v);
					}
				}
				if(strlen(attr->readwritemode)>0)
					if(MatchFilterString(TAG_ASM, filter, length))
						cJSON_AddStringToObject(pAttr, TAG_ASM, attr->readwritemode);
			}
			break;
		default:
			{
			}
			break;
		}

		{
			EXT_ATTRIBUTE_T* extattr = attr->extra;
			while(extattr)
			{
				if(MatchFilterString(extattr->name, filter, length))
				{
					switch (attr->type)
					{
					case attr_type_numeric:
						cJSON_AddNumberToObject(pAttr, extattr->name, extattr->v);
						break;
					case attr_type_boolean:
						if(extattr->bv)
							cJSON_AddTrueToObject(pAttr, extattr->name);
						else
							cJSON_AddFalseToObject(pAttr, extattr->name);
						break;
					case attr_type_string:
						cJSON_AddStringToObject(pAttr, extattr->name, attr->sv);
						break;
					case  attr_type_date:
						{
							cJSON* pDateRoot = cJSON_CreateObject();
							cJSON_AddItemToObject(pAttr, extattr->name, pDateRoot);
							cJSON_AddStringToObject(pDateRoot, TAG_DATE, attr->sv);
						}
						break;
					case  attr_type_timestamp:
						{
							cJSON* pDateRoot = cJSON_CreateObject();
							cJSON_AddItemToObject(pAttr, extattr->name, pDateRoot);
							cJSON_AddNumberToObject(pDateRoot, TAG_TIMESTAMP, attr->v);
						}
						break;
					default:
						{
						}
						break;
					}
				}
				extattr = extattr->next;
			}
		}
	}
	else
	{	
		switch (attr->type)
		{
		case attr_type_numeric:
			{
				if(MatchFilterString(attr->name, filter, length))
				{
					if(attr->bNull)
						cJSON_AddNullToObject(pClass, attr->name);
					else 
						cJSON_AddNumberToObject(pClass, attr->name, attr->v);
				}
			}
			break;
		case attr_type_boolean:
			{
				if(MatchFilterString(attr->name, filter, length))
				{
					if(attr->bNull)
						cJSON_AddNullToObject(pClass, attr->name);
					else if(attr->bv)
						cJSON_AddTrueToObject(pClass, attr->name);
					else
						cJSON_AddFalseToObject(pClass, attr->name);
				}
			}
			break;
		case attr_type_string:
			{
				if(attr->bNull)
				{
					if(MatchFilterString(TAG_VALUE, filter, length))
						cJSON_AddNullToObject(pClass, attr->name);
				}
				else if(strlen(attr->sv)>0)
				{
					if(MatchFilterString(attr->name, filter, length))
					{
						cJSON_AddStringToObject(pClass, attr->name, attr->sv);
					}
				}
			}
			break;
		case  attr_type_date:
			{
				if(attr->bNull)
				{
					if(MatchFilterString(attr->name, filter, length))
						cJSON_AddNullToObject(pClass, attr->name);
				}
				else if(strlen(attr->sv)>0)
				{
					if(MatchFilterString(attr->name, filter, length))
					{
						cJSON* pDateRoot = cJSON_CreateObject();
						cJSON_AddItemToObject(pClass, attr->name, pDateRoot);
						cJSON_AddStringToObject(pDateRoot, TAG_DATE, attr->sv);
					}
				}
			}
			break;
		case  attr_type_timestamp:
			{
				if(MatchFilterString(attr->name, filter, length))
				{

					if(attr->bNull)
						cJSON_AddNullToObject(pClass, attr->name);
					else 
					{
						cJSON* pDateRoot = cJSON_CreateObject();
						cJSON_AddItemToObject(pClass, attr->name, pDateRoot);
						cJSON_AddNumberToObject(pDateRoot, TAG_TIMESTAMP, attr->v);
					}
				}
			}
			break;
		default:
			{
			}
			break;
		}
	}
	return true;
}

cJSON * AddSingleJSONClassify(cJSON *pParent, MSG_CLASSIFY_T* msg, char** filter, int length)
{
	cJSON* pClass = NULL;

	MSG_CLASSIFY_T* curClass = NULL;
	MSG_CLASSIFY_T* nxtClass = NULL;

	if(!pParent || !msg)
		return pClass;

	if(msg->type == class_type_root)
		pClass = pParent;
	else
	{
		if(msg->type == class_type_array)
			pClass = cJSON_CreateArray();
		else
		{
			pClass = cJSON_CreateObject();
			if(msg->bIoTFormat)
			{
				if(MatchFilterString(TAG_BASE_NAME, filter, length))
					cJSON_AddStringToObject(pClass, TAG_BASE_NAME, msg->classname);

				if(MatchFilterString(TAG_VERSION, filter, length))
					if(strlen(msg->version)>0)
						cJSON_AddStringToObject(pClass, TAG_VERSION, msg->version);
			}
		}

		if(pParent->type == cJSON_Array)
			cJSON_AddItemToArray(pParent, pClass);
		else
			cJSON_AddItemToObject(pParent, msg->classname, pClass);
	}
	return pClass;
}

bool AddJSONClassifyWithSelected(cJSON *pRoot, MSG_CLASSIFY_T* msg, char** filter, int length, char* reqItem)
{
	cJSON* pClass = pRoot;
	char *delim = "/";
	char *str=NULL;
	char temp[260] = {0};
	char name[30] = {0};
	bool bFirst=true;
	MSG_CLASSIFY_T  *classify = msg;
	MSG_ATTRIBUTE_T* attr = NULL;

	if(!pRoot || !msg || !reqItem)
		return false;

	strcpy(temp,reqItem);
	str = strtok(temp,delim);

	while (str != NULL)
	{	
		if(classify == NULL)
			break;

		strcpy(name, str);
		str = strtok (NULL, delim);
		if(str == NULL)
		{
			attr = MSG_FindAttribute(classify, name, true);
			if(attr == NULL)
				attr = MSG_FindAttribute(classify, name, false);
			if(attr == NULL)
			{
				cJSON* pChild = NULL;
				classify = MSG_FindClassify(classify,name);
				if(classify == NULL)
					return false;
				pChild = cJSON_GetObjectItem(pClass, name);
				if(pChild == NULL)
				{
					AddJSONClassify(pClass, classify, filter, length);
				}
			}
			else
			{
				cJSON* pChild = cJSON_GetObjectItem(pClass, name);
				if(pChild == NULL)
				{
					AddSingleJSONAttribute(pClass, attr, filter, length);
				}
			}
		}
		else
		{
			cJSON* pChild = NULL;
			classify = MSG_FindClassify(classify,name);
			if(classify == NULL)
				return false;
			pChild = cJSON_GetObjectItem(pClass, name);
			if(pChild == NULL)
			{
				pClass = AddSingleJSONClassify(pClass, classify, filter, length);
			}
			else
				pClass = pChild;
		}
	}
	return true;
}

char *MSG_PrintUnformatted(MSG_CLASSIFY_T* msg)
{
	char* buffer = NULL;
	cJSON *pRoot = NULL;

	pRoot = cJSON_CreateObject();

	AddJSONClassify(pRoot, msg, NULL, 0);
	
	buffer = cJSON_PrintUnformatted(pRoot);
	cJSON_Delete(pRoot);
	pRoot = NULL;
	return buffer;
}

char *MSG_PrintWithFiltered(MSG_CLASSIFY_T* msg, char** filter, int length)
{
	char* buffer = NULL;
	cJSON *pRoot = NULL;

	pRoot = cJSON_CreateObject();

	AddJSONClassify(pRoot, msg, filter, length);
	
	buffer = cJSON_PrintUnformatted(pRoot);
	cJSON_Delete(pRoot);
	pRoot = NULL;
	return buffer;
}

char *MSG_PrintSelectedWithFiltered(MSG_CLASSIFY_T* msg, char** filter, int length, char* reqItems)
{
	char* buffer = NULL;
	cJSON *pRoot = NULL;
	cJSON *pReqItemList = NULL;
	cJSON *pReqItemRoot = NULL;

	if(reqItems == NULL)
		return buffer;

	pReqItemRoot = cJSON_Parse(reqItems);
	if(pReqItemRoot==NULL)
		return buffer;



	pRoot = cJSON_CreateObject();


	pReqItemList = cJSON_GetObjectItem(pReqItemRoot, "e");

	if(pReqItemList)
	{
		int size = cJSON_GetArraySize(pReqItemList);
		int i=0;
		for(i=0;i<size;i++)
		{
			cJSON* nNode = NULL;
			cJSON* item = cJSON_GetArrayItem(pReqItemList, i);
			if(item == NULL)
				continue;
			nNode = cJSON_GetObjectItem(item, "n");
			if(nNode == NULL)
				continue;
			AddJSONClassifyWithSelected(pRoot, msg, filter, length, nNode->valuestring);
		}
	}
	
	buffer = cJSON_PrintUnformatted(pRoot);
	cJSON_Delete(pRoot);
	cJSON_Delete(pReqItemRoot);
	pRoot = NULL;
	return buffer;
}

MSG_ATTRIBUTE_T* MSG_FindAttributeWithPath(MSG_CLASSIFY_T *msg,char *path, bool isSensorData)
{
	MSG_CLASSIFY_T  *classify=msg;
	MSG_ATTRIBUTE_T* attr = NULL;

	char *delim = "/";
	char *str=NULL;
	char temp[200];
	char name[30];
	bool bFirst=true;

	strcpy(temp,path);
	str = strtok(temp,delim);

	while (str != NULL)
	{	
		if(classify == NULL)
			break;

		strcpy(name, str);

		str = strtok (NULL, delim);
		if(str == NULL)
			attr = MSG_FindAttribute(classify, name, isSensorData);
		else
			classify = MSG_FindClassify(classify,name);

	}

	return attr;
}

bool MSG_IsAttributeExist(MSG_CLASSIFY_T *msg,char *path, bool isSensorData)
{
	MSG_ATTRIBUTE_T* attr = MSG_FindAttributeWithPath(msg, path, isSensorData);

	if(attr)
		return true;
	else 
		return false;
}

void MSG_SetDataChangeCallback(MSG_CLASSIFY_T* msg, AttributeChangedCbf on_datachanged, void* pRev1)
{
	MSG_ATTRIBUTE_T* curAttr = NULL;

	MSG_CLASSIFY_T* curClass = NULL;
	MSG_CLASSIFY_T* nxtSubtype = NULL;

	if(!msg)
		return;

	msg->on_datachanged = on_datachanged;
	msg->pRev1 = pRev1;

	curAttr = msg->attr_list;
	while (curAttr)
	{
		curAttr->on_datachanged = on_datachanged;
		curAttr->pRev1 = pRev1;
		curAttr = curAttr->next;
	}

	curClass = msg->sub_list;
	while (curClass)
	{
		MSG_SetDataChangeCallback(curClass, on_datachanged, pRev1);
		curClass = curClass->next;
	}

}

bool MSG_AppendIoTSensorAttributeDouble(MSG_ATTRIBUTE_T* attr, const char* attrname, double value)
{
	EXT_ATTRIBUTE_T *extattr = NULL,*extpre = NULL, *target = NULL;
	if(!attr)
		return false;
	extattr = attr->extra;

	while(extattr)
	{
		extpre = extattr;
		if(strcmp(extattr->name, attrname)==0)
		{
			target = extattr;
			break;
		}
		extattr = extattr->next;
	}

	if(target == NULL)
	{
		target = calloc(1, sizeof(EXT_ATTRIBUTE_T));
		strcpy(target->name, attrname);

		if(extpre == NULL)
			attr->extra = target;
		else
			extpre->next = target;
	}

	if(target->type == attr_type_date || target->type == attr_type_string)
	{
		if(target->sv)
			free(target->sv);
		target->sv = NULL;
	}
	target->v = value;
	target->type = attr_type_numeric;

	return true;
}

bool MSG_AppendIoTSensorAttributeBool(MSG_ATTRIBUTE_T* attr, const char* attrname, bool bvalue)
{
	EXT_ATTRIBUTE_T *extattr = NULL,*extpre = NULL, *target = NULL;
	if(!attr)
		return false;
	extattr = attr->extra;

	while(extattr)
	{
		extpre = extattr;
		if(strcmp(extattr->name, attrname)==0)
		{
			target = extattr;
			break;
		}
		extattr = extattr->next;
	}

	if(target == NULL)
	{
		target = calloc(1, sizeof(EXT_ATTRIBUTE_T));
		strcpy(target->name, attrname);

		if(extpre == NULL)
			attr->extra = target;
		else
			extpre->next = target;
	}

	if(target->type == attr_type_date || target->type == attr_type_string)
	{
		if(target->sv)
			free(target->sv);
		target->sv = NULL;
	}
	target->bv = bvalue;
	target->type = attr_type_boolean;

	return true;
}

bool MSG_AppendIoTSensorAttributeString(MSG_ATTRIBUTE_T* attr, const char* attrname, char *svalue)
{
	EXT_ATTRIBUTE_T *extattr = NULL,*extpre = NULL, *target = NULL;
	if(!attr)
		return false;
	extattr = attr->extra;

	while(extattr)
	{
		extpre = extattr;
		if(strcmp(extattr->name, attrname)==0)
		{
			target = extattr;
			break;
		}
		extattr = extattr->next;
	}

	if(target == NULL)
	{
		target = calloc(1, sizeof(EXT_ATTRIBUTE_T));
		strcpy(target->name, attrname);

		if(extpre == NULL)
			attr->extra = target;
		else
			extpre->next = target;
	}

	if(target->type == attr_type_date || target->type == attr_type_string)
	{
		if(target->sv)
			free(target->sv);
		target->sv = NULL;
	}

	if(svalue)
	{
		int length = strlen(svalue);
		target->sv = calloc(1, length+1);
		strncpy(target->sv, svalue, strlen(svalue));
		target->type = attr_type_string;
	}
	return true;
}

bool MSG_AppendIoTSensorAttributeTimestamp(MSG_ATTRIBUTE_T* attr, const char* attrname, unsigned int value)
{
	EXT_ATTRIBUTE_T *extattr = NULL,*extpre = NULL, *target = NULL;
	if(!attr)
		return false;
	extattr = attr->extra;

	while(extattr)
	{
		extpre = extattr;
		if(strcmp(extattr->name, attrname)==0)
		{
			target = extattr;
			break;
		}
		extattr = extattr->next;
	}

	if(target == NULL)
	{
		target = calloc(1, sizeof(EXT_ATTRIBUTE_T));
		strcpy(target->name, attrname);

		if(extpre == NULL)
			attr->extra = target;
		else
			extpre->next = target;
	}

	if(target->type == attr_type_date || target->type == attr_type_string)
	{
		if(target->sv)
			free(target->sv);
		target->sv = NULL;
	}
	target->v = value;
	target->type = attr_type_timestamp;
	return true;
}

bool MSG_AppendIoTSensorAttributeDate(MSG_ATTRIBUTE_T* attr, const char* attrname, char *svalue)
{
	EXT_ATTRIBUTE_T *extattr = NULL,*extpre = NULL, *target = NULL;
	if(!attr)
		return false;
	extattr = attr->extra;

	while(extattr)
	{
		extpre = extattr;
		if(strcmp(extattr->name, attrname)==0)
		{
			target = extattr;
			break;
		}
		extattr = extattr->next;
	}

	if(target == NULL)
	{
		target = calloc(1, sizeof(EXT_ATTRIBUTE_T));
		strcpy(target->name, attrname);

		if(extpre == NULL)
			attr->extra = target;
		else
			extpre->next = target;
	}

	if(target->type == attr_type_date || target->type == attr_type_string)
	{
		if(target->sv)
			free(target->sv);
		target->sv = NULL;
	}

	if(svalue)
	{
		int length = strlen(svalue);
		target->sv = calloc(1, length+1);
		strncpy(target->sv, svalue, strlen(svalue));
		target->type = attr_type_date;
	}
	return true;
}

#pragma endregion Generate_JSON 


