#ifndef _SUSIACCESSAGENT_LOG_H_
#define _SUSIACCESSAGENT_LOG_H_

#include <Log.h>



#define DEF_SUSIACCESSAGENT_LOG_NAME    "AgentLog.txt"   //default log file name
#define SUSIACCESSAGENT_LOG_ENABLE
//#define DEF_SUSIACCESSAGENT_LOG_MODE    (LOG_MODE_NULL_OUT)
//#define DEF_SUSIACCESSAGENT_LOG_MODE    (LOG_MODE_FILE_OUT)
#define DEF_SUSIACCESSAGENT_LOG_MODE    (LOG_MODE_CONSOLE_OUT|LOG_MODE_FILE_OUT)
LOGHANDLE SUSIAccessAgentLogHandle;
#ifdef SUSIACCESSAGENT_LOG_ENABLE
#define SUSIAccessAgentLog(level, fmt, ...)  do { if (SUSIAccessAgentLogHandle != NULL)   \
	WriteLog(SUSIAccessAgentLogHandle, DEF_SUSIACCESSAGENT_LOG_MODE, level, fmt, ##__VA_ARGS__); } while(0)
#else
#define SUSIAccessAgentLog(level, fmt, ...)
#endif

#endif