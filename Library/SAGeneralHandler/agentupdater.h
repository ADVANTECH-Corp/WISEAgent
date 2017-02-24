#ifndef _AGENTUPDATER_UPDATER_H_
#define _AGENTUPDATER_UPDATER_H_

#include <stdbool.h>

typedef bool  (*GeneralSendCbf) (int cmd, char const * msg, int len, void *pRev1, void* pRev2 ); 

typedef struct download_params_t{
	char ftpuserName[64];
	char ftpPassword[64];
	int port;
	char installerPath[260];
	char md5[128];

	char filename[260];
	char downloadpath[260];
	char updatepath[260];
}download_params_t;

#ifdef __cplusplus
extern "C" {
#endif

bool updater_update_start(char* server, void* data, int datalen, char* repMsg, GeneralSendCbf pSendCb, void* loghandle);
void updater_update_stop();
void updater_update_retry(char* server, void * data, int datalen, char* repMsg, GeneralSendCbf pSendCb, void* loghandle);

#ifdef __cplusplus
}
#endif

#endif