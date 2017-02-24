#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <wchar.h>

#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>  //BLKSSZGET & BLKGETSIZE
#include <sys/inotify.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef LONGLONG
#ifndef LONGLONG_is_defined
#define LONGLONG_is_defined
typedef long long  LONGLONG;
#endif

#ifndef BOOL
#define BOOL  	int 
#define  FALSE  0
#define  TRUE   1
#endif

#ifndef DWORD
typedef unsigned int DWORD;
#endif

//#ifndef size_t
//typedef unsigned int size_t;
//#endif

#ifndef  _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 512
#endif
//Wei.Gang add
#define PROC_NAME_LINE 1
#define PROC_PID_LINE 5
#define PROC_UID_LINE 8
#define PROC_GID_LINE 9
#define PROC_VMSIZE_LINE 12
#define BUFF_LEN 1024 
#define CPU_USAGE_INFO_LINE 1
#define MEM_TOTAL_LINE 1
#define MEM_FREE_LINE 2

#define PROCESS_ALL_ACCESS  0xFFFF
#define TH32CS_SNAPPROCESS  0x00000002
#define PROCESS_QUERY_INFORMATION 0x0400
#define TOKEN_ALL_ACCESS 0x0100

#ifndef HANDLE
typedef long HANDLE;
#endif

#ifndef PHANDLE
typedef HANDLE *PHANDLE;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef __int64
typedef long long __int64;
#endif 

#ifndef LONG
typedef long  LONG;
#endif 

#ifndef ULONGLONG
typedef unsigned long long  ULONGLONG;
#endif 

#ifndef CHAR
typedef char  CHAR;
#endif 

typedef pthread_mutex_t CRITICAL_SECTION;

typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wYMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _SYSTEM_INFO{
	DWORD dwNumberOfProcessors;
}SYSTEM_INFO, *LPSYSTEM_INFO;

//FILETIME is jiffies
typedef struct _FILETIME{
	DWORD dwLowDateTIme;
	DWORD dwHighDateTime;
}FILETIME, *PFILETIME,*LPFILETIME;

typedef struct _PROCESS_MEMORY_COUNTERS{
	LONG WorkingSetSize;
}PROCESS_MEMORY_COUNTERS, *PPROCESS_MEMORY_COUNTERS;

typedef struct tagPROCESSENTRY32
{
	DWORD dwSize;
	DWORD th32ProcessID;
	DWORD dwUID;
	CHAR szExeFile[260];
}PROCESSENTRY32, *PPROCESSENTRY32, *LPPROCESSENTRY32;

typedef struct PROCESSENTRY32_NODE{
	PROCESSENTRY32 prcMonInfo;
	struct PROCESSENTRY32_NODE * next;
}PROCESSENTRY32_NODE, *PPROCESSENTRY32_NODE;

typedef struct _MEMORYSTATUSEX {
	DWORD dwLength;
	ULONGLONG ullTotalPhys;
	ULONGLONG ullAvailPhys;
}MEMORYSTATUSEX, *LPMEMORYSTATUSEX;

