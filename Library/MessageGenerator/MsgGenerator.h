#ifndef _MESSAGE_GENERATE_H_
#define _MESSAGE_GENERATE_H_
#include <stdbool.h>

#define DEF_NAME_SIZE				260
#define DEF_CLASSIFY_SIZE			32
#define DEF_UNIT_SIZE				64
#define DEF_VERSION_SIZE			16
#define DEF_ASM_SIZE				3

#define TAG_BASE_NAME		"bn"
#define TAG_ATTR_NAME		"n"
#define TAG_VERSION			"ver"
#define TAG_E_NODE			"e"
#define TAG_VALUE			"v"
#define TAG_BOOLEAN			"bv"
#define TAG_STRING			"sv"
#define TAG_MAX				"max"
#define TAG_MIN				"min"
#define TAG_ASM				"asm"
#define TAG_UNIT			"u"
#define TAG_STATUS_CODE		"StatusCode"
#define TAG_DATE			"$date"
#define TAG_TIMESTAMP		"$timestamp"

typedef enum {
	attr_type_unknown = 0,
	attr_type_numeric = 1,
	attr_type_boolean = 2,
	attr_type_string = 3,
	attr_type_date = 4,
	attr_type_timestamp = 5,
} attr_type;

typedef enum {
	class_type_root = 0,
	class_type_object = 1,
	class_type_array = 2,
} class_type;

typedef void  (*AttributeChangedCbf) ( void* attribute, void* pRev1); 

typedef struct ext_attr{
	char name[DEF_NAME_SIZE];
	attr_type type;
	union
	{
		double v;
		bool bv;
		char* sv;
	};
	struct ext_attr *next;
}EXT_ATTRIBUTE_T;

typedef struct msg_attr{
	char name[DEF_NAME_SIZE];
	char readwritemode[DEF_ASM_SIZE];
	attr_type type;
	union
	{
		double v;
		bool bv;
		char* sv;
	};
	double max;
	double min;
	char unit[DEF_UNIT_SIZE];
	bool bRange;
	bool bSensor;
	bool bNull;
	struct msg_attr *next;

	struct ext_attr *extra;

	AttributeChangedCbf on_datachanged;
	void *pRev1;

}MSG_ATTRIBUTE_T;

typedef struct msg_class{
	char classname[DEF_NAME_SIZE];
	char version[DEF_VERSION_SIZE];
	bool bIoTFormat;
	class_type type;
	struct msg_attr *attr_list;
	struct msg_class *sub_list;
	struct msg_class *next;

	AttributeChangedCbf on_datachanged;
	void *pRev1;

}MSG_CLASSIFY_T;

#ifdef __cplusplus
extern "C" {
#endif
	long long MSG_GetTimeTick();

	MSG_CLASSIFY_T* MSG_CreateRoot();
	MSG_CLASSIFY_T* MSG_CreateRootEx(AttributeChangedCbf onchanged, void* pRev1);
	void MSG_ReleaseRoot(MSG_CLASSIFY_T* classify);

	MSG_CLASSIFY_T* MSG_AddClassify(MSG_CLASSIFY_T *pNode, char const* name, char const* version, bool bArray, bool isIoT);
	MSG_CLASSIFY_T* MSG_FindClassify(MSG_CLASSIFY_T* pNode, char const* name);
	bool MSG_DelClassify(MSG_CLASSIFY_T* pNode, char* name);

	MSG_ATTRIBUTE_T* MSG_AddAttribute(MSG_CLASSIFY_T* pClass, char const* attrname, bool isSensorData);
	MSG_ATTRIBUTE_T* MSG_FindAttribute(MSG_CLASSIFY_T* root, char const* attrname, bool isSensorData);
	bool MSG_DelAttribute(MSG_CLASSIFY_T* pNode, char* name, bool isSensorData);

	bool MSG_SetFloatValue(MSG_ATTRIBUTE_T* attr, float value, char* readwritemode, char *unit);
	bool MSG_SetFloatValueWithMaxMin(MSG_ATTRIBUTE_T* attr, float value, char* readwritemode, float max, float min, char *unit);
	bool MSG_SetDoubleValue(MSG_ATTRIBUTE_T* attr, double value, char* readwritemode, char *unit);
	bool MSG_SetDoubleValueWithMaxMin(MSG_ATTRIBUTE_T* attr, double value, char* readwritemode, double max, double min, char *unit);
	bool MSG_SetBoolValue(MSG_ATTRIBUTE_T* attr, bool bvalue, char* readwritemode);
	bool MSG_SetStringValue(MSG_ATTRIBUTE_T* attr, char *svalue, char* readwritemode);

	bool MSG_SetTimestampValue(MSG_ATTRIBUTE_T* attr, unsigned int value, char* readwritemode);
	bool MSG_SetDateValue(MSG_ATTRIBUTE_T* attr, char *svalue, char* readwritemode);
	bool MSG_SetNULLValue(MSG_ATTRIBUTE_T* attr, char* readwritemode);

	MSG_ATTRIBUTE_T* MSG_FindAttributeWithPath(MSG_CLASSIFY_T *msg,char *path, bool isSensorData);
	bool MSG_IsAttributeExist(MSG_CLASSIFY_T *msg,char *path, bool isSensorData);

	char* MSG_PrintUnformatted(MSG_CLASSIFY_T* msg);
	char *MSG_PrintWithFiltered(MSG_CLASSIFY_T* msg, char** filter, int length);
	char *MSG_PrintSelectedWithFiltered(MSG_CLASSIFY_T* msg, char** filter, int length, char* reqItems);

	void MSG_SetDataChangeCallback(MSG_CLASSIFY_T* msg, AttributeChangedCbf on_datachanged, void* pRev1);
	

#define MSG_AddJSONClassify(pNode, name, version, bArray) MSG_AddClassify(pNode, name, version, bArray, false);
#define MSG_AddIoTClassify(pNode, name, version, bArray) MSG_AddClassify(pNode, name, version, bArray, true);

#define MSG_AddJSONAttribute(pClass, attrname) MSG_AddAttribute(pClass, attrname, false);
#define MSG_AddIoTSensor(pClass, attrname) MSG_AddAttribute(pClass, attrname, true);
	bool MSG_AppendIoTSensorAttributeDouble(MSG_ATTRIBUTE_T* attr, const char* attrname, double value);
	bool MSG_AppendIoTSensorAttributeBool(MSG_ATTRIBUTE_T* attr, const char* attrname, bool bvalue);
	bool MSG_AppendIoTSensorAttributeString(MSG_ATTRIBUTE_T* attr, const char* attrname, char *svalue);
	bool MSG_AppendIoTSensorAttributeTimestamp(MSG_ATTRIBUTE_T* attr, const char* attrname, unsigned int value);
	bool MSG_AppendIoTSensorAttributeDate(MSG_ATTRIBUTE_T* attr, const char* attrname, char *svalue);

#define MSG_FindJSONAttribute(pClass, attrname) MSG_FindAttribute(pClass, attrname, false);
#define MSG_FindIoTSensor(pClass, attrname) MSG_FindAttribute(pClass, attrname, true);

#define MSG_DelJSONAttribute(pClass, attrname) MSG_DelAttribute(pClass, attrname, false);
#define MSG_DelIoTSensor(pClass, attrname) MSG_DelAttribute(pClass, attrname, true);

#ifdef __cplusplus
}
#endif
#endif