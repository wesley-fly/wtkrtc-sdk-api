#include "call_object.h"
#include "base/logging.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_instance.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/utility/tools_func.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/profile/wtk_user_profile.h"
#include "system_wrappers/include/sleep.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_define.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_misc_define.h"

CallObject::CallObject():
	is_callee_(false),
	is_caller_rec_rck_(false),
	media_type_(WTK_AUDIO_MEDIA_TYPE),
	call_type_(WTK_CALL_TYPE_IDLE),
	call_state_(WTK_CALL_STATE_IDLE),
	call_rec_state_(WTK_REC_CALL_TYPE_IDLE),
	call_hangup_reason_(WTK_REASON_HANGUP_NULL),
	new_call_thread_(NULL),
	call_no_(-1),
	media_server_index_(0),
	is_auto_end_call_(false)
{
	memset(callid_, 0, sizeof(callid_));
}
CallObject::~CallObject()
{
	if(new_call_thread_ != NULL)
	{
		new_call_thread_->Stop();
		delete new_call_thread_;
		new_call_thread_ = NULL;
	}
	
}
void CallObject::ForwardNewCall()
{
	DLOG(INFO) << __FUNCTION__;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	char dest[256] = {0};
	char host[256] = {0};
	char user[256] = {0};
	char cmd[256] = {0};
	char ext[1024] = {0};
	
	WtkUserProfile *cur_user_profile = wtkMediaInstance->GetUserProfile();

	UserProInfo tmp_user_profile;
	if(cur_user_profile->FindUserProfileBySrcId(callee_dst_pid_cid_, &tmp_user_profile) != 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't FindUserProfileBySrcId !";
		return;
	}
	std::string str_password = tmp_user_profile.user_server_token_ + ":" + tmp_user_profile.user_index_;
	
	sprintf(dest, "%s", callee_src_pid_cid_.c_str());
	sprintf(user, "%s:%s", callee_dst_pid_cid_.c_str(), str_password.c_str());
	if(media_type_ == WTK_AUDIO_OUTBOUND_MEDIA_TYPE || media_type_ == WTK_VIDEO_OUTBOUND_MEDIA_TYPE)
	{
		if( outbound_ticket_ != "" && outbound_ticket_.size() <= 1020)
		{
			sprintf(ext, "%s", outbound_ticket_.c_str());
		}
		strcpy(cmd, "/nortp/forward");
	}
	else
	{
		if(is_callee_)
		{
			sprintf(host, "%s", callee_ms_info_.c_str());
			sprintf(cmd, "%s%s]%s", WTK_MEDIA_IAX_KEY, callid_, callee_dst_pid_cid_.c_str());
			wtkcall_set_caller_number(callee_dst_pid_cid_.c_str());
		}
		else
		{
		
		}
	}
	
	call_no_ = wtkcall_dial(dest, host, user, cmd, ext);
}

