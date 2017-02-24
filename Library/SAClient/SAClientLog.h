#ifndef _SACLIENT_LOG_H_
#define _SACLIENT_LOG_H_

#include <Log.h>

#define DEF_SACLIENT_LOG_NAME    "AgentLog.txt"   //default log file name
#define SACLIENT_LOG_ENABLE
//#define DEF_SACLIENT_LOG_MODE    (LOG_MODE_NULL_OUT)
//#define DEF_SACLIENT_LOG_MODE    (LOG_MODE_FILE_OUT)
#define DEF_SACLIENT_LOG_MODE    (LOG_MODE_CONSOLE_OUT|LOG_MODE_FILE_OUT)

#ifdef SACLIENT_LOG_ENABLE
#define SAClientLog(handle, level, fmt, ...)  do { if (handle != NULL)   \
	WriteLog(handle, DEF_SACLIENT_LOG_MODE, level, fmt, ##__VA_ARGS__); } while(0)
#else
#define SAClientLog(level, fmt, ...)
#endif

#endif