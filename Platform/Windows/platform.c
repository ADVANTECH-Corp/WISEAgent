#include "platform.h"

#ifdef _MSC_VER
int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
 
    GetLocalTime(&wtm);
    tm.tm_year     = wtm.wYear - 1900;
    tm.tm_mon     = wtm.wMonth - 1;
    tm.tm_mday     = wtm.wDay;
    tm.tm_hour     = wtm.wHour;
    tm.tm_min     = wtm.wMinute;
    tm.tm_sec     = wtm.wSecond;
    tm. tm_isdst    = -1;
    clock = mktime(&tm);
    tp->tv_sec = (long)clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
 
    return (0);
}
#endif

int GetCurrentUTCISODate(char* datetime)
{
	SYSTEMTIME stUTC;
	GetSystemTime(&stUTC);

	sprintf(datetime, "%u-%u-%uT%u:%u:%u.%uZ", stUTC.wYear, stUTC.wMonth, stUTC.wDay, stUTC.wHour, stUTC.wMinute, stUTC.wSecond, stUTC.wMilliseconds);

	return (0);
}

void TrimStr(char * str)
{
	char *p, *stPos, *edPos;
	if(NULL == str) return;

	for (p = str; (*p == ' ' || *p == '\t') && *p != '\0'; p++);
	edPos = stPos = p;
	for (; *p != '\0'; p++)
	{
		if(*p != ' ' && *p != '\t') edPos = p;   
	}
	memmove(str, stPos, edPos - stPos + 1);
	*(str + (edPos - stPos + 1)) = '\0' ;
}

wchar_t * ANSIToUnicode(const char * str)
{
	int textLen = 0;
	wchar_t * wcRet = NULL;
	textLen = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	wcRet = (wchar_t*)malloc((textLen+1)*sizeof(wchar_t));
	memset(wcRet, 0, (textLen+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)wcRet, textLen);
	return wcRet;
}

char *UnicodeToANSI(const wchar_t *str)
{
	char *cRet = NULL;
	int textLen = 0;
	textLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	cRet = (char*)malloc((textLen+1)*sizeof(char));
	memset(cRet,0,sizeof(char)*(textLen+1));
	WideCharToMultiByte(CP_ACP, 0, str, -1, cRet, textLen,NULL,NULL);
	return cRet;
}

wchar_t *UTF8ToUnicode(const char* str)
{
	int textLen = 0;
	wchar_t * wcRet = NULL;
	textLen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	wcRet = (wchar_t *)malloc((textLen+1)*sizeof(wchar_t));
	memset(wcRet,0,(textLen+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)wcRet, textLen);
	return wcRet;
}

char* UnicodeToUTF8(const wchar_t* str)
{
	char * cRet = NULL;
	int textLen = 0;
	textLen = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	cRet = (char*)malloc((textLen+1)*sizeof(char));
	memset(cRet,0,sizeof(char)*(textLen+1));
	WideCharToMultiByte(CP_UTF8, 0, str, -1, cRet, textLen, NULL, NULL);
	return cRet;
}


char* ANSIToUTF8(const char* str)
{
	char * cRet = NULL;
	wchar_t * wcTmpStr = NULL;
	wcTmpStr = ANSIToUnicode(str);
	if(wcTmpStr != NULL)
	{
		cRet = UnicodeToUTF8(wcTmpStr);
		free(wcTmpStr);
	}
	return cRet;
}

char* UTF8ToANSI(const char* str)
{
	char * cRet = NULL;
	wchar_t * wcTmpStr = NULL;
	wcTmpStr = UTF8ToUnicode(str);
	if(wcTmpStr != NULL)
	{
		cRet = UnicodeToANSI(wcTmpStr);
		free(wcTmpStr);
	}
	return cRet;
}