bool CallObject::StartNewCallThread(void *param)
{
	if(param == NULL)
	{
		DLOG(INFO) << __FUNCTION__ << ", object id is null";
		return false;
	}
	
	std::string tmp_call_id = (char *)param;

	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return false;
	}
	int timeout = 0;
	std::map<std::string, std::string> tmp_dict_result;
	ToolsFunction tmp_tools;
	CallObject *tmp_call_object = (CallObject *)tmp_tools.FindObjectByCallId(wtkMediaInstance->cur_wtk_call_ctrl_->all_callid_to_call_obj_dict_, tmp_call_id);
	if (tmp_call_object == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't FindObjectByCallId callid = " << tmp_call_id;
		return false;
	}
	
	rtc::CritScope lock(&tmp_call_object->call_object_lock_);

	WtkUserProfile *cur_user_profile = wtkMediaInstance->GetUserProfile();
	
	//for(int i = 0; i < 1; i++)

	tmp_call_object->video_face_ = cur_user_profile->vec_user_media_info_[tmp_call_object->media_server_index_].user_camera_face_;
		
	{
		if(!(cur_user_profile->user_ms_state_ & WTKMEDIA_INITIAX) && !(cur_user_profile->user_ms_state_ & WTKMEDIA_LOGINIAX))
		{
			cur_user_profile->TryLoginMediaServer(tmp_call_object->caller_src_pid_cid_, true, tmp_call_object->media_server_index_);
			
			while (!(cur_user_profile->user_ms_state_ & WTKMEDIA_LOGINIAX) && timeout < cur_user_profile->user_reg_ms_timeout_)
			{
				if (tmp_call_object->call_state_ != WTK_CALL_STATE_HANGUP)
				{
					webrtc::SleepMs(1000);
					timeout++;
				}
				else
				{
					DLOG(INFO) << __FUNCTION__ << ",Caller Hangup this call!";
					timeout = cur_user_profile->user_reg_ms_timeout_;
				}
			}
		}
		else
		{
			DLOG(ERROR) << __FUNCTION__ << ", Should not come to here, New call but already login media server!!!!";
		}
		
		if(timeout >= cur_user_profile->user_reg_ms_timeout_)
		{
			DLOG(INFO) << __FUNCTION__ << ", Register Media Server timeout or Caller hangup this call!";
			tmp_dict_result[MSG_JSON_KEY_OPER_RESULT] = ERROR_NEWCALL_REGISTER;
		}
		else
		{
			std::string call_id = tmp_call_object->callid_;
			std::string timer_name = call_id + TIMER_SEPARATOR + NEW_CALL_SETUP_TIMEOUT_TIMER;
			tmp_tools.StartTimer(timer_name.c_str(), tmp_call_object->NewCallSetupCallBack, tmp_call_object->callid_, cur_user_profile->user_reg_ms_timeout_ * 1000, false);

			switch(tmp_call_object->media_type_)
			{
				case WTK_AUDIO_MEDIA_TYPE:
				case WTK_VIDEO_MEDIA_TYPE:
				{
					tmp_dict_result = WtkSyncHttpAPI::MobileNewCallAPI(tmp_call_object->caller_dst_pid_,
																		tmp_call_object->caller_src_pid_cid_,
																		tmp_call_object->callid_,
																		tmp_call_object->media_type_,
																		tmp_call_object->media_server_index_);
					if(tmp_dict_result.size() > 1)
					{
						tmp_call_object->core_server_callid_ = tmp_dict_result[MSG_JSON_KEY_CS_CALLID];
					}
				}
				break;
				case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
				case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
				{
					tmp_dict_result = WtkSyncHttpAPI::MobileNewOutboundCall(tmp_call_object->caller_dst_pid_,
																			tmp_call_object->caller_src_pid_cid_,
																			tmp_call_object->callid_,
																			tmp_call_object->outbound_pos_info_,
																			tmp_call_object->outbound_ms_info_,
																			tmp_call_object->media_type_);
					if(tmp_dict_result.size() > 1)
					{
						tmp_call_object->core_server_callid_ = tmp_dict_result[MSG_JSON_KEY_CS_CALLID];
						if(tmp_dict_result[MSG_JSON_KEY_CS_TICKETID] != "")
						{
							tmp_call_object->outbound_ticket_ = tmp_dict_result[MSG_JSON_KEY_CS_TICKETID];
						}
					}
				}
				break;
				default:
					tmp_dict_result[MSG_JSON_KEY_OPER_RESULT] = ERROR_NEWCALL_MEDIA_TYPE;
				break;
			}
		}
	}

	tmp_call_object->FinishNewCall(tmp_dict_result);

	return false;
}

int CallObject::HangupCall(void)
{
	if(call_no_ != -1)
	{
		wtkcall_hangup(call_no_);
	}
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	ToolsFunction tmp_tools;

	if(is_callee_)
	{
		if(call_state_ == WTK_CALL_STATE_INITIATE)
		{
			call_hangup_reason_ = WTK_REASON_HANGUP_REJECT;
			call_state_ = WTK_CALL_STATE_HANGUP;
			
			CalleeDealReportAndClear();
			tmp_tools.StopTimer("ALL");
		}
	}
	else
	{
		if(call_state_ == WTK_CALL_STATE_INITIATE)
		{
			call_hangup_reason_ = WTK_REASON_HANGUP_CANCEL;
			call_state_ = WTK_CALL_STATE_HANGUP;
			
			CallerDealReportAndClear();
			tmp_tools.StopTimer("ALL");
		}
	}

	return 0;
}

