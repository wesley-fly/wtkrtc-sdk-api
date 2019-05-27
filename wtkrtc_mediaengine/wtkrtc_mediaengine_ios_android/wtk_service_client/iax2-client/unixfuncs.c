/*
 * iaxclient: a cross-platform IAX softphone library
 *
 * Copyrights:
 * Copyright (C) 2003-2006, Horizon Wimba, Inc.
 * Copyright (C) 2007, Wimba, Inc.
 *
 * Contributors:
 * Steve Kann <stevek@stevek.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU Lesser (Library) General Public License.
 */

#define _BSD_SOURCE
#include <unistd.h>
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <time.h>
#include "iaxclient_lib.h"

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif


#ifdef ANDROID
#include <signal.h>
#endif

#define IAXC_NOTICE	IAXC_TEXT_TYPE_NOTICE

/*#ifndef NULL
#define NULL (0)
#endif*/

/* Unix-specific functions */

void os_init(void)
{
}

void iaxc_millisleep(long ms)
{
	struct timespec req;
#if 0
	req.tv_nsec = (ms%1000)*1000*1000;
	req.tv_sec = ms/1000;
#else /* we can make sure ms less than 1000 in WtkMedia*/
    req.tv_sec = 0;
    req.tv_nsec = ms*1000*1000;
#endif
    
    /* yes, it can return early.  We don't care */
    nanosleep(&req,NULL);
}


/* TODO: Implement for X/MacOSX? */
int iaxci_post_event_callback(iaxc_event ev)
{
#if 0
	iaxc_event *e;
	e = malloc(sizeof(ev));
	*e = ev;

	/* XXX Test return value? */
	PostMessage(post_event_handle,post_event_id,(WPARAM) NULL, (LPARAM) e);
#endif
	return 0;
}

#ifdef MACOSX
    /* Presently, OSX allows user-level processes to request RT
     * priority.  The API is nice, but the scheduler presently ignores
     * the parameters (but the API validates that you're not asking for
     * too much).  See
     * http://lists.apple.com/archives/darwin-development/2004/Feb/msg00079.html
     */
/* include mach stuff for declaration of thread_policy stuff */
#include <mach/mach.h>

int iaxci_prioboostbegin()
{
	struct thread_time_constraint_policy ttcpolicy;
	int params [2] = {CTL_HW,HW_BUS_FREQ};
	int hzms;
	size_t sz;
	int ret;

	/* get hz */
	sz = sizeof (hzms);
	sysctl (params, 2, &hzms, &sz, NULL, 0);

	/* make hzms actually hz per ms */
	hzms /= 1000;

	/* give us at least how much? 6-8ms every 10ms (we generally need less) */
	ttcpolicy.period = 10 * hzms; /* 10 ms */
	ttcpolicy.computation = 2 * hzms;
	ttcpolicy.constraint = 3 * hzms;
	ttcpolicy.preemptible = 1;

	if ( (ret = thread_policy_set(mach_thread_self(),
			THREAD_TIME_CONSTRAINT_POLICY, (int *)&ttcpolicy,
			THREAD_TIME_CONSTRAINT_POLICY_COUNT)) != KERN_SUCCESS )
	{
		fprintf(stderr, "thread_policy_set failed: %d.\n", ret);
	}
	return 0;
}

int iaxci_prioboostend()
{
    /* TODO */
    return 0;
}

#elif defined(ANDROID) // android platform thread: imported from webrtc by sunny

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <pthread.h>
#include <errno.h>

#define MAX_PRIORITY 99
#define MIN_PRIORITY 1
#define SCHEDULER_POLICY SCHED_RR //SCHED_FIFO
#define DBUG(...)
#define ERR_RPT(...) fprintf(stderr, __VA_ARGS__)

enum ThreadPriority
{
    kLowPriority = 1,
    kNormalPriority = 2,
    kHighPriority = 3,
    kHighestPriority = 4,
    kRealtimePriority = 5
};

pthread_attr_t _attr;

