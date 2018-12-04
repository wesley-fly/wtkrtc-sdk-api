#include "wtk_media_instance.h"
#include "base/logging.h"
#include "system_wrappers/include/sleep.h"

WtkMediaInstance::WtkMediaInstance():
	cur_http_client_(NULL),
	wtk_user_profile_(NULL),
	cur_media_log_manager_(NULL),
	cur_notification_manager_(NULL),
	cur_timer_manager_(NULL),
	cur_wtk_call_ctrl_(NULL),
	cur_wtk_conf_ctrl_(NULL),
	cur_wtk_ptt_ctrl_(NULL),
	call_back_thread_(NULL),
	call_back_data_pool_(NULL),
	call_back_reg_state_pool_(NULL),
	call_back_call_state_pool_(NULL),
	call_back_msg_state_pool_(NULL),
	call_back_ptt_state_pool_(NULL),
	call_back_vad_list_pool_(NULL)
{
	webrtc::MemoryPool<CallBackData>::CreateMemoryPool(call_back_data_pool_, 32);

	webrtc::MemoryPool<RegisterEvent>::CreateMemoryPool(call_back_reg_state_pool_, 16);
	webrtc::MemoryPool<CallStateEvent>::CreateMemoryPool(call_back_call_state_pool_, 16);
	webrtc::MemoryPool<MessageEvent>::CreateMemoryPool(call_back_msg_state_pool_, 16);
	webrtc::MemoryPool<PTTEvent>::CreateMemoryPool(call_back_ptt_state_pool_, 16);
	webrtc::MemoryPool<VadListEvent>::CreateMemoryPool(call_back_vad_list_pool_, 16);
}

WtkMediaInstance::~WtkMediaInstance()
{
	if(call_back_thread_ != NULL) {
		call_back_thread_->Stop();
		delete call_back_thread_;
		call_back_thread_ = NULL;
    }

	if(cur_http_client_ != NULL)
	{
		delete cur_http_client_;
		cur_http_client_ = NULL;
	}
	if(wtk_user_profile_ != NULL)
	{
		delete wtk_user_profile_;
		wtk_user_profile_ = NULL;
	}
	if(cur_notification_manager_ != NULL)
	{
		delete cur_notification_manager_;
		cur_notification_manager_ = NULL;
	}
	if(cur_media_log_manager_ != NULL)
	{
		delete cur_media_log_manager_;
		cur_media_log_manager_ = NULL;
	}
	if(cur_timer_manager_ != NULL)
	{
		delete cur_timer_manager_;
		cur_timer_manager_ = NULL;
	}
	if(cur_wtk_call_ctrl_ != NULL)
	{
		delete cur_wtk_call_ctrl_;
		cur_wtk_call_ctrl_ = NULL;
	}
	if(cur_wtk_conf_ctrl_ != NULL)
	{
		delete cur_wtk_conf_ctrl_;
		cur_wtk_conf_ctrl_ = NULL;
	}
	if(cur_wtk_ptt_ctrl_ != NULL)
	{
		delete cur_wtk_ptt_ctrl_;
		cur_wtk_ptt_ctrl_ = NULL;
	}

	if (call_back_data_pool_ != NULL)
	{
		webrtc::MemoryPool<CallBackData>::DeleteMemoryPool(call_back_data_pool_);
		call_back_data_pool_ = NULL;
	}

	if (call_back_reg_state_pool_ != NULL)
	{
		webrtc::MemoryPool<RegisterEvent>::DeleteMemoryPool(call_back_reg_state_pool_);
		call_back_reg_state_pool_ = NULL;
	}
	if (call_back_call_state_pool_ != NULL)
	{
		webrtc::MemoryPool<CallStateEvent>::DeleteMemoryPool(call_back_call_state_pool_);
		call_back_call_state_pool_ = NULL;
	}
	if (call_back_msg_state_pool_ != NULL)
	{
		webrtc::MemoryPool<MessageEvent>::DeleteMemoryPool(call_back_msg_state_pool_);
		call_back_msg_state_pool_ = NULL;
	}
	if (call_back_ptt_state_pool_ != NULL)
	{
		webrtc::MemoryPool<PTTEvent>::DeleteMemoryPool(call_back_ptt_state_pool_);
		call_back_ptt_state_pool_ = NULL;
	}
	if (call_back_vad_list_pool_ != NULL)
	{
		webrtc::MemoryPool<VadListEvent>::DeleteMemoryPool(call_back_vad_list_pool_);
		call_back_vad_list_pool_ = NULL;
	}
}
void WtkMediaInstance::InitInstance()
{
	cur_http_client_ = new WtkSyncHttpAPI();
	wtk_user_profile_ = new WtkUserProfile();
	cur_media_log_manager_ = new WtkMediaLog();
	cur_notification_manager_ = new WtkNotification();
	cur_timer_manager_ = new TimerManager();
	cur_wtk_call_ctrl_ = new WtkCallCtrl();
	cur_wtk_conf_ctrl_ = new WtkConfCtrl();
	cur_wtk_ptt_ctrl_ = new WtkPttCtrl();
	
	wtkcall_set_jni_event_callback(GetEventDataFromIAX);
	
	if(!call_back_thread_)
	{
		call_back_thread_ = new rtc::PlatformThread(ProcessEventThread,this,"WTK-EventCB");
		call_back_thread_->Start();
	}
}
WtkUserProfile *WtkMediaInstance::GetUserProfile()
{
	return wtk_user_profile_;
}
WtkMediaSDKEvent *WtkMediaInstance::GetSDKEventImpl()
{
	return wtk_media_sdk_event_;
}
WtkNotification *WtkMediaInstance::GetNotificationManager()
{
	return cur_notification_manager_;
}
WtkMediaLog *WtkMediaInstance::GetMediaLogManager()
{
	return cur_media_log_manager_;
}

