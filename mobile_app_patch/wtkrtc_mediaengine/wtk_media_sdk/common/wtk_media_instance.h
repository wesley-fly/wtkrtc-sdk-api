#ifndef _wtk_media_instance_h_
#define _wtk_media_instance_h_

#include <string>
#include <vector>
#include <queue>
#include <map>

#include "rtc_base/platform_thread.h"

#include "wtkrtc_mediaengine/wtk_service_client/wtkcall_lib.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_define.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/call/wtk_call_ctrl.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/conference/wtk_conf_ctrl.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/httpclient/wtk_sync_http_api.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/profile/wtk_user_profile.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/notification/wtk_notification.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/medialog/wtk_medialog.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/ptt/wtk_ptt_ctrl.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/utility/tools_func.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/third_party/memory_pool/memory_pool.h"

struct CallBackData
{
	int type;
	void *info;
};

class WtkMediaInstance
{
public:
	WtkMediaInstance();
	~WtkMediaInstance();
	void InitInstance();
	void TryStartEventThread();
	WtkUserProfile* GetUserProfile();
	WtkMediaSDKEvent* GetSDKEventImpl();	
	WtkNotification* GetNotificationManager();
	WtkMediaLog* GetMediaLogManager();
	static WtkMediaInstance* SharedWtkMediaInstance();
    
	WtkMediaSDKEvent* wtk_media_sdk_event_;
	WtkSyncHttpAPI* cur_http_client_;
	WtkUserProfile* wtk_user_profile_;
	WtkMediaLog* cur_media_log_manager_;
	WtkNotification* cur_notification_manager_;
	TimerManager* cur_timer_manager_;
	
	WtkCallCtrl* cur_wtk_call_ctrl_;
	//std::map<std::string, CallObject*> call_object_recycle_dict_;
	WtkConfCtrl* cur_wtk_conf_ctrl_;
	//std::map<std::string, ConfObject*> conf_object_recycle_dict_;
	WtkPttCtrl* cur_wtk_ptt_ctrl_;

private:
	static int GetEventDataFromIAX(int type, void* info);
	static bool ProcessEventThread(void *param);
	
	static void HandleRegistrationEvent(RegisterEvent *event);
	static void HandleCallStateEvent(CallStateEvent *event);
	static void HandleMessageEvent(MessageEvent *event);
	static void HandlePTTEvent(PTTEvent *event);
	static void HandleVadListEvent(VadListEvent *event);
	
	rtc::PlatformThread* 					call_back_thread_;
	
	std::queue<CallBackData*> 				call_back_data_queue_;
	
	webrtc::MemoryPool<CallBackData>* 		call_back_data_pool_;

	webrtc::MemoryPool<RegisterEvent>* 		call_back_reg_state_pool_;
	webrtc::MemoryPool<CallStateEvent>* 	call_back_call_state_pool_;
	webrtc::MemoryPool<MessageEvent>* 		call_back_msg_state_pool_;
	webrtc::MemoryPool<PTTEvent>* 			call_back_ptt_state_pool_;
	webrtc::MemoryPool<VadListEvent>* 		call_back_vad_list_pool_;
};
#endif
