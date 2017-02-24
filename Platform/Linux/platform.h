#ifndef _CAGENT_PLATFORM_H_
#define _CAGENT_PLATFORM_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#define ERROR_SUCCESS	0L
#define MAX_PATH		260

#define FILE_SEPARATOR   '/'
#define FILE_APPRUNDIR	"/var/run"

#define _vsnprintf vsnprintf
#define _strnicmp strncasecmp
#define _stricmp strcasecmp
#define stricmp _stricmp
#define _strdup strdup
#define _access access
#define strncpy_s(strDest, numberOfElements, strSource, count) (strncpy(strDest, strSource, count)) 

#define sprintf_s(buffer, buffer_size, stringbuffer, ...) (sprintf(buffer, stringbuffer, __VA_ARGS__))
#define gets_s(buffer, buffer_size) (fgets(buffer, buffer_size, stdin))
int GetCurrentUTCISODate(char* datetime);

int GetModuleFileName( char* sModuleName, char* sFileName, int nSize);

void TrimStr(char * str);

wchar_t * ANSIToUnicode(const char * str);

char * UnicodeToANSI(const wchar_t * str);

wchar_t * UTF8ToUnicode(const char * str);

char * UnicodeToUTF8(const wchar_t * str);

char * ANSIToUTF8(const char * str);

char * UTF8ToANSI(const char * str);

#endif
