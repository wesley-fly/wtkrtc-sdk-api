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
 * Frik Strecker <frik@gatherworks.com>
 * Mihai Balea <mihai AT hates DOT ms>
 * Peter Grayson <jpgrayson@gmail.com>
 * Bill Cholewka <bcholew@gmail.com>
 * Erik Bunce <kde@bunce.us>
 *
 * This program is free software, distributed under the terms of
 * the GNU Lesser (Library) General Public License.
 */

#include <assert.h>

#if defined(WIN32) || defined(_WIN32_WCE)
#include <stdlib.h>
#endif

/* Win32 has _vsnprintf instead of vsnprintf */
/*#if ! HAVE_VSNPRINTF
# if HAVE__VSNPRINTF
#  define vsnprintf _vsnprintf
# endif
#endif*/
#if defined(WIN32)
#define vsnprintf _vsnprintf
#endif

#include "iaxclient_lib.h"
#include "iax-client.h"

#if STDC_HEADERS
# include <stdarg.h>
#else
# if HAVE_VARARGS_H
#  include <varargs.h>
# endif
#endif

struct iaxc_registration
{
	struct iax_session *session;
	struct timeval last;
	char host[256];
	char user[256];
	char pass[256];
	long refresh;
	int id;
	struct iaxc_registration *next;
};

static int next_registration_id = 0;
static struct iaxc_registration *registrations = NULL;

static int minimum_outgoing_framesize = 160; /* 20ms */

static MUTEX iaxc_lock;
static MUTEX event_queue_lock;

static int iaxci_bound_port = -1;

// default to use port 4569 unless set by iaxc_set_preferred_source_udp_port
static int source_udp_port = IAX_DEFAULT_PORTNO;

static void service_network(void);
static int service_audio(void);

/* external global networking replacements */
static iaxc_sendto_t iaxc_sendto = (iaxc_sendto_t)sendto;
static iaxc_recvfrom_t iaxc_recvfrom = (iaxc_recvfrom_t)recvfrom;


static THREAD main_proc_thread;
#if defined(WIN32) || defined(_WIN32_WCE)
static THREADID main_proc_thread_id;
#endif

/* 0 running, 1 should quit, -1 not running */
static int main_proc_thread_flag = -1;

static iaxc_event_callback_t iaxc_event_callback = NULL;

// Internal queue of events, waiting to be posted once the library
// lock is released.
static iaxc_event *event_queue = NULL;
#define	DEFSTRVAL(x,y)	((x)?(x):(y))
void wtkState_clear_state(struct wtkState* call)
{
	call->activity = CALL_FREE;
	call->type = CALL_TYPE_IDEL;
	
	memset(&call->tm_start, 0, sizeof(struct timeval));
	memset(&call->tm_answer, 0, sizeof(struct timeval));
	
	call->duration = 0;
	call->hangup = 0; // not self, no timeout out
	call->reason = 0;
}
struct string_array
{
	char* text;
	short  data;  // reserved
	struct string_array* next;
};

static void release_string_array(struct string_array* strs)
{
	struct string_array* head = strs;
	struct string_array* node;
	while(head!=NULL)
	{
		node = head;
		head = head->next;
		
		free(node->text);
		free(node);
	}
}

static struct string_array* parse_string_array(const char*str, char delim)
{
	char text[IAXC_EVENT_BUFSIZ];
	int len, i, index, data;
	
	struct string_array* head = NULL;
	struct string_array* node = NULL;
	struct string_array* tail = NULL;

	index = 0;
	data = 0;

	len = (int)strlen(str);
	for(i=0; i<len; i++)
	{
		if(str[i]!=delim) 
		{
			text[index++] = str[i];
			text[index] = '\0';
			
			if(i<len-1) continue;
		}
		
		// create a text node
        node = (struct string_array*)malloc(sizeof(struct string_array));
		node->text = (char*)malloc(index+1);
		memcpy(node->text, text, index+1);
		node->data = data++;  // for priority
		node->next = NULL;
		if(head==NULL)	
			head = node;
		else
			tail->next = node;
		
		// move to next
		tail = node;
		index = 0;
	}
	return head;
}

// Lock the library
EXPORT void get_iaxc_lock(void)
{
	MUTEXLOCK(&iaxc_lock);
}

int try_iaxc_lock()
{
	return MUTEXTRYLOCK(&iaxc_lock);
}

// Unlock the library and post any events that were queued in the meantime
EXPORT void put_iaxc_lock(void)
{
	iaxc_event *prev, *event;

	MUTEXLOCK(&event_queue_lock);
	event = event_queue;
	event_queue = NULL;
	MUTEXUNLOCK(&event_queue_lock);

	MUTEXUNLOCK(&iaxc_lock);

	while (event)
	{
		iaxci_post_event(*event);
		prev = event;
		event = event->next;
		free(prev);
	}
}

long iaxci_usecdiff(struct timeval * t0, struct timeval * t1)
{
	return (t0->tv_sec - t1->tv_sec) * 1000000L + t0->tv_usec - t1->tv_usec;
}

long iaxci_msecdiff(struct timeval * t0, struct timeval * t1)
{
	return iaxci_usecdiff(t0, t1) / 1000L;
}

EXPORT void iaxc_set_event_callback(iaxc_event_callback_t func)
{
	iaxc_event_callback = func;
}

EXPORT void iaxc_free_event(iaxc_event *e)
{
	free(e);
}

EXPORT struct iaxc_ev_levels *iaxc_get_event_levels(iaxc_event *e)
{
	return &e->ev.levels;
}

EXPORT struct iaxc_ev_text *iaxc_get_event_text(iaxc_event *e)
{
	return &e->ev.text;
}

EXPORT struct iaxc_ev_call_state *iaxc_get_event_state(iaxc_event *e)
{
	return &e->ev.call;
}

// Messaging functions
static void default_message_callback(const char * message)
{
	fprintf(stderr, "WTK-IAXCLIENT: %s\n", message);
}

// Post Events back to clients
void iaxci_post_event(iaxc_event e)
{
	if ( e.type == 0 )
	{
		iaxci_usermsg(IAXC_ERROR,
			"Error: something posted to us an invalid event");
		return;
	}

	if ( MUTEXTRYLOCK(&iaxc_lock) )
	{
		iaxc_event **tail;

		MUTEXLOCK(&event_queue_lock);
		tail = &event_queue;
		e.next = NULL;
		while ( *tail )
			tail = &(*tail)->next;
		*tail = (iaxc_event *)malloc(sizeof(iaxc_event));
		memcpy(*tail, &e, sizeof(iaxc_event));
		MUTEXUNLOCK(&event_queue_lock);
		return;
	}

	MUTEXUNLOCK(&iaxc_lock);

	if ( iaxc_event_callback )
	{
		int rv;

		rv = iaxc_event_callback(e);

		if ( e.type == IAXC_EVENT_VIDEO )
		{
			free(e.ev.video.data);
		}
		else if ( e.type == IAXC_EVENT_AUDIO )
		{
			free(e.ev.audio.data);
		}

		if ( rv < 0 )
			default_message_callback("BIG PROBLEM, event callback returned failure!");
		// > 0 means processed
		if ( rv > 0 )
			return;

		// else, fall through to "defaults"
	}
	else
	{
		default_message_callback("BIG PROBLEM, event callback is null!");
	}
	switch ( e.type )
	{
		case IAXC_EVENT_TEXT:
			default_message_callback(e.ev.text.message);
			// others we just ignore too
			
		return;
	}
}


