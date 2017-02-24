#include "SAClientLog.h"
#include "network.h"
#include "SAClient.h"
#include "mqtthelper.h"
#include "topic.h"
#include "scparser.h"
#include "smloader.h"
#include "DES.h"
#include "Base64.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "AdvPlatform.h"  //for strdup and strcasecmp wrapping
#include "msgqueue.h"

#define DEF_DES_KEY					"29B4B9C5"
#define DEF_DES_IV					"42b19631"

void* g_coreloghandle = NULL;

susiaccess_agent_conf_body_t * g_config = NULL; 
susiaccess_agent_profile_body_t * g_profile = NULL;
void *g_mosq = NULL;
pthread_t g_reconnThreadHandler = 0;
bool g_bRetry = false;
bool g_bConnected = false;
bool g_bConnectOnce = false;

SACLIENT_CONNECTED_CALLBACK g_conn_cb = NULL;
SACLIENT_LOSTCONNECT_CALLBACK g_lost_conn_cb = NULL;
SACLIENT_DISCONNECT_CALLBACK g_disconn_cb = NULL;

bool DES_BASE64Decode(char * srcBuf,char **destBuf)
{
	bool bRet = false;
	if(srcBuf == NULL || destBuf == NULL) return bRet;
	{
		char *base64Dec = NULL;
		int decLen = 0;
		int len = strlen(srcBuf);
		int iRet = Base64Decode(srcBuf, len, &base64Dec, &decLen);
		if(iRet == 0)
		{
			char plaintext[512] = {0};
			iRet = DESDecodeEx(DEF_DES_KEY, DEF_DES_IV,  base64Dec, decLen, plaintext);
			if(iRet == 0)
			{
				*destBuf = (char *)malloc(len + 1);
				memset(*destBuf, 0, len + 1);
				strcpy(*destBuf, plaintext);
				bRet = true;
			}
		}
		if(base64Dec) free(base64Dec);
	}

	return bRet;
}

bool saclient_send_agentinfo(int status)
{
	int result = mqtt_err_success;
	char topicStr[128] = {0};
	char* pAgentInfoPayload;
	bool bRet = false;
	if(!g_mosq)
		return bRet;

	if(!g_profile)
		return bRet;

	pAgentInfoPayload = scparser_agentinfo_print(g_profile, status);
	
	sprintf(topicStr, DEF_INFOACK_TOPIC, g_profile->devId);
	result = MQTT_Publish(g_mosq, topicStr, pAgentInfoPayload, strlen(pAgentInfoPayload), 0, false);
	if(result == mqtt_err_success)
	{
		SAClientLog(g_coreloghandle, Debug, "Send Topic: %s, Data: %s", topicStr, pAgentInfoPayload);
		bRet = true;
	}
	else 
		SAClientLog(g_coreloghandle, Error, "Send AgentInfo Failed, error code: %d, Packet: %s", result, pAgentInfoPayload);

	free(pAgentInfoPayload);
	return bRet;
}

bool saclient_send_osinfo()
{
	int result = mqtt_err_success;
	char topicStr[128] = {0};
	char* pOSInfoPayload;
	bool bRet = false;
	if(!g_mosq)
		return bRet;

	if(!g_profile)
		return bRet;
	pOSInfoPayload = scparser_osinfo_print(g_profile);

	sprintf(topicStr, DEF_ACTIONREQ_TOPIC,  g_profile->devId);
	result = MQTT_Publish(g_mosq, topicStr, pOSInfoPayload, strlen(pOSInfoPayload), 0, false);
	if(result == mqtt_err_success)
	{
		SAClientLog(g_coreloghandle, Debug, "Send Topic: %s, Data: %s", topicStr, pOSInfoPayload);
		 bRet = true;
	}
	else 
		SAClientLog(g_coreloghandle, Error, "Send OSInfo Failed, error code: %d, Packet: %s", result, pOSInfoPayload);

	free(pOSInfoPayload);
	return bRet;
}

