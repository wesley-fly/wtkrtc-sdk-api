#include "wtk_conf_ctrl.h"
#include "base/logging.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_instance.h"

WtkConfCtrl::WtkConfCtrl():
	cur_conf_object_(NULL)
{
}
WtkConfCtrl::~WtkConfCtrl()
{
	if(cur_conf_object_ != NULL)
		cur_conf_object_ = NULL;
	all_confid_to_conf_obj_dcit_.clear();

}
bool WtkConfCtrl::IsExistCurrentConfObject(void)
{
	if (cur_conf_object_ != NULL) 
	{
        return true;
    }
	return false;
}

void WtkConfCtrl::CreateWtkConfCall(const char *dstID, const char * srcID, int media, char *outCallID)
{
	rtc::CritScope lock(&wtk_conf_lock_);

}

int WtkConfCtrl::AnswerConference(const char *ConfID)
{
	if (ConfID == NULL || strlen(ConfID) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", ConfID is null";
		return -1;
	}
	ToolsFunction tools;
	std::string confId = ConfID;
	ConfObject *confInfoObj = (ConfObject *)tools.FindConfObjectByConfId(all_confid_to_conf_obj_dcit_, confId);
	if (confInfoObj == NULL )
	{
		DLOG(INFO) << __FUNCTION__ << ", FindConfObjectByConfId failed";
		return -1;
	}

	return 0;
}
int WtkConfCtrl::HangupCall(const char *ConfID)
{
	if (ConfID == NULL || strlen(ConfID) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", ConfID is null";
		return -1;
	}
	ToolsFunction tools;
	std::string confId = ConfID;
	ConfObject *confObject = (ConfObject *)tools.FindConfObjectByConfId(all_confid_to_conf_obj_dcit_, confId);
	if (confObject == NULL )
	{
		DLOG(INFO) << __FUNCTION__ << ", FindConfObjectByConfId failed";
		return -1;
	}

	return 0;
}

void WtkConfCtrl::StartVideoSend(const char* ConfID)
{
	if (ConfID == NULL || strlen(ConfID) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return;
	}
	std::string confId = ConfID;
	ConfObject *confObject = NULL;
	ToolsFunction tools;
	confObject = (ConfObject *)tools.FindConfObjectByConfId(all_confid_to_conf_obj_dcit_, confId);
	if(confObject == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindConfObjectByConfId failed";
		return;
	}
}
void WtkConfCtrl::StopVideoSend(const char* ConfID, int reason)
{
	if (ConfID == NULL || strlen(ConfID) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return;
	}
	std::string confId = ConfID;
	ConfObject *confObject = NULL;
	ToolsFunction tools;
	confObject = (ConfObject *)tools.FindConfObjectByConfId(all_confid_to_conf_obj_dcit_, confId);
	if(confObject == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindConfObjectByConfId failed";
		return;
	}
}
void WtkConfCtrl::SetCamera(int device_id)
{
}

int WtkConfCtrl::JoinConference(std::vector<std::string>* DstIDs, const char* GroupID, int media_type, const char *srcID, const char *confID, char *outConfID)
{
	return 0;
}
int WtkConfCtrl::HangupConference(const char *ConfID)
{
	if (ConfID == NULL || strlen(ConfID) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	std::string confId = ConfID;
	ConfObject *confObject = NULL;
	ToolsFunction tools;
	confObject = (ConfObject *)tools.FindConfObjectByConfId(all_confid_to_conf_obj_dcit_, confId);
	if(confObject == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindConfObjectByConfId failed";
		return -1;
	}
	
	return 0;
}
int WtkConfCtrl::ListConference(const char *ConfID)
{
	if (ConfID == NULL || strlen(ConfID) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	std::string confId = ConfID;
	ConfObject *confObject = NULL;
	ToolsFunction tools;
	confObject = (ConfObject *)tools.FindConfObjectByConfId(all_confid_to_conf_obj_dcit_, confId);
	if(confObject == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindConfObjectByConfId failed";
		return -1;
	}
	
	return 0;
}
int WtkConfCtrl::CtrlConference(const char *ConfID, int ctrl_action, const char *dstID)
{
	if (ConfID == NULL || strlen(ConfID) == 0 || dstID == NULL || strlen(dstID) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	std::string confId = ConfID;
	ConfObject *confObject = NULL;
	ToolsFunction tools;
	confObject = (ConfObject *)tools.FindConfObjectByConfId(all_confid_to_conf_obj_dcit_, confId);
	if(confObject == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindConfObjectByConfId failed";
		return -1;
	}
	
	return 0;
}
int WtkConfCtrl::GetCoreServerConfId(const char* ConfID, char *CoreServerConfId)
{
	/*if (CallId == NULL || strlen(CallId) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	ToolsFunction tools;
	std::string tmp_call_id = CallId;
	CallObject *tmp_call_object = NULL;
	
	tmp_call_object = (CallObject *)tools.FindObjectByCallId(all_callid_to_call_obj_dict_, tmp_call_id);
	if(tmp_call_object == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindObjectByCallId failed";
		return -1;
	}

	sprintf(CoreServerId, "%s", (char *)tmp_call_object->core_server_callid_.c_str());*/

	return 0;
}

int WtkConfCtrl::GetConfQualityLevel(const char* ConfID)
{
	/*if (CallId == NULL || strlen(CallId) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	ToolsFunction tools;
	std::string tmp_call_id = CallId;
	CallObject *tmp_call_object = NULL;
	
	tmp_call_object = (CallObject *)tools.FindObjectByCallId(all_callid_to_call_obj_dict_, tmp_call_id);
	if(tmp_call_object == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindObjectByCallId failed";
		return -1;
	}

	sprintf(CoreServerId, "%s", (char *)tmp_call_object->core_server_callid_.c_str());*/

	return 0;
}

void WtkConfCtrl::HandleNCLMessage(std::string str_msg_json)
{
}
void WtkConfCtrl::HandleRCLMessage(std::string str_msg_json)
{
}
void WtkConfCtrl::HandleRCKMessage(std::string str_msg_json)
{
}
void WtkConfCtrl::HandleRCAMessage(std::string str_msg_json)
{
}
void WtkConfCtrl::HandleMLGMessage(std::string str_msg_json)
{
}
void WtkConfCtrl::HandleRCFMessage(std::string str_msg_json)
{
}

void WtkConfCtrl::HandleWtkConfStateEvent(CallStateEvent *call_event)
{
	rtc::CritScope lock(&wtk_conf_lock_);
	DLOG(ERROR) << __FUNCTION__ << ", TODO For Conf SubmitCallLogToServer";

}
void WtkConfCtrl::HandleWtkConfMsgEvent(MessageEvent* msg_event)
{
	DLOG(ERROR) << __FUNCTION__;
}

