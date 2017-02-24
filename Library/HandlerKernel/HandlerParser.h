/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/08/24 by Scott Chang								    */
/* Modified Date: 2016/08/24 by Scott Chang									*/
/* Abstract     : HandlerKernel API definition								*/
/* Reference    : None														*/
/****************************************************************************/

#ifndef _HANDLER_PARSER_H_
#define _HANDLER_PARSER_H_

#include <stdbool.h>
#include "HandlerThreshold.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <windows.h>
#ifndef HANDLERPARSER_API
	#define HANDLERPARSER_API WINAPI
#endif
#else
	#define HANDLERPARSER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* **************************************************************************************
 *  Function Name: HandlerParser_ParseReceivedCMD
 *  Description: Parse received command to command id
 *  Input : void* data
 *			int datalen
 *  Output: int * cmdID
 *			char* sessionID
 *  Return: true : Success
 *          false : Fail
 * ***************************************************************************************/
bool HANDLERPARSER_API HandlerParser_ParseReceivedCMD(void* data, int datalen, int * cmdID, char* sessionID);

/* **************************************************************************************
 *  Function Name: HandlerParser_ParseAutoReportCmd
 *  Description: Parse the auto report command
 *  Input : char * cmdJsonStr
 *			char * handlername
 *  Output: unsigned int * intervalTimeS
 *			char * reqItems
 *			bool * reqAll
 *  Return: true : Success
 *          false : Fail
 * ***************************************************************************************/
bool HANDLERPARSER_API HandlerParser_ParseAutoReportCmd(char * cmdJsonStr, char * handlername, unsigned int * intervalTimeS, char ** reqItems, bool * reqAll);

/* **************************************************************************************
 *  Function Name: HandlerParser_ParseAutoUploadCmd
 *  Description: Parse the live report command
 *  Input : char * cmdJsonStr
 *			char * handlername
 *  Output: unsigned int * intervalTimeMs
 *			unsigned int * timeoutMs
 *			char * reqItems
 *			bool * reqAll
 *  Return: true : Success
 *          false : Fail
 * ***************************************************************************************/
bool HANDLERPARSER_API HandlerParser_ParseAutoUploadCmd(char * cmdJsonStr, char * handlername, unsigned int * intervalTimeMs, unsigned int * timeoutMs, char ** reqItems, bool * reqAll);

/* **************************************************************************************
 *  Function Name: HandlerParser_ParseThrInfo
 *  Description: Parse the threshold set command
 *  Input : char * cmdJsonStr
 *  Output: thr_item_list thrList
 *			MSG_ATTRIBUTE_T* attr
 *  Return: true : Success
 *          false : Fail
 * ***************************************************************************************/
bool HANDLERPARSER_API HandlerParser_ParseThrInfo(char * thrJsonStr, thr_item_list thrList, void (*on_triggered)(struct thr_item_info_t* item, MSG_ATTRIBUTE_T* attr));

/* **************************************************************************************
 *  Function Name: HandlerParser_PackSetThrRep
 *  Description: Packet the threshold set reply message
 *  Input : char * repStr
 *  Output: char ** outputStr
 *  Return: true : Success
 *          false : Fail
 * ***************************************************************************************/
bool HANDLERPARSER_API HandlerParser_PackSetThrRep(char * repStr, char ** outputStr);

/* **************************************************************************************
 *  Function Name: HandlerParser_PackDelAllThrRep
 *  Description: Packet the threshold delete all reply message
 *  Input : char * repStr
 *  Output: char ** outputStr
 *  Return: true : Success
 *          false : Fail
 * ***************************************************************************************/
bool HANDLERPARSER_API HandlerParser_PackDelAllThrRep(char * repStr, char ** outputStr);

/* **************************************************************************************
 *  Function Name: HandlerParser_ParseAutoUploadCmd
 *  Description: Parse the live report command
 *  Input : char * cmdJsonStr
 *  Output: char** reqItems
 *			char** sessionID
 *  Return: true : Success
 *          false : Fail
 * ***************************************************************************************/
bool HANDLERPARSER_API HandlerParser_ParseSensorDataCmd(char * cmdJsonStr, char ** reqItems);

/* **************************************************************************************
 *  Function Name: HandlerParser_PackSensorCMDRep
 *  Description: Packet the sensor get/set command reply message
 *  Input : char * repStr
 *			char * sessionID
 *  Output: char ** outputStr
 *  Return: true : Success
 *          false : Fail
 * ***************************************************************************************/
bool HANDLERPARSER_API HandlerParser_PackSensorCMDRep(char * repStr, char * sessionID , char ** outputStr);


#ifdef __cplusplus
}
#endif

#endif

