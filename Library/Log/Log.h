#ifndef _LOG_H_
#define _LOG_H_

#include "AdvLog.h"

#if defined(WIN32)
	#pragma once
	#ifdef SALOG_EXPORTS
		#define SALOG_CALL __stdcall
		#define SALOG_EXPORT __declspec(dllexport)
	#else
		#define SALOG_CALL __stdcall
		#define SALOG_EXPORT
	#endif
#else
	#define SALOG_CALL
	#define SALOG_EXPORT
#endif

typedef void *  LOGHANDLE;

typedef int LOGMODE;

#define LOG_MODE_NULL_OUT         0x00
#define LOG_MODE_CONSOLE_OUT      0x01
#define LOG_MODE_FILE_OUT         0x02

typedef enum LogLevel { 
   Debug = 0,
   Normal, 
   Warning, 
   Error,
   Alarm,
   Fatal
}LogLevel;

#ifdef __cplusplus
extern "C" {
#endif

SALOG_EXPORT LOGHANDLE SALOG_CALL InitLog(char * logFileName);

SALOG_EXPORT void SALOG_CALL UninitLog(LOGHANDLE logHandle);

SALOG_EXPORT int SALOG_CALL TransId(int level);

//SALOG_EXPORT void SALOG_CALL WriteLog(LOGHANDLE logHandle, LOGMODE logMode, LogLevel level, const char * format, ...);
#define WriteLog(logHandle, logMode, level, format, ...) ADV_PRINT(TransId(level), format"\n", ##__VA_ARGS__)
//#define WriteLog(logHandle, logMode, level, format, ...)
//int GetLogID(LOGHANDLE logHandle, char * logname);

//SALOG_EXPORT void SALOG_CALL WriteIndividualLog(LOGHANDLE logHandle, char* group, LOGMODE logMode, LogLevel level, const char * format, ...);
#define WriteIndividualLog(logHandle, group, logMode, level, format, ...) ADV_PRINT(TransId(level), format"\n", ##__VA_ARGS__)
//#define WriteIndividualLog(logHandle, group, logMode, level, format, ...)
#ifdef __cplusplus
}
#endif
#endif