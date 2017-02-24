#include "basequeue.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

bool queue_init(struct queue *const q, const unsigned int slots, const unsigned int datasize)
{
    if (!q || slots < 1U)
        return false;

    q->queue = malloc(datasize * (size_t)(slots + 1));
    if (!q->queue)
        return false;

    q->size = slots + 1U; 
    q->head = 0U;
    q->tail = 0U;
    if(pthread_mutex_init(&q->lock, NULL)<0)
	{
		free(q->queue);
		q->queue = NULL;
		return false;
	}
	if(pthread_cond_init(&q->wait_room, NULL)<0)
	{
		free(q->queue);
		q->queue = NULL;
		return false;
	}
	if(pthread_cond_init(&q->wait_data, NULL)<0)
	{
		free(q->queue);
		q->queue = NULL;
		return false;
	}
    return true;
}

void queue_uninit(struct queue *const q, void(*freefn)(void*))
{
	void *data;
	pthread_mutex_lock(&q->lock);
	while (q->head != q->tail)
	{
		data = q->queue[q->tail];
		q->queue[q->tail] = NULL;
		q->tail = (q->tail + 1U) % q->size;
		if(freefn)
			freefn(data);
		data = NULL;
	}
	free(q->queue);
	q->queue = NULL;
	q->size = 0U;
    q->head = 0U;
    q->tail = 0U;
	
    pthread_mutex_unlock(&q->lock);
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->wait_room);
    pthread_cond_destroy(&q->wait_data);
}

void *queue_get(struct queue *const q)
{
	void *data = NULL;
	struct timespec time;
	struct timeval tv;
	int ret = 0;
	if(!q)
		return data;
	gettimeofday(&tv, NULL);
	time.tv_sec = tv.tv_sec;
	time.tv_nsec = tv.tv_usec * 1000 + 500 * 100000;
	
    pthread_mutex_lock(&q->lock);
    while (q->head == q->tail)
	{
        ret = pthread_cond_timedwait(&q->wait_data, &q->lock, &time);
		if(ret != 0)
		{
			pthread_mutex_unlock(&q->lock);
			return data;
		}
	}

    data = q->queue[q->tail];
    q->queue[q->tail] = NULL;
    q->tail = (q->tail + 1U) % q->size;

    pthread_cond_signal(&q->wait_room);

    pthread_mutex_unlock(&q->lock);
    return data;
}

bool queue_put(struct queue *const q, void *const data)
{
	struct timespec time;
	struct timeval tv;
	int ret = 0;
	if(!q)
		return false;
	gettimeofday(&tv, NULL);
	time.tv_sec = tv.tv_sec;
	time.tv_nsec = tv.tv_usec * 1000 + 500 * 100000;
    pthread_mutex_lock(&q->lock);
	while ((q->head + 1U) % q->size == q->tail)
	{
        ret = pthread_cond_timedwait(&q->wait_room, &q->lock, &time);
		if(ret != 0)
		{
			pthread_mutex_unlock(&q->lock);
			return false;
		}
	}

    q->queue[q->head] = data;
	
	q->head = (q->head + 1U) % q->size;

    pthread_cond_signal(&q->wait_data);

    pthread_mutex_unlock(&q->lock);
    return true;
}

bool queue_clear(struct queue *const q, void(*freefn)(void*))
{
	void *data = NULL;
	if(!q)
		return true;

	pthread_mutex_lock(&q->lock);
	while (q->head != q->tail)
	{
		data = q->queue[q->tail];
		q->queue[q->tail] = NULL;
		q->tail = (q->tail + 1U) % q->size;
		if(freefn)
			freefn(data);
		data = NULL;
	}
	
	pthread_cond_signal(&q->wait_room);
    pthread_mutex_unlock(&q->lock);

    return true;
}