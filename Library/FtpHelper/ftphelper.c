#include "ftphelper.h"
#include "Log.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FtpDownload.h"
#include "FtpUpload.h"

#define DEF_FTP_LOG_NAME    "FtpLog"   //Updater log file name
#define FTP_LOG_ENABLE
//#define DEF_FTP_LOG_MODE    (LOG_MODE_NULL_OUT)
//#define DEF_FTP_LOG_MODE    (LOG_MODE_FILE_OUT)
#define DEF_FTP_LOG_MODE    (LOG_MODE_CONSOLE_OUT|LOG_MODE_FILE_OUT)
LOGHANDLE ftpLogHandle;
#ifdef FTP_LOG_ENABLE
#define FtpLog(level, fmt, ...)  do { if (ftpLogHandle != NULL)   \
	WriteIndividualLog(ftpLogHandle, "ftphelper", DEF_FTP_LOG_MODE, level, fmt, ##__VA_ARGS__); } while(0)
#else
#define FtpLog(level, fmt, ...)
#endif

static void* FTPDownloadThreadStart(void* args)
{
	ftp_context_t * pFtpParams = (ftp_context_t *)args;

	if(pFtpParams)
	{
		char repMsg[1024] = {0};
		if(pFtpParams->ftphandler)
		{
			int iRet = 0;
			pFtpParams->isTransferring = true;
			iRet = FtpDownload(pFtpParams->ftphandler, pFtpParams->sFileUrl, pFtpParams->sLocalPath);
			if(iRet != 0)
			{
				memset(repMsg, 0, sizeof(repMsg));
				strcpy(repMsg, "File download error!");
				if(iRet > 0)
				{
					char errStr[256] = {0};
					FtpDownloadGetErrorStr(iRet, errStr);
					if(strlen(errStr)) 
					{
						sprintf(repMsg, "%s%s", repMsg, errStr);
					}
				}
				FtpLog(Error, "%s", repMsg);
			}
			else
			{
				memset(repMsg, 0, sizeof(repMsg));
				strcpy(repMsg, "File download OK!");
				FtpLog(Normal, "%s", repMsg);
			}
			pFtpParams->isTransferring = false;
			FtpDownloadCleanup(pFtpParams->ftphandler);
			pFtpParams->ftphandler = NULL;
		}
	}
	
	pthread_exit(0);
	return 0;
}

static void* FTPUploadThreadStart(void* args)
{
	ftp_context_t * pFtpParams = (ftp_context_t *)args;

	if(pFtpParams)
	{
		char repMsg[1024] = {0};
		if(pFtpParams->ftphandler)
		{
			int iRet = 0;
			pFtpParams->isTransferring = true;
			iRet = FtpUpload(pFtpParams->ftphandler,  pFtpParams->sLocalPath, pFtpParams->sFileUrl);
			if(iRet != 0)
			{
				memset(repMsg, 0, sizeof(repMsg));
				strcpy(repMsg, "File upload error!");
				if(iRet > 0)
				{
					char errStr[256] = {0};
					FtpUploadGetErrorStr(iRet, errStr);
					if(strlen(errStr)) 
					{
						sprintf(repMsg, "%s%s", repMsg, errStr);
					}
				}
				FtpLog(Error, "%s", repMsg);
			}
			else
			{
				memset(repMsg, 0, sizeof(repMsg));
				strcpy(repMsg, "File upload OK!");
				FtpLog(Normal, "%s", repMsg);
			}
			pFtpParams->isTransferring = false;
			FtpUploadCleanup(pFtpParams->ftphandler);
			pFtpParams->ftphandler = NULL;
		}
	}

	pthread_exit(0);
	return 0;
}

void ftphelper_EnableLog(void* logHandle)
{
	ftpLogHandle = logHandle;
}

ftp_context_t* ftphelper_FtpDownload(char* ftpserver, int port, char* ftpuserName, char* ftpPassword, char* remotePath, char* localPath)
{
	ftp_context_t* contex = NULL;
	pthread_t threadhandle = 0;
	if(!ftpserver || strlen(ftpserver) <= 0) return contex;

	if(!localPath || strlen(localPath) <= 0) return contex;
	
	contex = malloc(sizeof(ftp_context_t));
	memset(contex, 0, sizeof(ftp_context_t));
	contex->iType = FTP_TYPE_DOWNLOAD;
	if((ftpuserName && strlen(ftpuserName)) && (ftpPassword && strlen(ftpPassword)))
	{
		sprintf(contex->sFileUrl, "ftp://%s:%s@%s:%d%s", ftpuserName, ftpPassword,
			ftpserver, port, remotePath);
	}
	else
	{
		sprintf(contex->sFileUrl, "ftp://%s:%d%s", ftpserver, port, remotePath);
	}
	FtpLog(Normal, "FTP URL: %s", contex->sFileUrl);

	strcpy(contex->sLocalPath, localPath);
	FtpLog(Normal, "File Download Path:%s",contex->sLocalPath);

	/*Detected 292 byte memory leaks in cURL*/
	contex->ftphandler = FtpDownloadInit();
	if(!contex->ftphandler)
	{
		free(contex);
		contex = NULL;
		return contex;
	}
	FtpLog(Normal, "%s","File downloader initialize Ok!");

	if (pthread_create(&threadhandle, NULL, FTPDownloadThreadStart, contex) != 0)
	{
		contex->isThreadRunning = false;
	}
	else
	{
		contex->isThreadRunning = true;
		contex->threadHandler = (void*)threadhandle;
	}
	return contex;
}

/*Not verified.*/
ftp_context_t* ftphelper_FtpUpload(char* ftpserver, int port, char* ftpuserName, char* ftpPassword, char* remotePath, char * localFile)
{
	ftp_context_t* contex = NULL;
	pthread_t threadhandle = 0;

	if(!ftpserver || strlen(ftpserver) <= 0) return contex;

	if(!localFile || strlen(localFile) <= 0) return contex;

	contex = malloc(sizeof(ftp_context_t));
	memset(contex, 0, sizeof(ftp_context_t));
	contex->iType = FTP_TYPE_UPLOAD;
	if((ftpuserName && strlen(ftpuserName)) && (ftpPassword && strlen(ftpPassword)))
	{
		sprintf(contex->sFileUrl, "ftp://%s:%s@%s:%d%s", ftpuserName, ftpPassword,
			ftpserver, port, remotePath);
	}
	else
	{
		sprintf(contex->sFileUrl, "ftp://%s:%d%s", ftpserver, port, remotePath);
	}
	FtpLog(Normal, "FTP URL: %s", contex->sFileUrl);

	strcpy(contex->sLocalPath, localFile);
	FtpLog(Normal, "File Upload Path:%s",contex->sLocalPath);

	contex->ftphandler = FtpUploadInit();
	if(!contex->ftphandler)
	{
		free(contex);
		contex = NULL;
		return contex;
	}
	FtpLog(Normal, "%s","File uploader initialize Ok!");
	
	if (pthread_create(&threadhandle, NULL, FTPUploadThreadStart, contex) != 0)
	{
		contex->isThreadRunning = false;
	}
	else
	{
		contex->isThreadRunning = true;
		contex->threadHandler = (void*)threadhandle;
	}
	return contex;
}

void ftphelper_WaitTransferComplete(ftp_context_t* contex)
{
	if(!contex) return;
	if(contex->isThreadRunning == true)
	{
		//MonitorLog(g_loghandle, Normal, " %s> Wait Trhead stop!\n", MyTopic);	
		contex->isThreadRunning = false;
		pthread_join((pthread_t)contex->threadHandler, NULL);
		contex->threadHandler = NULL;
	}
}

void ftphelper_FtpCleanup(ftp_context_t* contex)
{
	if(!contex) return;
	if(contex->isThreadRunning == true)
	{
		//MonitorLog(g_loghandle, Normal, " %s> Wait Trhead stop!\n", MyTopic);	
		contex->isThreadRunning = false;
		pthread_join((pthread_t)contex->threadHandler, NULL);
		contex->threadHandler = NULL;
	}

	if(contex->ftphandler)
	{
		if(contex->iType == FTP_TYPE_DOWNLOAD)
		{
			FtpDownloadCleanup(contex->ftphandler);
		}
		else if(contex->iType == FTP_TYPE_UPLOAD)
		{
			FtpUploadCleanup(contex->ftphandler);
		}
		contex->ftphandler = NULL;
	}

	free(contex);
	contex = NULL;
}

int ftphelper_FtpGetSpeedKBS(ftp_context_t* contex, float * speedKBS)
{
	if(contex->iType == FTP_TYPE_DOWNLOAD)
	{
		return FtpDownloadGetSpeedKBS(contex->ftphandler, speedKBS);
	}
	*speedKBS = 0;
	return -1;
}

int ftphelper_FTPGetPersent(ftp_context_t* contex, unsigned int * persent)
{
	if(contex->iType == FTP_TYPE_DOWNLOAD)
	{
		return FTPDownloadGetPersent(contex->ftphandler, persent);
	}
	*persent = 0;
	return -1;
}

int ftphelper_FTPGetCurSizeKB(ftp_context_t* contex, unsigned int * curSizeKB)
{
	if(contex->iType == FTP_TYPE_DOWNLOAD)
	{
		return FTPDownloadGetCurDLSizeKB(contex->ftphandler, curSizeKB);
	}
	*curSizeKB = 0;
	return -1;
}

int ftphelper_FTPGetStatus(ftp_context_t* contex, FTPSTATUS * dlStatus)
{
	if(contex->iType == FTP_TYPE_DOWNLOAD)
	{
		return FTPDownloadGetStatus(contex->ftphandler, (FTPDLSTATUS*)dlStatus);
	}
	*dlStatus = 0;
	return -1;
}

int ftphelper_FTPGetLastError(ftp_context_t* contex, char * errorBuf, int bufLen)
{
	if(contex->iType == FTP_TYPE_DOWNLOAD)
	{
		return FTPDownLoadGetLastError(contex->ftphandler, errorBuf, bufLen);
	}
	return -1;
}

void ftphelper_FtpGetErrorStr(ftp_context_t* contex, int errorCode, char * errorStr)
{
	if(contex->iType == FTP_TYPE_DOWNLOAD)
	{
		FtpDownloadGetErrorStr(errorCode, errorStr);
	}
	else if(contex->iType == FTP_TYPE_UPLOAD)
	{
		FtpUploadGetErrorStr(errorCode, errorStr);
	}
}