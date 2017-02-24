#ifndef _MSGQUEUE_H_
#define _MSGQUEUE_H_
#include <stdbool.h>

typedef void (*MESSAGE_POP_CALLBACK)(const char* topic, const void* payload, const int payloadlen);

bool msgqueue_init(const unsigned int slots, MESSAGE_POP_CALLBACK func);

void msgqueue_uninit();

bool msgqueue_push(const char* topic, const void* payload, const int payloadlen);

void msgqueue_callback_set(MESSAGE_POP_CALLBACK func);

void msgqueue_on_recv(const char* topic, const void* payload, const int payloadlen);

void msgqueue_clear();

#endif
