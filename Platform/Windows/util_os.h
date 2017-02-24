#ifndef _UTIL_OS_H
#define _UTIL_OS_H 
#include <stdbool.h>
#include <stdint.h>

#ifndef ULONGLONG
typedef unsigned long long  ULONGLONG;
#endif 

typedef void *EVENT_HANDLE;

#ifdef __cplusplus
extern "C" {
#endif

	bool util_os_get_os_name(char * pOSNameBuf, unsigned long * bufLen);
	bool util_os_get_processor_name(char * pProcessorNameBuf, unsigned long * bufLen);
	bool util_os_get_architecture(char * pArchBuf, int *bufLen);
	bool util_os_get_free_memory(uint64_t *totalPhysMemKB, uint64_t *availPhysMemKB);
/*
	EVENT_HANDLE util_os_register_eventsource(char * serverName, char *sourceName);
	bool util_os_report_event(void* eventHandle, unsigned long dwEventID, char * eventStr);
	bool util_os_deregister_eventsource(void* eventHandle);
*/
#ifdef __cplusplus
}
#endif

#endif