void* threadonconnect(void* args)
{
	usleep(100*1000);
	saclient_getsocketaddress(g_profile->localip, sizeof(g_profile->localip));

	smloader_internal_subscribe();

	if(saclient_send_agentinfo(AGENT_STATUS_ONLINE) == true)
	{
		saclient_send_osinfo();
		smloader_connect_status_update(AGENT_STATUS_ONLINE);
	} else {
		SAClientLog(g_coreloghandle, Error, "Unable to send Client Info: AGENT_STATUS_ONLINE");
	}

	if(g_conn_cb != NULL)
		g_conn_cb();
	pthread_exit(0);
}

void saclient_on_connect_cb(int rc)
{
	pthread_t onconn = 0;
	g_bConnected = true;
	
	if(pthread_create(&onconn, NULL, threadonconnect, NULL)==0)
	{
		pthread_detach(onconn);
	}
	
}

void saclient_on_lost_connect_cb(int rc)
{
	g_bConnected = false;

	SAClientLog(g_coreloghandle, Warning, "Lost connection: %s", MQTT_GetConnAckString(rc));

	smloader_connect_status_update(AGENT_STATUS_CONNECTION_FAILED);

	if(g_lost_conn_cb != NULL)
		g_lost_conn_cb();
}

void saclient_on_disconnect_cb(int rc)
{
	g_bConnected = false;

	smloader_connect_status_update(AGENT_STATUS_OFFLINE);
	
	if(g_disconn_cb != NULL)
		g_disconn_cb();
}

void  saclient_on_recv(const char* topic, const void* payload, const int payloadlen)
{
	struct topic_entry * target = topic_find(topic);
	SAClientLog(g_coreloghandle, Debug, "Received Topic: %s, Data: %s", topic, (char *)payload);

	if(target != NULL)
	{
		susiaccess_packet_body_t pkt;

		scparser_message_parse(payload, payloadlen, &pkt);

		target->callback_func(topic, &pkt, NULL, NULL);

		free(pkt.content);
	}
}

int pw_callback(char *buf, int size, int rwflag, void *userdata)
{
	char* data = NULL;
	int length = 0;
	
	if(!buf)
		return 0;

	if(!g_config)
		return 0;

	data = g_config->cerpasswd;
	length = strlen(data);

	memset(buf, 0, size);
	if(length+1 >= size)
	{
		strncpy(buf, g_config->cerpasswd, size);
		return size;
	}
	else
	{
		strncpy(buf, g_config->cerpasswd, length+1);
		return length;
	}
}

int _connect()
{
	int iRet = saclient_false;
	int serverport = 0;
	char* pAgentInfoPayload;
	char* loginID = NULL;
	char* loginPwd = NULL;
	char* desSrc = NULL;
	char topicStr[128] = {0};
	int result = mqtt_err_success;

	if(!g_mosq)
		return iRet;

	if(!g_config)
		return iRet;

	if(DES_BASE64Decode(g_config->serverAuth, &desSrc))
	{
		loginID = strtok(desSrc, ":");
		loginPwd =  strtok(NULL, ":");
	}

	SAClientLog(g_coreloghandle, Normal, "Connecting to broker: %s", g_config->serverIP);
	serverport = atoi(g_config->serverPort);
	pAgentInfoPayload = scparser_agentinfo_print(g_profile, AGENT_STATUS_OFFLINE);
	sprintf(topicStr, DEF_WILLMSG_TOPIC, g_profile->devId);
	result = MQTT_Connect(g_mosq, g_config->serverIP, serverport, loginID, loginPwd, 30, topicStr, pAgentInfoPayload, strlen(pAgentInfoPayload));
	g_bConnectOnce = true;
	if(desSrc)
	{
		free(desSrc);
		desSrc = NULL;
		loginID = NULL;
		loginPwd = NULL;
	}
	free(pAgentInfoPayload);
	if(result != mqtt_err_success)
	{
		smloader_connect_status_update(AGENT_STATUS_CONNECTION_FAILED);

		SAClientLog(g_coreloghandle, Error, "Unable to connect to broker: %s, error code: %d.", g_config->serverIP, result);
		return iRet;
	}
	else
	{
			iRet = saclient_success;
	}
	return iRet;
}