typedef struct statstruct_proc {
  int           pid;                      /** The process id. **/
  char          exName [_POSIX_PATH_MAX]; /** The filename of the executable **/
  char          state; /** 1 **/          /** R is running, S is sleeping, 
			   D is sleeping in an uninterruptible wait,
			   Z is zombie, T is traced or stopped **/
  unsigned      euid,                      /** effective user id **/
                egid;                      /** effective group id */					     
  int           ppid;                     /** The pid of the parent. **/
  int           pgrp;                     /** The pgrp of the process. **/
  int           session;                  /** The session id of the process. **/
  int           tty;                      /** The tty the process uses **/
  int           tpgid;                    /** (too long) **/
  unsigned int	flags;                    /** The flags of the process. **/
  unsigned int	minflt;                   /** The number of minor faults **/
  unsigned int	cminflt;                  /** The number of minor faults with childs **/
  unsigned int	majflt;                   /** The number of major faults **/
  unsigned int  cmajflt;                  /** The number of major faults with childs **/
  int           utime;                    /** user mode jiffies **/
  int           stime;                    /** kernel mode jiffies **/
  int		cutime;                   /** user mode jiffies with childs **/
  int           cstime;                   /** kernel mode jiffies with childs **/
  int           counter;                  /** process's next timeslice **/
  int           priority;                 /** the standard nice value, plus fifteen **/
  unsigned int  timeout;                  /** The time in jiffies of the next timeout **/
  unsigned int  itrealvalue;              /** The time before the next SIGALRM is sent to the process **/
  int           starttime; /** 20 **/     /** Time the process started after system boot **/
  unsigned int  vsize;                    /** Virtual memory size **/
  unsigned int  rss;                      /** Resident Set Size **/
  unsigned int  rlim;                     /** Current limit in bytes on the rss **/
  unsigned int  startcode;                /** The address above which program text can run **/
  unsigned int	endcode;                  /** The address below which program text can run **/
  unsigned int  startstack;               /** The address of the start of the stack **/
  unsigned int  kstkesp;                  /** The current value of ESP **/
  unsigned int  kstkeip;                 /** The current value of EIP **/
  int		signal;                   /** The bitmap of pending signals **/
  int           blocked; /** 30 **/       /** The bitmap of blocked signals **/
  int           sigignore;                /** The bitmap of ignored signals **/
  int           sigcatch;                 /** The bitmap of catched signals **/
  unsigned int  wchan;  /** 33 **/        /** (too long) **/
  int		sched, 		  /** scheduler **/
                sched_priority;		  /** scheduler priority **/
		
} procinfo;

DWORD getPIDByName(char * prcName);
DWORD app_os_GetLastError(void);
BOOL app_os_GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer);
BOOL app_os_GetSystemTimes(LPFILETIME lpIdleTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime);
BOOL app_os_CloseHandle(void * handle);
BOOL app_os_GetProcessUsername(HANDLE hProcess, char * userNameBuf, int bufLen);
BOOL app_os_GetProcessTimes(
		HANDLE hPRocess, 
		LPFILETIME lpCreationTime, 
		LPFILETIME lpExitTime, 
		LPFILETIME lpKernelTime, 
		LPFILETIME lpUserTime);
void GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime);
void app_os_GetSystemInfo(SYSTEM_INFO *lpSystemInfo);
BOOL GetProcessMemoryUsageKBWithID(DWORD prcID, long * memUsageKB);
BOOL IsProcessActiveWithIDEx(DWORD prcID);
HANDLE app_os_CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID);
BOOL app_os_Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
BOOL app_os_Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
BOOL app_os_CloseSnapShot32Handle(HANDLE hSnapshot);
int app_os_InitializeCriticalSection(CRITICAL_SECTION *restrict_mutex);
int app_os_DeleteCriticalSection(CRITICAL_SECTION *restrict_mutex);
int app_os_EnterCriticalSection(CRITICAL_SECTION *restrict_mutex);
int app_os_LeaveCriticalSection(CRITICAL_SECTION *restrict_mutex);
BOOL app_os_GetSysLogonUserName(char * userNameBuf, unsigned int bufLen);
void app_os_GetSysLogonUserList(char * logonUserList, int *logonUserCnt ,int maxLogonUserCnt,int maxLogonUserNameLen);
//Wei.Gang add end


//**********************************************************************************
//Data definition about thread
//Need to be modified according to different platform
//**********************************************************************************
#define CAGENT_HANDLE          int

#define CAGENT_THREAD_HANDLE pthread_t

#define CAGENT_THREAD_ID   pthread_t

#define CAGENT_MUTEX_TYPE  pthread_mutex_t

#define CAGENT_COND_TYPE  pthread_cond_t 

#define CAGENT_PTHREAD_ENTRY_POINTER(pointer_name) void * (*pointer_name)(void *)

#define CAGENT_PTHREAD_ENTRY(entry_name, arg_name) void * entry_name(void *arg_name)

