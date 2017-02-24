#ifndef _UTIL_OS_H
#define _UTIL_OS_H 
#include <stdbool.h>
#include <stdint.h>

#ifndef ULONGLONG
typedef unsigned long long  ULONGLONG;
#endif 


#ifdef __cplusplus
extern "C" {
#endif

	bool util_os_get_os_name(char * pOSNameBuf, unsigned long * bufLen);
	bool util_os_get_processor_name(char * pProcessorNameBuf, unsigned long * bufLen);
	bool util_os_get_architecture(char * pArchBuf, int * bufLen);
	bool util_os_get_free_memory(uint64_t *totalPhysMemKB, uint64_t *availPhysMemKB);

#ifdef __cplusplus
}
#endif

#endif