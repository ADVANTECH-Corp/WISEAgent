#include "agentupdater.h"
#include "generallog.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "AdvPlatform.h"
#include "util_path.h"
#include "util_process.h"

#include "ghparser.h"
#include "ftphelper.h"
#include "general_def.h"

#include "md5.h"

#ifdef _WIN32
#define DEF_CAGENT_UPDATER_EXE_NAME                     "CAgentUpdater.exe"
#define DEF_CAGENT_INSTALLER_DOWNLOAD_FILE_NAME         "SA31_CAgent.exe"
#else
#define DEF_CAGENT_UPDATER_EXE_NAME                     "SA31_CAgent.run"
#define DEF_CAGENT_INSTALLER_DOWNLOAD_FILE_NAME         "SA31_CAgent.run"
#endif // _WIN32
#define DEF_ADVANTECH_FOLDER_NAME                       "Advantech"

#define DEF_MD5_SIZE                16
#define DEF_PER_MD5_DATA_SIZE       512

typedef struct update_cagent_params_t{
#ifdef _MSC_VER  
	struct download_params_t;
#else
	char ftpuserName[64];
	char ftpPassword[64];
	int port;
	char installerPath[260];
	char md5[128];

	char filename[260];
	char downloadpath[260];
	char updatepath[260];
#endif

	char ServerIP[128];
	ftp_context_t* ctxdl;

	pthread_t DLThreadHandle;
	pthread_t DLMonThreadHandle;
	pthread_t GetCapabilityThreadHandle;
	bool IsDLMonThreadRunning;
	GeneralSendCbf pSendCb;

}update_cagent_params_t;

LOGHANDLE g_UpdaterLogHandle = NULL;

static update_cagent_params_t * g_pUpdateParams = NULL;

bool md5_caculate(char * filePath, char * retMD5Str)
{
	bool bRet = false;
	if(NULL == filePath || NULL == retMD5Str) return bRet;
	{
		FILE *fptr = NULL;
		fptr = fopen(filePath, "rb");
		if(fptr)
		{
			MD5_CTX context;
			unsigned char retMD5[DEF_MD5_SIZE] = {0};
			char dataBuf[DEF_PER_MD5_DATA_SIZE] = {0};
			unsigned int readLen = 0, realReadLen = 0;
			MD5Init(&context);
			readLen = sizeof(dataBuf);
			while ((realReadLen = fread(dataBuf, sizeof(char), readLen, fptr)) != 0)
			{
				MD5Update(&context, dataBuf, realReadLen);
				memset(dataBuf, 0, sizeof(dataBuf));
				realReadLen = 0;
				readLen = sizeof(dataBuf);
			}
			MD5Final(retMD5, &context);

			{
				char md5str0x[DEF_MD5_SIZE*2+1] = {0};
				int i = 0;
				for(i = 0; i<DEF_MD5_SIZE; i++)
				{
					sprintf(&md5str0x[i*2], "%.2x", retMD5[i]);
				}
				strcpy(retMD5Str, md5str0x);
				bRet = true;
			}
			fclose(fptr);
		}
	}
	return bRet;
}

