#include "topic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static struct samanager_topic_entry *g_subscribe_topics = NULL;

struct samanager_topic_entry * samanager_topic_first()
{
	struct samanager_topic_entry *target = g_subscribe_topics;
	return target;
}

struct samanager_topic_entry * samanager_topic_last()
{
	struct samanager_topic_entry *topic = g_subscribe_topics;
	struct samanager_topic_entry *target = NULL;
	//printf("Find Last\n"); 
	while(topic != NULL)
	{
		//printf("Topic Name: %s\n", topic->name);
		target = topic;
		topic = topic->next;
	}
	return target;
}

struct samanager_topic_entry * samanager_topic_add(char const * topicname, samanager_topic_msg_cb_func_t cbfunc)
{
	struct samanager_topic_entry *topic = NULL;
	
	topic = (struct samanager_topic_entry *)malloc(sizeof(*topic));
	
	if (topic == NULL)
		return NULL;
	
	//snprintf(topic->name, strlen(topic->name), "%s", topicname);

	strncpy(topic->name, topicname, strlen(topicname)+1);
	topic->callback_func = cbfunc;
	topic->next = NULL;	
	topic->prev = NULL;	

	if(g_subscribe_topics == NULL)
	{
		g_subscribe_topics = topic;
	} else {
		struct samanager_topic_entry *lasttopic = samanager_topic_last();
		//printf("Last Topic Name: %s\n", lasttopic->name);
		lasttopic->next = topic;
		topic->prev = lasttopic;
	}
	return topic;
}

void samanager_topic_remove(char* topicname)
{
	struct samanager_topic_entry *topic = g_subscribe_topics;
	struct samanager_topic_entry *target = NULL;
	//printf("Remove Topic\n");
	while(topic != NULL)
	{
		//printf("Topic Name: %s\n", topic->name);
		if(strcmp(topic->name, topicname) == 0)
		{
			if(g_subscribe_topics == topic)
				g_subscribe_topics = topic->next;
			if(topic->prev != NULL)
				topic->prev->next = topic->next;
			if(topic->next != NULL)
				topic->next->prev = topic->prev;
			target = topic;
			break;
		}
		topic = topic->next;
	}
	if(target!=NULL)
	{
		free(target);
		target = NULL;
	}
}

struct samanager_topic_entry * samanager_topic_find(char const * topicname)
{
	struct samanager_topic_entry *topic = g_subscribe_topics;
	struct samanager_topic_entry *target = NULL;

	//printf("Find Topic\n");
	while(topic != NULL)
	{
		//printf("Topic Name: %s\n", topic->name);
		if(strcmp(topic->name, topicname) == 0)
		{
			target = topic;
			break;
		}
		topic = topic->next;
	}
	return target;
}