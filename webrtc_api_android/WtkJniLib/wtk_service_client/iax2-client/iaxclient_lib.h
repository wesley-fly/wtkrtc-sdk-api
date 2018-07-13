/*
 * iaxclient: a cross-platform IAX softphone library
 *
 * Copyrights:
 * Copyright (C) 2003-2006, Horizon Wimba, Inc.
 * Copyright (C) 2007, Wimba, Inc.
 *
 * Contributors:
 * Steve Kann <stevek@stevek.com>
 * Michael Van Donselaar <mvand@vandonselaar.org>
 * Shawn Lawrence <shawn.lawrence@terracecomm.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU Lesser (Library) General Public License.
 */
#ifndef _iaxclient_lib_h
#define _iaxclient_lib_h

#ifdef __cplusplus
extern "C" {
#endif


/* This is the internal include file for IAXCLIENT -- externally
 * accessible APIs should be declared in iaxclient.h */
#include <stdio.h>
#include <string.h>

#if defined(WIN32)  ||  defined(_WIN32_WCE)
#include <winsock.h>
#if !defined(_WIN32_WCE)
#include <process.h>
#endif
#include <stddef.h>
#include <time.h>

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>
#endif

#include <stdlib.h>
#include <math.h>

/* os-dependent macros, etc */
#if defined(WIN32) || defined(_WIN32_WCE)
#define THREAD HANDLE
#define THREADID unsigned
#define THREADCREATE(func, args, thread, id) \
(thread = (HANDLE)_beginthreadex(NULL, 0, func, (PVOID)args, 0, &id))
#define THREADCREATE_ERROR NULL
#define THREADFUNCDECL(func) unsigned __stdcall func(PVOID args)
#define THREADFUNCRET(r) int r = 0
#define THREADJOIN(t)
/* causes deadlock with wx GUI on MSW */
/* #define THREADJOIN(t) WaitForSingleObject(t, INFINITE) */
#ifndef _WIN32_WINNT
extern WINBASEAPI BOOL WINAPI TryEnterCriticalSection( LPCRITICAL_SECTION lpCriticalSection );
#endif
#define MUTEX CRITICAL_SECTION
#define MUTEXINIT(m) InitializeCriticalSection(m)
#define MUTEXLOCK(m) EnterCriticalSection(m)
#define MUTEXTRYLOCK(m) (!TryEnterCriticalSection(m))
#define MUTEXUNLOCK(m) LeaveCriticalSection(m)
#define MUTEXDESTROY(m) DeleteCriticalSection(m)

#else
#ifdef ANDROID
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
#define THREADFUNCDECL(func) void * func(void *args)
#define THREADFUNCRET(r) void * r = 0
#define THREADJOIN(t) pthread_join(t, 0)
#define MUTEX pthread_mutex_t
#define MUTEXINIT(m) pthread_mutex_init(m, NULL) //TODO: check error
#define MUTEXLOCK(m) pthread_mutex_lock(m)
#define MUTEXTRYLOCK(m) pthread_mutex_trylock(m)
#define MUTEXUNLOCK(m) pthread_mutex_unlock(m)
#define MUTEXDESTROY(m) pthread_mutex_destroy(m)
#endif

#ifdef MACOSX
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <sched.h>
#include <sys/sysctl.h>
#endif
//Xiaofan start
#define	WTKCALL_AUDIO_CODECS         	( IAXC_FORMAT_G729A | IAXC_FORMAT_OPUS)
#define WTKCALL_AUDIO_PREFERED       	( IAXC_FORMAT_G729A )
#define	WTKCALL_VIDEO_CODECS	   		( IAXC_FORMAT_H264 )
#define WTKCALL_VIDEO_PREFERED  		( IAXC_FORMAT_H264 )
#define RTP_HEADER_VERSION 				2
#define RTP_HEADER_SIZE    				12
#define RTP_PADDING         			0x20
#define RTP_PADDING_PARTCB  			0x40
#define RTP_PADDING_ENCRYPT 			0x80
#define PKTSIZE_ILBC_DEFAULT  			50
#define PKTSIZE_G729A_DEFAULT 			30
#define FORMAT_G729A_PT					18
#define FORMAT_OPUS_PT					109
#define FORMAT_ILBC_PT					102

#define IAXC_MEDIA_STATE_NONE					0  /* None effect*/
#define IAXC_MEDIA_STATE_MUTEMULTIPARTY			(1<<1)  /* Muted by the presider of a multiparty call*/
#define IAXC_MEDIA_STATE_MIXED                  (1<<2)  /* multiparty call */
#define IAXC_MEDIA_STATE_RECAUDIO               (1<<3)  /* audio stream recorded */
#define IAXC_MEDIA_STATE_RECVIDEO               (1<<4)  /* video stream recorded */
#define IAXC_MEDIA_STATE_HOLD                   (1<<5)  /* call hold */
#define IAXC_MEDIA_STATE_MUTE                   (1<<6)  /* call muted */ 
#define IAXC_MEDIA_STATE_REMOTEVIDEO            (1<<7)  /* receive remote video */
#define IAXC_MEDIA_STATE_LOCALVIDEO             (1<<8)  /* send local video */
#define IAXC_MEDIA_STATE_NORTP					(1<<9)  /* send voice data w/o RTP header */
#define IAXC_MEDIA_STATE_FORWARD				(1<<10) /* forward-dir call */

#define MAX_SHORT          				65536

enum _RTP_Audio_Format
{
    RTP_FORMAT_ILBC=0,	/* iLBC Audio with SN and TS extension */
    RTP_FORMAT_G729A=1,	/* ITU-T G.729 */
    RTP_FORMAT_OPUS=2, /* OPUS codec*/
};
#define IAXC_ERROR  IAXC_TEXT_TYPE_ERROR
#define IAXC_STATUS IAXC_TEXT_TYPE_STATUS
#define IAXC_NOTICE IAXC_TEXT_TYPE_NOTICE

#define DEFAULT_CALLERID_NAME    "Wattertek Custom"
#define DEFAULT_CALLERID_NUMBER  "1234567890"

//Xiaofan end
int selected_call; // XXX to be protected by mutex?
struct iaxc_call* calls;
int max_calls; // number of calls for this library session
uint64_t audio_format_capability;
uint64_t audio_format_preferred;
uint64_t video_format_capability;
uint64_t video_format_preferred;
//int new_callid; 


void os_init(void);
void iaxci_usermsg(int type, const char *fmt, ...);
void iaxci_do_levels_callback(float input, float output);
void iaxci_do_audio_callback(int callNo, unsigned int ts, int remote,
		int encoded, int format, int size, unsigned char *data);

#include "iaxclient.h"
#include "../wtkcall_lib.h"

#define MAX_TRUNK_LEN	(1<<16)
#define MAX_NO_SLICES	32

struct wtkState 
{
	//int	 id;			// unique identifier of this call's state machine 
	int  activity;		// the activity which application should perform 
	//int  last_state;	// previous state of this call

	struct timeval tm_start;    // time to setup the call
	struct timeval tm_answer;   // time to answer the call
	struct timeval tm_transfer; // p2p transfer finished time

	unsigned int  duration;		// duration of the call
	unsigned char type;			// see _Call_Type definition
	int reason;
	short hangup;			  // whether the hangup activity is invoked by self, and why hangup

	char local_codecs[32];  // codecs supported for sending
};

struct iaxc_call {
	/* the "state" of this call */
	int state;
	char remote[IAXC_EVENT_BUFSIZ];
	char remote_name[IAXC_EVENT_BUFSIZ];
	char local[IAXC_EVENT_BUFSIZ];
	char local_context[IAXC_EVENT_BUFSIZ];

	/* Outbound CallerID */
	char callerid_name[IAXC_EVENT_BUFSIZ];
	char callerid_number[IAXC_EVENT_BUFSIZ];

	/* reset whenever we receive packets from remote */
	struct 	 timeval 	last_activity;
	struct 	 timeval 	last_ping;

	/* our negotiated format */
	uint64_t format;
	uint64_t vformat;

	/* we've sent a silent frame since the last audio frame */
	int tx_silent;

	struct iax_session *session;
	
	//WTK
	int  mstate;
	struct wtkState sm;
};

/* post an event to the application */
void iaxci_post_event(iaxc_event e);

/* Priority boost support */
extern int iaxci_prioboostbegin(void);
extern int iaxci_prioboostend(void);

long iaxci_usecdiff(struct timeval *t0, struct timeval *t1);
long iaxci_msecdiff(struct timeval *t0, struct timeval *t1);

extern void  wtkcall_set_jni_event_callback( wtkcall_jni_event_callback_t func );
extern void  wtkcall_perform_registration_callback(int id, int reply);
extern void  wtkcall_perform_state_callback(struct wtkState* call, int callNo, int state, char* name, char* number);
extern void  wtkcall_perform_message_callback(struct wtkState* call, char *message);
extern void  wtkcall_perform_text_callback(char* text);
//extern int   get_new_callid();

#ifdef __cplusplus
}
#endif
#endif
