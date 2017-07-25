#ifndef _IOT_MESSAGE_GENERATE_H_
#define _IOT_MESSAGE_GENERATE_H_
#include "MsgGenerator.h"

typedef enum{
	IoT_NODEFINE = -1,
	IoT_READONLY = 0,
	IoT_WRITEONLY,
	IoT_READWRITE,
}IoT_READWRITE_MODE;

#ifdef __cplusplus
extern "C" {
#endif
	long long IoT_GetTimeTick();

	MSG_CLASSIFY_T* IoT_CreateRoot(char* handlerName);
	MSG_CLASSIFY_T* IoT_AddGroup(MSG_CLASSIFY_T* pNode, char* groupName);
	MSG_CLASSIFY_T* IoT_AddGroupArray(MSG_CLASSIFY_T* pNode, char* groupName);
	MSG_ATTRIBUTE_T* IoT_AddGroupAttribute(MSG_CLASSIFY_T* pNode, char* attrName);
	MSG_ATTRIBUTE_T* IoT_AddSensorNode(MSG_CLASSIFY_T* pNode, char* senName);
	
	bool IoT_SetDoubleValue(MSG_ATTRIBUTE_T* attr, double value, IoT_READWRITE_MODE readwritemode, char *unit);
	bool IoT_SetDoubleValueWithMaxMin(MSG_ATTRIBUTE_T* attr, double value, IoT_READWRITE_MODE readwritemode, double max, double min, char *unit);
	bool IoT_SetFloatValue(MSG_ATTRIBUTE_T* attr, float value, IoT_READWRITE_MODE readwritemode, char *unit);
	bool IoT_SetFloatValueWithMaxMin(MSG_ATTRIBUTE_T* attr, float value, IoT_READWRITE_MODE readwritemode, float max, float min, char *unit);
	bool IoT_SetBoolValue(MSG_ATTRIBUTE_T* attr, bool bvalue,IoT_READWRITE_MODE readwritemode);
	bool IoT_SetStringValue(MSG_ATTRIBUTE_T* attr, char *svalue, IoT_READWRITE_MODE readwritemode);

	bool IoT_SetTimestampValue(MSG_ATTRIBUTE_T* attr,  unsigned int value, IoT_READWRITE_MODE readwritemode);
	bool IoT_SetDateValue(MSG_ATTRIBUTE_T* attr, char *svalue, IoT_READWRITE_MODE readwritemode);
	bool IoT_SetNULLValue(MSG_ATTRIBUTE_T* attr, IoT_READWRITE_MODE readwritemode);

	MSG_CLASSIFY_T* IoT_FindGroup(MSG_CLASSIFY_T* pParentNode, char* groupName);
	MSG_ATTRIBUTE_T* IoT_FindGroupAttribute(MSG_CLASSIFY_T* pNode, char* attrName);
	MSG_ATTRIBUTE_T* IoT_FindSensorNode(MSG_CLASSIFY_T* pNode, char* senName);
	MSG_ATTRIBUTE_T* IoT_FindSensorNodeWithPath(MSG_CLASSIFY_T *msg,char *path);
	bool IoT_IsSensorExist(MSG_CLASSIFY_T *msg,char *path);

	bool IoT_DelGroup(MSG_CLASSIFY_T* pParentNode, char* groupName);
	bool IoT_DelGroupAttribute(MSG_CLASSIFY_T* pNode, char* attrName);
	bool IoT_DelSensorNode(MSG_CLASSIFY_T* pNode, char* senName);
	void IoT_ReleaseAll(MSG_CLASSIFY_T* pRoot);

	char *IoT_PrintCapability(MSG_CLASSIFY_T* pRoot);
	char *IoT_PrintFullCapability(MSG_CLASSIFY_T* pRoot, char *agentID);
	char *IoT_PrintData(MSG_CLASSIFY_T* pRoot);
	char *IoT_PrintFullData(MSG_CLASSIFY_T* pRoot, char *agentID);
	char *IoT_PrintSelectedData(MSG_CLASSIFY_T* pRoot, char* reqItems);
	char *IoT_PrintFullSelectedData(MSG_CLASSIFY_T* pRoot, char* reqItems, char *agentID);

	void IoT_SetDataChangeCallback(MSG_CLASSIFY_T* pRoot, AttributeChangedCbf on_datachanged, void* pRev1);

	char *IoT_GetReadWriteString(IoT_READWRITE_MODE readwritemode);
	IoT_READWRITE_MODE IoT_GetReadWriteMode(char* readwritemode);

#ifdef __cplusplus
}
#endif
#endif