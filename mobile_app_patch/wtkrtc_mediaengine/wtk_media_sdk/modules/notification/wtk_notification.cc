#include "wtk_notification.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_instance.h"
#include "base/logging.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/values.h"
#include <time.h>

WtkNotification::WtkNotification():
	deal_notification_thread_(NULL)
{
}
WtkNotification::~WtkNotification()
{
	if(deal_notification_thread_ != NULL) {
		deal_notification_thread_->Stop();
		delete deal_notification_thread_;
		deal_notification_thread_ = NULL;
	}
}
void WtkNotification::InitNotification()
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	if(!deal_notification_thread_)
	{
		deal_notification_thread_ = new rtc::PlatformThread(ProcessNotificationThread,this,"WTK-Notification");
		deal_notification_thread_->Start();
	}
}

bool WtkNotification::PushMessageToServer(std::string msgType,
									std::string callID,
                                    std::string srcID,
									std::string dstID,
                                    std::string isConf,
									std::string coreServerCallid,
									int causeCode,
									int callType)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return false;
	}
	if (coreServerCallid.empty())
	{
		DLOG(ERROR) << __FUNCTION__ << ",coreServerCallid is empty!";
		return false;
	}
	
	std::string json_string;

	char tmp_buf[64]={0};
	time_t tmp_seconds = time(NULL);
	sprintf(tmp_buf, "%ld", tmp_seconds);
	std::string str_time_seconds = tmp_buf;
	
	memset(tmp_buf, 0, sizeof(tmp_buf));
	sprintf(tmp_buf, "%d", causeCode);
	std::string str_cause_code = tmp_buf;
	
	memset(tmp_buf, 0, sizeof(tmp_buf));
	sprintf(tmp_buf, "%d", callType);
	std::string str_call_type = tmp_buf;
	
	base::DictionaryValue root_dict;
	root_dict.SetKey("type", base::Value(msgType));
	root_dict.SetKey("srcm", base::Value(""));
	root_dict.SetKey("src", base::Value(srcID));
	root_dict.SetKey("callid", base::Value(callID));
	root_dict.SetKey("time", base::Value(str_time_seconds));
	root_dict.SetKey("cause", base::Value(str_cause_code));
	root_dict.SetKey("calltype", base::Value(str_call_type));
	
	base::JSONWriter::Write(root_dict, &json_string);

	std::string str_cause;
	switch(causeCode)
	{
		case WTK_REASON_HANGUP_REJECT:
			str_cause = "reject";
		break;
		case WTK_REASON_HANGUP_BUSY:
			str_cause = "busy";
		break;
		case WTK_REASON_HANGUP_NO_ANSWER:
			str_cause = "timeout";
		break;
		case WTK_REASON_HANGUP_CANCEL:
			str_cause = "cancel";
		break;
		case WTK_REASON_HANGUP_ALREADY_ANSWERED:
			str_cause = "answer";
		break;
		case WTK_REASON_HANGUP_INGSMCALL:
			str_cause = "in_gsm_call";
		break;
		default:
			str_cause = "timeout";
		break;
	}
	DLOG(INFO) << __FUNCTION__ << ",json_string = " << json_string;

	wtkMediaInstance->GetSDKEventImpl()->onSendSignal((char*)dstID.c_str(),(char *)isConf.c_str(),(char *)srcID.c_str(),(char *)json_string.c_str(),(char *)msgType.c_str(),(char *)str_cause.c_str(), (char *)coreServerCallid.c_str());
	return true;
}

void WtkNotification::TryStartNotificationThread()
{
	if(deal_notification_thread_)
	{
		if (deal_notification_thread_->IsSleep() == true && deal_notification_thread_->IsRunning() == true)
		{
			deal_notification_thread_->Start();
			return;
		}
		else
		{
			if(deal_notification_thread_->IsRunning() == false)
			{
				DLOG(ERROR) << __FUNCTION__ << ", Thread Notification isn't running(should not be here!!!!)";
				deal_notification_thread_->Stop();
				deal_notification_thread_->Start();
			}
			return;
		}
		return;
	}
	else
	{
		DLOG(ERROR) << __FUNCTION__ << ", Thread Notification(is null) init failed";
		return;
	}
}

