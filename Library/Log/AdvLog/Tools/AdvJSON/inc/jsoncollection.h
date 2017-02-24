/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __JSON_COLLECTION_H__
#define __JSON_COLLECTION_H__

#include "jsontype.h"

typedef struct __JSONGarbage JSONGarbage;

struct __JSONGarbage {
	JSONGarbage *prev;
	JSONGarbage *next;
	JSONode *garbage;
};

typedef struct __JSONCollect {
	JSONGarbage *tail;
	int count;
} JSONCollect;

#ifdef __cplusplus
extern "C" {
#endif

JSONCollect *NewCollection();
JSONode *GetNewItem(JSONCollect* collect);
void DestroyItem(JSONCollect* collect, JSONode *node);
JSONode *ClearItem(JSONode *node);
void ReleaseCollection(JSONCollect** collect);

void LinkItem(JSONCollect* collect, JSONode *node);
void UnlinkItem(JSONCollect* collect, JSONode *node);


//global collection

void GC_Init();
void GC_Release();
JSONode *GC_NewItem();
void GC_DestroyItem(JSONode *node);

#ifdef __cplusplus
}
#endif
#endif //__JSON_COLLECTION_H__