void CallObject::FinishNewCall(std::map<std::string, std::string> &dictResult)
{
	DLOG(INFO) << __FUNCTION__;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	int ret_value = 0;
	ToolsFunction tmp_tools;
	
	tmp_tools.StopTimer("ALL");

	if (dictResult.size() != 0)
	{
		std::string oper_result = dictResult[MSG_JSON_KEY_OPER_RESULT];
		
		if(!oper_result.empty())
		{	
			ret_value = atoi(oper_result.c_str());
			switch(ret_value)
			{
				case 0://ERROR_NONE
				{
					if(call_state_ != WTK_CALL_STATE_HANGUP)
					{
						wtkMediaInstance->GetSDKEventImpl()->onCallStateEvent(callid_,call_state_,call_hangup_reason_);

						std::string call_id = callid_;
						std::string timer_name = call_id + TIMER_SEPARATOR + SEARCH_CALLEE_TIMEOUT_TIMER;
						switch(media_type_)
						{
							case WTK_AUDIO_MEDIA_TYPE:
							case WTK_VIDEO_MEDIA_TYPE:
							{
								tmp_tools.StartTimer(timer_name.c_str(), SearchCalleeRingCallBack, callid_, wtkMediaInstance->GetUserProfile()->user_call_timeout_ * 1000, false);
								return;
							}
							break;
							case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
							case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
							{
								tmp_tools.StartTimer(timer_name.c_str(), SearchCalleeRingCallBack, callid_, 100 * 1000, false);
								ForwardNewCall();
								return;
							}
							break;
							default:
								call_hangup_reason_ = WTK_REASON_HANGUP_UNKNOWN;
							break;
						}
					}
					else
					{
						call_hangup_reason_ = WTK_REASON_HANGUP_CANCEL;
					}
				}
				break;
				case -100://ERROR_NEWCALL_REGISTER:
				case -101://ERROR_NEWCALL_MEDIA_TYPE:
					call_hangup_reason_ = WTK_REASON_HANGUP_MS_REGISTER;
				break;
				case -200://ERROR_HTTP_PROFILE:
				case -201://ERROR_HTTP_INSTANCE:
				case -202://ERROR_HTTP_RETRY_TIMEOUT:
				case -203://ERROR_HTTP_READ_JSON:
				case -204://ERROR_RESTAPI_OTHER:
					call_hangup_reason_ = WTK_REASON_HANGUP_HTTP;
				break;
				case -300://ERROR_RESTAPI_PEEROFFLINE:
				case -301://ERROR_RESTAPI_REJECTED:
				case -302://ERROR_RESTAPI_PEERNOTFOUND:
				case -303://ERROR_RESTAPI_DENIED:
				case -304://ERROR_RESTAPI_TRANSMITTIMEOUT:
					call_hangup_reason_ = WTK_REASON_HANGUP_RESTAPI;
				break;
				default:
					call_hangup_reason_ = WTK_REASON_HANGUP_UNKNOWN;
				break;
			}
		}
		else
		{
			call_hangup_reason_ = WTK_REASON_HANGUP_UNKNOWN;
			DLOG(ERROR) << __FUNCTION__ << ",oper_result is empty";
		}
	}
	else
	{
		call_hangup_reason_ = WTK_REASON_HANGUP_UNKNOWN;
		DLOG(ERROR) << __FUNCTION__ << ",dictResult size is 0";
	}
	call_state_ = WTK_CALL_STATE_HANGUP;
	
	CallerDealReportAndClear();
	tmp_tools.StopTimer("ALL");
}
void CallObject::SearchCalleeRingCallBack(void *param)
{	
	DLOG(INFO) << __FUNCTION__;

	if (param == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",param callid = null";
		return;
	}
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	std::string tmp_call_id = (char *)param;

	ToolsFunction tmp_tools;
	CallObject* tmp_call_object = (CallObject *)tmp_tools.FindObjectByCallId(wtkMediaInstance->cur_wtk_call_ctrl_->all_callid_to_call_obj_dict_,tmp_call_id);
	if(tmp_call_object == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't FindObjectByCallId!";
		return;
	}
	tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
	tmp_call_object->call_hangup_reason_ = WTK_REASON_HANGUP_NO_ANSWER;

	tmp_call_object->CallerDealReportAndClear();
}

//Setup New Call failed -- 15 seconds -- user_reg_ms_timeout_
void CallObject::NewCallSetupCallBack(void *param)
{
	if (param == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",param callid = null";
		return;
	}
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	std::string tmp_call_id = (char *)param;
	DLOG(INFO) << __FUNCTION__ << ", New call timeout begin with call id = " << tmp_call_id;

	ToolsFunction tmp_tools;
	CallObject* tmp_call_object = (CallObject *)tmp_tools.FindObjectByCallId(wtkMediaInstance->cur_wtk_call_ctrl_->all_callid_to_call_obj_dict_,tmp_call_id);
	if(tmp_call_object == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't FindObjectByCallId!";
		return;
	}
	tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
	tmp_call_object->call_hangup_reason_ = WTK_REASON_HANGUP_TIMEOUT;
	
	tmp_call_object->CallerDealReportAndClear();
}

