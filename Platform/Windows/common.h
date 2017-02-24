#ifndef _COMMON_H_
#define _COMMON_H_

#include <Windows.h>
#include "time.h"
#include <TlHelp32.h>
#include <psapi.h>
#pragma comment(lib,"Psapi.lib")
//#include "IPHlpApi.h"
#include "Mprapi.h"

//**********************************************************************************
//Data definition about thread
//Need to be modified according to different platform
//**********************************************************************************
#define CAGENT_HANDLE          HANDLE

#define CAGENT_THREAD_HANDLE HANDLE

#define CAGENT_THREAD_ID   DWORD

#define CAGENT_MUTEX_TYPE  HANDLE

typedef struct { 
	UINT waiters_count;
	CRITICAL_SECTION waiters_count_lock;
	HANDLE signal_event;
	unsigned long num_wake;
	unsigned long generation;
} cagent_cond_t;

#define CAGENT_COND_TYPE  cagent_cond_t 

#define DEF_OS_NAME_LEN           34
#define DEF_OS_VERSION_LEN        64
#define DEF_BIOS_VERSION_LEN      256
#define DEF_PLATFORM_NAME_LEN     128
#define DEF_PROCESSOR_NAME_LEN    64
#define OS_UNKNOW                 "Unknow"
#define OS_WINDOWS_95             "Windows 95"
#define OS_WINDOWS_98             "Windows 98"
#define OS_WINDOWS_ME             "Windows ME"
#define OS_WINDOWS_NT_3_51        "Windows NT 3.51"
#define OS_WINDOWS_NT_4           "Windows NT 4"
#define OS_WINDOWS_2000           "Windows 2000"
#define OS_WINDOWS_XP             "Windows XP"
#define OS_WINDOWS_SERVER_2003    "Windows Server 2003"
#define OS_WINDOWS_VISTA          "Windows Vista"
#define OS_WINDOWS_7              "Windows 7"
#define OS_WINDOWS_8              "Windows 8"
#define OS_WINDOWS_SERVER_2012    "Windows Server 2012"
#define OS_WINDOWS_8_1            "Windows 8.1"
#define OS_WINDOWS_SERVER_2012_R2 "Windows Server 2012 R2"

#define DEF_OS_ARCH_LEN           16
#define ARCH_UNKNOW               "Unknow"
#define ARCH_X64                  "X64"
#define ARCH_ARM                  "ARM"
#define ARCH_IA64                 "IA64"
#define ARCH_X86                  "X86"

#define F_OK					  0
#define W_OK					  2	
#define R_OK					  4	
#define X_OK					  6	