void WtkNotification::ProcessNotificationTask(void)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}

	while(received_notification_queue_.size() > 0)
	{
		std::string str_msg_json = received_notification_queue_.front();
		if(str_msg_json.length()>0)
		{
			std::unique_ptr<base::Value> root = base::JSONReader::Read(str_msg_json);
			if(root)
			{
				base::DictionaryValue* root_dict = nullptr;
				if(root->GetAsDictionary(&root_dict))
				{
					std::string str_msg_type;
					std::string str_msg_call_id;
					std::string str_msg_media_type;

					root_dict->GetString("type", &str_msg_type);
					root_dict->GetString("calltype", &str_msg_media_type);
					int msg_media_type = atoi(str_msg_media_type.c_str());
					switch(msg_media_type)
					{
						case WTK_AUDIO_MEDIA_TYPE:
						case WTK_VIDEO_MEDIA_TYPE:
						case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
						case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
						{
							root_dict->GetString("callid", &str_msg_call_id);
						}
						break;
						case WTK_AUDIO_CONF_MEDIA_TYPE:
						case WTK_VIDEO_CONF_MEDIA_TYPE:
						case WTK_AUDIO_CONF_CENTER_MEDIA_TYPE:
						case WTK_VIDEO_CONF_CENTER_MEDIA_TYPE:
						{
							root_dict->GetString("sdkConfId", &str_msg_call_id);
						}
						break;
						default:
						break;
					}
					
					if(!str_msg_type.compare(PUSH_MSG_TYPE_NCL))
					{
						ReceivedNCLMessage(str_msg_json,str_msg_call_id,msg_media_type);
					}
					else if(!str_msg_type.compare(PUSH_MSG_TYPE_RCL))
					{
						ReceivedRCLMessage(str_msg_json,str_msg_call_id,msg_media_type);
					}
					else if(!str_msg_type.compare(PUSH_MSG_TYPE_RCK))
					{
						ReceivedRCKMessage(str_msg_json,str_msg_call_id,msg_media_type);
					}
					else if(!str_msg_type.compare(PUSH_MSG_TYPE_RCA))
					{
						ReceivedRCAMessage(str_msg_json,str_msg_call_id,msg_media_type);
					}
					else if(!str_msg_type.compare(PUSH_MSG_TYPE_MLG))
					{
						ReceivedMLGMessage(str_msg_json,str_msg_call_id,msg_media_type);
					}
					else if(!str_msg_type.compare(PUSH_MSG_TYPE_RCF))
					{
						ReceivedRCFMessage(str_msg_json,str_msg_call_id,msg_media_type);
					}
				}
			}
		}
		received_notification_queue_.pop();
	}
}
bool WtkNotification::ProcessNotificationThread(void* param)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return false;
	}
	WtkNotification* notification_object = (WtkNotification*)param;

	if(notification_object->received_notification_queue_.size() > 0)
	{
		notification_object->ProcessNotificationTask();
	}
	return false;
}
void WtkNotification::ReceivedNCLMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type)
{
	DLOG(INFO) << __FUNCTION__;
	
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	if(callid_to_msg_type_[str_msg_call_id].compare(PUSH_MSG_TYPE_NCL) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", Deal repeat (NCL->NCL) return by call id = " << str_msg_call_id;
		return;
	}
	else
	{
		callid_to_msg_type_[str_msg_call_id] = PUSH_MSG_TYPE_NCL;
	}
		
	switch(msg_media_type)
	{
		case WTK_AUDIO_MEDIA_TYPE:
		case WTK_VIDEO_MEDIA_TYPE:
		case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
		case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_call_ctrl_->HandleNCLMessage(str_msg_json);
		}
		break;
		case WTK_AUDIO_CONF_MEDIA_TYPE:
		case WTK_VIDEO_CONF_MEDIA_TYPE:
		case WTK_AUDIO_CONF_CENTER_MEDIA_TYPE:
		case WTK_VIDEO_CONF_CENTER_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_conf_ctrl_->HandleNCLMessage(str_msg_json);
		}
		break;
		default:
			DLOG(ERROR) << __FUNCTION__ << ",Parse str_msg_media_type error!";
		break;
	}
}
void WtkNotification::ReceivedRCLMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type)
{
	DLOG(INFO) << __FUNCTION__;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	if(callid_to_msg_type_[str_msg_call_id].compare(PUSH_MSG_TYPE_RCL) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", Deal repeat (RCL->RCL) return by call id = " << str_msg_call_id;
		return;
	}
	else
	{
		callid_to_msg_type_[str_msg_call_id] = PUSH_MSG_TYPE_RCL;
	}

	switch(msg_media_type)
	{
		case WTK_AUDIO_MEDIA_TYPE:
		case WTK_VIDEO_MEDIA_TYPE:
		case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
		case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_call_ctrl_->HandleRCLMessage(str_msg_json);
		}
		break;
		case WTK_AUDIO_CONF_MEDIA_TYPE:
		case WTK_VIDEO_CONF_MEDIA_TYPE:
		case WTK_AUDIO_CONF_CENTER_MEDIA_TYPE:
		case WTK_VIDEO_CONF_CENTER_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_conf_ctrl_->HandleRCLMessage(str_msg_json);
		}
		break;
		default:
			DLOG(ERROR) << __FUNCTION__ << ",Parse str_msg_media_type error!";
		break;
	}
}
void WtkNotification::ReceivedRCKMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type)
{
	DLOG(INFO) << __FUNCTION__;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	if(callid_to_msg_type_[str_msg_call_id].compare(PUSH_MSG_TYPE_RCK) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", Deal repeat (RCK->RCK) return by call id = " << str_msg_call_id;
		return;
	}
	else
	{
		callid_to_msg_type_[str_msg_call_id] = PUSH_MSG_TYPE_RCK;
	}

	switch(msg_media_type)
	{
		case WTK_AUDIO_MEDIA_TYPE:
		case WTK_VIDEO_MEDIA_TYPE:
		case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
		case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_call_ctrl_->HandleRCKMessage(str_msg_json);
		}
		break;
		case WTK_AUDIO_CONF_MEDIA_TYPE:
		case WTK_VIDEO_CONF_MEDIA_TYPE:
		case WTK_AUDIO_CONF_CENTER_MEDIA_TYPE:
		case WTK_VIDEO_CONF_CENTER_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_conf_ctrl_->HandleRCKMessage(str_msg_json);
		}
		break;
		default:
			DLOG(ERROR) << __FUNCTION__ << ",Parse str_msg_media_type error!";
		break;
	}
}