void iaxci_usermsg(int type, const char *fmt, ...)
{
	va_list args;
	iaxc_event e;

	e.type = IAXC_EVENT_TEXT;
	e.ev.text.type = type;
	e.ev.text.callNo = -1;
	va_start(args, fmt);
	vsnprintf(e.ev.text.message, IAXC_EVENT_BUFSIZ, fmt, args);
	va_end(args);

	iaxci_post_event(e);
}


void iaxci_do_levels_callback(float input, float output)
{
	iaxc_event e;
	e.type = IAXC_EVENT_LEVELS;
	e.ev.levels.input = input;
	e.ev.levels.output = output;
	iaxci_post_event(e);
}

void iaxci_do_state_callback(int callNo)
{
	iaxc_event e;
	if ( callNo < 0 || callNo >= max_calls )
		return;
	e.type = IAXC_EVENT_STATE;
	e.ev.call.callNo = callNo;
	e.ev.call.state = calls[callNo].state;
	e.ev.call.format = calls[callNo].format;
	e.ev.call.vformat = calls[callNo].vformat;
	strncpy(e.ev.call.remote,        calls[callNo].remote,        IAXC_EVENT_BUFSIZ);
	strncpy(e.ev.call.remote_name,   calls[callNo].remote_name,   IAXC_EVENT_BUFSIZ);
	strncpy(e.ev.call.local,         calls[callNo].local,         IAXC_EVENT_BUFSIZ);
	strncpy(e.ev.call.local_context, calls[callNo].local_context, IAXC_EVENT_BUFSIZ);
	iaxci_post_event(e);
}

void iaxci_do_registration_callback(int id, int reply, int msgcount)
{
	iaxc_event e;
	e.type = IAXC_EVENT_REGISTRATION;
	e.ev.reg.id = id;
	e.ev.reg.reply = reply;
	e.ev.reg.msgcount = msgcount;
	iaxci_post_event(e);
}

void iaxci_do_audio_callback(int callNo, unsigned int ts, int source,
		int encoded, int format, int size, unsigned char *data)
{
	iaxc_event e;

	e.type = IAXC_EVENT_AUDIO;
	e.ev.audio.ts = ts;
	e.ev.audio.encoded = encoded;
	assert(source == IAXC_SOURCE_REMOTE || source == IAXC_SOURCE_LOCAL);
	e.ev.audio.source = source;
	e.ev.audio.size = size;
	e.ev.audio.callNo = callNo;
	e.ev.audio.format = format;

	e.ev.audio.data = (unsigned char *)malloc(size);

	if ( !e.ev.audio.data )
	{
		iaxci_usermsg(IAXC_ERROR,
				"failed to allocate memory for audio event");
		return;
	}

	memcpy(e.ev.audio.data, data, size);

	iaxci_post_event(e);
}

void iaxci_do_dtmf_callback(int callNo, char digit)
{
	iaxc_event e;
	e.type = IAXC_EVENT_DTMF;
	e.ev.dtmf.callNo = callNo;
	e.ev.dtmf.digit  = digit;
	iaxci_post_event(e);
}

int iaxc_remove_registration_by_id(int id)
{
	struct iaxc_registration *curr, *prev;
	int count = 0;
	for ( prev = NULL, curr = registrations; curr != NULL;
			prev = curr, curr = curr->next )
	{
		if ( curr->id == id )
		{
			count++;
			if ( curr->session != NULL )
				iax_destroy( curr->session );
			if ( prev != NULL )
				prev->next = curr->next;
			else
				registrations = curr->next;
			free( curr );
			break;
		}
	}
	return count;
}

EXPORT int iaxc_first_free_call()
{
	int i;
	for ( i = 0; i < max_calls; i++ )
		if ( calls[i].state == IAXC_CALL_STATE_FREE )
			return i;

	return -1;
}


EXPORT void iaxc_clear_call(int toDump)
{
	calls[toDump].state = IAXC_CALL_STATE_FREE;
	calls[toDump].format = 0;
	calls[toDump].vformat = 0;
	calls[toDump].mstate = IAXC_MEDIA_STATE_NONE;
	calls[toDump].rtp_seqno = 0;
	calls[toDump].rtp_ts = 0;
	calls[toDump].vrtp_seqno = 0;
	calls[toDump].vrtp_ts = 0;
	if (calls[toDump].session)
	{
		iax_destroy(calls[toDump].session);
		calls[toDump].session = NULL;
	}
	iaxci_do_state_callback(toDump);

	//wtkState_clear_state(&calls[toDump].sm);
}

/* select a call.  */
/* XXX Locking??  Start/stop audio?? */
EXPORT int iaxc_select_call(int callNo)
{
	if ( callNo >= max_calls )
	{
		iaxci_usermsg(IAXC_ERROR, "Error: tried to select out_of_range call %d", callNo);
		return -1;
	}

	if ( callNo < 0 )
	{
		if ( selected_call >= 0 )
		{
			calls[selected_call].state &= ~IAXC_CALL_STATE_SELECTED;
		}
		selected_call = callNo;
		return 0;
	}

	// de-select and notify the old call if not also the new call
	if ( callNo != selected_call )
	{
		if ( selected_call >= 0 )
		{
			calls[selected_call].state &= ~IAXC_CALL_STATE_SELECTED;
			iaxci_do_state_callback(selected_call);
		}
		selected_call = callNo;
		calls[selected_call].state |= IAXC_CALL_STATE_SELECTED;
	}

	// if it's an incoming call, and ringing, answer it.
	if ( !(calls[selected_call].state & IAXC_CALL_STATE_OUTGOING) &&
	      (calls[selected_call].state & IAXC_CALL_STATE_RINGING) )
	{
		iaxc_answer_call(selected_call);
	} else
	{
		calls[selected_call].state |= IAXC_CALL_STATE_SELECTED;
		iaxci_do_state_callback(selected_call);
	}

	return 0;
}

/* external API accessor */
EXPORT int iaxc_selected_call()
{
	return selected_call;
}

EXPORT void iaxc_set_networking(iaxc_sendto_t st, iaxc_recvfrom_t rf)
{
	iaxc_sendto = st;
	iaxc_recvfrom = rf;
}

// Note: Must be called before iaxc_initialize()
EXPORT void iaxc_set_preferred_source_udp_port(int port)
{
	source_udp_port = port;
}

EXPORT int iaxc_get_bind_port()
{
	return iaxci_bound_port;
}
static void iaxci_notice( const char* str ) { iaxci_usermsg( IAXC_TEXT_TYPE_NOTICE, str ); }
static void iaxci_error( const char* str )  { iaxci_usermsg( IAXC_TEXT_TYPE_ERROR, str );   }

