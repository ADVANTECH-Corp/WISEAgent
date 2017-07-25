/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.														 */
/* Create Date  : 2015 by Zach Chih															     */
/* Modified Date: 2015 by Zach Chih															 */
/* Abstract     : Modbus Handler                                   													*/
/* Reference    : None																									 */
/****************************************************************************/
#ifndef _MODBUS_HANDLER_PARSER_H_
#define _MODBUS_HANDLER_PARSER_H_

//#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "Modbus_Handler.h"
//---------------
#include "util_path.h"


#define DEF_HANDLER_NAME            "Modbus_Handler"
#define AGENTINFO_BODY_STRUCT			"susiCommData"
#define AGENTINFO_REQID					"requestID"
#define AGENTINFO_CMDTYPE				"commCmd"

//--------------------------SUSI Ctrl monitor----------------------
#define MODBUS_CAPABILITY                  "Platform Information"
#define MODBUS_HARDWARE_MONITOR            "Hardware Monitor"
#define MODBUS_E_FLAG                      "e"
#define MODBUS_N_FLAG                      "n"
#define MODBUS_V_FLAG                      "v"
#define MODBUS_SV_FLAG                     "sv"
#define MODBUS_BV_FLAG                     "bv"
#define MODBUS_ID_FLAG                     "id"
#define MODBUS_DESC_FLAG                   "desc"
#define MODBUS_ASM_FLAG                    "asm"
#define MODBUS_STATUS_CODE_FLAG            "StatusCode"
#define MODBUS_HWM_TEMP                    "Temperature"
#define MODBUS_HWM_VOLT                    "Voltage"
#define MODBUS_HWM_FAN                     "Fan Speed"
#define MODBUS_SENSORS_ID                  "sensorsID"
#define MODBUS_SESSION_ID                  "sessionID"
#define MODBUS_SENSOR_ID_LIST              "sensorIDList"
#define MODBUS_SENSOR_INFO_LIST            "sensorInfoList"
#define MODBUS_SENSOR_SET_RET              "result"
#define MODBUS_ERROR_REP                   "errorRep"
#define MODBUS_SENSOR_SET_PARAMS           "sensorSetParams"
#define MODBUS_SENSOR_ID                   "id"
#define MODBUS_SENSOR_SET_RET              "result"
#define MODBUS_AUTOREP_REQ_ITEMS           "requestItems"
#define MODBUS_AUTOREP_ALL                 "All"
#define MODBUS_AUTOREP_INTERVAL_SEC        "autoUploadIntervalSec"
#define MODBUS_AUTOUPLOAD_INTERVAL_MS      "autoUploadIntervalMs"
#define MODBUS_AUTOUPLOAD_CONTINUE_MS      "autoUploadTimeoutMs"
#define MODBUS_SET_THR_REP                 "setThrRep"
#define MODBUS_DEL_ALL_THR_REP             "delAllThrRep"
#define MODBUS_THR_CHECK_STATUS            "thrCheckStatus"
#define MODBUS_THR_CHECK_MSG               "thrCheckMsg"

#define MODBUS_JSON_ROOT_NAME              "susiCommData"
#define MODBUS_THR                         "susictrlThr"
#define MODBUS_THR_DESC                    "desc"
#define MODBUS_THR_ID                      "id"
#define MODBUS_THR_MAX                     "max"
#define MODBUS_THR_MIN                     "min"
#define MODBUS_THR_TYPE                    "type"
#define MODBUS_THR_LTIME                   "lastingTimeS"
#define MODBUS_THR_ITIME                   "intervalTimeS"
#define MODBUS_THR_ENABLE                  "enable"
//-----------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------
//findType   0      unknow
//           1      value
//           2      id
//-----------------------------------------------------
typedef enum{
	PFT_UNKONW = 0,
	PFT_VALUE  =1,
	PFT_ID     =2,
	PFT_ASM    =3,
	PFT_DESC   =4,
}path_find_t;

bool ParseReceivedData(void* data, int datalen, int * cmdID);
int Parser_PackModbusError(char * errorStr, char** outputStr);
bool Parser_ParseGetSensorDataReqEx(void * data, sensor_info_list siList, char * pSessionID);
bool Parser_ParseSetSensorDataReqEx(void* data, sensor_info_list sensorInfoList, char * sessionID);
bool Parser_ParseAutoReportCmd(char * cmdJsonStr, unsigned int * intervalTimeS, char * repFilter);
bool Parser_ParseAutoUploadCmd(char * cmdJsonStr, unsigned int * intervalTimeMs, unsigned int * continueTimeMs, char * repFilter);

//--------------------------------------------------------------------------------------------------------------
//------------------------------------------------Modbus_General_Node_Parser-------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
//Parse Sensor Node in JSON
bool Modbus_General_Node_Parser(sensor_info_node_t *,WISE_Sensor *,int);
//Parse Sensor Path
bool Modbus_General_Paths_Parser(char *,WISE_Sensor *,int);
//Check Format of "Set" Command
bool Modbus_Parser_Set_FormatCheck(sensor_info_node_t *,bool *,double *,char *);



#ifdef __cplusplus
}
#endif

#endif