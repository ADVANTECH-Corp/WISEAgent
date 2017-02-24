#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <io.h>
#include <memory.h> 
#include <sys/shm.h>
#include "export.h"
#include "wrapper.h"

typedef struct {
	HANDLE mapObject;
	size_t size;
	int init;
} SHMID;

long ADVPLAT_CALL shmget(key_t key, size_t size, int shmflg) {

	SHMID *shmid = NULL;
	HANDLE hMapObject = NULL;  // handle to file mapping
	BOOL fInit;
#ifdef UNICODE
	WCHAR wsz[64];
	swprintf(wsz, 64, L"%X", key);
	LPCWSTR p = wsz;
#else
	CHAR wsz[64];
	snprintf(wsz, 64, "%X", key);
	LPCSTR p = wsz;
#endif


	hMapObject = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		p);               // name of mapping object

	
	
	
	if (hMapObject == NULL) {
		if (shmflg & IPC_CREAT) {
			hMapObject = CreateFileMapping(
				INVALID_HANDLE_VALUE,   // use paging file
				NULL,                   // default security attributes
				PAGE_READWRITE,         // read/write access
				0,                      // size: high 32-bits
				size + sizeof(SHMID),  // size: low 32-bits
				p); // name of map object
			// The first process to attach initializes memory

			fInit = (GetLastError() != ERROR_ALREADY_EXISTS);
		}
		if (hMapObject == NULL) return -1;
		shmid = (SHMID *)malloc(sizeof(SHMID));
		shmid->mapObject = hMapObject;
		shmid->size = size;
		shmid->init = 1;
	}
	else 
	{
		if ((shmflg & IPC_EXCL) && (shmflg & IPC_CREAT)) {
			CloseHandle(hMapObject);
			return -1;
		}
		shmid = (SHMID *)malloc(sizeof(SHMID));
		shmid->mapObject = hMapObject;
		shmid->size = size;
		shmid->init = 0;
	}

	return (long)shmid;
}
void * ADVPLAT_CALL shmat(long shmid, const void *shmaddr, int shmflg) {
	SHMID *id = (SHMID *)shmid;
	char *ptr = NULL;
	// Get a pointer to the file-mapped shared memory
	ptr = (char *)MapViewOfFile(
		(HANDLE)id->mapObject,     // object to map view of
		FILE_MAP_WRITE, // read/write access
		0,              // high offset:  map from
		0,              // low offset:   beginning
		0);             // default: map entire file
	if (ptr == NULL)
		return FALSE;
	CloseHandle(id->mapObject);

	// Initialize memory if this is the first process

	if (id->init)
		memset(ptr, '\0', id->size + sizeof(SHMID));

	id->init = 0;

	*(SHMID *)ptr = *id;
	free(id);
	ptr += sizeof(SHMID);
	return ptr;
}

int ADVPLAT_CALL shmdt(const void *shmaddr) {
	BOOL fRet;
	if((long)shmaddr < sizeof(SHMID)) return -1;
	
	char *ptr = (char *)shmaddr - sizeof(SHMID);
	SHMID shmid = *(SHMID *)ptr;
	fRet = UnmapViewOfFile(ptr);
	return 0;
}