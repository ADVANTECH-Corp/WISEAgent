#include "mqtthelper.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mosquitto.h>
#include <time.h>
#include <sys/time.h>
struct mqttmsg
{
	int mid;
	pthread_mutex_t waitlock;
	pthread_cond_t waitresponse;
	struct mqttmsg* next;
};

struct msg_queue {
    struct mqttmsg* root;
	int size;
};

MQTT_CONNECT_CALLBACK on_mqtt_connect_cb = NULL;
MQTT_LOSTCONNECT_CALLBACK on_mqtt_lostconnect_cb = NULL;
MQTT_DISCONNECT_CALLBACK on_mqtt_disconnect_cb = NULL;
MQTT_MESSAGE_CALLBACK on_mqtt_message_func = NULL;

pthread_mutex_t g_publishmutex;
struct msg_queue g_msg_queue;
bool g_bConnected = false;

struct mqttmsg* MQTT_LastMsgQueue()
{
	struct mqttmsg* msg = g_msg_queue.root;
	struct mqttmsg* target = NULL;
	if(msg == NULL)
		return target;
	else
		target = msg;

	while(target->next)
	{
		target = target->next;
	}
	return target;	
}

void MQTT_InitMsgQueue()
{
	memset(&g_msg_queue, 0, sizeof(struct msg_queue));
}

void MQTT_UninitMsgQueue()
{
	struct mqttmsg* msg = g_msg_queue.root;
	struct mqttmsg* target = NULL;
	while(msg)
	{
		target = msg;
		msg = msg->next;
		pthread_mutex_unlock(&target->waitlock);
		pthread_cond_signal(&target->waitresponse);
		pthread_mutex_destroy(&target->waitlock);
		pthread_cond_destroy(&target->waitresponse);
		free(target);
		g_msg_queue.size--;
	}
}

struct mqttmsg* MQTT_AddMsgQueue(int mid)
{
	struct mqttmsg* msg = NULL;
	struct mqttmsg* newmsg = malloc(sizeof(struct mqttmsg));
	memset(newmsg, 0, sizeof(struct mqttmsg));
	newmsg->mid = mid;
	pthread_mutex_init(&newmsg->waitlock, NULL);
	pthread_cond_init(&newmsg->waitresponse, NULL);

	msg = MQTT_LastMsgQueue();
	if(msg==NULL)
	{
		g_msg_queue.size = 1;
		g_msg_queue.root = newmsg;
	}
	else
	{
		g_msg_queue.size++;
		msg->next = newmsg;
	}
	return newmsg;
}

struct mqttmsg* MQTT_FindMsgQueue(int mid)
{
	struct mqttmsg* msg = g_msg_queue.root;
	struct mqttmsg* target = NULL;

	while(msg)
	{
		if(msg->mid == mid)
		{
			target = msg;
			break;
		}
		msg = msg->next;
	}
	return target;
}

void MQTT_FreeMsgQueue(int mid)
{
	struct mqttmsg* msg = g_msg_queue.root;
	struct mqttmsg* target = NULL;
	if(msg == NULL)
		return;
	if(msg->mid == mid)
	{
		g_msg_queue.root = msg->next;
		g_msg_queue.size--;
		target = msg;
		goto FREE_MSG;
	}

	while(msg->next)
	{
		if(msg->next->mid == mid)
		{
			target = msg->next;
			msg->next = target->next;
			g_msg_queue.size--;
			break;
		}
		msg = msg->next;
	}
FREE_MSG:
	if(target == NULL) return;
	pthread_mutex_destroy(&msg->waitlock);
	pthread_cond_destroy(&msg->waitresponse);
	free(msg);
}

void MQTT_connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	//printf("%s\n",mosquitto_connack_string(rc));
	if(rc == mqtt_err_success)
	{
		g_bConnected = true;
		if(on_mqtt_connect_cb != NULL)
			on_mqtt_connect_cb(rc);
	}
	else
	{	
		g_bConnected = false;
		if(on_mqtt_lostconnect_cb != NULL)
				on_mqtt_lostconnect_cb(rc);
	}
}

