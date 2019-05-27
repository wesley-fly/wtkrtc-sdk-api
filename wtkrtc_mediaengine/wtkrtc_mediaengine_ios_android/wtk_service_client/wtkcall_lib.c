#include "iax2-client/iaxclient_lib.h"
#include "iax2-client/iaxclient.h"
#include "iax2-client/iax-client.h"

#include "../wtk_rtc_api/wtk_rtc_api.h"
#include "wtkcall_lib.h"
extern uint32_t wtk_audio_ssrc;
extern uint32_t wtk_video_ssrc;

#define MAX_CALLS	4
#define USE_PTT 	0
#ifdef ANDROID
int wtkcall_init_AndroidEnv(void* javaVM, void* context)
{
	int ret = -1;

	ret = libwtk_init_AndroidVideoEnv(javaVM, context);
	return ret;
}
#endif
static char* map_state(int state)
{
	static char *map[] = { "unknown", "active", "outgoing", "ringing",
		"complete", "selected", "busy", "transfered_rs", "transfered_nat","transfered_p2p",NULL };
	static char states[256]; 	/* buffer to hold ascii states */
	int i, j;
	int next=0;

	states[0] = '\0';
	if( state == 0 )
		return "free";
	for( i=0, j=1; map[i] != NULL; i++, j<<=1 ) {
		if( state & j ) {
			if (next)
				strcat(states, ",");
			strcat(states, map[i]);
			next = 1;
		}
	}
	return states;
}

static int wtkcall_iax_event_callback( iaxc_event e )
{
	switch(e.type) {
		case IAXC_EVENT_REGISTRATION:
			wtkcall_perform_registration_callback(e.ev.reg.id, e.ev.reg.reply);
		break;
		
		case IAXC_EVENT_STATE:
      iaxci_usermsg(IAXC_NOTICE,"wtkcall_iax_event_callback==>S\tcallNo=%d\tstate=[0x%x\t%s]",e.ev.call.callNo,e.ev.call.state,map_state(e.ev.call.state));
			wtkcall_perform_state_callback(&calls[e.ev.call.callNo].sm,e.ev.call.callNo, e.ev.call.state, e.ev.call.remote_name, e.ev.call.remote);	
		break;

		case IAXC_EVENT_TEXT:
			if(e.ev.text.type==IAXC_TEXT_TYPE_IAX) 
			{
				iaxci_usermsg(IAXC_NOTICE,"wtkcall_iax_event_callback==>T\ttext_type=%d\tcallNo=%d\tmessage=%.255s",e.ev.text.type,e.ev.text.callNo,e.ev.text.message);
				// Remote Text Message
				wtkcall_perform_message_callback(e.ev.text.callNo, e.ev.text.message);
			}
			else 
			{
				// Local Text message
				wtkcall_perform_text_callback(e.ev.text.message);
			}
		break; 

		default:
			iaxci_usermsg(IAXC_NOTICE, "wtkcall_iax_event_callback==>WTK uncared state:\t%d\n", e.type );
		break;
	}
	return 1;// = 0 means will print to std also
}
int wtkcall_send_audio_callback(char* data, int len)
{
	int rtp_samples;
	int send_len = 0;
	
	if((selected_call < 0) || (calls[selected_call].session == NULL))
		return -1;
	
	rtp_samples = 40*(48000/1000);
	send_len = iaxc_push_audio(data, len, rtp_samples);
	return send_len;
}
int wtkcall_send_video_callback(char* data, int len)
{
	if((selected_call < 0) || (calls[selected_call].session == NULL))
		return -1;

	int send_len;
	send_len = iaxc_push_video(data, len, 0);
	return send_len;
}

//Extern API for SDK or APP
int wtkcall_initialize_iax(void)
{
	iaxc_set_event_callback(wtkcall_iax_event_callback); 

	// Initialize IAX Library
	if( iaxc_initialize( MAX_CALLS ) ) 
	{
		iaxci_usermsg(IAXC_ERROR, "Cannot initialize iaxclient!");
		return -1;
	}

	if(iaxc_start_processing_thread() == 0)
	{
		libwtk_set_audio_transport(wtkcall_send_audio_callback);
		libwtk_set_video_transport(wtkcall_send_video_callback);
		iaxci_usermsg(IAXC_NOTICE, "IAX2 Client Initialized OK");
		return 0;
	}
	else
	{
		iaxci_usermsg(IAXC_NOTICE, "IAX2 Client Initialized Error");
		return -1;
	}
}