int WtkMediaInstance::GetEventDataFromIAX(int type, void* info)
{
	WtkMediaInstance* wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if (wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return -1;
	}
	if (type == WTK_EVENT_LOG)
	{
		DLOG(INFO) << "WtkMedia SDK Log:" << (char*)info;
	}
	else
	{
		RegisterEvent* reg_state_event = NULL;
		CallStateEvent* call_state_event = NULL;
		MessageEvent* msg_state_event = NULL;
		PTTEvent* ptt_state_event = NULL;
		VadListEvent* vad_list_event = NULL;
		
		CallBackData* event = NULL;
		if (wtkMediaInstance->call_back_data_pool_->PopMemory(event) != 0)
		{
			DLOG(ERROR) << __FUNCTION__ << ",Couldn't event malloc failed";
			return -1;
		}

		switch(type)
		{
			case WTK_EVENT_REG:
				if (wtkMediaInstance->call_back_reg_state_pool_->PopMemory(reg_state_event) == 0)
				{
					RegistrationInfo* reg_info = (RegistrationInfo*)info;
					
					reg_state_event->regID = reg_info->id;
					reg_state_event->regReply = reg_info->reply;

					event->type = type;
					event->info = reg_state_event;
				}
				else
				{
					DLOG(ERROR) << __FUNCTION__ << ",Couldn't reg_state_event malloc failed";
					return -1;
				}
			break;
			case WTK_EVENT_STATE:
				if (wtkMediaInstance->call_back_call_state_pool_->PopMemory(call_state_event) == 0)
				{
					CallInfo* call_info = (CallInfo*)info;
					
					call_state_event->callNo = call_info->callNo;
					sprintf(call_state_event->peer_name,"%s", call_info->peer_name);
					sprintf(call_state_event->peer_number, "%s", call_info->peer_number);
					call_state_event->activity = call_info->activity;
					call_state_event->reason = call_info->reason;
					call_state_event->start = call_info->start;
					call_state_event->duration = call_info->duration;
					call_state_event->type = call_info->type;

					event->type = type;
					event->info = call_state_event;
				}
				else
				{
					DLOG(ERROR) << __FUNCTION__ << ",Couldn't call_state_event malloc failed";
					return -1;
				}
			break;
			case WTK_EVENT_MESSAGE:
				if (wtkMediaInstance->call_back_msg_state_pool_->PopMemory(msg_state_event) == 0)
				{
					MessageInfo* msg_info = (MessageInfo*)info;
					
					msg_state_event->callNo= msg_info->callNo;
					sprintf(msg_state_event->message, "%s", msg_info->message);

					event->type = type;
					event->info = msg_state_event;
				}
				else
				{
					DLOG(ERROR) << __FUNCTION__ << ",Couldn't msg_state_event malloc failed";
					return -1;
				}
			break;
			case WTK_EVENT_PTT:
				if (wtkMediaInstance->call_back_ptt_state_pool_->PopMemory(ptt_state_event) == 0)
				{
					PttInfo* ptt_info = (PttInfo*)info;
					
					ptt_state_event->callNo= ptt_info->callNo;
					ptt_state_event->event= ptt_info->event;
					sprintf(ptt_state_event->speaker_id, "%s", ptt_info->speaker_id);
					ptt_state_event->position = ptt_info->position;

					event->type = type;
					event->info = ptt_state_event;
				}
				else
				{
					DLOG(ERROR) << __FUNCTION__ << ",Couldn't msg_state_event malloc failed";
					return -1;
				}
			break;
			case WTK_EVENT_VADLIST:
				if (wtkMediaInstance->call_back_vad_list_pool_->PopMemory(vad_list_event) == 0)
				{
					VadListInfo* vad_info = (VadListInfo*)info;
					
					vad_list_event->callNo= vad_info->callNo;
					sprintf(vad_list_event->vadlist, "%s", vad_info->vadlist);

					event->type = type;
					event->info = vad_list_event;
				}
				else
				{
					DLOG(ERROR) << __FUNCTION__ << ",Couldn't msg_state_event malloc failed";
					return -1;
				}
			break;
			default:
				DLOG(ERROR) << __FUNCTION__ << ", unknown wtk sdk event type " << type;
			break;
		}
		
		wtkMediaInstance->call_back_data_queue_.push(event);
		wtkMediaInstance->TryStartEventThread();
	}

	return 0;
}

