#include <curl/curl.h>
#include "FtpDownload.h"
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct FTPDLCONTEXT
{
	char * remoteUrl;
	char * localFile;
	void * hCurl;
	pthread_t dlInfoGetThreadHanle;
	int dlInfoGetThreadRunning;
	unsigned int dlFileLenB;
	unsigned int dlPersent;
	unsigned int dlCurSizeKB;
	float dlSpeedKBS;
	FTPDLSTATUS  dlStatus;
	char dlLastError[256];
}FTPDLCONTEXT, *PFTPDLCONTEXT;


typedef struct FtpFile {
	const char *filename;
	FILE *stream;
}FtpFile;

static bool IsCurlGlobalInit = false;

static void* FtpDLInfoGetThreadStart(void* pThreadParam)
{
	PFTPDLCONTEXT pFtpDLContext = (PFTPDLCONTEXT)pThreadParam;

	usleep(100*1000);
	if(pFtpDLContext && pFtpDLContext->hCurl)
	{
		CURL *curl = pFtpDLContext->hCurl;
		CURLcode res;
		double dlCountLen = 0;
		unsigned long dwPersent = 0;
		double dlCurSize = 0, dlSpeed = 0;
		while(pFtpDLContext->dlInfoGetThreadRunning)
		{
			if(dlCountLen <= 0)
			{
				res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dlCountLen);
				if(res == CURLE_OK)
				{
					pFtpDLContext->dlFileLenB = (unsigned int)dlCountLen;
				}
				else
				{
					char * tmpStr = NULL;
					pFtpDLContext->dlStatus = FTP_DSC_ERROR;
					tmpStr = (char *)curl_easy_strerror(res);
					if(tmpStr && strlen(tmpStr))
					{
						memset(pFtpDLContext->dlLastError, 0, sizeof(pFtpDLContext->dlLastError));
						strcpy(pFtpDLContext->dlLastError, tmpStr);
					}
					break;
				}
			}
			else
			{
				pFtpDLContext->dlStatus = FTP_DSC_DOWNLOADING;
				res = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dlCurSize);
				if(res == CURLE_OK)
				{
					pFtpDLContext->dlCurSizeKB = (unsigned int)(dlCurSize/1024);
					dwPersent = (unsigned long)((dlCurSize*100)/dlCountLen);
					pFtpDLContext->dlPersent = dwPersent;
				}
				else
				{
					char * tmpStr = NULL;
					pFtpDLContext->dlStatus = FTP_DSC_ERROR;
					tmpStr = (char *)curl_easy_strerror(res);
					if(tmpStr && strlen(tmpStr))
					{
						memset(pFtpDLContext->dlLastError, 0, sizeof(pFtpDLContext->dlLastError));
						strcpy(pFtpDLContext->dlLastError, tmpStr);
					}
					break;
				}

				res = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &dlSpeed);
				if(res == CURLE_OK)
				{
					pFtpDLContext->dlSpeedKBS = (float)dlSpeed/1024;
				}
				else
				{
					char * tmpStr = NULL;
					pFtpDLContext->dlStatus = FTP_DSC_ERROR;
					tmpStr = (char *)curl_easy_strerror(res);
					if(tmpStr && strlen(tmpStr))
					{
						memset(pFtpDLContext->dlLastError, 0, sizeof(pFtpDLContext->dlLastError));
						strcpy(pFtpDLContext->dlLastError, tmpStr);
					}
					break;
				}
			}
			if(dwPersent == 100 ) 
			{
				pFtpDLContext->dlStatus = FTP_DSC_FINISHED;
				break;
			}
			usleep(100*1000);
		}
	}
	pthread_exit(0);
	return 0;
}

static size_t FtpDwlWriteCallback(void *buffer, size_t size, size_t nmemb, void *stream)
{
	struct FtpFile *out=(struct FtpFile *)stream;
	if(out && !out->stream) 
	{
		out->stream=fopen(out->filename, "wb");
		if(!out->stream) return -1;
	}
	return fwrite(buffer, size, nmemb, out->stream);
}

