#ifndef _UTIL_PROCESS_H
#define _UTIL_PROCESS_H 
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool util_process_launch(char * appPath);

bool util_process_kill(char * processName);

#ifdef __cplusplus
}
#endif

#endif