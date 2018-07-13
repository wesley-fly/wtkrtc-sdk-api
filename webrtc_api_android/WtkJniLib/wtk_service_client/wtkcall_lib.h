#ifndef _wtkcall_lib_h
#define _wtkcall_lib_h

#include <stdbool.h>

#define COMMAND_CODEC           "COD"   // codec negotiation
#define COMMAND_VIDEO           "VID"   // video control
#define COMMAND_VAD             "VAD"   // enbale/disable audio vad
#define COMMAND_PACKET          "PKT"   // increase/descrease packet combo number
#define COMMAND_ENCRYPT         "ENC"   // encryption method negotiation
#define COMMAND_ENCRYPT_STATE   "ENS"   // encryption notification (start/stop)
#define COMMAND_HOLD            "HLD"   // call hold notification
#define COMMAND_RESUME          "RES"   // call resume notification
#define COMMAND_DTMF            "DTM"   // DTMF message
#define COMMAND_CHANGE_PATH     "CHP"   // Change Media Path

#define HANGUP_SELF		    (1<<0)  // wether hangup by my self
#define HANGUP_TIMEOUT      (1<<1)	// if hanguped by remote, wehter there is a timeout 

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
	int state;				// call state flags
	int reason;            // reason to finish call, when activity = CALL_FREE - 20170519
	unsigned int start;				// Start Time
	unsigned int duration;      // Call Duration
	short type;					     // type of history record (see _Call_Type)
    char  relay_token[256];   // token allocated by relay server

}CallInfo;

typedef struct _Message_Info {
	int  callNo;   // which call does the message received
	char message[256];
}MessageInfo;
typedef struct _Control_Info {
	int callNo;	 // which call does the control received
	int type;    // type of the control (see _Control_Type)
	int value;   // parameter
}ControlInfo;

// Hold or Resume Command
enum _Hold_Resume {
	RESUME=0,
	HOLD=1,
};

// Event Type of Callback for jni level
enum _Event_Type
{
	EVENT_REGISTRATION=0,		/* Registration Result, info -> struct RegistrationInfo*/
	EVENT_STATE=1,			/* Call state Changed, info -> struct CallInfo */
	EVENT_MESSAGE=2,			/* Message Text, info -> struct MessageInfo */
	EVENT_LOG=3,				/* Log Text, info -> char* */
	EVENT_CONTROL=4,			/* conversation control from the peer, info -> struct ControlInfo* */
	EVENT_VIDEO=5,            /* Video Updated */
    EVENT_PTT=6,              /* PTT Event */
    EVENT_VADLIST=7,
};

//refer perform_state_callback
enum _Call_State  {
	CALL_FREE=0,		/* the call has been hangup byrenwu local or remote user*/
	CALL_OUTGOING=1,    /* the caller has initiated an outgoing call */
	CALL_RINGIN=2,		/* an incoming call received and play ring-in sound*/
	CALL_RINGBACK=3,    /* an outgoing call received by callee and play ringback sound*/
	CALL_ANSWERED=4,	/* the call has been answered */
	CALL_TRANSFERED=5,	/* the call has been transfered from Asterisk */
    CALL_TRANSFERED_FREE=6, /* the call has been transfered from Asterisk to a free path, e.g NAT/LAN */
};
enum _Call_Type {
	OUTGOING_CALL=0,
	INCOMING_CALL=1,
	MISSED_CALL=2,
	REJECTED_CALL=3,
};


typedef struct _Ptt_Info
{
    int   callNo;  // call associated with the band
    int   event;   // PTT event type (refer to _PTT_EVENT)
    char  speaker_id[32];  // Speaker's FreePP ID, available when event=TB_TAKEN
    short position;        // position of my request in queue of server available when event=TB_QUEUED
}PttInfo;
typedef struct _VadList_Info
{
    int   callNo;//dont to use it
    char vadlist[512];
}VadListInfo;

typedef int (*wtkcall_jni_event_callback_t)( int event_type, void* event_info);
//Extern API
#ifdef __cplusplus
extern "C" {
#endif
extern void 	wtkcall_set_jni_event_callback( wtkcall_jni_event_callback_t func );
extern int 		wtkcall_initialize_iax(void);
extern int 		wtkcall_initialize_media(void);
extern void 	wtkcall_shutdown_iax(void);
extern void 	wtkcall_shutdown_media(void);
extern int 		wtkcall_register(const char* name,const char *number,const char *pass,const char *host,const char *port);
extern void 	wtkcall_unregister( int id );
extern int 		wtkcall_dial(const char* dest,const char* host,const char* user,const char *cmd,const char* ext);
extern void 	wtkcall_answer( int callNo );
extern void 	wtkcall_select(int callNo);
extern int 		wtkcall_hangup( int callNo);
extern int 		wtkcall_hold(int callNo, bool hold);
extern int 		wtkcall_mute(int callNo, bool mute);
extern int 		wtkcall_set_format( int callNo, int rtp_format);
#ifdef __cplusplus
}
#endif

#endif