//**********************************************************************************
//Data definition about OS and CPU
//**********************************************************************************
#define DEF_OS_NAME_LEN           34
#define DEF_OS_VERSION_LEN        64
#define DEF_BIOS_VERSION_LEN      256
#define DEF_PLATFORM_NAME_LEN     128
#define DEF_PROCESSOR_NAME_LEN    64

#define DEF_OS_ARCH_LEN           16
//**********************************************************************************

//=================================Part of ProtectHandler for Linux===================================
/*********************************************Macros and types**************************************/
typedef unsigned char BYTE;
typedef BYTE * PBYTE;
typedef wchar_t *LPWSTR;
typedef struct _EVENTLOGRECORD {
  DWORD Length;
  DWORD Reserved;
  DWORD RecordNumber;
  DWORD TimeGenerated;
  DWORD TimeWritten;
  DWORD EventID;
  WORD  EventType;
  WORD  NumStrings;
  WORD  EventCategory;
  WORD  ReservedFlags;
  DWORD ClosingRecordNumber;
  DWORD StringOffset;
  DWORD UserSidLength;
  DWORD UserSidOffset;
  DWORD DataLength;
  DWORD DataOffset;
} EVENTLOGRECORD, *PEVENTLOGRECORD;
#define ERROR_INSUFFICIENT_BUFFER		122L          //dderror
#define ERROR_HANDLE_EOF				38L
#define EVENTLOG_SEQUENTIAL_READ		0x0001
#define EVENTLOG_SEEK_READ				0x0002
#define EVENTLOG_FORWARDS_READ			0x0004
/******************************************Functions declaration************************************/
BOOL ResetEvent(HANDLE hEvent);
BOOL NotifyChangeEventLog(HANDLE hEventLog, HANDLE hEvent);

CAGENT_HANDLE app_os_CreateProcessWithAppNameEx(const char * appName, char * params);
CAGENT_HANDLE app_os_RegisterEventSource(char * serverName, char *sourceName);
BOOL app_os_ReportEvent(void* eventHandle, DWORD dwEventID, char * eventStr);
BOOL app_os_DeregisterEventSource(void* eventHandle);
BOOL app_os_GetExitCodeProcess(void* prcHandle, DWORD * exitCode);
CAGENT_HANDLE app_os_CreateProcessWithCmdLineEx(const char * cmdLine);
BOOL app_os_GetGUID(char * guidBuf, int bufSize);
BOOL app_os_TerminateThread(void * hThrHandle, DWORD dwExitCode);
BOOL app_os_CloseEventLog(void * hEventHandle);
BOOL app_os_set_event(void *handle);
BOOL app_os_get_regLocalMachine_value(char * lpSubKey, char * lpValueName,\
		void*lpValue, DWORD valueSize);
BOOL app_os_set_regLocalMachine_value(char * lpSubKey, char * lpValueName, \
		void*lpValue, DWORD valueSize);
unsigned int app_os_GetSystemDirectory(char* dirBuffer, unsigned int bufSize);
void * app_os_open_event_log(char * sourceName);
void * app_os_create_event(BOOL bManualReset, BOOL bInitialState, char * eventName);
BOOL app_os_ReadEventLog(void * hEventLog, DWORD dwReadFlags, DWORD dwRecordOffset,\
								 void * lpBuffer, DWORD nNumberOfBytesToRead, \
								 DWORD *pnBytesRead, DWORD *pnMinNumberOfBytesNeeded);
BOOL app_os_GetOldestEventLogRecord(void * hEvnetLog, DWORD* oldestRecord);
BOOL app_os_GetNumberOfEventLogRecords(void * hEvnetLog, DWORD* numOfRecords);
/****************************************Common function********************************************/
BOOL app_os_Str2Tm(const char *buf, const char *format, struct tm *tm);
BOOL app_os_Str2Tm_MDY(const char *buf, const char *format, struct tm *tm);
BOOL app_os_Str2Tm_YMD(const char *buf, const char *format, struct tm *tm);
BOOL app_os_FileIsUnicode(char * fileName);
int app_os_GetFileLineCount(const char * pFileName);
BOOL app_os_get_programfiles_path(char *pProgramFilesPath);
BOOL app_os_get_os_name(char * pOSNameBuf);
DWORD app_os_get_last_error();
//=================================Part of ProtectHandler for Linux===================================

