#include "wtk_call_ctrl.h"
#include "base/logging.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_instance.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/values.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_misc_define.h"
#include "system_wrappers/include/sleep.h"

WtkCallCtrl::WtkCallCtrl():
	cur_call_object_(NULL)
{
}
WtkCallCtrl::~WtkCallCtrl()
{
	if(cur_call_object_ != NULL)
		cur_call_object_ = NULL;
	all_callid_to_call_obj_dict_.clear();
}
bool WtkCallCtrl::IsExistCurrentCallObject(void)
{
	if (all_callid_to_call_obj_dict_.size() != 0)
		return true;
	else
		return false;
}

void WtkCallCtrl::CreateWtkCall(const char *dstID, const char * srcID, int media, char *outCallID)
{
	rtc::CritScope lock(&wtk_call_lock_);
	if (dstID == NULL || strlen(dstID) == 0 || srcID == NULL || strlen(srcID) == 0 || outCallID == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CreateWtkCall error param";
		return;
	}
	if (all_callid_to_call_obj_dict_.size() != 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CreateWtkCall has already one call object!";
		return;
	}
	CallObject *tmp_call_object = new CallObject();
	ToolsFunction tmp_tools;
	
	tmp_call_object->caller_src_pid_cid_ = srcID;
	tmp_call_object->caller_dst_pid_ = dstID;
	tmp_call_object->media_type_ = media;

	tmp_call_object->call_type_ = WTK_CALL_TYPE_OUTGOING;
	tmp_call_object->call_state_ = WTK_CALL_STATE_INITIATE;
	
	tmp_tools.GenCurrentCallID(srcID, tmp_call_object->callid_);
	
	cur_call_object_ = tmp_call_object;
	all_callid_to_call_obj_dict_[tmp_call_object->callid_] = tmp_call_object;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}

	WtkUserProfile* cur_user_profile =  wtkMediaInstance->GetUserProfile();
	
	wtkcall_config_bitrate(cur_user_profile->user_audio_min_bps_,cur_user_profile->user_audio_max_bps_,cur_user_profile->user_video_min_bps_,cur_user_profile->user_video_max_bps_);
	wtkcall_config_video(cur_user_profile->user_video_send_codec_,cur_user_profile->user_video_frame_width_,cur_user_profile->user_video_frame_height_,15,45);

	if(tmp_call_object->new_call_thread_ == NULL)
	{
		tmp_call_object->new_call_thread_ = new rtc::PlatformThread(tmp_call_object->StartNewCallThread,tmp_call_object->callid_,"WTK-NewCall");
		DLOG(INFO) << __FUNCTION__ << ", Create New WTK Call thread";
	}

	if(tmp_call_object->new_call_thread_ != NULL)
	{
		tmp_call_object->new_call_thread_->Start();
		DLOG(INFO) << __FUNCTION__ << ", Start New WTK Call thread(Normal call)";
	}
	
	sprintf(outCallID, "%s", tmp_call_object->callid_);
}
void WtkCallCtrl::CreateOutboundCall(const char *dstID,const char *srcID,const char *posInfo,const char *msInfo,const char *via, int media_type, char *outCallID)
{
	rtc::CritScope lock(&wtk_call_lock_);
	if (dstID == NULL || strlen(dstID) == 0 || srcID == NULL || strlen(srcID) == 0 || outCallID == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CreateWtkCall error param";
		return;
	}
	if (all_callid_to_call_obj_dict_.size() != 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CreateWtkCall has already one call object!";
		return;
	}
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	CallObject *tmp_call_object = new CallObject();
	ToolsFunction tmp_tools;
	
	tmp_call_object->caller_src_pid_cid_ = srcID;
	tmp_call_object->caller_dst_pid_ = dstID;
	tmp_call_object->media_type_ = media_type;
	tmp_call_object->outbound_ms_info_ = msInfo;
	tmp_call_object->outbound_pos_info_ = posInfo;
	tmp_call_object->outbound_via_info_ = via;

	tmp_call_object->call_type_ = WTK_CALL_TYPE_OUTGOING;
	tmp_call_object->call_state_ = WTK_CALL_STATE_INITIATE;
	
	tmp_tools.GenCurrentCallID(srcID, tmp_call_object->callid_);
	cur_call_object_ = tmp_call_object;
	all_callid_to_call_obj_dict_[tmp_call_object->callid_] = tmp_call_object;
	
	if(tmp_call_object->new_call_thread_ == NULL)
	{
		tmp_call_object->new_call_thread_ = new rtc::PlatformThread(tmp_call_object->StartNewCallThread,tmp_call_object->callid_,"WTK-NewCall");
		DLOG(INFO) << __FUNCTION__ << ", Create New WTK Call thread";
	}

	if(tmp_call_object->new_call_thread_ != NULL)
	{
		tmp_call_object->new_call_thread_->Start();
		DLOG(INFO) << __FUNCTION__ << ", Start New WTK Call thread(Center call)";
	}
	sprintf(outCallID, "%s", tmp_call_object->callid_);

}

