#ifndef _UTIL_PROCESS_H
#define _UTIL_PROCESS_H 
#include <stdbool.h>

bool util_process_launch(char * appPath);

bool util_process_kill(char * processName);

#endif