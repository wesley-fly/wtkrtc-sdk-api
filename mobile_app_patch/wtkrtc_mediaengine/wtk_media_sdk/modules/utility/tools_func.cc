#include "tools_func.h"
#include "base/logging.h"
#include "base/rand_util.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_instance.h"
#include "system_wrappers/include/sleep.h"
#include <time.h>

ToolsFunction::ToolsFunction()
{
}
ToolsFunction::~ToolsFunction()
{
}

void ToolsFunction::GenCurrentConfID(const char *pid_cid, char *confID)
{
	rtc::CritScope lock(&wtk_tools_lock_);
	char strPidSub[CONFID_SUB_LEN+2] = {0};
	char strTimeSeconds[32] = {0};
	char strRandom[4] = {0};
	
	strncpy(strPidSub, pid_cid, CONFID_SUB_LEN);
	time_t tmp_seconds = time(NULL);
	sprintf(strTimeSeconds, "%ld", tmp_seconds);
	int tmp_random_number = base::RandInt(0,99);
	sprintf(strRandom, "%02d", tmp_random_number);
	
	sprintf(confID, "%s-%s-%s", strPidSub, strTimeSeconds, strRandom);
	//DLOG(INFO) << __FUNCTION__ << ",strPidSub is " << strPidSub << ", confID is " << confID;
}

void ToolsFunction::GenCurrentCallID(const char *pid_cid, char *callID)
{
	rtc::CritScope lock(&wtk_tools_lock_);
	char strPidSub[CALLID_SUB_LEN+2] = {0};
	char strTimeSeconds[32] = {0};
	char strRandom[4] = {0};
	
	strncpy(strPidSub, pid_cid, CALLID_SUB_LEN);
	time_t tmp_seconds = time(NULL);
	sprintf(strTimeSeconds, "%ld", tmp_seconds);
	int tmp_random_number = base::RandInt(0,99);
	sprintf(strRandom, "%02d", tmp_random_number);
	
	sprintf(callID, "%s-%s-%s", strPidSub, strTimeSeconds, strRandom);
	//DLOG(INFO) << __FUNCTION__ << ",strPidSub is " << strPidSub << ", callID is " << callID;
}
CallObject* ToolsFunction::FindObjectByCallId(std::map<std::string, CallObject *> & dict, std::string &callId)
{
	if (callId.empty() || dict.size() ==0)
		return NULL;
	
	std::map<std::string, CallObject *>::iterator it;
	CallObject* tmp_call_object = NULL;
	for (it = dict.begin(); it != dict.end(); it++)
	{
		tmp_call_object = it->second;
		if(tmp_call_object != NULL)
		{
			if (strcmp(tmp_call_object->callid_, callId.c_str()) == 0)
				return it->second;
		}
	}
	return NULL;
}
CallObject* ToolsFunction::FindObjectByCallNo(std::map<std::string, CallObject *> & dict, int call_no)
{
	if (call_no < 0 || dict.size() ==0)
		return NULL;
	
	std::map<std::string, CallObject *>::iterator it;
	CallObject *tmp_call_object = NULL;
	for (it = dict.begin(); it != dict.end(); it++)
	{
		tmp_call_object = it->second;
		if (tmp_call_object != NULL)
		{
			if (tmp_call_object->call_no_ == call_no)
				return tmp_call_object;
		}
	}
	return NULL;
}

ConfObject* ToolsFunction::FindConfObjectByConfId(std::map<std::string, ConfObject *> & dict, std::string &confId)
{
	if (confId.empty() || dict.size() ==0)
		return NULL;
	
	std::map<std::string, ConfObject *>::iterator it;
	ConfObject* tmp_conf_object = NULL;
	for (it = dict.begin(); it != dict.end(); it++)
	{
		tmp_conf_object = it->second;
		if(tmp_conf_object != NULL)
		{
			if (strcmp(tmp_conf_object->confid_, confId.c_str()) == 0)
				return it->second;
		}
	}
	return NULL;
}
void ToolsFunction::ReleaseCallObjFromDict(CallObject *call_object, std::map<std::string, CallObject*> & dict)
{
	if(call_object == NULL || dict.size() ==0)
	{
		return;
	}

	if(strcmp(call_object->callid_, "") !=0)
	{
		dict.erase(call_object->callid_);
	}
	else
	{
		return;
	}
}
void ToolsFunction::ReleaseAllCallObjFromDict(std::map<std::string, CallObject*> & dict)
{
	if(dict.size() ==0)
	{
		return;
	}
	dict.clear();
}
void ToolsFunction::ReleaseConfObjFromDict(ConfObject *conf_object, std::map<std::string, ConfObject*> & dict)
{
	if(conf_object == NULL || dict.size() ==0)
	{
		return;
	}

	if(strcmp(conf_object->confid_, "") !=0)
	{
		dict.erase(conf_object->confid_);
	}
	else
	{
		return;
	}
}
void ToolsFunction::ReleaseAllConfObjFromDict(std::map<std::string, ConfObject*> & dict)
{
	if(dict.size() ==0)
	{
		return;
	}
	dict.clear();
}