EXPORT int iaxc_initialize(int num_calls)
{
	int i;
	int port;

	os_init();

	MUTEXINIT(&iaxc_lock);
	MUTEXINIT(&event_queue_lock);
	
	iax_set_output( iaxci_notice );
	iax_set_error( iaxci_error );

	if ( iaxc_recvfrom != (iaxc_recvfrom_t)recvfrom )
		iax_set_networking(iaxc_sendto, iaxc_recvfrom);

	/* Note that iax_init() only sets up the receive port when the
	 * sendto/recvfrom functions have not been replaced. We need
	 * to call iaxc_init in either case because there is other
	 * initialization beyond the socket setup that needs to be done.
	 */
	if ( (port = iax_init(0)) < 0 )//source_udp_port
	{
		iaxci_usermsg(IAXC_ERROR,
				"Fatal error: failed to initialize iax with port %d",
				port);
		return -1;
	}

	if ( iaxc_recvfrom == (iaxc_recvfrom_t)recvfrom )
		iaxci_bound_port = port;
	else
		iaxci_bound_port = -1;

	max_calls = num_calls;
	/* initialize calls */
	if ( max_calls <= 0 )
		max_calls = 1; /* 0 == Default? */

	/* calloc zeroes for us */
	calls = (struct iaxc_call *)calloc(sizeof(struct iaxc_call), max_calls);
	if ( !calls )
	{
		iaxci_usermsg(IAXC_ERROR, "Fatal error: can't allocate memory");
		return -1;
	}

	selected_call = -1;

	for ( i = 0; i < max_calls; i++ )
	{
		strncpy(calls[i].callerid_name,   DEFAULT_CALLERID_NAME,   IAXC_EVENT_BUFSIZ);
		strncpy(calls[i].callerid_number, DEFAULT_CALLERID_NUMBER, IAXC_EVENT_BUFSIZ);
		wtkState_clear_state(&calls[i].sm);
	}
	

	/* Default audio format capabilities */
	audio_format_capability = WTKCALL_AUDIO_CODECS;
	audio_format_preferred = WTKCALL_AUDIO_PREFERED;
	video_format_capability = WTKCALL_VIDEO_CODECS;
	video_format_preferred = WTKCALL_VIDEO_PREFERED;

	return 0;
}

EXPORT void iaxc_shutdown()
{
	iaxc_dump_all_calls();

	get_iaxc_lock();
	
	if ( calls ) 
	{
		free(calls);
		calls = NULL;
	}
	put_iaxc_lock();
#ifdef WIN32
	closesocket(iax_get_fd());
#endif

	MUTEXDESTROY(&event_queue_lock);
	MUTEXDESTROY(&iaxc_lock);
}


EXPORT void iaxc_set_formats(int preferred, int allowed)
{
	audio_format_capability = allowed;
	audio_format_preferred = preferred;
}

EXPORT void iaxc_set_min_outgoing_framesize(int samples)
{
	minimum_outgoing_framesize = samples;
}

EXPORT void iaxc_set_callerid(const char * name, const char * number)
{
	int i;

	for ( i = 0; i < max_calls; i++ )
	{
		strncpy(calls[i].callerid_name,   name,   IAXC_EVENT_BUFSIZ);
		strncpy(calls[i].callerid_number, number, IAXC_EVENT_BUFSIZ);
	}
}

EXPORT void iaxc_note_activity(int callNo)
{
	if ( callNo < 0 )
		return;
	calls[callNo].last_activity = iax_tvnow();
}

static void iaxc_refresh_registrations()
{
	struct iaxc_registration *cur;
	struct timeval now;

	now = iax_tvnow();

	for ( cur = registrations; cur != NULL; cur = cur->next )
	{
		// If there is less than three seconds before the registration is about
		// to expire, renew it.
		if ( iaxci_usecdiff(&now, &cur->last) > (cur->refresh - 3) * 1000 *1000 )
		{
			if ( cur->session != NULL )
			{
				iax_destroy( cur->session );
			}
			cur->session = iax_session_new();
			if ( !cur->session )
			{
				iaxci_usermsg(IAXC_ERROR, "Can't make new registration session");
				return;
			}
			iax_register(cur->session, cur->host, cur->user, cur->pass, cur->refresh);
			cur->last = now;
		}
	}
}

#define LOOP_SLEEP 5 // In ms
static THREADFUNCDECL(main_proc_thread_func)
{
	static int refresh_registration_count = 0;

	THREADFUNCRET(ret);

	/* Increase Priority */
	iaxci_prioboostbegin();

	while ( !main_proc_thread_flag )
	{
		get_iaxc_lock();

		service_network();
		//service_audio();

		// Check registration refresh once a second
		if ( refresh_registration_count++ > 1000/LOOP_SLEEP )
		{
			iaxc_refresh_registrations();
			refresh_registration_count = 0;
		}

		put_iaxc_lock();

		iaxc_millisleep(LOOP_SLEEP);
	}

	/* Decrease priority */
	iaxci_prioboostend();

	main_proc_thread_flag = -1;

	return ret;
}

EXPORT int iaxc_start_processing_thread()
{
	main_proc_thread_flag = 0;

	if ( THREADCREATE(main_proc_thread_func, NULL, main_proc_thread,
				main_proc_thread_id) == THREADCREATE_ERROR)
		return -1;

	return 0;
}

EXPORT int iaxc_stop_processing_thread()
{
	if ( main_proc_thread_flag >= 0 )
	{
		main_proc_thread_flag = 1;
		THREADJOIN(main_proc_thread);
	}

	return 0;
}

static int service_audio(void)
{
	/* TODO: maybe we shouldn't allocate 8kB on the stack here. */

	return 0;
}

/* handle IAX text events */
static void handle_text_event(struct iax_event *e, int callNo)
{
	iaxc_event ev;
	int        len;

	if ( callNo < 0 )
		return;

	memset(&ev, 0, sizeof(iaxc_event));
	ev.type = IAXC_EVENT_TEXT;
	ev.ev.text.type = IAXC_TEXT_TYPE_IAX;
	ev.ev.text.callNo = callNo;

	len = e->datalen <= IAXC_EVENT_BUFSIZ - 1 ? e->datalen : IAXC_EVENT_BUFSIZ - 1;
	strncpy(ev.ev.text.message, (char *) e->data, len);
	iaxci_post_event(ev);
}