void wtkcall_shutdown_iax(void)
{
	iaxc_stop_processing_thread();
	iaxc_shutdown(); 
}

int wtkcall_get_audio_stats(int *send_bps,int * rec_bps,int * package_lost)
{
	if(libwtk_get_audio_stats(send_bps,rec_bps,package_lost) == 0)
		return 0;
	else
		return -1;
}
int wtkcall_get_video_stats(int * send_bps,int * rec_bps,int * prefer_bps)
{
	if(libwtk_get_video_stats(send_bps,rec_bps,prefer_bps) == 0)
		return 0;
	else
		return -1;
}

int wtkcall_get_call_quality(int *audio_level, int *video_level)
{
	libwtk_get_call_quality(audio_level, video_level);

	return 0;
}

void wtkcall_mute(bool mute)
{
	libwtk_set_mute(mute);
}

void wtkcall_start_audio(void)
{
	//iaxci_usermsg(IAXC_ERROR, "wtkcall_start_audio getpid = %d, pthread_self = %lu\n", getpid(),pthread_self());
	libwtk_create_call();
	libwtk_create_audio_send_stream(wtk_audio_ssrc);
	libwtk_create_audio_receive_stream(wtk_audio_ssrc);
	libwtk_start_audio_stream();
}

void wtkcall_stop_audio(void)
{
	//iaxci_usermsg(IAXC_ERROR, "wtkcall_stop_audio getpid = %d, pthread_self = %lu\n", getpid(),pthread_self());
	libwtk_stop_audio_stream();
	libwtk_destroy_audio_send_stream();
	libwtk_destroy_audio_receive_stream();
	libwtk_destroy_call();
}

void wtkcall_start_video(void)
{
	//iaxci_usermsg(IAXC_ERROR, "wtkcall_start_video getpid = %d, pthread_self = %lu\n", getpid(),pthread_self());
	libwtk_create_video_send_stream(wtk_video_ssrc);
	libwtk_create_video_receive_stream(wtk_video_ssrc);
	libwtk_start_video_stream();
}

void wtkcall_stop_video(void)
{
	//iaxci_usermsg(IAXC_ERROR, "wtkcall_stop_video getpid = %d, pthread_self = %lu\n", getpid(),pthread_self());	
	libwtk_stop_video_stream();
	libwtk_destroy_video_send_stream();
	libwtk_destroy_video_receive_stream();
	libwtk_destory_capture();
}
void wtkcall_start_capturer(void)
{
	libwtk_start_capture();
}

void wtkcall_stop_capturer(void)
{
	libwtk_stop_capture();
}
void wtkcall_send_text(const char* text)
{
	iaxc_send_text(text);
}

