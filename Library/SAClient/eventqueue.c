#include "eventqueue.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "basequeue.h"


struct evtdata {
	int status;
};

struct recv_ctx{
   void*			threadHandler;
   bool				isThreadRunning;
   struct queue		*evtqueue;
};

static struct recv_ctx g_recvthreadctx;
EVENT_UPDATECONNECTSTATE g_on_event_func = NULL;

bool event_create(struct evtdata *const evt, int status)
{
	if(!evt)
		return false;

	evt->status = status;

	return true;
}

void event_free(struct evtdata *const evt)
{
	if(!evt)
		return;

	free(evt);
}

void* threat_event_queue(void* args)
{
	struct recv_ctx *precvContex = (struct recv_ctx *)args;
	unsigned long interval = 100*1000; //microsecond.
	while(precvContex->isThreadRunning)
	{
		struct evtdata *evt = NULL;
		usleep(interval);
		evt = (struct evtdata *)queue_get(precvContex->evtqueue);
		if(!evt)
			continue;
		if(g_on_event_func!= NULL)
		{
			g_on_event_func(evt->status);
		}
		event_free(evt);
		evt = NULL;
	}

	pthread_exit(0);
	return 0;
}

bool evtqueue_init(const unsigned int slots, EVENT_UPDATECONNECTSTATE func)
{
	memset(&g_recvthreadctx, 0, sizeof(struct recv_ctx));
	g_recvthreadctx.evtqueue = malloc(sizeof( struct queue));
	if(g_recvthreadctx.evtqueue)
	{
		if(!queue_init(g_recvthreadctx.evtqueue, slots, sizeof(struct evtdata)))
		{
			free(g_recvthreadctx.evtqueue);
			g_recvthreadctx.evtqueue = NULL;			
		}
		else
		{
			g_recvthreadctx.isThreadRunning = true;
			g_on_event_func = func;
			if (pthread_create(&g_recvthreadctx.threadHandler, NULL, threat_event_queue, &g_recvthreadctx) != 0)
			{
				g_on_event_func = NULL;
				g_recvthreadctx.isThreadRunning = false;
				queue_uninit(g_recvthreadctx.evtqueue, event_free);
				free(g_recvthreadctx.evtqueue);
				g_recvthreadctx.evtqueue = NULL;
			}
			else
				return true;
		}
	}
	return false;
}

void evtqueue_uninit()
{
	g_on_event_func = NULL;

	if(g_recvthreadctx.isThreadRunning == true)
	{
		g_recvthreadctx.isThreadRunning = false;
		pthread_join(g_recvthreadctx.threadHandler, NULL);
		g_recvthreadctx.threadHandler = 0;

		usleep(500*1000);
	}

	if(g_recvthreadctx.evtqueue)
	{
		queue_uninit(g_recvthreadctx.evtqueue, event_free);
		free(g_recvthreadctx.evtqueue);
		g_recvthreadctx.evtqueue = NULL;
	}
}

bool evtqueue_push(int status)
{
	if(g_recvthreadctx.evtqueue)
	{
		struct evtdata *newevt = malloc(sizeof(struct evtdata));
		event_create(newevt, status);
		if(!queue_put(g_recvthreadctx.evtqueue, newevt))
		{
			event_free(newevt);
			newevt = NULL;
		}
		else
			return true;
	}
	return false;
}

void evtqueue_clear()
{
	if(g_recvthreadctx.evtqueue)
	{
		queue_clear(g_recvthreadctx.evtqueue, event_free);
	}
}

