#include "util_process.h"
#include <Windows.h>
#include <tlhelp32.h>

bool util_process_launch(char * appPath)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if(NULL == appPath) return -1;

	memset(&si, 0, sizeof(si));
	si.dwFlags = STARTF_USESHOWWINDOW;  
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	if(CreateProcess(appPath, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW , NULL, NULL, &si, &pi))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool adjust_privileges() 
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	TOKEN_PRIVILEGES oldtp;
	DWORD dwSize=sizeof(TOKEN_PRIVILEGES);
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
	{
		if (GetLastError()==ERROR_CALL_NOT_IMPLEMENTED) return true;
		else return false;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) 
	{
		CloseHandle(hToken);
		return false;
	}

	memset(&tp, 0, sizeof(tp));
	tp.PrivilegeCount=1;
	tp.Privileges[0].Luid=luid;
	tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize)) 
	{
		CloseHandle(hToken);
		return false;
	}

	CloseHandle(hToken);
	return true;
}

bool util_process_kill(char * processName)
{
	bool bRet = false;
	PROCESSENTRY32 pe;
	HANDLE hSnapshot=NULL;
	if(NULL == processName) return bRet;
	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe.dwSize=sizeof(PROCESSENTRY32);
	if(!Process32First(hSnapshot,&pe))
	{
		CloseHandle(hSnapshot);
		return bRet;
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
				bRet = true;
				CloseHandle(hPrc);
			}

			break;
		}
	}
	CloseHandle(hSnapshot);
	return bRet;
}