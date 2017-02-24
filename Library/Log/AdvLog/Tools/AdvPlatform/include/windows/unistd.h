#ifndef __unistd_h__
#define __unistd_h__
#ifdef __cplusplus
extern "C" {
#endif
#include "export.h"
#include <io.h>
#include <direct.h>
#include <windows.h>

#define sleep(u) Sleep((u)*1000)
#define usleep(u) Sleep((u)/1000)

#define getcwd _getcwd

#define F_OK 0x00
#define access _access


ADVPLAT_EXPORT int ADVPLAT_CALL getppid();

#ifdef __cplusplus
}
#endif
#endif //__unistd_h__