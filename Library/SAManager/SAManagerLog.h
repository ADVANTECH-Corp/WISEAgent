#ifndef _SACLIENT_LOG_H_
#define _SACLIENT_LOG_H_

#include <Log.h>

#define DEF_SACLIENT_LOG_NAME    "AgentLog.txt"   //default log file name
#define SAMANAGER_LOG_ENABLE
//#define DEF_SAMANAGER_LOG_MODE    (LOG_MODE_NULL_OUT)
//#define DEF_SAMANAGER_LOG_MODE    (LOG_MODE_FILE_OUT)
#define DEF_SAMANAGER_LOG_MODE    (LOG_MODE_CONSOLE_OUT|LOG_MODE_FILE_OUT)

#ifdef SAMANAGER_LOG_ENABLE
#define SAManagerLog(handle, level, fmt, ...)  do { if (handle != NULL)   \
	WriteLog(handle, DEF_SAMANAGER_LOG_MODE, level, fmt, ##__VA_ARGS__); } while(0)
#else
#define SAManagerLog(level, fmt, ...)
#endif

#endif