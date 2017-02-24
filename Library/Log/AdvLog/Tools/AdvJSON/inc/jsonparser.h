/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include "jsontype.h"
#include "jsoncollection.h"
#ifdef __cplusplus
extern "C" {
#endif
JSONode *ParseString(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev);
JSONode *ParseNumber(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev);
JSONode *ParseValue(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev);
JSONode *ParseObject(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev);
JSONode *ParseArray(const char *json, JSONCollect* collect, JSONode *root, JSONode *prev);
#ifdef __cplusplus
}
#endif
#endif //__JSON_PARSER_H__


