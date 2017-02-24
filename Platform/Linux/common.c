
#include "platform.h"
#include "common.h"
#include <ctype.h>
#include <wchar.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/ioctl.h>  
//#include <net/if.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
//#include <sys/types.h>
//#include <arpa/inet.h>
#include <string.h>

#define DIV                       (1024)

//=================================Part of ProtectHandler for Linux===================================

BOOL ResetEvent(HANDLE hEvent)
{
	return TRUE;
}

BOOL NotifyChangeEventLog(HANDLE hEventLog, HANDLE hEvent)
{
	return FALSE;
}

CAGENT_HANDLE app_os_RegisterEventSource(char * serverName, char *sourceName)
{
	/*return RegisterEventSource(serverName, sourceName);*/
	return NULL;
}

BOOL app_os_ReportEvent(void* eventHandle, DWORD dwEventID, char * eventStr)
{
	return FALSE;
}

BOOL app_os_DeregisterEventSource(void* eventHandle)
{
	return FALSE;
}

BOOL app_os_GetExitCodeProcess(void* prcHandle, DWORD * exitCode)
{
	/*return GetExitCodeProcess(prcHandle, exitCode);*/
	return FALSE;
}

CAGENT_HANDLE app_os_CreateProcessWithAppNameEx(const char * appName, char * params)
{
	return NULL;
}

BOOL app_os_GetGUID(char * guidBuf, int bufSize)
{
	BOOL bRet = FALSE;
	if(NULL == guidBuf || bufSize <= 0) return bRet;
	return bRet;
}

BOOL app_os_TerminateThread(void * hThrHandle, DWORD dwExitCode)
{
	/*return TerminateThread(hThrHandle, dwExitCode);*/
	return FALSE;
}

BOOL app_os_set_event(void *handle)
{
	/*return SetEvent(handle);*/
	return FALSE;
}

BOOL app_os_CloseEventLog(void * hEventHandle)
{
	/*return CloseEventLog(hEventHandle);*/
	return FALSE;
}

BOOL app_os_get_regLocalMachine_value(char * lpSubKey, char * lpValueName, void*lpValue, DWORD valueSize)
{
	BOOL bRet = FALSE;
	if(lpSubKey == NULL || lpValueName==NULL || lpValue==NULL) return bRet;
	return bRet;
}

BOOL app_os_set_regLocalMachine_value(char * lpSubKey, char * lpValueName, void*lpValue, DWORD valueSize)
{
	BOOL bRet = FALSE;
	if(lpSubKey == NULL || lpValueName==NULL || lpValue==NULL) return bRet;
	return bRet;
}

unsigned int app_os_GetSystemDirectory(char* dirBuffer, unsigned int bufSize)
{
	/*return GetSystemDirectory(dirBuffer, bufSize);*/
	return -1;
}

void * app_os_open_event_log(char * sourceName)
{
	void * ret = NULL;
	if(NULL == sourceName) return ret;
	/*ret = OpenEventLog(NULL, sourceName);*/
	return ret;
}

void * app_os_create_event(BOOL bManualReset, BOOL bInitialState, char * eventName)
{
	void * ret = NULL;
	if(NULL == eventName) return ret;
	/*ret = CreateEvent(NULL, bManualReset, bInitialState, eventName);*/
	return ret;
}

BOOL app_os_ReadEventLog(void * hEventLog, DWORD dwReadFlags, DWORD dwRecordOffset,\
								 void * lpBuffer, DWORD nNumberOfBytesToRead, \
								 DWORD *pnBytesRead, DWORD *pnMinNumberOfBytesNeeded)
{
	return FALSE;
}

BOOL app_os_GetOldestEventLogRecord(void * hEvnetLog, DWORD* oldestRecord)
{
	return FALSE;
}

