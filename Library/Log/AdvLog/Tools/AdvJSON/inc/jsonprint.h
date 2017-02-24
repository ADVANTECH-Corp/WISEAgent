/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __JSON_PRINT_H__
#define __JSON_PRINT_H__

#include "jsontype.h"

#ifdef __cplusplus
extern "C" {
#endif
int PrintString(char* buffer, int size, JSONode *json);
int PrintNumber(char* buffer, int size, JSONode *json);
int PrintValue(char* buffer, int size, JSONode *json, int depth);
int PrintObject(char* buffer, int size, JSONode *json, int depth);
int PrintArray(char* buffer, int size, JSONode *json, int depth);

int PLString(JSONode *json, int depth);
int PLNumber(JSONode *json, int depth);
int PLValue(JSONode *json, int depth);
int PLObject(JSONode *json, int depth);
int PLArray(JSONode *json, int depth);

#ifdef __cplusplus
}
#endif
#endif //__JSON_PRINT_H__