#include "topic.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static struct topic_entry *g_subscribe_topics = NULL;
struct topic_entry * topic_first()
{
	struct topic_entry *target = g_subscribe_topics;
	return target;
}

struct topic_entry * topic_last()
{
	struct topic_entry *topic = g_subscribe_topics;
	struct topic_entry *target = NULL;
	while(topic != NULL)
	{
		target = topic;
		topic = topic->next;
	}
	return target;
}

struct topic_entry * topic_add(char const * topicname, TOPIC_MESSAGE_CB cbfunc)
{
	struct topic_entry *topic = NULL;
	
	topic = (struct topic_entry *)malloc(sizeof(*topic));
	
	if (topic == NULL)
		return NULL;

	strncpy(topic->name, topicname, strlen(topicname)+1);
	topic->callback_func = cbfunc;
	topic->next = NULL;	
	topic->prev = NULL;	

	if(g_subscribe_topics == NULL)
	{
		g_subscribe_topics = topic;
	} else {
		struct topic_entry *lasttopic = topic_last();
		lasttopic->next = topic;
		topic->prev = lasttopic;
	}
	return topic;
}

void topic_remove(char* topicname)
{
	struct topic_entry *topic = g_subscribe_topics;
	struct topic_entry *target = NULL;
	while(topic != NULL)
	{
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

struct topic_entry * topic_find(char const * topicname)
{
	struct topic_entry *topic = g_subscribe_topics;
	struct topic_entry *target = NULL;

	while(topic != NULL)
	{
		if(strchr(topic->name, '+')>0)
		{
			if(strchr(topicname, '+')>0)
			{
				if(strcmp(topic->name, topicname) == 0)
				{
					target = topic;
					break;
				}
			}
			else
			{
				char *delim = "+";
				char *p = NULL;
				char tName[128] = {0};
				bool match = true;
				char* ss = (char *)topicname;
				strcpy(tName, topic->name);
				p = strtok(tName, delim);
				while(p)
				{
					ss = strstr(ss, p);
					if(ss > 0)
						ss += strlen(p);
					else
					{
						match = false;
						break;
					}
					p=strtok(NULL,delim);
				}
				if(match)
					target = topic;
			}
		}
		else
		{
			if(strcmp(topic->name, topicname) == 0)
			{
				target = topic;
				break;
			}
		}
		topic = topic->next;
	}
	return target;
}