void ToolsFunction::StartTimer(const char *timer, TimerCallBack callback, void* param, int delay_ms, bool loop)
{
    WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
    timer_param * timerParam = new timer_param;
    memset(timerParam,0,sizeof(timer_param));
    
    sprintf(timerParam->timer_name,"%s",timer);
    sprintf(timerParam->timer_param,"%s",(char *)param);
    timerParam->is_repeat = loop;
    timerParam->interval_ms = delay_ms;
    timerParam->callback = callback;

    wtkMediaInstance->cur_timer_manager_->AddTimer(timerParam);
}
void ToolsFunction::StopTimer(const char *timer)
{
    WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
    wtkMediaInstance->cur_timer_manager_->DeleteTimer(timer);
}
void ToolsFunction::ComponentsSeparatedByString(const char *str, char *substr,std::vector<std::string> & array)
{
	if (str == NULL || substr == NULL || strlen(str) == 0 || strlen(substr) == 0)
	{
		DLOG(ERROR) << __FUNCTION__ << ",params error!";
		return;
	}
	
	int strLen = (int)strlen(str);
	int substrLen = (int)strlen(substr);
	int startPos = 0;
	int endPos = 0;

	std::string s = str;
	while (endPos < strlen(str) && startPos < strlen(str))
	{
		endPos = (int)s.find(substr, startPos);
		if (endPos > 0 && endPos < strLen)
			array.push_back(s.substr(startPos, endPos - startPos));
		else
		{
			array.push_back(s.substr(startPos, strLen - startPos));
			break;
		}
		startPos = endPos + substrLen;
	}
	return;
}

TimerManager::TimerManager():
	is_start_(false),
	timer_manager_thread_(NULL)
{
	timer_manager_thread_ = new rtc::PlatformThread(ProcessTimerThread, this,"WTK-Timer");
	if(timer_manager_thread_ == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ", Create TimerManager thread failed !";
	}
	timer_manager_thread_->Start();
}
TimerManager::~TimerManager()
{
    DeleteTimer(NULL);
    if (timer_manager_thread_ != NULL)
    {
    	timer_manager_thread_->Stop();
        delete timer_manager_thread_;
        timer_manager_thread_ = NULL;
    }
}
void TimerManager::TryStartTimerThread()
{
	if(timer_manager_thread_)
	{
		if (timer_manager_thread_->IsSleep() == true && timer_manager_thread_->IsRunning() == true)
		{
			timer_manager_thread_->Start();
			return;
		}
		else
		{
			if(timer_manager_thread_->IsRunning() == false)
			{
				DLOG(ERROR) << __FUNCTION__ << ", Thread Timer thread isn't running(should not be here!!!!)";
				timer_manager_thread_->Stop();
				timer_manager_thread_->Start();
			}
			return;
		}
	}
	else
	{
		DLOG(ERROR) << __FUNCTION__ << ", Thread Timer init failed";
		return;
	}
}

