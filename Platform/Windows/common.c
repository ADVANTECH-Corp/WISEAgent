#include "common.h"
#include "platform.h"
#include <process.h>
#include <string.h>
#include <tlhelp32.h>
#include <ifmib.h>
#define DIV                       (1024)
#include <PowrProf.h>
//#include <IPHlpApi.h>
//#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"PowrProf.lib")
//#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Mprapi.lib")



// HMODULE == HINSTANCE  == HANDLE == PVOID = void *
void* app_load_library(const char* path )
{
	void* lib = NULL;
	SetErrorMode(SEM_FAILCRITICALERRORS);
	lib = LoadLibrary(path); // HMODULE  WINAPI LoadLibrary( LPCTSTR lpFileName );
	SetErrorMode((UINT)NULL);
	return lib;
}

int app_free_library(void * handle )
{
	BOOL bRet = FreeLibrary((HMODULE)handle); // HMODULE  WINAPI LoadLibrary( LPCTSTR lpFileName );
	if(bRet == FALSE)
		return -1;
	else
		return 0;
}

void* app_get_proc_address( void * handle, const char *name )
{
	return (void*) GetProcAddress( (HMODULE)handle, name ); // FARPROC WINAPI GetProcAddress( HMODULE hModule, LPCSTR lpProcName ); 
}

char * app_load_error()
{
	LPSTR lpMsgBuf;
	DWORD dw = GetLastError(); 
	char* result = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) & lpMsgBuf,
		0, NULL );

	// Display the error message and exit the process
	result = (char*)malloc(strlen(lpMsgBuf)+1);
	memset(result, 0, strlen(lpMsgBuf)+1);
	sprintf(result,"%s",lpMsgBuf);

	LocalFree(lpMsgBuf);

	return result;
}

void app_os_sleep(unsigned int const timeoutMs)
{
	//Add code 
	Sleep(timeoutMs);
	return;
}

int app_os_thread_create(CAGENT_THREAD_HANDLE * threadHandle, CAGENT_PTHREAD_ENTRY_POINTER(threadStart), void *args)
{
	int iRet = 0;
	//Add code 
	*threadHandle = (HANDLE)CreateThread(NULL, 0, threadStart, args, 0, NULL);
	if(!(*threadHandle)) iRet = -1;
	return iRet;
}

int app_os_thread_join(CAGENT_THREAD_HANDLE threadHandle)
{
	int iRet = 0;
	if(!threadHandle) return -1;
	if(WaitForSingleObject(threadHandle, INFINITE) != WAIT_OBJECT_0) iRet = -1;
	CloseHandle(threadHandle);
	return iRet;
}

int app_os_thread_self(CAGENT_THREAD_ID * threadId)
{
	int iRet = 0;
	*threadId = GetCurrentThreadId();
	return iRet;
}

int app_os_thread_equal(CAGENT_THREAD_ID threadIdA, CAGENT_THREAD_ID threadIdB)
{
	int iRet = 0;
	if(threadIdA != threadIdB) iRet = -1;
	return iRet;
}

int app_os_thread_cancel(CAGENT_THREAD_HANDLE threadHandle)
{
	int iRet = 0;
	if(!TerminateThread(threadHandle , 0)) iRet = -1;
	return iRet;
}

void app_os_thread_exit(unsigned long code)
{
	//Add code 
	ExitThread(code);
	return;
}

void app_os_thread_detach(CAGENT_THREAD_HANDLE threadHandle)
{
	CloseHandle(threadHandle);
}

int app_os_mutex_setup(CAGENT_MUTEX_TYPE * mutex)
{
	int iRet = 0;
	//Add code 
	*mutex = CreateMutex(NULL, FALSE, NULL);
	if(!(*mutex)) iRet =  -1;
	return iRet;
}

int app_os_mutex_lock(CAGENT_MUTEX_TYPE * mutex)
{
	int iRet = 0;
	//Add code
	if (* mutex == NULL) 
	{
		iRet = -1;
	}
	else
	{
		if(WaitForSingleObject(*mutex, INFINITE) == WAIT_FAILED) iRet = -1;
	}
	return iRet;
}

int app_os_mutex_unlock(CAGENT_MUTEX_TYPE * mutex)
{
	int iRet = 0;
	//Add code 
	if(!ReleaseMutex(*mutex)) iRet = -1;
	return iRet;
}

int app_os_mutex_cleanup(CAGENT_MUTEX_TYPE * mutex)
{
	int iRet = 0;
	//Add code
	if(*mutex)
	{
		CloseHandle(*mutex);
		*mutex = NULL;
	}
	return iRet;
}

int app_os_cond_setup(CAGENT_COND_TYPE * cond)
{
	int iRet = 0;
	//Add code 
	cond->waiters_count = 0;
	cond->signal_event = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
	InitializeCriticalSection(&cond->waiters_count_lock);
	cond->generation = 0;
	cond->num_wake = 0;
	return iRet;
}

int app_os_cond_signal(CAGENT_COND_TYPE * cond)
{
	int iRet = 0;
	//Add code
	unsigned int wake = 0;
	EnterCriticalSection(&cond->waiters_count_lock);
	if (cond->waiters_count > cond->num_wake) 
	{
		wake = 1;
		cond->num_wake++;
		cond->generation++;
	}
	LeaveCriticalSection(&cond->waiters_count_lock);

	if (wake) 
	{
		ReleaseSemaphore(cond->signal_event, 1, NULL);
	}
	return iRet;
}

static int thread_cond_timedwait(CAGENT_COND_TYPE *cond, CAGENT_MUTEX_TYPE *mutex, 
								 unsigned long timeout_ms) 
{
	DWORD res;
	int rv;
	unsigned int wake = 0;
	unsigned long generation;

	EnterCriticalSection(&cond->waiters_count_lock);
	cond->waiters_count++;
	generation = cond->generation;
	LeaveCriticalSection(&cond->waiters_count_lock);

	app_os_mutex_unlock(mutex);

	do 
	{
		res = WaitForSingleObject(cond->signal_event, timeout_ms);

		EnterCriticalSection(&cond->waiters_count_lock);

		if (cond->num_wake) 
		{
			if (cond->generation != generation) 
			{
				cond->num_wake--;
				cond->waiters_count--;
				rv = 0;
				break;
			} 
			else 
			{
				wake = 1;
			}
		}
		else if (res != WAIT_OBJECT_0) 
		{
			cond->waiters_count--;
			rv = 1;
			break;
		}

		LeaveCriticalSection(&cond->waiters_count_lock);

		if (wake) 
		{
			wake = 0;
			ReleaseSemaphore(cond->signal_event, 1, NULL);
		}
	} while (1);

	LeaveCriticalSection(&cond->waiters_count_lock);
	app_os_mutex_lock(mutex);
	return rv;
}

int app_os_cond_broadcast(CAGENT_COND_TYPE * cond)
{
	int iRet = 0;
	//Add code
	unsigned long num_wake = 0;

	EnterCriticalSection(&cond->waiters_count_lock);
	if (cond->waiters_count > cond->num_wake) 
	{
		num_wake = cond->waiters_count -cond->num_wake;
		cond->num_wake = cond->waiters_count;
		cond->generation++;
	}
	LeaveCriticalSection(&cond->waiters_count_lock);

	if (num_wake) 
	{
		ReleaseSemaphore(cond->signal_event, num_wake, NULL);
	}
	return iRet;
}

int app_os_cond_wait(CAGENT_COND_TYPE * cond, CAGENT_MUTEX_TYPE *mutex)
{
	int iRet = 0;
	//Add code 
	iRet = thread_cond_timedwait(cond, mutex, INFINITE);
	return iRet;
}

int app_os_cond_timed_wait(CAGENT_COND_TYPE * cond, CAGENT_MUTEX_TYPE *mutex, int *ms)
{
	int iRet = 0;
	//Add code
	iRet = thread_cond_timedwait(cond, mutex, *ms);
	return iRet;
}

int app_os_cond_cleanup(CAGENT_COND_TYPE * cond)
{
	int iRet = 0;
	//Add code
	if(cond->signal_event)
	{
		CloseHandle(cond->signal_event);
		cond->signal_event = NULL;
	}
	DeleteCriticalSection(&cond->waiters_count_lock);
	return iRet;
}

int app_os_get_process_id(void)
{
	return _getpid();
}

int app_os_get_sysmem_usage_kb(long * totalPhysMemKB, long * availPhysMemKB) 
{ 
	MEMORYSTATUSEX memStatex;
	if(NULL == totalPhysMemKB || NULL == availPhysMemKB) return -1;
	memStatex.dwLength = sizeof (memStatex);
	GlobalMemoryStatusEx (&memStatex);

	*totalPhysMemKB = (long)(memStatex.ullTotalPhys/DIV);
	*availPhysMemKB = (long)(memStatex.ullAvailPhys/DIV);

	return 0;
}

int app_os_get_module_path(char * moudlePath)
{
	int iRet = 0;
	char * lastSlash = NULL;
	char tempPath[MAX_PATH] = {0};
	if(ERROR_SUCCESS != GetModuleFileName(NULL, tempPath, sizeof(tempPath)))
	{
		lastSlash = strrchr(tempPath, FILE_SEPARATOR);
		if(NULL != lastSlash)
		{
			if(moudlePath)
				strncpy(moudlePath, tempPath, lastSlash - tempPath + 1);
			iRet = lastSlash - tempPath + 1;
		}
	}
	return iRet;
}

