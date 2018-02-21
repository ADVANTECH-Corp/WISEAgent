#include "IPSOParser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "IoTMessageGenerate.h"
#include "cJSON.h"
#include "AdvPlatform.h"

bool transfer_node_check(cJSON* target, char* name, int type)
{
	if(!target) return false;
	if(!name) return false;
	if(!target->string) return false;
	if(strcasecmp(target->string, name)==0)
		if(target->type == type)
			return true;
	return false;
}

cJSON* transfer_node_find(cJSON* target, char* name, int type)
{
	cJSON* result = NULL;
	if(!target) return result;
	result = target->child;
	while(result)
	{
		if(transfer_node_check(result, name, type))
			return result;
		result = result->next;
	}

	return result;
}

void transfer_sensor_generate(cJSON* target, MSG_CLASSIFY_T *pGroup)
{
	MSG_ATTRIBUTE_T* pCurAttr = NULL;
	//cJSON* child = NULL;
	
	if(!target) return;

	if(target->type == cJSON_Object)
	{
		cJSON* baseName = transfer_node_find(target, TAG_ATTR_NAME, cJSON_String);
		if(!baseName) return;
		if(!baseName->valuestring) return;
		pCurAttr = IoT_FindSensorNode(pGroup, baseName->valuestring);
		if(!pCurAttr)
			pCurAttr = IoT_AddSensorNode(pGroup, baseName->valuestring);
	}
	else
		return;

	{
		cJSON* data = NULL;
		cJSON* readwrite = NULL;
		IoT_READWRITE_MODE readwritemode = IoT_NODEFINE;
		readwrite = transfer_node_find(target, TAG_ASM, cJSON_String);
		if(readwrite)
		{
			if(strcasecmp(readwrite->valuestring, "r")==0)
				readwritemode = IoT_READONLY;
			else if(strcasecmp(readwrite->valuestring, "w")==0)
				readwritemode = IoT_WRITEONLY;
			else if(strcasecmp(readwrite->valuestring, "rw")==0)
				readwritemode = IoT_READWRITE;
		}


		if(data = transfer_node_find(target, TAG_VALUE, cJSON_Number))
		{
			cJSON* max = transfer_node_find(target, TAG_MAX, cJSON_Number);
			cJSON* min = transfer_node_find(target, TAG_MIN, cJSON_Number);
			cJSON* unit = transfer_node_find(target, TAG_UNIT, cJSON_String);
			if(max || min)
				IoT_SetDoubleValueWithMaxMin(pCurAttr, data->valuedouble, readwritemode, max?max->valuedouble:0, min?min->valuedouble:0, unit?unit->valuestring:NULL);
			else
				IoT_SetDoubleValue(pCurAttr, data->valuedouble, readwritemode, unit?unit->valuestring:NULL);
		}
		else if(data = transfer_node_find(target, TAG_BOOLEAN, cJSON_False))
		{
			IoT_SetBoolValue(pCurAttr, false, readwritemode);
		}
		else if(data = transfer_node_find(target, TAG_BOOLEAN, cJSON_True))
		{
			IoT_SetBoolValue(pCurAttr, true, readwritemode);
		}
		else if(data = transfer_node_find(target, TAG_BOOLEAN, cJSON_Number))
		{
			if(data->valueint == 0)
				IoT_SetBoolValue(pCurAttr, false, readwritemode);
			else
				IoT_SetBoolValue(pCurAttr, true, readwritemode);
		}
		else if(data = transfer_node_find(target, TAG_STRING, cJSON_String))
		{
			IoT_SetStringValue(pCurAttr, data->valuestring, readwritemode);
		}
		else if(data = transfer_node_find(target, TAG_STRING, cJSON_NULL))
		{
			IoT_SetStringValue(pCurAttr, "", readwritemode);
		}
	}

	/* Not Support!!
	child = target->child;
	while(child)
	{
		MSG_ATTRIBUTE_T* attr = NULL;
		if(transfer_node_check(child, TAG_ATTR_NAME, cJSON_String))
			strncpy(pCurAttr->name, child->valuestring, strlen(child->valuestring));
		else if(transfer_node_check(child, TAG_VALUE, cJSON_Number))
		{
			child = child->next;
			continue;
		}
		else if(transfer_node_check(child, TAG_BOOLEAN, cJSON_Number))
		{
			child = child->next;
			continue;
		}
		else if(transfer_node_check(child, TAG_STRING, cJSON_Number))
		{
			child = child->next;
			continue;
		}
		else if(transfer_node_check(child, TAG_MAX, cJSON_Number))
		{
			child = child->next;
			continue;
		}
		else if(transfer_node_check(child, TAG_MIN, cJSON_Number))
		{
			child = child->next;
			continue;
		}
		else if(transfer_node_check(child, TAG_ASM, cJSON_Number))
		{
			child = child->next;
			continue;
		}
		else if(transfer_node_check(child, TAG_UNIT, cJSON_Number))
		{
			child = child->next;
			continue;
		}
		else
		{
			
		}
		child = child->next;
	}
	*/
}