void wtkcall_set_caller_number(const char *number)
{
	int i;
	
	if(calls)
	{
		for(i=0; i<max_calls; i++) 
		{
			strncpy(calls[i].callerid_number, number, IAXC_EVENT_BUFSIZ);
		}
	}
}
int wtkcall_register(const char* name,const char *number,const char *pass,const char *host, int refresh )
{
    char temp[256] = {0};
    char *pwd = NULL;
	int i;

	if(calls)
	{
		for(i=0; i<max_calls; i++) 
		{
			strncpy(calls[i].callerid_name, name, IAXC_EVENT_BUFSIZ);
			strncpy(calls[i].callerid_number, number, IAXC_EVENT_BUFSIZ);
		}
	}

	if(pass && strchr(pass, ':'))
	{
		strncpy(temp, pass, sizeof(temp));
		pwd = strtok(temp, ":");
	}
	else {
		pwd = pass;
	}

	return iaxc_register( number, pass, host, refresh );
}
void wtkcall_unregister( int id )
{
	int count = 0;
	get_iaxc_lock();
	count = iaxc_remove_registration_by_id(id);
	put_iaxc_lock();
	return;
}
int wtkcall_dial(const char* dest,const char* host,const char* user,const char *cmd,const char* ext)
{
	int callNo = -1;
	struct iax_session* newsession = NULL;

	char *name, *option, *secret, *pwd;
	char tmp[256] = {0};
    
	get_iaxc_lock();

	// if no call is selected, get a new appearance
	if ( selected_call < 0 )
	{
		callNo = iaxc_first_free_call();
	} 
	else
	{
		// use selected call if not active, otherwise, get a new appearance
		if ( calls[selected_call].state & IAXC_CALL_STATE_ACTIVE )
		{
			callNo = iaxc_first_free_call();
		} else
		{
			callNo = selected_call;
		}
	}

	if ( callNo < 0 )
	{
		iaxci_usermsg(IAXC_STATUS, "No free call appearances");
		put_iaxc_lock();
		return callNo;
	}

	newsession = iax_session_new();
	
	//fprintf(stderr, "WTK-IAXCLIENT: iax_session_new pingtime = %d !!!\n", newsession->callno);
	if ( !newsession )
	{
		iaxci_usermsg(IAXC_ERROR, "Can't make new session");
		put_iaxc_lock();
		return callNo;
	}
	else
	{
		fprintf(stderr, "WTK-IAXCLIENT: iax_session_new is ok !!!\n");
	}
	calls[callNo].session = newsession;
	
	/* When the ACCEPT comes back from the other-end, these formats
	 * are set. Whether the format is set or not determines whether
	 * we are in the Linked state (see the iax2 rfc).
	 * These will have already been cleared by iaxc_clear_call(),
	 * but we reset them anyway just to be pedantic.
	 */
	calls[callNo].format = 0;
	calls[callNo].vformat = 0;
	calls[callNo].rtp_seqno = 0;
	calls[callNo].rtp_ts = 0;
	calls[callNo].vrtp_seqno = 0;
	calls[callNo].vrtp_ts = 0;

	if ( dest )
	{
		sprintf(calls[callNo].remote_name, "%s@%s/%s", user, host, dest);
		strncpy(calls[callNo].remote, dest, IAXC_EVENT_BUFSIZ);
	} 
	else
	{
		sprintf(calls[callNo].remote_name, "%s@%s",user, host);
		strncpy(calls[callNo].remote,      "" , IAXC_EVENT_BUFSIZ);
	}

	if( cmd==NULL || strcmp(cmd, "")==0)
		strncpy(calls[callNo].local, calls[callNo].callerid_name, IAXC_EVENT_BUFSIZ);
	else 
	{
		strncpy(tmp, cmd, sizeof(tmp));
		
		if(cmd[0]=='/')
		{
			// there is no customized user name, use the caller name instead
			strncpy(calls[callNo].local, calls[callNo].callerid_name, IAXC_EVENT_BUFSIZ);
			option = strtok(tmp, "/");
		}
		else 
		{
			name = strtok(tmp, "/");
			if(name!=NULL)
			{
				strncpy(calls[callNo].local, name, IAXC_EVENT_BUFSIZ);
				//iaxci_usermsg(IAXC_NOTICE, "name = %s", name);
				//[wtk-mobile@34924663-1543285652-15]70f42532-601c-4534-85ae-c5fe5cccb763+4f56e20b-b2f1-37f4-8e5b-65c4638af975
			}
			option = strtok(NULL, "/");
		}
		while (option!=NULL) 
		{
			if(strcmp(option, "mixer")==0) {
				calls[callNo].mstate |= IAXC_MEDIA_STATE_MIXED;
			}
			else if(strcmp(option, "nortp")==0) {
				calls[callNo].mstate |= IAXC_MEDIA_STATE_NORTP;
			}
			else if(strcmp(option, "forward")==0) {
				calls[callNo].mstate |= IAXC_MEDIA_STATE_FORWARD;
			}
			// next option
			option = strtok(NULL, "/");
		}
	}

  strncpy(tmp, user, sizeof(tmp));
  if (strchr(tmp, ':')) {
		name = strtok(tmp, ":");
		secret = strtok(NULL, ":");
        
    if(secret && strchr(secret, ':')) {
        strncpy(tmp, secret, sizeof(tmp));
        pwd = strtok(tmp, ":");
    }
    else {
        pwd = secret;
    }
  }

	strncpy(calls[callNo].local_context, "default", IAXC_EVENT_BUFSIZ);

	calls[callNo].state = IAXC_CALL_STATE_OUTGOING |IAXC_CALL_STATE_ACTIVE;
	calls[callNo].last_ping = calls[callNo].last_activity;
	
	/* reset activity and ping "timers" */
	iaxc_note_activity(callNo);
	iaxci_usermsg(IAXC_NOTICE, "Originating an %s call", video_format_preferred ? "audio+video" : "audio only");

	iax_call(calls[callNo].session, calls[callNo].callerid_number,
		calls[callNo].local, user, host, dest,
		audio_format_preferred | video_format_preferred, 
		audio_format_capability | video_format_capability,
        ext);

	iaxc_select_call(callNo);
        
	put_iaxc_lock();
	return callNo;
}
void wtkcall_answer( int callNo )
{
    get_iaxc_lock();

	if ( callNo >= 0 && callNo < max_calls ) {
        iaxc_answer_call( callNo );
    }
	put_iaxc_lock();
}
void wtkcall_select(int callNo)
{
	get_iaxc_lock();
	if( callNo >= 0 && callNo < max_calls) {
        iaxc_select_call(callNo);
    }
	put_iaxc_lock();
}
int wtkcall_control_conference( int callNo,int type)
{
	if( callNo >= 0 && callNo < max_calls )
	{
		if(type == 3)
		{
			iaxc_send_text("KIC {\"fi\":\"ALL\"}");
			//wtkcall_hangup(callNo);
		}
	}

	return 0;
}