HFTPDL FtpDownloadInit()
{
	PFTPDLCONTEXT pFtpDLContext = NULL;
	CURL *curl = NULL;
	CURLcode rc = CURLE_OK;
	if(!IsCurlGlobalInit)
	{
		rc = curl_global_init(CURL_GLOBAL_ALL);
		if(rc == CURLE_OK)
		{
			IsCurlGlobalInit = true;
			curl = curl_easy_init();
			if(curl == NULL)
			{
				IsCurlGlobalInit = false;
				curl_global_cleanup();
			}
		}
	}
	else
	{
		curl = curl_easy_init();
	}
	if(curl != NULL)
	{
		pFtpDLContext = (PFTPDLCONTEXT)malloc(sizeof(FTPDLCONTEXT));
		memset(pFtpDLContext, 0, sizeof(FTPDLCONTEXT));
		pFtpDLContext->hCurl = curl;
	}
	return pFtpDLContext;
}

int FtpDownload(HFTPDL hFtpDl, char * remoteUrl, char * localFile)
{
	CURLcode rc = CURLE_OK;
	if(hFtpDl == NULL || localFile == NULL || remoteUrl == NULL) return -1;
	{
		PFTPDLCONTEXT pFTPDLContext = (PFTPDLCONTEXT)hFtpDl;
		CURL *curl = (CURL *)pFTPDLContext->hCurl;
		int tmpLen = 0;
		if(curl == NULL) return -1;
		tmpLen = strlen(remoteUrl) + 1;
		pFTPDLContext->remoteUrl = (char *)malloc(tmpLen);
		memset(pFTPDLContext->remoteUrl, 0, tmpLen);
		strcpy(pFTPDLContext->remoteUrl, remoteUrl);
		tmpLen = strlen(localFile) + 1;
		pFTPDLContext->localFile = (char *)malloc(tmpLen);
		memset(pFTPDLContext->localFile, 0, tmpLen);
		strcpy(pFTPDLContext->localFile, localFile);
		{
			struct FtpFile ftpfile = {pFTPDLContext->localFile, NULL};
			curl_easy_setopt(curl, CURLOPT_URL, pFTPDLContext->remoteUrl);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FtpDwlWriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
			//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			pFTPDLContext->dlInfoGetThreadRunning = 1;

			pthread_create(&pFTPDLContext->dlInfoGetThreadHanle, NULL, FtpDLInfoGetThreadStart, pFTPDLContext);

			if(!pFTPDLContext->dlInfoGetThreadHanle)
			{
				pFTPDLContext->dlInfoGetThreadRunning = 0;
			}
			pFTPDLContext->dlStatus = FTP_DSC_START;
			rc= curl_easy_perform(curl);
			if(rc != CURLE_OK && !(rc == CURLE_OPERATION_TIMEDOUT && pFTPDLContext->dlPersent == 100))
			{
				char * tmpStr = NULL;
				pFTPDLContext->dlStatus = FTP_DSC_ERROR;
				tmpStr = (char *)curl_easy_strerror(rc);
				if(tmpStr && strlen(tmpStr))
				{
					memset(pFTPDLContext->dlLastError, 0, sizeof(pFTPDLContext->dlLastError));
					strcpy(pFTPDLContext->dlLastError, tmpStr);
				}
			}
			else
			{
				rc = CURLE_OK;
			}
			if(ftpfile.stream) fclose(ftpfile.stream); 
		}
	}
	return rc;
}

int FtpDownloadGetSpeedKBS(HFTPDL hFtpDl, float * speedKBS)
{
	PFTPDLCONTEXT pFTPDLContext = (PFTPDLCONTEXT)hFtpDl;
   if(hFtpDl == NULL || speedKBS == NULL) return -1;
	if(pFTPDLContext->dlStatus == FTP_DSC_DOWNLOADING || pFTPDLContext->dlStatus == FTP_DSC_FINISHED)
	{
		*speedKBS = pFTPDLContext->dlSpeedKBS;
	}
	else
	{
		*speedKBS = 0;
	}
	return 0;
}

