/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/08/18 by Fred Chang									*/
/* Modified Date: 2015/08/18 by Fred Chang									*/
/* Abstract     :  					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef __dlfcn_h__
#define __dlfcn_h__
#ifdef __cplusplus
extern "C" {
#endif
#include "export.h"

#define RTLD_LAZY 0


ADVPLAT_EXPORT void* ADVPLAT_CALL dlopen(const char *filename, int flag);
ADVPLAT_EXPORT char* ADVPLAT_CALL dlerror(void);
ADVPLAT_EXPORT void* ADVPLAT_CALL dlsym(void *handle, const char *symbol);
ADVPLAT_EXPORT int ADVPLAT_CALL dlclose(void *handle);




#ifdef __cplusplus
}
#endif

#endif //__dlfcn_h__