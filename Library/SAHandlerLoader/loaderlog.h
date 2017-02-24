#ifndef _SALOADER_LOG_H_
#define _SALOADER_LOG_H_

#include "Log.h"

#define DEF_SALOADER_LOG_NAME    "AgentLog.txt"   //default log file name
#define SALOADER_LOG_ENABLE
//#define DEF_SALOADER_LOG_MODE    (LOG_MODE_NULL_OUT)
//#define DEF_SALOADER_LOG_MODE    (LOG_MODE_FILE_OUT)
#define DEF_SALOADER_LOG_MODE    (LOG_MODE_CONSOLE_OUT|LOG_MODE_FILE_OUT)
LOGHANDLE SALoaderLogHandle;
#ifdef SALOADER_LOG_ENABLE
#define SALoaderLog(level, fmt, ...)  do { if (SALoaderLogHandle != NULL)   \
	WriteLog(SALoaderLogHandle, DEF_SALOADER_LOG_MODE, level, fmt, ##__VA_ARGS__); } while(0)
#else
#define SALoaderLog(level, fmt, ...)
#endif

#endif