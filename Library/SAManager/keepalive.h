#ifndef _SA_KEEPALIVE_H_
#define _SA_KEEPALIVE_H_
#include "susiaccess_handler_mgmt.h"

void keepalive_initialize(Handler_List_t *pLoaderList, char* workdir, void * logger);
void keepalive_uninitialize();

#endif