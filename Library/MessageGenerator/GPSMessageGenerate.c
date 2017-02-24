#include "GPSMessageGenerate.h"
#include <string.h>
#include <stdio.h>

#define TAG_GPS_ROOT			"GPS"
#define TAG_GPS_DEVICE_GROUP	"Devices"
#define CLASS_GPS_TPV			"TPV"

#define ATTR_GPS_TIME			"time"
#define ATTR_GPS_LATITUDE		"lat"
#define ATTR_GPS_LONGITUDE		"lon"
#define ATTR_GPS_ALTITUDE		"alt"
#define ATTR_GPS_TRACK			"track"
#define ATTR_GPS_SPEED			"speed"
#define ATTR_GPS_CLIMB			"climb"
#define ATTR_GPS_MODE			"mode"



#pragma region Add_Resource
MSG_CLASSIFY_T* GPS_CreateGPS(char* handlername)
{
	MSG_CLASSIFY_T *pRoot = MSG_CreateRoot();
	MSG_CLASSIFY_T *pGPS, *pDevs, *pHandler;
	if(pRoot)
	{
		pHandler =MSG_AddJSONClassify(pRoot, handlername, NULL, false);
		pGPS =MSG_AddIoTClassify(pHandler, TAG_GPS_ROOT, NULL, false);
		pDevs = MSG_AddIoTClassify(pGPS, TAG_GPS_DEVICE_GROUP, "1.0", false);
	}
	return pRoot;
}

MSG_CLASSIFY_T* GPS_AddDevice(MSG_CLASSIFY_T* gps, char const* device, char const* ver)
{
	MSG_CLASSIFY_T* gpsDev = NULL;
	MSG_CLASSIFY_T* target = NULL;
	if(!gps)
		return gpsDev;

	if(!gps->sub_list)
		return gpsDev;


	target = MSG_FindClassify(gps->sub_list, TAG_GPS_ROOT);
	if(!target)
		return gpsDev;

	target = MSG_FindClassify(target, TAG_GPS_DEVICE_GROUP);
	if(!target)
		return gpsDev;

	gpsDev = MSG_AddIoTClassify(target, device, ver, false);
	
	return gpsDev;
}

MSG_CLASSIFY_T* GPS_FindDevice(MSG_CLASSIFY_T* gps, char const* device)
{
	MSG_CLASSIFY_T* gpsDev = NULL;
	MSG_CLASSIFY_T* target = NULL;
	if(!gps)
		return gpsDev;

	if(!gps->sub_list)
		return gpsDev;

	target = MSG_FindClassify(gps->sub_list, TAG_GPS_ROOT);
	if(!target)
		return gpsDev;

	target = MSG_FindClassify(target, TAG_GPS_DEVICE_GROUP);
	if(!target)
		return gpsDev;

	gpsDev =  MSG_FindClassify(target, device);
	return gpsDev;
}

MSG_ATTRIBUTE_T* GPS_SetTimeAttribute(MSG_CLASSIFY_T* device, char* time)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device || !time)
		return attr;

	target = MSG_FindClassify(device, CLASS_GPS_TPV);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, CLASS_GPS_TPV, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, ATTR_GPS_TIME);
	if(!attr)
		attr = MSG_AddIoTSensor(target, ATTR_GPS_TIME);

	MSG_SetStringValue(attr, time, "r");

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetLatitudeAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, CLASS_GPS_TPV);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, CLASS_GPS_TPV, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, ATTR_GPS_LATITUDE);
	if(!attr)
		attr = MSG_AddIoTSensor(target, ATTR_GPS_LATITUDE);

	MSG_SetFloatValueWithMaxMin(attr, value, "r", max, min, unit);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetLongitudeAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, CLASS_GPS_TPV);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, CLASS_GPS_TPV, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, ATTR_GPS_LONGITUDE);
	if(!attr)
		attr = MSG_AddIoTSensor(target, ATTR_GPS_LONGITUDE);

	MSG_SetFloatValueWithMaxMin(attr, value, "r", max, min, unit);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetAltitudeAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, CLASS_GPS_TPV);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, CLASS_GPS_TPV, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, ATTR_GPS_ALTITUDE);
	if(!attr)
		attr = MSG_AddIoTSensor(target, ATTR_GPS_ALTITUDE);

	MSG_SetFloatValueWithMaxMin(attr, value, "r", max, min, unit);

	return attr;
}