int TimerManager::AddTimer(timer_param* timer_params)
{
	rtc::CritScope lock(&wtk_timer_lock_);

	timer_params->start_time = time(NULL);
	if (timer_params->interval_ms < 1000)
	{
		timer_params->next_time = timer_params->start_time + (timer_params->interval_ms + TIMER_THREAD_SLEEP_MS) / 1000;
	}
	else
	{
		timer_params->next_time = timer_params->start_time + timer_params->interval_ms/1000;
	}
	
	std::string timer_name = timer_params->timer_name;
	std::map<std::string, timer_param*>::iterator it = timer_dict_.find(timer_params->timer_name);
	if(it != timer_dict_.end())
	{
		delete it->second;
		it->second = NULL;
		timer_dict_.erase(timer_params->timer_name);
		DLOG(INFO) << __FUNCTION__ << "(" << timer_params->timer_name << ") reset[refresh] success,now timer dict size = " << timer_dict_.size();
	}

	timer_dict_[timer_params->timer_name] = timer_params;
	DLOG(INFO) << __FUNCTION__ << "(" << timer_params->timer_name << ") success,now timer dict size = " << timer_dict_.size();
	
	TryStartTimerThread();

	return 0;
}
int TimerManager::DeleteTimer(const char* timer_name)
{
	rtc::CritScope lock(&wtk_timer_lock_);
	if(strcmp(timer_name,"ALL"))
	{
		std::string tmp_timer_name = timer_name;
		if(timer_dict_.size() > 0)
		{
			std::map<std::string, timer_param*>::iterator it = timer_dict_.find(tmp_timer_name);
			if(it != timer_dict_.end())
			{
				delete it->second;
				it->second = NULL;
				timer_dict_.erase(tmp_timer_name);
				DLOG(INFO) << __FUNCTION__ << "(" << tmp_timer_name << ") deleted success,now timer dict size = " << timer_dict_.size();
			}
			else
			{
				DLOG(INFO) << __FUNCTION__ << "(" << tmp_timer_name << ") has been deleted success,now timer dict size = " << timer_dict_.size();
			}
		}
		if(timer_dict_.size() == 0)
		{
			is_start_ = false;
		}
	}
	else
	{
		if (timer_dict_.size() > 0)
		{
			std::map<std::string, timer_param*>::iterator it;
			for(it = timer_dict_.begin(); it != timer_dict_.end(); it++)
			{
				if (it->second != NULL)
				{
					delete it->second;
					it->second = NULL;
				}
			}
			timer_dict_.clear();
		}
		is_start_ = false;
		
		DLOG(INFO) << __FUNCTION__ << "(ALL<All timer>) success,now timer dict size = " << timer_dict_.size();
	}

	return 0;
}
bool TimerManager::ProcessTimerThread(void *param)
{
	TimerManager* tmp_timer_manager = (TimerManager *)param;
	if(tmp_timer_manager == NULL)
	{
		DLOG(ERROR) << __FUNCTION__ << ", tmp_timer_manager is null !";
		return false;
	}
	
	timer_param *tmp_timer_param = NULL;
	std::map<std::string, timer_param*>::iterator it;
	std::vector<std::string> tmp_delete_array;

	if(tmp_timer_manager->timer_dict_.size() > 0)
	{
		rtc::CritScope lock(&tmp_timer_manager->wtk_timer_lock_);
		for(it = tmp_timer_manager->timer_dict_.begin(); it != tmp_timer_manager->timer_dict_.end(); it++)
		{
			tmp_timer_param = it->second;
			if(tmp_timer_param != NULL)
			{
				time_t now_ms = time(NULL);
				if (now_ms == tmp_timer_param->next_time)
				{
					tmp_timer_param->callback((void *)tmp_timer_param->timer_param);
					if (tmp_timer_param->is_repeat == false)
						tmp_delete_array.push_back(std::string(tmp_timer_param->timer_name));
					else
						tmp_timer_manager->CountNextTime(tmp_timer_param);
				}
				else if( now_ms > tmp_timer_param->next_time)
				{
					tmp_timer_param->next_time = now_ms;
					tmp_timer_manager->CountNextTime(tmp_timer_param);
				}
				else
				{
				}
			}
		}
	}
	if(tmp_delete_array.size() > 0)
	{
		rtc::CritScope lock(&tmp_timer_manager->wtk_timer_lock_);
		unsigned int i;
		for(i = 0; i < tmp_delete_array.size();i++)
		{
			it = tmp_timer_manager->timer_dict_.find(tmp_delete_array.at(i));
			if (it != tmp_timer_manager->timer_dict_.end())
			{
				delete it->second;
				it->second = NULL;
				tmp_timer_manager->timer_dict_.erase(tmp_delete_array.at(i));
				DLOG(INFO) << __FUNCTION__ << ",After timeouted timer, now timer size is" << tmp_timer_manager->timer_dict_.size();
			}
		}
	}
	if(tmp_timer_manager->timer_dict_.size() == 0)
	{
		return false;
	}

	webrtc::SleepMs(TIMER_THREAD_SLEEP_MS);
    return true;
}

void TimerManager::CountNextTime(timer_param* timer)
{
	if (timer->interval_ms < 1000 && timer->count % 2 == 0)
		timer->next_time += (timer->interval_ms + TIMER_THREAD_SLEEP_MS) / 1000;
	else
		timer->next_time += timer->interval_ms/1000;

	timer->count++;
}


