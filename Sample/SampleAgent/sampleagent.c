/* This provides a crude manner of testing the performance of a broker in messages/s. */
#include <stdio.h>
#include <string.h>
#include "AdvPlatform.h"
#include "util_path.h"
#include "agentlog.h"
#include "version.h"

#include <SAClient.h>

//-------------------------Memory leak check define--------------------------------
#ifdef MEM_LEAK_CHECK
#include <crtdbg.h>
_CrtMemState memStateStart, memStateEnd, memStateDiff;
#endif
//---------------------------------------------------------------------------------

#define DEF_SUSIACCESSAGENT_CONFIG_NAME "agent_config.xml"

/*agent connected callback function*/
void on_connect_cb()
{
	SUSIAccessAgentLog(Normal, "CB_Connected ");
}

/*agent lost connect callback function*/
void on_lost_connect_cb()
{
	SUSIAccessAgentLog(Normal, "CB_Lostconnect ");
}

/*agent disconnect callback function*/
void on_disconnect_cb()
{
	SUSIAccessAgentLog(Normal, "CB_Disconnect ");
}

/*agent received message callback function*/
void on_msgrecv(char* topic, susiaccess_packet_body_t *pkt, void *pRev1, void* pRev2)
{
	/*user can process received command here*/
	SUSIAccessAgentLog(Normal, "Packet received, %s\r\n", pkt->content);
}