static void* thread_agent_dl_monitor(void* args)
{
	if(args != NULL)
	{
		update_cagent_params_t* pUpdateParam = NULL;
		FTPSTATUS ftpDlStatus = FTP_UNKNOWN;
		bool isBreak = false;
		int iRet = 0, i = 0;
		pUpdateParam = (update_cagent_params_t*)args;
		usleep(10*1000);
		while(pUpdateParam->IsDLMonThreadRunning)
		{
			iRet = 0;
			if(pUpdateParam->ctxdl)
				iRet = ftphelper_FTPGetStatus(pUpdateParam->ctxdl, &ftpDlStatus);
			if(iRet == 0)
			{
				switch(ftpDlStatus)
				{
				case FTP_START:
					{
						for(i = 0; i<5 && pUpdateParam->IsDLMonThreadRunning; i++) usleep(100*1000);
						break;
					}
				case FTP_TRANSFERRING:
					{
						char downloadDetial[128] = {0};
						unsigned int dPercent = 0;
						unsigned int dCurSizeKB = 0;
						float dSpeed = 0;
						ftphelper_FTPGetPersent(pUpdateParam->ctxdl, &dPercent);
						ftphelper_FtpGetSpeedKBS(pUpdateParam->ctxdl, &dSpeed);
						ftphelper_FTPGetCurSizeKB(pUpdateParam->ctxdl, &dCurSizeKB);
						if(pUpdateParam->pSendCb)
						{
							sprintf(downloadDetial,"Downloading,Download Cursize:%dKB, Persent:%d%%, Speed: %4.2fKB/S\n", dCurSizeKB, dPercent, dSpeed);
							pUpdateParam->pSendCb(glb_update_cagent_rep, downloadDetial, strlen(downloadDetial)+1, NULL, NULL);
						}
						for(i = 0; i<10 && pUpdateParam->IsDLMonThreadRunning; i++) usleep(100*1000);
						break;
					}
				case FTP_FINISHED:
					{
						char downloadDetial[128] = {0};
						unsigned int dPercent = 0;
						unsigned int dCurSizeKB = 0;
						float dSpeed = 0;
						ftphelper_FTPGetPersent(pUpdateParam->ctxdl, &dPercent);
						ftphelper_FtpGetSpeedKBS(pUpdateParam->ctxdl, &dSpeed);
						ftphelper_FTPGetCurSizeKB(pUpdateParam->ctxdl, &dCurSizeKB);
						if(pUpdateParam->pSendCb)
						{
							sprintf(downloadDetial,"Download Finished,Download Cursize:%dKB, Persent:%d%%, Speed: %4.2fKB/S\n", dCurSizeKB, dPercent, dSpeed);
							pUpdateParam->pSendCb(glb_update_cagent_rep, downloadDetial, strlen(downloadDetial)+1, NULL, NULL);
						}
						isBreak = true;
						break;
					}
				case FTP_ERROR:
					{
						char lastMsgTmp[1024] = {0};
						char lastDownloaderErrorMsg[512] = {0};
						ftphelper_FTPGetLastError(pUpdateParam->ctxdl, lastDownloaderErrorMsg, sizeof(lastDownloaderErrorMsg));
						if(pUpdateParam->pSendCb)
						{
							sprintf(lastMsgTmp,"File downloader status error!Error msg:%s\n", lastDownloaderErrorMsg);
							pUpdateParam->pSendCb(glb_update_cagent_rep, lastMsgTmp, strlen(lastMsgTmp)+1, NULL, NULL);
						}
						isBreak = true;
						break;
					}
				default:
				case FTP_UNKNOWN:
					break;
				}
			}
			if(isBreak) break;
			usleep(10*1000);
		}
		pUpdateParam->IsDLMonThreadRunning = false;
	}
	pthread_exit(0);
	return 0;
}

#ifndef _WIN32
bool linux_updater_launch(const char * updaterPath)
{
	bool bRet = true;
	pid_t pid = fork();
	if (0 == pid) {
		setpgid(0, 0);
		exit(execlp("/bin/sh", "sh", "-c", updaterPath, NULL));
	} else if (pid < 0) {
		bRet = false;
	}
	return bRet;
}
#endif //_WIN32

bool updater_installer_exec(char * installPath)
{
	bool bRet = false;
	if(installPath == NULL || access(installPath, F_OK)) return bRet;
	{
		char updaterPath[MAX_PATH] = {0}; 
#ifdef _WIN32
		char modulePath[MAX_PATH] = {0};     
		util_module_path_get(modulePath);
		util_path_combine(updaterPath, modulePath, DEF_CAGENT_UPDATER_EXE_NAME);
#else
		sprintf(updaterPath, "%s", installPath);
		if(!access(updaterPath, F_OK))
#   ifdef ANDROID
			chmod(updaterPath, S_IRUSR|S_IWUSR|S_IXUSR);
#   else
			chmod(updaterPath, S_IREAD|S_IWRITE|S_IEXEC);
#   endif //ANDROID
#endif // _WIN32
		if(!access(updaterPath, F_OK))
		{
			util_process_kill(DEF_CAGENT_UPDATER_EXE_NAME);
#ifdef _WIN32
			bRet = util_process_launch(updaterPath);
#else
			bRet = linux_updater_launch(updaterPath);
#endif // _WIN32
			if(bRet)
			{
				SAGeneralLog(g_UpdaterLogHandle, Normal, "FTP ERR MSG: %s", "Create updater process ok!");
			}
			else
			{
				SAGeneralLog(g_UpdaterLogHandle, Error, "FTP ERR MSG: %s", "Create updater process error!");
			}
		}
		else
		{
			SAGeneralLog(g_UpdaterLogHandle, Error, "FTP ERR MSG: updaterPath not exist:%s", updaterPath);
		}
	}
	return bRet;
}

