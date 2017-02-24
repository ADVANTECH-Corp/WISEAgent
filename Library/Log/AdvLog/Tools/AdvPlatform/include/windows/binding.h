/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/08/18 by Fred Chang									*/
/* Modified Date: 2015/08/18 by Fred Chang									*/
/* Abstract     :  					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef __binding_h__
#define __binding_h__
#ifdef __cplusplus
extern "C" {
#endif
#include "export.h"
typedef int     ssize_t;
ADVPLAT_EXPORT ssize_t ADVPLAT_CALL getline(char **lineptr, size_t *n, void *stream);
#ifdef __cplusplus
}
#endif
#endif //__binding_h__