/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2014/07/04 by Scott Chang								    */
/* Modified Date: 2014/07/04 by Scott Chang									*/
/* Abstract     : SAClient API definition									*/
/* Reference    : None														*/
/****************************************************************************/

#ifndef _SA_CLIENT_H_
#define _SA_CLIENT_H_
#include "susiaccess_def.h"

typedef enum {
   saclient_false = -1,               // Has error. 
   saclient_success = 0,               // No error.
   saclient_config_error = 1,
   saclient_profile_error = 2,
   saclient_no_init, 
   saclient_callback_null,
   saclient_callback_error,
   saclient_no_connnect,
   saclient_connect_error,
   saclient_init_error,
   saclient_network_sock_timeout = 0x10,
   saclient_network_sock_error,
   saclient_send_data_error,
   saclient_report_agentinfo_error,
   saclient_send_willmsg_error,
   /*TBD*/
} saclient_result;

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <windows.h>
#ifndef SACLIENT_API
	#define SACLIENT_API WINAPI
#endif
#else
	#define SACLIENT_API
#endif

typedef void (*SACLIENT_CONNECTED_CALLBACK)();
typedef void (*SACLIENT_LOSTCONNECT_CALLBACK)();
typedef void (*SACLIENT_DISCONNECT_CALLBACK)();
typedef void (*SACLIENT_MESSAGE_RECV_CALLBACK)(char* topic, susiaccess_packet_body_t *pkt, void *pRev1, void* pRev2);

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Function: saclient_initialize
 *
 * General initialization of the SAClient API. Prior to calling any SAClient API functions, 
 * the library needs to be initialized by calling this function. The status code for all 
 * SAClient API function will be saclient_no_init unless this function is called.
 *
 * Parameters:
 * 	config -	the configuration data in susiaccess_agent_conf_body_t struct.
 * 	profile -	the profile data in susiaccess_agent_profile_body_t struct.
 * 	loghandle -	the handle object of log
 *
 * Returns:
 * 	saclient_success -			on success.
 * 	saclient_false -			initialize failed
 * 	saclient_config_error -		the input config data were invalid.
 * 	saclient_profile_error -	the input profile data were invalid.
 */
int SACLIENT_API saclient_initialize(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile, void * loghandle);

/* 
 * Function: saclient_uninitialize
 *
 * General function to uninitialize the SAClient API library that should be called before program exit.
 *
 * Parameters:
 * 	None
 *
 * Returns:
 * 	None
 */
void SACLIENT_API saclient_uninitialize();

/* 
 * Function: saclient_connect
 *
 * Connect to server that defined in Configuration data of saclient_initialize parameters.
 *
 * Parameters:
 * 	None
 *
 * Returns:
 * 	saclient_success -					on success.
 * 	saclient_no_init -					not initialized or init failed before.	
 * 	saclient_network_sock_timeout -		connect timeout	
 * 	saclient_network_sock_error -		create socket failed
 * 	saclient_report_agentinfo_error -	send agentinfo failed.
 * 	saclient_send_willmsg_error -		send will message failed
 * 	saclient_false -					connect failed
 */
int SACLIENT_API saclient_connect();

/* 
 * Function: saclient_server_connect
 *
 * Connect to specific server.
 *
 * Parameters:
 * 	ip -		specific server IP or Domain name.
 * 	port -		specific server listen port.
 * 	mqttauth -	the encrypted mqtt account and password string.
 *
 * Returns:
* 	saclient_success -					on success.
 * 	saclient_no_init -					not initialized or init failed before.	
 * 	saclient_network_sock_timeout -		connect timeout	
 * 	saclient_network_sock_error -		create socket failed
 * 	saclient_report_agentinfo_error -	send agentinfo failed.
 * 	saclient_send_willmsg_error -		send will message failed
 * 	saclient_false -					connect failed
 */
int SACLIENT_API  saclient_server_connect(char const * ip, int port, char const * mqttauth);

/* 
 * Function: saclient_server_connect_ssl
 *
 * Connect to specific server with SSL support.
 *
 * Parameters:
 * 	ip -		specific server IP or Domain name.
 * 	port -		specific server listen port.
 * 	mqttauth -	the encrypted mqtt account and password string.
 *  tlstype -   0: disable, 1: CA file mode, 2: pre-share key Mode
 *  psk -       pre-share key
 *
 * Returns:
* 	saclient_success -					on success.
 * 	saclient_no_init -					not initialized or init failed before.	
 * 	saclient_network_sock_timeout -		connect timeout	
 * 	saclient_network_sock_error -		create socket failed
 * 	saclient_report_agentinfo_error -	send agentinfo failed.
 * 	saclient_send_willmsg_error -		send will message failed
 * 	saclient_false -					connect failed
 */