void WtkNotification::ReceivedRCAMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type)
{
	DLOG(INFO) << __FUNCTION__;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	if(callid_to_msg_type_[str_msg_call_id].compare(PUSH_MSG_TYPE_RCA) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", Deal repeat (RCA->RCA) return by call id = " << str_msg_call_id;
		return;
	}
	else
	{
		callid_to_msg_type_[str_msg_call_id] = PUSH_MSG_TYPE_RCA;
	}

	switch(msg_media_type)
	{
		case WTK_AUDIO_MEDIA_TYPE:
		case WTK_VIDEO_MEDIA_TYPE:
		case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
		case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_call_ctrl_->HandleRCAMessage(str_msg_json);
		}
		break;
		case WTK_AUDIO_CONF_MEDIA_TYPE:
		case WTK_VIDEO_CONF_MEDIA_TYPE:
		case WTK_AUDIO_CONF_CENTER_MEDIA_TYPE:
		case WTK_VIDEO_CONF_CENTER_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_conf_ctrl_->HandleRCAMessage(str_msg_json);
		}
		break;
		default:
			DLOG(ERROR) << __FUNCTION__ << ",Parse str_msg_media_type error!";
		break;
	}
}

void WtkNotification::ReceivedMLGMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type)
{
	DLOG(INFO) << __FUNCTION__;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	if(callid_to_msg_type_[str_msg_call_id].compare(PUSH_MSG_TYPE_MLG) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", Deal repeat (MLG->MLG) return by call id = " << str_msg_call_id;
		return;
	}
	else
	{
		callid_to_msg_type_[str_msg_call_id] = PUSH_MSG_TYPE_MLG;
	}

	switch(msg_media_type)
	{
		case WTK_AUDIO_MEDIA_TYPE:
		case WTK_VIDEO_MEDIA_TYPE:
		case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
		case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_call_ctrl_->HandleMLGMessage(str_msg_json);
		}
		break;
		case WTK_AUDIO_CONF_MEDIA_TYPE:
		case WTK_VIDEO_CONF_MEDIA_TYPE:
		case WTK_AUDIO_CONF_CENTER_MEDIA_TYPE:
		case WTK_VIDEO_CONF_CENTER_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_conf_ctrl_->HandleMLGMessage(str_msg_json);
		}
		break;
		default:
			DLOG(ERROR) << __FUNCTION__ << ",Parse str_msg_media_type error!";
		break;
	}
}

void WtkNotification::ReceivedRCFMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type)
{
	DLOG(INFO) << __FUNCTION__;
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	
	if(callid_to_msg_type_[str_msg_call_id].compare(PUSH_MSG_TYPE_RCF) == 0)
	{
		DLOG(INFO) << __FUNCTION__ << ", Deal repeat (RCF->RCF) return by call id = " << str_msg_call_id;
		return;
	}
	else
	{
		callid_to_msg_type_[str_msg_call_id] = PUSH_MSG_TYPE_RCF;
	}

	switch(msg_media_type)
	{
		case WTK_AUDIO_MEDIA_TYPE:
		case WTK_VIDEO_MEDIA_TYPE:
		case WTK_AUDIO_OUTBOUND_MEDIA_TYPE:
		case WTK_VIDEO_OUTBOUND_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_call_ctrl_->HandleRCFMessage(str_msg_json);
		}
		break;
		case WTK_AUDIO_CONF_MEDIA_TYPE:
		case WTK_VIDEO_CONF_MEDIA_TYPE:
		case WTK_AUDIO_CONF_CENTER_MEDIA_TYPE:
		case WTK_VIDEO_CONF_CENTER_MEDIA_TYPE:
		{
			wtkMediaInstance->cur_wtk_conf_ctrl_->HandleRCFMessage(str_msg_json);
		}
		break;
		default:
			DLOG(ERROR) << __FUNCTION__ << ",Parse str_msg_media_type error!";
		break;
	}
}

