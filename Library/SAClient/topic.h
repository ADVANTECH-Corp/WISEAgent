#ifndef _CAGENT_TOPIC_H_
#define _CAGENT_TOPIC_H_

typedef int (*TOPIC_MESSAGE_CB)(char* topic, void *payload, void *pRev1, void* pRev2);

typedef struct topic_entry
{    
	char name[128];
	TOPIC_MESSAGE_CB callback_func;
	struct topic_entry *prev;
	struct topic_entry *next;
} topic_entry_st;

struct topic_entry * topic_first();
struct topic_entry * topic_last();
struct topic_entry * topic_add(char const * topicname, TOPIC_MESSAGE_CB cbfunc);
void topic_remove(char* topicname);
struct topic_entry * topic_find(char const * topicname);

#endif