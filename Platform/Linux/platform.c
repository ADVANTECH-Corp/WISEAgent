#include "platform.h"
#include <wchar.h>

int GetCurrentUTCISODate(char* datetime)
{
  time_t rawtime;
  
  struct tm * ptm;

  struct timeval tv;

  gettimeofday(&tv, NULL); 

  time ( &rawtime );

  ptm = gmtime ( &rawtime );

  sprintf(datetime, "%u-%u-%uT%u:%u:%u.%dZ", (1900+ptm->tm_year), (1+ptm->tm_mon), ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tv.tv_usec);
  
  return (0);
}

int GetModuleFileName( char* sModuleName, char* sFileName, int nSize)
{
 int ret = 0;
 if(sModuleName == NULL)
 {
 	readlink("/proc/self/exe", sFileName, nSize);

	if( 0 == access( sFileName, F_OK ) )
  	{
    	ret = strlen(sFileName);
  	}
 }
 else if(strchr( sModuleName,'/' ) != NULL )
 {
  strcpy( sFileName, sModuleName );
  if( 0 == access( sFileName, F_OK ) )
  {
    ret = strlen(sFileName);
  }
 }
 else
 {
  char* sPath = getenv("PATH");
  char* pHead = sPath;
  char* pTail = NULL;
  while( pHead != NULL && *pHead != '\0' )
  {
   pTail = strchr( pHead, ':' );
   if( pTail != NULL )
   {
    strncpy( sFileName, pHead, pTail-pHead );
    sFileName[pTail-pHead] = '\0';
    pHead = pTail+1;
   }
   else
   {
    strcpy( sFileName, pHead );
    pHead = NULL;
   }
   
   int nLen = strlen(sFileName);
   if( sFileName[nLen] != '/' )sFileName[nLen] = '/';
   strcpy( sFileName+nLen+1,sModuleName);
   if( 0 == access( sFileName, F_OK ) )
   {
    ret = strlen(sFileName);
    break;
   }
  }
 }
 return ret;
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
	/*TODO*/
	return wcRet;
}

char* UnicodeToUTF8(const wchar_t* str)
{
	char * cRet = NULL;
	/*TODO*/
	return cRet;
}


char* ANSIToUTF8(const char* str)
{
	char * cRet = NULL;
	/*TODO*/
	return cRet;
}

char* UTF8ToANSI(const char* str)
{
	char * cRet = NULL;
	/*TODO*/
	return cRet;
}