int app_os_get_os_version(char * osVersionBuf,  int bufLen)
{
	int bRet = -1;
	if(osVersionBuf == NULL || bufLen <= 0) return bRet;
	{
		OSVERSIONINFOEX osvInfo;
		bool isUnknow = false;

		memset(&osvInfo, 0, sizeof(OSVERSIONINFOEX));
		osvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		bRet = GetVersionEx((LPOSVERSIONINFO)&osvInfo);
		if(!bRet) return bRet;
		switch(osvInfo.dwPlatformId)
		{
		case VER_PLATFORM_WIN32_WINDOWS:
			{
				if( 4 == osvInfo.dwMajorVersion )
				{
					switch(osvInfo.dwMinorVersion)
					{
					case 0:
						{
							memcpy(osVersionBuf, OS_WINDOWS_95, sizeof(OS_WINDOWS_95));
							break;
						}
					case 10:
						{
							memcpy(osVersionBuf, OS_WINDOWS_98, sizeof(OS_WINDOWS_98));
							break;
						}
					case 90:
						{
							memcpy(osVersionBuf, OS_WINDOWS_ME, sizeof(OS_WINDOWS_ME));
							break;
						}
					default:
						{
							isUnknow = true;
							break;
						}
					}
				}
				break;
			}
		case VER_PLATFORM_WIN32_NT:
			{
				switch(osvInfo.dwMajorVersion)
				{
				case 3:
					{
						memcpy(osVersionBuf, OS_WINDOWS_NT_3_51, sizeof(OS_WINDOWS_NT_3_51));
						break;
					}
				case 4:
					{
						memcpy(osVersionBuf, OS_WINDOWS_NT_4, sizeof(OS_WINDOWS_NT_4));
						break;
					}
				case 5:
					{
						switch(osvInfo.dwMinorVersion)
						{
						case 0:
							{
								memcpy(osVersionBuf, OS_WINDOWS_2000, sizeof(OS_WINDOWS_2000));
								break;
							}
						case 1:
							{
								memcpy(osVersionBuf, OS_WINDOWS_XP, sizeof(OS_WINDOWS_XP));
								break;
							}
						case 2:
							{
								memcpy(osVersionBuf, OS_WINDOWS_SERVER_2003, sizeof(OS_WINDOWS_SERVER_2003));
								break;
							}
						default:
							{
								isUnknow = true;
								break;
							}
						}
						break;
					}
				case 6:
					{
						switch(osvInfo.dwMinorVersion)
						{
						case 0:
							{
								memcpy(osVersionBuf, OS_WINDOWS_VISTA, sizeof(OS_WINDOWS_VISTA));
								break;
							}
						case 1:
							{
								memcpy(osVersionBuf, OS_WINDOWS_7, sizeof(OS_WINDOWS_7));
								break;
							}
						case 2:
							{
								if(osvInfo.wProductType == VER_NT_WORKSTATION)
								{
									memcpy(osVersionBuf, OS_WINDOWS_8, sizeof(OS_WINDOWS_8));
								}
								else
								{
									memcpy(osVersionBuf, OS_WINDOWS_SERVER_2012, sizeof(OS_WINDOWS_SERVER_2012));
								}
								break;
							}
						case 3:
							{
								if(osvInfo.wProductType == VER_NT_WORKSTATION)
								{
									memcpy(osVersionBuf, OS_WINDOWS_8_1, sizeof(OS_WINDOWS_8_1));
								}
								else
								{
									memcpy(osVersionBuf, OS_WINDOWS_SERVER_2012_R2, sizeof(OS_WINDOWS_SERVER_2012_R2));
								}
								break;
							}
						default:
							{
								isUnknow = true;
								break;
							}
						}
						break;
					}
				default:
					{
						isUnknow = true;
						break;
					}
				}
				break;
			}
		default: 
			{
				isUnknow = true;
				break;
			}
		}
		if(isUnknow) memcpy(osVersionBuf, OS_UNKNOW, sizeof(OS_UNKNOW));
		else
		{
			sprintf_s(osVersionBuf, bufLen, "%s %s", osVersionBuf, osvInfo.szCSDVersion);
			bRet = 0;
		}
	}

	return bRet;
}

int app_os_get_processor_name(char * processorNameBuf, unsigned long * bufLen)
{
	int bRet = -1;
	HKEY hk;
	char regName[] = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
	char valueName[] = "ProcessorNameString";
	char valueData[256] = {0};    
	unsigned long  valueDataSize = sizeof(valueData);
	if(processorNameBuf == NULL || bufLen == NULL || *bufLen <= 0) return bRet;
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_ALL_ACCESS, &hk)) return bRet;
	else
	{
		if(ERROR_SUCCESS != RegQueryValueEx(hk, valueName, 0, NULL, (LPBYTE)valueData, &valueDataSize)) 
		{
			RegCloseKey(hk);
			return bRet;
		}
		RegCloseKey(hk);
		bRet = valueDataSize == 0 ? -1 : 0;
	}
	if(bRet == 0)
	{
		unsigned int cpyLen = valueDataSize < *bufLen ? valueDataSize : *bufLen; 
		memcpy(processorNameBuf, valueData, cpyLen);
		*bufLen = cpyLen;
	}
	return bRet;
}

int app_os_get_architecture(char * osArchBuf, int bufLen)
{
	int bRet = -1;
	if(osArchBuf == NULL || bufLen <= 0) return bRet;
	{
		SYSTEM_INFO siSysInfo;
		typedef void (WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO);
		LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress(GetModuleHandle("kernel32"), "GetNativeSystemInfo");
		if (NULL != fnGetNativeSystemInfo)
		{
			fnGetNativeSystemInfo(&siSysInfo);
		}
		else
		{
			GetSystemInfo(&siSysInfo);
		}

		switch(siSysInfo.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
			{
				memcpy(osArchBuf, ARCH_X64, sizeof(ARCH_X64));
				break;
			}
		case PROCESSOR_ARCHITECTURE_ARM:
			{
				memcpy(osArchBuf, ARCH_ARM, sizeof(ARCH_ARM));
				break;
			}
		case PROCESSOR_ARCHITECTURE_IA64:
			{
				memcpy(osArchBuf, ARCH_IA64, sizeof(ARCH_IA64));
				break;
			}
		case PROCESSOR_ARCHITECTURE_INTEL:
			{
				memcpy(osArchBuf, ARCH_X86, sizeof(ARCH_X86));
				break;
			}
		case PROCESSOR_ARCHITECTURE_UNKNOWN:
			{
				memcpy(osArchBuf, ARCH_UNKNOW, sizeof(ARCH_UNKNOW));
				break;
			}
		}
		bRet = 0;
	}
	return bRet;
}

void split_path_file(char const *filepath, char* path, char* file) 
{
	const char *slash = filepath, *next;
	while ((next = strpbrk(slash + 1, "\\/"))) slash = next;
	if (filepath != slash) slash++;
	strncpy(path, filepath, slash - filepath);
	strcpy(file, slash);
}

void path_combine(char* destination, const char* path1, const char* path2)
{
	if(path1 == NULL && path2 == NULL) {
		strcpy(destination, "");
	}
	else if(path2 == NULL || strlen(path2) == 0) {
		strcpy(destination, path1);
	}
	else if(path1 == NULL || strlen(path1) == 0) {
		strcpy(destination, path2);
	} 
	else {
		char directory_separator[] = {FILE_SEPARATOR, 0};
		const char *last_char, *temp = path1;
		const char *skip_char = path2;
		int append_directory_separator = 0;

		while(*temp != '\0')
		{
			last_char = temp;
			temp++;        
		}

		if(strcmp(last_char, directory_separator) != 0) {
			append_directory_separator = 1;
		}
		strncpy(destination, path1, strlen(path1));
		if(append_directory_separator)
		{
			if(strncmp(path2, directory_separator, strlen(directory_separator) != 0))
				strncat(destination, directory_separator, strlen(directory_separator));
		}
		else
		{
			if(*skip_char == FILE_SEPARATOR)
				skip_char++;   
		}
		strncat(destination, skip_char, strlen(skip_char));
	}
}

int app_os_get_temppath(char* lpBuffer, int nBufferLength)
{
	return GetTempPath (nBufferLength, lpBuffer);
}

BOOL adjust_privileges() 
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	TOKEN_PRIVILEGES oldtp;
	DWORD dwSize=sizeof(TOKEN_PRIVILEGES);
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
	{
		if (GetLastError()==ERROR_CALL_NOT_IMPLEMENTED) return TRUE;
		else return FALSE;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) 
	{
		CloseHandle(hToken);
		return FALSE;
	}

	memset(&tp, 0, sizeof(tp));
	tp.PrivilegeCount=1;
	tp.Privileges[0].Luid=luid;
	tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize)) 
	{
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hToken);
	return TRUE;
}

int app_os_kill_process_name(char * processName)
{
	int iRet = -1;
	PROCESSENTRY32 pe;
	HANDLE hSnapshot=NULL;
	if(NULL == processName) return iRet;
	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe.dwSize=sizeof(PROCESSENTRY32);
	if(!Process32First(hSnapshot,&pe))
	{
		CloseHandle(hSnapshot);
		return iRet;
	}
	while(true)
	{
		pe.dwSize=sizeof(PROCESSENTRY32);
		if(Process32Next(hSnapshot,&pe)==false)
			break;
		if(strcmp(pe.szExeFile,processName)==0)
		{
			HANDLE hPrc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
			if(hPrc == NULL) 
			{
				DWORD dwRet = GetLastError();          
				if(dwRet == 5)
				{
					if(adjust_privileges())
					{
						hPrc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
					}
				}
			}

			if(hPrc)
			{
				TerminateProcess(hPrc, 0);    //asynchronous
				WaitForSingleObject(hPrc, 5000);
				iRet = 0;
				CloseHandle(hPrc);
			}

			break;
		}
	}
	CloseHandle(hSnapshot);
	return iRet;
}

int app_os_launch_process(char * appPath)
{
	//char cmdLine[BUFSIZ] = {0};
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if(NULL == appPath) return -1;

	memset(&si, 0, sizeof(si));
	si.dwFlags = STARTF_USESHOWWINDOW;  
	si.wShowWindow = SW_HIDE;
	//si.wShowWindow = SW_SHOW;
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	//sprintf(cmdLine, "%s \"%s\" ", "cmd.exe /c ", appPath);

	//if(RunProcessAsUser(updaterPath, TRUE, FALSE, NULL))
	if(CreateProcess(appPath, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW , NULL, NULL, &si, &pi))
		//if(CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW , NULL, NULL, &si, &pi))
		//if(CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int app_os_create_directory(char* path)
{
	return CreateDirectory(path,NULL)==TRUE?0:-1;
}

int app_os_set_executable(char* path)
{
	return 0;
}

int app_os_get_iso_date(char* datetime, int length)
{
	size_t result = 0;
	time_t clock;
	struct tm *tm;
	time(&clock);
	tm = gmtime(&clock);

	if(datetime == NULL) return 0;

	result = strftime(datetime, length, "%Y-%m-%dT%H:%M:%S%z", tm);
	return result;
}

//----------------------dhl add S--------------------------------------
int app_os_PowerOff()
{
	int iRet = 0;
	char cmdLine[BUFSIZ] = {0};
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.dwFlags = STARTF_USESHOWWINDOW;  
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	memset(cmdLine, 0, sizeof(cmdLine));
	sprintf(cmdLine, "%s", "cmd.exe /c shutdown /s /f /t 0");
	if(!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW , NULL, NULL, &si, &pi))
	{
		iRet = -1;
	}
	return iRet;
}

