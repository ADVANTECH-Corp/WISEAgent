#ifndef _TRIGGERQUEUE_H_
#define _TRIGGERQUEUE_H_
#include <stdbool.h>
#include "HandlerThreshold.h"
#include "IoTMessageGenerate.h"

typedef void (*THRESHOLD_ON_TRIGGER)(bool isNormal, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev);

bool triggerqueue_init(const unsigned int slots, THRESHOLD_ON_TRIGGER func);

void triggerqueue_uninit();

bool triggerqueue_push(struct thr_item_info_t* item, MSG_ATTRIBUTE_T* attr);

void triggerqueue_clear();

#endif