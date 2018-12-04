#ifndef _wtk_media_sdk_misc_define_h_
#define _wtk_media_sdk_misc_define_h_

#define ERROR_NONE		"0"
//REGISTER MEDIA SERVER
#define ERROR_NEWCALL_REGISTER		"-100"
#define ERROR_NEWCALL_MEDIA_TYPE	"-101"
//COMMON ERROR
#define ERROR_HTTP_PROFILE			"-200"
#define ERROR_HTTP_INSTANCE			"-201"
#define ERROR_HTTP_RETRY_TIMEOUT	"-202"
#define ERROR_HTTP_READ_JSON		"-203"
#define ERROR_RESTAPI_OTHER			"-204"
//NEW CALL
#define ERROR_RESTAPI_PEEROFFLINE		"-300"
#define ERROR_RESTAPI_REJECTED			"-301"
#define ERROR_RESTAPI_PEERNOTFOUND		"-302"
#define ERROR_RESTAPI_DENIED			"-303"
#define ERROR_RESTAPI_TRANSMITTIMEOUT	"-304"
//CALL LOG
#define ERROR_RESTAPI_LOG_CALLNOTFOUND		"-400"
#define ERROR_RESTAPI_LOG_UPLOADDBERROR		"-401"
#define ERROR_RESTAPI_LOG_INVALIDFORMAT		"-402"
#define ERROR_RESTAPI_LOG_DENIED			"-403"

//Json keys
#define MSG_JSON_KEY_OPER_RESULT		"oper_result"
#define MSG_JSON_KEY_CS_CALLID          "cs_callid"
#define MSG_JSON_KEY_CS_TICKETID        "cs_ticketid"

//Timer
#define NEW_CALL_SETUP_TIMEOUT_TIMER	"new_call_setup_timer"
#define SEARCH_CALLEE_TIMEOUT_TIMER		"search_callee_ring_timer"
#define INCOMING_TIMEOUT_TIMER			"incoming_ring_timer"
#define QUAILTY_DETECT_TIMER			"quailty_detect_timer"

#define TIMER_SEPARATOR  "/"

#endif