void MQTT_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	//printf("%s\n",mosquitto_connack_string(rc));
	g_bConnected = false;
	if(rc == mqtt_err_success)
	{
		if(on_mqtt_disconnect_cb != NULL)
			on_mqtt_disconnect_cb(rc);
	}
	else
	{
		if(on_mqtt_lostconnect_cb != NULL)
			on_mqtt_lostconnect_cb(rc);
	}
}

void MQTT_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{	
	if(on_mqtt_message_func != NULL)
		on_mqtt_message_func(msg->topic, msg->payload, msg->payloadlen);
}

void MQTT_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{	
	struct mqttmsg* msg = MQTT_FindMsgQueue(mid);
	if(msg == NULL) return;
	pthread_cond_signal(&msg->waitresponse);
}

int MQTT_lib_version(int *major, int *minor, int *revision)
{
	return mosquitto_lib_version(major, minor, revision);
}

void * MQTT_Initialize(char const * devid)
{
	struct mosquitto *mosq = NULL;
	g_bConnected = false;
	if(pthread_mutex_init(&g_publishmutex, NULL)!=0)
	{
		return mosq;
	}

	mosquitto_lib_init();
	mosq = mosquitto_new(devid, true, NULL);
	if (!mosq)
		return mosq;
	mosquitto_connect_callback_set(mosq, MQTT_connect_callback);
	mosquitto_disconnect_callback_set(mosq, MQTT_disconnect_callback);
	mosquitto_message_callback_set(mosq, MQTT_message_callback);

	
	mosquitto_publish_callback_set(mosq, MQTT_publish_callback);

	return mosq;
}

