#ifndef _BASEQUEUE_H_
#define _BASEQUEUE_H_
#include <pthread.h>
#include <stdbool.h>

struct queue {
    pthread_mutex_t		lock;
    pthread_cond_t		wait_room;
    pthread_cond_t		wait_data;
    unsigned int		size;
    unsigned int		head;
    unsigned int		tail;
    void				**queue;
};

bool queue_init(struct queue *const q, const unsigned int slots, const unsigned int datasize);

void queue_uninit(struct queue *const q, void(*freefn)(void*));

void *queue_get(struct queue *const q);

bool queue_put(struct queue *const q, void *const data);

bool queue_clear(struct queue *const q, void(*freefn)(void*));

#endif