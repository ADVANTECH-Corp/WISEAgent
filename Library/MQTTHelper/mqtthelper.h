#ifndef _CAGENT_MQTTHELPER_H_
#define _CAGENT_MQTTHELPER_H_
#include <stdbool.h>
#define DEF_WILLMSG_TOPIC        "/cagent/admin/%s/willmessage"	/*publish*/
#define DEF_INFOACK_TOPIC		"/cagent/admin/%s/agentinfoack"	/*publish*/
#define DEF_AUTOREPORT_TOPIC	"/cagent/admin/%s/%s"	/*publish*/
#define DEF_AGENTCONTROL_TOPIC	"/server/admin/+/agentctrl"	/*Subscribe*/

#define DEF_CALLBACKACK_TOPIC		"/cagent/admin/%s/agentcallbackack"	/*publish*/
#define DEF_CALLBACKREQ_TOPIC		"/cagent/admin/%s/agentcallbackreq"	/*Subscribe*/

#define DEF_ACTIONACK_TOPIC		"/cagent/admin/%s/agentactionack"	/*Subscribe*/
#define DEF_ACTIONREQ_TOPIC		"/cagent/admin/%s/agentactionreq"	/*publish*/

#define DEF_CUSTOM_CALLBACKACK_TOPIC		"/cagent/%s/%s/agentcallbackack"	/*publish*/
#define DEF_CUSTOM_CALLBACKREQ_TOPIC		"/cagent/%s/%s/agentcallbackreq"	/*Subscribe*/

#define DEF_CUSTOM_ACTIONACK_TOPIC		"/cagent/%s/%s/agentactionack"	/*Subscribe*/
#define DEF_CUSTOM_ACTIONREQ_TOPIC		"/cagent/%s/%s/agentactionreq"	/*publish*/

typedef enum mqtt_connection_result{
	mqtt_success = 0,			/** * 0 - success*/
	mqtt_unaccept_protocol,		/** * 1 - connection refused (unacceptable protocol version)*/
	mqtt_reject_identifier,		/** * 2 - connection refused (identifier rejected)*/
	mqtt_unavailable_broker,	/** * 3 - connection refused (broker unavailable)*/
}conn_connection_result_enum;

/* Error values */
enum mqtt_err_t {
	mqtt_err_conn_pending = -1,
	mqtt_err_success = 0,
	mqtt_err_nomem = 1,
	mqtt_err_protocol = 2,
	mqtt_err_inval = 3,
	mqtt_err_no_conn = 4,
	mqtt_err_conn_refused = 5,
	mqtt_err_not_found = 6,
	mqtt_err_conn_lost = 7,
	mqtt_err_tls = 8,
	mqtt_err_payload_size = 9,
	mqtt_err_not_supported = 10,
	mqtt_err_auth = 11,
	mqtt_err_acl_denied = 12,
	mqtt_err_unknow = 13,
	mqtt_err_errno = 14,
	mqtt_err_eai = 15
};

typedef void (*MQTT_CONNECT_CALLBACK)(int rc);
typedef void (*MQTT_LOSTCONNECT_CALLBACK)(int rc);
typedef void (*MQTT_DISCONNECT_CALLBACK)(int rc);
typedef void (*MQTT_MESSAGE_CALLBACK)(const char* topic, const void* payload, const int payloadlen);

typedef struct{
  void * mosq;
  struct topic_entry *subscribe_topics;
  MQTT_CONNECT_CALLBACK on_connect;
  MQTT_DISCONNECT_CALLBACK on_disconnect;
}cagent_callback_body_t;

#ifdef __cplusplus
extern "C" {
#endif

int MQTT_lib_version(int *major, int *minor, int *revision);
void * MQTT_Initialize(char const * devid);
void MQTT_Uninitialize(void *mosq);
void MQTT_Callback_Set(void *mosq, MQTT_CONNECT_CALLBACK connect_cb, MQTT_LOSTCONNECT_CALLBACK lostconnect_cb, MQTT_DISCONNECT_CALLBACK disconnect_cb, MQTT_MESSAGE_CALLBACK message_cb);
int MQTT_SetTLS(void *mosq, const char *cafile, const char *capath, const char *certfile, const char *keyfile, int (*pw_callback)(char *buf, int size, int rwflag, void *userdata));
int MQTT_SetTLSPSK(void *mosq, const char *psk, const char *identity, const char *ciphers);
int MQTT_Connect(void *mosq, char const * ip, int port, char const * username, char const * password, int keepalive, char* willtopic, const void *willmsg, int msglen );
int MQTT_Reconnect(void *mosq);
void MQTT_Disconnect(void *mosq, bool bForce);
int MQTT_Publish(void *mosq,  char* topic, const void *msg, int msglen, int qos, bool retain);
int MQTT_Subscribe(void *mosq,  char* topic, int qos);
void MQTT_Unsubscribe(void *mosq,  char* topic);
int MQTT_GetSocket(void *mosq);
const char* MQTT_GetErrorString(int rc);
const char* MQTT_GetConnAckString(int rc);

#ifdef __cplusplus
}
#endif

#endif
