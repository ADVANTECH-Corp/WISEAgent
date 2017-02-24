#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#define pthread_t HANDLE

#define pthread_mutex_t HANDLE

#define pthread_mutexattr_t unsigned long

typedef struct { 
	UINT waiters_count;
	CRITICAL_SECTION waiters_count_lock;
	HANDLE signal_event;
	unsigned long num_wake;
	unsigned long generation;
} pthread_cond_t;



#if _MSC_VER < 1900
struct timespec {
	time_t   tv_sec;        /* seconds */
	long     tv_nsec;       /* nanoseconds */
};
#else 
#include <time.h> // VS2015 or WIN_IOT
#endif // WIN_IOT


#define pthread_condattr_t unsigned long

int pthread_create(pthread_t *thread, void *attr, void *(*start_routine)(void*), void *arg);

int pthread_join(pthread_t thread, void **retval);

pthread_t pthread_self(void);

int pthread_equal(pthread_t threadIdA, pthread_t threadIdB);

int pthread_cancel(pthread_t thread);

void pthread_exit(void *retval);

int pthread_detach(pthread_t thread);

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

int pthread_mutex_lock(pthread_mutex_t *mutex);

int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutex_destroy(pthread_mutex_t *mutex);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);

int pthread_cond_signal(pthread_cond_t *cond);

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

int pthread_cond_destroy(pthread_cond_t *cond);

#ifdef __cplusplus
}
#endif

#endif