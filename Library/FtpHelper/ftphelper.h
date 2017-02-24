#ifndef _FTP_HELPER_H_
#define _FTP_HELPER_H_

typedef void * HFTPHANDLER;

typedef enum FTPType{
	FTP_TYPE_UNKNOWN = 0,
	FTP_TYPE_DOWNLOAD,
	FTP_TYPE_UPLOAD,
}FTPTYPE;

typedef struct{
	void* threadHandler;
	HFTPHANDLER ftphandler; 
	int isThreadRunning;
	int isTransferring;
	char sFileUrl[260*2];
	char sLocalPath[260];
	FTPTYPE iType;
}ftp_context_t;

typedef enum FTPStatusCode{
	FTP_UNKNOWN = 0,
	FTP_START,
	FTP_TRANSFERRING,
	FTP_FINISHED,
	FTP_ERROR,
}FTPSTATUS;

#ifdef  __cplusplus
extern "C" {
#endif

void ftphelper_EnableLog(void* logHandle);

ftp_context_t* ftphelper_FtpDownload(char* ftpserver, int port, char* ftpuserName, char* ftpPassword, char* remotePath, char* localPath);

ftp_context_t* ftphelper_FtpUpload(char* ftpserver, int port, char* ftpuserName, char* ftpPassword, char* remotePath, char * localFile);

void ftphelper_WaitTransferComplete(ftp_context_t* contex);

void ftphelper_FtpCleanup(ftp_context_t* contex);

int ftphelper_FtpGetSpeedKBS(ftp_context_t* contex, float * speedKBS);

int ftphelper_FTPGetPersent(ftp_context_t* contex, unsigned int * persent);

int ftphelper_FTPGetCurSizeKB(ftp_context_t* contex, unsigned int * curSizeKB);

int ftphelper_FTPGetStatus(ftp_context_t* contex, FTPSTATUS * dlStatus);

int ftphelper_FTPGetLastError(ftp_context_t* contex, char * errorBuf, int bufLen);

void ftphelper_FtpGetErrorStr(ftp_context_t* contex, int errorCode, char * errorStr);

#ifdef  __cplusplus
}
#endif

#endif