#ifdef __cplusplus
extern "C" {
#endif

#define CAGENT_PTHREAD_ENTRY_POINTER(pointer_name)  DWORD (__stdcall *pointer_name)(LPVOID)

#define CAGENT_PTHREAD_ENTRY(entry_name, arg_name)  DWORD WINAPI entry_name(void *arg_name)

	void* app_load_library(const char* path );

	int app_free_library(void * handle );

	void* app_get_proc_address( void * handle, const char *Name );

	char * app_load_error();

	void app_os_sleep(unsigned int const timeoutMs);

	int app_os_thread_create(CAGENT_THREAD_HANDLE * threadHandle, CAGENT_PTHREAD_ENTRY_POINTER(threadStart), void *args);

	int app_os_thread_join(CAGENT_THREAD_HANDLE threadHandle);

	int app_os_thread_cancel(CAGENT_THREAD_HANDLE threadHandle);

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
	
	int app_os_get_process_id(void);

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

	int app_os_IsPwrSuspendAllowed();

	int app_os_IsPwrHibernateAllowed();

	BOOL app_os_PowerSuspend();

	BOOL app_os_PowerHibernate();

	//BOOL app_os_SAWakeOnLan(char * mac, int size);  //move to network.c

	unsigned long long app_os_GetTickCount();

	//int app_os_GetAllMacs(char macsStr[][20], int n); //move to network.c
	//----------------------dhl add E--------------------------------------

	BOOL app_os_is_file_exist(const char * pFilePath);

	BOOL app_os_file_access(const char * pFilePath, int mode);

	BOOL app_os_file_copy(const char * srcFilePath, const char * destFilePath);

	BOOL app_os_file_copyEx(const char * srcFilePath, const char * destFilePath);

	void app_os_file_remove(const char * filePath);

	BOOL app_os_is_workstation_locked();

	BOOL app_os_process_check(char * processName);

	BOOL app_os_run_process_as_user(char * cmdLine);

	struct tm *app_os_gmtime(const time_t *_Time);

	size_t app_os_strftime(char *_Buf, size_t _SizeInBytes, const char *_Format, const struct tm *_Tm);

	BOOL app_os_get_programfiles_path(char *pProgramFilesPath);

	BOOL app_os_get_os_name(char * pOSNameBuf);

	void * app_os_create_event(BOOL bManualReset, BOOL bInitialState, char * eventName);

	BOOL app_os_set_event(void *handle);

	void * app_os_open_event_log(char * sourceName);

	BOOL app_os_GetOldestEventLogRecord(void * hEvnetLog, DWORD* oldestRecord);

	BOOL app_os_GetNumberOfEventLogRecords(void * hEvnetLog, DWORD* numOfRecords);

	DWORD app_os_get_last_error();

	BOOL app_os_ReadEventLog(void * hEventLog, DWORD dwReadFlags, DWORD dwRecordOffset,
		void * lpBuffer, DWORD nNumberOfBytesToRead, DWORD *pnBytesRead, DWORD *pnMinNumberOfBytesNeeded);

	DWORD app_os_WaitForSingleObject(void* hHandle, DWORD dwMilliseconds);

	unsigned int app_os_GetSystemDirectory(char* dirBuffer, unsigned int bufSize);

	BOOL app_os_CloseHandle(CAGENT_HANDLE handle);

	CAGENT_HANDLE app_os_FindFirstChangeNotification(char * lpPathName, BOOL bWatchSubtree, DWORD dwNotifyFilter);

	BOOL app_os_FindNextChangeNotification(CAGENT_HANDLE handle);

	BOOL app_os_FindCloseChangeNotification(void * hChangeHandle);

	BOOL app_os_CloseEventLog(void * hEventHandle);

	int app_os_GetFileLineCount(const char * pFileName);

	BOOL app_os_FileIsUnicode(char * fileName);

	BOOL app_os_CreateProcess(const char * cmdLine);

	BOOL app_os_get_regLocalMachine_value(char * lpSubKey, char * lpValueName, void*lpValue, DWORD valueSize);

	BOOL app_os_set_regLocalMachine_value(char * lpSubKey, char * lpValueName, void*lpValue, DWORD valueSize);

	BOOL app_os_TerminateThread(void * hThrHandle, DWORD dwExitCode);

	BOOL app_os_GetGUID(char * guidBuf, int bufSize);

	BOOL app_os_Str2Tm(const char *buf, const char *format, struct tm *tm);

	BOOL app_os_Str2Tm_MDY(const char *buf, const char *format, struct tm *tm);

	BOOL app_os_Str2Tm_YMD(const char *buf, const char *format, struct tm *tm);

	CAGENT_HANDLE app_os_CreateProcessWithCmdLineEx(const char * cmdLine);

	CAGENT_HANDLE app_os_CreateProcessWithAppNameEx(const char * appName, char * params);

	BOOL app_os_GetExitCodeProcess(void* prcHandle, DWORD * exitCode);

	CAGENT_HANDLE app_os_RegisterEventSource(char * serverName, char *sourceName);

	BOOL app_os_ReportEvent(void* eventHandle, DWORD dwEventID, char * eventStr);

	BOOL app_os_DeregisterEventSource(void* eventHandle);

	BOOL app_os_Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);

	BOOL app_os_Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);

	DWORD app_os_GetLastError(void);

	DWORD app_os_GetSrvStatus(char * srvName);

	DWORD app_os_StopSrv(char * srvName);

	BOOL app_os_IsSrvExist(char * srvName);

	DWORD app_os_StartSrv(char * srvName);

	BOOL app_os_GetRandomStr(char *randomStrBuf, int bufSize);

	BOOL app_os_AdjustPrivileges();

	HANDLE app_os_GetProcessHandle(char * processName);

	BOOL app_os_GetProcessUsername(HANDLE hProcess, char * userNameBuf, int bufLen); 

	//BOOL app_os_GetHostIP(char * ipStr);

	CAGENT_HANDLE app_os_PrepAndLaunchRedirectedChild(CAGENT_HANDLE hChildStdOut, CAGENT_HANDLE hChildStdIn,	CAGENT_HANDLE hChildStdErr);

	BOOL app_os_CloseHandleEx(CAGENT_HANDLE handle);
	
	BOOL app_os_ReadFile(CAGENT_HANDLE fHandle, char * buf, unsigned int nSizeToRead, unsigned int * nSizeRead);

	BOOL app_os_WriteFile(CAGENT_HANDLE fHandle, char * buf, unsigned int nSizeToWrite, unsigned int * nSizeWrite);

	BOOL app_os_CreatePipe(CAGENT_HANDLE * hReadPipe, CAGENT_HANDLE * hWritePipe);

	BOOL app_os_DupHandle(CAGENT_HANDLE srcHandle, CAGENT_HANDLE * trgHandle);

	HANDLE app_os_GetCurrentProcess( VOID);

	BOOL app_os_FreeLibrary (__in HMODULE hLibModule);

	BOOL app_os_SystemTimeToFileTime(__in  CONST SYSTEMTIME *lpSystemTime,	__out LPFILETIME lpFileTime);

	LONG app_os_CompareFileTime(__in CONST FILETIME *lpFileTime1,__in CONST FILETIME *lpFileTime2);

	HLOCAL app_os_LocalFree(__deref HLOCAL hMem);

	VOID app_os_InitializeCriticalSection(__inout LPCRITICAL_SECTION lpCriticalSection);

	VOID app_os_EnterCriticalSection(__inout LPCRITICAL_SECTION lpCriticalSection);

	VOID app_os_LeaveCriticalSection(__inout LPCRITICAL_SECTION lpCriticalSection);

	VOID app_os_DeleteCriticalSection(__inout LPCRITICAL_SECTION lpCriticalSection);

	BOOL app_os_GetSysLogonUserName(char * userNameBuf, unsigned int bufLen);

	VOID app_os_GetProcessHandleList(HANDLE *eplHandleList, char *prcName, int *logonUserCnt, int maxLogonUserCnt);

	VOID app_os_GetSysLogonUserList(char * logonUserList, int *logonUserCnt ,int maxLogonUserCnt,int maxLogonUserNameLen);

	BOOL app_os_OpenProcessToken (
		__in        HANDLE ProcessHandle,
		__in        DWORD DesiredAccess,
		__deref_out PHANDLE TokenHandle
		);

	HANDLE	app_os_OpenProcess(
		__in DWORD dwDesiredAccess,
		__in BOOL bInheritHandle,
		__in DWORD dwProcessId
		);

	HANDLE app_os_CreateToolhelp32Snapshot(
		DWORD dwFlags,
		DWORD th32ProcessID
		);

	BOOL app_os_TerminateProcess(
		__in HANDLE hProcess,
		__in UINT uExitCode
		);

	HMODULE app_os_LoadLibraryExA(
		__in       LPCSTR lpLibFileName,
		__reserved HANDLE hFile,
		__in       DWORD dwFlags
		);

	DWORD app_os_FormatMessageA(
		__in     DWORD dwFlags,
		__in_opt LPCVOID lpSource,
		__in     DWORD dwMessageId,
		__in     DWORD dwLanguageId,
		__out    LPSTR lpBuffer,
		__in     DWORD nSize,
		__in_opt va_list *Arguments
		);

	BOOL app_os_ReadEventLogA (
		__in  HANDLE     hEventLog,
		__in  DWORD      dwReadFlags,
		__in  DWORD      dwRecordOffset,
		__out_bcount_part(nNumberOfBytesToRead, *pnBytesRead) LPVOID     lpBuffer,
		__in  DWORD      nNumberOfBytesToRead,
		__out DWORD      *pnBytesRead,
		__out DWORD      *pnMinNumberOfBytesNeeded
		);

	HANDLE app_os_OpenEventLogA (
		__in_opt LPCSTR lpUNCServerName,
		__in     LPCSTR lpSourceName
		);


	BOOL app_os_LookupAccountSidA(
		__in_opt LPCSTR lpSystemName,
		__in PSID Sid,
		__out_ecount_part_opt(*cchName, *cchName + 1) LPSTR Name,
		__inout  LPDWORD cchName,
		__out_ecount_part_opt(*cchReferencedDomainName, *cchReferencedDomainName + 1) LPSTR ReferencedDomainName,
		__inout LPDWORD cchReferencedDomainName,
		__out PSID_NAME_USE peUse
		);

	BOOL app_os_CreateProcessAsUserA (
		__in_opt    HANDLE hToken,
		__in_opt    LPCSTR lpApplicationName,
		__inout_opt LPSTR lpCommandLine,
		__in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
		__in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
		__in        BOOL bInheritHandles,
		__in        DWORD dwCreationFlags,
		__in_opt    LPVOID lpEnvironment,
		__in_opt    LPCSTR lpCurrentDirectory,
		__in        LPSTARTUPINFOA lpStartupInfo,
		__out       LPPROCESS_INFORMATION lpProcessInformation
		);

	DWORD app_os_QueryDosDeviceA(
		__in_opt LPCSTR lpDeviceName,
		__out_ecount_part_opt(ucchMax, return) LPSTR lpTargetPath,
		__in     DWORD ucchMax
		);

	DWORD	app_os_GetProcessImageFileNameA (
		__in HANDLE hProcess,
		__out_ecount(nSize) LPSTR lpImageFileName,
		__in DWORD nSize
		);


	BOOL app_os_GlobalMemoryStatusEx(
		__out LPMEMORYSTATUSEX lpBuffer
		);


	BOOL app_os_FileTimeToLocalFileTime(
		__in  CONST FILETIME *lpFileTime,
		__out LPFILETIME lpLocalFileTime
		);

	BOOL app_os_FileTimeToSystemTime(
		__in  CONST FILETIME *lpFileTime,
		__out LPSYSTEMTIME lpSystemTime
		);


	LSTATUS app_os_RegOpenKeyExA (
		__in HKEY hKey,
		__in_opt LPCSTR lpSubKey,
		__reserved DWORD ulOptions,
		__in REGSAM samDesired,
		__out PHKEY phkResult
		);

	LSTATUS app_os_RegQueryValueExA (
		__in HKEY hKey,
		__in_opt LPCSTR lpValueName,
		__reserved LPDWORD lpReserved,
		__out_opt LPDWORD lpType,
		__out_bcount_part_opt(*lpcbData, *lpcbData) __out_data_source(REGISTRY) LPBYTE lpData,
		__inout_opt LPDWORD lpcbData
		);

	LSTATUS	app_os_RegCloseKey (__in HKEY hKey	);

	DWORD app_os_ExpandEnvironmentStringsA(
		__in LPCSTR lpSrc,
		__out_ecount_part_opt(nSize, return) LPSTR lpDst,
		__in DWORD nSize
		);

	BOOL app_os_LookupPrivilegeValueA(
		__in_opt LPCSTR lpSystemName,
		__in     LPCSTR lpName,
		__out    PLUID   lpLuid
		);

	BOOL app_os_AdjustTokenPrivileges (
		__in      HANDLE TokenHandle,
		__in      BOOL DisableAllPrivileges,
		__in_opt  PTOKEN_PRIVILEGES NewState,
		__in      DWORD BufferLength,
		__out_bcount_part_opt(BufferLength, *ReturnLength) PTOKEN_PRIVILEGES PreviousState,
		__out_opt PDWORD ReturnLength
		);

	VOID app_os_GetSystemInfo(
		__out LPSYSTEM_INFO lpSystemInfo
		);


	BOOL app_os_GetProcessTimes(
		__in  HANDLE hProcess,
		__out LPFILETIME lpCreationTime,
		__out LPFILETIME lpExitTime,
		__out LPFILETIME lpKernelTime,
		__out LPFILETIME lpUserTime
		);


	DWORD app_os_GetWindowThreadProcessId(
		__in HWND hWnd,
		__out_opt LPDWORD lpdwProcessId);

	BOOL app_os_IsWindowVisible(
		__in HWND hWnd);


	BOOL app_os_GetSystemTimes(
		__out_opt LPFILETIME lpIdleTime,
		__out_opt LPFILETIME lpKernelTime,
		__out_opt LPFILETIME lpUserTime
		);

	HANDLE app_os_CreateFileA(
		__in     LPCSTR lpFileName,
		__in     DWORD dwDesiredAccess,
		__in     DWORD dwShareMode,
		__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		__in     DWORD dwCreationDisposition,
		__in     DWORD dwFlagsAndAttributes,
		__in_opt HANDLE hTemplateFile
		);

	BOOL app_os_DeleteFileA(__in LPCSTR lpFileName);

	int app_os_GetFileLineCount(const char * pFileName);

	BOOL app_os_GetDefProgramFilesPath(char *pProgramFilesPath);

	BOOL app_os_GetDiskPatitionSizeBytes(char *partName, LONGLONG *freeSpaceSizeToCaller, LONGLONG *totalSpaceSize);

	DWORD app_os_StartSrv(char * srvName);

	BOOL app_os_FileAppend(const char *srcFilePath, const char * destFilePath);

	BOOL app_os_ProcessCheck(char * processName);

	BOOL app_os_GetDiskSizeBytes(unsigned int diskNum, LONGLONG *outSize);

	BOOL app_os_FileCopy(const char * srcFilePath, const char * destFilePath);

	int app_os_GetDiskPartitionCnt();

	BOOL app_os_GetMinAndMaxSpaceMB( LONGLONG *minSpace, LONGLONG *maxSpace);

	DWORD app_os_GetSrvStatus(char * srvName);

	BOOL app_os_FindCloseChangeNotification(__in HANDLE hChangeHandle);

	LSTATUS	app_os_RegDeleteValueA (
		__in HKEY hKey,
		__in_opt LPCSTR lpValueName
		);

	LSTATUS app_os_RegSetValueExA (
		__in HKEY hKey,
		__in_opt LPCSTR lpValueName,
		__reserved DWORD Reserved,
		__in DWORD dwType,
		__in_bcount_opt(cbData) CONST BYTE* lpData,
		__in DWORD cbData
		);

	BOOL app_os_GetDiskSizeBytes(unsigned int diskNum, LONGLONG *outSize);

	DWORD app_os_MprConfigServerConnect(
		__in IN      LPWSTR                  lpwsServerName,
		OUT     HANDLE*                 phMprConfig
		);

	DWORD app_os_MprConfigGetFriendlyName(
		IN      HANDLE                  hMprConfig,
		__in IN      PWCHAR                  pszGuidName,
		__out_bcount(dwBufferSize) OUT     PWCHAR                  pszBuffer,
		IN      DWORD                   dwBufferSize);

	/*DWORD	app_os_GetIfTable(
		OUT    PMIB_IFTABLE pIfTable,
		IN OUT PULONG       pdwSize,
		IN     BOOL         bOrder
		);

	ULONG	app_os_GetAdaptersInfo(
		IN PIP_ADAPTER_INFO AdapterInfo, 
		IN OUT PULONG SizePointer
		);*/
	BOOL app_os_CloseSnapShot32Handle(HANDLE hSnapshot);

	char *app_os_itoa(int value, char *dest, int radix);

#ifdef __cplusplus
}
#endif


#endif