int app_os_PowerRestart()
{
	int iRet = 0;
	char cmdLine[BUFSIZ] = {0};
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.dwFlags = STARTF_USESHOWWINDOW;  
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	memset(cmdLine, 0, sizeof(cmdLine));
	sprintf(cmdLine, "%s", "cmd.exe /c shutdown /r /f /t 0");
	if(!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW , NULL, NULL, &si, &pi))
	{
		iRet = -1;
	}
	return iRet;
}

void app_os_disableResumePassword()
{
	char cmdLine[BUFSIZ] = {0};
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.dwFlags = STARTF_USESHOWWINDOW;  
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	memset(cmdLine, 0, sizeof(cmdLine));
	sprintf(cmdLine, "%s", "cmd.exe /c powercfg /globalpowerflag off /option:resumepassword");
	if(CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW , NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

BOOL app_os_IsPwrSuspendAllowed()
{
	BOOL bRet = FALSE;
	bRet = IsPwrSuspendAllowed();
	return bRet;
}

BOOL app_os_IsPwrHibernateAllowed()
{
	BOOL bRet = FALSE;
	bRet = IsPwrHibernateAllowed();
	return bRet;
}

BOOL app_os_PowerSuspend()
{
	BOOL bRet = FALSE;
	bRet = SetSuspendState(FALSE, FALSE, FALSE);
	return bRet;
}

BOOL app_os_PowerHibernate()
{
	BOOL bRet = FALSE;
	bRet = SetSuspendState(TRUE, FALSE, FALSE);
	return bRet;
}

//BOOL app_os_SAWakeOnLan(char * mac, int size)
//{
//	BOOL bRet = FALSE;
//	if(size < 6) return bRet;
//	{
//		unsigned char packet[102];
//		struct sockaddr_in addr;
//		SOCKET sockfd;
//		int i = 0, j = 0;
//		BOOL optVal = TRUE;
//		WSADATA wsaData;
//		WSAStartup(MAKEWORD(2, 2), &wsaData);
//
//		for(i=0;i<6;i++)  
//		{
//			packet[i] = 0xFF;  
//		}
//		for(i=1;i<17;i++)
//		{
//			for(j=0;j<6;j++)
//			{
//				packet[i*6+j] = mac[j];
//			}
//		}
//
//		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//		if(sockfd > 0)
//		{
//			int iRet = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,(char *)&optVal, sizeof(optVal));
//			if(iRet == 0)
//			{
//				memset((void*)&addr, 0, sizeof(addr));
//				addr.sin_family = AF_INET;
//				addr.sin_port = htons(9);
//				addr.sin_addr.s_addr=INADDR_BROADCAST;
//				iRet = sendto(sockfd, (const char *)packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr));
//				if(iRet != SOCKET_ERROR) bRet = TRUE;
//			}
//			closesocket(sockfd);
//		}
//		WSACleanup();
//	}
//	return bRet;
//}

// unsigned long long app_os_GetTickCount()
// {
// #if WINVER >= 0x0600
// 	return GetTickCount64();
// #else
// 	return GetTickCount(); /* FIXME - need to deal with overflow. */
// #endif
// }

unsigned long long app_os_GetTickCount()
{
	LARGE_INTEGER qpc;
	LARGE_INTEGER qpf;
	unsigned long long tickMs = 0;
	QueryPerformanceCounter(&qpc);
	QueryPerformanceFrequency(&qpf);
	tickMs = qpc.QuadPart*1000/qpf.QuadPart;
	return tickMs;
}

/*Move to network.c*/
//int app_os_GetAllMacs(char macsStr[][20], int n)
//{
//	int iRet = -1;
//	PIP_ADAPTER_INFO pAdapterInfo;
//	PIP_ADAPTER_INFO pAdapter = NULL;
//	PIP_ADAPTER_INFO pAdInfo = NULL;
//	ULONG            ulSizeAdapterInfo = 0;  
//	DWORD            dwStatus;  
//
//	MIB_IFROW MibRow = {0}; 
//	int macIndex = 0;
//	if(n <= 0) return iRet;
//	ulSizeAdapterInfo = sizeof(IP_ADAPTER_INFO); 
//	pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulSizeAdapterInfo);
//
//	if (GetAdaptersInfo( pAdapterInfo, &ulSizeAdapterInfo) != ERROR_SUCCESS) 
//	{
//		free (pAdapterInfo);
//		pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulSizeAdapterInfo);
//	}
//
//	dwStatus = GetAdaptersInfo(pAdapterInfo,   &ulSizeAdapterInfo);  
//
//	if(dwStatus != ERROR_SUCCESS)  
//	{  
//		free(pAdapterInfo);  
//		return  iRet;  
//	}  
//
//	pAdInfo = pAdapterInfo; 
//	while(pAdInfo)  
//	{  	
//		if(pAdInfo->Type != MIB_IF_TYPE_ETHERNET && pAdInfo->Type != IF_TYPE_IEEE80211) 
//		{
//			pAdInfo = pAdInfo->Next; 
//			continue;
//		}
//
//		memset(&MibRow, 0, sizeof(MIB_IFROW));
//		MibRow.dwIndex = pAdInfo->Index;  
//		MibRow.dwType = pAdInfo->Type;  
//
//		if(GetIfEntry(&MibRow) == NO_ERROR)  
//		{  
//			//if (MibRow.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL)
//			{
//				sprintf_s(macsStr[macIndex], sizeof(macsStr),"%02X:%02X:%02X:%02X:%02X:%02X",
//					MibRow.bPhysAddr[0],
//					MibRow.bPhysAddr[1],
//					MibRow.bPhysAddr[2],
//					MibRow.bPhysAddr[3],
//					MibRow.bPhysAddr[4],
//					MibRow.bPhysAddr[5]);
//				macIndex++;
//				if(macIndex >= n) break;
//			}
//		}
//		pAdInfo = pAdInfo->Next; 
//	}
//	free(pAdapterInfo); 
//	iRet = macIndex;
//	return iRet;
//}
//----------------------dhl add E--------------------------------------

BOOL app_os_is_file_exist(const char * pFilePath)
{
	if(NULL == pFilePath) return FALSE;
	return _access(pFilePath, 0) == 0 ? TRUE : FALSE;
}

BOOL app_os_file_access(const char * pFilePath, int mode)
{
	if(NULL == pFilePath) return FALSE;
	return _access(pFilePath, mode) == 0 ? TRUE : FALSE;
}

BOOL app_os_file_copy(const char * srcFilePath, const char * destFilePath)
{
	BOOL bRet = FALSE;
	FILE *fpSrc = NULL, *fpDest = NULL;
	if(NULL == srcFilePath || NULL == destFilePath) goto done;
	fpSrc = fopen(srcFilePath, "rb");
	if(NULL == fpSrc) goto done;
	fpDest = fopen(destFilePath, "wb");
	if(NULL == fpDest) goto done;
	{
		char buf[BUFSIZ] = {0};
		int size = 0;
		while ((size = fread(buf, 1, BUFSIZ, fpSrc)) != 0) 
		{ 
			fwrite(buf, 1, size, fpDest);
		}
		bRet = TRUE;
	}
done:
	if(fpSrc) fclose(fpSrc);
	if(fpDest) fclose(fpDest);
	return bRet;
}

BOOL app_os_file_copyEx(const char * srcFilePath, const char * destFilePath)
{
	return app_os_file_copy(srcFilePath, destFilePath);
}

void app_os_file_remove(const char * filePath)
{
	if(filePath && strlen(filePath))
	{
		remove(filePath);
	}
}

BOOL app_os_is_workstation_locked()
{
	// note: we can't call OpenInputDesktop directly because it's not
	// available on win 9x
	typedef HDESK (WINAPI *PFNOPENDESKTOP)(LPSTR lpszDesktop, DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess);
	typedef BOOL (WINAPI *PFNCLOSEDESKTOP)(HDESK hDesk);
	typedef BOOL (WINAPI *PFNSWITCHDESKTOP)(HDESK hDesk);

	// load user32.dll once only
	HMODULE hUser32 = LoadLibrary("user32.dll");

	if (hUser32)
	{
		PFNOPENDESKTOP fnOpenDesktop = (PFNOPENDESKTOP)GetProcAddress(hUser32, "OpenDesktopA");
		PFNCLOSEDESKTOP fnCloseDesktop = (PFNCLOSEDESKTOP)GetProcAddress(hUser32, "CloseDesktop");
		PFNSWITCHDESKTOP fnSwitchDesktop = (PFNSWITCHDESKTOP)GetProcAddress(hUser32, "SwitchDesktop");

		if (fnOpenDesktop && fnCloseDesktop && fnSwitchDesktop)
		{
			HDESK hDesk = fnOpenDesktop("Default", 0, FALSE, DESKTOP_SWITCHDESKTOP);

			if (hDesk)
			{
				BOOL bLocked = !fnSwitchDesktop(hDesk);

				// cleanup
				fnCloseDesktop(hDesk);
				FreeLibrary(hUser32);
				return bLocked;
			}
		}
		FreeLibrary(hUser32);
	}

	// must be win9x
	return FALSE;
}

BOOL app_os_process_check(char * processName)
{
	BOOL bRet = FALSE;
	PROCESSENTRY32 pe;
	DWORD id=0;
	HANDLE hSnapshot=NULL;
	if(NULL == processName) return bRet;
	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe.dwSize=sizeof(PROCESSENTRY32);
	if(!Process32First(hSnapshot,&pe))
	{
		CloseHandle(hSnapshot);
		return bRet;
	}
	while(TRUE)
	{
		pe.dwSize=sizeof(PROCESSENTRY32);
		if(Process32Next(hSnapshot,&pe)==FALSE)
			break;
		if(_stricmp(pe.szExeFile,processName)==0)
		{
			id=pe.th32ProcessID;
			bRet = TRUE;
			break;
		}
	}
	CloseHandle(hSnapshot);
	return bRet;
}

BOOL app_os_run_process_as_user(char * cmdLine)
{
	BOOL bRet = FALSE;
	if(NULL == cmdLine) return bRet;
	{
		BOOL isRun = TRUE;
		HANDLE hToken = NULL;
		HANDLE hPrc = NULL;
		PROCESSENTRY32 pe;
		HANDLE hSnapshot=NULL;
		hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		pe.dwSize=sizeof(PROCESSENTRY32);
		if(Process32First(hSnapshot,&pe))
		{
			while(isRun)
			{
				pe.dwSize=sizeof(PROCESSENTRY32);
				isRun = Process32Next(hSnapshot,&pe);
				if(isRun)
				{
					if(_stricmp(pe.szExeFile, "EXPLORER.EXE")==0)
					{	
						hPrc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
						bRet = OpenProcessToken(hPrc,TOKEN_ALL_ACCESS, &hToken);
						/*{
						char userName[64] = {0};
						GetProcessUsername(hPrc, userName, sizeof(userName));
						if(strlen(userName))
						{
						SUSIAgentLog(Normal, "---Cur username: %s---", userName);
						}
						}*/
						if(bRet)
						{
							//bRet = ScreenshotAdjustPrivileges(hToken);
							if(bRet)
							{
								STARTUPINFO si;
								PROCESS_INFORMATION pi;
								DWORD dwCreateFlag = CREATE_NO_WINDOW;
								memset(&si, 0, sizeof(si));
								si.dwFlags = STARTF_USESHOWWINDOW; 
								si.wShowWindow = SW_HIDE;
								si.cb = sizeof(si);
								memset(&pi, 0, sizeof(pi));
								bRet = FALSE;
								if(CreateProcessAsUser(hToken, NULL, cmdLine, NULL ,NULL,
									FALSE, dwCreateFlag, NULL, NULL, &si, &pi))
								{
									bRet = TRUE;
									isRun = FALSE;
									WaitForSingleObject(pi.hProcess, INFINITE);
									CloseHandle(pi.hProcess);
									CloseHandle(pi.hThread);
								}
							}
							CloseHandle(hToken);
						}
						CloseHandle(hPrc);
					}
				}
			}
		}
		if(hSnapshot) CloseHandle(hSnapshot);
	}
	return bRet;
}

unsigned int app_os_strftime(char * _Buf, unsigned int _SizeInBytes, const char * _Format, const struct tm * _Tm)
{
	return strftime(_Buf, _SizeInBytes, _Format, _Tm);
}

BOOL app_os_get_programfiles_path(char *pProgramFilesPath)
{
	BOOL bRet = FALSE;
	HKEY hk;
	char regName[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion";
	if(NULL == pProgramFilesPath) return bRet;

	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_ALL_ACCESS, &hk)) return bRet;
	else
	{
		char valueName[] = "ProgramFilesDir";
		char valueData[MAX_PATH] = {0};
		int  valueDataSize = sizeof(valueData);
		if(ERROR_SUCCESS != RegQueryValueEx(hk, valueName, 0, NULL, (LPBYTE)valueData, (LPDWORD)&valueDataSize)) return bRet;
		else
		{
			strcpy(pProgramFilesPath, valueData);
			bRet = TRUE;
		}
		RegCloseKey(hk);
	}
	return bRet;
}

BOOL app_os_get_os_name(char * pOSNameBuf)
{
	BOOL bRet = FALSE;
	OSVERSIONINFOEX osvInfo;
	BOOL isUnknow = FALSE;
	if(NULL == pOSNameBuf) return bRet;

	memset(&osvInfo, 0, sizeof(OSVERSIONINFOEX));
	osvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	bRet = GetVersionEx((OSVERSIONINFO *)&osvInfo);
	if(!bRet) return bRet;
	switch(osvInfo.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_WINDOWS:
		{
			if( 4 == osvInfo.dwMajorVersion )
			{
				switch(osvInfo.dwMinorVersion)
				{
				case 0:
					{
						memcpy(pOSNameBuf, OS_WINDOWS_95, sizeof(OS_WINDOWS_95));
						break;
					}
				case 10:
					{
						memcpy(pOSNameBuf, OS_WINDOWS_98, sizeof(OS_WINDOWS_98));
						break;
					}
				case 90:
					{
						memcpy(pOSNameBuf, OS_WINDOWS_ME, sizeof(OS_WINDOWS_ME));
						break;
					}
				default:
					{
						isUnknow = TRUE;
						break;
					}
				}
			}
			break;
		}
	case VER_PLATFORM_WIN32_NT:
		{
			switch(osvInfo.dwMajorVersion)
			{
			case 3:
				{
					memcpy(pOSNameBuf, OS_WINDOWS_NT_3_51, sizeof(OS_WINDOWS_NT_3_51));
					break;
				}
			case 4:
				{
					memcpy(pOSNameBuf, OS_WINDOWS_NT_4, sizeof(OS_WINDOWS_NT_4));
					break;
				}
			case 5:
				{
					switch(osvInfo.dwMinorVersion)
					{
					case 0:
						{
							memcpy(pOSNameBuf, OS_WINDOWS_2000, sizeof(OS_WINDOWS_2000));
							break;
						}
					case 1:
						{
							memcpy(pOSNameBuf, OS_WINDOWS_XP, sizeof(OS_WINDOWS_XP));
							break;
						}
					case 2:
						{
							memcpy(pOSNameBuf, OS_WINDOWS_SERVER_2003, sizeof(OS_WINDOWS_SERVER_2003));
							break;
						}
					default:
						{
							isUnknow = TRUE;
							break;
						}
					}
					break;
				}
			case 6:
				{
					switch(osvInfo.dwMinorVersion)
					{
					case 0:
						{
							memcpy(pOSNameBuf, OS_WINDOWS_VISTA, sizeof(OS_WINDOWS_VISTA));
							break;
						}
					case 1:
						{
							memcpy(pOSNameBuf, OS_WINDOWS_7, sizeof(OS_WINDOWS_7));
							break;
						}
					case 2:
						{
							if(osvInfo.wProductType == VER_NT_WORKSTATION)
							{
								memcpy(pOSNameBuf, OS_WINDOWS_8, sizeof(OS_WINDOWS_8));
							}
							else
							{
								memcpy(pOSNameBuf, OS_WINDOWS_SERVER_2012, sizeof(OS_WINDOWS_SERVER_2012));
							}
							break;
						}
					case 3:
						{
							if(osvInfo.wProductType == VER_NT_WORKSTATION)
							{
								memcpy(pOSNameBuf, OS_WINDOWS_8_1, sizeof(OS_WINDOWS_8_1));
							}
							else
							{
								memcpy(pOSNameBuf, OS_WINDOWS_SERVER_2012_R2, sizeof(OS_WINDOWS_SERVER_2012_R2));
							}
							break;
						}
					default:
						{
							isUnknow = TRUE;
							break;
						}
					}
					break;
				}
			default:
				{
					isUnknow = TRUE;
					break;
				}
			}
			break;
		}
	default: 
		{
			isUnknow = TRUE;
			break;
		}
	}
	if(isUnknow) memcpy(pOSNameBuf, OS_UNKNOW, sizeof(OS_UNKNOW));
	return bRet;
}

void * app_os_create_event(BOOL bManualReset, BOOL bInitialState, char * eventName)
{
	void * ret = NULL;
	if(NULL == eventName) return ret;
	ret = CreateEvent(NULL, bManualReset, bInitialState, eventName);
	return ret;
}

BOOL app_os_set_event(void *handle)
{
	return SetEvent(handle);
}

void * app_os_open_event_log(char * sourceName)
{
	void * ret = NULL;
	if(NULL == sourceName) return ret;
	ret = OpenEventLog(NULL, sourceName);
	return ret;
}

BOOL app_os_GetOldestEventLogRecord(void * hEvnetLog, DWORD* oldestRecord)
{
	return GetOldestEventLogRecord(hEvnetLog, oldestRecord);
}

BOOL app_os_GetNumberOfEventLogRecords(void * hEvnetLog, DWORD* numOfRecords)
{
	return GetNumberOfEventLogRecords(hEvnetLog, numOfRecords);
}

DWORD app_os_get_last_error()
{
	return GetLastError();
}

BOOL app_os_ReadEventLog(void * hEventLog, DWORD dwReadFlags, DWORD dwRecordOffset,
						 void * lpBuffer, DWORD nNumberOfBytesToRead, DWORD *pnBytesRead, DWORD *pnMinNumberOfBytesNeeded)
{
	return ReadEventLog(hEventLog, dwReadFlags, dwRecordOffset, lpBuffer, nNumberOfBytesToRead, pnBytesRead, pnMinNumberOfBytesNeeded);
}

DWORD app_os_WaitForSingleObject(void* hHandle, DWORD dwMilliseconds)
{
	return WaitForSingleObject(hHandle, dwMilliseconds);
}

unsigned int app_os_GetSystemDirectory(char* dirBuffer, unsigned int bufSize)
{
	return GetSystemDirectory(dirBuffer, bufSize);
}

BOOL app_os_CloseHandle(CAGENT_HANDLE handle)
{
	return CloseHandle(handle);
}

CAGENT_HANDLE app_os_FindFirstChangeNotification(char * lpPathName, BOOL bWatchSubtree, DWORD dwNotifyFilter)
{
	return FindFirstChangeNotification(lpPathName, bWatchSubtree, dwNotifyFilter);
}

BOOL app_os_FindNextChangeNotification(CAGENT_HANDLE handle)
{
	return FindNextChangeNotification(handle);
}

BOOL app_os_FindCloseChangeNotification(void * hChangeHandle)
{
	return FindCloseChangeNotification(hChangeHandle);
}

BOOL app_os_CloseEventLog(void * hEventHandle)
{
	return CloseEventLog(hEventHandle);
}

int app_os_GetFileLineCount(const char * pFileName)
{
	FILE * fp = NULL;
	char lineBuf[BUFSIZ] = {0};
	char * p = NULL;
	int lineLen = 0;
	int lineCount = 0;
	int isCountAdd = 0;
	if(NULL == pFileName) return -1;

	fp = fopen (pFileName, "rb");
	if(fp)
	{
		while ((lineLen = fread(lineBuf, 1, BUFSIZ, fp)) != 0) 
		{   
			isCountAdd = 1;
			p = lineBuf;
			while ((p = (char*)memchr((void*)p, '\n', (lineBuf + lineLen) - p)))
			{
				++p;
				++lineCount;
				isCountAdd = 0;
			}
			memset(lineBuf, 0, BUFSIZ);
		}
		if(isCountAdd) ++lineCount;
		fclose(fp);
	}
	return lineCount;
}

BOOL app_os_FileIsUnicode(char * fileName)
{
	BOOL bRet = FALSE;
	FILE *fp = NULL;
	unsigned char headBuf[2] = {0};
	if(NULL == fileName) return bRet;
	fp = fopen(fileName, "r");
	if(NULL == fp) return bRet;
	fread(headBuf, sizeof(unsigned char), 2, fp);
	fclose(fp);
	if(headBuf[0] == 0xFF && headBuf[1] == 0xFE) bRet = TRUE;
	return bRet;
}

BOOL app_os_CreateProcess(const char * cmdLine)
{
	BOOL bRet = FALSE;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	if(cmdLine == NULL) return bRet;
	memset(&si, 0, sizeof(si));
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));
	if(CreateProcess(NULL, (char*)cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		bRet = TRUE;
	}
	return bRet;
}

