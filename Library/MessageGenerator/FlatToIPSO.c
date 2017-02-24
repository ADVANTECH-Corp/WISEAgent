#include "FlatToIPSO.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "IoTMessageGenerate.h"
#include "cJSON.h"

int transfer_node_index(cJSON* target)
{
	int count = 0;
	cJSON* prev = target;
	while(prev)
	{
		count++;
		prev = prev->prev;
	}
	return count;
}

void transfer_node_generate(cJSON* target, MSG_CLASSIFY_T *pGroup, cJSON* parent)
{
	if(!target)
		return;

	while(target)
	{
		MSG_ATTRIBUTE_T* attr = NULL;
		cJSON* child = target->child;
		switch (target->type)
		{
		case cJSON_Array:
			{
				char tmp[DEF_NAME_SIZE] = {0};
				MSG_CLASSIFY_T *pChildGroup = NULL;
				if(target->string == NULL)
				{
					if(parent && parent->string)
						sprintf(tmp, "%s%d", parent->string,transfer_node_index(target));
					else
						sprintf(tmp, "arr%d",transfer_node_index(target));
				}
				else
					strcpy(tmp, target->string);
				pChildGroup = IoT_FindGroup(pGroup, tmp);
				if(pChildGroup == NULL)
					pChildGroup = IoT_AddGroupArray(pGroup, tmp);
				while(child)
				{
					transfer_node_generate(child, pChildGroup, target);
					child = child->next;
				}
			}
			break;
		case cJSON_False:
			attr = IoT_FindSensorNode(pGroup, target->string);
			if(attr == NULL)
				attr = IoT_AddSensorNode(pGroup, target->string);
			IoT_SetBoolValue(attr, false, IoT_READONLY);
			break;
		case cJSON_True:
			attr = IoT_FindSensorNode(pGroup, target->string);
			if(attr == NULL)
				attr = IoT_AddSensorNode(pGroup, target->string);
			IoT_SetBoolValue(attr, true, IoT_READONLY);
			break;
		case cJSON_Number:
			attr = IoT_FindSensorNode(pGroup, target->string);
			if(attr == NULL)
				attr = IoT_AddSensorNode(pGroup, target->string);
			IoT_SetDoubleValue(attr, target->valuedouble, IoT_READONLY, NULL);
			break;
		case cJSON_String:
			attr = IoT_FindSensorNode(pGroup, target->string);
			if(attr == NULL)
				attr = IoT_AddSensorNode(pGroup, target->string);
			IoT_SetStringValue(attr, target->valuestring, IoT_READONLY);
			break;
		case cJSON_Object:
			{
				cJSON* date = cJSON_GetObjectItem(target, "$date");
				if(date)
				{
					attr = IoT_FindSensorNode(pGroup, target->string);
					if(attr == NULL)
						attr = IoT_AddSensorNode(pGroup, target->string);
					if(date->valuestring)
						IoT_SetDateValue(attr, date->valuestring, IoT_READONLY);
					else
						IoT_SetTimestampValue(attr, date->valueint, IoT_READONLY);
				}
				else
				{
					date = cJSON_GetObjectItem(target, "$timestamp");
					if(date)
					{
						attr = IoT_FindSensorNode(pGroup, target->string);
						if(attr == NULL)
							attr = IoT_AddSensorNode(pGroup, target->string);
						IoT_SetTimestampValue(attr, date->valueint, IoT_READONLY);
					}
					else
					{
						char tmp[DEF_NAME_SIZE] = {0};
						MSG_CLASSIFY_T *pChildGroup = NULL;
						if(target->string == NULL)
						{
							if(parent && parent->string)
								sprintf(tmp, "%s%d", parent->string,transfer_node_index(target));
							else
								sprintf(tmp, "arr%d",transfer_node_index(target));
						}
						else
							strcpy(tmp, target->string);
						pChildGroup = IoT_FindGroup(pGroup, tmp);
						if(pChildGroup == NULL)
							pChildGroup = IoT_AddGroup(pGroup, tmp);
						while(child)
						{
							transfer_node_generate(child, pChildGroup, target);
							child = child->next;
						}
					}
				}
			}
			break;
		case cJSON_NULL:
			attr = IoT_FindSensorNode(pGroup, target->string);
			if(attr == NULL)
			{
				attr = IoT_AddSensorNode(pGroup, target->string);
				IoT_SetStringValue(attr, "", IoT_READONLY);
			}
			IoT_SetNULLValue(attr, IoT_READONLY);
			break;
		}
		target = target->next;
	}
}

bool transfer_parse_json(const char* data, MSG_CLASSIFY_T *pGroup)
{
	/*{"susiCommData":{"commCmd":251,"catalogID":4,"requestID":10}}*/

	cJSON* root = NULL;
	cJSON* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;

	if(!data) return false;
	
	root = cJSON_Parse(data);
	if(!root) return false;
	target = root->child;
	transfer_node_generate(target, pGroup, root);

	cJSON_Delete(root);
	return true;
}