int _reconnect()
{
	int iRet = saclient_false;
	int result = mqtt_err_success;

	if(!g_mosq)
		return iRet;

	SAClientLog(g_coreloghandle, Normal, "Reconnecting to broker: %s", g_config->serverIP);

	result = MQTT_Reconnect(g_mosq);

	if(result != mqtt_err_success)
	{
		smloader_connect_status_update(AGENT_STATUS_CONNECTION_FAILED);

		SAClientLog(g_coreloghandle, Error, "Unable to connect to broker: %s, error code: %d.", g_config->serverIP, result);
		return iRet;
	}
	else
	{
			iRet = saclient_success;
	}
	return iRet;
}

void _disconnect(bool bForce)
{
	struct topic_entry *iter_topic = NULL;
	struct topic_entry *tmp_topic = NULL;

	if(g_bConnected)
	{
		SAClientLog(g_coreloghandle, Debug, "saclient_send_agentinfo");
		saclient_send_agentinfo(AGENT_STATUS_OFFLINE);
	}

	if(g_mosq)
	{
		SAClientLog(g_coreloghandle, Debug, "MQTT_Disconnect.");
		MQTT_Disconnect(g_mosq, bForce);
	}
	
	iter_topic = topic_first();
	while(iter_topic != NULL)
	{
		tmp_topic = iter_topic->next;
		if(g_mosq)
		{
			MQTT_Unsubscribe(g_mosq, iter_topic->name);
		}
		topic_remove(iter_topic->name);
		iter_topic = tmp_topic;
	}
	
	g_bConnected = false;
}

static void* ReconnThreadStart(void* args)
{
	int count;
	int iRet = saclient_false;
CONN_RETRY:
	count = 50;
	while (count>0)
	{
		count--;
		usleep(100*1000);
		if(!g_bRetry)
		{
			pthread_exit(0);
			return 0;
		}
	}
	
	if(!g_bConnectOnce)
		iRet = _connect();
	else
		iRet = _reconnect();
	if(iRet != saclient_success)
	{
		if(g_bRetry)
		{
			goto CONN_RETRY;
		}
	}
	else
	{
		smloader_connect_status_update(AGENT_STATUS_OFFLINE);
	}
	pthread_exit(0);
	return 0;
}