BOOL app_os_get_regLocalMachine_value(char * lpSubKey, char * lpValueName, void*lpValue, DWORD valueSize)
{
	BOOL bRet = FALSE;
	if(lpSubKey == NULL || lpValueName==NULL || lpValue==NULL) return bRet;
	{
		HKEY hk;
		DWORD  valueDataSize = valueSize;
		if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_ALL_ACCESS, &hk)) return bRet;
		else
		{
			if(ERROR_SUCCESS != RegQueryValueEx(hk, lpValueName, 0, NULL, (LPBYTE)lpValue, &valueDataSize)) 
			{
				RegCloseKey(hk);
				return bRet;
			}
			RegCloseKey(hk);
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL app_os_set_regLocalMachine_value(char * lpSubKey, char * lpValueName, void*lpValue, DWORD valueSize)
{
	BOOL bRet = FALSE;
	if(lpSubKey == NULL || lpValueName==NULL || lpValue==NULL) return bRet;
	{
		HKEY hk;
		DWORD  valueDataSize = valueSize;
		if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_ALL_ACCESS, &hk)) return bRet;
		else
		{
			int  valueDataSize = valueSize;
			RegSetValueEx(hk, lpValueName, 0, REG_SZ, (BYTE*)lpValue, valueDataSize);
			RegCloseKey(hk);
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL app_os_TerminateThread(void * hThrHandle, DWORD dwExitCode)
{
	return TerminateThread(hThrHandle, dwExitCode);
}

BOOL app_os_GetGUID(char * guidBuf, int bufSize)
{
	BOOL bRet = FALSE;
	GUID guid;
	if(NULL == guidBuf || bufSize <= 0) return bRet;
	if(S_OK == CoCreateGuid(&guid))
	{
		_snprintf(guidBuf, bufSize
			, "%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X"
			, guid.Data1, guid.Data2, guid.Data3, guid.Data4[0]
		, guid.Data4[1] , guid.Data4[2], guid.Data4[3], guid.Data4[4]
		, guid.Data4[5]	, guid.Data4[6], guid.Data4[7] );
		bRet = TRUE;
	}
	return bRet;
}

BOOL app_os_Str2Tm(const char *buf, const char *format, struct tm *tm)
{
	BOOL bRet = FALSE;
	int iYear = 0, iMon = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;
	if(NULL == buf || NULL == format || NULL == tm) return bRet;
	if(sscanf(buf, format, &iYear, &iMon, &iDay, &iHour, &iMin, &iSec) != 0)
	{
		tm->tm_year = iYear - 1900;
		tm->tm_mon = iMon - 1;
		tm->tm_mday = iDay;
		tm->tm_hour = iHour;
		tm->tm_min = iMin;
		tm->tm_sec = iSec;
		tm->tm_isdst = 0;
		bRet = TRUE;
	}
	return bRet;
}

BOOL app_os_Str2Tm_MDY(const char *buf, const char *format, struct tm *tm)
{
	BOOL bRet = FALSE;
	int iYear = 0, iMon = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;
	if(NULL == buf || NULL == format || NULL == tm) return bRet;
	if(sscanf(buf, format, &iMon, &iDay, &iYear, &iHour, &iMin, &iSec) != 0)
	{
		tm->tm_year = iYear - 1900;
		tm->tm_mon = iMon - 1;
		tm->tm_mday = iDay;
		tm->tm_hour = iHour;
		tm->tm_min = iMin;
		tm->tm_sec = iSec;
		tm->tm_isdst = 0;
		bRet = TRUE;
	}
	return bRet;
}

BOOL app_os_Str2Tm_YMD(const char *buf, const char *format, struct tm *tm)
{
	BOOL bRet = FALSE;
	int iYear = 0, iMon = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;
	if(NULL == buf || NULL == format || NULL == tm) return bRet;
	if(sscanf(buf, format, &iYear, &iMon, &iDay,  &iHour, &iMin, &iSec) != 0)
	{
		tm->tm_year = iYear - 1900;
		tm->tm_mon = iMon - 1;
		tm->tm_mday = iDay;
		tm->tm_hour = iHour;
		tm->tm_min = iMin;
		tm->tm_sec = iSec;
		tm->tm_isdst = 0;
		bRet = TRUE;
	}
	return bRet;
}

CAGENT_HANDLE app_os_CreateProcessWithCmdLineEx(const char * cmdLine)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	if(cmdLine == NULL) return NULL;
	memset(&si, 0, sizeof(si));
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));
	if(CreateProcess(NULL, ( char*)cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		//CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return pi.hProcess;
}

CAGENT_HANDLE app_os_CreateProcessWithAppNameEx(const char * appName, char * params)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	if(appName == NULL) return NULL;
	memset(&si, 0, sizeof(si));
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));
	if(CreateProcess(appName, params, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		//CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return pi.hProcess;
}

BOOL app_os_GetExitCodeProcess(void* prcHandle, DWORD * exitCode)
{
	return GetExitCodeProcess(prcHandle, exitCode);
}

CAGENT_HANDLE app_os_RegisterEventSource(char * serverName, char *sourceName)
{
	return RegisterEventSource(serverName, sourceName);
}

BOOL app_os_ReportEvent(void* eventHandle, DWORD dwEventID, char * eventStr)
{
	return ReportEvent(eventHandle, EVENTLOG_INFORMATION_TYPE, 0, dwEventID, NULL, 1, 0, (LPCSTR *)eventStr, NULL);
}

BOOL app_os_DeregisterEventSource(void* eventHandle)
{
	return DeregisterEventSource(eventHandle);
}

BOOL app_os_Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
	return Process32First(hSnapshot, lppe);
}

