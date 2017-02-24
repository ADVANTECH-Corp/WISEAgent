#include <stdio.h>
#include <string.h>

#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "FtpUpload.h"

static size_t FtpUplReadCallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  //curl_off_t nread;
  size_t retcode = fread(ptr, size, nmemb, stream);
  //nread = (curl_off_t)retcode;
  return retcode;
}

HFTPUPL FtpUploadInit()
{
	CURL *curl = NULL;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	return curl;
}

int FtpUpload(HFTPUPL hFtpUpl, char * localFile, char * remoteUrl)
{
	CURLcode rc = CURLE_OK;
	if(hFtpUpl == NULL || localFile == NULL || remoteUrl == NULL) return -1;
	{
		CURL *curl = (CURL *)hFtpUpl;
		FILE *hFile = NULL;
		struct stat fileInfo;
		curl_off_t fSize;

		if(stat(localFile, &fileInfo)) 
		{
			return -1;
		}
		fSize = (curl_off_t)fileInfo.st_size;
		hFile = fopen(localFile, "rb");
		if(!hFile) return -1;
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, FtpUplReadCallback);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, remoteUrl);
		curl_easy_setopt(curl, CURLOPT_READDATA, hFile);
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,(curl_off_t)fSize);
		rc = curl_easy_perform(curl);
		fclose(hFile);
	}
	return rc;
}

void FtpUploadGetErrorStr(int errorCode, char * errorStr)
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

void FtpUploadCleanup(HFTPUPL hFtpUpl)
{
	if(hFtpUpl) curl_easy_cleanup(hFtpUpl);
	curl_global_cleanup();
}