int FTPDownloadGetPersent(HFTPDL hFtpDl, unsigned int * persent)
{
	PFTPDLCONTEXT pFTPDLContext = (PFTPDLCONTEXT)hFtpDl;
	if(hFtpDl == NULL || persent == NULL) return -1;
	if(pFTPDLContext->dlStatus == FTP_DSC_DOWNLOADING || pFTPDLContext->dlStatus == FTP_DSC_FINISHED)
	{
		*persent = pFTPDLContext->dlPersent;
	}
	else
	{
		*persent = 0;
	}
	return 0;
}

int FTPDownloadGetCurDLSizeKB(HFTPDL hFtpDl, unsigned int * curSizeKB)
{
	PFTPDLCONTEXT pFTPDLContext = (PFTPDLCONTEXT)hFtpDl;
	if(hFtpDl == NULL || curSizeKB == NULL) return -1;
	if(pFTPDLContext->dlStatus == FTP_DSC_DOWNLOADING || pFTPDLContext->dlStatus == FTP_DSC_FINISHED)
	{
		*curSizeKB = pFTPDLContext->dlCurSizeKB;
	}
	else
	{
		*curSizeKB = 0;
	}
	return 0;
}

int FTPDownloadGetStatus(HFTPDL hFtpDl, FTPDLSTATUS * dlStatus)
{
	PFTPDLCONTEXT pFTPDLContext = (PFTPDLCONTEXT)hFtpDl;
	if(hFtpDl == NULL || dlStatus == NULL) return -1;
	*dlStatus = pFTPDLContext->dlStatus;
	return 0;
}

int FTPDownLoadGetLastError(HFTPDL hFtpDl, char * errorBuf, int bufLen)
{
	PFTPDLCONTEXT pFTPDLContext = (PFTPDLCONTEXT)hFtpDl;
	if(hFtpDl == NULL || errorBuf == NULL) return -1;
	if(strlen(pFTPDLContext->dlLastError))
	{
		strcpy(errorBuf, pFTPDLContext->dlLastError);
	}
	return 0;
}

void FtpDownloadGetErrorStr(int errorCode, char * errorStr)
{
	if(errorCode<0 || errorStr == NULL) return;
	{
		CURLcode rc = (CURLcode) errorCode;
		char * tmpStr = (char *)curl_easy_strerror(rc);
		if(tmpStr && strlen(tmpStr))
		{
			strcpy(errorStr, tmpStr);
		}
	}
}

void FtpDownloadCleanup(HFTPDL hFtpDl)
{
	if(hFtpDl) 
	{
		PFTPDLCONTEXT pFTPDLContext = (PFTPDLCONTEXT)hFtpDl;
		if(pFTPDLContext->remoteUrl) 
		{
			free(pFTPDLContext->remoteUrl);
			pFTPDLContext->remoteUrl = NULL;
		}
		if(pFTPDLContext->localFile)
		{
			free(pFTPDLContext->localFile);
			pFTPDLContext->localFile = NULL;
		}
		if(pFTPDLContext->dlInfoGetThreadRunning)
		{
			pFTPDLContext->dlInfoGetThreadRunning = 0;
			if(pFTPDLContext->dlInfoGetThreadHanle)
			{
				pthread_join(pFTPDLContext->dlInfoGetThreadHanle, NULL);
				pFTPDLContext->dlInfoGetThreadHanle = NULL;
			}
		}
		if(pFTPDLContext->hCurl)
		{
			curl_easy_cleanup(pFTPDLContext->hCurl);
		}
	}
	if(IsCurlGlobalInit)
	{
		curl_global_cleanup();
		IsCurlGlobalInit = false;
	}
}