int SACLIENT_API saclient_initialize(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile, void * loghandle)
{
	int iRet = saclient_false;
	int major = 0;
	int minor = 0;
	int revision = 0;

	struct mosquitto *mosq = NULL;

	if(config == NULL)
	{
		iRet = saclient_config_error;
		return iRet;
	}

	if(profile == NULL)
	{
		iRet = saclient_profile_error;
		return iRet;
	}

	if(config->tlstype == tls_type_tls)
	{
		FILE *fptr = NULL;
		if((strlen(config->cafile)==0 && strlen(config->capath)==0) || (strlen(config->certfile)>0 && strlen(config->keyfile)==0) || (strlen(config->certfile)==0 && strlen(config->keyfile)>0)) return saclient_config_error;
		if(strlen(config->certfile) > 0)
		{
			fptr = fopen(config->certfile, "r");
			if(fptr){
				fclose(fptr);
			}else{
				return saclient_config_error;
			}
		}

		if(strlen(config->cafile) > 0)
		{
			fptr = fopen(config->cafile, "r");
			if(fptr){
				fclose(fptr);
			}else{
				return saclient_config_error;
			}
		}

		if(strlen(config->keyfile) > 0)
		{
			fptr = fopen(config->keyfile, "r");
			if(fptr){
				fclose(fptr);
			}else{
				return saclient_config_error;
			}
		}
	}

	if(loghandle != NULL)
	{
		g_coreloghandle = loghandle;
		SAClientLog(g_coreloghandle, Debug, "Start logging: %s", __FUNCTION__);
	}
	else
	{
		g_coreloghandle = NULL;
	}

	if(MQTT_lib_version(&major, &minor, &revision) != 0)
	{
		SAClientLog(g_coreloghandle, Normal, "Mosquitto version: %d.%d.%d", major, minor, revision);
	}

	mosq = MQTT_Initialize(profile->devId);
	g_bConnectOnce = false;

	if(mosq)
	{
		if(config->tlstype == tls_type_tls)
		{
			int result = MQTT_SetTLS(mosq, strlen(config->cafile)>0?config->cafile:NULL, strlen(config->capath)>0?config->capath:NULL, strlen(config->certfile)>0?config->certfile:NULL, strlen(config->keyfile)>0?config->keyfile:NULL, pw_callback);
			if(result == mqtt_err_success)
				iRet = saclient_success;
			else
			{
				SAClientLog(g_coreloghandle, Error, "Mosquitto SetTLS Failed, error code: %d", result);
				iRet = saclient_config_error;
				return iRet;
			}
		}
		else if(config->tlstype == tls_type_psk)
		{
			int result = MQTT_SetTLSPSK(mosq, config->psk, config->identity, strlen(config->ciphers)>0?config->ciphers:NULL);
			if(result == mqtt_err_success)
				iRet = saclient_success;
			else
			{
				SAClientLog(g_coreloghandle, Error, "Mosquitto SetTLSPSK Failed, error code: %d", result);
				iRet = saclient_config_error;
				return iRet;
			}
		}
		
	}

	iRet = mosq!=NULL ? saclient_success : saclient_false;

	if(iRet == saclient_success)
	{
		g_mosq = mosq;
		MQTT_Callback_Set(g_mosq, saclient_on_connect_cb, saclient_on_lost_connect_cb, saclient_on_disconnect_cb, msgqueue_on_recv/*saclient_HandlRecv*/);
	}
	else
		return iRet;

	g_config = malloc(sizeof(susiaccess_agent_conf_body_t));
	memset(g_config, 0, sizeof(susiaccess_agent_conf_body_t));
	memcpy(g_config, config, sizeof(susiaccess_agent_conf_body_t));
	g_profile = malloc(sizeof(susiaccess_agent_profile_body_t));
	memset(g_profile, 0, sizeof(susiaccess_agent_profile_body_t));
	memcpy(g_profile, profile, sizeof(susiaccess_agent_profile_body_t));

	/*Init Message Queue*/
	msgqueue_init(1000, saclient_on_recv);

	/*Load SAManager*/
	smloader_init(g_config, g_profile, g_coreloghandle);
	smloader_callback_set(saclient_publish, saclient_subscribe, saclient_server_connect_ssl, saclient_disconnect);
	smloader_osinfo_send_set(saclient_send_osinfo);
	
	return iRet;
}

void SACLIENT_API  saclient_uninitialize()
{
	if(g_reconnThreadHandler)
	{
		g_bRetry = false;
		pthread_detach(g_reconnThreadHandler);
		//pthread_join(g_reconnThreadHandler, NULL);
		g_reconnThreadHandler = 0;
	}
	smloader_callback_set(NULL, NULL, NULL, NULL);

	_disconnect(false);

	msgqueue_uninit();

	/*Release SAManager*/	
	smloader_uninit();
	
	g_conn_cb = NULL;
	g_lost_conn_cb = NULL;
	g_disconn_cb = NULL;

	if(g_config != NULL)
	{
		free(g_config);
		g_config = NULL;
	}
	if(g_profile != NULL)
	{
		free(g_profile);
		g_profile = NULL;
	}
	
	if(g_mosq != NULL)
	{
		MQTT_Callback_Set(g_mosq, NULL, NULL, NULL, NULL);
		SAClientLog(g_coreloghandle, Debug, "MQTT_Uninitialize");
		MQTT_Uninitialize(g_mosq);
		g_mosq = NULL;
	}
	g_coreloghandle = NULL;
}

