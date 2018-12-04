#ifndef _wtk_user_profile_h_
#define _wtk_user_profile_h_

#include <string>
#include <vector>
#include "rtc_base/criticalsection.h"

typedef struct 
{
	std::string user_pid_;
	std::string user_cid_;
	std::string user_uid_;
	std::string user_server_token_; //For media server register
	std::string user_token_;//For Rest API 
	std::string user_index_;
	std::string user_cs_server_;
	std::string user_cs_port_;
}UserProInfo;

typedef struct
{
	std::string user_media_server_;
	std::string user_media_port_;
	std::string user_camera_autosend_;
	std::string user_camera_face_;
}UserMediaInfo;

class WtkUserProfile
{
public: 
	WtkUserProfile();
	~WtkUserProfile();

	void TryLoginMediaServer(std::string pid_cid, bool is_active, int media_server_index);
	void TryLogoutMediaServer(bool is_shutdown);
	void SetUserProfile(std::string profile, std::string ms_info);
	int FindUserProfileBySrcId(std::string srcId, UserProInfo *userProInfo);

	std::vector<UserProInfo> vec_user_profile_;
	std::vector<UserMediaInfo> vec_user_media_info_;

	//Common profile params
	int user_ms_state_;
	int user_reg_ms_timeout_;
	int user_call_timeout_;
	int user_handle_gsm_;
	int user_audio_mode_;
	int user_use_https_;
	std::string user_https_ca_path_;
	int user_http_retry_times_;
	int user_http_retry_wait_time_;
	int user_audio_min_bps_;
	int user_audio_max_bps_;
	int user_audio_input_volume_level_;
	int user_audio_output_volume_level_;
	int user_video_auto_send_;
	int user_video_camera_mode_;
	int user_video_frame_width_;
	int user_video_frame_height_;
	int user_video_orientation_;
	int user_video_min_bps_;
	int user_video_max_bps_;
	int user_video_send_codec_;
	//std::string user_ptt_server_;
	//std::string user_ptt_port_;

private:
	rtc::CriticalSection user_profile_lock_;
	int register_id_;
};
#endif
