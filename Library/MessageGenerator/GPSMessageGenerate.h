#ifndef _GPS_MESSAGE_GENERATE_H_
#define _GPS_MESSAGE_GENERATE_H_
#include "MsgGenerator.h"

#ifdef __cplusplus
extern "C" {
#endif

	MSG_CLASSIFY_T* GPS_CreateGPS(char* handlername);
	void GPS_ReleaseGPS(MSG_CLASSIFY_T* gps);

	MSG_CLASSIFY_T* GPS_AddDevice(MSG_CLASSIFY_T* gps, char const* device, char const* ver);
	MSG_CLASSIFY_T* GPS_FindDevice(MSG_CLASSIFY_T* gps, char const* device);

	MSG_ATTRIBUTE_T* GPS_SetTimeAttribute(MSG_CLASSIFY_T* device, char* time);
	MSG_ATTRIBUTE_T* GPS_SetLatitudeAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetLongitudeAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetAltitudeAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetTrackAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetSpeedAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetClimbAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetModeAttribute(MSG_CLASSIFY_T* device, int value);
	MSG_ATTRIBUTE_T* GPS_SetCustomFloatAttribute(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, float value, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetCustomFloatAttributeWithMaxMin(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, float value, float max, float min, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetCustomDoubleAttribute(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, double value, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetCustomDoubleAttributeWithMaxMin(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, double value, double max, double min, char* unit);
	MSG_ATTRIBUTE_T* GPS_SetCustomBooleanAttribute(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode,bool value);
	MSG_ATTRIBUTE_T* GPS_SetCustomStringAttribute(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, char* value);

	char  *GPS_PrintUnformatted(MSG_CLASSIFY_T* gps);

#ifdef __cplusplus
}
#endif
#endif