int wtkcall_hangup( int callNo)
{
	int ret = -1;
    
	get_iaxc_lock();
	if( callNo >= 0 && callNo < max_calls )
	{
		calls[callNo].sm.hangup |= HANGUP_SELF; //calls[callNo].sm.self = 1;

		if( (calls[callNo].state & IAXC_CALL_STATE_COMPLETE) ||
		  (calls[callNo].state & IAXC_CALL_STATE_OUTGOING) )
		{
		  iaxc_dump_one_call(callNo);  // hangup an outgoing call or answered call
		}
		else
		{
		  iax_reject(calls[callNo].session, "Call rejected manually.");  // reject an incoming call
		  iaxc_clear_call(callNo);
		}
		ret = 0;
	}
	put_iaxc_lock();
	return ret;
}
int wtkcall_set_hold(int callNo, bool hold)
{
	int ret = -1;
	get_iaxc_lock();
	if(callNo >= 0 && callNo < max_calls)
	{
		ret = 0;

		if(!hold)
		{
			iaxc_unquelch(callNo);
		}
		else
		{
			iaxc_quelch(callNo, HOLD);
		}
	}
	put_iaxc_lock();
	return ret;
}

void wtkcall_set_render(void* LocalSurface, void* RemoteSurface)
{
	libwtk_create_local_render(LocalSurface);
	libwtk_create_remote_render(RemoteSurface);
}
void wtkcall_switch_camera(int deviceId)
{
	libwtk_switch_camera(deviceId);
}
void wtkcall_set_capture_rotation(int rotation)
{
	//libwtk_set_capture_rotation(rotation);
	//libwtk_switch_camera(1);
}

void wtkcall_config_video(int codec, int width, int height, int fps, int maxqp)
{
	libwtk_config_video(codec,width,height,fps,maxqp);
}
void wtkcall_config_bitrate(int audio_min_bps, int audio_max_bps, int video_min_bps, int video_max_bps)
{
	libwtk_config_bitrate(audio_min_bps,audio_max_bps,video_min_bps,video_max_bps);
}

void wtkcall_set_conf_render(void* Surface0,void* Surface1,void* Surface2,void* Surface3)
{
	//libwtk_create_capture(1);//1:Front; 0:back
	libwtk_create_conf_render(Surface0,Surface1,Surface2,Surface3);
}
void wtkcall_start_conf_video(int participant_ssrc)
{
	libwtk_create_video_conf_send_stream(participant_ssrc);
	libwtk_create_video_conf_receive_stream(20000000);
	libwtk_start_video_conf_stream();
}
void wtkcall_stop_conf_video(void)
{
	libwtk_stop_capture();
	
	libwtk_stop_video_conf_stream();
	libwtk_destroy_video_conf_stream();
	libwtk_destory_capture();
}