bool updater_download(update_cagent_params_t *pUpdateParams)
{
	char repMsg[1024] = {0};
	char md5Str[64] = {0};
	ftp_context_t* ctxdl = NULL;

	if(!pUpdateParams)
		return false;
	
	ftphelper_EnableLog((void *)g_UpdaterLogHandle);

	ctxdl = ftphelper_FtpDownload(pUpdateParams->ServerIP, pUpdateParams->port, pUpdateParams->ftpuserName, pUpdateParams->ftpPassword, pUpdateParams->installerPath, pUpdateParams->downloadpath);
	pUpdateParams->ctxdl = ctxdl;
	ftphelper_WaitTransferComplete(ctxdl);
	pUpdateParams->IsDLMonThreadRunning = false;
	ftphelper_FTPGetLastError(ctxdl, repMsg, sizeof(repMsg));
	ftphelper_FtpCleanup(ctxdl);

	if(strlen(repMsg))
	{
		SAGeneralLog(g_UpdaterLogHandle, Normal, "FTP ERR MSG: %s", repMsg);
		if(pUpdateParams->pSendCb)
		{
			pUpdateParams->pSendCb(glb_update_cagent_rep, repMsg, strlen(repMsg), NULL, NULL);
		}
		return false;
	}

	if(md5_caculate(pUpdateParams->downloadpath, md5Str))
	{
		if(strcasecmp(md5Str, pUpdateParams->md5) != 0)
		{
			memset(repMsg, 0, sizeof(repMsg));
			sprintf(repMsg, "Check md5 error!");
			SAGeneralLog(g_UpdaterLogHandle, Error, "FTP ERR MSG: %s DWMd5:%s, CalcMD5:%s", repMsg, pUpdateParams->md5, md5Str);
			if(pUpdateParams->pSendCb)
			{
				pUpdateParams->pSendCb(glb_update_cagent_rep, repMsg, strlen(repMsg), NULL, NULL);
			}
			return false;
		}
	}
	else
	{
		SAGeneralLog(g_UpdaterLogHandle, Error, "FTP ERR MSG: Calculate MD5 failed on: %s", pUpdateParams->filename);
		if(pUpdateParams->pSendCb)
		{
			sprintf(repMsg, "Calculate MD5 failed!");
			pUpdateParams->pSendCb(glb_update_cagent_rep, repMsg, strlen(repMsg), NULL, NULL);
		}
		return false;
	}

	SAGeneralLog(g_UpdaterLogHandle, Normal, "FTP ERR MSG: Check md5 OK! DWMd5:%s, CalcMD5:%s",pUpdateParams->md5, md5Str);

	if(!access(pUpdateParams->updatepath, F_OK))
	{
		if(remove(pUpdateParams->updatepath)<0)
		{
			sprintf(repMsg, "Cannot remove file: %s", pUpdateParams->updatepath);
			SAGeneralLog(g_UpdaterLogHandle, Error, "FTP ERR MSG: %s", repMsg);
		}
	}

	if(rename(pUpdateParams->downloadpath, pUpdateParams->updatepath)<0)
	{
		sprintf(repMsg, "Change file name failed on: %s", pUpdateParams->downloadpath);
		SAGeneralLog(g_UpdaterLogHandle, Error, "FTP ERR MSG: %s", repMsg);
		if(pUpdateParams->pSendCb)
		{
			pUpdateParams->pSendCb(glb_update_cagent_rep, repMsg, strlen(repMsg), NULL, NULL);
		}
		return false;
	}
	memset(repMsg, 0, sizeof(repMsg));
	sprintf(repMsg, "%s", "CAgentUpdater run...");
	SAGeneralLog(g_UpdaterLogHandle, Normal, "FTP ERR MSG: %s", repMsg);
	if(pUpdateParams->pSendCb)
		pUpdateParams->pSendCb(glb_update_cagent_rep, repMsg, strlen(repMsg), NULL, NULL);
	//run installer
	if(!updater_installer_exec(pUpdateParams->updatepath))
	{
		memset(repMsg, 0, sizeof(repMsg));
		sprintf(repMsg, "%s", "CAgentUpdater run error!");
		SAGeneralLog(g_UpdaterLogHandle, Error, "FTP ERR MSG: %sInstallerDownloadPath:%s", repMsg, pUpdateParams->downloadpath);
		if(pUpdateParams->pSendCb)
		{
			pUpdateParams->pSendCb(glb_update_cagent_rep, repMsg, strlen(repMsg), NULL, NULL);
		}
		return false;
	}
	return true;
}

