#ifndef _wtk_call_ctrl_h_
#define _wtk_call_ctrl_h_

#include "call_object.h"
#include <map>

class WtkCallCtrl
{
public:
	WtkCallCtrl();
	~WtkCallCtrl();
	
	bool IsExistCurrentCallObject(void);

	void CreateWtkCall(const char *dstID, const char *srcID, int media, char *outCallID);
	int AnswerCall(const char* CallId);
	int HoldCall(const char* CallId, int hold);
	int MuteCall(const char* CallId, int mute);
	int HangupCall(const char* CallId);
	void SetVideoDisplay(void *localView, void *remoteView);
	void StartVideoSend(const char* CallId);
	void StopVideoSend(const char* CallId, int reason);
	void SetCamera(int device_id);
	void CreateOutboundCall(const char *dstID, const char *srcID, const char *posInfo,const char *msInfo,const char *via, int media_type, char *outCallID);
	int GetCoreServerId(const char* CallId, char *CoreServerId);
	int GetCallQualityLevel(const char* CallId);
	
	void HandleNCLMessage(std::string str_msg_json);
	void HandleRCLMessage(std::string str_msg_json);
	void HandleRCKMessage(std::string str_msg_json);
	void HandleRCAMessage(std::string str_msg_json);
	void HandleMLGMessage(std::string str_msg_json);
	void HandleRCFMessage(std::string str_msg_json);
	
	void HandleWtkCallStateEvent(CallStateEvent *call_event);
	void HandleWtkCallMsgEvent(MessageEvent* msg_event);

	CallObject *cur_call_object_;
	std::map<std::string, CallObject*> all_callid_to_call_obj_dict_;
	
private:
	rtc::CriticalSection wtk_call_lock_;
};

#endif
