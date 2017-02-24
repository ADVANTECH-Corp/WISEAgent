#ifndef _UTIL_STRING_H
#define _UTIL_STRING_H 

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
void TrimStr(char * str);

void TrimStr(char * str);

wchar_t * ANSIToUnicode(const char * str);

char * UnicodeToANSI(const wchar_t * str);

wchar_t * UTF8ToUnicode(const char * str);

char * UnicodeToUTF8(const wchar_t * str);

char * ANSIToUTF8(const char * str);

char * UTF8ToANSI(const char * str);

bool IsUTF8(const char * string);

#ifdef __cplusplus
}
#endif

#endif