BOOL app_os_Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
	return Process32Next(hSnapshot, lppe);
}

DWORD app_os_GetLastError(void)
{
	return GetLastError();
}

DWORD app_os_GetSrvStatus(char * srvName)
{
	DWORD dwRet = 0;
	SC_HANDLE hSCM = NULL;
	SC_HANDLE hSrv = NULL;
	if(NULL == srvName) return dwRet;
	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCM) return dwRet;
	hSrv = OpenService(hSCM, srvName, SERVICE_ALL_ACCESS);
	if(hSrv)
	{
		SERVICE_STATUS srvStat;
		memset(&srvStat, 0, sizeof(SERVICE_STATUS));
		QueryServiceStatus(hSrv, &srvStat);
		dwRet = srvStat.dwCurrentState;
	}
	if(hSrv) CloseServiceHandle(hSrv);
	if(hSCM) CloseServiceHandle(hSCM);
	return dwRet;
}

DWORD app_os_StopSrv(char * srvName)
{
	DWORD dwRet = 0;
	SC_HANDLE hSCM = NULL;
	SC_HANDLE hSrv = NULL;
	if(NULL == srvName) return dwRet;
	hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCM) return dwRet;
	hSrv = OpenService(hSCM, srvName, SERVICE_ALL_ACCESS);
	if(hSrv)
	{
		SERVICE_STATUS srvStat;
		memset(&srvStat, 0, sizeof(SERVICE_STATUS));
		QueryServiceStatus(hSrv, &srvStat);

		if(srvStat.dwCurrentState == SERVICE_RUNNING)
		{
			memset(&srvStat, 0, sizeof(SERVICE_STATUS));
			ControlService(hSrv, SERVICE_CONTROL_STOP, &srvStat);
			dwRet = srvStat.dwCurrentState;
			while(dwRet == SERVICE_STOP_PENDING)
			{
				Sleep(250);
				memset(&srvStat, 0, sizeof(SERVICE_STATUS));
				QueryServiceStatus(hSrv, &srvStat);   
				dwRet = srvStat.dwCurrentState;
			}
		}
	}
	if(hSrv) CloseServiceHandle(hSrv);
	if(hSCM) CloseServiceHandle(hSCM);
	return dwRet;
}

BOOL app_os_IsSrvExist(char * srvName)
{
	DWORD dwRet = 0;
	SC_HANDLE hSCM = NULL;
	SC_HANDLE hSrv = NULL;
	if(NULL == srvName) return dwRet;
	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCM) return FALSE;
	hSrv = OpenService(hSCM, srvName, SERVICE_ALL_ACCESS);
	if(NULL == hSrv)
	{
		DWORD dwErrorCode = app_os_GetLastError();
		if(ERROR_SERVICE_DOES_NOT_EXIST == dwErrorCode) return FALSE;
	}
	if(hSrv) CloseServiceHandle(hSrv);
	if(hSCM) CloseServiceHandle(hSCM);
	return TRUE;
}

