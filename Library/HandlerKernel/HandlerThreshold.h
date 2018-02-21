/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/08/24 by Scott Chang								    */
/* Modified Date: 2016/08/24 by Scott Chang									*/
/* Abstract     : HandlerKernel API definition								*/
/* Reference    : None														*/
/****************************************************************************/

#ifndef _HANDLER_THRESHOLD_H_
#define _HANDLER_THRESHOLD_H_

#include <stdbool.h>
#include "IoTMessageGenerate.h"

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

typedef enum{
	range_unknow = 0,
	range_max,
	range_min,
	range_maxmin,
}range_type_t;

typedef struct check_value_node_t{
	double ckV;
	long long checkValTime;
	struct check_value_node_t * next;
}check_value_node_t;

typedef struct check_value_list_t{
	check_value_node_t * head;
	int nodeCnt;
}check_value_list_t;

typedef struct thr_item_info_t{
	char pathname[256];
	double maxThr;
	double minThr;
	range_type_t rangeType;
	bool isEnable;
	int lastingTimeS;
	int intervalTimeS;
	check_type_t checkType;
	double checkRetValue;
	check_value_list_t checkSrcValList;
	long long repThrTime;
	bool isNormal;
	bool isInvalid;
	void (*on_triggered)(void* qtrigger, struct thr_item_info_t* item, MSG_ATTRIBUTE_T* attr);
	void* pTriggerQueue;
}thr_item_info_t;

typedef struct thr_item_node_t{
	thr_item_info_t thrItemInfo;
	struct thr_item_node_t * next;
}thr_item_node_t;

typedef thr_item_node_t * thr_item_list;

typedef struct sa_thr_rep_info_t{
	bool isTotalNormal;
	char * repInfo;
}sa_thr_rep_info_t;

typedef sa_thr_rep_info_t thr_rep_t;

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <windows.h>
#ifndef HANDLERPARSER_API
	#define HANDLERTHRESHOLD_API WINAPI
#endif
#else
	#define HANDLERTHRESHOLD_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

thr_item_list HANDLERTHRESHOLD_API HandlerThreshold_CreateThrList();
void HANDLERTHRESHOLD_API HandlerThreshold_DestroyThrList(thr_item_list thrList);
bool HANDLERTHRESHOLD_API HandlerThreshold_InsertThrList(thr_item_list thrList, thr_item_info_t * pThrItem);
thr_item_node_t * HANDLERTHRESHOLD_API HandlerThreshold_FindThrNode(thr_item_list thrList, char* pathname);
bool HANDLERTHRESHOLD_API HandlerThreshold_DeleteThrNode(thr_item_list thrList, char* pathname);
bool HANDLERTHRESHOLD_API HandlerThreshold_DeleteAllThrNode(thr_item_list thrList);
bool HANDLERTHRESHOLD_API HandlerThreshold_UpdateThrInfoList(thr_item_list curThrList, thr_item_list newThrList, char ** checkRetMsg, unsigned int bufLen);
void HANDLERTHRESHOLD_API HandlerThreshold_WhenDelThrCheckNormal(thr_item_list thrItemList, char** checkMsg, unsigned int bufLen);
void HANDLERTHRESHOLD_API HandlerThreshold_IsThrItemListNormal(thr_item_list curThrItemList, bool * isNormal);
bool HANDLERTHRESHOLD_API HandlerThreshold_IsThrListEmpty(thr_item_list thrList);
bool HANDLERTHRESHOLD_API HandlerThreshold_CheckThr(thr_item_list curThrItemList, MSG_CLASSIFY_T* pCapability, char ** checkRetMsg, unsigned int bufLen, bool * isNormal);

#ifdef __cplusplus
}
#endif

#endif