int main(int argc, char *argv[])
{
	int iRet = 0;
	char moudlePath[MAX_PATH] = {0};
	
	/*agent configuration structure: define how does the agent connect to Server*/
	susiaccess_agent_conf_body_t config;

	/*agent profile structure: define agent platform information*/
	susiaccess_agent_profile_body_t profile;


#ifdef MEM_LEAK_CHECK
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
	//_CrtSetBreakAlloc(3719);
	_CrtMemCheckpoint( &memStateStart);
#endif

	memset(moudlePath, 0 , sizeof(moudlePath));
	util_module_path_get(moudlePath);
		
	// Initialize Log Library
	SUSIAccessAgentLogHandle = InitLog(moudlePath);
	SUSIAccessAgentLog(Normal, "Current path: %s", moudlePath);

	// Pre-set Agent Config struct
	memset(&config, 0 , sizeof(susiaccess_agent_conf_body_t));
	strcpy(config.runMode,"remote"); //runMode default is remote. There are no other mode in WISE Agent version 3.x
	strcpy(config.autoStart,"True"); //autoStart default is True. The Agent will reconnect to server automatically.
	strcpy(config.serverIP,"dev-wisepaas.eastasia.cloudapp.azure.com"); //serverIP indicate the server RUL or IP Address
	strcpy(config.serverPort,"1883"); //serverPort indocate the server (MQTT Broker) listen port, default is 1883 in WISE Agent version 3.1 or later, WISE Agent version 3.0 is 10001.
	strcpy(config.serverAuth,"fENl4B7tnuwpIbs61I5xJQ=="); //serverAuth is the server (MQTT Broker) authentication string. the string is encode from <ID>:<PASS>. It only worked on SSL Mode.
	config.tlstype = tls_type_none; //tlstype define the TLS (SSL) mode
	switch(config.tlstype)
	{
	case tls_type_none: //disable TLS (SSL).
		break;
	case tls_type_tls: //setup TLS with certificate file.
		{
			strcpy(config.cafile, "ca.crt");
			strcpy(config.capath, "");
			strcpy(config.certfile, "server.crt");
			strcpy(config.keyfile, "server.key");
			strcpy(config.cerpasswd, "123456");
		}
		break;
	case tls_type_psk: //setup TLS with pre share key.
		{
			strcpy(config.psk, "");
			strcpy(config.identity, "SAClientSample");
			strcpy(config.ciphers, "");
		}
		break;
	}

	// Pre-set Agent Profile struct
	memset(&profile, 0 , sizeof(susiaccess_agent_profile_body_t));
	snprintf(profile.version, DEF_VERSION_LENGTH, "%d.%d.%d.%d", 3, 1, 0, 0);  //version indicate the version fo the application.
	strcpy(profile.hostname,"SAClientSample"); //hostname indicate the name of target device ro agent.
	strcpy(profile.devId,"000014DAE996BE04"); //devId is the Unique ID of the defice or agent.
	strcpy(profile.sn,"14DAE996BE04"); //sn indicate the device serial number.
	strcpy(profile.mac,"14DAE996BE04"); //mac indicate the MAC Address of first ethernet or wireless card.
	strcpy(profile.type,"IPC"); //type indicate the agent type, defualt is IPC. User can define their own type for customization.
	strcpy(profile.product,"Sample Agent"); //produce indicate the product name
	strcpy(profile.manufacture,"test"); //manufacture indicate the manufacture name
	strcpy(profile.osversion,"NA"); //osversion indicate the OS version of target device
	strcpy(profile.biosversion,"NA"); //biosversion indicate the BIOS version of target device
	strcpy(profile.platformname,"NA"); //platformname indicate the platform (board) name of target device
	strcpy(profile.processorname,"NA"); //processorname indicate the processor name of target device
	strcpy(profile.osarchitect,"NA"); //osarchitect indicate the OS architecture name of target device
	profile.totalmemsize = 40832; //totalmemsize indicate the OS recognized total memory size of target device
	strcpy(profile.maclist,"14DAE996BE04"); //maclist list all the ethernet and wireless card MAC Address.
	strcpy(profile.localip,"172.21.73.151"); //localip indicate the local IP of target device
	strcpy(profile.account,"anonymous"); //account bind the device or anget to the sepcific account, default is anonymous.
	strcpy(profile.passwd,""); //passwd indicate the encrypted password of account.
	strcpy(profile.workdir, moudlePath); //workdir indicate current executable binary file location.


	/*Initialize SAClient with Agent Configure and Profile structure, and the Log File Handle*/
	iRet = saclient_initialize(&config, &profile, SUSIAccessAgentLogHandle);

	if(iRet != saclient_success)
	{
		SUSIAccessAgentLog(Error, "Unable to initialize AgentCore.");
		goto EXIT;
	}

	SUSIAccessAgentLog(Normal, "Agent Initialized");

	/*register the conect, lost connect and disconnect callback function*/
	saclient_connection_callback_set(on_connect_cb, NULL, on_disconnect_cb);

	SUSIAccessAgentLog(Normal, "Agent Set Callback");
	
	/*start connect to server, server is defined in agent config*/
	iRet = saclient_connect();

	if(iRet != saclient_success){
		SUSIAccessAgentLog(Error, "sampleagent Unable to connect to broker.");
		goto EXIT;
	} else {
		SUSIAccessAgentLog(Normal, "sampleagent Connect to broker: %s", config.serverIP);
	}
	
	{
		
		char topicStr[128] = {0};
		susiaccess_packet_body_t pkt;

		/* Add  subscribe topic Callback*/
		sprintf(topicStr, "/cagent/admin/%s/testreq", profile.devId);
		saclient_subscribe(topicStr, 0, on_msgrecv);
		
		/*Send test packet to specific topic*/
		strcpy(pkt.devId, profile.devId);
		strcpy(pkt.handlerName, "Test");
		pkt.requestID = 0;
		pkt.cmd = 0;
		pkt.content = (char*)malloc(strlen("{\"Test\":100}")+1);
		memset(pkt.content, 0, strlen("{\"Test\":100}")+1);
		strcpy(pkt.content, "{\"Test\":100}");
		saclient_publish(topicStr, 0, false, &pkt);
		free(pkt.content);
	}

EXIT:
	printf("Click enter to exit");
	fgetc(stdin);

	/*disconnect from server*/
	saclient_disconnect();
	SUSIAccessAgentLog(Normal, "Send Client Info: disconnect");
	/*release SAClient resource*/
	saclient_uninitialize();
    SUSIAccessAgentLog(Normal, "Agent Uninitialize");
	/*release log resource*/
	UninitLog(SUSIAccessAgentLogHandle);

#ifdef MEM_LEAK_CHECK
	_CrtMemCheckpoint( &memStateEnd );
	if ( _CrtMemDifference( &memStateDiff, &memStateStart, &memStateEnd) )
		_CrtMemDumpStatistics( &memStateDiff );
#endif

	return iRet;
}