DWORD app_os_StartSrv(char * srvName)
{
	DWORD dwRet = 0;
	SC_HANDLE hSCM = NULL;
	SC_HANDLE hSrv = NULL;
	if(NULL == srvName) return dwRet;
	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCM) return dwRet;
	hSrv = OpenService(hSCM, srvName, SERVICE_ALL_ACCESS);
	if(hSrv)
	{
		SERVICE_STATUS srvStat;
		memset(&srvStat, 0, sizeof(SERVICE_STATUS));
		QueryServiceStatus(hSrv, &srvStat);
		dwRet = srvStat.dwCurrentState;
		if(dwRet != SERVICE_RUNNING)
		{
			if(dwRet == SERVICE_STOPPED)
			{
				if(StartService(hSrv, 0, NULL))
				{
					memset(&srvStat, 0, sizeof(SERVICE_STATUS));
					QueryServiceStatus(hSrv, &srvStat);
					dwRet = srvStat.dwCurrentState;
				}
			}
		}
	}
	if(hSrv) CloseServiceHandle(hSrv);
	if(hSCM) CloseServiceHandle(hSCM);
	return dwRet;
}


BOOL app_os_GetRandomStr(char *randomStrBuf, int bufSize)
{
	int flag = 0, strLen = 0, i = 0;
	LARGE_INTEGER perfCnt = {0};
	if(NULL == randomStrBuf || bufSize <= 0) return FALSE;

	QueryPerformanceCounter(&perfCnt);
	srand((unsigned int)perfCnt.QuadPart); //us

	//srand((unsigned int)GetTickCount());  //10ms
	//Sleep(11);  //10ms deviation

	//srand((unsigned int)time(NULL)); //sec

	strLen = bufSize - 1;

	while(i < strLen)
	{
		flag = rand()%3;
		switch(flag)
		{
		case 0:
			{
				randomStrBuf[i] = 'a' + rand()%26;
				break;
			}
		case 1:
			{
				randomStrBuf[i] = '0' + rand()%10;
				break;
			}
		case 2:
			{
				randomStrBuf[i] = 'A' + rand()%26;
				break;
			}
		default: break;
		}
		i++;
	}
	randomStrBuf[strLen] = '\0';
	return TRUE; 
}


BOOL app_os_AdjustPrivileges()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	TOKEN_PRIVILEGES oldtp;
	DWORD dwSize=sizeof(TOKEN_PRIVILEGES);
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
	{
		if (app_os_GetLastError()==ERROR_CALL_NOT_IMPLEMENTED) return TRUE;
		else return FALSE;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) 
	{
		app_os_CloseHandle(hToken);
		return FALSE;
	}

	memset(&tp, 0, sizeof(tp));
	tp.PrivilegeCount=1;
	tp.Privileges[0].Luid=luid;
	tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize)) 
	{
		app_os_CloseHandle(hToken);
		return FALSE;
	}

	app_os_CloseHandle(hToken);
	return TRUE;
}


HANDLE app_os_GetProcessHandle(char * processName)
{
	HANDLE hPrc = NULL;
	PROCESSENTRY32 pe;
	HANDLE hSnapshot=NULL;
	if(NULL == processName) return hPrc;
	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe.dwSize=sizeof(PROCESSENTRY32);
	if(!Process32First(hSnapshot,&pe))
		return hPrc;
	while(TRUE)
	{
		pe.dwSize=sizeof(PROCESSENTRY32);
		if(Process32Next(hSnapshot,&pe)==FALSE)
			break;
		if(strcmp(pe.szExeFile, processName)==0)
		{
			hPrc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);

			if(hPrc == NULL) 
			{
				DWORD dwRet = app_os_GetLastError();          
				if(dwRet == 5)
				{
					if(app_os_AdjustPrivileges())
					{
						hPrc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
					}
				}
			}
			break;
		}
	}
	if(hSnapshot) app_os_CloseHandle(hSnapshot);
	return hPrc;
}


BOOL app_os_GetProcessUsername(HANDLE hProcess, char * userNameBuf, int bufLen)
{
	BOOL bRet = FALSE;
	if(hProcess == NULL || userNameBuf == NULL || bufLen <= 0) return bRet;
	{
		HANDLE hToken;
		TOKEN_USER * pTokenUser = NULL;
		if (OpenProcessToken(hProcess,TOKEN_QUERY,&hToken))
		{
			DWORD dwNeedLen = 0;
			GetTokenInformation(hToken, TokenUser, NULL, 0, &dwNeedLen);
			if(dwNeedLen > 0)
			{
				pTokenUser = (TOKEN_USER *)malloc(dwNeedLen);
				if(GetTokenInformation(hToken, TokenUser, pTokenUser, dwNeedLen, &dwNeedLen))
				{
					SID_NAME_USE sn;
					char szDomainName[MAX_PATH] = {0};
					DWORD dwDmLen = MAX_PATH;
					if(LookupAccountSid(NULL, pTokenUser->User.Sid, userNameBuf, (LPDWORD)&bufLen, szDomainName, &dwDmLen, &sn))
					{
						bRet = TRUE;
					}
				}
				if(pTokenUser)
				{
					free(pTokenUser);
					pTokenUser = NULL;
				}
			}
			app_os_CloseHandle(hToken);
		}
	}

	return bRet;
}

//BOOL app_os_GetHostIP(char * ipStr)
//{
//	BOOL bRet = FALSE;
//	if(NULL == ipStr) return bRet;
//	{
//		WSADATA wsaData;
//		char hostName[255] = {0};
//		int i = 0;
//		struct in_addr curAddr;
//		struct hostent *phostent = NULL;
//
//		if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) return bRet;
//
//		if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) return bRet;
//
//		phostent = gethostbyname(hostName);
//		if (phostent == NULL) return bRet;
//
//		for (i = 0; phostent->h_addr_list[i] != 0; ++i) 
//		{
//			memset(&curAddr, 0, sizeof(struct in_addr));
//			memcpy(&curAddr, phostent->h_addr_list[i], sizeof(struct in_addr));
//			strcpy(ipStr, inet_ntoa(curAddr));
//			if(strlen(ipStr) >0)
//			{
//				bRet = TRUE;
//				break;
//			}
//		}
//
//		WSACleanup();
//	}
//	return bRet;
//}

CAGENT_HANDLE app_os_PrepAndLaunchRedirectedChild(CAGENT_HANDLE hChildStdOut, CAGENT_HANDLE hChildStdIn,	CAGENT_HANDLE hChildStdErr)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	// Set up the start up info struct.
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hChildStdOut;
	si.hStdInput  = hChildStdIn;
	si.hStdError  = hChildStdErr;
	// Use this if you want to hide the child:
	//     si.wShowWindow = SW_HIDE;
	// Note that dwFlags must include STARTF_USESHOWWINDOW if you want to
	// use the wShowWindow flags.


	// Launch the process that you want to redirect (in this case,
	// Child.exe). Make sure Child.exe is in the same directory as
	// redirect.c launch redirect from a command line to prevent location
	// confusion.
	if (!CreateProcess(NULL,"Cmd.exe /u",NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi))
		return 0;

	// Close any unnecessary handles.
	if (!CloseHandle(pi.hThread)) return 0;
	// Set global child process handle to cause threads to exit.
	return pi.hProcess;
}

BOOL app_os_CloseHandleEx(CAGENT_HANDLE handle)
{
	return CloseHandle(handle);
}

BOOL app_os_ReadFile(CAGENT_HANDLE fHandle, char * buf, unsigned int nSizeToRead, unsigned int * nSizeRead)
{
	return ReadFile(fHandle, buf, nSizeToRead, (LPDWORD)nSizeRead, NULL);
}

BOOL app_os_WriteFile(CAGENT_HANDLE fHandle, char * buf, unsigned int nSizeToWrite, unsigned int * nSizeWrite)
{
	return WriteFile(fHandle, buf, nSizeToWrite, (LPDWORD)nSizeWrite, NULL);
}

BOOL app_os_CreatePipe(CAGENT_HANDLE * hReadPipe, CAGENT_HANDLE * hWritePipe)
{
	BOOL bRet = FALSE;
	if(hReadPipe == NULL || hWritePipe == NULL) return bRet;
	{
		SECURITY_ATTRIBUTES sa;
		sa.nLength= sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		bRet = CreatePipe(hReadPipe, hWritePipe, &sa,0);
	}
	return bRet;
}

BOOL app_os_DupHandle(CAGENT_HANDLE srcHandle, CAGENT_HANDLE * trgHandle)
{
	BOOL bRet = FALSE;
	if(trgHandle == NULL) return bRet;
	{
		bRet = DuplicateHandle(app_os_GetCurrentProcess(), srcHandle, app_os_GetCurrentProcess(), trgHandle, 0, TRUE,DUPLICATE_SAME_ACCESS); 
	}
	return bRet; 
}

HANDLE app_os_GetCurrentProcess(VOID)
{
	return GetCurrentProcess();
}

BOOL app_os_FreeLibrary (__in HMODULE hLibModule)
{
	return FreeLibrary(hLibModule);
}

BOOL app_os_SystemTimeToFileTime(__in  CONST SYSTEMTIME *lpSystemTime,	__out LPFILETIME lpFileTime)
{
	return SystemTimeToFileTime(lpSystemTime,lpFileTime);
}

LONG app_os_CompareFileTime(__in CONST FILETIME *lpFileTime1,__in CONST FILETIME *lpFileTime2)
{
	return CompareFileTime(lpFileTime1,lpFileTime2);
}

HLOCAL app_os_LocalFree(__deref HLOCAL hMem)
{
	return LocalFree(hMem);
}

VOID app_os_InitializeCriticalSection(__inout LPCRITICAL_SECTION lpCriticalSection)
{
	InitializeCriticalSection(lpCriticalSection);
}

VOID app_os_EnterCriticalSection(__inout LPCRITICAL_SECTION lpCriticalSection)
{
	EnterCriticalSection(lpCriticalSection);
}

VOID app_os_LeaveCriticalSection(__inout LPCRITICAL_SECTION lpCriticalSection)
{
	LeaveCriticalSection(lpCriticalSection);
}

