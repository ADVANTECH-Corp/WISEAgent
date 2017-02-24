#include "util_string.h"
#include <string.h>
#include <Windows.h>

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

bool IsUTF8(const char * string)
{
	const unsigned char * bytes = NULL;
    if(!string)
        return false;

   bytes = (const unsigned char *)string;
    while(*bytes)
    {
        if( (// ASCII
             // use bytes[0] <= 0x7F to allow ASCII control characters
                bytes[0] == 0x09 ||
                bytes[0] == 0x0A ||
                bytes[0] == 0x0D ||
                (0x20 <= bytes[0] && bytes[0] <= 0x7E)
            )
        ) {
            bytes += 1;
            continue;
        }

        if( (// non-overlong 2-byte
                (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF)
            )
        ) {
            bytes += 2;
            continue;
        }

        if( (// excluding overlongs
                bytes[0] == 0xE0 &&
                (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// straight 3-byte
                ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
                    bytes[0] == 0xEE ||
                    bytes[0] == 0xEF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// excluding surrogates
                bytes[0] == 0xED &&
                (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            )
        ) {
            bytes += 3;
            continue;
        }

        if( (// planes 1-3
                bytes[0] == 0xF0 &&
                (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// planes 4-15
                (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// plane 16
                bytes[0] == 0xF4 &&
                (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            )
        ) {
            bytes += 4;
            continue;
        }

        return false;
    }

    return true;
}