/* handle IAX URL events */
void handle_url_event( struct iax_event *e, int callNo )
{
	iaxc_event ev;

	if ( callNo < 0 )
		return;

	ev.ev.url.callNo = callNo;
	ev.type = IAXC_EVENT_URL;
	strcpy( ev.ev.url.url, "" );

	switch ( e->subclass )
	{
		case AST_HTML_URL:
			ev.ev.url.type = IAXC_URL_URL;
			if ( e->datalen )
			{
				if ( e->datalen > IAXC_EVENT_BUFSIZ )
				{
					fprintf( stderr, "ERROR: URL too long %d > %d\n",
							e->datalen, IAXC_EVENT_BUFSIZ );
				} else
				{
					strncpy( ev.ev.url.url, (char *) e->data, e->datalen );
				}
			}
			/* fprintf( stderr, "URL:%s\n", ev.ev.url.url ); */
			break;
		case AST_HTML_LINKURL:
			ev.ev.url.type = IAXC_URL_LINKURL;
			/* fprintf( stderr, "LINKURL event\n" ); */
			break;
		case AST_HTML_LDCOMPLETE:
			ev.ev.url.type = IAXC_URL_LDCOMPLETE;
			/* fprintf( stderr, "LDCOMPLETE event\n" ); */
			break;
		case AST_HTML_UNLINK:
			ev.ev.url.type = IAXC_URL_UNLINK;
			/* fprintf( stderr, "UNLINK event\n" ); */
			break;
		case AST_HTML_LINKREJECT:
			ev.ev.url.type = IAXC_URL_LINKREJECT;
			/* fprintf( stderr, "LINKREJECT event\n" ); */
			break;
		default:
			fprintf( stderr, "Unknown URL event %d\n", e->subclass );
			break;
	}
	iaxci_post_event( ev );
}

/* DANGER: bad things can happen if iaxc_netstat != iax_netstat.. */
EXPORT int iaxc_get_netstats(int call, int *rtt, struct iaxc_netstat *local,
		struct iaxc_netstat *remote)
{
	return iax_get_netstats(calls[call].session, rtt,
			(struct iax_netstat *)local,
			(struct iax_netstat *)remote);
}

/* handle IAX text events */
static void generate_netstat_event(int callNo)
{
	iaxc_event ev;

	if ( callNo < 0 )
		return;

	ev.type = IAXC_EVENT_NETSTAT;
	ev.ev.netstats.callNo = callNo;

	/* only post the event if the session is valid, etc */
	if ( !iaxc_get_netstats(callNo, &ev.ev.netstats.rtt,
				&ev.ev.netstats.local, &ev.ev.netstats.remote))
		iaxci_post_event(ev);
}
#define RTP_HEADER_LEN	12
#define RTP_HEADER_VER	2
uint32_t wtk_audio_ssrc = 10000000;
uint32_t wtk_video_ssrc = 10000001;

static __inline void generate_rtp_header( struct iaxc_call *call, struct iax_event *e, unsigned char* buf , int is_video)
{
	 // sampling duration, in milisecond
	int ts, seqno;
	
	buf[0] = (RTP_HEADER_VER<<6) & 0xFF;
	
	if(is_video)
	{
		int rtp_sample = 20;
		ts = ( e->ts / rtp_sample ) * rtp_sample;
		seqno = (int)call->vrtp_seqno + ( ts - (int)call->vrtp_ts ) / rtp_sample;
		if(seqno < 0)	 
			seqno = MAX_SHORT + seqno;
		else if(seqno > MAX_SHORT)	
			seqno -= MAX_SHORT;
		call->vrtp_seqno = (unsigned short)seqno + 1;	
		call->vrtp_ts = ts + rtp_sample;
		buf[1] = kWtkPayloadTypeVP8;
		*(unsigned short *)&buf[2] = htons( call->vrtp_seqno );
		*(unsigned long *)&buf[4] = htonl( ts * 48 );
		*((unsigned long *)&buf[8]) = htonl(wtk_video_ssrc);

		/*iaxci_usermsg(IAXC_STATUS, "WatterTek Lib -> IAX-TS=%d; ts = %u, RTP-SeqNo=%d; RTP-TS=%u;  RTP->SSRC=%u",
				  e->ts,  // millisecond
				  ts,
				  ntohs(*((unsigned short *)&buf[2])),
				  ntohl(*((unsigned long *)&buf[4])),
                  ntohl(*((unsigned long *)&buf[8])));*/
	}
	else
	{
		int rtp_sample = 20;
		ts = ( e->ts / rtp_sample ) * rtp_sample;
		seqno = (int)call->rtp_seqno + ( ts - (int)call->rtp_ts ) / rtp_sample;
		if(seqno < 0)	 
			seqno = MAX_SHORT + seqno;
		else if(seqno > MAX_SHORT)	
			seqno -= MAX_SHORT;
		call->rtp_seqno = (unsigned short)seqno + 1;	
		call->rtp_ts = ts + rtp_sample;
		buf[1] = kWtkPayloadTypeOpus;
		*(unsigned short *)&buf[2] = htons( call->rtp_seqno );
		*(unsigned long *)&buf[4] = htonl( ts * 48 );
		*((unsigned long *)&buf[8]) = htonl(wtk_audio_ssrc);
	}
}

static int wtkcall_recv_audio_event(struct iaxc_call *call, struct iax_event *e, int offset)
{
	unsigned char *outbuf;
	int outlen = 0;
	unsigned char buf[512];
	
	unsigned char *raw;
	int  rawlen;
	int  retlen = 0;  // return length

	rawlen = e->datalen - offset;
	if(rawlen <= 0) 
		return -1;

	raw = e->data + offset;
    retlen = rawlen;
	
	if(call->mstate & IAXC_MEDIA_STATE_NORTP)
	{
		memset(buf,0,sizeof(buf)/sizeof(buf[0]));
		generate_rtp_header( call, e, buf, 0 );

		outbuf = buf + RTP_HEADER_LEN;
		memcpy(outbuf, raw, rawlen);

		outbuf = buf; 
		outlen = rawlen + RTP_HEADER_LEN;
	}
	else
    {
		outbuf = raw;
		outlen = rawlen;
	}

	libwtk_decode_audio(outbuf, outlen);
	
	return retlen;
}

static int wtkcall_recv_video_event(struct iaxc_call *call, struct iax_event *e, int offset)
{
	unsigned char *outbuf;
	int outlen = 0;
	unsigned char buf[512*4];
	
	unsigned char *raw;
	int  rawlen;
	int  retlen = 0;  // return length

	rawlen = e->datalen - offset;
	if(rawlen <= 0) 
		return -1;

	raw = e->data + offset;
    retlen = rawlen;

	if(call->mstate & IAXC_MEDIA_STATE_NORTP)
	{
		memset(buf,0,sizeof(buf)/sizeof(buf[0]));
		generate_rtp_header( call, e, buf, 1 );
		//iaxci_usermsg(IAXC_STATUS, "wtkcall_recv_video_event outbuf[16] = %x",raw[3]);

		outbuf = buf + RTP_HEADER_LEN;
		memcpy(outbuf, raw, rawlen);
		
		outbuf = buf; 
		outlen = rawlen + RTP_HEADER_LEN;
	}
	else
    {
		outbuf = raw;
		outlen = rawlen;
	}

	libwtk_decode_video(outbuf, outlen);
	
	return retlen;
}