void WtkMediaInstance::TryStartEventThread()
{
	if(call_back_thread_)
	{
		if (call_back_thread_->IsSleep() == true && call_back_thread_->IsRunning() == true)
		{
			DLOG(ERROR) << __FUNCTION__ << ",just start";
			call_back_thread_->Start();
			return;
		}
		else
		{
			if(call_back_thread_->IsRunning() == false)
			{
				DLOG(ERROR) << __FUNCTION__ << ", Thread Callback isn't running(should not be here!!!!)";
				call_back_thread_->Stop();
				call_back_thread_->Start();
			}
			return;
		}
	}
	else
	{
		DLOG(ERROR) << __FUNCTION__ << ", Thread callback init failed";
		return;
	}
}

bool WtkMediaInstance::ProcessEventThread(void *param)
{
	WtkMediaInstance *wtkMediaInstance = (WtkMediaInstance*)param;
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance from param!";
		return false;
	}

	if(wtkMediaInstance->call_back_data_queue_.size() > 0)
	{
		CallBackData *data = wtkMediaInstance->call_back_data_queue_.front();
		switch(data->type)
		{
			case EVENT_REGISTRATION:
				HandleRegistrationEvent((RegisterEvent*)data->info);
			break;
			case EVENT_STATE:
				HandleCallStateEvent((CallStateEvent*)data->info);
			break;
			case EVENT_MESSAGE:
				HandleMessageEvent((MessageEvent*)data->info);
			break;
			case EVENT_PTT:
				HandlePTTEvent((PTTEvent*)data->info);
			break;
			case EVENT_VADLIST:
				HandleVadListEvent((VadListEvent*)data->info);
			break;
			default:
				DLOG(ERROR) << __FUNCTION__ << ", can not deal this data type:" << data->type;
			break;
		}
		wtkMediaInstance->call_back_data_pool_->PushMemory(data);
		wtkMediaInstance->call_back_data_queue_.pop();
		data = NULL;

		return true;
	}
	else
	{
		webrtc::SleepMs(CALL_BACK_THREAD_SLEEP_MS);
		return true;
	}
}

