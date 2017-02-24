#ifndef _SAGENERAL_LOG_H_
#define _SAGENERAL_LOG_H_

#include "Log.h"

#define SAGENERAL_LOG_ENABLE
//#define DEF_SAGENERAL_LOG_MODE    (LOG_MODE_NULL_OUT)
//#define DEF_SAGENERAL_LOG_MODE    (LOG_MODE_FILE_OUT)
#define DEF_SAGENERAL_LOG_MODE    (LOG_MODE_CONSOLE_OUT|LOG_MODE_FILE_OUT)

#ifdef SAGENERAL_LOG_ENABLE
#define SAGeneralLog(handle, level, fmt, ...)  do { if (handle != NULL)   \
	WriteLog(handle, DEF_SAGENERAL_LOG_MODE, level, fmt, ##__VA_ARGS__); } while(0)
#else
#define SAGeneralLog(level, fmt, ...)
#endif

#endif