VOID app_os_DeleteCriticalSection(__inout LPCRITICAL_SECTION lpCriticalSection)
{
	DeleteCriticalSection(lpCriticalSection);
}

BOOL app_os_OpenProcessToken (
										__in        HANDLE ProcessHandle,
										__in        DWORD DesiredAccess,
										__deref_out PHANDLE TokenHandle
										)
{
	return OpenProcessToken(ProcessHandle,DesiredAccess,TokenHandle);
}

HANDLE	app_os_OpenProcess(
									 __in DWORD dwDesiredAccess,
									 __in BOOL bInheritHandle,
									 __in DWORD dwProcessId
									 )
{
	return OpenProcess(dwDesiredAccess,bInheritHandle,dwProcessId);
}

HANDLE app_os_CreateToolhelp32Snapshot(
													DWORD dwFlags,
													DWORD th32ProcessID
													)
{
	return CreateToolhelp32Snapshot(dwFlags,th32ProcessID);
}


BOOL app_os_TerminateProcess(
									  __in HANDLE hProcess,
									  __in UINT uExitCode
									  )
{
	return TerminateProcess(hProcess,uExitCode);
}

HMODULE app_os_LoadLibraryExA(
										__in       LPCSTR lpLibFileName,
										__reserved HANDLE hFile,
										__in       DWORD dwFlags
										)
{
	return LoadLibraryExA(lpLibFileName,hFile,dwFlags);
}

DWORD app_os_FormatMessageA(
									 __in     DWORD dwFlags,
									 __in_opt LPCVOID lpSource,
									 __in     DWORD dwMessageId,
									 __in     DWORD dwLanguageId,
									 __out    LPSTR lpBuffer,
									 __in     DWORD nSize,
									 __in_opt va_list *Arguments
									 )
{
	return FormatMessageA(dwFlags,lpSource,dwMessageId,dwLanguageId,lpBuffer,nSize,Arguments);
}

BOOL app_os_ReadEventLogA (
									__in  HANDLE     hEventLog,
									__in  DWORD      dwReadFlags,
									__in  DWORD      dwRecordOffset,
									__out_bcount_part(nNumberOfBytesToRead, *pnBytesRead) LPVOID     lpBuffer,
									__in  DWORD      nNumberOfBytesToRead,
									__out DWORD      *pnBytesRead,
									__out DWORD      *pnMinNumberOfBytesNeeded
						   )
{
	return ReadEventLogA(hEventLog,dwReadFlags,dwRecordOffset,lpBuffer,nNumberOfBytesToRead,pnBytesRead,pnMinNumberOfBytesNeeded);
}

HANDLE app_os_OpenEventLogA (
									  __in_opt LPCSTR lpUNCServerName,
									  __in     LPCSTR lpSourceName
									  )
{
	return OpenEventLogA(lpUNCServerName,lpSourceName);
}

BOOL app_os_LookupAccountSidA(
										__in_opt LPCSTR lpSystemName,
										__in PSID Sid,
										__out_ecount_part_opt(*cchName, *cchName + 1) LPSTR Name,
										__inout  LPDWORD cchName,
										__out_ecount_part_opt(*cchReferencedDomainName, *cchReferencedDomainName + 1) LPSTR ReferencedDomainName,
										__inout LPDWORD cchReferencedDomainName,
										__out PSID_NAME_USE peUse
										)
{
	return LookupAccountSidA(lpSystemName,Sid,Name,cchName,ReferencedDomainName,cchReferencedDomainName,peUse);
}


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
											 )
{
	return CreateProcessAsUserA(hToken,lpApplicationName,lpCommandLine,lpProcessAttributes,lpThreadAttributes,bInheritHandles,
		dwCreationFlags,lpEnvironment,lpCurrentDirectory,lpStartupInfo,lpProcessInformation);
}

DWORD app_os_QueryDosDeviceA(
									  __in_opt LPCSTR lpDeviceName,
									  __out_ecount_part_opt(ucchMax, return) LPSTR lpTargetPath,
									  __in     DWORD ucchMax
									  )
{
	return QueryDosDeviceA(lpDeviceName,lpTargetPath,ucchMax);
}

DWORD	app_os_GetProcessImageFileNameA (
													__in HANDLE hProcess,
													__out_ecount(nSize) LPSTR lpImageFileName,
													__in DWORD nSize
													)
{
	return GetProcessImageFileNameA(hProcess,lpImageFileName,nSize);
}


BOOL app_os_GlobalMemoryStatusEx(
											__out LPMEMORYSTATUSEX lpBuffer
											)
{
	return GlobalMemoryStatusEx(lpBuffer);
}

BOOL app_os_FileTimeToLocalFileTime(
												__in  CONST FILETIME *lpFileTime,
												__out LPFILETIME lpLocalFileTime
												)
{
	return FileTimeToLocalFileTime(lpFileTime,lpLocalFileTime);
}

BOOL app_os_FileTimeToSystemTime(
											__in  CONST FILETIME *lpFileTime,
											__out LPSYSTEMTIME lpSystemTime
											)
{
	return FileTimeToSystemTime(lpFileTime,lpSystemTime);
}

LSTATUS app_os_RegOpenKeyExA (
										__in HKEY hKey,
										__in_opt LPCSTR lpSubKey,
										__reserved DWORD ulOptions,
										__in REGSAM samDesired,
										__out PHKEY phkResult
										)
{
	return RegOpenKeyExA(hKey,lpSubKey,ulOptions,samDesired,phkResult);
}

LSTATUS app_os_RegQueryValueExA (
											__in HKEY hKey,
											__in_opt LPCSTR lpValueName,
											__reserved LPDWORD lpReserved,
											__out_opt LPDWORD lpType,
											__out_bcount_part_opt(*lpcbData, *lpcbData) __out_data_source(REGISTRY) LPBYTE lpData,
											__inout_opt LPDWORD lpcbData
											)
{
	return RegQueryValueExA(hKey,lpValueName,lpReserved,lpType,lpData,lpcbData);
}

LSTATUS	app_os_RegCloseKey (__in HKEY hKey	)
{
	return RegCloseKey(hKey);
}


DWORD app_os_ExpandEnvironmentStringsA(
													__in LPCSTR lpSrc,
													__out_ecount_part_opt(nSize, return) LPSTR lpDst,
													__in DWORD nSize
													)
{
	return ExpandEnvironmentStringsA(lpSrc,lpDst,nSize);
}

BOOL app_os_LookupPrivilegeValueA(
											 __in_opt LPCSTR lpSystemName,
											 __in     LPCSTR lpName,
											 __out    PLUID   lpLuid
											 )
{
	return LookupPrivilegeValueA(lpSystemName,lpName,lpLuid);
}

BOOL app_os_AdjustTokenPrivileges (
											  __in      HANDLE TokenHandle,
											  __in      BOOL DisableAllPrivileges,
											  __in_opt  PTOKEN_PRIVILEGES NewState,
											  __in      DWORD BufferLength,
											  __out_bcount_part_opt(BufferLength, *ReturnLength) PTOKEN_PRIVILEGES PreviousState,
											  __out_opt PDWORD ReturnLength
											  )
{
	return AdjustTokenPrivileges(TokenHandle,DisableAllPrivileges,NewState,BufferLength,PreviousState,ReturnLength);
}

VOID app_os_GetSystemInfo(
								  __out LPSYSTEM_INFO lpSystemInfo
								  )
{
	GetSystemInfo(lpSystemInfo);
}

BOOL app_os_GetProcessTimes(
									 __in  HANDLE hProcess,
									 __out LPFILETIME lpCreationTime,
									 __out LPFILETIME lpExitTime,
									 __out LPFILETIME lpKernelTime,
									 __out LPFILETIME lpUserTime
									 )
{
	return GetProcessTimes(hProcess,lpCreationTime,lpExitTime,lpKernelTime,lpUserTime);
}

DWORD app_os_GetWindowThreadProcessId(
												  __in HWND hWnd,
												  __out_opt LPDWORD lpdwProcessId)
{
	return GetWindowThreadProcessId(hWnd,lpdwProcessId);
}

BOOL app_os_IsWindowVisible(
									 __in HWND hWnd)
{
	return IsWindowVisible(hWnd);
}

BOOL app_os_GetSystemTimes(
									__out_opt LPFILETIME lpIdleTime,
									__out_opt LPFILETIME lpKernelTime,
									__out_opt LPFILETIME lpUserTime
						   )
{
	return GetSystemTimes(lpIdleTime,lpKernelTime,lpUserTime);
}