void WtkMediaInstance::HandleRegistrationEvent(RegisterEvent *event)
{
	RegisterEvent *regEvent = (RegisterEvent*)event;

	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	switch (regEvent->regReply) 
	{
		case WTK_REG_ACCEPT:
			if (!(wtkMediaInstance->wtk_user_profile_->user_ms_state_ & WTKMEDIA_LOGINIAX))
			{
				wtkMediaInstance->wtk_user_profile_->user_ms_state_ |= WTKMEDIA_LOGINIAX;
				DLOG(INFO) << __FUNCTION__ << ",RegistrationEvent Login Media Server Succeeded!";
			}
			else
			{
				DLOG(INFO) << __FUNCTION__ << ",RegistrationEvent Already Login Media Server Succeeded!";
			}
		break;
		case WTK_REG_REJECT:
			if(wtkMediaInstance->cur_wtk_call_ctrl_->IsExistCurrentCallObject()||wtkMediaInstance->cur_wtk_conf_ctrl_->IsExistCurrentConfObject())
			{
			}
			else
			{
				DLOG(INFO) << __FUNCTION__ << ",RegistrationEvent Login Media Server Reject!";
				wtkMediaInstance->wtk_user_profile_->TryLogoutMediaServer(true);
			}
        break;
		case WTK_REG_TIMEOUT:
			if(wtkMediaInstance->cur_wtk_call_ctrl_->IsExistCurrentCallObject()||wtkMediaInstance->cur_wtk_conf_ctrl_->IsExistCurrentConfObject())
			{
			}
			else
			{
				DLOG(INFO) << __FUNCTION__ << ",RegistrationEvent Login Media Server Timeout!";
				wtkMediaInstance->wtk_user_profile_->TryLogoutMediaServer(true);
			}
		break;
		default:
		break;
	}

	wtkMediaInstance->call_back_reg_state_pool_->PushMemory(event);
}
void WtkMediaInstance::HandleCallStateEvent(CallStateEvent *event)
{
 	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	if(wtkMediaInstance->cur_wtk_call_ctrl_ == NULL || wtkMediaInstance->cur_wtk_conf_ctrl_ == NULL || wtkMediaInstance->cur_wtk_ptt_ctrl_ == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get call conf ptt control object!";
		return;
	}
	DLOG(ERROR) << __FUNCTION__;
	CallStateEvent* call_state_event = (CallStateEvent*)event;

	if(wtkMediaInstance->cur_wtk_call_ctrl_->IsExistCurrentCallObject())
	{
		wtkMediaInstance->cur_wtk_call_ctrl_->HandleWtkCallStateEvent(call_state_event);
	}
	else if(wtkMediaInstance->cur_wtk_conf_ctrl_->IsExistCurrentConfObject())
	{
		wtkMediaInstance->cur_wtk_conf_ctrl_->HandleWtkConfStateEvent(call_state_event);
	}
	else if(wtkMediaInstance->cur_wtk_ptt_ctrl_->IsExistCurrentPttObject())
	{
		DLOG(ERROR) << __FUNCTION__ << ", TODO For PTT";
	}

	wtkMediaInstance->call_back_call_state_pool_->PushMemory(event);
}
void WtkMediaInstance::HandleMessageEvent(MessageEvent *event)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}

	MessageEvent *message_event = (MessageEvent*)event;

	std::vector<std::string> out_array_message;
	ToolsFunction tmp_tools;
	tmp_tools.ComponentsSeparatedByString(message_event->message, (char *)" ", out_array_message);
	if (out_array_message.size() < 2) {
		DLOG(ERROR) << __FUNCTION__ << ",ComponentsSeparatedByString return error!";
		return;
	}

	std::string msg_cmd = out_array_message.at(0);
	if(msg_cmd.size() == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ",msg_cmd size is 0!";
		return;
	}

	if(!msg_cmd.compare(VIDEO_START_CMD_REQ) || !msg_cmd.compare(VIDEO_START_CMD_RSP)||
		!msg_cmd.compare(VIDEO_STOP_CMD_REQ) || !msg_cmd.compare(VIDEO_STOP_CMD_RSP))
	{
		if(wtkMediaInstance->cur_wtk_call_ctrl_->IsExistCurrentCallObject())
		{
			wtkMediaInstance->cur_wtk_call_ctrl_->HandleWtkCallMsgEvent(message_event);
		}
		else
		{
			DLOG(ERROR) << __FUNCTION__ << ",cur_wtk_call_ctrl_ not IsExistCurrentCallObject!";
		}
	}
	else if(!msg_cmd.compare(CONF_CMD_KEY_QCS) || !msg_cmd.compare(CONF_CMD_KEY_ICS)||
			!msg_cmd.compare(CONF_CMD_KEY_LST) || !msg_cmd.compare(CONF_CMD_KEY_KIC)||
			!msg_cmd.compare(CONF_CMD_KEY_JOI) || !msg_cmd.compare(CONF_CMD_KEY_MUT))
	{
		if(wtkMediaInstance->cur_wtk_conf_ctrl_->IsExistCurrentConfObject())
		{
			wtkMediaInstance->cur_wtk_conf_ctrl_->HandleWtkConfMsgEvent(message_event);
		}
		else
		{
			DLOG(ERROR) << __FUNCTION__ << ",cur_wtk_conf_ctrl_ not IsExistCurrentConfObject!";
		}
	}
	
	wtkMediaInstance->call_back_msg_state_pool_->PushMemory(event);
}

void WtkMediaInstance::HandlePTTEvent(PTTEvent *event)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	PTTEvent *ptt_event = (PTTEvent*)event;
	DLOG(ERROR) << __FUNCTION__ << ",speaker_id = " << ptt_event->speaker_id;

	wtkMediaInstance->call_back_ptt_state_pool_->PushMemory(event);
}
void WtkMediaInstance::HandleVadListEvent(VadListEvent *event)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(wtkMediaInstance == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}

	VadListEvent *vad_list_event = (VadListEvent*)event;
	DLOG(ERROR) << __FUNCTION__ << ",vadlist = " << vad_list_event->vadlist;
	
	wtkMediaInstance->call_back_vad_list_pool_->PushMemory(event);
}

WtkMediaInstance* WtkMediaInstance::SharedWtkMediaInstance()
{
	static WtkMediaInstance* sharedInstance = NULL;

	if (sharedInstance != NULL)
	{
		return sharedInstance;
	}

	sharedInstance = new WtkMediaInstance();

	return sharedInstance;
}

