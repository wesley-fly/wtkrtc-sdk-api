#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FALSE 0 
#define TRUE 1

// Headers for Windows
#if defined(WIN32)  ||  defined(_WIN32_WCE)
#if !defined(_WIN32_WCE)
#include <process.h>
#endif

#include <winsock.h>
#include <stddef.h>
#include <time.h>

#define	PTT_CLOSE_SOCKET(s)  closesocket(s)
#define THREAD HANDLE
#define STRCPY(dst, src, size ) strcpy_s(dst,size,src)
// Headers for Linux or Unix
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#define PTT_CLOSE_SOCKET(s) close(s)
#define STRCPY(dst, src, size ) strncpy(dst,src,size)
#endif

/* os-dependent macros, etc */
#if defined(WIN32) || defined(_WIN32_WCE)
#define THREAD HANDLE
#define THREADID unsigned
#define THREADCREATE(func, args, thread, id) \
			(thread = (HANDLE)_beginthreadex(NULL, 0, func, (PVOID)args, 0, &id))
#define THREADCREATE_ERROR NULL
#define THREADFUNCDECL(func, args) unsigned __stdcall func(PVOID args)
#define THREADFUNCRET(r) int r = 0
//#define THREADJOIN(t) /* causes deadlock with wx GUI on MSW */
#define THREADJOIN(t) WaitForSingleObject( t, 10000 /* org: INFINITE */ )
#define SET_PTHREAD_PRIORITY   SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL)

#ifndef _WIN32_WINNT
extern WINBASEAPI BOOL WINAPI TryEnterCriticalSection( LPCRITICAL_SECTION lpCriticalSection );
#endif
#define MUTEX CRITICAL_SECTION
#define MUTEXINIT(m) InitializeCriticalSection(m)
#define MUTEXLOCK(m) EnterCriticalSection(m)
#define MUTEXTRYLOCK(m) (!TryEnterCriticalSection(m))
#define MUTEXUNLOCK(m) LeaveCriticalSection(m)
#define MUTEXDESTROY(m) DeleteCriticalSection(m)

#else /* not WIN32 */

#ifdef ANDROID /* Android */
extern int android_pthread_create(void* tidp, void*restrict_attr, void*(*start_rtn)(void*), void*arg);
#define THREADCREATE(func, args, thread, id) \
    android_pthread_create(&thread, NULL, func, args)
#else /* MacOS/iOS/Linux */
#define THREADCREATE(func, args, thread, id) \
    pthread_create(&thread, NULL, func, args)
#endif

#define THREAD pthread_t
#define THREADID unsigned /* unused for Posix Threads */
#define THREADCREATE_ERROR -1
#define THREADFUNCDECL(func, args) void * func(void *args)
#define THREADFUNCRET(r) void * r = 0
#define THREADJOIN(t) pthread_join(t, 0)
#define MUTEX pthread_mutex_t
#define MUTEXINIT(m) pthread_mutex_init(m, NULL) //TODO: check error
#define MUTEXLOCK(m) pthread_mutex_lock(m)
#define MUTEXTRYLOCK(m) pthread_mutex_trylock(m)
#define MUTEXUNLOCK(m) pthread_mutex_unlock(m)
#define MUTEXDESTROY(m) pthread_mutex_destroy(m)
#endif

// PTT QUEUE
#define PUSH_QUEUE_TAIL(e,q,l) { MUTEXLOCK(&l);\
                                if(q==NULL) {q=e; q->tail=q;}\
                                else { q->tail->next=e; q->tail=e;}\
                                MUTEXUNLOCK(&l); }
// pop an rtp event from queue head
#define POP_QUEUE_HEAD(e,q,l) { MUTEXLOCK(&l);\
                                if(q!=NULL) { e=q; q=q->next; if(q!=NULL) q->tail=e->tail; \
                                              e->tail=NULL; e->next=NULL; }\
                                else e=NULL;\
                                MUTEXUNLOCK(&l); }
// attach a queue to another queue tail
#define ATTACH_QUEUE_TAIL(e,q,l) { MUTEXLOCK(&l); \
                                   if(q==NULL) q=e; \
                                   else q->tail->next = e;\
                                   q->tail=e->tail;\
                                   MUTEXUNLOCK(&l); }
// release a queue
#define RELEASE_QUEUE(q,l,t) { MUTEXLOCK(&l); \
                               while(q!=NULL) { t=q; q=q->next; free(t->data); free(t); }\
                               MUTEXUNLOCK(&l); }

// flush queue
#define FLUSH_QUEUE(e,q,l) { MUTEXLOCK(&l); \
                             e=q; q=NULL; \
                             MUTEXUNLOCK(&l);}

#endif 
