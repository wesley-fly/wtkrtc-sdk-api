#ifndef _wtk_conf_ctrl_h_
#define _wtk_conf_ctrl_h_

#include "conf_object.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_define.h"

class WtkConfCtrl
{
public:
	WtkConfCtrl();
	~WtkConfCtrl();
	
	bool IsExistCurrentConfObject(void);
	
	//Normal functions
	void CreateWtkConfCall(const char *dstID, const char * srcID, int media, char *outCallID);
	int AnswerConference(const char *ConfID);
	int HangupCall(const char *ConfID);
	void StartVideoSend(const char* ConfID);
	void StopVideoSend(const char* ConfID, int reason);
	void SetCamera(int device_id);
	int JoinConference(std::vector<std::string>* DstIDs,const char* GroupID, int media_type,const char *srcID,const char *confID, char *outConfID);
	int HangupConference(const char *ConfID);
	int ListConference(const char *ConfID);
	int CtrlConference(const char *ConfID, int ctrl_action,const char *dstID);
	int GetCoreServerConfId(const char* ConfID, char *CoreServerConfId);
	int GetConfQualityLevel(const char* ConfID);
	
	//Hander event
	void HandleNCLMessage(std::string str_msg_json);
	void HandleRCLMessage(std::string str_msg_json);
	void HandleRCKMessage(std::string str_msg_json);
	void HandleRCAMessage(std::string str_msg_json);
	void HandleMLGMessage(std::string str_msg_json);
	void HandleRCFMessage(std::string str_msg_json);
	
	void HandleWtkConfStateEvent(CallStateEvent *call_event);
	void HandleWtkConfMsgEvent(MessageEvent* msg_event);

	ConfObject *cur_conf_object_;
	std::map<std::string, ConfObject*> all_confid_to_conf_obj_dcit_;
private:
	rtc::CriticalSection wtk_conf_lock_;
};
#endif
