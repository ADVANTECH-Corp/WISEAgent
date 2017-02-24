/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/09/25 by Fred Chang									*/
/* Modified Date: 2015/09/25 by Fred Chang									*/
/* Abstract     : Advantech JSON Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __ADVANTECH_COLLECTION_H__
#define __ADVANTECH_COLLECTION_H__

typedef struct __JSONode JSONode;

JSONode *Allocate_GC();
void Destory_GC(JSONode *node);
JSONode *AllocateNumber_GC(double number);





#endif //__ADVANTECH_COLLECTION_H__
