#ifndef _wtk_media_sdk_define_h
#define _wtk_media_sdk_define_h


#define WTK_MEDIA_SDK_VERSION		"V67.0.0.2"
#define CONFID_LEN			27
#define MAX_CALLID_SIZE		32
#define MAX_NAMEID_SIZE		128

#define WTKMEDIA_INITIAX	(1<<0)
#define WTKMEDIA_LOGINIAX	(1<<1)

#define WTK_MEDIA_IAX_KEY	"[wtk-mobile@"
#define CALL_BACK_THREAD_SLEEP_MS	200

/********************************************
WTK supported params
********************************************/
#define KEY_REG_MS_TIMEOUT				"reg_ms_timeout"
#define KEY_CALL_TIMEOUT				"call_timeout"
#define KEY_HANDLE_GSM_STATE 			"handle_gsm_state"
#define KEY_AUDIO_MODE 					"audio_mode"
#define KEY_USE_HTTPS 					"use_https"
#define KEY_HTTPS_CA_PATH				"https_ca_path"
#define KEY_HTTP_RETRY_TIMES 			"http_retry_times"
#define KEY_HTTP_RETRY_WAIT_TIME 		"http_retry_wait_time"
#define KEY_AUDIO_INPUT_VOLUME_LEVEL 	"audio_input_volume_level"
#define KEY_AUDIO_OUTPUT_VOLUME_LEVEL 	"audio_output_volume_level"
#define KEY_AUDIO_MIN_BPS				"audio_min_bps"
#define KEY_AUDIO_MAX_BPS 				"audio_max_bps"
#define KEY_AUTO_SEND_VIDEO				"video_auto_send"
#define KEY_VIDEO_CAMERA_MODE			"video_camera_mode"
#define KEY_VIDEO_FRAME_WIDTH			"video_frame_width"
#define KEY_VIDEO_FRAME_HEIGHT			"video_frame_height"
#define KEY_VIDEO_ORIENTATION			"video_orientation"
#define KEY_VIDEO_MIN_BPS				"video_min_bps"
#define KEY_VIDEO_MAX_BPS				"video_max_bps"
#define KEY_VIDEO_SEND_CODEC			"video_send_codec"
// Push Message Type
#define PUSH_MSG_TYPE_NCL		"NCL"
#define PUSH_MSG_TYPE_RCL		"RCL"
#define PUSH_MSG_TYPE_RCK		"RCK"
#define PUSH_MSG_TYPE_RCA		"RCA"
#define PUSH_MSG_TYPE_MLG		"MLG"
#define PUSH_MSG_TYPE_RCF       "RCF" 

//Message control: video and conference 
#define VIDEO_START_CMD_REQ         "WTK_VIDEO_START_REQ"
#define VIDEO_START_CMD_RSP         "WTK_VIDEO_START_RSP"
#define VIDEO_STOP_CMD_REQ          "WTK_VIDEO_STOP_REQ"
#define VIDEO_STOP_CMD_RSP          "WTK_VIDEO_STOP_RSP"

#define CONF_CMD_KEY_QCS  		"QCS" // Query Conference Status
#define CONF_CMD_KEY_ICS  		"ICS" // Initialize Conference Status
#define CONF_CMD_KEY_LST  		"LST" //participants info list return
#define CONF_CMD_KEY_KIC		"KIC"
#define CONF_CMD_KEY_JOI  		"JOI"
#define CONF_CMD_KEY_MUT   		"MUT"


//SDK event accord with wtkcall_lib.h
enum Wtk_Reg_Result {
	WTK_REG_ACCEPT=0,
	WTK_REG_REJECT=1,
	WTK_REG_TIMEOUT=2,
};

enum Wtk_Event_Type
{
	WTK_EVENT_REG=0,
	WTK_EVENT_STATE=1,
	WTK_EVENT_MESSAGE=2,
	WTK_EVENT_LOG=3,
    WTK_EVENT_PTT=4,
    WTK_EVENT_VADLIST=5,
	//WTK_EVENT_CONTROL=6,
	//WTK_EVENT_VIDEO=7,
};

