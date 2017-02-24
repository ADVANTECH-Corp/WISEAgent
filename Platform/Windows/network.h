#ifndef _CAGENT_NETWORK_H_
#define _CAGENT_NETWORK_H_

#include <winsock2.h>
#include <IPHlpApi.h>
#include <stdbool.h>

#define socket_handle   SOCKET

//************************Define private fuction return code************************
typedef enum {
   network_error = -1,
   network_success = 0,               // No error. 
   network_no_init, 
   network_callback_null,
   network_callback_error,
   network_no_connnect,
   network_connect_error,
   network_init_error,   
   network_waitsock_timeout = 0x10,
   network_waitsock_error,
   network_act_unrecognized,
   network_terminate_error,
   network_send_data_error,
   network_reg_action_comm_error,
   network_reg_callback_comm_error,
   network_report_agentinfo_error,  
   network_send_action_data_error,
   network_send_callback_response_data_error,
   network_reg_willmsg_error,
   network_send_willmsg_error,
   // S--Added by wang.nan--S
   network_mindray_main_error,
   // E--Added by wang.nan--E
} network_status_t;

typedef enum{
   network_waitsock_read     = 0x01,
   network_waitsock_write    = 0x02,
   network_waitsock_rw       = 0x03,
}network_waitsock_mode_t;

int network_host_name_get(char * phostname, int size);
int network_ip_get(char * ipaddr, int size);
int network_mac_get(char * macstr);
int network_mac_get_ex(char * macstr);
int network_mac_list_get(char macsStr[][20], int n);
int network_local_ip_get(int socket, char* clientip, int size);
bool network_magic_packet_send(char * mac, int size);

#endif