void transfer_group_generate(cJSON* target, MSG_CLASSIFY_T *pGroup)
{
	MSG_CLASSIFY_T *pCurNode = NULL;
	cJSON *child = NULL;
	cJSON *current = NULL;
	
	if(!target) return;

	while(target) 
	{
	if(target->type == cJSON_Object)
	{
		cJSON* baseName = transfer_node_find(target, TAG_BASE_NAME, cJSON_String);
		if(!baseName)
		{
			transfer_group_generate(target->child, pGroup);
				target = target->next;
				continue;
				/*pCurNode = IoT_FindGroup(pGroup, target->string);
				if(!pCurNode)
					pCurNode = IoT_AddGroup(pGroup, target->string);*/
		}
			else
			{
		if(!baseName->valuestring) return;
		pCurNode = IoT_FindGroup(pGroup, baseName->valuestring);
		if(!pCurNode)
			pCurNode = IoT_AddGroup(pGroup, baseName->valuestring);
	}
		}
	else if(target->type == cJSON_Array)
	{
		pCurNode = IoT_FindGroup(pGroup, target->string);
		if(!pCurNode)
			pCurNode = IoT_AddGroupArray(pGroup, target->string);
	}
	else
		{
			target = target->next;
			continue;
		}

	child = target->child;
	while(child)
	{
		MSG_ATTRIBUTE_T* attr = NULL;

		if(transfer_node_check(child, TAG_BASE_NAME, cJSON_String))
			strncpy(pCurNode->classname, child->valuestring, strlen(child->valuestring));
		else if(transfer_node_check(child, TAG_VERSION, cJSON_String))
			strncpy(pCurNode->version, child->valuestring, strlen(child->valuestring));
		else if(transfer_node_check(child, TAG_E_NODE, cJSON_Array))
		{
			cJSON* sensor = child->child;
			while(sensor)
			{
				transfer_sensor_generate(sensor, pCurNode);
				sensor = sensor->next;
			}
		}
		else
		{
			switch(child->type)
			{
				case cJSON_False:
				attr = IoT_FindGroupAttribute(pCurNode, child->string);
				if(attr == NULL)
					attr = IoT_AddGroupAttribute(pCurNode, child->string);
				IoT_SetBoolValue(attr, false, IoT_READONLY);
				break;
			case cJSON_True:
				attr = IoT_FindGroupAttribute(pCurNode, child->string);
				if(attr == NULL)
					attr = IoT_AddGroupAttribute(pCurNode, child->string);
				IoT_SetBoolValue(attr, true, IoT_READONLY);
				break;
			case cJSON_Number:
				attr = IoT_FindGroupAttribute(pCurNode, child->string);
				if(attr == NULL)
					attr = IoT_AddGroupAttribute(pCurNode, child->string);
				IoT_SetDoubleValue(attr, child->valuedouble, IoT_READONLY, NULL);
				break;
			case cJSON_String:
				attr = IoT_FindGroupAttribute(pCurNode, child->string);
				if(attr == NULL)
					attr = IoT_AddGroupAttribute(pCurNode, child->string);
				IoT_SetStringValue(attr, child->valuestring, IoT_READONLY);
				break;
			case cJSON_NULL:
				attr = IoT_FindGroupAttribute(pCurNode, child->string);
				if(attr == NULL)
				{
					attr = IoT_AddGroupAttribute(pCurNode, child->string);
					IoT_SetStringValue(attr, "", IoT_READONLY);
				}
				IoT_SetNULLValue(attr, IoT_READONLY);
				break;
			case cJSON_Array:
				transfer_group_generate(child, pCurNode);
				break;
			case cJSON_Object:
				{
					cJSON* date = cJSON_GetObjectItem(child, "$date");
					if(date)
					{
						attr = IoT_FindGroupAttribute(pCurNode, child->string);
						if(attr == NULL)
							attr = IoT_AddGroupAttribute(pCurNode, child->string);
						if(date->valuestring)
							IoT_SetDateValue(attr, date->valuestring, IoT_READONLY);
						else
							IoT_SetTimestampValue(attr, date->valueint, IoT_READONLY);
					}
					else
					{
						date = cJSON_GetObjectItem(child, "$timestamp");
						if(date)
						{
							attr = IoT_FindGroupAttribute(pCurNode, child->string);
							if(attr == NULL)
								attr = IoT_AddGroupAttribute(pCurNode, child->string);
							IoT_SetTimestampValue(attr, date->valueint, IoT_READONLY);
						}
						else
						{
							transfer_group_generate(child, pCurNode);
						}
					}
				}
				break;
			}
		}
		child = child->next;
	}
		target = target->next;
	}
}

bool transfer_parse_ipso(const char* data, MSG_CLASSIFY_T *pGroup)
{
	cJSON* root = NULL;
	cJSON* target = NULL;

	if(!data) return false;
	
	root = cJSON_Parse(data);
	if(!root) return false;
	target = root->child;
	transfer_group_generate(target, pGroup);
	cJSON_Delete(root);
	return true;
}

bool transfer_get_ipso_handlername(const char* data, char* handlerName)
{
	cJSON* root = NULL;
	cJSON* target = NULL;
	bool bRet = false;
	if(!data) return false;

	if(!handlerName) return false;
	
	root = cJSON_Parse(data);
	if(!root) return false;

	if(!root->child) return false;

	target = transfer_node_find(root->child, TAG_BASE_NAME, cJSON_String);
	if(!target)
	{
		strcpy(handlerName, root->child->string);
		bRet = true;
	}

	cJSON_Delete(root);
	return bRet;
}
