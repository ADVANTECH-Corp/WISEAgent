#include "pthread.h"

int ADVPLAT_CALL pthread_create(pthread_t *thread, void *attr,void *(*start_routine) (void *), void *arg) {
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)start_routine,arg,0,thread);
	return 0;
}

int ADVPLAT_CALL pthread_join(pthread_t thread, void **retval) {
	DWORD exitCode;
	HANDLE h = OpenThread(THREAD_QUERY_INFORMATION, FALSE, thread);
	WaitForSingleObject(h, INFINITE);
	GetExitCodeThread(h, &exitCode);
	CloseHandle(h);
	*retval = (void *)exitCode;
	return 0;
}

int ADVPLAT_CALL pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t * attr) {
	*mutex=CreateMutex(NULL,FALSE,NULL);
	return 0;
}

