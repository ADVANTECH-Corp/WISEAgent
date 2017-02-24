#include "util_os.h"
#include <Windows.h>

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
#define OS_WINDOWS_10			  "Windows 10"
#define OS_WINDOWS_SERVER_2016	  "Windows Server 2016"

#define ARCH_UNKNOW               "Unknow"
#define ARCH_X64                  "X64"
#define ARCH_ARM                  "ARM"
#define ARCH_IA64                 "IA64"
#define ARCH_X86                  "X86"

#define DIV                       (1024)

bool util_os_get_os_name_reg_x64(char * pOSNameBuf, unsigned long * bufLen)
{
	bool bRet = false;
	HKEY hk;
	char regName[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
	char valueName[] = "ProductName";
	char valueData[256] = {0};    
	unsigned long  valueDataSize = sizeof(valueData);
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_READ|KEY_WOW64_64KEY, &hk)) return false;
	else
	{
		if(ERROR_SUCCESS != RegQueryValueEx(hk, valueName, 0, NULL, (LPBYTE)valueData, &valueDataSize)) 
		{
			RegCloseKey(hk);
			return false;
		}
		RegCloseKey(hk);
		bRet = valueDataSize == 0 ? false : true;
	}
	if(bRet)
	{
		//unsigned int cpyLen = valueDataSize < *bufLen ? valueDataSize : *bufLen; 
		if(pOSNameBuf)
			memcpy(pOSNameBuf, valueData, valueDataSize);
		if(bufLen)
			*bufLen = valueDataSize;
	}
	return bRet;
}

bool util_os_get_os_name_reg(char * pOSNameBuf, unsigned long * bufLen)
{
	bool bRet = false;
	HKEY hk;
	char regName[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
	char valueName[] = "ProductName";
	char valueData[256] = {0};    
	unsigned long  valueDataSize = sizeof(valueData);
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_READ, &hk)) return util_os_get_os_name_reg_x64(pOSNameBuf, bufLen);
	else
	{
		if(ERROR_SUCCESS != RegQueryValueEx(hk, valueName, 0, NULL, (LPBYTE)valueData, &valueDataSize)) 
		{
			RegCloseKey(hk);
			return util_os_get_os_name_reg_x64(pOSNameBuf, bufLen);
		}
		RegCloseKey(hk);
		bRet = valueDataSize == 0 ? false : true;
	}
	if(bRet)
	{
		//unsigned int cpyLen = valueDataSize < *bufLen ? valueDataSize : *bufLen; 
		if(pOSNameBuf)
			memcpy(pOSNameBuf, valueData, valueDataSize);
		if(bufLen)
			*bufLen = valueDataSize;
	}
	return bRet;
}

