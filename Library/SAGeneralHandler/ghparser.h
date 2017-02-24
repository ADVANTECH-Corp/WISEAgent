#ifndef _SAGENERAL_PARSER_H_
#define _SAGENERAL_PARSER_H_

#include <stdbool.h>
#include "agentupdater.h"
#include "susiaccess_handler_ex_api.h"

typedef struct GENERAL_CTRL_MSG
{
	int statuscode;
	char* msg;
	char serverIP[DEF_MAX_STRING_LENGTH];
	int serverPort;
	char serverAuth[DEF_USER_PASS_LENGTH];

}GENERAL_CTRL_MSG, General_Ctrl_Msg;

#ifdef __cplusplus
extern "C" {
#endif

bool ParseUpdateCMD(void* data, int datalen, download_params_t *pDownloadParams);

bool ParseReceivedCMD(void* data, int datalen, int * cmdID, char* pSessionID);

bool ParseRenameCMD(void* data, int datalen, char* pNewName);

int ParseServerCtrl(void* data, int datalen, char* workdir, GENERAL_CTRL_MSG *pMessage);

bool ParseHeartbeatRateUpdateCMD(void* data, int datalen, int* heartbeatrate);

#ifdef __cplusplus
}
#endif

#endif