void MQTT_Uninitialize(void *mosq)
{
	pthread_mutex_destroy(&g_publishmutex);
	g_bConnected = false;
	on_mqtt_connect_cb = NULL;
	on_mqtt_lostconnect_cb = NULL;
	on_mqtt_disconnect_cb = NULL;
	on_mqtt_message_func = NULL;

	if(mosq == NULL)
		return;

	mosquitto_connect_callback_set(mosq, NULL);
	mosquitto_disconnect_callback_set(mosq, NULL);
	mosquitto_message_callback_set(mosq, NULL);

	mosquitto_publish_callback_set(mosq, NULL);
	MQTT_UninitMsgQueue();

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

void MQTT_Callback_Set(void *mosq, MQTT_CONNECT_CALLBACK connect_cb, MQTT_LOSTCONNECT_CALLBACK lostconnect_cb, MQTT_DISCONNECT_CALLBACK disconnect_cb, MQTT_MESSAGE_CALLBACK message_cb)
{
	if(mosq == NULL)
		return;
	on_mqtt_connect_cb = connect_cb;
	on_mqtt_lostconnect_cb = lostconnect_cb;
	on_mqtt_disconnect_cb = disconnect_cb;
	on_mqtt_message_func = message_cb;
}

int MQTT_SetTLS(void *mosq, const char *cafile, const char *capath, const char *certfile, const char *keyfile, int (*pw_callback)(char *buf, int size, int rwflag, void *userdata))
{
	int result = MOSQ_ERR_SUCCESS;
	if(mosq == NULL)
		return MOSQ_ERR_INVAL;
	mosquitto_tls_insecure_set(mosq, true);
	result = mosquitto_tls_set(mosq, cafile, capath, certfile, keyfile, pw_callback);
	return result;
}

int MQTT_SetTLSPSK(void *mosq, const char *psk, const char *identity, const char *ciphers)
{
	int result = MOSQ_ERR_SUCCESS;
	if(mosq == NULL)
		return MOSQ_ERR_INVAL;
	result = mosquitto_tls_psk_set(mosq, psk, identity, ciphers) ;
	return result;
}

int MQTT_Connect(void *mosq, char const * ip, int port, char const * username, char const * password, int keepalive, char* willtopic, const void *willmsg, int msglen )
{
	int result = MOSQ_ERR_SUCCESS;
	if(mosq == NULL)
		return MOSQ_ERR_INVAL;

	MQTT_UninitMsgQueue();
	MQTT_InitMsgQueue();

	if( username!= NULL && password != NULL)
		mosquitto_username_pw_set(mosq,username,password);

	mosquitto_will_clear(mosq);

	if(willmsg != NULL) {
		mosquitto_will_set(mosq, willtopic, msglen, willmsg, 2, false);
	}
	result = mosquitto_connect(mosq, ip, port, keepalive);
	if(result == MOSQ_ERR_SUCCESS)
	{
		mosquitto_loop_start(mosq);
	}
	return result;
}

void MQTT_Disconnect(void *mosq, bool bForce)
{
	if(mosq == NULL)
		return;
	g_bConnected = false;
	MQTT_UninitMsgQueue();

	if(!bForce)
		mosquitto_loop_write(mosq, 1);

	if(mosquitto_disconnect(mosq) == MOSQ_ERR_SUCCESS)
	{
		printf("MQTT Disconnected\n");
		if(!bForce)
			mosquitto_loop_write(mosq, 1);
	}

	mosquitto_loop_stop(mosq, false);	
}

int MQTT_Reconnect(void *mosq)
{
	int result = MOSQ_ERR_SUCCESS;

	if(mosq == NULL)
		return MOSQ_ERR_INVAL;
	result = mosquitto_reconnect(mosq);
	if(result == MOSQ_ERR_SUCCESS)
	{
		mosquitto_loop_start(mosq);
	}
	return result;
}

int MQTT_Publish(void *mosq,  char* topic, const void *msg, int msglen, int qos, bool retain)
{
	struct mqttmsg* mqttmsg = NULL;
	int mid = 0;
	int result = MOSQ_ERR_SUCCESS;
	if(mosq == NULL)
		return MOSQ_ERR_INVAL;
	if(!g_bConnected)
		return MOSQ_ERR_NO_CONN;
	pthread_mutex_lock(&g_publishmutex);
	result = mosquitto_publish(mosq, &mid, topic, msglen, msg, qos, retain);
	if(result == MOSQ_ERR_SUCCESS)
	{
		if(qos>0)
			mqttmsg = MQTT_AddMsgQueue(mid);
	}
	
	if(mqttmsg)
	{
		struct timespec time;
		struct timeval tv;
		int ret = 0;
		gettimeofday(&tv, NULL);
		time.tv_sec = tv.tv_sec + 3;
		time.tv_nsec = tv.tv_usec;
		ret = pthread_cond_timedwait(&mqttmsg->waitresponse, &mqttmsg->waitlock, &time);
		if(ret != 0)
		{
			printf("Publish wait mid:%d timeout", mid);
			result = MOSQ_ERR_INVAL;
		}
		MQTT_FreeMsgQueue(mid);
	}
	pthread_mutex_unlock(&g_publishmutex);
	return result;
}

int MQTT_Subscribe(void *mosq,  char* topic, int qos)
{
	int result = MOSQ_ERR_SUCCESS;
	if(mosq == NULL)
		return MOSQ_ERR_INVAL;
	result = mosquitto_subscribe(mosq, NULL, topic, qos);
	return result;
}

void MQTT_Unsubscribe(void *mosq,  char* topic)
{
	if(mosq == NULL)
		return;

	mosquitto_unsubscribe(mosq, NULL, topic);
}

int MQTT_GetSocket(void *mosq)
{
	if(mosq == NULL)
		return -1;
	return mosquitto_socket(mosq);
}

const char* MQTT_GetErrorString(int rc)
{
	return mosquitto_strerror(rc);
}

const char* MQTT_GetConnAckString(int rc)
{
	return mosquitto_connack_string(rc);
}
