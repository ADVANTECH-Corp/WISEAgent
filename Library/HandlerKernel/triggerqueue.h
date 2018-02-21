#ifndef _TRIGGERQUEUE_H_
#define _TRIGGERQUEUE_H_
#include <stdbool.h>
#include "HandlerThreshold.h"
#include "IoTMessageGenerate.h"

typedef void (*THRESHOLD_ON_TRIGGER)(bool isNormal, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev);

void* triggerqueue_init(const unsigned int slots);

void triggerqueue_uninit(void* qtrigger);

bool triggerqueue_setcb(void* qtrigger, THRESHOLD_ON_TRIGGER func);

bool triggerqueue_push(void* qtrigger, struct thr_item_info_t* item, MSG_ATTRIBUTE_T* attr);

void triggerqueue_clear(void* qtrigger);

#endif