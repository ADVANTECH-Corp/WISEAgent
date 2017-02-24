#include "Log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


//using namespace zsummer::log4z;
//static ILog4zManager* g_logger = NULL;
#if 0
static void _WriteLog(LOGHANDLE logHandle, LoggerId id, ENUM_LOG_LEVEL level, char* log)
{
	if(!logHandle)
		return;
	g_logger = (ILog4zManager*)logHandle;
	if (g_logger->prePushLog(id,level))
	{
		char logBuf[LOG4Z_LOG_BUF_SIZE]={0};
		zsummer::log4z::Log4zStream ss(logBuf, LOG4Z_LOG_BUF_SIZE);
		ss << log;
		g_logger->pushLog(id, level, logBuf, NULL, NULL);
	}
}

static void WriteLogFile(LOGHANDLE logHandle, int id, LOGMODE logMode, LogLevel logLevel, const char * format, va_list ap)
{
    char logStr[2048] = {0};
    if(logHandle == NULL) return;
    vsnprintf(logStr, sizeof(logStr), format, ap);
	switch(logLevel)
    {
	case Debug:
	  {
		_WriteLog(logHandle, (LoggerId)id, LOG_LEVEL_DEBUG, logStr);
		break;
	  }
    case Normal:
	  {
		_WriteLog(logHandle, (LoggerId)id, LOG_LEVEL_INFO, logStr);
		break;
	  }
	case Warning:
      {
         _WriteLog(logHandle, (LoggerId)id, LOG_LEVEL_WARN, logStr);
         break;
      }
   case Error:
      {
         _WriteLog(logHandle, (LoggerId)id, LOG_LEVEL_ERROR, logStr);
         break;
      }
   case Alarm:
      {
         _WriteLog(logHandle, (LoggerId)id, LOG_LEVEL_ALARM, logStr);
         break;
      }
   case Fatal:
      {
         _WriteLog(logHandle, (LoggerId)id, LOG_LEVEL_FATAL, logStr);
         break;
      }
   default:
      break;
   }
}
#endif

static LOGHANDLE loghandle = (LOGHANDLE)0;
#define FILE_SEPARATOR   '\\'

#ifdef WIN32
#include <windows.h>
int util_module_path_get(char * moudlePath)
{
	int iRet = 0;
	char * lastSlash = NULL;
	char tempPath[512] = {0};
	if(NULL == moudlePath) return iRet;
	if(ERROR_SUCCESS != GetModuleFileName(NULL, tempPath, sizeof(tempPath)))
	{
		lastSlash = strrchr(tempPath, FILE_SEPARATOR);
		if(NULL != lastSlash)
		{
			strncpy(moudlePath, tempPath, lastSlash - tempPath + 1);
			iRet = lastSlash - tempPath + 1;
			moudlePath[iRet] = 0;
		}
	}
	return iRet;
}
#else
int util_module_path_get(char * moudlePath)
{
	/*int iRet = 0;
	char * lastSlash = NULL;
	char tempPath[512] = {0};
	if(NULL == moudlePath) return iRet;
	
	readlink("/proc/self/exe", tempPath, sizeof(tempPath));

	if( 0 == access( tempPath, F_OK ) )
	{
		lastSlash = strrchr(tempPath, FILE_SEPARATOR);
		if(NULL != lastSlash)
		{
			strncpy(moudlePath, tempPath, lastSlash - tempPath + 1);
			iRet = lastSlash - tempPath + 1;
		}	
	}
	return iRet;*/
	strcpy(moudlePath,"./");
	return 0;
}
#endif

static LOGHANDLE _InitLog(char * logFileName)
{
   char moudlePath[512];
   util_module_path_get(moudlePath);
   AdvLog_Init();

   char filename[512];
   char conf[512];
   sprintf(filename,"%s%s",moudlePath,"log.ini");
   sprintf(conf,"-f \"%s\" -n default",filename);
   AdvLog_Default(conf);
   loghandle = (LOGHANDLE)1;
   return loghandle;
}

SALOG_EXPORT LOGHANDLE SALOG_CALL InitLog(char * logFileName)
{
   //return _InitLog(logFileName);
   return (LOGHANDLE)1;
}

int GetLogID(LOGHANDLE logHandle,char * logname)
{
	return 0;
}
	
SALOG_EXPORT void SALOG_CALL UninitLog(LOGHANDLE logHandle)
{
	if(loghandle == (LOGHANDLE)1) {
		AdvLog_Uninit();
		loghandle = (LOGHANDLE)0;
	}
}

SALOG_EXPORT int SALOG_CALL TransId(int level) {
	if(loghandle == (LOGHANDLE)0) {
		_InitLog(NULL);
	}

	switch(level) {
		case Debug:
			return LOG_DEBUG;
		case Normal:
			return LOG_INFO;
		case Warning:
			return LOG_WARN;
		case Error:
			return LOG_ERROR;
		case Alarm:
			return LOG_NOTICE;
		case Fatal:
			return LOG_CRASH;
		default:
			return LOG_INFO;
	}
}

#if 0
SALOG_EXPORT void SALOG_CALL WriteLog(LOGHANDLE logHandle, LOGMODE logMode, LogLevel level, const char * format, ...)
{
	/*int id = -1;
	va_list ap;
    va_start(ap, format);
	id = GetLogID(logHandle, "agent");
    WriteLogFile(logHandle, id, logMode, level, format, ap);
    va_end(ap);*/
}

SALOG_EXPORT void SALOG_CALL WriteIndividualLog(LOGHANDLE logHandle, char* group, LOGMODE logMode, LogLevel level, const char * format, ...)
{
	/*int id = -1;
	va_list ap;
    va_start(ap, format);
	id = GetLogID(logHandle, group);
    WriteLogFile(logHandle, id, logMode, level, format, ap);
    va_end(ap);*/
}
#endif
