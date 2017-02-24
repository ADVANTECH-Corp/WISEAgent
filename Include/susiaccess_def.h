/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2014/07/04 by Scott Chang								    */
/* Modified Date: 2014/07/04 by Scott Chang									*/
/* Abstract     : SUSIAccessn definition									*/
/* Reference    : None														*/
/****************************************************************************/

#ifndef _SUSIACCESS_DEF_H_
#define _SUSIACCESS_DEF_H_

#define DEF_FILENAME_LENGTH			32
#define DEF_DEVID_LENGTH			32
#define DEF_FW_DESCRIPTION_LENGTH	128
#define DEF_PROCESSOR_NAME_LEN		64

#define DEF_MARK_LENGTH				8
#define DEF_ACCOUNT_LENGTH			32
#define DEF_TYPE_LENGTH				32
#define DEF_HOSTNAME_LENGTH			42
#define DEF_SN_LENGTH				32
#define DEF_MAC_LENGTH				32
#define DEF_LAL_LENGTH				20
#define DEF_VERSION_LENGTH			32
#define DEF_MAX_STRING_LENGTH		128
#define DEF_RUN_MODE_LENGTH			32
#define DEF_ENABLE_LENGTH			8
#define DEF_USER_PASS_LENGTH		128
#define DEF_PORT_LENGTH				8
#define DEF_KVM_MODE_LENGTH			16
#define MAX_TOPIC_LEN               32
#define DEF_OSVERSION_LEN			64
#define DEF_MAX_PATH				260
#define DEF_MAX_CIPHER				4095

#define AGENT_STATUS_OFFLINE		0  /**< Agent offline flag */
#define AGENT_STATUS_ONLINE			1  /**< Agent online  flag: Server responsed */
#define AGENT_STATUS_CONNECTION_FAILED	2  /**< Agent connect failed flag */

#define DEF_SERVER_IP_LIST_FILE   "server_IP_List.txt"

typedef enum{
	tls_type_unknown = -1,
	tls_type_none = 0,
	tls_type_tls,
	tls_type_psk,
}tls_type;

/** Agent configuration struct define*/
typedef struct {
	/*Connection Mode*/
	char runMode[DEF_RUN_MODE_LENGTH];
	//char lunchConnect[DEF_ENABLE_LENGTH];
	char autoStart[DEF_ENABLE_LENGTH];
	//char autoReport[DEF_ENABLE_LENGTH]; // IoT Auto rpeort.

	/*Connection Info*/
	char serverIP[DEF_MAX_STRING_LENGTH];
	char serverPort[DEF_PORT_LENGTH];
	char serverAuth[DEF_USER_PASS_LENGTH];

	tls_type tlstype;
	/*TLS*/
	char cafile[DEF_MAX_PATH];
	char capath[DEF_MAX_PATH];
	char certfile[DEF_MAX_PATH];
	char keyfile[DEF_MAX_PATH];
	char cerpasswd[DEF_USER_PASS_LENGTH];
	/*pre-shared-key*/
	char psk[DEF_USER_PASS_LENGTH];
	char identity[DEF_USER_PASS_LENGTH];
	char ciphers[DEF_MAX_CIPHER];

	/*char loginID[DEF_USER_PASS_LENGTH];
	char loginPwd[DEF_USER_PASS_LENGTH];*/
}susiaccess_agent_conf_body_t;

typedef struct {
	/*Agent Info*/
	char version[DEF_MAX_STRING_LENGTH];
	char hostname[DEF_HOSTNAME_LENGTH];
	char devId[DEF_DEVID_LENGTH];
	char sn[DEF_SN_LENGTH];					/**< Agent sn */
	char mac[DEF_MAC_LENGTH];				/*connected MAC address*/
	char lal[DEF_LAL_LENGTH];
	char account[DEF_USER_PASS_LENGTH];
	char passwd[DEF_USER_PASS_LENGTH];
	char workdir[DEF_MAX_PATH];

	/*Custom Info*/
	char type[DEF_MAX_STRING_LENGTH];
	char product[DEF_MAX_STRING_LENGTH];
	char manufacture[DEF_MAX_STRING_LENGTH];

	/*OS Info*/
	char osversion[DEF_OSVERSION_LEN];
	char biosversion[DEF_VERSION_LENGTH];
	char platformname[DEF_FILENAME_LENGTH];
	char processorname[DEF_PROCESSOR_NAME_LEN];
	char osarchitect[DEF_FILENAME_LENGTH];
	long totalmemsize;
	char maclist[DEF_MAC_LENGTH*16];
	char localip[DEF_MAX_STRING_LENGTH]; /*connected IP address*/
}susiaccess_agent_profile_body_t;

typedef enum{
	pkt_type_susiaccess = 0,
	pkt_type_custom,
}packet_type;
/** Packet Structure*/
typedef struct{
	int cmd;
	//int catalogID;      /**< Callback catalog id */
	int requestID;      /**< Callback request id */
	char devId[DEF_DEVID_LENGTH]; /**< Agent device id */
	char handlerName[MAX_TOPIC_LEN];
	char* content;     /**< Callback request content */
	packet_type type;
}susiaccess_packet_body_t;

#endif