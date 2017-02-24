#include "triggerqueue.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "basequeue.h"




struct triggerdata {
	bool isNormal;
	char sensorname[256];
	double value;
	MSG_ATTRIBUTE_T* attr;
	void *pRev;
};

struct trigger_ctx{
   void*			threadHandler;
   bool				isThreadRunning;
   struct triggerdata		*triggerqueue;
};

static struct trigger_ctx g_triggerthreadctx;
THRESHOLD_ON_TRIGGER g_on_triggered_cb = NULL;

bool trigger_create(struct triggerdata *const trigger, bool isNormal, char* sensorname, double value, MSG_ATTRIBUTE_T* attr, void *pRev)
{
	if(!trigger)
		return false;

	trigger->isNormal = isNormal;
	strcpy(trigger->sensorname, sensorname);
	trigger->value = value;
	trigger->attr = attr;
	trigger->pRev = pRev;

	return true;
}

void trigger_free(struct triggerdata *const trigger)
{
	if(!trigger)
		return;

	free(trigger);
}

void* threat_trigger_queue(void* args)
{
	struct trigger_ctx *precvContex = (struct trigger_ctx *)args;
	unsigned long interval = 100*1000; //microsecond.
	while(precvContex->isThreadRunning)
	{
		struct triggerdata *trigger = NULL;
		usleep(interval);
		trigger = (struct triggerdata *)queue_get(precvContex->triggerqueue);
		if(!trigger)
			continue;
		if(g_on_triggered_cb!= NULL)
		{
			g_on_triggered_cb(trigger->isNormal, trigger->sensorname, trigger->value, trigger->attr, trigger->pRev);
		}
		trigger_free(trigger);
		trigger = NULL;
	}

	pthread_exit(0);
	return 0;
}

bool triggerqueue_init(const unsigned int slots, THRESHOLD_ON_TRIGGER func)
{
	memset(&g_triggerthreadctx, 0, sizeof(struct trigger_ctx));
	g_triggerthreadctx.triggerqueue = malloc(sizeof( struct queue));
	g_on_triggered_cb = NULL;
	if(g_triggerthreadctx.triggerqueue)
	{
		if(!queue_init(g_triggerthreadctx.triggerqueue, slots, sizeof(struct triggerdata)))
		{
			free(g_triggerthreadctx.triggerqueue);
			g_triggerthreadctx.triggerqueue = NULL;			
		}
		else
		{
			g_on_triggered_cb = func;
			g_triggerthreadctx.isThreadRunning = true;
			if (pthread_create(&g_triggerthreadctx.threadHandler, NULL, threat_trigger_queue, &g_triggerthreadctx) != 0)
			{
				g_triggerthreadctx.isThreadRunning = false;
				queue_uninit(g_triggerthreadctx.triggerqueue, trigger_free);
				free(g_triggerthreadctx.triggerqueue);
				g_triggerthreadctx.triggerqueue = NULL;
			}
			else
			{
				pthread_detach(g_triggerthreadctx.threadHandler);
				g_triggerthreadctx.threadHandler = 0;
				return true;
			}
		}
	}
	return false;
}

void triggerqueue_uninit()
{
	g_on_triggered_cb = NULL;

	if(g_triggerthreadctx.isThreadRunning == true)
	{
		g_triggerthreadctx.isThreadRunning = false;
		//pthread_join(g_triggerthreadctx.threadHandler, NULL);
		//g_triggerthreadctx.threadHandler = 0;

		usleep(500*1000);
	}

	if(g_triggerthreadctx.triggerqueue)
	{
		queue_uninit(g_triggerthreadctx.triggerqueue, trigger_free);
		free(g_triggerthreadctx.triggerqueue);
		g_triggerthreadctx.triggerqueue = NULL;
	}
}

bool triggerqueue_push(struct thr_item_info_t* item, MSG_ATTRIBUTE_T* attr)
{
	if(item == NULL) return false;
	if(g_triggerthreadctx.triggerqueue)
	{
		struct triggerdata *newtrigger = malloc(sizeof(struct triggerdata));
		trigger_create(newtrigger, item->isNormal, item->pathname, item->checkRetValue, attr, NULL);
		if(!queue_put(g_triggerthreadctx.triggerqueue, newtrigger))
		{
			trigger_free(newtrigger);
			newtrigger = NULL;
		}
		else
			return true;
	}
	return false;
}

void triggerqueue_clear()
{
	if(g_triggerthreadctx.triggerqueue)
	{
		queue_clear(g_triggerthreadctx.triggerqueue, trigger_free);
	}
}