int WtkCallCtrl::AnswerCall(const char* CallId)
{
	if (CallId == NULL || strlen(CallId) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	std::string callID = CallId;
	CallObject *tmp_call_object = NULL;
	ToolsFunction tmp_tools;
	tmp_call_object = (CallObject *)tmp_tools.FindObjectByCallId(all_callid_to_call_obj_dict_, callID);
	if(tmp_call_object == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindObjectByCallId failed";
		return -1;
	}
	tmp_tools.StopTimer("ALL");
	
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	WtkUserProfile *cur_user_profile = wtkMediaInstance->GetUserProfile();

	if(!(cur_user_profile->user_ms_state_ & WTKMEDIA_INITIAX))
	{
		cur_user_profile->TryLoginMediaServer(tmp_call_object->callee_dst_pid_cid_,false, 0);
	}
		
	wtkcall_config_bitrate(cur_user_profile->user_audio_min_bps_,cur_user_profile->user_audio_max_bps_,cur_user_profile->user_video_min_bps_,cur_user_profile->user_video_max_bps_);
	wtkcall_config_video(cur_user_profile->user_video_send_codec_,cur_user_profile->user_video_frame_width_,cur_user_profile->user_video_frame_height_,15,45);

	tmp_call_object->ForwardNewCall();
	
	return 0;
}
int WtkCallCtrl::HoldCall(const char* CallId, int hold)
{
	if (CallId == NULL || strlen(CallId) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	std::string callID = CallId;
	CallObject *tmp_call_object = NULL;
	ToolsFunction tools;
	tmp_call_object = (CallObject *)tools.FindObjectByCallId(all_callid_to_call_obj_dict_, callID);
	if(tmp_call_object == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindObjectByCallId failed";
		return -1;
	}

	return 0;
}
int WtkCallCtrl::MuteCall(const char* CallId, int mute)
{
	if (CallId == NULL || strlen(CallId) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	std::string callID = CallId;
	CallObject *tmp_call_object = NULL;
	ToolsFunction tools;
	tmp_call_object = (CallObject *)tools.FindObjectByCallId(all_callid_to_call_obj_dict_, callID);
	if(tmp_call_object == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindObjectByCallId failed";
		return -1;
	}
	if(mute)
	{
		wtkcall_mute(true);
	}
	else
	{
		wtkcall_mute(false);
	}
	return 0;
}
int WtkCallCtrl::HangupCall(const char* CallId)
{
	if (CallId == NULL || strlen(CallId) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	std::string callID = CallId;
	CallObject *tmp_call_object = NULL;
	ToolsFunction tools;
	tmp_call_object = (CallObject *)tools.FindObjectByCallId(all_callid_to_call_obj_dict_, callID);
	if(tmp_call_object == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindObjectByCallId failed";
		return -1;
	}
	int	ret = tmp_call_object->HangupCall();
	
	return ret;
}
void WtkCallCtrl::SetVideoDisplay(void *localView, void *remoteView)
{
	if (localView == NULL || remoteView == NULL)
	{
		wtkcall_stop_video();
		DLOG(INFO) << __FUNCTION__ << ",localView or remoteView is null";
		return;
	}
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}

	
	wtkcall_set_render(remoteView);
	if(wtkMediaInstance->GetUserProfile()->user_video_camera_mode_ == 1)
	{
		wtkcall_start_video(1);
	}
	else
	{
		wtkcall_start_video(0);
	}

	if(wtkMediaInstance->GetUserProfile()->user_video_orientation_ == 1)
	{
		wtkcall_set_capture_rotation(270);
	}
	else
	{
		wtkcall_set_capture_rotation(90);
	}
}
void WtkCallCtrl::StartVideoSend(const char* CallId)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	if (CallId == NULL || strlen(CallId) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return;
	}
	std::string call_id = CallId;
	CallObject* tmp_call_object = NULL;
	ToolsFunction tmp_tools;
	tmp_call_object = (CallObject *)tmp_tools.FindObjectByCallId(all_callid_to_call_obj_dict_, call_id);
	if(tmp_call_object == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindObjectByCallId failed";
		return;
	}

	char msg_buf[64] = {0};
	sprintf(msg_buf, "%s %s",VIDEO_START_CMD_REQ,CallId);
	wtkcall_send_text(msg_buf);
	
	wtkcall_start_capturer();
}
void WtkCallCtrl::StopVideoSend(const char* CallId, int reason)
{
	if (CallId == NULL || strlen(CallId) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", CallId is null";
		return;
	}
	std::string call_id = CallId;
	CallObject *tmp_call_object = NULL;
	ToolsFunction tmp_tools;
	tmp_call_object = (CallObject *)tmp_tools.FindObjectByCallId(all_callid_to_call_obj_dict_, call_id);
	if(tmp_call_object == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", FindObjectByCallId failed";
		return;
	}
	
	char msg_buf[64] = {0};
	sprintf(msg_buf, "%s %s",VIDEO_STOP_CMD_REQ,CallId);
	wtkcall_send_text(msg_buf);
	
	wtkcall_stop_capturer();
}
void WtkCallCtrl::SetCamera(int device_id)
{
	wtkcall_switch_camera(device_id);
}
int WtkCallCtrl::GetCoreServerId(const char* CallId, char *CoreServerId)
{
	if (CallId == NULL || strlen(CallId) == 0)
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

	sprintf(CoreServerId, "%s", (char *)tmp_call_object->core_server_callid_.c_str());

	return 0;
}

int WtkCallCtrl::GetCallQualityLevel(const char* CallId)
{
	if (CallId == NULL || strlen(CallId) == 0)
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
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	int audio_level = 0;
	int video_level = 0;

	int audio_poor_level = wtkMediaInstance->GetUserProfile()->user_audio_min_bps_/2;
	int audio_good_level = wtkMediaInstance->GetUserProfile()->user_audio_min_bps_;
	int audio_great_level = wtkMediaInstance->GetUserProfile()->user_audio_max_bps_/2;
	int audio_perfect_level = wtkMediaInstance->GetUserProfile()->user_audio_max_bps_;

	int video_poor_level = wtkMediaInstance->GetUserProfile()->user_video_min_bps_/2;
	int video_good_level = wtkMediaInstance->GetUserProfile()->user_video_min_bps_;
	int video_great_level = wtkMediaInstance->GetUserProfile()->user_video_min_bps_/2;
	int video_perfect_level = wtkMediaInstance->GetUserProfile()->user_video_max_bps_;

	wtkcall_get_call_quality(&audio_level,&video_level);
	DLOG(ERROR) << __FUNCTION__ << ",audio_level = " << audio_level << ",video_level = " << video_level;

	int ret_level = 0;
	int ret_video_level = 0;
	int ret_audio_level = 0;

	if(audio_level == 0)
	{
		ret_audio_level = 1;
	}
	else if(audio_level > 0 && audio_level < audio_poor_level)
	{
		ret_audio_level = 2;
	}
	else if(audio_level >= audio_poor_level && audio_level < audio_good_level)
	{
		ret_audio_level = 3;
	}
	else if(audio_level >= audio_good_level && audio_level < audio_great_level)
	{
		ret_audio_level = 4;
	}
	else if(audio_level >= audio_great_level && audio_level < audio_perfect_level)
	{
		ret_audio_level = 5;
	}
	else if(audio_level >= audio_perfect_level)
	{
		ret_audio_level = 6;
	}

	if(video_level == 0)
	{
		ret_video_level = 1;
	}
	else if(video_level > 0 && video_level < video_poor_level)
	{
		ret_video_level = 2;
	}
	else if(video_level >= video_poor_level && video_level < video_good_level)
	{
		ret_video_level = 3;
	}
	else if(video_level >= video_good_level && video_level < video_great_level)
	{
		ret_video_level = 4;
	}
	else if(video_level >= video_great_level && video_level < video_perfect_level)
	{
		ret_video_level = 5;
	}
	else if(video_level >= video_perfect_level)
	{
		ret_video_level = 6;
	}

	ret_level = ((ret_video_level << 16) & 0xFFFF0000);
	ret_level |= (ret_audio_level & 0x0000FFFF);
	return ret_level;
}

void WtkCallCtrl::HandleNCLMessage(std::string str_msg_json)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	CallObject* tmp_call_object = new CallObject();
	std::unique_ptr<base::Value> root = base::JSONReader::Read(str_msg_json);
	if(root)
	{
		base::DictionaryValue* root_dict = nullptr;
		if(root->GetAsDictionary(&root_dict))
		{
			std::string str_call_type;
			root_dict->GetString("calltype", &str_call_type);
			tmp_call_object->media_type_ = atoi(str_call_type.c_str());
			root_dict->GetString("cs_callid", &tmp_call_object->core_server_callid_);
			std::string str_call_id;
			root_dict->GetString("callid", &str_call_id);
			sprintf(tmp_call_object->callid_, "%s", str_call_id.c_str());
			root_dict->GetString("rs", &tmp_call_object->callee_ms_info_);
			root_dict->GetString("dst", &tmp_call_object->callee_dst_pid_cid_);
			root_dict->GetString("src", &tmp_call_object->callee_src_pid_cid_);
			root_dict->GetString("srcm", &tmp_call_object->callee_srcm_);
			if(tmp_call_object->media_type_ == WTK_AUDIO_OUTBOUND_MEDIA_TYPE||tmp_call_object->media_type_ == WTK_VIDEO_OUTBOUND_MEDIA_TYPE)
			{
				root_dict->GetString("ticket", &tmp_call_object->outbound_ticket_);
			}
			
			tmp_call_object->is_callee_ = true;
			tmp_call_object->call_type_ = WTK_CALL_TYPE_INCOMING;
			tmp_call_object->call_state_ = WTK_CALL_STATE_INITIATE;
			tmp_call_object->call_rec_state_ = WTK_REC_CALL_TYPE_INCOMING;

			if(all_callid_to_call_obj_dict_.size() > 0)
			{
				/*wtkMediaInstance->GetSDKEventImpl()->onReceiveCallEvent(tmp_call_object->callid_,
																		(char*)tmp_call_object->callee_src_pid_cid_.c_str(),
																		(char*)tmp_call_object->callee_dst_pid_cid_.c_str(),
																		(char*)tmp_call_object->callee_srcm_.c_str(),
																		tmp_call_object->media_type_,
																		WTK_REC_CALL_TYPE_MISSED_CALL);*/
				
				wtkMediaInstance->GetNotificationManager()->PushMessageToServer(PUSH_MSG_TYPE_RCK,
																				tmp_call_object->callid_,
																				tmp_call_object->callee_dst_pid_cid_,
																				tmp_call_object->callee_src_pid_cid_,
																				"",
																				tmp_call_object->core_server_callid_,
																				WTK_REASON_HANGUP_BUSY,
																				tmp_call_object->media_type_);

				return;
			}
			tmp_call_object->video_face_ = wtkMediaInstance->GetUserProfile()->vec_user_media_info_[tmp_call_object->media_server_index_].user_camera_face_;
			cur_call_object_ = tmp_call_object;
			all_callid_to_call_obj_dict_[tmp_call_object->callid_] = tmp_call_object;

			wtkMediaInstance->GetSDKEventImpl()->onReceiveCallEvent(tmp_call_object->callid_,
																	(char*)tmp_call_object->callee_src_pid_cid_.c_str(),
																	(char*)tmp_call_object->callee_dst_pid_cid_.c_str(),
																	(char*)tmp_call_object->callee_srcm_.c_str(),
																	tmp_call_object->media_type_,
																	tmp_call_object->call_rec_state_);
			ToolsFunction tmp_tools;
			std::string timer_name = str_call_id + TIMER_SEPARATOR + INCOMING_TIMEOUT_TIMER;
			tmp_tools.StartTimer(timer_name.c_str(),tmp_call_object->TimeoutAutoEndIncomingCall,tmp_call_object->callid_,(wtkMediaInstance->GetUserProfile()->user_call_timeout_) * 1000, false);
		}
		else
		{
			DLOG(ERROR) << __FUNCTION__ << ",Parse json error!";
		}
	}
	else
	{
		DLOG(ERROR) << __FUNCTION__ << ",Parse json error!";
	}
}
void WtkCallCtrl::HandleRCLMessage(std::string str_msg_json)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	std::unique_ptr<base::Value> root = base::JSONReader::Read(str_msg_json);
	if(root)
	{
		base::DictionaryValue* root_dict = nullptr;
		if(root->GetAsDictionary(&root_dict))
		{
			std::string str_call_id;
			ToolsFunction tmp_tools;
			
			root_dict->GetString("callid", &str_call_id);
			
			CallObject* tmp_call_object = (CallObject*)tmp_tools.FindObjectByCallId(all_callid_to_call_obj_dict_, str_call_id);
			if(tmp_call_object == NULL)
			{
				DLOG(ERROR) << __FUNCTION__ << ",FindObjectByCallId return null";
				return;
			}
			else
			{				
				std::string str_hangup_cause;
				root_dict->GetString("cause", &str_hangup_cause);

				tmp_call_object->call_type_ = WTK_CALL_TYPE_MISSED;
				tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
				tmp_call_object->call_rec_state_ = WTK_REC_CALL_TYPE_MISSED_CALL;

				int call_hangup_cause = atoi(str_hangup_cause.c_str());
				if(call_hangup_cause == WTK_REASON_HANGUP_NO_ANSWER)
				{
					tmp_call_object->call_hangup_reason_ = WTK_REASON_HANGUP_CANCEL;
				}
				else
				{
					tmp_call_object->call_hangup_reason_ = call_hangup_cause;
				}
				
				tmp_call_object->CalleeDealReportAndClear();
				tmp_tools.StopTimer("ALL");
			}
		}
		else
		{
			DLOG(ERROR) << __FUNCTION__ << ",Parse json error!";
		}
	}
	else
	{
		DLOG(ERROR) << __FUNCTION__ << ",Parse json error!";
	}
}
void WtkCallCtrl::HandleRCKMessage(std::string str_msg_json)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}

	std::unique_ptr<base::Value> root = base::JSONReader::Read(str_msg_json);
	if(root)
	{
		base::DictionaryValue* root_dict = nullptr;
		if(root->GetAsDictionary(&root_dict))
		{
			std::string str_call_id;
			ToolsFunction tmp_tools;
			
			root_dict->GetString("callid", &str_call_id);
			
			CallObject* tmp_call_object = (CallObject*)tmp_tools.FindObjectByCallId(all_callid_to_call_obj_dict_, str_call_id);
			if(tmp_call_object == NULL)
			{
				DLOG(ERROR) << __FUNCTION__ << ",FindObjectByCallId return null";
				return;
			}
			else
			{				
				std::string str_hangup_cause;
				root_dict->GetString("cause", &str_hangup_cause);
				
				tmp_call_object->is_caller_rec_rck_ = true;

				tmp_call_object->call_type_ = WTK_CALL_TYPE_REJECTED;
				tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
				tmp_call_object->call_hangup_reason_ = atoi(str_hangup_cause.c_str());

				tmp_call_object->CallerDealReportAndClear();
				tmp_tools.StopTimer("ALL");
			}
		}
		else
		{
			DLOG(ERROR) << __FUNCTION__ << ",Parse json error!";
		}
	}
	else
	{
		DLOG(ERROR) << __FUNCTION__ << ",Parse json error!";
	}
}
void WtkCallCtrl::HandleRCAMessage(std::string str_msg_json)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	std::unique_ptr<base::Value> root = base::JSONReader::Read(str_msg_json);
	if(root)
	{
		base::DictionaryValue* root_dict = nullptr;
		if(root->GetAsDictionary(&root_dict))
		{
			std::string str_call_id;
			ToolsFunction tmp_tools;
			
			root_dict->GetString("callid", &str_call_id);
			
			CallObject* tmp_call_object = (CallObject*)tmp_tools.FindObjectByCallId(all_callid_to_call_obj_dict_, str_call_id);
			if(tmp_call_object == NULL)
			{
				DLOG(ERROR) << __FUNCTION__ << ",FindObjectByCallId return null";
				return;
			}
			else
			{				
				std::string str_hangup_cause;
				root_dict->GetString("cause", &str_hangup_cause);
				
				tmp_call_object->is_caller_rec_rck_ = true;

				tmp_call_object->call_type_ = WTK_CALL_TYPE_REJECTED;
				tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
				tmp_call_object->call_hangup_reason_ = atoi(str_hangup_cause.c_str());

				tmp_call_object->CallerDealReportAndClear();
				tmp_tools.StopTimer("ALL");
			}
		}
		else
		{
			DLOG(ERROR) << __FUNCTION__ << ",Parse json error!";
		}
	}
	else
	{
		DLOG(ERROR) << __FUNCTION__ << ",Parse json error!";
	}
}
void WtkCallCtrl::HandleMLGMessage(std::string str_msg_json)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}

	std::unique_ptr<base::Value> root = base::JSONReader::Read(str_msg_json);
	if(root)
	{
		base::DictionaryValue* root_dict = nullptr;
		if(root->GetAsDictionary(&root_dict))
		{
			std::string str_call_id;
			ToolsFunction tmp_tools;
			
			root_dict->GetString("callid", &str_call_id);
			
			CallObject* tmp_call_object = (CallObject*)tmp_tools.FindObjectByCallId(all_callid_to_call_obj_dict_, str_call_id);
			if(tmp_call_object == NULL)
			{
				DLOG(ERROR) << __FUNCTION__ << ",FindObjectByCallId return null";
				return;
			}
			else
			{				
				std::string str_hangup_cause;
				root_dict->GetString("cause", &str_hangup_cause);

				tmp_call_object->call_type_ = WTK_CALL_TYPE_MISSED;
				tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
				tmp_call_object->call_rec_state_ = WTK_REC_CALL_TYPE_MISSED_CALL;
				tmp_call_object->call_hangup_reason_ = atoi(str_hangup_cause.c_str());
				wtkMediaInstance->GetSDKEventImpl()->onReceiveCallEvent(tmp_call_object->callid_,
																		(char*)tmp_call_object->callee_src_pid_cid_.c_str(),
																		(char*)tmp_call_object->callee_dst_pid_cid_.c_str(),
																		(char*)tmp_call_object->callee_srcm_.c_str(),
																		tmp_call_object->media_type_,
																		tmp_call_object->call_rec_state_);
				tmp_call_object->CallerDealReportAndClear();
				ToolsFunction tmp_tools;
				tmp_tools.StopTimer("ALL");
			}
		}
		else
		{
			DLOG(ERROR) << __FUNCTION__ << ",Parse json error!";
		}
	}
}
void WtkCallCtrl::HandleRCFMessage(std::string str_msg_json)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
}