MSG_ATTRIBUTE_T* GPS_SetTrackAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, CLASS_GPS_TPV);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, CLASS_GPS_TPV, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, ATTR_GPS_TRACK);
	if(!attr)
		attr = MSG_AddIoTSensor(target, ATTR_GPS_TRACK);

	MSG_SetFloatValueWithMaxMin(attr, value, "r", max, min, unit);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetSpeedAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, CLASS_GPS_TPV);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, CLASS_GPS_TPV, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, ATTR_GPS_SPEED);
	if(!attr)
		attr = MSG_AddIoTSensor(target, ATTR_GPS_SPEED);

	MSG_SetFloatValueWithMaxMin(attr, value, "r", max, min, unit);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetClimbAttribute(MSG_CLASSIFY_T* device, float value, float max, float min, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, CLASS_GPS_TPV);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, CLASS_GPS_TPV, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, ATTR_GPS_CLIMB);
	if(!attr)
		attr = MSG_AddIoTSensor(target, ATTR_GPS_CLIMB);

	MSG_SetFloatValueWithMaxMin(attr, value, "r", max, min, unit);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetModeAttribute(MSG_CLASSIFY_T* device, int value)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, CLASS_GPS_TPV);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, CLASS_GPS_TPV, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, ATTR_GPS_MODE);
	if(!attr)
		attr = MSG_AddIoTSensor(target, ATTR_GPS_MODE);

	MSG_SetFloatValue(attr, value, "r", NULL);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetCustomFloatAttribute(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, float value, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, classname);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, classname, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, attrname);
	if(!attr)
		attr = MSG_AddIoTSensor(target, attrname);

	MSG_SetFloatValue(attr, value, readwritemode, unit);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetCustomFloatAttributeWithMaxMin(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, float value, float max, float min, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, classname);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, classname, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, attrname);
	if(!attr)
		attr = MSG_AddIoTSensor(target, attrname);

	MSG_SetFloatValueWithMaxMin(attr, value, readwritemode, max, min, unit);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetCustomDoubleAttribute(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, double value, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, classname);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, classname, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, attrname);
	if(!attr)
		attr = MSG_AddIoTSensor(target, attrname);

	MSG_SetDoubleValue(attr, value, readwritemode, unit);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetCustomDoubleAttributeWithMaxMin(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, double value, double max, double min, char* unit)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, classname);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, classname, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, attrname);
	if(!attr)
		attr = MSG_AddIoTSensor(target, attrname);

	MSG_SetDoubleValueWithMaxMin(attr, value, readwritemode, max, min, unit);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetCustomBooleanAttribute(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode,bool value)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, classname);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, classname, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, attrname);
	if(!attr)
		attr = MSG_AddIoTSensor(target, attrname);

	MSG_SetBoolValue(attr, value, readwritemode);

	return attr;
}

MSG_ATTRIBUTE_T* GPS_SetCustomStringAttribute(MSG_CLASSIFY_T* device, char *classname, char *attrname, char *readwritemode, char* value)
{
	MSG_CLASSIFY_T* target = NULL;
	MSG_ATTRIBUTE_T* attr = NULL;
	if(!device)
		return attr;

	target = MSG_FindClassify(device, classname);
	if(!target)
	{
		target = MSG_AddIoTClassify(device, classname, NULL, false);
	}

	if(!target)
		return attr;

	attr = MSG_FindIoTSensor(target, attrname);
	if(!attr)
		attr = MSG_AddIoTSensor(target, attrname);

	MSG_SetStringValue(attr, value, readwritemode);

	return attr;
}

#pragma endregion Add_Resource

#pragma region Release_Resource
void GPS_ReleaseGPS(MSG_CLASSIFY_T* gps)
{
	MSG_ReleaseRoot(gps);
}
#pragma endregion Release_Resource

#pragma region Find_Resource

#pragma endregion Find_Resource

#pragma region Generate_JSON 
char *GPS_PrintUnformatted(MSG_CLASSIFY_T* gps)
{
	return MSG_PrintUnformatted(gps);
}
#pragma endregion Generate_JSON 