void CallObject::CallerDealReportAndClear()
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	wtkMediaInstance->GetUserProfile()->TryLogoutMediaServer(true);
	DLOG(INFO) << __FUNCTION__;
	if(!is_caller_rec_rck_)
	{
		switch(call_hangup_reason_)
		{
			case WTK_REASON_HANGUP_NO_ANSWER:
			case WTK_REASON_HANGUP_CANCEL:
				wtkMediaInstance->GetNotificationManager()->PushMessageToServer(PUSH_MSG_TYPE_RCL,
																				callid_,
																				caller_src_pid_cid_,
																				caller_dst_pid_,
																				"",
																				core_server_callid_,
																				call_hangup_reason_,
																				media_type_);
			break;
			default:
			break;
		}
	}
	switch(media_type_)
	{
		case WTK_AUDIO_MEDIA_TYPE:
		case WTK_VIDEO_MEDIA_TYPE:
		case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
		case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
		{
			wtkMediaInstance->GetSDKEventImpl()->onCallStateEvent(callid_, call_state_, call_hangup_reason_);
		}
		break;
		default:
		break;
	}
	
	std::string tmp_call_id = callid_;
	wtkMediaInstance->GetNotificationManager()->callid_to_msg_type_.erase(tmp_call_id);
		
	ToolsFunction tmp_tools;
	tmp_tools.ReleaseCallObjFromDict(this, wtkMediaInstance->cur_wtk_call_ctrl_->all_callid_to_call_obj_dict_);
	wtkMediaInstance->cur_wtk_call_ctrl_->cur_call_object_ = NULL;
}
void CallObject::CalleeDealReportAndClear()
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	wtkMediaInstance->GetUserProfile()->TryLogoutMediaServer(true);
	DLOG(INFO) << __FUNCTION__;
	
	switch(call_hangup_reason_)
	{
		case WTK_REASON_HANGUP_REJECT:
		case WTK_REASON_HANGUP_NO_ANSWER:
			wtkMediaInstance->GetNotificationManager()->PushMessageToServer(PUSH_MSG_TYPE_RCK,
																			callid_,
																			callee_dst_pid_cid_,
																			callee_src_pid_cid_,
																			"",
																			core_server_callid_,
																			call_hangup_reason_,
																			media_type_);
		break;
		default:
		break;
	}
	switch(media_type_)
	{
		case WTK_AUDIO_MEDIA_TYPE:
		case WTK_VIDEO_MEDIA_TYPE:
		case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
		case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
		{
			if(call_rec_state_ == WTK_REC_CALL_TYPE_MISSED_CALL)
			{
				wtkMediaInstance->GetSDKEventImpl()->onReceiveCallEvent(callid_,
																		(char*)callee_src_pid_cid_.c_str(),
																		(char*)callee_dst_pid_cid_.c_str(),
																		(char*)callee_srcm_.c_str(),
																		media_type_,
																		call_rec_state_);
			}
			if(call_hangup_reason_ != WTK_REASON_HANGUP_NO_ANSWER)
			{
				wtkMediaInstance->GetSDKEventImpl()->onCallStateEvent(callid_, call_state_, call_hangup_reason_);
			}
		}
		break;
		default:
		break;
	}

	std::string tmp_call_id = callid_;
	wtkMediaInstance->GetNotificationManager()->callid_to_msg_type_.erase(tmp_call_id);
	
	ToolsFunction tmp_tools;
	tmp_tools.ReleaseCallObjFromDict(this, wtkMediaInstance->cur_wtk_call_ctrl_->all_callid_to_call_obj_dict_);
	wtkMediaInstance->cur_wtk_call_ctrl_->cur_call_object_ = NULL;
}
//Incoming
void CallObject::TimeoutAutoEndIncomingCall(void* param)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	std::string call_id = (char *)param;
	ToolsFunction tmp_tools;
	CallObject* tmp_call_object = (CallObject *)tmp_tools.FindObjectByCallId(wtkMediaInstance->cur_wtk_call_ctrl_->all_callid_to_call_obj_dict_, call_id);
	if(tmp_call_object == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",FindObjectByCallId return null";
		return;
	}
	
	tmp_call_object->call_type_ = WTK_CALL_TYPE_MISSED;
	tmp_call_object->call_state_ = WTK_CALL_STATE_HANGUP;
	tmp_call_object->call_rec_state_ = WTK_REC_CALL_TYPE_MISSED_CALL;
	tmp_call_object->call_hangup_reason_ = WTK_REASON_HANGUP_NO_ANSWER;

	tmp_call_object->CalleeDealReportAndClear();
}
