#ifndef _call_object_h_
#define _call_object_h_

#include <string>
#include <map>
#include <stdlib.h>

#include "rtc_base/criticalsection.h"
#include "rtc_base/platform_thread.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_define.h"

class CallObject
{
public:
	CallObject();
	~CallObject();
	
	static bool StartNewCallThread(void *param);
	int HangupCall(void);
	static void TimeoutAutoEndIncomingCall(void* param);
	void CallerDealReportAndClear();
	void CalleeDealReportAndClear();
	void ForwardNewCall();
	
	std::string video_face_;

	bool is_callee_;
	bool is_caller_rec_rck_;
	std::string caller_src_pid_cid_;
	std::string caller_dst_pid_;
	
	std::string callee_src_pid_cid_;//For callee this is caller pid+cid; for caller this is dst pid_cid(caller_dst_pid_cid_)
	std::string callee_srcm_;
	std::string callee_dst_pid_cid_; //For callee this is my pid+cid
	std::string callee_ms_info_;
	
	std::string core_server_callid_;
	std::string outbound_ticket_;
	std::string outbound_ms_info_;
	std::string outbound_pos_info_;
	std::string outbound_via_info_;
	
	int media_type_;
	
	int call_type_;
	int call_state_;
	int call_rec_state_;
	int call_hangup_reason_;

	rtc::PlatformThread* new_call_thread_;

	char callid_[MAX_CALLID_SIZE];
	int call_no_;
	int media_server_index_;
private:
	void FinishNewCall(std::map<std::string, std::string> &dictResult);
	static void SearchCalleeRingCallBack(void *param);
	static void NewCallSetupCallBack(void *param);

	bool is_auto_end_call_;//nouse
	rtc::CriticalSection call_object_lock_;
};
#endif