//=================================Part of RecoverHandler for Linux===================================
/*************macro and declaration****************/
/*
#define FILE_NOTIFY_CHANGE_SIZE		0x00000008

#define	WAIT_TIMEOUT				258L
#define	WAIT_OBJECT_0				0L

#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
DECLARE_HANDLE(HKEY);
#define HKEY_LOCAL_MACHINE                  (( HKEY )(0x80000002L) )
#define KEY_ALL_ACCESS				(0x000F003FL)
#define REG_SZ						(1)
#define KEY_READ					(0x000F0019L)

#define	SERVICE_STOPPED				0x00000001
#define SERVICE_STOP_PENDING		0x00000003
#define	SERVICE_RUNNING				0x00000004

#define REG_OPTION_NON_VOLATILE		(0x00000000L)
#define INFINITE					0xFFFFFFFF
*/
/*************function****************/
/*DWORD app_os_GetSrvStatus(char * srvName);
BOOL app_os_GetDiskSizeBytes(unsigned int diskNum, LONGLONG *outSize);

long  app_os_RegOpenKeyExA(HKEY hKey, const char* lpSubKey,DWORD ulOptions,DWORD samDesired, HKEY *phkResult);
long app_os_RegDeleteValueA(HKEY hKey,const char *lpValueName);
long app_os_RegSetValueExA(HKEY hKey, const char *lpValueName, DWORD Reserved, DWORD dwType, const char *lpData, DWORD cbData);
long RegCreateKeyEx(HKEY hKey,const char *lpSubKey,DWORD Reserved, \
		char *lpClass, DWORD dwOptions,\
		DWORD samDesired, DWORD lpSecurityAttributes,\
		HKEY *phkResult, char * lpdwDisposition);
long app_os_RegCloseKey(HKEY hKey);
long app_os_RegQueryValueExA(HKEY hKey, const char *lpValueName,\
		DWORD *lpReserved, DWORD *lpType,\
		char * lpData, DWORD *lpcbData);
DWORD app_os_StartSrv(char * srvName);*/

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((void *)-1)
#endif

#define INFINITE					0xFFFFFFFF
#define FILE_NOTIFY_CHANGE_SIZE		0x00000008
#define	WAIT_OBJECT_0				0L
#define	WAIT_TIMEOUT				258L

BOOL app_os_ProcessCheck(char * processName);
BOOL app_os_FindNextChangeNotification(CAGENT_HANDLE handle);
CAGENT_HANDLE app_os_FindFirstChangeNotification(char * lpPathName, BOOL bWatchSubtree, DWORD dwNotifyFilter);
DWORD app_os_WaitForSingleObject(void* hHandle, DWORD dwMilliseconds);
BOOL app_os_FindCloseChangeNotification(void * hChangeHandle);
//common
BOOL app_os_GetDefProgramFilesPath(char *pProgramFilesPath);
BOOL app_os_GetMinAndMaxSpaceMB( LONGLONG *minSpace, LONGLONG *maxSpace);
BOOL app_os_CreateProcess(const char * cmdLine);
char *app_os_itoa(int value, char *dest, int radix);
//=================================Part of RecoverHandler for Linux===================================

char *trimwhitespace(char *str);

void* app_load_library(const char* path );

int app_free_library(void * handle );

void* app_get_proc_address( void * handle, const char *Name );

char * app_load_error();

void app_os_sleep(unsigned int const timeoutMs);

int app_os_thread_create(CAGENT_THREAD_HANDLE * threadHandle, CAGENT_PTHREAD_ENTRY_POINTER(threadStart), void *args);

int app_os_thread_join(CAGENT_THREAD_HANDLE threadHandle);

int app_os_thread_cancel(CAGENT_THREAD_ID threadId);

