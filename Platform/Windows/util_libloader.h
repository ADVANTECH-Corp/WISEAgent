#ifndef _UTIL_LIBLOADER_H_
#define _UTIL_LIBLOADER_H_
#include <stdbool.h>
#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

bool util_dlexist(char* path);
bool util_dlopen(char* path, void ** lib);
bool util_dlclose(void * lib);
char* util_dlerror();
void* util_dlsym( void * handle, const char *name );

#ifdef __cplusplus
}
#endif

#endif
