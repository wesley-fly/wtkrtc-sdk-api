#include "wtk_medialog.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_instance.h"
#include "base/logging.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_misc_define.h"

WtkMediaLog::WtkMediaLog():
	call_duration_(-1),
	submit_call_log_thread_(NULL)
{
}
WtkMediaLog::~WtkMediaLog()
{
	if(submit_call_log_thread_ != NULL) {
		submit_call_log_thread_->Stop();
		delete submit_call_log_thread_;
		submit_call_log_thread_ = NULL;
	}
}
void WtkMediaLog::InitMediaLog()
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
	if(!submit_call_log_thread_)
	{
		submit_call_log_thread_ = new rtc::PlatformThread(ProcessMediaLogThread,this,"WTK-SubmitCallLog");
		submit_call_log_thread_->Start();
	}
}
void WtkMediaLog::TryStartMediaLogThread()
{
	if(submit_call_log_thread_)
	{
		if (submit_call_log_thread_->IsSleep() == true && submit_call_log_thread_->IsRunning() == true)
		{
			submit_call_log_thread_->Start();
			return;
		}
		else
		{
			if(submit_call_log_thread_->IsRunning() == false)
			{
				DLOG(ERROR) << __FUNCTION__ << ", Thread Submit call log isn't running(should not be here!!!!)";
				submit_call_log_thread_->Stop();
				submit_call_log_thread_->Start();
			}
			return;
		}
		return;
	}
	else
	{
		DLOG(ERROR) << __FUNCTION__ << ", Thread Submit call log(is null) init failed";
		return;
	}
}
bool WtkMediaLog::ProcessMediaLogThread(void* param)
{
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return false;
	}
	WtkMediaLog* media_log_object = (WtkMediaLog*)param;

	if(media_log_object->call_duration_ == -1)
	{
		return false;
	}
	std::map<std::string, std::string> tmp_dict_result;
	tmp_dict_result = WtkSyncHttpAPI::ReportCallStatsAPI(media_log_object->pid_cid_,media_log_object->stats_json_string_,media_log_object->core_server_callid_,media_log_object->call_duration_);
	if(tmp_dict_result.size() > 0)
	{
		int result = atoi(tmp_dict_result[MSG_JSON_KEY_OPER_RESULT].c_str());
		switch(result)
		{
			case 0:
				DLOG(INFO) << __FUNCTION__ << ", ReportCallStatsAPI:Success!";
			break;
			case -200:
			case -201:
			case -202:
			case -203:
			case -204:
				DLOG(ERROR) << __FUNCTION__ << ", ReportCallStatsAPI:JSON/HTTP ERROR!";
			break;
			case -400:
			case -401:
			case -402:
			case -403:
				DLOG(ERROR) << __FUNCTION__ << ", ReportCallStatsAPI:Denied/CallNotFound/UploadDbError/InvalidFormat ERROR!";
			break;
		}
	}
	return false;
}