int SACLIENT_API saclient_reinitialize(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile)
{
	int iRet = saclient_false;
	struct mosquitto *mosq = NULL;

	/*Uninitialize*/
	if(g_reconnThreadHandler)
	{
		g_bRetry = false;
		pthread_join(g_reconnThreadHandler, NULL);
		g_reconnThreadHandler = 0;
	}

	if(g_mosq != NULL)
	{
		MQTT_Callback_Set(g_mosq, NULL, NULL, NULL, NULL);
	}

	_disconnect(true);

	/*Reset Message Queue*/
	msgqueue_clear();
	evtqueue_clear();

	if(g_mosq != NULL)
	{
		SAClientLog(g_coreloghandle, Debug, "MQTT_Uninitialize");
		MQTT_Uninitialize(g_mosq);
		g_mosq = NULL;
	}

	if(g_config == NULL)
	{
		iRet = saclient_config_error;
		return iRet;
	}

	if(g_profile == NULL)
	{
		iRet = saclient_profile_error;
		return iRet;
	}

	if(config)
	{
		memcpy(g_config, config, sizeof(susiaccess_agent_conf_body_t));
	}

	if(profile)
	{
		memcpy(g_profile, profile, sizeof(susiaccess_agent_profile_body_t));
	}

	mosq = MQTT_Initialize(g_profile->devId);
	
	g_bConnectOnce = false;

	if(mosq)
	{
		if(g_config->tlstype == tls_type_tls)
		{
			int result = MQTT_SetTLS(mosq, strlen(g_config->cafile)>0?g_config->cafile:NULL, strlen(g_config->capath)>0?g_config->capath:NULL, strlen(g_config->certfile)>0?g_config->certfile:NULL, strlen(g_config->keyfile)>0?g_config->keyfile:NULL, pw_callback);
			if(result == mqtt_err_success)
				iRet = saclient_success;
			else
			{
				SAClientLog(g_coreloghandle, Error, "Mosquitto SetTLS Failed, error code: %d", result);
				iRet = saclient_config_error;
				return iRet;
			}
		}
		else if(g_config->tlstype == tls_type_psk)
		{
			int result = MQTT_SetTLSPSK(mosq, g_config->psk, g_config->identity, strlen(g_config->ciphers)>0?g_config->ciphers:NULL);
			if(result == mqtt_err_success)
				iRet = saclient_success;
			else
			{
				SAClientLog(g_coreloghandle, Error, "Mosquitto SetTLSPSK Failed, error code: %d", result);
				iRet = saclient_config_error;
				return iRet;
			}
		}
		MQTT_Callback_Set(mosq, saclient_on_connect_cb, saclient_on_lost_connect_cb, saclient_on_disconnect_cb, msgqueue_on_recv/*saclient_HandlRecv*/);
		g_mosq = mosq;
		return saclient_success;
	}
	else
		return saclient_false;
}

int SACLIENT_API saclient_connect()
{
	int iRet = saclient_false;
	
	if(g_reconnThreadHandler)
	{
		g_bRetry = false;
		pthread_join(g_reconnThreadHandler, NULL);
		g_reconnThreadHandler = 0;
	}

	if(!g_bConnectOnce)
		iRet = _connect();
	else
		iRet = _reconnect();
	
	if(iRet == saclient_false)
	{
		if(strcasecmp(g_config->autoStart, "True")==0)
		{
			saclient_reconnect();
		}
	}
	return iRet;
}

int SACLIENT_API  saclient_server_connect(char const * ip, int port, char const * mqttauth)
{
	return saclient_server_connect_ssl(ip, port, mqttauth, tls_type_none, NULL);
}

int SACLIENT_API  saclient_server_connect_ssl(char const * ip, int port, char const * mqttauth, tls_type tlstype, char const * psk)
{
	susiaccess_agent_conf_body_t config;

	if(!g_config)
		return saclient_config_error;

	memset(&config, 0, sizeof(susiaccess_agent_conf_body_t));
	memcpy(&config, g_config, sizeof(susiaccess_agent_conf_body_t));

	if(ip)
	{
		strncpy(config.serverIP, ip, strlen(ip)+1);
	}

	if(port>0)
	{
		sprintf(config.serverPort, "%d", port);
	}

	if(mqttauth)
	{
		strncpy(config.serverAuth, mqttauth, strlen(mqttauth)+1);
	}

	config.tlstype = tlstype;

	if(psk)
	{
		strncpy(config.psk, psk, strlen(psk)+1);
	}

	return  saclient_server_connect_config(&config, NULL);
}

