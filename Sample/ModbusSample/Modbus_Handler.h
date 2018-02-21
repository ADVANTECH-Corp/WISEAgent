/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.														 */
/* Create Date  : 2015 by Zach Chih															     */
/* Modified Date: 2015 by Zach Chih															 */
/* Abstract     : Modbus Handler                                   													*/
/* Reference    : None																									 */
/****************************************************************************/
#ifndef _MODBUS_HANDLER_H_
#define _MODBUS_HANDLER_H_

#define cagent_request_susi_control  20
#define cagent_reply_susi_control    114

#define HANDLER_NAME_LENGTH 50
#define SENSOR_TYPE_LENGTH 50
#define SENSOR_NAME_LENGTH 50

#if defined(_WIN32) || defined(WIN32)
#include <winsock2.h>
#endif

#if defined(__GNUC__)
#  define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__ * 10)
#  if GCC_VERSION >= 430
// Since GCC >= 4.30, GCC provides __builtin_bswapXX() alternatives so we switch to them
#    undef bswap_32
#    define bswap_32 __builtin_bswap32
#  endif
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
# define bswap_32 _byteswap_ulong
#endif

#include "modbus.h"
#include "util_path.h"

typedef struct{
	char handler[HANDLER_NAME_LENGTH];
	char type[SENSOR_TYPE_LENGTH];
	char name[SENSOR_NAME_LENGTH];
}WISE_Sensor;


//iot set/get result code
#define IOT_SGRC_NOT_FOUND     404		//Get&Set
#define IOT_SGRC_SUCCESS       200		//Get&Set
#define IOT_SGRC_FAIL          500		//Get&Set
#define IOT_SGRC_READ_ONLY     405		//Set 
#define IOT_SGRC_WRIT_ONLY     405		//Get no use in Modbus
#define IOT_SGRC_FORMAT_ERROR  415		//Set
#define IOT_SGRC_SYS_BUSY      503		//Set
#define IOT_SGRC_OUT_RANGE     416		//Set

#define IOT_SGRC_STR_NOT_FOUND       "Not Found"
#define IOT_SGRC_STR_SUCCESS         "Success"
#define IOT_SGRC_STR_FAIL            "Fail"
#define IOT_SGRC_STR_READ_ONLY       "Read Only" 
#define IOT_SGRC_STR_WRIT_ONLY       "Writ Only" 
#define IOT_SGRC_STR_FORMAT_ERROR    "Format Error" 
#define IOT_SGRC_STR_SYS_BUSY        "Sys Busy"
#define IOT_SGRC_STR_OUT_RANGE       "Out of Range"

typedef enum{
	SSR_SUCCESS = 0,
	SSR_FAIL=1,
	SSR_READ_ONLY = 2,
	SSR_WRITE_ONLY = 3,
	SSR_OVER_RANGE = 4,
	SSR_SYS_BUSY = 5,
	SSR_WRONG_FORMAT = 6,
	SSR_NOT_FOUND = 7,
}sensor_set_ret;

typedef struct sensor_info_t{
	int id;
	char * pathStr;
	char * jsonStr;
	sensor_set_ret setRet;
}sensor_info_t;
typedef struct sensor_info_node_t{
	sensor_info_t sensorInfo;
	struct sensor_info_node_t * next;
}sensor_info_node_t;
typedef sensor_info_node_t * sensor_info_list;

typedef enum{
	IOT_IT_Unknown,
	IOT_IT_PLAT_INFO,
	IOT_IT_HWM_TEMP,
	IOT_IT_HWM_VOLT,
	IOT_IT_HWM_FAN,
}iot_item_type_t;

typedef enum{
	VT_Unknown,
	VT_S,
	VT_F,
}iot_value_type_t;

typedef union iot_value_u_t{
	float vf;
	char * vs;
}iot_value_u_t;

typedef struct iot_value_t{
	iot_value_type_t vType;
	iot_value_u_t    uVal;
}iot_value_t;

typedef struct iot_data_info_t{
	iot_value_t val;
	unsigned int id;
	char * name;
}iot_data_info_t;

typedef struct iot_data_node_t{
	iot_data_info_t dataInfo;
	struct iot_data_node_t * next;
}iot_data_node_t;

typedef iot_data_node_t * iot_data_list;

typedef struct iot_hwm_info_t{
	iot_data_list tempDataList;
	iot_data_list voltDataList;
	iot_data_list fanDataList;
}iot_hwm_info_t;


//---------------------------------thr----------------------------
#define DEF_THR_UNKNOW_TYPE                0
#define DEF_THR_MAX_TYPE                   1
#define DEF_THR_MIN_TYPE                   2
#define DEF_THR_MAXMIN_TYPE                3

#define DEF_INVALID_TIME                   (-1) 
#define DEF_INVALID_VALUE                  (-999) 

#define DEF_MAX_THR_EVENT_STR                   "#tk#maxThreshold#tk#"
#define DEF_MIN_THR_EVENT_STR                   "#tk#minThreshold#tk#"
#define DEF_AVG_EVENT_STR                       "#tk#average#tk#"
#define DEF_MAX_EVENT_STR                       "#tk#max#tk#"
#define DEF_MIN_EVENT_STR                       "#tk#min#tk#"
#define DEF_AND_EVENT_STR                       "#tk#and#tk#"
#define DEF_NOR_EVENT_STR                       "#tk#normal#tk#"
#define DEF_NOT_SUPT_EVENT_STR                  "#tk#not surport#tk#"

typedef enum{
	ck_type_unknow = 0,
	ck_type_max,
	ck_type_min,
	ck_type_avg,
}check_type_t;

/*
typedef union check_value_t{
	float vf;
	int vi;
}check_value_t;*/

typedef struct check_value_node_t{
	float ckV;
	long long checkValTime;
	struct check_value_node_t * next;
}check_value_node_t;

typedef struct check_value_list_t{
	check_value_node_t * head;
	int nodeCnt;
}check_value_list_t;

typedef struct sa_thr_item_info_t{
	unsigned int id;
	char * name;
	char * desc;
	float maxThr;
	float minThr;
	int thrType;
	int isEnable;
	int lastingTimeS;
	int intervalTimeS;
	check_type_t checkType;
	float checkRetValue;
	check_value_list_t checkSrcValList;
	long long repThrTime;
	int isNormal;
}sa_thr_item_info_t;

typedef sa_thr_item_info_t  modbus_thr_item_info_t;

typedef struct modbus_thr_item_node_t{
	modbus_thr_item_info_t thrItemInfo;
	struct modbus_thr_item_node_t * next;
}modbus_thr_item_node_t;

typedef modbus_thr_item_node_t * modbus_thr_item_list;

typedef struct sa_thr_rep_info_t{
	int isTotalNormal;
	char repInfo[1024*2];
}sa_thr_rep_info_t;

typedef sa_thr_rep_info_t modbus_thr_rep_t;

typedef iot_item_type_t modbus_thr_item_type_t;

typedef iot_value_type_t  modbus_thr_value_type_t;

//----------------------------------------------------------------
#endif