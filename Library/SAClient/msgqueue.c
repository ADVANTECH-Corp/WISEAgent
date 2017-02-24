#include "msgqueue.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "basequeue.h"
#include <string.h>

struct message {
	char* topic;
	void* payload;
	int pauloadlen;
};

struct recv_ctx{
   void*			threadHandler;
   bool				isThreadRunning;
   struct queue		*msgqueue;
};

static struct recv_ctx g_recvthreadctx;
MESSAGE_POP_CALLBACK g_on_message_func = NULL;

bool message_create(struct message* msg, const char* topic, const void* payload, const int payloadlen)
{
	if(!msg)
		return false;
	
	if(!topic || !payload)
		return false;

	msg->topic = strdup(topic);

	msg->payload = malloc(payloadlen + 1);
    if (!msg->payload)
        return false;

	msg->pauloadlen = payloadlen;

	memset(msg->payload, 0, payloadlen + 1);
	memcpy(msg->payload, payload, payloadlen);
	return true;
}

void message_free(struct message *const msg)
{
	if(!msg)
		return;
	
	if (msg->topic)
        free(msg->topic);
	msg->topic = NULL;

	if (msg->payload)
		free(msg->payload);
	msg->payload = NULL;

	msg->pauloadlen = 0;
	
	free(msg);
}

void* threat_message_queue(void* args)
{
	struct recv_ctx *precvContex = (struct recv_ctx *)args;
	unsigned long interval = 100*1000; //microsecond.
	while(precvContex->isThreadRunning)
	{
		struct message *msg = NULL;
		usleep(interval);
		msg = (struct message *)queue_get(precvContex->msgqueue);
		if(!msg)
			continue;
		if(g_on_message_func != NULL)
		{
			g_on_message_func(msg->topic, msg->payload, msg->pauloadlen);
		}
		message_free(msg);
		msg = NULL;
	}

	pthread_exit(0);
	return 0;
}

bool msgqueue_init(const unsigned int slots, MESSAGE_POP_CALLBACK func)
{
	memset(&g_recvthreadctx, 0, sizeof(struct recv_ctx));
	g_recvthreadctx.msgqueue = malloc(sizeof( struct queue));
	if(g_recvthreadctx.msgqueue)
	{
		if(!queue_init(g_recvthreadctx.msgqueue, slots, sizeof(struct message)))
		{
			free(g_recvthreadctx.msgqueue);
			g_recvthreadctx.msgqueue = NULL;			
		}
		else
		{
			g_on_message_func = func;
			g_recvthreadctx.isThreadRunning = true;
			if (pthread_create(&g_recvthreadctx.threadHandler, NULL, threat_message_queue, &g_recvthreadctx) != 0)
			{
				g_recvthreadctx.isThreadRunning = false;
				queue_uninit(g_recvthreadctx.msgqueue, message_free);
				free(g_recvthreadctx.msgqueue);
				g_recvthreadctx.msgqueue = NULL;
			}
			else
				return true;
		}
	}
	return false;
}

void msgqueue_uninit()
{
	if(g_recvthreadctx.isThreadRunning == true)
	{
		g_recvthreadctx.isThreadRunning = false;
		pthread_join(g_recvthreadctx.threadHandler, NULL);
		g_recvthreadctx.threadHandler = 0;

		usleep(500*1000);
	}
	g_on_message_func = NULL;
	if(g_recvthreadctx.msgqueue)
	{
		queue_uninit(g_recvthreadctx.msgqueue, message_free);
		free(g_recvthreadctx.msgqueue);
		g_recvthreadctx.msgqueue = NULL;
	}
}

bool msgqueue_push(const char* topic, const void* payload, const int payloadlen)
{
	if(g_recvthreadctx.msgqueue)
	{
		struct message *newmsg = malloc(sizeof(struct message));
		message_create(newmsg, topic, payload, payloadlen);
		if(!queue_put(g_recvthreadctx.msgqueue, newmsg))
		{
			message_free(newmsg);
			newmsg = NULL;
		}
		else
			return true;
	}
	return false;
}

void msgqueue_clear()
{
	if(g_recvthreadctx.msgqueue)
	{
		queue_clear(g_recvthreadctx.msgqueue, message_free);
	}
}

void msgqueue_callback_set(MESSAGE_POP_CALLBACK func)
{
	g_on_message_func = func;
}

void msgqueue_on_recv(const char* topic, const void* payload, const int payloadlen)
{
	if(g_recvthreadctx.msgqueue)
	{
		struct message *newmsg = malloc(sizeof(struct message));
		message_create(newmsg, topic, payload, payloadlen);
		if(!queue_put(g_recvthreadctx.msgqueue, newmsg))
		{
			message_free(newmsg);
			newmsg = NULL;
		}
	}
}