bool util_os_get_os_name(char * pOSNameBuf, unsigned long * bufLen)
{
#if 0
	bool bRet = false;
	OSVERSIONINFOEX osvInfo;
	bool isUnknow = false;

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
						if(pOSNameBuf)
							memcpy(pOSNameBuf, OS_WINDOWS_95, sizeof(OS_WINDOWS_95));
						if(bufLen)
							*bufLen = sizeof(OS_WINDOWS_95);
						break;
					}
				case 10:
					{
						if(pOSNameBuf)
							memcpy(pOSNameBuf, OS_WINDOWS_98, sizeof(OS_WINDOWS_98));
						if(bufLen)
							*bufLen = sizeof(OS_WINDOWS_98);
						break;
					}
				case 90:
					{
						if(pOSNameBuf)
							memcpy(pOSNameBuf, OS_WINDOWS_ME, sizeof(OS_WINDOWS_ME));
						if(bufLen)
							*bufLen = sizeof(OS_WINDOWS_ME);
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
					if(pOSNameBuf)
						memcpy(pOSNameBuf, OS_WINDOWS_NT_3_51, sizeof(OS_WINDOWS_NT_3_51));
					if(bufLen)
						*bufLen = sizeof(OS_WINDOWS_NT_3_51);
					break;
				}
			case 4:
				{
					if(pOSNameBuf)
						memcpy(pOSNameBuf, OS_WINDOWS_NT_4, sizeof(OS_WINDOWS_NT_4));
					if(bufLen)
						*bufLen = sizeof(OS_WINDOWS_NT_4);
					break;
				}
			case 5:
				{
					switch(osvInfo.dwMinorVersion)
					{
					case 0:
						{
							if(pOSNameBuf)
								memcpy(pOSNameBuf, OS_WINDOWS_2000, sizeof(OS_WINDOWS_2000));
							if(bufLen)
								*bufLen = sizeof(OS_WINDOWS_2000);
							break;
						}
					case 1:
						{
							if(pOSNameBuf)
								memcpy(pOSNameBuf, OS_WINDOWS_XP, sizeof(OS_WINDOWS_XP));
							if(bufLen)
								*bufLen = sizeof(OS_WINDOWS_XP);
							break;
						}
					case 2:
						{
							if(pOSNameBuf)
								memcpy(pOSNameBuf, OS_WINDOWS_SERVER_2003, sizeof(OS_WINDOWS_SERVER_2003));
							if(bufLen)
								*bufLen = sizeof(OS_WINDOWS_SERVER_2003);
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
							if(pOSNameBuf)
								memcpy(pOSNameBuf, OS_WINDOWS_VISTA, sizeof(OS_WINDOWS_VISTA));
							if(bufLen)
								*bufLen = sizeof(OS_WINDOWS_VISTA);
							break;
						}
					case 1:
						{
							if(pOSNameBuf)
								memcpy(pOSNameBuf, OS_WINDOWS_7, sizeof(OS_WINDOWS_7));
							if(bufLen)
								*bufLen = sizeof(OS_WINDOWS_7);
							break;
						}
					case 2:
						{
							if(osvInfo.wProductType == VER_NT_WORKSTATION)
							{
								if(pOSNameBuf)
									memcpy(pOSNameBuf, OS_WINDOWS_8, sizeof(OS_WINDOWS_8));
								if(bufLen)
									*bufLen = sizeof(OS_WINDOWS_8);
							}
							else
							{
								if(pOSNameBuf)
									memcpy(pOSNameBuf, OS_WINDOWS_SERVER_2012, sizeof(OS_WINDOWS_SERVER_2012));
								if(bufLen)
									*bufLen = sizeof(OS_WINDOWS_SERVER_2012);
							}
							break;
						}
					case 3:
						{
							if(osvInfo.wProductType == VER_NT_WORKSTATION)
							{
								if(pOSNameBuf)
									memcpy(pOSNameBuf, OS_WINDOWS_8_1, sizeof(OS_WINDOWS_8_1));
								if(bufLen)
									*bufLen = sizeof(OS_WINDOWS_8_1);
							}
							else
							{
								if(pOSNameBuf)
									memcpy(pOSNameBuf, OS_WINDOWS_SERVER_2012_R2, sizeof(OS_WINDOWS_SERVER_2012_R2));
								if(bufLen)
									*bufLen = sizeof(OS_WINDOWS_SERVER_2012_R2);
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
			case 10:
				{
					switch(osvInfo.dwMinorVersion)
					{
					case 0:
						{
							if(osvInfo.wProductType == VER_NT_WORKSTATION)
							{
								if(pOSNameBuf)
									memcpy(pOSNameBuf, OS_WINDOWS_10, sizeof(OS_WINDOWS_10));
								if(bufLen)
									*bufLen = sizeof(OS_WINDOWS_10);
							}
							else
							{
								if(pOSNameBuf)
									memcpy(pOSNameBuf, OS_WINDOWS_SERVER_2016, sizeof(OS_WINDOWS_SERVER_2016));
								if(bufLen)
									*bufLen = sizeof(OS_WINDOWS_SERVER_2016);

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
	if(isUnknow)
	{
		if(pOSNameBuf)
			memcpy(pOSNameBuf, OS_UNKNOW, sizeof(OS_UNKNOW));
		if(bufLen)
			*bufLen = sizeof(OS_UNKNOW);
	}
	bRet = true;
	return bRet;
#else
	return util_os_get_os_name_reg(pOSNameBuf, bufLen);
#endif
}

bool util_os_get_processor_name(char * pProcessorNameBuf, unsigned long * bufLen)
{
	bool bRet = false;
	HKEY hk;
	char regName[] = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
	char valueName[] = "ProcessorNameString";
	char valueData[256] = {0};    
	unsigned long  valueDataSize = sizeof(valueData);
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_ALL_ACCESS, &hk)) return false;
	else
	{
		if(ERROR_SUCCESS != RegQueryValueEx(hk, valueName, 0, NULL, (LPBYTE)valueData, &valueDataSize)) 
		{
			RegCloseKey(hk);
			return false;
		}
		RegCloseKey(hk);
		bRet = valueDataSize == 0 ? false : true;
	}
	if(bRet)
	{
		//unsigned int cpyLen = valueDataSize < *bufLen ? valueDataSize : *bufLen; 
		if(pProcessorNameBuf)
			memcpy(pProcessorNameBuf, valueData, valueDataSize);
		if(bufLen)
			*bufLen = valueDataSize;
	}
	return bRet;
}

bool util_os_get_architecture(char * pArchBuf, int *bufLen)
{
	bool bRet = true;
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
			if(pArchBuf)
				memcpy(pArchBuf, ARCH_X64, sizeof(ARCH_X64));
			if(bufLen)
				*bufLen = sizeof(ARCH_X64);
			break;
		}
	case PROCESSOR_ARCHITECTURE_ARM:
		{
			if(pArchBuf)
				memcpy(pArchBuf, ARCH_ARM, sizeof(ARCH_ARM));
			if(bufLen)
				*bufLen = sizeof(ARCH_ARM);
			break;
		}
	case PROCESSOR_ARCHITECTURE_IA64:
		{
			if(pArchBuf)
				memcpy(pArchBuf, ARCH_IA64, sizeof(ARCH_IA64));
			if(bufLen)
				*bufLen = sizeof(ARCH_IA64);
			break;
		}
	case PROCESSOR_ARCHITECTURE_INTEL:
		{
			if(pArchBuf)
				memcpy(pArchBuf, ARCH_X86, sizeof(ARCH_X86));
			if(bufLen)
				*bufLen = sizeof(ARCH_X86);
			break;
		}
	case PROCESSOR_ARCHITECTURE_UNKNOWN:
		{
			if(pArchBuf)
				memcpy(pArchBuf, ARCH_UNKNOW, sizeof(ARCH_UNKNOW));
			if(bufLen)
				*bufLen = sizeof(ARCH_UNKNOW);
			break;
		}
	default:
		bRet = false;
		break;
	}
	return bRet;
}

bool util_os_get_free_memory(uint64_t *totalPhysMemKB, uint64_t *availPhysMemKB)
{
	MEMORYSTATUSEX memStatex;
	memStatex.dwLength = sizeof (memStatex);
	GlobalMemoryStatusEx (&memStatex);

	if(totalPhysMemKB)
		*totalPhysMemKB = (long)(memStatex.ullTotalPhys/DIV);
	if(availPhysMemKB)
		*availPhysMemKB = (long)(memStatex.ullAvailPhys/DIV);

	return true;
}
/*
EVENT_HANDLE util_os_register_eventsource(char * serverName, char *sourceName)
{
	return RegisterEventSource(serverName, sourceName);
}

bool util_os_report_event(void* eventHandle, unsigned long dwEventID, char * eventStr)
{
	return ReportEvent(eventHandle, EVENTLOG_INFORMATION_TYPE, 0, dwEventID, NULL, 1, 0, (LPCSTR *)eventStr, NULL)?true:false;
}

bool util_os_deregister_eventsource(void* eventHandle)
{
	return DeregisterEventSource(eventHandle)?true:false;
}
*/