#include <string.h>
#include <stdio.h>
#include <windows.h>

#include <sys/sem.h>
#include "export.h"
#include "wrapper.h"

ADVPLAT_EXPORT long ADVPLAT_CALL semget(key_t key, int nsems, int semflg) {

	HANDLE hSemObject;
#ifdef UNICODE
	WCHAR wsz[64];
	swprintf(wsz, 64, L"%X", key);
	LPCWSTR p = wsz;
#else
	CHAR wsz[64];
	snprintf(wsz, 64, "%X", key);
	LPCSTR p = wsz;
#endif
	return 0;
}

ADVPLAT_EXPORT int ADVPLAT_CALL semop(int semid, struct sembuf *sops, unsigned nsops) {
	return 0;
}