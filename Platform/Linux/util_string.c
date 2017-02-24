#include "util_string.h"
#include <string.h>

wchar_t * ANSIToUnicode(const char * str)
{
	int textLen = 0;
	wchar_t * wcRet = NULL;
	int n = 0;
	textLen = mbstowcs(wcRet, str, 0)+1;  
	wcRet = (wchar_t*)malloc((textLen+1)*sizeof(wchar_t));
	memset(wcRet, 0, (textLen)*sizeof(wchar_t));
	n = mbstowcs(wcRet, str, textLen);  
	if(-1 == n)
	{
		free(wcRet);
		wcRet = NULL;
	} 
	return wcRet;
}

char *UnicodeToANSI(const wchar_t *str)
{
	char *cRet = NULL;
	int len = wcstombs(cRet, str, 0)+1; 
	int n = 0;
	cRet = (char*)malloc((len)*sizeof(char));
	memset(cRet,0,sizeof(char)*(len));
	n = wcstombs(cRet, str, len);
	if(-1 == n)
	{
		free(cRet);
		cRet = NULL;
	}    
	return cRet;
}

wchar_t *UTF8ToUnicode(const char* str)
{
	wchar_t * wcRet = NULL;
	/*todo*/
	wcRet = strdup(str);
	return wcRet;
}

char* UnicodeToUTF8(const wchar_t* str)
{
	char * cRet = NULL;
	/*todo*/
	cRet = strdup(str);
	return cRet;
}


char* ANSIToUTF8(const char* str)
{
	char * cRet = NULL;
	/*todo*/
	cRet = strdup(str);
	return cRet;
}

char* UTF8ToANSI(const char* str)
{
	char * cRet = NULL;
	/*todo*/
	cRet = strdup(str);
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