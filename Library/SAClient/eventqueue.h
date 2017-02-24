#ifndef _EVTQUEUE_H_
#define _EVTQUEUE_H_
#include <stdbool.h>
#include "susiaccess_def.h"

typedef void (*EVENT_UPDATECONNECTSTATE)(int status);

bool evtqueue_init(const unsigned int slots, EVENT_UPDATECONNECTSTATE func);

void evtqueue_uninit();

bool evtqueue_push(int status);

void evtqueue_clear();

#endif
