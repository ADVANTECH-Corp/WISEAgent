#ifndef _DLFCN_H
#define _DLFCN_H 

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_LAZY 0

void *dlopen(const char *filename, int flags);

int dlclose(void *handle);

void *dlsym(void *handle, const char *name); 

char *dlerror(void);

#ifdef __cplusplus
}
#endif


#endif