void WtkCallCtrl::HandleWtkCallStateEvent(CallStateEvent *call_event)
{
	/*DLOG(INFO) << __FUNCTION__ << "callEvent->callNo = " << call_event->callNo;
	DLOG(INFO) << __FUNCTION__ << "callEvent->peer_name = " << call_event->peer_name;
	DLOG(INFO) << __FUNCTION__ << "callEvent->peer_number = " << call_event->peer_number;
	DLOG(INFO) << __FUNCTION__ << "callEvent->activity = " << call_event->activity;
	DLOG(INFO) << __FUNCTION__ << "callEvent->reason = " << call_event->reason;
	DLOG(INFO) << __FUNCTION__ << "callEvent->start = " << call_event->start;
	DLOG(INFO) << __FUNCTION__ << "callEvent->duration = " << call_event->duration;
	DLOG(INFO) << __FUNCTION__ << "callEvent->type = " << call_event->type;*/

	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	ToolsFunction tmp_tools;
	CallObject *tmp_call_object = NULL;
	std::string tmp_peer_name = call_event->peer_name;
	WtkUserProfile* user_profile = wtkMediaInstance->GetUserProfile();
	if(tmp_peer_name.find(WTK_MEDIA_IAX_KEY) != std::string::npos)
	{
		int start_pos = (int)tmp_peer_name.find(WTK_MEDIA_IAX_KEY);
		int end_pos = (int)tmp_peer_name.find("]");

		std::string tmp_call_id = tmp_peer_name.substr(start_pos + strlen(WTK_MEDIA_IAX_KEY), (end_pos - start_pos - strlen(WTK_MEDIA_IAX_KEY)));
		tmp_call_object = tmp_tools.FindObjectByCallId(all_callid_to_call_obj_dict_, tmp_call_id);
	}
	else
	{
		tmp_call_object = tmp_tools.FindObjectByCallNo(all_callid_to_call_obj_dict_, call_event->callNo);
	}
	
	if(tmp_call_object == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CALL_ANSWERED still can not find tmp_call_object, maybe error!";
		return;
	}

	switch(call_event->activity)
	{
		case CALL_FREE:
		{
			DLOG(ERROR) << __FUNCTION__ << ", CALL_FREE";
			
			if(tmp_call_object->call_state_ == WTK_CALL_STATE_ANSWER || tmp_call_object->call_state_ == WTK_CALL_STATE_HOLD)
			{
				std::string stats_json_string;

				base::DictionaryValue root_dict;
				std::unique_ptr<base::DictionaryValue> period_dict0(new base::DictionaryValue());
				int audio_send_bps;
				int audio_rec_bps;
				int audio_pkg_lost;
				int video_send_bps;
				int video_rec_bps;
				int video_perfer_bps;
				
				if(wtkcall_get_audio_stats(&audio_send_bps, &audio_rec_bps, &audio_pkg_lost) != 0)
				{
					audio_send_bps = 0;
					audio_rec_bps = 0;
					audio_pkg_lost = 0;
				}
				
				if(wtkcall_get_video_stats(&video_send_bps, &video_rec_bps, &video_perfer_bps) != 0)
				{
					video_send_bps = 0;
					video_rec_bps = 0;
					video_perfer_bps = 0;
				}

				period_dict0->SetKey("audio_send_bps", base::Value(audio_send_bps));
				period_dict0->SetKey("audio_rec_bps", base::Value(audio_rec_bps));
				period_dict0->SetKey("audio_pkg_lost", base::Value(audio_pkg_lost));
				period_dict0->SetKey("video_send_bps", base::Value(video_send_bps));
				period_dict0->SetKey("video_rec_bps", base::Value(video_rec_bps));
				
				root_dict.SetWithoutPathExpansion("statics", std::move(period_dict0));
		
				root_dict.SetKey("start_time", base::Value((int)call_event->start));
				root_dict.SetKey("is_caller", base::Value(tmp_call_object->is_callee_));
				std::string media_server_port = user_profile->vec_user_media_info_[tmp_call_object->media_server_index_].user_media_server_ + ":" + user_profile->vec_user_media_info_[tmp_call_object->media_server_index_].user_media_port_;
				root_dict.SetKey("server_ip_port", base::Value(media_server_port));
				root_dict.SetKey("wtk_sdk_callid", base::Value(tmp_call_object->callid_));
				root_dict.SetKey("wtk_sdk_version", base::Value(WTK_MEDIA_SDK_VERSION));
				base::JSONWriter::Write(root_dict, &stats_json_string);

				WtkMediaLog* submit_call_log = wtkMediaInstance->GetMediaLogManager();

				if(tmp_call_object->is_callee_)
				{
					submit_call_log->pid_cid_ = tmp_call_object->callee_dst_pid_cid_;
				}
				else
				{
					submit_call_log->pid_cid_ = tmp_call_object->caller_src_pid_cid_;
				}
				submit_call_log->stats_json_string_ = stats_json_string;
				submit_call_log->core_server_callid_ = tmp_call_object->core_server_callid_;
				submit_call_log->call_duration_ = call_event->duration;
				submit_call_log->TryStartMediaLogThread();
				
				wtkcall_stop_capturer();
				wtkcall_stop_video();
				wtkcall_stop_audio();
			}
			//Normal Free event handle
			if(tmp_call_object->is_callee_)
			{
				if(call_event->reason == 58)//Call rejected by remote (int)e->ies.causecode = 58 Unable to negotiate codec
				{
					tmp_call_object->call_hangup_reason_ = WTK_REASON_HANGUP_REJECT;
					tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
				}
				else
				{
					tmp_call_object->call_hangup_reason_ = WTK_REASON_HANGUP_NULL;
					tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
				}
				tmp_call_object->CalleeDealReportAndClear();
				tmp_tools.StopTimer("ALL");
			}
			else
			{
				if(call_event->reason == 58)//Call rejected by remote (int)e->ies.causecode = 58 Unable to negotiate codec
				{
					tmp_call_object->call_hangup_reason_ = WTK_REASON_HANGUP_REJECT;
					tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
				}
				else
				{
					tmp_call_object->call_hangup_reason_ = WTK_REASON_HANGUP_NULL;
					tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
				}
				tmp_call_object->CalleeDealReportAndClear();
				tmp_tools.StopTimer("ALL");
			}
		}
		break;
		case CALL_OUTGOING:
		{
			DLOG(ERROR) << __FUNCTION__ << ", CALL_OUTGOING";
		}
		break;
		case CALL_RINGIN:
		{
			DLOG(ERROR) << __FUNCTION__ << ", CALL_RINGIN:" << tmp_call_object->callee_src_pid_cid_;
			
			if(!tmp_call_object->is_callee_)
			{
				//Caller update from dst pid to dst pid+cid
				tmp_call_object->callee_src_pid_cid_ = call_event->peer_number;
			}
			//Because core server first send ncl to peer and then response caller's RESTAPI resuest, 
			//so at here wait for core server's response first
			int count = 0;
			while(tmp_call_object->core_server_callid_.empty() && count < 20)
			{
				webrtc::SleepMs(500);
				count++;
			}
			if(count < 20)
			{
				if (tmp_call_object->call_no_ < 0)
				{
					tmp_call_object->call_no_ = call_event->callNo;
					wtkcall_answer(call_event->callNo);
					wtkcall_select(call_event->callNo);
				}
			}
			else
			{
				wtkcall_hangup(call_event->callNo);
				DLOG(ERROR) << __FUNCTION__ << ", CALL_RINGIN: Caller can't wait RestAPI response, so no coreserver call id, this call will no RCA and 45s later will hangup, so hangup at first!";
			}
		}
		break;
		case CALL_RINGBACK:
		{
			DLOG(ERROR) << __FUNCTION__ << ", CALL_RINGBACK";
		}
		break;
		case CALL_ANSWERED:
		{
			DLOG(ERROR) << __FUNCTION__ << ", CALL_ANSWERED";
			
			wtkcall_start_audio();
			
			tmp_call_object->call_state_ = WTK_CALL_STATE_ANSWER;
			switch (tmp_call_object->media_type_)
			{
				case WTK_AUDIO_MEDIA_TYPE:
				case WTK_VIDEO_MEDIA_TYPE:
				case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
				case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
					wtkMediaInstance->GetSDKEventImpl()->onCallStateEvent(tmp_call_object->callid_,tmp_call_object->call_state_,WTK_REASON_ANSWER_NORMAL);
				break;
				default:
				break;
			}
			if(!tmp_call_object->is_callee_)
			{
				wtkMediaInstance->GetNotificationManager()->PushMessageToServer(PUSH_MSG_TYPE_RCA,
																				tmp_call_object->callid_,
																				tmp_call_object->caller_src_pid_cid_,
																				tmp_call_object->callee_src_pid_cid_,
																				"",
																				tmp_call_object->core_server_callid_,
																				WTK_REASON_HANGUP_ALREADY_ANSWERED,
																				tmp_call_object->media_type_);
			}
			tmp_tools.StopTimer("ALL");
			//wtkMediaInstance->GetUserProfile()->TryLogoutMediaServer(false);
		}
		break;
		case CALL_TRANSFERED_RS:
		{
			DLOG(ERROR) << __FUNCTION__ << ", CALL_TRANSFERED_RS";
			tmp_call_object->call_state_ = WTK_CALL_STATE_ANSWER;
			switch (tmp_call_object->media_type_)
			{
				case WTK_AUDIO_MEDIA_TYPE:
				case WTK_VIDEO_MEDIA_TYPE:
				case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
				case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
					wtkMediaInstance->GetSDKEventImpl()->onCallStateEvent(tmp_call_object->callid_,tmp_call_object->call_state_,WTK_REASON_ANSWER_RELAY);
				break;
				default:
				break;
			}
		}
		break;
		case CALL_TRANSFERED_NAT:
		{
			DLOG(ERROR) << __FUNCTION__ << ", CALL_TRANSFERED_NAT";
			tmp_call_object->call_state_ = WTK_CALL_STATE_ANSWER;
			switch (tmp_call_object->media_type_)
			{
				case WTK_AUDIO_MEDIA_TYPE:
				case WTK_VIDEO_MEDIA_TYPE:
				case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
				case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
					wtkMediaInstance->GetSDKEventImpl()->onCallStateEvent(tmp_call_object->callid_,tmp_call_object->call_state_,WTK_REASON_ANSWER_NAT);
				break;
				default:
				break;
			}
		}
		break;
		case CALL_TRANSFERED_P2P:
		{
			DLOG(ERROR) << __FUNCTION__ << ", CALL_TRANSFERED_P2P";
			tmp_call_object->call_state_ = WTK_CALL_STATE_ANSWER;
			switch (tmp_call_object->media_type_)
			{
				case WTK_AUDIO_MEDIA_TYPE:
				case WTK_VIDEO_MEDIA_TYPE:
				case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
				case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
					wtkMediaInstance->GetSDKEventImpl()->onCallStateEvent(tmp_call_object->callid_,tmp_call_object->call_state_,WTK_REASON_ANSWER_P2P);
				break;
				default:
				break;
			}
		}
		break;
		default:
		break;
	}
}