static void* thread_agent_update(void* args)
{
	update_cagent_params_t * pUpdateParams = (update_cagent_params_t *)args;

	if(pUpdateParams)
	{
		char buf [MAX_PATH] = {0};
		char filename[MAX_PATH] = {0};
		char downloadpath[MAX_PATH] = {0};
		char updatepath[MAX_PATH] = {0};
		char repMsg[1024] = {0};

		pthread_t DLMonThreadHandle = 0;

		util_split_path_file(pUpdateParams->installerPath, buf, filename);

		memset(buf, 0, MAX_PATH);
		
		if(util_get_temp_path(buf, MAX_PATH) > 0)
		{
			char advantchPath[MAX_PATH] = {0};
			util_path_combine(advantchPath, buf, DEF_ADVANTECH_FOLDER_NAME);
			util_create_directory(advantchPath);
			util_path_combine(downloadpath, advantchPath, filename);
			util_path_combine(updatepath, advantchPath, DEF_CAGENT_INSTALLER_DOWNLOAD_FILE_NAME);
		}
		else
		{
			char modulePath[MAX_PATH] = {0};     
			util_module_path_get(modulePath);
			util_path_combine(downloadpath, modulePath, filename);
			util_path_combine(updatepath, modulePath, DEF_CAGENT_INSTALLER_DOWNLOAD_FILE_NAME);
		}

		strcpy(pUpdateParams->downloadpath, downloadpath);
		strcpy(pUpdateParams->updatepath, updatepath);
		strcpy(pUpdateParams->filename, filename);
		g_pUpdateParams->IsDLMonThreadRunning = true;
		if(pthread_create(&DLMonThreadHandle, NULL, thread_agent_dl_monitor, pUpdateParams) != 0)
		{
			g_pUpdateParams->IsDLMonThreadRunning = false;
			DLMonThreadHandle = 0;
			memset(repMsg, 0, sizeof(repMsg));
			sprintf(repMsg, "%s", "CAgent installer download monitor start failed!");
			SAGeneralLog(g_UpdaterLogHandle, Normal, "FTP ERR MSG: %s", repMsg);
			if(pUpdateParams->pSendCb)
				pUpdateParams->pSendCb(glb_update_cagent_rep, repMsg, strlen(repMsg), NULL, NULL);
		}
		else
		{
			pUpdateParams->DLMonThreadHandle = DLMonThreadHandle;
			updater_download(pUpdateParams);
			pUpdateParams->IsDLMonThreadRunning = false;
			pthread_join(DLMonThreadHandle, NULL);
		}
	}

	pthread_exit(0);
	return 0;
}



bool updater_update_start(char* server, void* data, int datalen, char* repMsg, GeneralSendCbf pSendCb, void* loghandle)
{
	if(!repMsg) return false;
	if(!data) return false;
	if(datalen<=0) return false;
	if(!server) return false;

	if(g_pUpdateParams)
	{
		updater_update_stop();
	}

	g_UpdaterLogHandle = loghandle;
	g_pUpdateParams = malloc(sizeof(update_cagent_params_t));
	memset(g_pUpdateParams, 0, sizeof(update_cagent_params_t));
	strcpy(g_pUpdateParams->ServerIP, server);
	g_pUpdateParams->IsDLMonThreadRunning = false;
	g_pUpdateParams->pSendCb = pSendCb;

	{
		//char repMsg[2*1024] = {0};
		if(ParseUpdateCMD(data, datalen, (download_params_t*)g_pUpdateParams)==true)
		{
			if (pthread_create(&g_pUpdateParams->DLThreadHandle, NULL, thread_agent_update, g_pUpdateParams) != 0)
			{
				sprintf(repMsg, "%s", "Update cagent thread start error!");
			}
		}
		else
		{
			if(g_pUpdateParams->pSendCb)
			{
				sprintf(repMsg, "%s", "Update command parse error!");
				g_pUpdateParams->pSendCb(glb_update_cagent_rep, repMsg, strlen(repMsg), NULL, NULL);
			}
			return false;
		}
	}
	return true;
}

void updater_update_stop()
{
	if(g_pUpdateParams)
	{
		if(g_pUpdateParams->IsDLMonThreadRunning && g_pUpdateParams->DLMonThreadHandle)
		{
			g_pUpdateParams->IsDLMonThreadRunning = false;
			pthread_join(g_pUpdateParams->DLMonThreadHandle, NULL);
			g_pUpdateParams->DLMonThreadHandle = 0;
		}

		ftphelper_FtpCleanup(g_pUpdateParams->ctxdl);

		if(g_pUpdateParams->DLThreadHandle)
		{
			pthread_join(g_pUpdateParams->DLThreadHandle, NULL);
			g_pUpdateParams->DLThreadHandle = 0;
		}
	}
	free(g_pUpdateParams);
	g_pUpdateParams = NULL;	
}

void updater_update_retry(char* server, void * data, int datalen, char* repMsg, GeneralSendCbf pSendCb, void* loghandle)
{
	updater_update_stop();

	if(!updater_update_start(server, data, datalen, repMsg, pSendCb, loghandle))
	{
		free(g_pUpdateParams);
		g_pUpdateParams = NULL;
	}
}
