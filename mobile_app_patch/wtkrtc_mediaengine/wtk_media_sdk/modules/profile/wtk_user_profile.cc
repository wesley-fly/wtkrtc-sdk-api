#include "wtk_user_profile.h"
#include "wtkrtc_mediaengine/wtk_service_client/wtkcall_lib.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_define.h"
#include "base/logging.h"
#include "base/json/json_reader.h"
#include "base/values.h"

WtkUserProfile::WtkUserProfile():
	user_ms_state_(0),
	user_reg_ms_timeout_(15),
	user_call_timeout_(45),
	user_handle_gsm_(0),
	//user_audio_mode_(0),
	user_use_https_(0),
	user_http_retry_times_(3),
	user_http_retry_wait_time_(2),
	user_audio_min_bps_(8000),
	user_audio_max_bps_(16000),
	user_audio_input_volume_level_(0),
	user_audio_output_volume_level_(0),
	user_video_auto_send_(0),
	user_video_camera_mode_(1),
	user_video_frame_width_(640),
	user_video_frame_height_(480),
	user_video_orientation_(1),
	user_video_min_bps_(64000),
	user_video_max_bps_(512000),
	user_video_send_codec_(2),
	register_id_(-1)
{
}

WtkUserProfile::~WtkUserProfile()
{
}
void WtkUserProfile::TryLoginMediaServer(std::string pid_cid, bool is_active, int media_server_index)
{
	rtc::CritScope lock(&user_profile_lock_);

	int media_refresh = 120;
	std::string media_server_port;
	std::string media_passwd_index;
	UserProInfo tmp_user_profile;
	
	if(FindUserProfileBySrcId(pid_cid, &tmp_user_profile) != 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't FindUserProfileBySrcId !";
		return;
	}
	if (tmp_user_profile.user_cs_server_.length() == 0 || vec_user_media_info_.size() == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ", CoreServer is NULL or ms info is null!!";
		return;
	}

	if(!(user_ms_state_ & WTKMEDIA_INITIAX))
	{
		if(!wtkcall_initialize_iax())
		{
			user_ms_state_ |= WTKMEDIA_INITIAX;
			DLOG(INFO) << __FUNCTION__ << ",Wtk IAX2 init success!";
			if(is_active)
			{
				if(!(user_ms_state_ & WTKMEDIA_LOGINIAX))
				{
					media_server_port = vec_user_media_info_[media_server_index].user_media_server_ + ":" + vec_user_media_info_[media_server_index].user_media_port_;
					media_passwd_index = tmp_user_profile.user_server_token_ + ":" + tmp_user_profile.user_index_;
					
					register_id_ = wtkcall_register((char*)"Wtk-Media-User", (char*)pid_cid.c_str(), (char*)media_passwd_index.c_str(), (char*)media_server_port.c_str(), media_refresh);
					DLOG(INFO) << __FUNCTION__ << ",Wtk IAX2 register_id_ = " << register_id_ << ", media_passwd = " << media_passwd_index << ",media_server_port = " << media_server_port;
				}
				else
				{
					DLOG(ERROR) << __FUNCTION__ << ",Wtk IAX2 already actived!";
				}
			}
		}
		else
		{
			DLOG(ERROR) << __FUNCTION__ << ",Wtk IAX2 init failed!";
		}
	}
}
void WtkUserProfile::TryLogoutMediaServer(bool is_shutdown)
{
	rtc::CritScope lock(&user_profile_lock_);

	DLOG(ERROR) << __FUNCTION__ << ",user_ms_state_ = " << user_ms_state_;
	
	if(user_ms_state_ & WTKMEDIA_LOGINIAX)
	{
		if(register_id_ != -1)
		{
			wtkcall_unregister(register_id_);
			register_id_ = -1;
		}
		user_ms_state_ &= ~WTKMEDIA_LOGINIAX;
	}
	if(is_shutdown && (user_ms_state_ & WTKMEDIA_INITIAX))
	{
		wtkcall_shutdown_iax();
		user_ms_state_ &= ~WTKMEDIA_INITIAX;
	}
	
}

void WtkUserProfile::SetUserProfile(std::string profile, std::string ms_info)
{
	rtc::CritScope lock(&user_profile_lock_);

	int i,json_size;
	
	std::unique_ptr<base::ListValue> profile_list = base::ListValue::From(base::JSONReader().Read(profile));
	json_size = profile_list->GetSize();
	if(json_size > 0)
	{
		vec_user_profile_.clear();
	
		for(i=0; i<json_size; i++)
		{
			base::DictionaryValue* dict = nullptr;
			profile_list->GetDictionary(i, &dict);
			
			if(dict != nullptr)
			{
				UserProInfo user_profile;
				
				dict->GetString("core_server", &user_profile.user_cs_server_);
				dict->GetString("core_port", &user_profile.user_cs_port_);
				dict->GetString("uid", &user_profile.user_uid_);
				dict->GetString("pid", &user_profile.user_pid_);
				dict->GetString("cid", &user_profile.user_cid_);
				dict->GetString("server_token", &user_profile.user_server_token_);
				dict->GetString("token", &user_profile.user_token_);
				dict->GetString("index", &user_profile.user_index_);

				vec_user_profile_.push_back(user_profile);
			}
			else
			{
				DLOG(ERROR) << __FUNCTION__ << ",GetDictionary profile Error!";
			}
		}
	}

	std::unique_ptr<base::ListValue> ms_list = base::ListValue::From(base::JSONReader().Read(ms_info));
	json_size = ms_list->GetSize();

	if(json_size > 0)
	{
		vec_user_media_info_.clear();
	
		for(i=0; i<json_size; i++)
		{
			base::DictionaryValue* dict = nullptr;
			ms_list->GetDictionary(i, &dict);
			
			if(dict != nullptr)
			{
				UserMediaInfo user_ms;
				
				dict->GetString("rs_server", &user_ms.user_media_server_);
				dict->GetString("rs_port", &user_ms.user_media_port_);
				dict->GetString("auto_send", &user_ms.user_camera_autosend_);
				dict->GetString("camera_mode", &user_ms.user_camera_face_);

				vec_user_media_info_.push_back(user_ms);
			}
			else
			{
				DLOG(ERROR) << __FUNCTION__ << ",GetDictionary media server Error!";
			}
		}
	}
}
int WtkUserProfile::FindUserProfileBySrcId(std::string srcId, UserProInfo *userProInfo)
{
	rtc::CritScope lock(&user_profile_lock_);
	
	int pos = (int)srcId.find("+");
	if(pos > 0)
	{
		std::string pid = srcId.substr(0, pos);
		std::vector<UserProInfo>::iterator item;
		for(item = vec_user_profile_.begin(); item != vec_user_profile_.end(); item++)
		{
			if(!pid.compare(item->user_pid_))
			{
				userProInfo->user_pid_ = item->user_pid_;
				userProInfo->user_cid_ = item->user_cid_;
				userProInfo->user_uid_ = item->user_uid_;

				userProInfo->user_server_token_ = item->user_server_token_;
				userProInfo->user_token_ = item->user_token_;
				userProInfo->user_index_ = item->user_index_;
				userProInfo->user_cs_server_ = item->user_cs_server_;
				userProInfo->user_cs_port_ = item->user_cs_port_;
				
				return 0;
			}
		}
	}

	return -1;
}
