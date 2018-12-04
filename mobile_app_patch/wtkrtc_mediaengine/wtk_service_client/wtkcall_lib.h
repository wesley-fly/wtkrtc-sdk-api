#ifndef _wtkcall_lib_h
#define _wtkcall_lib_h

#include <stdbool.h>


//Belowe modify should be together with SDK wtk_media_sdk_define.h 
enum _Event_Type
{
	EVENT_REGISTRATION=0,
	EVENT_STATE=1,
	EVENT_MESSAGE=2,
	EVENT_LOG=3,
	EVENT_PTT=4,
    EVENT_VADLIST=5,
	//EVENT_CONTROL=6,
	//EVENT_VIDEO=7,
};
enum _Call_State  {
	CALL_FREE=0,
	CALL_OUTGOING=1,
	CALL_RINGIN=2,
	CALL_RINGBACK=3,
	CALL_ANSWERED=4,
	CALL_TRANSFERED_RS=5,
    CALL_TRANSFERED_NAT=6,
    CALL_TRANSFERED_P2P=7,
};
enum _Call_Type {
	CALL_TYPE_IDEL=0,
	CALL_TYPE_OUTGOING=1,
	CALL_TYPE_INCOMING=2,
	CALL_TYPE_MISSED=3,
	CALL_TYPE_REJECTED=4,
};

// Hold or Resume Command
enum _Hold_Resume {
	RESUME=0,
	HOLD=1,
};

enum _Registration_Result {
	REGISTRATION_ACCEPT=0,
	REGISTRATION_REJECT=1,
	REGISTRATION_TIMEOUT=2,
};
typedef struct _Registration_Info {
	int id;
	int reply;
}RegistrationInfo;
typedef struct _Call_Info {
	int callNo;					// unique identifier of the call
	char peer_name[256];		// peer's name (UTF-8 String)
	char peer_number[256];      // peer's freepp number
	int activity;				// current activity of the call (see _Call_State)
	int reason;            		// reason to finish call, when activity = CALL_FREE - 20170519
	unsigned int start;			// Start Time
	unsigned int duration;      // Call Duration
	unsigned int type;			// type for SDK(jni) level
}CallInfo;
typedef struct _Message_Info {
	int  callNo;
	char message[256];
}MessageInfo;
typedef struct _Ptt_Info
{
    int   callNo;
    int   event;
    char  speaker_id[256];
    short position;
}PttInfo;
typedef struct _VadList_Info
{
    int callNo;
    char vadlist[512];
}VadListInfo;

typedef int (*wtkcall_jni_event_callback_t)( int event_type, void* event_info);
//Extern API
#ifdef __cplusplus
extern "C" {
#endif
#ifdef ANDROID
extern int 		wtkcall_init_AndroidEnv(void* javaVM, void* context);
#endif
extern void 	wtkcall_set_jni_event_callback( wtkcall_jni_event_callback_t func );
extern int 		wtkcall_initialize_iax(void);
extern void 	wtkcall_shutdown_iax(void);
extern int 		wtkcall_get_audio_stats(int *send_bps,int * rec_bps,int * package_lost);
extern int 		wtkcall_get_video_stats(int * send_bps,int * rec_bps,int * prefer_bps);
extern int 		wtkcall_get_call_quality(int *audio_level, int *video_level);
extern void 	wtkcall_mute(bool mute);
extern void 	wtkcall_start_audio(void);
extern void 	wtkcall_stop_audio(void);
extern void 	wtkcall_start_video(int is_front);
extern void 	wtkcall_stop_video(void);
extern void 	wtkcall_start_capturer(void);
extern void 	wtkcall_stop_capturer(void);
extern void 	wtkcall_send_text(const char* text);
extern void 	wtkcall_set_caller_number(const char *number);
extern int 		wtkcall_register(const char* name,const char *number,const char *pass,const char *host,int refresh );
extern void 	wtkcall_unregister( int id );
extern int 		wtkcall_dial(const char* dest,const char* host,const char* user,const char *cmd,const char* ext);
extern void 	wtkcall_answer( int callNo );
extern void 	wtkcall_select(int callNo);
extern int 		wtkcall_control_conference( int callNo,int type);
extern int 		wtkcall_hangup( int callNo);
extern int 		wtkcall_set_hold(int callNo, bool hold);
extern void 	wtkcall_set_render(void* RemoteSurface);
extern void 	wtkcall_switch_camera(int deviceId);
extern void 	wtkcall_set_capture_rotation(int rotation);
extern void 	wtkcall_config_video(int codec, int width, int height, int fps, int maxqp);
extern void 	wtkcall_config_bitrate(int audio_min_bps, int audio_max_bps, int video_min_bps, int video_max_bps);
extern void 	wtkcall_set_conf_render(void* Surface0,void* Surface1,void* Surface2,void* Surface3);
extern void		wtkcall_start_conf_video(int participant_ssrc);
extern void		wtkcall_stop_conf_video(void);

#ifdef __cplusplus
}
#endif

#endif
