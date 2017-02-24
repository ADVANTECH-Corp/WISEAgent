/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/08/18 by Fred Chang									*/
/* Modified Date: 2015/08/18 by Fred Chang									*/
/* Abstract     :  					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef __wrapper_h__
#define __wrapper_h__
#ifdef __cplusplus
extern "C" {
#endif
#include "export.h"
#include <process.h>
#include <direct.h>
#include <windows.h>
#include <stdio.h>

#define __func__ __FUNCTION__

//A
//B
//C
#define chmod _chmod
//D
//E
//F
#define fileno _fileno
//G
#define getpid _getpid
//H
//I
//J
//K
//L
#define localtime_r(t,tm) localtime_s(tm,t)
//M
#define mkdir(path,flag) _mkdir(path);
//N
//O
//P
#define popen _popen
#define pclose _pclose
//Q
//R
//S
#define strdup _strdup
#define snprintf(dst,size,format,...) _snprintf(dst,size,format,##__VA_ARGS__)
#define strtok_r strtok_s
#define strcasecmp _stricmp
typedef int     ssize_t;
//T
//U
#define unlink _unlink
//V
#ifndef va_copy
    #ifdef __va_copy
        #define va_copy(dest, src)          __va_copy((dest), (src))
    #else
        #define va_copy(dest, src)          memcpy((&dest), (&src), sizeof(va_list))
    #endif
#endif
//W
#define warn(format, ...) fprintf(stderr,format,##__VA_ARGS__)
//X
//Y
//Z



















ADVPLAT_EXPORT ssize_t ADVPLAT_CALL getline(char **lineptr, size_t *n, void *stream);

#ifdef __cplusplus
}
#endif
#endif //__wrapper_h__