enum Wtk_Media_Type {
	WTK_AUDIO_MEDIA_TYPE = 1,
	WTK_VIDEO_MEDIA_TYPE = 2,
	WTK_AUDIO_CONF_MEDIA_TYPE = 3,
	WTK_VIDEO_CONF_MEDIA_TYPE = 4,
	WTK_AUDIO_CONF_CENTER_MEDIA_TYPE = 5,
	WTK_VIDEO_CONF_CENTER_MEDIA_TYPE = 6,
	WTK_AUDIO_OUTBOUND_MEDIA_TYPE = 7,
	WTK_VIDEO_OUTBOUND_MEDIA_TYPE = 8
};

enum Wtk_Call_State  {
    WTK_CALL_STATE_IDLE=0,
    WTK_CALL_STATE_INITIATE=1,
    WTK_CALL_STATE_ANSWER=2,
    WTK_CALL_STATE_HOLD=3,
    WTK_CALL_STATE_HANGUP=4,
};
enum Wtk_Call_Type {
	WTK_CALL_TYPE_OUTGOING=0,
	WTK_CALL_TYPE_INCOMING=1,
	WTK_CALL_TYPE_MISSED=2,
	WTK_CALL_TYPE_REJECTED=3,
	WTK_CALL_TYPE_IDLE=4,
};
enum Wtk_Receive_Call_Type {
	WTK_REC_CALL_TYPE_IDLE = 0,
	WTK_REC_CALL_TYPE_INCOMING = 1,
    WTK_REC_CALL_TYPE_MISSED_CALL = 2,
    WTK_REC_CALL_TYPE_FINISHED_CONF = 3
};
enum Wtk_call_answer_reason {
    WTK_REASON_ANSWER_NORMAL = 0,
    WTK_REASON_ANSWER_RELAY = 1,
    WTK_REASON_ANSWER_NAT = 2,
    WTK_REASON_ANSWER_P2P = 3,
};

enum Wtk_Call_Hangup_Reason {
    WTK_REASON_HANGUP_NULL = 0,
	WTK_REASON_HANGUP_MS_REGISTER = 1,
    WTK_REASON_HANGUP_REJECT = 2,
    WTK_REASON_HANGUP_BUSY = 3,
    WTK_REASON_HANGUP_NO_ANSWER = 4,//search callee ring timeout
    WTK_REASON_HANGUP_HTTP = 5,//http error
    WTK_REASON_HANGUP_RESTAPI = 6,//core server return error
    WTK_REASON_HANGUP_TIMEOUT = 7,//new call setup timeout
    WTK_REASON_HANGUP_UNKNOWN = 8,
    WTK_REASON_HANGUP_CANCEL = 9,
    WTK_REASON_HANGUP_ALREADY_ANSWERED = 14,
    WTK_REASON_HANGUP_FINISH = 15, 
    WTK_REASON_HANGUP_INGSMCALL = 16,
};
enum Wtk_Remote_Video_State  {
    WTK_REMOTE_VIDEO_STOP=0,
    WTK_REMOTE_VIDEO_START=1,
};

//Report to APP's event, include register/callstate/callctrl/message/ptt/vadlist events.
struct RegisterEvent
{
	int regID;
	int regReply;
};
struct CallStateEvent {
	int callNo;
	char peer_name[256];
	char peer_number[256];
	int activity;
	int reason;
	unsigned int start;
	unsigned int duration;
	unsigned int type;
};
struct MessageEvent
{
	int callNo;
	char message[256];
};

struct PTTEvent {
    int   callNo;
    int   event;
    char  speaker_id[256];
    short position;
};
struct VadListEvent {
    int   callNo;
    char vadlist[512];
}; 

#endif
