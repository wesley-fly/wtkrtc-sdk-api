#include "wtk_media_sdk.h"
#include "base/logging.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_instance.h"

bool WtkMediaSDK::Initialize(const char *appKey, void * sdkEventImpl)
{
	if(sdkEventImpl == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't initialize cause sdkEvent is null!";
		return false;
	}
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance)
	{
		wtkMediaInstance->InitInstance();
		wtkMediaInstance->wtk_media_sdk_event_ = (WtkMediaSDKEvent*)sdkEventImpl;
		wtkMediaInstance->cur_notification_manager_->InitNotification();
		wtkMediaInstance->cur_media_log_manager_->InitMediaLog();
		return true;
	}
	else
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return false;
	}
}
void WtkMediaSDK::SetUserProfile(const char* profile, const char* ms_info)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	if(profile == NULL || strlen(profile) == 0 || ms_info == NULL || strlen(ms_info) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ",profile or mediaserver_info string is null!";
		return;
	}
	wtkMediaInstance->wtk_user_profile_->SetUserProfile(profile, ms_info);
}
int WtkMediaSDK::SetParameter(const char *key, const char *value)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if(wtkMediaInstance->wtk_user_profile_ == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",wtkMediaInstance->wtk_user_profile_ is null!";
		return -1;
	}
	if (key == NULL || value == NULL) {
		DLOG(ERROR) << __FUNCTION__ << ",key or value string is null!";
		return -1;
	}
	//DLOG(INFO) << __FUNCTION__ << ",SetParameter key = " << key << ", value = " << value;
	if(strcmp(key, KEY_REG_MS_TIMEOUT) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_reg_ms_timeout_ = atoi(value);
	}
	else if(strcmp(key, KEY_CALL_TIMEOUT) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_call_timeout_ = atoi(value);
	}
	else if(strcmp(key, KEY_HANDLE_GSM_STATE) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_handle_gsm_ = atoi(value);
	}
	else if(strcmp(key, KEY_AUDIO_MODE) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_audio_mode_ = atoi(value);
	}
	else if(strcmp(key, KEY_USE_HTTPS) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_use_https_ = atoi(value);
	}
	else if(strcmp(key, KEY_HTTPS_CA_PATH) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_https_ca_path_ = value;
	}
	else if(strcmp(key, KEY_HTTP_RETRY_TIMES) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_http_retry_times_ = atoi(value);
	}
	else if(strcmp(key, KEY_HTTP_RETRY_WAIT_TIME) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_http_retry_wait_time_ = atoi(value);
	}
	else if(strcmp(key, KEY_AUDIO_INPUT_VOLUME_LEVEL) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_audio_input_volume_level_ = atoi(value);
	}
	else if(strcmp(key, KEY_AUDIO_OUTPUT_VOLUME_LEVEL) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_audio_output_volume_level_ = atoi(value);
	}
	else if(strcmp(key, KEY_AUTO_SEND_VIDEO) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_video_auto_send_ = atoi(value);
	}
	else if(strcmp(key, KEY_VIDEO_CAMERA_MODE) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_video_camera_mode_ = atoi(value);
	}
	else if(strcmp(key, KEY_VIDEO_FRAME_WIDTH) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_video_frame_width_ = atoi(value);
		wtkMediaInstance->wtk_user_profile_->user_audio_min_bps_ = 8*1000;
		wtkMediaInstance->wtk_user_profile_->user_audio_max_bps_ = 16*1000;
		wtkMediaInstance->wtk_user_profile_->user_video_min_bps_ = 64*1000;
		wtkMediaInstance->wtk_user_profile_->user_video_max_bps_ = 512*1000;
		/*if(atoi(value) == 176)
		{
			wtkMediaInstance->wtk_user_profile_->user_audio_min_bps_ = 8*1000;
			wtkMediaInstance->wtk_user_profile_->user_audio_max_bps_ = 16*1000;
			wtkMediaInstance->wtk_user_profile_->user_video_min_bps_ = 64*1000;
			wtkMediaInstance->wtk_user_profile_->user_video_max_bps_ = 512*1000;
		}
		else if(atoi(value) == 352)
		{
			wtkMediaInstance->wtk_user_profile_->user_audio_min_bps_ = 8*1000;
			wtkMediaInstance->wtk_user_profile_->user_audio_max_bps_ = 16*1000;
			wtkMediaInstance->wtk_user_profile_->user_video_min_bps_ = 64*1000;
			wtkMediaInstance->wtk_user_profile_->user_video_max_bps_ = 512*1000;
		}
		else if(atoi(value) == 640)
		{
			wtkMediaInstance->wtk_user_profile_->user_audio_min_bps_ = 8*1000;
			wtkMediaInstance->wtk_user_profile_->user_audio_max_bps_ = 16*1000;
			wtkMediaInstance->wtk_user_profile_->user_video_min_bps_ = 64*1000;
			wtkMediaInstance->wtk_user_profile_->user_video_max_bps_ = 512*1000;
		}
		else if(atoi(value) == 1280)
		{
			wtkMediaInstance->wtk_user_profile_->user_audio_min_bps_ = 8*1000;
			wtkMediaInstance->wtk_user_profile_->user_audio_max_bps_ = 16*1000;
			wtkMediaInstance->wtk_user_profile_->user_video_min_bps_ = 64*1000;
			wtkMediaInstance->wtk_user_profile_->user_video_max_bps_ = 512*1000;
		}*/
	}
	else if(strcmp(key, KEY_VIDEO_FRAME_HEIGHT) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_video_frame_height_ = atoi(value);
	}
	else if(strcmp(key, KEY_VIDEO_ORIENTATION) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_video_orientation_ = atoi(value);
	}
	else if(strcmp(key, KEY_VIDEO_SEND_CODEC) == 0)
	{
		wtkMediaInstance->wtk_user_profile_->user_video_send_codec_ = atoi(value);
	}
	return 0;
}
void WtkMediaSDK::GetParameter(const char *key, char *outValue)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	if(wtkMediaInstance->wtk_user_profile_ == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",wtkMediaInstance->wtk_user_profile_ is null!";
		return;
	}
	if (key == NULL || strlen(key) == 0) {
		DLOG(ERROR) << __FUNCTION__ << ",key string is null!";
		return;
	}

	if(strcmp(key, KEY_REG_MS_TIMEOUT) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_reg_ms_timeout_);
	}
	else if(strcmp(key, KEY_CALL_TIMEOUT) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_call_timeout_);
	}
	else if(strcmp(key, KEY_HANDLE_GSM_STATE) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_handle_gsm_);
	}
	else if(strcmp(key, KEY_AUDIO_MODE) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_audio_mode_);
	}
	else if(strcmp(key, KEY_USE_HTTPS) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_use_https_);
	}
	else if(strcmp(key, KEY_HTTPS_CA_PATH) == 0)
	{
		sprintf(outValue, "%s", wtkMediaInstance->wtk_user_profile_->user_https_ca_path_.c_str());
	}
	else if(strcmp(key, KEY_HTTP_RETRY_TIMES) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_http_retry_times_);
	}
	else if(strcmp(key, KEY_HTTP_RETRY_WAIT_TIME) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_http_retry_wait_time_);		
	}
	else if(strcmp(key, KEY_AUDIO_INPUT_VOLUME_LEVEL) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_audio_input_volume_level_);
	}
	else if(strcmp(key, KEY_AUDIO_OUTPUT_VOLUME_LEVEL) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_audio_output_volume_level_);
	}
	else if(strcmp(key, KEY_AUTO_SEND_VIDEO) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_video_auto_send_);
	}
	else if(strcmp(key, KEY_VIDEO_CAMERA_MODE) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_video_camera_mode_);
	}
	else if(strcmp(key, KEY_VIDEO_FRAME_WIDTH) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_video_frame_width_);
	}
	else if(strcmp(key, KEY_VIDEO_FRAME_HEIGHT) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_video_frame_height_);
	}
	else if(strcmp(key, KEY_VIDEO_ORIENTATION) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_video_orientation_);
	}
	else if(strcmp(key, KEY_VIDEO_SEND_CODEC) == 0)
	{
		sprintf(outValue, "%d", wtkMediaInstance->wtk_user_profile_->user_video_send_codec_);
	}
	//DLOG(INFO) << __FUNCTION__ << ",GetParameter key = " << key << ", value = " << outValue;
	
	return;
}
int WtkMediaSDK::ReceiveCallNotification(const char *jsonCallNotification)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (jsonCallNotification == NULL || strlen(jsonCallNotification) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", jsonCallNotification is null";
		return -1;
	}
	
	wtkMediaInstance->cur_notification_manager_->received_notification_queue_.push(jsonCallNotification);
	wtkMediaInstance->cur_notification_manager_->TryStartNotificationThread();

	return 0;
}
int WtkMediaSDK::MakeCall(const char *dstID, const char *srcID, int media, char *outCallID)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if(dstID == NULL || strlen(dstID) == 0 || srcID == NULL || strlen(srcID) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ",src id or dst id is null!";
		return -1;
	}
	wtkMediaInstance->cur_wtk_call_ctrl_->CreateWtkCall(dstID, srcID, media,outCallID);

	return 0;
}
int WtkMediaSDK::AnswerCall(const char *CallID)
{
	int res = -1;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (CallID == NULL || strlen(CallID) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	if(strlen(CallID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", AnswerCall";
		res = wtkMediaInstance->cur_wtk_call_ctrl_->AnswerCall(CallID);
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", AnswerConference";
		res = wtkMediaInstance->cur_wtk_conf_ctrl_->AnswerConference(CallID);
	}

	return res;
}
int WtkMediaSDK::HoldCall(const char *CallID, int hold_state)
{
	int res = -1;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (CallID == NULL || strlen(CallID) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	if(strlen(CallID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", HoldCall";
		res = wtkMediaInstance->cur_wtk_call_ctrl_->HoldCall(CallID, hold_state);
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", is not aviliable on conference, please use ctrlConference API!";
	}

	return res;
}
int WtkMediaSDK::MuteCall(const char *CallID, int mute_state)
{
	int res = -1;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (CallID == NULL || strlen(CallID) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	if(strlen(CallID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", MuteCall";
		res = wtkMediaInstance->cur_wtk_call_ctrl_->MuteCall(CallID, mute_state);
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", is not aviliable on conference, please use ctrlConference API!";
	}

	return res;
}
int WtkMediaSDK::HangupCall(const char *CallID)
{
	int res = -1;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (CallID == NULL || strlen(CallID) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CallId is null";
		return -1;
	}
	if(strlen(CallID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", HangupCall";
		res = wtkMediaInstance->cur_wtk_call_ctrl_->HangupCall(CallID);
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", HangupCall Conference for exit myself from conference";
		res = wtkMediaInstance->cur_wtk_conf_ctrl_->HangupCall(CallID);
	}
	return res;
}

int WtkMediaSDK::SetVideoDisplay(void *localView, void *remoteView)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (localView == NULL || remoteView == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ", localView or remoteView is null";
		return -1;
	}
	wtkMediaInstance->cur_wtk_call_ctrl_->SetVideoDisplay(localView, remoteView);

	return 0;
}
int WtkMediaSDK::StartVideoSend(const char *CallID)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if(CallID == NULL || strlen(CallID) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CallID is null";
		return -1;
	}

	if(strlen(CallID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", StartVideoSend";
		wtkMediaInstance->cur_wtk_call_ctrl_->StartVideoSend(CallID);
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", StartVideoSend for conference";
		wtkMediaInstance->cur_wtk_conf_ctrl_->StartVideoSend(CallID);
	}

	return 0;
}
int WtkMediaSDK::StopVideoSend(const char *CallID, int reason)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if(CallID == NULL || strlen(CallID) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CallID is null";
		return -1;
	}

	if(strlen(CallID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", StopVideoSend";
		wtkMediaInstance->cur_wtk_call_ctrl_->StopVideoSend(CallID, reason);
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", StopVideoSend for conference";
		wtkMediaInstance->cur_wtk_conf_ctrl_->StopVideoSend(CallID, reason);
	}

	return 0;
}
int WtkMediaSDK::SetCamera(int device_id)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}

	if(wtkMediaInstance->cur_wtk_call_ctrl_->IsExistCurrentCallObject())
	{
		wtkMediaInstance->cur_wtk_call_ctrl_->SetCamera(device_id);
	}
	else if(wtkMediaInstance->cur_wtk_conf_ctrl_->IsExistCurrentConfObject())
	{
		wtkMediaInstance->cur_wtk_conf_ctrl_->SetCamera(device_id);
	}

	return 0;
}

int WtkMediaSDK::MakeOutboundCall(const char *dstID, const char *srcID, const char *posInfo, const char *msInfo, const char *via, int media_type, char *outCallID)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (dstID == NULL || strlen(dstID) == 0 || srcID == NULL || strlen(srcID) == 0|| msInfo == NULL || strlen(msInfo) == 0)
    {
        DLOG(ERROR) << __FUNCTION__ << ",dstID or srcID or msInfo is null";
		return -1;
    }
	
	wtkMediaInstance->cur_wtk_call_ctrl_->CreateOutboundCall(dstID, srcID, posInfo, msInfo, via, media_type, outCallID);

	return 0;
}
int WtkMediaSDK::JoinConference(const char *dstIDs, const char *groupID, int media_type, const char* srcID, const char* confID, char *outConfID)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (dstIDs == NULL || strlen(dstIDs) == 0 || srcID == NULL || strlen(srcID) == 0|| groupID == NULL || strlen(groupID) == 0 || confID == NULL || strlen(confID) == 0)
    {
        DLOG(ERROR) << __FUNCTION__ << ", dstIDs or srcID or groupID or confID is null";
		return -1;
    }
	std::vector<std::string> vec_dst_ids;
	wtkMediaInstance->cur_wtk_conf_ctrl_->JoinConference(&vec_dst_ids, groupID, media_type, srcID, confID, outConfID);

	return 0;
}
int WtkMediaSDK::HangupConference(const char* ConfID)
{
	int res = -1;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (ConfID == NULL || strlen(ConfID) == 0)
    {
        DLOG(ERROR) << __FUNCTION__ << ",ConfID is null";
		return -1;
    }
	
	if(strlen(ConfID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", HangupConference not aviliable for call!";
		res = -1;
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", HangupConference";
		res = wtkMediaInstance->cur_wtk_conf_ctrl_->HangupConference(ConfID);
	}

	return res;
}
int WtkMediaSDK::ListConference(const char* ConfID)
{
	int res = -1;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (ConfID == NULL || strlen(ConfID) == 0)
    {
        DLOG(ERROR) << __FUNCTION__ << ",ConfID is null";
		return -1;
    }
	
	if(strlen(ConfID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", ListConference not aviliable for call!";
		res = -1;
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", ListConference";
		res = wtkMediaInstance->cur_wtk_conf_ctrl_->ListConference(ConfID);
	}

	return res;
}
int WtkMediaSDK::CtrlConference(const char* ConfID, int ctrl_action, const char *dstID)
{
	int res = -1;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (dstID == NULL || strlen(dstID) == 0 || ConfID == NULL || strlen(ConfID) == 0)
    {
        DLOG(ERROR) << __FUNCTION__ << ",ConfID or dstID is null";
		return -1;
    }
	
	if(strlen(ConfID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", CtrlConference not aviliable for call!";
		res = -1;
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", CtrlConference";
		res = wtkMediaInstance->cur_wtk_conf_ctrl_->CtrlConference(ConfID, ctrl_action, dstID);
	}

	return res;
}
int WtkMediaSDK::GetServerCallID(const char* callID, char *coreServerId)
{
	int res = -1;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (callID == NULL || strlen(callID) == 0)
    {
        DLOG(ERROR) << __FUNCTION__ << ",callID is null";
		return -1;
    }
	
	if(strlen(callID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", GetServerCallID";
		res = wtkMediaInstance->cur_wtk_call_ctrl_->GetCoreServerId(callID, coreServerId);
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", GetServerCallID for conference";
		res = wtkMediaInstance->cur_wtk_conf_ctrl_->GetCoreServerConfId(callID, coreServerId);
	}

	return res;
}

int WtkMediaSDK::GetCallQualityLevel(const char* callID)
{
	int res = -1;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (callID == NULL || strlen(callID) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ",callID is null";
		return -1;
	}
	
	if(strlen(callID) < CONFID_LEN)
	{
		DLOG(INFO) << __FUNCTION__ << ", GetServerCallID";
		res = wtkMediaInstance->cur_wtk_call_ctrl_->GetCallQualityLevel(callID);
	}
	else
	{
		DLOG(INFO) << __FUNCTION__ << ", GetServerCallID for conference";
		res = wtkMediaInstance->cur_wtk_conf_ctrl_->GetConfQualityLevel(callID);
	}

	return res;
}

