/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __JSON_CREATOR_H__
#define __JSON_CREATOR_H__

#include "jsontype.h"
#include "jsoncollection.h"
#ifdef __cplusplus
extern "C" {
#endif
JSONode *CreateVoid(JSONCollect* collect, JSONode *root, JSONode *prev);
JSONode *CreateString(const char *name, JSONCollect* collect, JSONode *root, JSONode *prev);
JSONode *CreateNullValue(JSONCollect* collect, JSONode *root, JSONode *prev);

JSONode *CreatePair(const char *name, JSONCollect* collect, JSONode *root, JSONode *prev);
JSONode *CreateObject(const char *name, JSONCollect* collect, JSONode *root, JSONode *prev, JSONode *old);

JSONode *CreateElement(JSONCollect* collect, JSONode *root, JSONode *prev);
JSONode *CreateArray(JSONCollect* collect, JSONode *root, JSONode *prev, JSONode *old);


void DeleteNode(JSONode *del);
JSONode *CopyNode(JSONode *node, JSONCollect* collect);
#ifdef __cplusplus
}
#endif
#endif //__JSON_CREATOR_H__