static void handle_audio_event(struct iax_event *e, int callNo)
{
	int cur=0;
	int ofs=0;
	
	if( callNo <0 ) 
		return; /* drop audio for non-existed call? */
	
	if( callNo != selected_call )
	{	
		/*if( !((calls[selected_call].mstate & IAXC_MEDIA_STATE_MIXED) && 
			 (calls[callNo].mstate & IAXC_MEDIA_STATE_MIXED)) )*/
		return; 
	}

	for( ofs = 0; ofs < e->datalen; ofs += cur )
	{
		if( ( cur = wtkcall_recv_audio_event( &calls[callNo], e, ofs ) ) < 0 ) 
		{
			iaxci_usermsg(IAXC_STATUS, "Bad or incomplete voice packet.  Unable to decode. dropping\n");
			break;
		}
	}
}


static void handle_video_event(struct iax_event *e, int callNo)
{
	int cur=0;
	int ofs=0;

	if ( callNo < 0 )
		return;

	if ( callNo != selected_call )
	{
		return;
	}

	for( ofs = 0; ofs < e->datalen; ofs += cur )
	{
		if( ( cur = wtkcall_recv_video_event( &calls[callNo], e, ofs ) ) < 0 ) 
		{
			iaxci_usermsg(IAXC_STATUS, "Bad or incomplete voice packet.  Unable to decode. dropping\n");
			break;
		}
	}
}

static void iaxc_handle_network_event(struct iax_event *e, int callNo)
{
	if ( callNo < 0 )
		return;

	iaxc_note_activity(callNo);

	switch ( e->etype )
	{
	case IAX_EVENT_NULL:
		break;
	case IAX_EVENT_HANGUP:
		iaxci_usermsg(IAXC_STATUS, "Call disconnected by remote");
		calls[callNo].state = IAXC_CALL_STATE_FREE;
		calls[callNo].sm.reason = (int)e->ies.causecode;
		iaxc_clear_call(callNo);
		break;
	case IAX_EVENT_REJECT:
		iaxci_usermsg(IAXC_STATUS, "Call rejected by remote (int)e->ies.causecode = %d",(int)e->ies.causecode);
		calls[callNo].sm.reason = (int)e->ies.causecode;
		iaxc_clear_call(callNo);
		break;
	case IAX_EVENT_ACCEPT:
		calls[callNo].state |= IAXC_CALL_STATE_ACTIVE;
		uint64_t format1,format2;
		format1 = e->ies.format & IAXC_AUDIO_FORMAT_MASK;
		format2 = e->ies.format & IAXC_AUDIO_FORMAT2_MASK;
		calls[callNo].format = format1 | format2;
		calls[callNo].vformat = e->ies.format & IAXC_VIDEO_FORMAT_MASK;

		if ( !(e->ies.format & IAXC_VIDEO_FORMAT_MASK) )
		{
			iaxci_usermsg(IAXC_NOTICE,
					"Failed video codec negotiation.");
		}
		iaxci_usermsg(IAXC_STATUS,"Call %d accepted", callNo);
		break;
	case IAX_EVENT_ANSWER:
		calls[callNo].state &= ~IAXC_CALL_STATE_RINGING;
		calls[callNo].state |= IAXC_CALL_STATE_COMPLETE;
		iaxci_do_state_callback(callNo);
		iaxci_usermsg(IAXC_STATUS,"Call %d answered", callNo);
		//iaxc_answer_call(callNo);
		// notify the user?
		break;
	case IAX_EVENT_BUSY:
		calls[callNo].state &= ~IAXC_CALL_STATE_RINGING;
		calls[callNo].state |= IAXC_CALL_STATE_BUSY;
		iaxci_do_state_callback(callNo);
		iaxci_usermsg(IAXC_STATUS, "Call %d busy", callNo);
		iax_hangup(e->session, "Remote Busy");
		iaxc_clear_call(callNo);
		break;
	case IAX_EVENT_VOICE:
		handle_audio_event(e, callNo);
		if ( (calls[callNo].state & IAXC_CALL_STATE_OUTGOING) &&
		     (calls[callNo].state & IAXC_CALL_STATE_RINGING) )
		{
			calls[callNo].state &= ~IAXC_CALL_STATE_RINGING;
			calls[callNo].state |= IAXC_CALL_STATE_COMPLETE;
			iaxci_do_state_callback(callNo);
			iaxci_usermsg(IAXC_STATUS,"Call %d progress",
				     callNo);
		}
		break;
	case IAX_EVENT_VIDEO:
		handle_video_event(e, callNo);
		break;
	case IAX_EVENT_TEXT:
		handle_text_event(e, callNo);
		break;
	case IAX_EVENT_RINGA:
		if(!(calls[callNo].state & IAXC_CALL_STATE_COMPLETE)) {
            calls[callNo].state |= IAXC_CALL_STATE_RINGING;
            iaxci_do_state_callback(callNo);
            iaxci_usermsg(IAXC_STATUS,"Call %d ringing", callNo);
        }
		break;
	case IAX_EVENT_PONG:
		generate_netstat_event(callNo);
		break;
	case IAX_EVENT_URL:
		handle_url_event(e, callNo);
		break;
	case IAX_EVENT_CNG:
		break;
	case IAX_EVENT_TIMEOUT:
		iax_hangup(e->session, "Call timed out");
		iaxci_usermsg(IAXC_STATUS, "Call %d timed out.", callNo);
		calls[callNo].sm.hangup |= HANGUP_TIMEOUT;
		iaxc_clear_call(callNo);
		break;
	case IAX_EVENT_TRANSFER_RS:
		calls[callNo].state &= ~IAXC_CALL_STATE_TRANSFER_NAT;
		calls[callNo].state &= ~IAXC_CALL_STATE_TRANSFER_P2P;
		calls[callNo].state |= IAXC_CALL_STATE_TRANSFER_RS;
		iaxci_do_state_callback(callNo);
		iaxci_usermsg(IAXC_STATUS,"Call %d Relay Server transfer released", callNo);
		break;
	case IAX_EVENT_TRANSFER_NAT:
		calls[callNo].state &= ~IAXC_CALL_STATE_TRANSFER_RS;
		calls[callNo].state &= ~IAXC_CALL_STATE_TRANSFER_P2P;
		calls[callNo].state |= IAXC_CALL_STATE_TRANSFER_NAT;
		iaxci_do_state_callback(callNo);
		iaxci_usermsg(IAXC_STATUS,"Call %d NAT transfer released", callNo);
		break;
	case IAX_EVENT_TRANSFER_P2P:
		calls[callNo].state &= ~IAXC_CALL_STATE_TRANSFER_RS;
		calls[callNo].state &= ~IAXC_CALL_STATE_TRANSFER_NAT;
		calls[callNo].state |= IAXC_CALL_STATE_TRANSFER_P2P;
		iaxci_do_state_callback(callNo);
		iaxci_usermsg(IAXC_STATUS,"Call %d P2P transfer released", callNo);
		break;
	case IAX_EVENT_DTMF:
		iaxci_do_dtmf_callback(callNo,e->subclass);
		iaxci_usermsg(IAXC_STATUS, "DTMF digit %c received", e->subclass);
        	break;
	default:
		iaxci_usermsg(IAXC_STATUS, "Unknown event: %d for call %d", e->etype, callNo);
		break;
	}
}

EXPORT int iaxc_unregister( int id )
{
	int count = 0;
	get_iaxc_lock();
	count = iaxc_remove_registration_by_id(id);
	put_iaxc_lock();
	return count;
}