int android_pthread_create(void* tidp,void *restrict_attr,void*(*start_rtn)(void*),void *arg)
{
	int result = 0;
    // initialize pthread attributes
	if(restrict_attr == NULL) {
		pthread_attr_init(&_attr);
		restrict_attr = &_attr;
	}
		
	pthread_attr_setdetachstate(restrict_attr, PTHREAD_CREATE_DETACHED);
	result |= pthread_attr_setstacksize(&_attr, 1024*1024);
	result |= pthread_create(tidp, restrict_attr, start_rtn, arg);
	if (result != 0)    
	{   DBUG("android_pthread_create: Error!\n");     
        return -1;    
	}
	DBUG("android_pthread_create: OK!\n");     
	return 0;
}

int iaxci_prioboostbegin()
{
    struct sched_param param;

    int result = 0;
    // TODO: Adjust thread priority here
    int _prio = kRealtimePriority;

#ifdef WEBRTC_THREAD_RR
    const int policy = SCHED_RR;
#else
    const int policy = SCHED_FIFO;
#endif

    const int minPrio = sched_get_priority_min(policy);
    const int maxPrio = sched_get_priority_max(policy);
    if ((minPrio == EINVAL) || (maxPrio == EINVAL))
    {
        return 0;
    }

    // Choose priority
    switch (_prio)
    {
    case kLowPriority:
        param.sched_priority = minPrio + 1;
        break;
    case kNormalPriority:
        param.sched_priority = (minPrio + maxPrio) / 2;
        break;
    case kHighPriority:
        param.sched_priority = maxPrio - 3;
        break;
    case kHighestPriority:
        param.sched_priority = maxPrio - 2;
        break;
    case kRealtimePriority:
        param.sched_priority = maxPrio - 1;
        break;
    default:
        return 0;
    }
	
    // Inscrease thread's priority
    result = pthread_setschedparam(pthread_self(), policy, &param);
    if (result == EINVAL)
    {
        return 0;
    }
    return result;
}

int iaxci_prioboostend()
{
	pthread_attr_destroy(&_attr);
	return 0;
}

#else // Obsuleted: Unix/Linux


/* Priority boosting/monitoring:  Adapted from portaudio/pa_unix.c ,
 * which carries the following copyright notice:
 * PortAudio Portable Real-Time Audio Library
 * Latest Version at: http://www.portaudio.com
 * Linux OSS Implementation by douglas repetto and Phil Burk
 *
 * Copyright (c) 1999-2000 Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 */

/* It has been clarified by the authors that the request to send modifications
   is a request, and not a condition */

/* Theory:
 *  The main thread is boosted to a medium real-time priority.
 *  Two additional threads are created:
 *  Canary:  Runs as normal priority, updates a timevalue every second.
 *  WatchDog:  Runs as a higher real-time priority.  Checks to see that
 *	      Canary is running.  If Canary isn't running, lowers
 *	      priority of calling thread, which has presumably run away
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <pthread.h>
#include <errno.h>

//#define DBUG(...) fprintf(stderr, __VA_ARGS__)
#define DBUG(...)
#define ERR_RPT(...) fprintf(stderr, __VA_ARGS__)

#define SCHEDULER_POLICY SCHED_RR
#define WATCHDOG_INTERVAL_USEC 1000000
#define WATCHDOG_MAX_SECONDS 3

typedef void *(*pthread_function_t)(void *);

struct prioboost
{
	int priority;
	int min_priority;
	int max_priority;

	pthread_t ThreadID;

	struct timeval CanaryTime;
	int CanaryRun;
	pthread_t CanaryThread;
	int IsCanaryThreadValid;

	int WatchDogRun;
	pthread_t WatchDogThread;
	int IsWatchDogThreadValid;

};

static struct prioboost *pb;

static int CanaryProc(struct prioboost *b)
{
	int result = 0;
	struct sched_param schat = { 0 };

	/* set us up with normal priority, please */
	if( pthread_setschedparam(pthread_self(), SCHED_OTHER, &schat) != 0)
		return 1;

	while( b->CanaryRun)
	{
		usleep( WATCHDOG_INTERVAL_USEC );
		gettimeofday( &b->CanaryTime, NULL );
	}

	return result;
}