int SACLIENT_API  saclient_server_connect_config(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile)
{
	if(g_reconnThreadHandler)
	{
		g_bRetry = false;
		pthread_join(g_reconnThreadHandler, NULL);
		g_reconnThreadHandler = 0;
	}

	saclient_reinitialize(config, profile);
	
	return  saclient_connect();
}

void SACLIENT_API saclient_disconnect()
{
	if(g_reconnThreadHandler)
	{
		g_bRetry = false;
		pthread_join(g_reconnThreadHandler, NULL);
		g_reconnThreadHandler = 0;
	}

	_disconnect(false);
}

void SACLIENT_API saclient_reconnect()
{
	if(g_reconnThreadHandler)
	{
		g_bRetry = false;
		pthread_join(g_reconnThreadHandler, NULL);
		g_reconnThreadHandler = 0;
	}

	_disconnect(true);

	g_bRetry = true;
	pthread_create(&g_reconnThreadHandler, NULL, ReconnThreadStart, NULL);
}


void SACLIENT_API  saclient_connection_callback_set(SACLIENT_CONNECTED_CALLBACK on_connect, SACLIENT_LOSTCONNECT_CALLBACK on_lost_connect, SACLIENT_DISCONNECT_CALLBACK on_disconnect)
{
	g_conn_cb = on_connect;
	g_lost_conn_cb = on_lost_connect;
	g_disconn_cb = on_disconnect;
}

int SACLIENT_API  saclient_publish(char const * topic, int qos, int retain, susiaccess_packet_body_t const * pkt)
{
	int iRet = saclient_false;
	int result = mqtt_err_success;
	char* pPayload = NULL;
	
	if(!g_mosq)
		return saclient_no_init;

	if(!pkt)
		return saclient_send_data_error;

	if(!g_bConnected)
		return saclient_no_connnect;

	pPayload = scparser_packet_print(pkt);

	if(!pPayload)
	{
		SAClientLog(g_coreloghandle, Error, "Incorrect Packet Format: <name: %s, command: %d, content: %s>", pkt->handlerName, pkt->cmd, pkt->content);
		return saclient_send_data_error;
	}
	result = MQTT_Publish(g_mosq, (char *)topic, pPayload, strlen(pPayload), qos, retain==1?true:false);
	if(result == mqtt_err_success)
	{
		iRet = saclient_success;
		SAClientLog(g_coreloghandle, Debug, "Send Topic: %s, Data: %s", topic, pPayload);
	}
	else {
		SAClientLog(g_coreloghandle, Error, "Send Failed, error code: %d, Packet: <name: %s, command: %d, content: %s>", result, pkt->handlerName, pkt->cmd, pkt->content);
	}
	free(pPayload);
	return iRet;
}

int SACLIENT_API  saclient_subscribe(char const * topic, int qos, SACLIENT_MESSAGE_RECV_CALLBACK msg_recv_callback)
{
	int iRet = saclient_false;
	int result = mqtt_err_success;

	if(!g_mosq)
		return saclient_no_init;

	if(!msg_recv_callback)
		return saclient_callback_null;

	result = MQTT_Subscribe(g_mosq, (char *)topic, qos);
	if(result == mqtt_err_success)
	{
		if(topic_find(topic)!=NULL)
		{
			topic_remove(topic);
		}
		topic_add(topic, (TOPIC_MESSAGE_CB)msg_recv_callback);
		iRet = saclient_success;
	}
	else
	{
		SAClientLog(g_coreloghandle, Error, "Mosquitto Subscribe Failed, error code: %d", result);
	}

	
	return iRet;
}

int SACLIENT_API  saclient_getsocketaddress(char* clientip, int size)
{
	int iRet = 0;
	int newfd = MQTT_GetSocket(g_mosq);
	if(newfd<0)
		return saclient_false;
	iRet =network_local_ip_get(newfd, clientip, size);
	return iRet==0?saclient_success:saclient_false;
}