EXPORT int iaxc_register(const char * user, const char * pass, const char * host, int refresh)
{
	return iaxc_register_ex(user, pass, host, refresh);
}

EXPORT int iaxc_register_ex(const char * user, const char * pass, const char * host, int refresh)
{
	struct iaxc_registration *newreg;

	newreg = (struct iaxc_registration *)malloc(sizeof (struct iaxc_registration));
	if ( !newreg )
	{
		iaxci_usermsg(IAXC_ERROR, "Can't make new registration");
		return -1;
	}

	get_iaxc_lock();
	newreg->session = iax_session_new();
	if ( !newreg->session )
	{
		iaxci_usermsg(IAXC_ERROR, "Can't make new registration session");
		put_iaxc_lock();
		return -1;
	}

	newreg->last = iax_tvnow();
	newreg->refresh = refresh;  

	strncpy(newreg->host, host, 256);
	strncpy(newreg->user, user, 256);
	strncpy(newreg->pass, pass, 256);

	/* send out the initial registration with refresh seconds */
	iax_register(newreg->session, host, user, pass, refresh);

	/* add it to the list; */
	newreg->id = ++next_registration_id;
	newreg->next = registrations;
	registrations = newreg;

	put_iaxc_lock();
	return newreg->id;
}

EXPORT void iaxc_send_busy_on_incoming_call(int callNo)
{
	if ( callNo < 0 )
		return;

	iax_busy(calls[callNo].session);
}

EXPORT void iaxc_answer_call(int callNo)
{
	if ( callNo < 0 )
		return;

	calls[callNo].state |= IAXC_CALL_STATE_COMPLETE;
	calls[callNo].state &= ~IAXC_CALL_STATE_RINGING;
	iax_answer(calls[callNo].session);
	iaxci_do_state_callback(callNo);
}

EXPORT void iaxc_blind_transfer_call(int callNo, const char * dest_extension)
{
	if ( callNo < 0 || !(calls[callNo].state & IAXC_CALL_STATE_ACTIVE) )
		return;

	iax_transfer(calls[callNo].session, dest_extension);
}

EXPORT void iaxc_setup_call_transfer(int sourceCallNo, int targetCallNo)
{
	if ( sourceCallNo < 0 || targetCallNo < 0 ||
			!(calls[sourceCallNo].state & IAXC_CALL_STATE_ACTIVE) ||
			!(calls[targetCallNo].state & IAXC_CALL_STATE_ACTIVE) )
		return;

	iax_setup_transfer(calls[sourceCallNo].session, calls[targetCallNo].session);
}

EXPORT void iaxc_dump_one_call(int callNo)
{
	if ( callNo < 0 )
		return;
	if ( calls[callNo].state == IAXC_CALL_STATE_FREE )
		return;

	iax_hangup(calls[callNo].session,"Dumped Call");
	iaxci_usermsg(IAXC_ERROR, "Hanging up call %d", callNo);
	iaxc_clear_call(callNo);
}

EXPORT void iaxc_dump_all_calls(void)
{
	int callNo;
	get_iaxc_lock();
	for ( callNo = 0; callNo < max_calls; callNo++ )
		iaxc_dump_one_call(callNo);
	put_iaxc_lock();
}


EXPORT void iaxc_dump_call_number( int callNo )
{
	if ( ( callNo >= 0 ) && ( callNo < max_calls ) )
	{
		get_iaxc_lock();
		iaxc_dump_one_call(callNo);
		put_iaxc_lock();
	}
}

EXPORT void iaxc_dump_call(void)
{
	if ( selected_call >= 0 )
	{
		get_iaxc_lock();
		iaxc_dump_one_call(selected_call);
		put_iaxc_lock();
	}
}

EXPORT void iaxc_reject_call(void)
{
	if ( selected_call >= 0 )
	{
		iaxc_reject_call_number(selected_call);
	}
}

EXPORT void iaxc_reject_call_number( int callNo )
{
	if ( ( callNo >= 0 ) && ( callNo < max_calls ) )
	{
		get_iaxc_lock();
		iax_reject(calls[callNo].session, "Call rejected manually.");
		iaxc_clear_call(callNo);
		put_iaxc_lock();
	}
}

EXPORT void iaxc_send_dtmf(char digit)
{
	if ( selected_call >= 0 )
	{
		get_iaxc_lock();
		if ( calls[selected_call].state & IAXC_CALL_STATE_ACTIVE )
			iax_send_dtmf(calls[selected_call].session,digit);
		put_iaxc_lock();
	}
}

EXPORT void iaxc_send_text(const char * text)
{
	if ( selected_call >= 0 && (calls != NULL))
	{
		get_iaxc_lock();
		if ( calls[selected_call].state & IAXC_CALL_STATE_ACTIVE )
			iax_send_text(calls[selected_call].session, text);
		put_iaxc_lock();
	}
}

EXPORT void iaxc_send_text_call(int callNo, const char * text)
{
	if ( callNo < 0 || !(calls[callNo].state & IAXC_CALL_STATE_ACTIVE) )
		return;

	get_iaxc_lock();
	if ( calls[callNo].state & IAXC_CALL_STATE_ACTIVE )
		iax_send_text(calls[callNo].session, text);
	put_iaxc_lock();
}

EXPORT void iaxc_send_url(const char * url, int link)
{
	if ( selected_call >= 0 )
	{
		get_iaxc_lock();
		if ( calls[selected_call].state & IAXC_CALL_STATE_ACTIVE )
			iax_send_url(calls[selected_call].session, url, link);
		put_iaxc_lock();
	}
}

static int iaxc_find_call_by_session(struct iax_session *session)
{
	int i;
	for ( i = 0; i < max_calls; i++ )
		if ( calls[i].session == session )
			return i;
	return -1;
}

static struct iaxc_registration *iaxc_find_registration_by_session(
		struct iax_session *session)
{
	struct iaxc_registration *reg;
	for (reg = registrations; reg != NULL; reg = reg->next)
		if ( reg->session == session )
			break;
	return reg;
}

static void iaxc_handle_regreply(struct iax_event *e, struct iaxc_registration *reg)
{
	iaxci_do_registration_callback(reg->id, e->etype, e->ies.msgcount);

	// XXX I think the session is no longer valid.. at least, that's
	// what miniphone does, and re-using the session doesn't seem to
	// work!
	iax_destroy(reg->session);
	reg->session = NULL;

	if ( e->etype == IAX_EVENT_REGREJ )
	{
		// we were rejected, so end the registration
		iaxc_remove_registration_by_id(reg->id);
	}
}