void app_os_thread_exit(unsigned long code);

void app_os_thread_detach(CAGENT_THREAD_HANDLE threadHandle);

int app_os_mutex_setup(CAGENT_MUTEX_TYPE * mutex);

int app_os_mutex_lock(CAGENT_MUTEX_TYPE * mutex);

int app_os_mutex_unlock(CAGENT_MUTEX_TYPE * mutex);

int app_os_mutex_cleanup(CAGENT_MUTEX_TYPE * mutex);

int app_os_cond_setup(CAGENT_COND_TYPE * cond);

int app_os_cond_signal(CAGENT_COND_TYPE * cond);

int app_os_cond_broadcast(CAGENT_COND_TYPE * cond);

int app_os_cond_wait(CAGENT_COND_TYPE * cond, CAGENT_MUTEX_TYPE *mutex);

int app_os_cond_timed_wait(CAGENT_COND_TYPE * cond, CAGENT_MUTEX_TYPE *mutex, int *ms);

int app_os_cond_cleanup(CAGENT_COND_TYPE * cond);

int app_os_get_sysmem_usage_kb(long * totalPhysMemKB, long * availPhysMemKB);

int app_os_get_module_path(char * moudlePath);

int app_os_get_os_version(char * osVersionBuf,  int bufLen);

int app_os_get_processor_name(char * processorNameBuf, unsigned long * bufLen);

int app_os_get_architecture(char * osArchBuf, int bufLen);

void split_path_file(char const *filepath, char* path, char* file);

void path_combine(char* destination, const char* path1, const char* path2);

int app_os_get_temppath(char* lpBuffer, int nBufferLength);

int app_os_kill_process_name(char * processName);

int app_os_launch_process(char * appPath);

int app_os_create_directory(char* path);

int app_os_set_executable(char* path);

int app_os_get_iso_date(char* datetime, int length);

//----------------------dhl add S--------------------------------------
int app_os_PowerOff();

int app_os_PowerRestart();

void app_os_disableResumePassword();

BOOL app_os_IsPwrSuspendAllowed();

BOOL app_os_IsPwrHibernateAllowed();

BOOL app_os_PowerSuspend();

BOOL app_os_PowerHibernate();

//BOOL app_os_SAWakeOnLan(char * mac, int size);

unsigned long long app_os_GetTickCount();

//int app_os_GetAllMacs(char macsStr[][20], int n);

BOOL app_os_GetRandomStr(char *randomStrBuf, int bufSize);

//BOOL app_os_GetHostIP(char * ipStr);

int app_os_RunX11vnc(char * x11vncPath, char * runParams);

BOOL app_os_CloseHandleEx(CAGENT_HANDLE handle);

BOOL app_os_ReadFile(CAGENT_HANDLE fHandle, char * buf, unsigned int nSizeToRead, unsigned int * nSizeRead);

BOOL app_os_WriteFile(CAGENT_HANDLE fHandle, char * buf, unsigned int nSizeToWrite, unsigned int * nSizeWrite);

BOOL app_os_CreatePipe(CAGENT_HANDLE * hReadPipe, CAGENT_HANDLE * hWritePipe);

BOOL app_os_DupHandle(CAGENT_HANDLE srcHandle, CAGENT_HANDLE * trgHandle);

CAGENT_HANDLE app_os_PrepAndLaunchRedirectedChild(CAGENT_HANDLE hChildStdOut, CAGENT_HANDLE hChildStdIn,	CAGENT_HANDLE hChildStdErr);

BOOL app_os_is_file_exist(const char * pFilePath);

BOOL app_os_file_access(const char * pFilePath, int mode);

BOOL app_os_file_copy(const char * srcFilePath, const char * destFilePath);

BOOL app_os_file_copyEx(const char * srcFilePath, const char * destFilePath);

void app_os_file_remove(const char * filePath);

BOOL app_os_is_workstation_locked();

BOOL app_os_process_check(char * processName);

//----------------------dhl add E--------------------------------------

#ifdef __cplusplus
}
#endif

#endif

