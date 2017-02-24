/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/08/18 by Fred Chang									*/
/* Modified Date: 2015/08/18 by Fred Chang									*/
/* Abstract     :  					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef __dirent_h__
#define __dirent_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "export.h"
#include <windows.h>

struct dirent {
	char *d_name;
};

typedef struct {
	HANDLE hFind;
	struct dirent dir;
	char *name;
} DIR;

ADVPLAT_EXPORT DIR *ADVPLAT_CALL opendir(const char *name);
ADVPLAT_EXPORT struct dirent *ADVPLAT_CALL readdir(DIR *dirp);
ADVPLAT_EXPORT int ADVPLAT_CALL closedir(DIR *dirp);

#ifdef __cplusplus
}
#endif
#endif //__dirent_h__