/* this is what asterisk does */
static int iaxc_choose_codec(uint64_t formats)
{
	int i;
	static uint64_t codecs[] =
	{
		IAXC_FORMAT_ULAW,
		IAXC_FORMAT_ALAW,
		IAXC_FORMAT_SLINEAR,
		IAXC_FORMAT_G726,
		IAXC_FORMAT_ADPCM,
		IAXC_FORMAT_GSM,
		IAXC_FORMAT_ILBC,
		IAXC_FORMAT_SPEEX,
		IAXC_FORMAT_LPC10,
		IAXC_FORMAT_G729A,
		IAXC_FORMAT_G723_1,
		IAXC_FORMAT_OPUS,

		/* To negotiate video codec */
		IAXC_FORMAT_JPEG,
		IAXC_FORMAT_PNG,
		IAXC_FORMAT_H261,
		IAXC_FORMAT_H263,
		IAXC_FORMAT_H263_PLUS,
		IAXC_FORMAT_MPEG4,
		IAXC_FORMAT_H264,
		IAXC_FORMAT_VP8,
		IAXC_FORMAT_THEORA,
	};
	for ( i = 0; i < (int)(sizeof(codecs) / sizeof(int)); i++ )
		if ( codecs[i] & formats )
			return codecs[i];
	return 0;
}

static void iaxc_handle_connect(struct iax_event * e)
{
	uint64_t video_format = 0;
	uint64_t format = 0;
	int callno;

	callno = iaxc_first_free_call();

	if ( callno < 0 )
	{
		iaxci_usermsg(IAXC_STATUS,
				"%i \n Incoming Call, but no appearances",
				callno);
		// XXX Reject this call!, or just ignore?
		//iax_reject(e->session, "Too many calls, we're busy!");
		iax_accept(e->session, audio_format_preferred & e->ies.capability);
		iax_busy(e->session);
		return;
	}

	/* negotiate codec */
	/* first, try _their_ preferred format */
	format = audio_format_capability & e->ies.format;
	if ( !format )
	{
		/* then, try our preferred format */
		format = audio_format_preferred & e->ies.capability;
	}

	if ( !format )
	{
		/* finally, see if we have one in common */
		format = audio_format_capability & e->ies.capability;

		/* now choose amongst these, if we got one */
		if ( format )
		{
			format = iaxc_choose_codec(format);
		}
	}

	if ( !format )
	{
		iax_reject(e->session, "Could not negotiate common codec");
		return;
	}

	/* first, see if they even want video */
	video_format = (e->ies.format & IAXC_VIDEO_FORMAT_MASK);

	if ( video_format )
	{
		/* next, try _their_ preferred format */
		video_format &= video_format_capability;

		if ( !video_format )
		{
			/* then, try our preferred format */
			video_format = video_format_preferred &
				(e->ies.capability & IAXC_VIDEO_FORMAT_MASK);
		}

		if ( !video_format )
		{
			/* finally, see if we have one in common */
			video_format = video_format_capability &
				(e->ies.capability & IAXC_VIDEO_FORMAT_MASK);

			/* now choose amongst these, if we got one */
			if ( video_format )
			{
				video_format = iaxc_choose_codec(video_format);
			}
		}

		/* All video negotiations failed, then warn */
		if ( !video_format )
		{
			iaxci_usermsg(IAXC_NOTICE,
					"Notice: could not negotiate common video codec");
			iaxci_usermsg(IAXC_NOTICE,
					"Notice: switching to audio-only call");
		}
	}

	calls[callno].vformat = video_format;
	calls[callno].format = format;
	
	if ( e->ies.called_number )
		strncpy(calls[callno].local, e->ies.called_number,
				IAXC_EVENT_BUFSIZ);
	else
		strncpy(calls[callno].local, "unknown",
				IAXC_EVENT_BUFSIZ);

	if ( e->ies.called_context )
		strncpy(calls[callno].local_context, e->ies.called_context,
				IAXC_EVENT_BUFSIZ);
	else
		strncpy(calls[callno].local_context, "",
				IAXC_EVENT_BUFSIZ);

	if ( e->ies.calling_number )
		strncpy(calls[callno].remote, e->ies.calling_number,
				IAXC_EVENT_BUFSIZ);
	else
		strncpy(calls[callno].remote, "unknown",
				IAXC_EVENT_BUFSIZ);

	if ( e->ies.calling_name )
		strncpy(calls[callno].remote_name, e->ies.calling_name,
				IAXC_EVENT_BUFSIZ);
	else
		strncpy(calls[callno].remote_name, "unknown",
				IAXC_EVENT_BUFSIZ);

	if( strncmp( calls[callno].remote_name, "[msg@", 5 ) ==0 )
	{
		iax_reject(calls[callno].session, "Call rejected manually.");
		iaxci_usermsg(IAXC_NOTICE, "iaxc_handle_connect : Call %d rejected due to bad name %s.", callno, calls[callno].remote_name);
		iaxc_clear_call(callno);
		return;
	}

	iaxc_note_activity(callno);
	iaxci_usermsg(IAXC_STATUS, "Call from (%s)", calls[callno].remote);

	calls[callno].session = e->session;
	calls[callno].state = IAXC_CALL_STATE_ACTIVE|IAXC_CALL_STATE_RINGING;
	//iax_set_callerid(calls[callno].session, calls[callno].callerid_number);
	//calls[callno].sm.id = get_new_callid();

	iax_accept(calls[callno].session, format | video_format);
	iax_ring_announce(calls[callno].session);

	iaxci_do_state_callback(callno);

	iaxci_usermsg(IAXC_STATUS, "Incoming call on line %d", callno);
}

static void service_network(void)
{
	struct iax_event *e = 0;
	int callNo;
	struct iaxc_registration *reg;

	while ( (e = iax_get_event(0)) )
	{
#ifdef WIN32
		iaxc_millisleep(0); 
#endif
		// first, see if this is an event for one of our calls.
		callNo = iaxc_find_call_by_session(e->session);
		if ( e->etype == IAX_EVENT_NULL )
		{
			// Should we do something here?
			// Right now we do nothing, just go with the flow
			// and let the event be deallocated.
		} else if ( callNo >= 0 )
		{
			iaxc_handle_network_event(e, callNo);
		} else if ( (reg = iaxc_find_registration_by_session(e->session)) != NULL )
		{
			iaxc_handle_regreply(e,reg);
		} else if ( e->etype == IAX_EVENT_REGACK || e->etype == IAX_EVENT_REGREJ )
		{
			iaxci_usermsg(IAXC_ERROR, "Unexpected registration reply");
		} else if ( e->etype == IAX_EVENT_REGREQ )
		{
			iaxci_usermsg(IAXC_ERROR,
					"Registration requested by someone, but we don't understand!");
		} else if ( e->etype == IAX_EVENT_CONNECT )
		{
			iaxc_handle_connect(e);
		} else if ( e->etype == IAX_EVENT_TIMEOUT )
		{
			iaxci_usermsg(IAXC_STATUS,
					"Timeout for a non-existant session. Dropping",
					e->etype);
		} else
		{
			iaxci_usermsg(IAXC_STATUS,
					"Event (type %d) for a non-existant session. Dropping",
					e->etype);
		}
		iax_event_free(e);
	}
}
EXPORT int iaxc_quelch(int callNo, int MOH)
{
	struct iax_session *session = calls[callNo].session;
	if ( !session )
		return -1;

	return iax_quelch_moh(session, MOH);
}

