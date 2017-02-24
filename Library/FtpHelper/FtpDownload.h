#ifndef _FTP_DOWNLOAD_H_
#define _FTP_DOWNLOAD_H_

typedef enum FTPDLStatusCode{
	FTP_DSC_UNKNOWN = 0,
	FTP_DSC_START,
	FTP_DSC_DOWNLOADING,
	FTP_DSC_FINISHED,
	FTP_DSC_ERROR,
}FTPDLSTATUS;

#ifndef HFTPUPL
typedef void * HFTPDL;
#endif

#ifdef  __cplusplus
extern "C" {
#endif

HFTPDL FtpDownloadInit();

int FtpDownload(HFTPDL hFtpDl, char * remoteUrl, char * localFile);

int FtpDownloadGetSpeedKBS(HFTPDL hFtpDl, float * speedKBS);

int FTPDownloadGetPersent(HFTPDL hFtpDl, unsigned int * persent);

int FTPDownloadGetCurDLSizeKB(HFTPDL hFtpDl, unsigned int * curSizeKB);

int FTPDownloadGetStatus(HFTPDL hFtpDl, FTPDLSTATUS * dlStatus);

int FTPDownLoadGetLastError(HFTPDL hFtpDl, char * errorBuf, int bufLen);

void FtpDownloadGetErrorStr(int errorCode, char * errorStr);

void FtpDownloadCleanup(HFTPDL hFtpDl);

#ifdef  __cplusplus
}
#endif

#endif
