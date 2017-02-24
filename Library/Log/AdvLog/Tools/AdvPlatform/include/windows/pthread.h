/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/08/18 by Fred Chang									*/
/* Modified Date: 2015/08/18 by Fred Chang									*/
/* Abstract     :  					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef __pthread_h__
#define __pthread_h__
#ifdef __cplusplus
extern "C" {
#endif

#include "export.h"
#include <windows.h>

#define pthread_t DWORD
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_create(pthread_t *thread, void *attr,void *(*start_routine) (void *), void *arg);
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_join(pthread_t thread, void **retval);


#define pthread_mutex_t HANDLE
#define __SIZEOF_PTHREAD_MUTEXATTR_T 4
typedef union
{
  char __size[__SIZEOF_PTHREAD_MUTEXATTR_T];
  int __align;
} pthread_mutexattr_t;

ADVPLAT_EXPORT int ADVPLAT_CALL pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t * attr);

#define pthread_mutex_destroy(x)	CloseHandle(*x)
#define pthread_mutex_lock(x)		WaitForSingleObject(*x,INFINITE)
#define pthread_mutex_unlock(x)		ReleaseMutex(*x)
#ifdef __cplusplus
}
#endif

#endif //__pthread_h__