static int WatchDogProc(struct prioboost *b )
{
	struct sched_param schp = { 0 };
	struct sched_param schat = { 0 };
	int pri = b->priority + 4;

	/* Run at a priority level above main thread so we can still run if it
	 * hangs. Rise more than 1 because of rumored off-by-one scheduler
	 * bugs. */
	if ( pri > b->max_priority )
		pri = b->max_priority;

	for ( ; pri > b->priority; pri-- )
	{
		schp.sched_priority = pri;

		if ( pthread_setschedparam(pthread_self(), SCHEDULER_POLICY,
				&schp) )
			ERR_RPT("WatchDogProc: cannot set watch dog priority!"
					" %d\n", pri);
		else
			break;
	}

	/* If the watchdog thread cannot get a higher priority than the canary,
	 * the whole scheme falls apart. Bail.
	 */
	if ( pri <= b->priority )
		goto killAudio;

	DBUG("prioboost: WatchDog priority set to level %d!\n",
			schp.sched_priority);

	/* Compare watchdog time with audio and canary thread times. */
	/* Sleep for a while or until thread cancelled. */
	while ( b->WatchDogRun )
	{
		int delta;
		struct timeval currentTime;

		usleep( WATCHDOG_INTERVAL_USEC );
		gettimeofday( &currentTime, NULL );

#if 0
		/* If audio thread is not advancing, then it must be hung so kill it. */
		delta = currentTime.tv_sec - b->EntryTime.tv_sec;
		DBUG(("WatchDogProc: audio delta = %d\n", delta ));
		if( delta > WATCHDOG_MAX_SECONDS )
		{
			goto killAudio;
		}
#endif

		/* If canary died, then lower audio priority and halt canary. */
		delta = currentTime.tv_sec - b->CanaryTime.tv_sec;
		DBUG("WatchDogProc: dogging, delta = %ld, mysec=%d\n", delta, currentTime.tv_sec);
		if( delta > WATCHDOG_MAX_SECONDS )
		{
			ERR_RPT("WatchDogProc: canary died!\n");
			goto lowerAudio;
		}
	}

	DBUG("WatchDogProc: exiting.\n");
	return 0;

lowerAudio:
	if ( pthread_setschedparam(b->ThreadID, SCHED_OTHER, &schat) != 0 )
	{
		ERR_RPT("WatchDogProc: failed to lower audio priority. "
				"errno = %d\n", errno);
		/* Fall through into killing audio thread. */
	}
	else
	{
		ERR_RPT("WatchDogProc: lowered audio priority to prevent "
				"hogging of CPU.\n");
		goto cleanup;
	}

killAudio:
	ERR_RPT("WatchDogProc: killing hung audio thread!\n");
	//pthread_cancel( b->ThreadID);
	//pthread_join( b->ThreadID);
	exit(1);

cleanup:
	b->CanaryRun = 0;
	DBUG("WatchDogProc: cancel Canary\n");


	

#ifdef ANDROID   // add by swenson , NDK phtread not support  pthread_cancel
	pthread_exit(NULL); 
#else 
    pthread_cancel( b->CanaryThread );
#endif 

	DBUG("WatchDogProc: join Canary\n");
	pthread_join( b->CanaryThread, NULL );
	DBUG("WatchDogProc: forget Canary\n");
	b->IsCanaryThreadValid = 0;

#ifdef GNUSTEP
	GSUnregisterCurrentThread();  /* SB20010904 */
#endif
	return 0;
}



#ifdef ANDROID 
	void thread_exit_handler( int sig)
	{
        iaxci_usermsg(IAXC_NOTICE,"IAX:Thread exit .............................................\n");

        pthread_exit(NULL);
    }
#endif 




