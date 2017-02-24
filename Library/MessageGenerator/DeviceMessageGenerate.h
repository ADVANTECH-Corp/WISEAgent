#ifndef _DEVICE_MESSAGE_GENERATE_H_
#define _DEVICE_MESSAGE_GENERATE_H_
#include "MsgGenerator.h"
#include "susiaccess_def.h"

#ifdef __cplusplus
extern "C" {
#endif

	MSG_CLASSIFY_T* DEV_CreateAgentInfo(susiaccess_agent_profile_body_t* profile);
	
	MSG_CLASSIFY_T* DEV_CreateWillMessage(susiaccess_agent_profile_body_t* profile);

	MSG_CLASSIFY_T* DEV_CreateOSInfo(susiaccess_agent_profile_body_t* profile);

	MSG_CLASSIFY_T* DEV_CreateHandlerList(char* devID, char** handldelist, int count);

	MSG_CLASSIFY_T* DEV_CreateEventNotify(char* subtype, char* message);

	MSG_CLASSIFY_T* DEV_CreateFullEventNotify(char* devID, int severity, char* handler, char* subtype, char* message);

	char *DEV_GetAgentInfoTopic(char* devID);

	char *DEV_GetWillMessageTopic(char* devID);

	char *DEV_GetActionReqTopic(char* devID);

	char *DEV_GetEventNotifyTopic(char* devID);

#define DEV_GetOSInfoTopic(devID) DEV_GetActionReqTopic(devID);

#define DEV_GetHandlerListTopic(devID) DEV_GetActionReqTopic(devID);

	void DEV_ReleaseDevice(MSG_CLASSIFY_T* dev);

	char  *DEV_PrintUnformatted(MSG_CLASSIFY_T* dev);

#ifdef __cplusplus
}
#endif
#endif