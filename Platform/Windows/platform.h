#ifndef _CAGENT_PLATFORM_H_
#define _CAGENT_PLATFORM_H_

#include <stdio.h>
#include <stdlib.h>

#define snprintf sprintf_s
#define popen _popen
#define pclose _pclose
typedef int ssize_t;

#include <stdbool.h>
//#ifdef _MSC_VER  
//#	define bool char
//#	define true 1
//#	define false 0
//#else
//#	ifndef __cplusplus
//#		include <stdbool.h>
//#	endif
//#endif

#ifdef _MSC_VER  
	typedef unsigned __int8 uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef __int32 int32_t; 
	typedef unsigned __int32 uint32_t; 
	typedef __int64 int64_t; 
	typedef unsigned __int64 uint64_t;  
#else 
#	ifndef __cplusplus
#		include <stdint.h> 
#	endif
#endif 


#ifdef _MSC_VER
#	include <time.h>
#   include <windows.h>
int gettimeofday(struct timeval *tp, void *tzp);
#else
#   include <sys/time.h>
#endif
int GetCurrentUTCISODate(char* datetime);

#ifdef _MSC_VER
#	include <io.h>
#	include <process.h>
#else
#   include <unistd.h>
#endif

#define FILE_SEPARATOR   '\\'

#define strcasecmp _stricmp

#define strncasecmp  strnicmp 

void TrimStr(char * str);

wchar_t * ANSIToUnicode(const char * str);

char * UnicodeToANSI(const wchar_t * str);

wchar_t * UTF8ToUnicode(const char * str);

char * UnicodeToUTF8(const wchar_t * str);

char * ANSIToUTF8(const char * str);

char * UTF8ToANSI(const char * str);

#endif