int SACLIENT_API  saclient_server_connect_ssl(char const * ip, int port, char const * mqttauth, tls_type tlstype, char const * psk);

/* 
 * Function: saclient_server_connect_config
 *
 * Connect to specific server with config and profile.
 *
 * Parameters:
 * 	config -		new server configuration.
 * 	profile -		new agent profile.
 *
 * Returns:
* 	saclient_success -					on success.
 * 	saclient_no_init -					not initialized or init failed before.	
 * 	saclient_network_sock_timeout -		connect timeout	
 * 	saclient_network_sock_error -		create socket failed
 * 	saclient_report_agentinfo_error -	send agentinfo failed.
 * 	saclient_send_willmsg_error -		send will message failed
 * 	saclient_false -					connect failed
 */
int SACLIENT_API  saclient_server_connect_config(susiaccess_agent_conf_body_t * config, susiaccess_agent_profile_body_t * profile);

/* 
 * Function: saclient_disconnect
 *
 * Disconnect from server.
 *
 * Parameters:
 * 	None
 *
 * Returns:
 * 	None
 */
void SACLIENT_API saclient_disconnect();

/* 
 * Function: saclient_reconnect
 *
 * Reconnect to current server.
 *
 * Parameters:
 * 	None
 *
 * Returns:
 * 	None
 */
void SACLIENT_API saclient_reconnect();

/* 
 * Function: saclient_connection_callback_set
 *
 * Register the connection callback function to handle the connection event.
 *
 * Parameters:
 * 	on_connect -		Function Pointer to handle connect success event.
 *  on_lost_connect -	Function Pointer to handle lost connect event,
 *						The SAClient will reconnect automatically, if left as NULL.
 *  on_disconnect -		Function Pointer to handle disconnect event.
 *
 * Returns:
 * 	None
 */
void SACLIENT_API saclient_connection_callback_set(SACLIENT_CONNECTED_CALLBACK on_connect, SACLIENT_LOSTCONNECT_CALLBACK on_lost_connect, SACLIENT_DISCONNECT_CALLBACK on_disconnect);

/* 
 * Function: saclient_publish
 *
 * Send message wrapped in packet structure to server on specific MQTT topic.
 *
 * Parameters:
 * 	topic -		the MQTT topic to publish.
 * 	qos -        integer value 0, 1 or 2 indicating the Quality of Service to be
 *               used for the message.
 * 	retain -     set to 1 to make the message retained.
 * 	pkt -		the message to publish, in susiaccess_packet_body_t struct.
 *
 * Returns:
 * 	saclient_success -					on success.
 * 	saclient_no_init -					not initialized or init failed before.	
 *  saclient_no_connnect -				not connect before or lost connection.	
 * 	saclient_false -					connect failed
 */
int SACLIENT_API saclient_publish(char const * topic, int qos, int retain, susiaccess_packet_body_t const * pkt);

/* 
 * Function: saclient_subscribe
 *
 * Register a callback function to receive message from server on specific MQTT topic.
 *
 * Parameters:
 * 	topic -					the MQTT topic to subscribe.
 * 	qos -        integer value 0, 1 or 2 indicating the Quality of Service to be
 *               used for the message.
 * 	msg_recv_callback -		the callback function to receive the message.
 *
 * Returns:
 * 	saclient_success -					on success.
 * 	saclient_no_init -					not initialized or init failed before.	
 *  saclient_no_connnect -				not connect before or lost connection.	
 * 	saclient_false -					connect failed
 */
int SACLIENT_API saclient_subscribe(char const * topic, int qos, SACLIENT_MESSAGE_RECV_CALLBACK msg_recv_callback);

/* 
 * Function: saclient_getsocketaddress
 *
 * Get local IP address that connect to server.
 *
 * Parameters:
 * 	clientip -	local ip address.
 * 	size -		the string length of ip address.
 *
 * Returns:
 * 	saclient_success -					on success.
 * 	saclient_no_init -					not initialized or init failed before.	
 *  saclient_no_connnect -				not connect before or lost connection.	
 * 	saclient_false -					connect failed
 */
int SACLIENT_API  saclient_getsocketaddress(char* clientip, int size);

#ifdef __cplusplus
}
#endif

#endif