EXPORT int iaxc_unquelch(int call)
{
	return iax_unquelch(calls[call].session);
}

EXPORT char* iaxc_version(char * ver)
{
#ifndef LIBVER
#define LIBVER "WTK-IAX2-Ver.01"
#endif
	strncpy(ver, LIBVER, 32);
	return ver;
}

EXPORT int iaxc_push_audio(void *data, unsigned int size, unsigned int samples)
{
	struct iaxc_call *call;
	unsigned char *outbuf = NULL;
	int  outlen;

	if ( selected_call < 0 )
		return -1;

	call = &calls[selected_call];
	
	if(call->mstate & IAXC_MEDIA_STATE_NORTP) {
		outbuf = data + RTP_HEADER_LEN;
		outlen = size - RTP_HEADER_LEN;
	}
	else {
		outbuf = data;
		outlen = size;
	}

	if ( iax_send_voice(call->session, call->format, outbuf, outlen, samples) == -1 )
	{
		//iaxci_usermsg(IAXC_STATUS, "iaxc_push_audio: failed to send audio frame of size %d on call %d\n", size, selected_call);
		return -1;
	}

	return 0;
}

EXPORT int iaxc_push_video(void *data, unsigned int size, int fullframe)
{
	struct iaxc_call *call;
	unsigned char *outbuf = NULL;
	int  outlen;

	if ( selected_call < 0 )
		return -1;

	call = &calls[selected_call];
	
	if((call->mstate & IAXC_MEDIA_STATE_NORTP)&&size > RTP_HEADER_LEN)
	{
		//outbuf = data + RTP_HEADER_LEN;
		//outlen = size - RTP_HEADER_LEN;
		outbuf = data;
		outlen = size;
	}
	else 
	{
		outbuf = data;
		outlen = size;
	}

	if ( iax_send_video(call->session, call->vformat, outbuf, outlen, fullframe) == -1 )
	{
		//iaxci_usermsg(IAXC_STATUS, "iaxc_push_video: failed to send video frame of size %d on call %d\n", size, selected_call);
		return -1;
	}

	return 0;
}


wtkcall_iax_event_callback_t wtkcall_send_iax_event = NULL;
EXPORT void wtkcall_set_iax_event_callback( wtkcall_iax_event_callback_t func )  
{
	wtkcall_send_iax_event = func;
}

EXPORT void  wtkcall_perform_registration_callback(int id, int reply)
{
	RegistrationInfo info;

	switch(reply) 
	{
		case IAXC_REGISTRATION_REPLY_ACK:  // "registration accepted"
			info.reply = REGISTRATION_ACCEPT;
			break;
		case IAXC_REGISTRATION_REPLY_REJ:  // "registration rejected"
			info.reply = REGISTRATION_REJECT;  
			break;
		case IAXC_REGISTRATION_REPLY_TIMEOUT:  // "registration timeout"
			info.reply = REGISTRATION_TIMEOUT;
			break;
		default:
			info.reply = 0;
			break;
	}
	info.id = id;

	if(wtkcall_send_iax_event)
		wtkcall_send_iax_event(EVENT_REGISTRATION, (void*)&info);
}
EXPORT void  wtkcall_perform_state_callback(struct wtkState* call, int callNo, int state, char* name, char* number)
{
	int new_state = -1;
	int new_activity = -1;

	new_state = state;

	if(new_state & IAXC_CALL_STATE_ACTIVE)
	{
		if(new_state & IAXC_CALL_STATE_RINGING)
		{
			call->tm_start = iax_tvnow();
			if(new_state & IAXC_CALL_STATE_OUTGOING)
			{
				new_activity = CALL_OUTGOING;
				call->type = CALL_TYPE_OUTGOING;
			}
			else
			{
				new_activity = CALL_RINGIN;
				call->type = CALL_TYPE_INCOMING;
			}
		}
		else if((new_state & IAXC_CALL_STATE_COMPLETE)&&(new_state & IAXC_CALL_STATE_SELECTED))
		{
			call->tm_answer = iax_tvnow();
			if(new_state & IAXC_CALL_STATE_TRANSFER_RS)
			{
				new_activity = CALL_TRANSFERED_RS;
			}
			else if(new_state & IAXC_CALL_STATE_TRANSFER_NAT)
			{
				new_activity = CALL_TRANSFERED_NAT;
			}
			else if(new_state & IAXC_CALL_STATE_TRANSFER_P2P)
			{
				new_activity = CALL_TRANSFERED_P2P;
			}
			else
			{
				new_activity = CALL_ANSWERED;
			}
		}
	}
	else if(new_state==IAXC_CALL_STATE_FREE)
	{
		/* the call will be free */
		new_activity = CALL_FREE;
		if((call->activity==CALL_ANSWERED)||
			(call->activity==CALL_TRANSFERED_RS)||(call->activity==CALL_TRANSFERED_NAT)||(call->activity==CALL_TRANSFERED_P2P))
		{
			struct timeval end;
			end = iax_tvnow();
			call->duration = (unsigned int)(end.tv_sec - call->tm_answer.tv_sec);
		}
		else if(!(call->hangup & HANGUP_SELF))
		{
			call->type = CALL_TYPE_REJECTED;
		}
	}

	if(new_activity != -1)  
	{
		call->activity = new_activity;
		if(wtkcall_send_iax_event!=NULL)
		{
			CallInfo info;
			memset((CallInfo*)&info, 0, sizeof(CallInfo));
			
			/* current state and informaton */
			info.callNo = callNo;
			strcpy(info.peer_name, name);
			strcpy(info.peer_number, number);
			
			info.activity	= new_activity;
			info.start		= call->tm_start.tv_sec;
			info.duration 	= call->duration;
			info.type		= call->type;
			info.reason   	= call->reason;
			
			if(new_activity==CALL_TRANSFERED_RS)
			{
			}
			
			iaxci_usermsg(IAXC_STATUS, "wtkcall_send_iax_event:peerName=%s,peerNumber=%s,activity=%d,reason=%d(H:%d),duration=%d,type_sdk(jni)=%d",
					info.peer_name,info.peer_number,info.activity,
					info.reason,call->hangup,info.duration,info.type);
			wtkcall_send_iax_event(EVENT_STATE, (void*)&info);

			if(new_activity == CALL_FREE)
				wtkState_clear_state(&calls[callNo].sm);
		}
	}
}

EXPORT void  wtkcall_perform_message_callback(int callNo,char *message)
{
	MessageInfo info;
	memset((MessageInfo*)&info, 0, sizeof(MessageInfo));
			
	info.callNo = callNo;
	strcpy(info.message, message);
	if(wtkcall_send_iax_event)
		wtkcall_send_iax_event(EVENT_MESSAGE, (void*)&info);
}
EXPORT void  wtkcall_perform_text_callback(char* text)
{
	if(wtkcall_send_iax_event)
		wtkcall_send_iax_event(EVENT_LOG, text);
}
EXPORT void  wtkcall_perform_control_callback(char* text)
{
	if(wtkcall_send_iax_event)
		wtkcall_send_iax_event(EVENT_LOG, text);
}

