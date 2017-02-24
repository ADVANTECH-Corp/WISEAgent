/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/08/18 by Fred Chang									*/
/* Modified Date: 2015/08/18 by Fred Chang									*/
/* Abstract     :  					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef __sys_shm_h__
#define __sys_shm_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "export.h"
#include <sys/ipc.h>
	ADVPLAT_EXPORT long ADVPLAT_CALL shmget(key_t key, size_t size, int shmflg);
	ADVPLAT_EXPORT void *ADVPLAT_CALL shmat(long shmid, const void *shmaddr, int shmflg);
	ADVPLAT_EXPORT int ADVPLAT_CALL shmdt(const void *shmaddr);

#ifdef __cplusplus
}
#endif

#endif //__sys_shm_h__