BOOL app_os_GetNumberOfEventLogRecords(void * hEvnetLog, DWORD* numOfRecords)
{
	return FALSE;
}
/************************************COMMON****************************************************/
CAGENT_HANDLE app_os_CreateProcessWithCmdLineEx(const char * cmdLine)
{
	if(cmdLine == NULL) return -1;
	pid_t pid = fork();
	if (0 == pid) /* Child process */
	{
		execlp("/bin/sh", "sh", "-c", cmdLine, NULL);
	}
	if (-1 == pid)
		return 0;//Means failed -- like 'FALSE'
	else
		return pid;
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

DWORD app_os_get_last_error()
{
	//return ERR_peek_last_error();
	return errno;
}

BOOL app_os_get_programfiles_path(char *pProgramFilesPath)
{
	return app_os_GetDefProgramFilesPath(pProgramFilesPath);
}

BOOL app_os_get_os_name(char * pOSNameBuf)
{
	if (!app_os_get_os_version(pOSNameBuf,  0))//Its return 0/-1 --> success/failed
		return TRUE;
	else
		return FALSE;
}
//=================================Part of ProtectHandler for Linux===================================

//=================================Part of RecoverHandler for Linux===================================
/*DWORD app_os_GetSrvStatus(char * srvName)
{
	return 0;
}

BOOL app_os_GetDiskSizeBytes(unsigned int diskNum, LONGLONG *outSize)
{
	return TRUE;
}

long  app_os_RegOpenKeyExA(HKEY hKey, const char* lpSubKey, DWORD ulOptions, DWORD samDesired, HKEY *phkResult)
{
	return 0;
}

long app_os_RegDeleteValueA(HKEY hKey, const char *lpValueName)
{
	return 0;
}
long app_os_RegSetValueExA(HKEY hKey, const char *lpValueName, DWORD Reserved, DWORD dwType, const char *lpData, DWORD cbData)
{
	return 0;
}

long RegCreateKeyEx(HKEY hKey,const char *lpSubKey,DWORD Reserved, \
		char *lpClass, DWORD dwOptions,\
		DWORD samDesired, DWORD lpSecurityAttributes,\
		HKEY *phkResult, char * lpdwDisposition)
{
	return 0;
}

long app_os_RegQueryValueExA(HKEY hKey, const char *lpValueName,\
		DWORD *lpReserved, DWORD *lpType,\
		char * lpData, DWORD *lpcbData)
{
	return 0;
}

long app_os_RegCloseKey(HKEY hKey)
{
	return 0;
}

DWORD app_os_StartSrv(char * srvName)
{
	DWORD dwRet = 0;
	return dwRet;
}*/
BOOL app_os_FindNextChangeNotification(CAGENT_HANDLE handle)
{
	return TRUE;
}

CAGENT_HANDLE app_os_FindFirstChangeNotification(char * lpPathName, BOOL bWatchSubtree, DWORD dwNotifyFilter)
{
	return NULL;
}

DWORD app_os_WaitForSingleObject(void* hHandle, DWORD dwMilliseconds)
{
	return 0;
}
BOOL app_os_FindCloseChangeNotification(void * hChangeHandle)
{
	return TRUE;
}
//********************************common******************************
char *getdiskdevname(BOOL exclude_usbdev, char *devname)
{
	char buf1[64] = {0}, buf2[64] = {0}, tab[16][64] = {0};
	char *tdn= devname;
	int cnt = 0;
	FILE *fd = NULL;
	if (!devname) //not NULL
		return NULL;

	if (exclude_usbdev)
	{
		fd = popen("find /dev/disk/by-path/ -type l -iname *usb*part* -print0 | xargs -0 -iD readlink -f D", "r");
		while (fgets(buf1, sizeof(buf1), fd))
		{
			if (strncmp(buf1, tab[cnt-1], 8))// not equal
			{
				strncpy(tab[cnt], buf1, 8);
				cnt++;
				if (cnt >= 16)	break;
			}
		}
		pclose(fd);
	}

	fd = popen("find /dev/disk/by-path/ -type l -iname *part* -print0 | xargs -0 -iD readlink -f D", "r");
	while (fgets(buf1, sizeof(buf1), fd))
	{
		if (strncmp(buf1, buf2, 8))// not equal
		{
			if (exclude_usbdev)
			{
				int i;
				for(i = 0; i < cnt; i++)
				{
					if (!strncmp(buf1, tab[i], 8))
						goto next;
				}
			}
			strncpy(buf2, buf1, 8);
			strncpy(tdn, buf2, 8);
			tdn += 8;
			*tdn++ = ':';
		}
next: ;
	}
	*tdn = '\0';
	pclose(fd);

	return devname;
}

long long getdiskspace(const char *devname)
{
	int dev_fd = 0;
	long long sector_size = 0;
	long long sector_cnt = 0;
	long long total_size_MB = -1;

	dev_fd = open(devname, O_RDONLY);
	if(dev_fd < 0)
	{
		perror("open");
		goto err;
	}
    if (ioctl(dev_fd, BLKSSZGET, &sector_size) < 0)
	{
		perror("ioctl");
		goto err;
	}
    if(ioctl(dev_fd, BLKGETSIZE, &sector_cnt) < 0)
	{
		perror("ioctl");
		goto err;
	}
	total_size_MB = ((sector_cnt >> 20) * sector_size);

err:
	if (dev_fd >= 0)
	{
		close(dev_fd);
	}
	return total_size_MB;
}

BOOL app_os_GetMinAndMaxSpaceMB( LONGLONG *minSpace, LONGLONG *maxSpace)
{
	char buf[BUFSIZ] = {0};
	FILE * fd = popen("df -m", "r");

	if (!minSpace || !maxSpace)	// not NULL
		return FALSE;

	*minSpace = 0, *maxSpace = 0; // clean
	while (fgets(buf, sizeof(buf), fd))
	{
		char *tab[4] = {NULL};
		int i;
		//this filter exclude 'USB flash disk & ASZ'
		if (strstr(buf, "media") || strstr(buf, "ACRONIS SZ"))
			continue;
		tab[0] = strtok(buf, " ");
		for(i = 1; i < 4; i++)
		{
			tab[i] = strtok(NULL, " ");
		}
		*minSpace += atoi(tab[2]);
	}
	pclose(fd);

	if(getdiskdevname(TRUE, buf))//success
	{
		char *tmp = strtok(buf, ":");
		while (tmp)
		{
			*maxSpace += getdiskspace(tmp);
			tmp = strtok(NULL, ":");
		}
	}
	*maxSpace -= *minSpace;
	return TRUE;
}

BOOL app_os_GetDefProgramFilesPath(char *pProgramFilesPath)
{
	//use getcwd() is not safe when run susiaccessagent's symbol link
	//readlink() is standby
	if (getcwd(pProgramFilesPath, MAX_PATH))
		return TRUE;
	else
		return FALSE;
}

BOOL app_os_CreateProcess(const char * cmdLine)
{
	BOOL bRet = FALSE;
	if(cmdLine == NULL) return bRet;
	pid_t pid = fork();
	if (0 == pid) /* Child process */
	{
		execlp("/bin/sh", "sh", "-c", cmdLine, NULL);
	}
	else if (pid > 0)/* Parent process */
	{
		waitpid(pid, NULL, 0);
		bRet = TRUE;
	}
	return bRet;
}

char *app_os_itoa(int value, char *dest, int radix)
{
	//dest is not NULL and radix can't smaller than 2
	if (dest && radix > 1)
	{
		unsigned int uval, i = 0;
		char *ctp = dest;
		char *buf = (char *)malloc(128);

		if ((10 == radix) && (value < 0))
			uval = -value;
		else
			uval = (unsigned int)value;

		if (radix > 10)
		{
			int tmp;
			do{
				if ((tmp = uval % radix) > 9)
					buf[i++] = tmp % 10 + 'A';
				else
					buf[i++] = tmp + '0';
				uval /= radix;
			}while (uval > 0);
		}
		else
		{
			do{
				buf[i++] = uval % radix + '0';
				uval /= radix;
			}while (uval > 0);
		}

		if ((10 == radix) && (value < 0))
			*ctp++ = '-';
		for (; i > 0; i--)
		{
			*ctp++ = buf[i-1];
		}
		*ctp = '\0';

		free(buf);
	}
	return dest;
}

BOOL app_os_ProcessCheck(char * processName)
{
	BOOL isFind = FALSE;
	if (processName && strlen(processName) > 0)
	{
		FILE *fd = NULL;
		char buf[BUFSIZ];

		sprintf(buf, "ps -ely | grep %s | grep -v grep", processName);
		fd = popen(buf, "r");
		while (fgets(buf, sizeof(buf), fd))
		{
			if (strstr(buf, processName) && !strstr(buf, "<defunct>"))//not a zombie process
			{
				isFind = TRUE;
				break;
			}
		}
		pclose(fd);
	}
	return isFind;
}
//=================================Part of RecoverHandler for Linux===================================



BOOL read_proc(PROCESSENTRY32* info,const char* c_pid);
int read_line(FILE* fp,char* buff,int b_l,int l);
//-----------------------------------------------------------------------------
// Internal Used:
//-----------------------------------------------------------------------------
char *trimwhitespace(char *str)
{
  char *end = str + strlen(str) - 1;

  // Trim leading space
  while(str < end && isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}
//-----------------------------------------------------------------------------


// HMODULE == HINSTANCE  == HANDLE == PVOID = void *
void* app_load_library(const char* path )
{
	return dlopen( path, RTLD_LAZY );
}

int app_free_library(void * handle )
{
	return dlclose(handle);
}

void* app_get_proc_address( void * handle, const char *name )
{
	return dlsym( handle, name );
}

char * app_load_error()
{
	char *error = NULL;
	char *err = dlerror();
	if(err) {
	error = malloc(strlen(err)+1);
	memset(error, 0, strlen(err)+1);
	strcpy(error, err);
	}
	return error;
}

void app_os_sleep(unsigned int const timeoutMs)
{
   //Add code 
   unsigned int const timeoutus = timeoutMs * 1000;
   usleep(timeoutus);
   return;
}

int app_os_thread_create(CAGENT_THREAD_HANDLE * threadHandle, CAGENT_PTHREAD_ENTRY_POINTER(threadStart), void *args)
{
   int iRet = 0;
   //Add code 
   iRet = pthread_create(threadHandle, NULL, threadStart, args);
   return iRet;
}

int app_os_thread_join(CAGENT_THREAD_HANDLE threadHandle)
{
   int iRet = 0;
   iRet = pthread_join(threadHandle, NULL);
   return iRet;
}

int app_os_thread_self(CAGENT_THREAD_ID * threadId)
{
   int iRet = 0;
   //Add code
   *threadId = pthread_self();
   return iRet;
}

int app_os_thread_equal(CAGENT_THREAD_ID threadIdA, CAGENT_THREAD_ID threadIdB)
{
   int iRet = 0;
   //Add code
   if(!pthread_equal(threadIdA, threadIdB)) iRet = -1;
   return iRet;
}

int app_os_thread_cancel(CAGENT_THREAD_ID threadId)
{
   int iRet = 0;
   //Add code 
   iRet = pthread_cancel(threadId);
   return iRet;
}

void app_os_thread_exit(unsigned long code)
{
   //Add code 
   pthread_exit(code);
   return;
}

void app_os_thread_detach(CAGENT_THREAD_HANDLE threadHandle)
{
	pthread_detach(threadHandle);
}

int app_os_mutex_setup(CAGENT_MUTEX_TYPE * mutex)
{
   int iRet = 0;
   //Add code 
   iRet = pthread_mutex_init(mutex, NULL);
   return iRet;
}

int app_os_mutex_lock(CAGENT_MUTEX_TYPE * mutex)
{
   int iRet = 0;
   //Add code
   iRet = pthread_mutex_lock(mutex);
   return iRet;
}

int app_os_mutex_unlock(CAGENT_MUTEX_TYPE * mutex)
{
   int iRet = 0;
   //Add code 
   iRet = pthread_mutex_unlock(mutex);
   return iRet;
}

int app_os_mutex_cleanup(CAGENT_MUTEX_TYPE * mutex)
{
   int iRet = 0;
   //Add code
   iRet = pthread_mutex_destroy(mutex);
   return iRet;
}

int app_os_cond_setup(CAGENT_COND_TYPE * cond)
{
   int iRet = 0;
   //Add code 
   iRet = pthread_cond_init(cond, NULL);
   return iRet;
}

int app_os_cond_signal(CAGENT_COND_TYPE * cond)
{
   int iRet = 0;
   //Add code
   iRet = pthread_cond_signal(cond);
   return iRet;
}

int app_os_cond_broadcast(CAGENT_COND_TYPE * cond)
{
   int iRet = 0;
   //Add code
   return iRet;
}

int app_os_cond_wait(CAGENT_COND_TYPE * cond, CAGENT_MUTEX_TYPE *mutex)
{
   int iRet = 0;
   //Add code 
   iRet = pthread_cond_wait(cond, mutex);
   return iRet;
}

int app_os_cond_timed_wait(CAGENT_COND_TYPE * cond, CAGENT_MUTEX_TYPE *mutex, int *ms)
{
   int iRet = 0;
   struct timeval now;
   struct timespec outtime;

   //Add code
   gettimeofday(&now, NULL);

   int tmp_sec =  (*ms/1000) + now.tv_sec;
   int tmp_usec = (*ms%1000 * 1000) + now.tv_usec;
   if (tmp_usec > 1000000)
   {
      tmp_usec -= 1000000;
      tmp_sec++;
   }
        

   outtime.tv_sec = tmp_sec;
   outtime.tv_nsec = tmp_usec * 1000;

   iRet = pthread_cond_timedwait(cond, mutex, &outtime);
   return iRet;
}

int app_os_cond_cleanup(CAGENT_COND_TYPE * cond)
{
   int iRet = 0;
   //Add code
   iRet = pthread_cond_destroy(cond);
   return iRet;
}

int app_os_get_process_id(void)
{
   return getpid();
}

int app_os_get_sysmem_usage_kb(long * totalPhysMemKB, long * availPhysMemKB) 
{
	struct sysinfo info;
	int iRet = -1;
	if(NULL == totalPhysMemKB || NULL == availPhysMemKB) return iRet;
	iRet = sysinfo(&info);
	if(iRet == 0)
	{
		*totalPhysMemKB = (long)(info.totalram * info.mem_unit / DIV);
		*availPhysMemKB = (long)(info.freeram * info.mem_unit / DIV);
	}
	return iRet;
}

int app_os_get_module_path(char * moudlePath)
{
	int iRet = 0;
	char * lastSlash = NULL;
	char tempPath[MAX_PATH] = {0};
	if(NULL == moudlePath) return iRet;
	if(ERROR_SUCCESS != GetModuleFileName(NULL, tempPath, sizeof(tempPath)))
	{
		lastSlash = strrchr(tempPath, FILE_SEPARATOR);
		if(NULL != lastSlash)
		{
			strncpy(moudlePath, tempPath, lastSlash - tempPath + 1);
			iRet = lastSlash - tempPath + 1;
		}
	}
	return iRet;
}

int app_os_get_os_version(char * osVersionBuf,  int bufLen)
{
	int iRet = -1;
	FILE *fp = NULL;
	char osname[128];
	char *p, *q;
	if(osVersionBuf == NULL) return iRet;
	fp = popen("lsb_release -d", "r");
	fgets(osname, 128, fp);
	pclose(fp);
	p = strtok(osname, ":");
	p = strtok(NULL, ":");
	q = trimwhitespace(p);
	strncpy(osVersionBuf, q, strlen(q));
	iRet = 0;
	return iRet;
}

int app_os_get_processor_name(char * processorNameBuf, unsigned long * bufLen)
{
	int iRet = -1;
	FILE *fp = NULL;
	char modelname[128];
	char *p, *q;
	if(processorNameBuf == NULL) return iRet;
	fp = popen("cat /proc/cpuinfo | grep 'model name'", "r");
	fgets(modelname, 128, fp);
	pclose(fp);
	p = strtok(modelname, ":");
	p = strtok(NULL, ":");
	q = trimwhitespace(p);
	strncpy(processorNameBuf, q, strlen(q));
	*bufLen = strlen(q);
	iRet = 0;
	return iRet;
}

int app_os_get_architecture(char * osArchBuf, int bufLen)
{
	int iRet = -1;
	FILE *fp = NULL;
	char *buff = malloc(bufLen);
	memset(buff, 0, bufLen);
	if(osArchBuf == NULL) return iRet;
	fp = popen("uname -m", "r");
	fgets(buff, bufLen, fp);
	pclose(fp);
	strcpy(osArchBuf, strtok(buff, "\n"));
	free(buff);
	iRet = 0;
	return iRet;
}

void split_path_file(char const *filepath, char* path, char* file)
{
	char* dirc = strdup(filepath);
       char* basec = strdup(filepath);
	char* dir = dirname(dirc);
	char* name = basename(basec);
	strcpy(path, dir);
	strcpy(file, name);
	free(dirc);
	free(basec);
}

void path_combine(char* destination, const char* path1, const char* path2)
{
	if(path1 == NULL && path2 == NULL) {
		strcpy(destination, "");;
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
		strcpy(destination, path1);
		if(append_directory_separator)
		{
			if(strncmp(path2, directory_separator, strlen(directory_separator) != 0))
				strcat(destination, directory_separator);
		}
		else
		{
			if(*skip_char == FILE_SEPARATOR)
				skip_char++;   
		}
		strcat(destination, skip_char);
	}
}

int app_os_get_temppath(char* lpBuffer, int nBufferLength)
{
	int len = 0;
	char const *folder = getenv("TMPDIR");
	if (folder == 0)
	{
		len = strlen("/tmp/");
		strncpy(lpBuffer, "/tmp/", strlen("/tmp/")+1);
	}
	else
	{
		len = strlen(folder);
		if(nBufferLength < len+1)
		{
			strncpy(lpBuffer, folder, nBufferLength);
			len = nBufferLength;
		}
		else
			strncpy(lpBuffer, folder, len+1);
	}
	return len;
}

int app_os_kill_process_name(char * processName)
{
	pid_t p;
	size_t i, j;
	char* s = (char*)malloc(264);
	char buf[128];
	FILE* st;
	DIR* d = opendir("/proc");
	if (d == NULL) { free(s); return -1; }
	struct dirent* f;
	while ((f = readdir(d)) != NULL) {
		if (f->d_name[0] == '.') continue;
		for (i = 0; isdigit(f->d_name[i]); i++);
		if (i < strlen(f->d_name)) continue;
		strcpy(s, "/proc/");
		strcat(s, f->d_name);
		strcat(s, "/status");
		st = fopen(s, "r");
		if (st == NULL) { closedir(d); free(s); return -1; }
		do {
			if (fgets(buf, 128, st) == NULL) { fclose(st); closedir(d); free(s); return -1; }
		} while (strncmp(buf, "Name:", 5));
		fclose(st);
		for (j = 5; isspace(buf[j]); j++);
		*strchr(buf, '\n') = 0;
		if (!strcmp(&(buf[j]), processName)) {
			sscanf(&(s[6]), "%d", &p);
			kill(p, SIGKILL);
		}
	}
	closedir(d);
	free(s);
	return 0;
}

int app_os_launch_process(char * appPath)
{
    int bRet = 0;
    pid_t pid = fork();
	if ( 0 == pid ) {/* Child process */
		exit(execlp("/bin/sh", "sh", "-c", appPath, NULL));
	} else if (pid < 0){ /* fork() failed */
        bRet = -1;    
	}
	return bRet;
}

int app_os_create_directory(char* path)
{
    struct stat st;
    int status = 0;
    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        status = -1;
    }

    return(status);
}

int app_os_set_executable(char* path)
{
	if(NULL == path) return -1;
	if(access(path, 0) != 0) return -1;

	return chmod(path, S_IREAD|S_IWRITE|S_IEXEC);
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
	//"/sbin/poweroff"
	int iRet = 0;
	pid_t fpid;
	fpid = fork();
   if(fpid == 0)
	{
		if(execlp("/sbin/poweroff", "poweroff",  NULL) < 0)
		{
			iRet = -1;
		}
	}
	else if(fpid < 0)
	{
		iRet = -1;
	}
   return iRet;
}

int app_os_PowerRestart()
{
	//"/sbin/reboot"
	int iRet = 0;
	pid_t fpid;
	fpid = fork();
	if(fpid == 0)
	{
		if(execlp("/sbin/reboot", "reboot",  NULL) < 0)
		{
			iRet = -1;
		}
	}
	else if(fpid < 0)
	{
		iRet = -1;
	}
	return iRet;
}

void app_os_disableResumePassword()
{
	return;
}

BOOL app_os_IsPwrSuspendAllowed()
{
	BOOL bRet1 = FALSE;
	BOOL bRet2 = FALSE;
	FILE *fp = NULL;
	fp = popen("/bin/cat  /sys/power/state ", "r");
	if(fp)
	{
		char tmpLineStr[512] = {0};
		while(fgets(tmpLineStr, sizeof(tmpLineStr), fp)!=NULL)
		{
			if(strstr(tmpLineStr, "mem"))
			{
				bRet1 = TRUE;
				break;
			}
			memset(tmpLineStr, 0, sizeof(tmpLineStr));
		}
		pclose(fp);
	}
	fp = popen("if [ -f \"/usr/sbin/pm-suspend\" ]; then echo yes; else echo no; fi", "r");
	if(fp)
	{
		char tmpLineStr[512] = {0};
		while(fgets(tmpLineStr, sizeof(tmpLineStr), fp)!=NULL)
		{
			if(strstr(tmpLineStr, "yes"))
			{
				bRet2 = TRUE;
				break;
			}
			memset(tmpLineStr, 0, sizeof(tmpLineStr));
		}
		pclose(fp);
	}	
	return bRet1&&bRet2;
}

BOOL app_os_IsPwrHibernateAllowed()
{
	BOOL bRet1 = FALSE;
	BOOL bRet2 = FALSE;
	BOOL bRet3 = FALSE;
	FILE *fp = NULL;
	fp = popen("/bin/cat /sys/power/state ", "r");
	if(fp)
	{
		char tmpLineStr[512] = {0};
		while(fgets(tmpLineStr, sizeof(tmpLineStr), fp)!=NULL)
		{
			if(strstr(tmpLineStr, "disk"))
			{
				bRet1 = TRUE;
				break;
			}
			memset(tmpLineStr, 0, sizeof(tmpLineStr));
		}
		pclose(fp);
	}	

	fp = popen("fdisk -l | grep swap ", "r");
	if(fp)
	{
		char tmpLineStr[512] = {0};
		while(fgets(tmpLineStr, sizeof(tmpLineStr), fp)!=NULL)
		{
			if(strstr(tmpLineStr, "swap"))
			{
				bRet2 = TRUE;
				break;
			}
			memset(tmpLineStr, 0, sizeof(tmpLineStr));
		}
		pclose(fp);
	}
	fp = popen("if [ -f \"/usr/sbin/pm-hibernate\" ]; then echo yes; else echo no; fi", "r");
	if(fp)
	{
		char tmpLineStr[512] = {0};
		while(fgets(tmpLineStr, sizeof(tmpLineStr), fp)!=NULL)
		{
			if(strstr(tmpLineStr, "yes"))
			{
				bRet3 = TRUE;
				break;
			}
			memset(tmpLineStr, 0, sizeof(tmpLineStr));
		}
		pclose(fp);
	}
	return bRet1&&bRet2&&bRet3;
}

BOOL app_os_PowerSuspend()
{
	//"/usr/sbin/pm-suspend"
	int bRet = TRUE;
	pid_t fpid;
	fpid = fork();
	if(fpid == 0)
	{
		if(execlp("/usr/sbin/pm-suspend", "pm-suspend", "-h", NULL) < 0)
		{
			bRet = FALSE;
		}
	}
	else if(fpid < 0)
	{
		bRet = FALSE;
	}
	return bRet;
}

BOOL app_os_PowerHibernate()
{
	//"/usr/sbin/pm-hibernate"
	int bRet = TRUE;
	pid_t fpid;
	fpid = fork();
	if(fpid == 0)
	{
		if(execlp("/usr/sbin/pm-hibernate", "pm-hibernate", NULL) < 0)
		{
			bRet = FALSE;
		}
	}
	else if(fpid < 0)
	{
		bRet = FALSE;
	}
	return bRet;
}
/*
BOOL app_os_SAWakeOnLan(char * mac, int size)
{
	BOOL bRet = FALSE;
	if(size < 6) return bRet;
	{
		unsigned char packet[102];
		struct sockaddr_in addr;
		int sockfd;
		int i = 0, j = 0;
		BOOL optVal = TRUE;

		for(i=0;i<6;i++)  
		{
			packet[i] = 0xFF;  
		}
		for(i=1;i<17;i++)
		{
			for(j=0;j<6;j++)
			{
				packet[i*6+j] = mac[j];
			}
		}

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd > 0)
		{
			int iRet = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,(char *)&optVal, sizeof(optVal));
			if(iRet == 0)
			{
				memset((void*)&addr, 0, sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_port = htons(9);
				addr.sin_addr.s_addr= INADDR_BROADCAST;
				iRet = sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr));
				if(iRet != -1) bRet = TRUE;
			}
			close(sockfd);
		}
	}
	return bRet;
}
*/
unsigned long long app_os_GetTickCount()
{
	struct timespec ts;  
	clock_gettime(CLOCK_MONOTONIC, &ts);  
	return (ts.tv_sec * 1000 + ts.tv_nsec/1000000); 
}
/*
int app_os_GetAllMacs(char macsStr[][20], int n)
{
	int iRet = -1;
	if(n <= 0) return iRet;
	{
		int fd;
		struct ifreq ifqBuf;
		int macIndex = 0;
		if((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)  
		{
			FILE * fp = fopen("/proc/net/dev", "r");
			if(fp)
			{
				char lineBuf[512] = {0};
				fgets(lineBuf, sizeof(lineBuf), fp);
				fgets(lineBuf, sizeof(lineBuf), fp);
				memset(lineBuf, 0, sizeof(lineBuf));
				while(fgets(lineBuf, sizeof(lineBuf), fp))
				{
					char nameBuf[128] = {0}; 
					int nLen = 0;
					sscanf(lineBuf, "%s", nameBuf);
					nLen = strlen(nameBuf);
					if(nLen <= 0) continue;
					if(nameBuf[nLen-1] == ':') nameBuf[nLen-1] = '\0';
					if(!strcasecmp(nameBuf, "lo")) continue;
					else 
					{
						memset(&ifqBuf, 0, sizeof(struct ifreq));
						strncpy(ifqBuf.ifr_name, nameBuf, sizeof(ifqBuf.ifr_name)-1);
						if (!(ioctl(fd, SIOCGIFHWADDR, (char *)&ifqBuf)))  
						{  
							sprintf(macsStr[macIndex], "%02X:%02X:%02X:%02X:%02X:%02X",  
								(unsigned char)ifqBuf.ifr_hwaddr.sa_data[0],  
								(unsigned char)ifqBuf.ifr_hwaddr.sa_data[1],  
								(unsigned char)ifqBuf.ifr_hwaddr.sa_data[2],  
								(unsigned char)ifqBuf.ifr_hwaddr.sa_data[3],  
								(unsigned char)ifqBuf.ifr_hwaddr.sa_data[4],  
								(unsigned char)ifqBuf.ifr_hwaddr.sa_data[5]);  
							macIndex++; 
						}
					}
				}
			}
			fclose(fp);
		}
		iRet = macIndex;
	}
	return iRet;
}
*/
BOOL app_os_GetRandomStr(char *randomStrBuf, int bufSize)
{
	int flag = 0, strLen = 0, i = 0;
	if(NULL == randomStrBuf || bufSize <= 0) return FALSE;

	srand((unsigned int)app_os_GetTickCount());  //10ms
	app_os_sleep(10);  //10ms deviation

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

/*
BOOL app_os_GetHostIP(char * ipStr)  
{  
#define DEF_IFR_CNT     8
	BOOL bRet = FALSE;
	if(ipStr == NULL) return bRet;
	{
		int getIPSock;  
		char ipAddr[32] = {0}; 
		char ifrName[32] = {0};
		struct sockaddr_in *sin = NULL;  
		struct ifreq ifr_ip;  
		int i = 0;

		if ((getIPSock=socket(AF_INET, SOCK_STREAM, 0)) == -1)  
		{  
			return bRet;  
		}  
		for(i=0; i<DEF_IFR_CNT; i++)
		{
			sprintf(ifrName, "eth%d", i);
			memset(&ifr_ip, 0, sizeof(ifr_ip));     
			strncpy(ifr_ip.ifr_name, ifrName, sizeof(ifr_ip.ifr_name)-1);
			if(ioctl(getIPSock, SIOCGIFADDR, &ifr_ip) < 0)     
			{     
				continue;     
			}
			sin = (struct sockaddr_in *)&(ifr_ip.ifr_addr);     
			strcpy(ipAddr,(char *)inet_ntoa(sin->sin_addr));         
			if(strlen(ipAddr))
			{
				strcpy(ipStr, ipAddr);
				bRet = TRUE;
				break;
			}
		}
		close(getIPSock);
	}
	return bRet;
}
*/
int app_os_RunX11vnc(char * x11vncPath, char * runParams)
{
	if(x11vncPath == NULL || runParams == NULL) return -1;
	{
		pid_t fpid = 0;
		char * word[16] = {NULL};
		char *buf = runParams;
		int i = 0;
		word[i]="x11vnc";
		i++;
		while((word[i] = strtok(buf, " ")) != NULL)
		{
			i++;
			buf=NULL; 
		}
		if(i>1)
		{
			fpid = fork();
			if(fpid == 0)
			{
				execvp(x11vncPath, word);
			}
		}
		return fpid;
	}
}

BOOL app_os_CloseHandleEx(CAGENT_HANDLE handle)
{
	BOOL bRet = FALSE;
	if(handle == NULL) return bRet;
	if(close(handle) == 0)
	{
		bRet = TRUE;
	}
	return bRet;
}

BOOL app_os_ReadFile(CAGENT_HANDLE fHandle, char * buf, unsigned int nSizeToRead, unsigned int * nSizeRead)
{
	BOOL bRet = FALSE;
	if(fHandle == NULL || buf == NULL || nSizeRead == NULL) return bRet;
	{
		ssize_t tmpReadRet = 0;
		tmpReadRet = read(fHandle, buf, nSizeToRead);
		if(tmpReadRet != -1)
		{
			if(tmpReadRet>0)
			{
				*nSizeRead = tmpReadRet;
			}
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL app_os_WriteFile(CAGENT_HANDLE fHandle, char * buf, unsigned int nSizeToWrite, unsigned int * nSizeWrite)
{
	BOOL bRet = FALSE;
	if(fHandle == NULL || buf == NULL || nSizeWrite == NULL) return bRet;
	{
		ssize_t tmpWriteRet = 0;
		tmpWriteRet = write(fHandle, buf, nSizeToWrite);
		if(tmpWriteRet != -1)
		{
			if(tmpWriteRet>0)
			{
				*nSizeWrite = tmpWriteRet;
			}
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL app_os_CreatePipe(CAGENT_HANDLE * hReadPipe, CAGENT_HANDLE * hWritePipe)
{
	BOOL bRet = FALSE;
	if(hReadPipe == NULL || hWritePipe == NULL) return bRet;
	{
		int fds[2];
		if(pipe(fds) == 0)
		{
			int flags = 0;
			flags = fcntl(fds[0], F_GETFL);
			fcntl(fds[0], F_SETFL,flags | O_NONBLOCK);
			fcntl(fds[1], F_SETFL,flags | O_NONBLOCK);
			*hReadPipe = fds[0];
			*hWritePipe = fds[1];
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL app_os_DupHandle(CAGENT_HANDLE srcHandle, CAGENT_HANDLE * trgHandle)
{
	BOOL bRet = FALSE;
	if(trgHandle == NULL) return bRet;
	{
		CAGENT_HANDLE newHandle;
		newHandle = dup(srcHandle);
		if(newHandle != -1)
		{
			int flags = 0;
			flags = fcntl(newHandle, F_GETFL);
			fcntl(newHandle, F_SETFL,flags | O_NONBLOCK);
			*trgHandle = newHandle;
			bRet = TRUE;
		}
	}
	return bRet;
}

CAGENT_HANDLE app_os_PrepAndLaunchRedirectedChild(CAGENT_HANDLE hChildStdOut, CAGENT_HANDLE hChildStdIn,	CAGENT_HANDLE hChildStdErr)
{
	CAGENT_HANDLE hRet = NULL;
	pid_t pid;
	pid = fork();
	if(pid == 0)
	{
		dup2(hChildStdIn, STDIN_FILENO);
		dup2(hChildStdOut, STDOUT_FILENO);
		dup2(hChildStdErr, STDERR_FILENO);
		execlp("/bin/sh", "sh", NULL);
	}
	else if(pid > 0)
	{
		hRet = pid;
	}
	return hRet;
}

BOOL app_os_is_file_exist(const char * pFilePath)
{
	if(NULL == pFilePath) return FALSE;
	return access(pFilePath, F_OK) == 0 ? TRUE : FALSE;
}

BOOL app_os_file_access(const char * pFilePath, int mode)
{
	if(NULL == pFilePath) return FALSE;
	return access(pFilePath, mode) == 0 ? TRUE : FALSE;
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
	BOOL bRet = FALSE;
	if(srcFilePath == NULL || destFilePath == NULL) return bRet;
	{
		char cmdLine[512] = {0};
		int iRet = 0;
		sprintf(cmdLine, "cp -p %s %s", srcFilePath, destFilePath);
      iRet = system(cmdLine);
		if(iRet >0 ) bRet = TRUE;
	}
	return bRet;
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
	return FALSE;
}

BOOL app_os_process_check(char * processName)
{
	return app_os_ProcessCheck(processName);
}
//----------------------dhl add E--------------------------------------

DWORD getPIDByName(char * prcName)
{
	FILE *fopen = NULL;
	unsigned int pid = 0;	
	char buf[16];
	char cmdLine[256];
	sprintf(cmdLine,"pidof -s %s",prcName);
	if((fopen = popen(cmdLine,"r"))==NULL)
		return 0;
	if(!fgets(buf,sizeof(buf),fopen))
	{
		pclose(fopen);
		return 0;
	}
	sscanf(buf,"%d",&pid);
	pclose(fopen);
	return pid;
}

int app_os_InitializeCriticalSection(CRITICAL_SECTION *restrict_mutex)
{
	return pthread_mutex_init(restrict_mutex, NULL);
}

int app_os_DeleteCriticalSection(CRITICAL_SECTION *restrict_mutex)
{
	return pthread_mutex_destroy(restrict_mutex);
}

int app_os_EnterCriticalSection(CRITICAL_SECTION *restrict_mutex)
{
	return pthread_mutex_lock(restrict_mutex);
}

int app_os_LeaveCriticalSection(CRITICAL_SECTION *restrict_mutex)
{
	return pthread_mutex_unlock(restrict_mutex);
}

DWORD app_os_GetLastError(void)
{
	return errno;
}

void app_os_GetSystemInfo(SYSTEM_INFO *lpSystemInfo)
{
	
	if (lpSystemInfo == NULL) return; 
	lpSystemInfo->dwNumberOfProcessors = sysconf(_SC_NPROCESSORS_CONF);
	return;
}

void GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
	int iRet = 0;
   	struct timeval now;
   	struct timespec outtime;

   	//Add code
   	gettimeofday(&now, NULL);
	int sysTickHZ = sysconf(_SC_CLK_TCK);



   	__int64 tmp_sec =  now.tv_sec;
   	__int64 tmp_usec = now.tv_usec;
	*((__int64 *)lpSystemTimeAsFileTime) = tmp_sec * sysTickHZ + (tmp_usec*sysTickHZ)/1000000 ;
	return;
}

static int get_proc_info(pid_t pid, procinfo * pinfo);
procinfo m_procInfo;
BOOL IsProcessActiveWithIDEx(DWORD prcID)
{
	BOOL bRet = FALSE;

	if(prcID == m_procInfo.pid)
	{
		if(m_procInfo.state == 'R' || m_procInfo.state == 'S')	
			bRet = TRUE;
		//printf("[IsProcessActiveWithIDEx] pid %d is %c\n",m_procInfo.pid, m_procInfo.state);
	}
	//else
	//	printf("[IsProcessActiveWithIDEx] pid %d is invalid\n", prcID);
	return bRet;
}

BOOL GetProcessMemoryUsageKBWithID(DWORD prcID, long * memUsageKB)
{
	if(!prcID || NULL == memUsageKB) return FALSE;
	bool nRet = FALSE;
	int pageSize = getpagesize();
	if(prcID == m_procInfo.pid)
	{
		//printf("[GetProcessMemoryUsageKBWithID] get page size %d",getpagesize());
		*memUsageKB = m_procInfo.rss * (pageSize/DIV); // rss is page count, 4KB/Page.
		 nRet = TRUE;
	}
	return nRet;
}

static int get_proc_info(pid_t pid, procinfo * pinfo)
{
  char szFileName [_POSIX_PATH_MAX],
    szStatStr [2048],
    *s, *t;
  FILE *fp;
  struct stat st;
  
  if (NULL == pinfo) {
    errno = EINVAL;
    return -1;
  }

  sprintf (szFileName, "/proc/%u/stat", (unsigned) pid);
  
  if (-1 == access (szFileName, R_OK)) {
    return (pinfo->pid = -1);
  } /** if **/

  if (-1 != stat (szFileName, &st)) {
  	pinfo->euid = st.st_uid;
  	pinfo->egid = st.st_gid;
  } else {
  	pinfo->euid = pinfo->egid = -1;
  }
  
  
  if ((fp = fopen (szFileName, "r")) == NULL) {
    return (pinfo->pid = -1);
  } /** IF_NULL **/
  
  if ((s = fgets (szStatStr, 2048, fp)) == NULL) {
    fclose (fp);
    return (pinfo->pid = -1);
  }

  /** pid **/
  sscanf (szStatStr, "%u", &(pinfo->pid));
  s = strchr (szStatStr, '(') + 1;
  t = strchr (szStatStr, ')');
  strncpy (pinfo->exName, s, t - s);
  pinfo->exName [t - s] = '\0';
  
  sscanf (t + 2, "%c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
	  /*       1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33*/
	  &(pinfo->state),
	  &(pinfo->ppid),
	  &(pinfo->pgrp),
	  &(pinfo->session),
	  &(pinfo->tty),
	  &(pinfo->tpgid),
	  &(pinfo->flags),
	  &(pinfo->minflt),
	  &(pinfo->cminflt),
	  &(pinfo->majflt),
	  &(pinfo->cmajflt),
	  &(pinfo->utime),
	  &(pinfo->stime),
	  &(pinfo->cutime),
	  &(pinfo->cstime),
	  &(pinfo->counter),
	  &(pinfo->priority),
	  &(pinfo->timeout),
	  &(pinfo->itrealvalue),
	  &(pinfo->starttime),
	  &(pinfo->vsize),
	  &(pinfo->rss),
	  &(pinfo->rlim),
	  &(pinfo->startcode),
	  &(pinfo->endcode),
	  &(pinfo->startstack),
	  &(pinfo->kstkesp),
	  &(pinfo->kstkeip),
	  &(pinfo->signal),
	  &(pinfo->blocked),
	  &(pinfo->sigignore),
	  &(pinfo->sigcatch),
	  &(pinfo->wchan));
  fclose (fp);
  return 0;
}

BOOL app_os_GetProcessTimes(
		HANDLE hPRocess, 
		LPFILETIME lpCreationTime, 
		LPFILETIME lpExitTime,     //reserved but unused.
		LPFILETIME lpKernelTime, 
		LPFILETIME lpUserTime)
{
	if(!hPRocess || !lpCreationTime || !lpKernelTime || !lpUserTime) return false;
	//procinfo procInfo;
	int ret = get_proc_info(hPRocess, &m_procInfo);
	if(ret < 0) return false;
	//printf("[app_os_GetProcessTimes]  procid:%d,realid%d, utime: %d, stime: %d, start_time: %d\n",hPRocess, m_procInfo.pid ,m_procInfo.utime, m_procInfo.stime, m_procInfo.starttime);
	
	*((__int64 *)lpCreationTime) = m_procInfo.starttime;
	*((__int64 *)lpKernelTime) = m_procInfo.stime;
	*((__int64 *)lpUserTime) = m_procInfo.utime;
	return true;
}


BOOL app_os_CloseHandle(void * handle)
{
	return TRUE;
}

//hProcess is pid, 
PROCESSENTRY32_NODE * m_curNode = NULL;
BOOL app_os_GetProcessUsername(HANDLE hProcess, char * userNameBuf, int bufLen)
{
	if(!hProcess) return false;
	DWORD uid = 0;
	if (m_curNode == NULL || m_curNode->prcMonInfo.th32ProcessID != hProcess) return false;
	
	struct passwd *pw;
	pw = getpwuid(m_curNode->prcMonInfo.dwUID);
	if(!pw) return false;
	unsigned int strLength = strlen((char *)(pw->pw_name));
	if(strLength >= bufLen) return false;
	strcpy(userNameBuf,(char *)(pw->pw_name));
	return true;

}

BOOL read_proc(PROCESSENTRY32* info,const char* c_pid)
{
    FILE* fp = NULL;
    char file[512] = {0};
    char line_buff[BUFF_LEN] = {0};
    
    sprintf(file,"/proc/%s/status",c_pid);
    if (!(fp = fopen(file,"r")))
    {
        return FALSE;
    }

    char name[32];
    if (read_line(fp,line_buff,BUFF_LEN,PROC_NAME_LINE))
    {
        sscanf(line_buff,"%s %s",name,(info->szExeFile));
    }
    else
    {
        fclose(fp);
	return FALSE;
    }

	DWORD dwID_tmp = 0;
    fseek(fp,0,SEEK_SET);
    if (read_line(fp,line_buff,BUFF_LEN,PROC_PID_LINE))
    {
        sscanf(line_buff,"%s %d",name,&dwID_tmp);
		if(!strcmp("Pid:",name))
			info->th32ProcessID = dwID_tmp;
		else
		{
			fseek(fp,0,SEEK_SET);
			if (read_line(fp,line_buff,BUFF_LEN,PROC_PID_LINE-1))
			{
				sscanf(line_buff,"%s %d",name,&dwID_tmp);
				if(!strcmp("Pid:",name))
					info->th32ProcessID = dwID_tmp;
				else
				{
					fclose(fp);
					return FALSE;
				}
			}
		}
    }
    else
    {
        fclose(fp);
	return FALSE;
    }

	dwID_tmp = 0;
    fseek(fp,0,SEEK_SET);
    if (read_line(fp,line_buff,BUFF_LEN,PROC_UID_LINE))
    {
        sscanf(line_buff,"%s %d",name,&dwID_tmp);
		if(!strcmp("Uid:",name))
			info->dwUID = dwID_tmp;
		else
		{
			fseek(fp,0,SEEK_SET);
			if (read_line(fp,line_buff,BUFF_LEN,PROC_UID_LINE-1))
			{
				sscanf(line_buff,"%s %d",name,&dwID_tmp);
				if(!strcmp("Uid:",name))
					info->dwUID = dwID_tmp;
				else
				{
					fclose(fp);
					return FALSE;
				}
			}
		}
    }
    else
    {
        fclose(fp);
		return FALSE;
    }

    fclose(fp);
    return TRUE;
}

int read_line(FILE* fp,char* buff,int b_l,int l)
{
    if (!fp)
        return FALSE;
    
    char line_buff[b_l];
    int i;
    for (i = 0; i < l-1; i++)
    {
        if (!fgets (line_buff, sizeof(line_buff), fp))
        {
            return FALSE;
        }
    }

    if (!fgets (line_buff, sizeof(line_buff), fp))
    {
        return FALSE;
    }

    memcpy(buff,line_buff,b_l);

    return TRUE;
}

//PROCESSENTRY32_NODE * headNodeSnapshot = NULL;
//PROCESSENTRY32_NODE * curNode = NULL;
HANDLE app_os_CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID)
{
	PROCESSENTRY32_NODE * headNodeSnapshot = NULL;
	PROCESSENTRY32_NODE * curInfoNode = NULL;
	PROCESSENTRY32_NODE * prcInfoNode = NULL;
    	DIR *dir;
    	struct dirent *ptr;
	if (!(dir = opendir("/proc")))
        	return 0;
	headNodeSnapshot = (PROCESSENTRY32_NODE *)malloc(sizeof(PROCESSENTRY32_NODE));
	memset(headNodeSnapshot, 0, sizeof(PROCESSENTRY32_NODE));
	curInfoNode = headNodeSnapshot;

    	while (ptr = readdir(dir))
    	{
        	if (ptr->d_name[0] > '0' && ptr->d_name[0] <= '9')
        	{
			prcInfoNode = (PROCESSENTRY32_NODE *)malloc(sizeof(PROCESSENTRY32_NODE));
			memset(prcInfoNode, 0, sizeof(PROCESSENTRY32_NODE));
            		if(!read_proc(&(prcInfoNode->prcMonInfo),ptr->d_name))
			{
				free(prcInfoNode);
				continue;
			}
			curInfoNode->next = prcInfoNode;
			curInfoNode = prcInfoNode;
		//printf("[ProcessMonitorHandler] proc name %s pid %d uid %d.\n",prcInfoNode->prcMonInfo.szExeFile, prcInfoNode->prcMonInfo.th32ProcessID, prcInfoNode->prcMonInfo.dwUID);
        	}
    	}
	curInfoNode = headNodeSnapshot;
	closedir(dir);
	return headNodeSnapshot;
}

BOOL app_os_Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
	if(!hSnapshot) return false;
	PROCESSENTRY32_NODE * headInfoNode = (PROCESSENTRY32_NODE *)hSnapshot;
	m_curNode=headInfoNode->next;
	if(m_curNode != NULL)
		memcpy(lppe, &(m_curNode->prcMonInfo), sizeof(PROCESSENTRY32));
	else
		return false;
	//printf("[ProcessMonitorHandler]0 proc name %s pid %d\n",lppe->szExeFile, lppe->th32ProcessID);
	//lppe = &(curNode->prcMonInfo);
	return true;
}

BOOL app_os_Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
	//printf("7\n");
	if(!hSnapshot) return false;
	m_curNode=m_curNode->next;
	if(m_curNode != NULL)
		memcpy(lppe, &(m_curNode->prcMonInfo), sizeof(PROCESSENTRY32));
	else
		return false;
	//printf("[ProcessMonitorHandler]1 proc name %s pid %d\n",m_curNode->prcMonInfo.szExeFile, m_curNode->prcMonInfo.th32ProcessID);
	//printf("[ProcessMonitorHandler]2 proc name %s pid %d\n",lppe->szExeFile, lppe->th32ProcessID);
	//lppe = &(m_curNode->prcMonInfo);
	return true;
}

BOOL app_os_CloseSnapShot32Handle(HANDLE hSnapshot)
{
	if(!hSnapshot) return false;
	PROCESSENTRY32_NODE * headInfoNode = (PROCESSENTRY32_NODE *)hSnapshot;
	PROCESSENTRY32_NODE * curInfoNode = NULL;
	PROCESSENTRY32_NODE * nextInfoNode = NULL;
	curInfoNode = headInfoNode->next;
	while(curInfoNode != NULL)
	{
		nextInfoNode = curInfoNode->next;
		free(curInfoNode);
		curInfoNode = nextInfoNode;
	}
	free(headInfoNode);
	headInfoNode = NULL;
	m_curNode = NULL;
	return true;
}

BOOL app_os_GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer)
{
	if(!lpBuffer) return false;

	char name[32];
	char proc_pic_path[128];
	const int bufLength = 256;
	char buf[bufLength];
	sprintf(proc_pic_path,"/proc/meminfo");
	FILE * fp = NULL;
	fp = fopen(proc_pic_path,"r");
	if(NULL != fp)
	{
		if (read_line(fp,buf,bufLength,MEM_TOTAL_LINE))
    		{
        		sscanf(buf,"%s %llu",name,&(lpBuffer->ullTotalPhys));
    		} 
		fseek(fp,0,SEEK_SET);
		if (read_line(fp,buf,bufLength,MEM_FREE_LINE))
    		{
        		sscanf(buf,"%s %llu",name,&(lpBuffer->ullAvailPhys));
    		} 
	}
    	fclose(fp);
	return true;
}

BOOL app_os_GetSystemTimes(LPFILETIME lpIdleTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime)
{
	if(!lpIdleTime || !lpKernelTime || !lpUserTime) return false;
	__int64 niceTime;
	char name[32];
	char proc_pic_path[128];
	const int bufLength = 256;
	char buf[bufLength];
	sprintf(proc_pic_path,"/proc/stat");
	FILE * fp = NULL;
	fp = fopen(proc_pic_path,"r");
	if(NULL != fp)
	{
		if (read_line(fp,buf,bufLength,CPU_USAGE_INFO_LINE))
    		{
        		sscanf(buf,"%s %lld %lld %lld %lld",name,lpUserTime,&niceTime,lpKernelTime,lpIdleTime);
			//printf("[app_os_GetSystemTime] name:%s, utime:%lld, NTime:%lld, ktime:%lld, itime:%lld.\n",name,*lpUserTime,niceTime,*lpKernelTime,*lpIdleTime);
			//*(__int64 *)lpUserTime =*(__int64 *)lpUserTime + niceTime;
    		} 
	}
    fclose(fp);
	return true;
}

BOOL app_os_GetSysLogonUserName(char * userNameBuf, unsigned int bufLen)
{
	int i = 0;
	if (userNameBuf == NULL || bufLen == 0) return false;

	FILE * fp = NULL;
	char cmdline[128];
	const int bufLength = 512;
	char buf[bufLength];
	char cmdbuf[12][32]={0};

	sprintf(cmdline,"last|grep still");//for opensusi kde desktop
	fp = popen(cmdline,"r");
	if(NULL != fp){
		if (fgets(buf, sizeof(buf), fp))
    	{
        	sscanf(buf,"%s",cmdbuf[0]);
		}
	}
	/*sprintf(cmdline,"ps -aux|grep startkde");//for opensusi kde desktop
	fp = popen(cmdline,"r");
	if(NULL != fp)
	{
		if (fgets(buf, sizeof(buf), fp))
    	{
        	sscanf(buf,"%s %s %s %s %s %s %s %s %s %s %s %s",cmdbuf[0],cmdbuf[1],cmdbuf[2],cmdbuf[3],
				cmdbuf[4],cmdbuf[5],cmdbuf[6],cmdbuf[7],cmdbuf[8],cmdbuf[9],cmdbuf[10],cmdbuf[11]);
		    //printf("[app_os_GetSysLogonUserName] kde name:%s, utime:%s, NTime:%s, ktime:%s.\n",cmdbuf[0],cmdbuf[1],cmdbuf[10],cmdbuf[11]);
    	}
		else
		{
			pclose(fp);
			sprintf(cmdline,"ps -aux|grep gnome-keyring-daemon");//for ubuntu gnome desktop gnome
			fp = popen(cmdline,"r");
			if (fgets(buf, sizeof(buf), fp))
    		{
        	sscanf(buf,"%s %s %s %s %s %s %s %s %s %s %s %s",cmdbuf[0],cmdbuf[1],cmdbuf[2],cmdbuf[3],
				cmdbuf[4],cmdbuf[5],cmdbuf[6],cmdbuf[7],cmdbuf[8],cmdbuf[9],cmdbuf[10],cmdbuf[11]);
		    //printf("[app_os_GetSysLogonUserName] gnome name:%s, utime:%s, NTime:%s, ktime:%s.\n",cmdbuf[0],cmdbuf[1],cmdbuf[10],cmdbuf[11]);
    		}
		}

	}*/
    pclose(fp);

	i = strlen(cmdbuf[0]);
	if(i>0 && i< bufLen)
		strcpy(userNameBuf, cmdbuf[0]);
	else 
		return false;
	return true;
}

void app_os_GetSysLogonUserList(char * logonUserList, int *logonUserCnt ,int maxLogonUserCnt,int maxLogonUserNameLen)
{
	if (logonUserList == NULL || logonUserCnt == NULL) return false;

	FILE * fp = NULL;
	char cmdline[128];
	const int bufLength = 512;
	char buf[bufLength];
	char cmdbuf[12][32]={0};
	*logonUserCnt = 0;
	sprintf(cmdline,"last|grep still");
	fp = popen(cmdline,"r");
	if(NULL != fp){
		while (fgets(buf, sizeof(buf), fp))
    		{
			int i = 0;
			BOOL isAddIn = FALSE;
        		sscanf(buf,"%s",cmdbuf[0]);
			for (i = 0; i < *logonUserCnt && i< maxLogonUserCnt; i++)
			{
				if(!strcmp(&(logonUserList[maxLogonUserNameLen * i]),cmdbuf[0]))
				{
					isAddIn = TRUE;
					break;
				}
			}
			if(!isAddIn)
			{
				if(*logonUserCnt < maxLogonUserCnt)
				{
					strcpy(&(logonUserList[maxLogonUserNameLen * (*logonUserCnt)]),cmdbuf[0]);
					(*logonUserCnt)++;
				}
			}
			
		}
	}
    	pclose(fp);
//Wei.gang add to debug
//	int i = 0;
//	for (i = 0; i< *logonUserCnt; i++)
//		printf("[common] app_os_GetSysLogonUserList. get user %d: %s\n",i, &(logonUserList[maxLogonUserNameLen * i]));
//Wei.Gang add end
	return true;
}


