#ifndef _FTP_UPLOAD_H_
#define _FTP_UPLOAD_H_

#ifndef HFTPUPL
typedef void * HFTPUPL;
#endif

#ifdef  __cplusplus
extern "C" {
#endif

HFTPUPL FtpUploadInit();

int FtpUpload(HFTPUPL hFtpUpl, char * localFile, char * remoteUrl);

void FtpUploadGetErrorStr(int errorCode, char * errorStr);

void FtpUploadCleanup(HFTPUPL hFtpUpl);

#ifdef  __cplusplus
}
#endif

#endif