void WtkCallCtrl::HandleWtkCallMsgEvent(MessageEvent* msg_event)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}

	std::vector<std::string> out_array_message;
	ToolsFunction tmp_tools;
	tmp_tools.ComponentsSeparatedByString(msg_event->message, (char *)" ", out_array_message);
	if (out_array_message.size() != 2) {
		DLOG(ERROR) << __FUNCTION__ << ",ComponentsSeparatedByString return size not equal two!";
		return;
	}

	std::string msg_cmd = out_array_message.at(0);
	std::string call_id = out_array_message.at(1);
	CallObject* tmp_call_object = tmp_tools.FindObjectByCallId(all_callid_to_call_obj_dict_, call_id);
	if(tmp_call_object == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",FindObjectByCallId return null!";
		return;
	}

	char msg_buf[64] = {0};
	if(!msg_cmd.compare(VIDEO_START_CMD_REQ))
	{
		sprintf(msg_buf, "%s %s",VIDEO_START_CMD_RSP,call_id.c_str());
		wtkcall_send_text(msg_buf);
		
		wtkMediaInstance->GetSDKEventImpl()->onRemoteVideoStateEvent(tmp_call_object->callid_, WTK_REMOTE_VIDEO_START);
	}
	else if(!msg_cmd.compare(VIDEO_START_CMD_RSP))
	{
		DLOG(ERROR) << __FUNCTION__ << "PEER ALREADY SEND VIDEO_START_CMD_RSP";
	}
	else if(!msg_cmd.compare(VIDEO_STOP_CMD_REQ))
	{
		sprintf(msg_buf, "%s %s",VIDEO_STOP_CMD_RSP,call_id.c_str());
		wtkcall_send_text(msg_buf);
		
		wtkMediaInstance->GetSDKEventImpl()->onRemoteVideoStateEvent(tmp_call_object->callid_, WTK_REMOTE_VIDEO_STOP);
	}
	else if(!msg_cmd.compare(VIDEO_STOP_CMD_RSP))
	{
		DLOG(ERROR) << __FUNCTION__ << "PEER ALREADY SEND VIDEO_STOP_CMD_RSP";
	}
}