HANDLE app_os_CreateFileA(
								  __in     LPCSTR lpFileName,
								  __in     DWORD dwDesiredAccess,
								  __in     DWORD dwShareMode,
								  __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								  __in     DWORD dwCreationDisposition,
								  __in     DWORD dwFlagsAndAttributes,
								  __in_opt HANDLE hTemplateFile
								  )
{
	return CreateFile(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
}

BOOL app_os_DeleteFileA(
								__in LPCSTR lpFileName
								)
{
	return DeleteFile(lpFileName);
}


BOOL app_os_GetDefProgramFilesPath(char *pProgramFilesPath)
{
	BOOL bRet = FALSE;
	HKEY hk;
	char regName[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion";
	if(NULL == pProgramFilesPath) return bRet;

	if(ERROR_SUCCESS != app_os_RegOpenKeyExA(HKEY_LOCAL_MACHINE, regName, 0, KEY_ALL_ACCESS, &hk)) return bRet;
	else
	{
		char valueName[] = "ProgramFilesDir";
		char valueData[MAX_PATH] = {0};
		int  valueDataSize = sizeof(valueData);
		if(ERROR_SUCCESS != app_os_RegQueryValueExA(hk, valueName, 0, NULL, (LPBYTE)valueData, (LPDWORD)&valueDataSize)) return bRet;
		else
		{
			strcpy(pProgramFilesPath, valueData);
			bRet = TRUE;
		}
		app_os_RegCloseKey(hk);
	}
	return bRet;
}

BOOL app_os_GetDiskPatitionSizeBytes(char *partName, LONGLONG *freeSpaceSizeToCaller, LONGLONG *totalSpaceSize)
{
	BOOL bRet = FALSE;
	LONGLONG freeSpaceSize = 0;
	if(NULL == partName || NULL == freeSpaceSizeToCaller || NULL == totalSpaceSize) return bRet;

	bRet = GetDiskFreeSpaceEx(partName, (PULARGE_INTEGER)freeSpaceSizeToCaller, (PULARGE_INTEGER)totalSpaceSize, (PULARGE_INTEGER)&freeSpaceSize);

	return bRet;
}

BOOL app_os_ProcessCheck(char * processName)
{
	BOOL bRet = FALSE;
	PROCESSENTRY32 pe;
	DWORD id=0;
	HANDLE hSnapshot=NULL;
	if(NULL == processName) return bRet;
	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe.dwSize=sizeof(PROCESSENTRY32);
	if(!Process32First(hSnapshot,&pe))
	{
		app_os_CloseHandle(hSnapshot);
		return bRet;
	}
	while(TRUE)
	{
		pe.dwSize=sizeof(PROCESSENTRY32);
		if(Process32Next(hSnapshot,&pe)==FALSE)
			break;
		if(_stricmp(pe.szExeFile,processName)==0)
		{
			id=pe.th32ProcessID;
			bRet = TRUE;
			break;
		}
	}
	app_os_CloseHandle(hSnapshot);
	return bRet;
}


BOOL app_os_GetDiskSizeBytes(unsigned int diskNum, LONGLONG *outSize)
{
	BOOL bRet = FALSE;
	char strDiskName[32] = "\\\\.\\PHYSICALDRIVE";
	char strIndex[3] = {'\0'};
	HANDLE hDisk;
	_itoa_s(diskNum, strIndex, sizeof(strIndex), 10);
	strcat(strDiskName, strIndex);

	hDisk = app_os_CreateFileA(strDiskName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hDisk)
	{
		return bRet;
	}

	{
		DWORD dwReturnBytes = 0;
		GET_LENGTH_INFORMATION lenInfo;

		bRet = DeviceIoControl(hDisk, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &lenInfo, sizeof(lenInfo), &dwReturnBytes, NULL);
		if (!bRet)
		{
			if (INVALID_HANDLE_VALUE != hDisk)
			{
				app_os_CloseHandle(hDisk);
			}
			return bRet;
		}
		*outSize = lenInfo.Length.QuadPart;
		bRet = TRUE;
	}

	if (INVALID_HANDLE_VALUE != hDisk)
	{
		app_os_CloseHandle(hDisk);
	}
	return bRet;
}

int app_os_GetDiskPartitionCnt()
{
	int cnt = 0;
	int index = 1;
	int i = 0;
	DWORD dwDrvNum = GetLogicalDrives();
	for(i = 0; i < sizeof(DWORD) * 8; i++)
	{
		if(dwDrvNum & (index << i))
		{
			cnt ++;
		}
	}

	return cnt;
}

BOOL app_os_GetMinAndMaxSpaceMB( LONGLONG *minSpace, LONGLONG *maxSpace)
{
	BOOL bRet = FALSE;
	LONGLONG totalDiskSpace = 0;
	LONGLONG totalAvailableSpace = 0;
	int diskDrvCnt = 0;
	int drvNamesStrLen = 0;
	char * pDrvNamesStr = NULL;
	if(NULL == minSpace || NULL == maxSpace) goto done;
	diskDrvCnt = app_os_GetDiskPartitionCnt();
	drvNamesStrLen = diskDrvCnt*4 + 1;
	pDrvNamesStr = (char *)malloc(drvNamesStrLen);
	memset(pDrvNamesStr, 0, drvNamesStrLen);
	app_os_GetDiskSizeBytes(0, &totalDiskSpace);

	drvNamesStrLen = GetLogicalDriveStrings(drvNamesStrLen, pDrvNamesStr);
	if(!drvNamesStrLen)
	{
		goto done;
	}

	{
		int drvIndex = 0;
		LONGLONG freeSpaceSizeToCaller = 0, totalSpaceSize = 0;
		char * pDrvName = NULL;
		int drvType = DRIVE_UNKNOWN;
		while(drvIndex < diskDrvCnt)
		{
			pDrvName = pDrvNamesStr + drvIndex * 4;
			drvType = GetDriveType(pDrvName);
			if(drvType == DRIVE_FIXED)
			{
				app_os_GetDiskPatitionSizeBytes(pDrvName, &freeSpaceSizeToCaller, &totalSpaceSize);
				if(!strcmp(pDrvName, "C:\\"))
				{
					*minSpace = (totalSpaceSize - freeSpaceSizeToCaller)/1024/1024;
					totalAvailableSpace += (totalSpaceSize - freeSpaceSizeToCaller);
				}
				else
					totalAvailableSpace += totalSpaceSize;
				totalSpaceSize = 0;
				freeSpaceSizeToCaller = 0;
			}
			drvIndex++;
		}
	}

	*maxSpace = (totalDiskSpace - totalAvailableSpace)/1024/1024;
	bRet = TRUE;
done:
	if(pDrvNamesStr) free(pDrvNamesStr);
	return bRet;
}



LSTATUS	app_os_RegDeleteValueA (
											__in HKEY hKey,
											__in_opt LPCSTR lpValueName
											)
{
	return RegDeleteValue(hKey,lpValueName);

}

LSTATUS app_os_RegSetValueExA (
										 __in HKEY hKey,
										 __in_opt LPCSTR lpValueName,
										 __reserved DWORD Reserved,
										 __in DWORD dwType,
										 __in_bcount_opt(cbData) CONST BYTE* lpData,
										 __in DWORD cbData
										 )
{
	return RegSetValueEx(hKey,lpValueName,Reserved,dwType,lpData,cbData);
}


DWORD app_os_MprConfigServerConnect(
												__in IN      LPWSTR                  lpwsServerName,
												OUT     HANDLE*                 phMprConfig
												)
{
	return MprConfigServerConnect(lpwsServerName,phMprConfig);
}

DWORD app_os_MprConfigGetFriendlyName(
												  IN      HANDLE                  hMprConfig,
												  __in IN      PWCHAR                  pszGuidName,
												  __out_bcount(dwBufferSize) OUT     PWCHAR                  pszBuffer,
												  IN      DWORD                   dwBufferSize)
{
	return MprConfigGetFriendlyName(hMprConfig,pszGuidName,pszBuffer,dwBufferSize);
}

//DWORD	app_os_GetIfTable(
//								OUT    PMIB_IFTABLE pIfTable,
//								IN OUT PULONG       pdwSize,
//								IN     BOOL         bOrder
//								)
//{
//	return GetIfTable(pIfTable,pdwSize,bOrder);
//}
//
//ULONG	app_os_GetAdaptersInfo(
//									  IN PIP_ADAPTER_INFO AdapterInfo, 
//									  IN OUT PULONG SizePointer
//									  )
//{
//	return GetAdaptersInfo(AdapterInfo,SizePointer);
//}

BOOL app_os_CloseSnapShot32Handle(HANDLE hSnapshot)
{
	return CloseHandle(hSnapshot);
}

char *app_os_itoa(int value, char *dest, int radix)
{
	return _itoa(value, dest, radix);
}

BOOL app_os_GetSysLogonUserName(char * userNameBuf, unsigned int bufLen)
{
	BOOL bRet = FALSE;
	if(userNameBuf == NULL || bufLen <= 0) return bRet;
	{
		HANDLE eplHandle = NULL;
		eplHandle = app_os_GetProcessHandle("explorer.exe");
		if(eplHandle)
		{
			if(app_os_GetProcessUsername(eplHandle, userNameBuf, bufLen))
			{
				bRet = TRUE;
			}
		}
		if(eplHandle) app_os_CloseHandle(eplHandle);
	}
	return bRet;
}

VOID app_os_GetProcessHandleList(HANDLE *eplHandleList, char *processName, int *logonUserCnt , int maxLogonUserCnt)
{
	HANDLE hPrc = NULL;
	PROCESSENTRY32 pe;
	HANDLE hSnapshot=NULL;
	if(NULL == eplHandleList || NULL == processName || NULL == logonUserCnt) return ;
	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe.dwSize=sizeof(PROCESSENTRY32);
	if(!Process32First(hSnapshot,&pe))
		return ;
	*logonUserCnt = 0;
	while(TRUE)
	{
		pe.dwSize=sizeof(PROCESSENTRY32);
		if(Process32Next(hSnapshot,&pe)==FALSE)
			break;
		if(strcmp(pe.szExeFile, processName)==0)
		{
			hPrc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);

			if(hPrc == NULL) 
			{
				DWORD dwRet = app_os_GetLastError();          
				if(dwRet == 5)
				{
					if(app_os_AdjustPrivileges())
					{
						hPrc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
					}
				}
			}
			if(*logonUserCnt < maxLogonUserCnt)
			{
				eplHandleList[*logonUserCnt] = hPrc;
				(*logonUserCnt)++;
			}
			//break;
		}
	}
	if(hSnapshot) app_os_CloseHandle(hSnapshot);
	//return hPrc;
}

VOID app_os_GetSysLogonUserList(char * logonUserList, int *logonUserCnt ,int maxLogonUserCnt,int maxLogonUserNameLen)
{
	if(logonUserList == NULL || logonUserCnt == NULL) return ;
	{
		//HANDLE eplHandleList[maxLogonUserCnt] = {0};
		HANDLE *eplHandleList = (HANDLE *)malloc(sizeof(HANDLE) * maxLogonUserCnt);
		memset(eplHandleList,0,sizeof(HANDLE) * maxLogonUserCnt);
		app_os_GetProcessHandleList(eplHandleList,"explorer.exe", logonUserCnt ,maxLogonUserCnt);
		if(*logonUserCnt > 0)
		{
			int i = 0;
			for(i = 0; i< *logonUserCnt; i++)
			{
				/*if(app_os_GetProcessUsername(eplHandleList[i], logonUserList[i], maxLogonUserNameLen))
				{
					continue
				}*/
				app_os_GetProcessUsername(eplHandleList[i], &(logonUserList[i*maxLogonUserNameLen]), maxLogonUserNameLen);
				if(eplHandleList[i]) app_os_CloseHandle(eplHandleList[i]);
			}
		}
		free(eplHandleList);
		//if(eplHandle) app_os_CloseHandle(eplHandle);
	}
	return ;
}