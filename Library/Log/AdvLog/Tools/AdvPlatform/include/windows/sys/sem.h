/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/05/25 by Fred Chang									*/
/* Modified Date: 2016/05/25 by Fred Chang									*/
/* Abstract     :  					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef __sys_sem_h__
#define __sys_sem_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "export.h"
#include <sys/ipc.h>
	ADVPLAT_EXPORT long ADVPLAT_CALL semget(key_t key, int nsems, int semflg);
	ADVPLAT_EXPORT int ADVPLAT_CALL semop(int semid, struct sembuf *sops, unsigned nsops);
#ifdef __cplusplus
}
#endif

#endif //__sys_sem_h__