static void StopWatchDog(struct prioboost *b)
{


#ifdef  ANDROID  // add by swenson , NDK phtread not support  pthread_cancel

		struct sigaction actions;
		memset(&actions, 0, sizeof(actions)); 
		sigemptyset(&actions.sa_mask);
		actions.sa_flags = 0; 
		actions.sa_handler = thread_exit_handler;
		sigaction(SIGUSR1,&actions,NULL);
#endif 
	
	/* Cancel WatchDog thread if there is one. */
	if( b->IsWatchDogThreadValid )
	{
		b->WatchDogRun = 0;
		DBUG("StopWatchDog: cancel WatchDog\n");

#ifdef ANDROID		
        iaxci_usermsg(IAXC_NOTICE,"IAX:Thread exit .............................................\n");

		pthread_kill(b->WatchDogThread, SIGUSR1); 
#else 
		pthread_cancel( b->WatchDogThread );
#endif 
		pthread_join( b->WatchDogThread, NULL );
		b->IsWatchDogThreadValid = 0;
	}
	/* Cancel Canary thread if there is one. */
	if( b->IsCanaryThreadValid )
	{
		b->CanaryRun = 0;
		DBUG("StopWatchDog: cancel Canary\n");
		
#ifdef ANDROID	// add by swenson , NDK phtread not support  pthread_cancel	
        iaxci_usermsg(IAXC_NOTICE,"IAX:Thread exit .............................................\n");

		pthread_kill(b->WatchDogThread, SIGUSR1); 
#else 
		pthread_cancel( b->WatchDogThread );
#endif 
		DBUG("StopWatchDog: join Canary\n");
		pthread_join( b->CanaryThread, NULL );
		b->IsCanaryThreadValid = 0;
	}
}

static int StartWatchDog(struct prioboost *b)
{
	int  hres;
	int  result = 0;

	/* The watch dog watches for these timer updates */
	gettimeofday( &b->CanaryTime, NULL );

	/* Launch a canary thread to detect priority abuse. */
	b->CanaryRun = 1;
	hres = pthread_create(&(b->CanaryThread),
			NULL /*pthread_attr_t * attr*/,
			(pthread_function_t)CanaryProc, b);
	if( hres != 0 )
	{
		b->IsCanaryThreadValid = 0;
		result = 1;
		goto error;
	}
	b->IsCanaryThreadValid = 1;

	/* Launch a watchdog thread to prevent runaway audio thread. */
	b->WatchDogRun = 1;
	hres = pthread_create(&(b->WatchDogThread),
			NULL /*pthread_attr_t * attr*/,
			(pthread_function_t)WatchDogProc, b);
	if( hres != 0 )     {
		b->IsWatchDogThreadValid = 0;
		result = 1;
		goto error;
	}
	b->IsWatchDogThreadValid = 1;
	return result;

error:
	StopWatchDog( b );
	return result;
}

int iaxci_prioboostbegin()
{
	struct sched_param schp = { 0 };
	struct prioboost *b = calloc(1, sizeof(*b));

	int result = 0;

	b->min_priority = sched_get_priority_min(SCHEDULER_POLICY); 
	b->max_priority = sched_get_priority_max(SCHEDULER_POLICY);
	b->priority = (b->max_priority - b->min_priority) / 2;
	schp.sched_priority = b->priority;

	b->ThreadID = pthread_self();

	if (pthread_setschedparam(b->ThreadID, SCHEDULER_POLICY, &schp) != 0)
	{
		DBUG("prioboost: only superuser can use real-time priority.\n");
	}
	else
	{
		DBUG("prioboost: priority set to level %d!\n", schp.sched_priority);
		/* We are running at high priority so we should have a watchdog
		 * in case audio goes wild. */
		result = StartWatchDog( b );
	}

	if (result == 0)
	{
		pb = b;
	}
	else
	{
		pb = NULL;
		schp.sched_priority = 0;
		pthread_setschedparam(b->ThreadID, SCHED_OTHER, &schp);
		free(b);
	}

	return result;
}

int iaxci_prioboostend()
{
	if ( pb )
	{
		StopWatchDog(pb);
		free(pb);
	}
	return 0;
}

#endif

