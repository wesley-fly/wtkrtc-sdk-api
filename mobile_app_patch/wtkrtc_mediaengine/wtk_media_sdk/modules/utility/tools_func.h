#ifndef _tools_func_h_
#define _tools_func_h_

#include <string>
#include <map>
#include <vector>

#include "wtkrtc_mediaengine/wtk_media_sdk/modules/call/wtk_call_ctrl.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/modules/conference/wtk_conf_ctrl.h"
#include "rtc_base/platform_thread.h"


#define CALLID_SUB_LEN			8
#define CONFID_SUB_LEN			13
#define TIMER_NAME_LEN			128
#define TIMER_THREAD_SLEEP_MS	500

typedef void (*TimerCallBack)(void *);
struct timer_param
{
    char timer_name[TIMER_NAME_LEN];
    char timer_param[TIMER_NAME_LEN];
    bool is_repeat;
    time_t interval_ms;
    TimerCallBack callback;
	time_t start_time;
	time_t next_time;
    time_t count;
};

class ToolsFunction
{
public:
	ToolsFunction();
	~ToolsFunction();
	
	void GenCurrentConfID(const char *pid_cid, char *confID);
	void GenCurrentCallID(const char *pid_cid, char *callID);
	CallObject* FindObjectByCallNo(std::map<std::string, CallObject *> & dict, int call_no);
	CallObject* FindObjectByCallId(std::map<std::string, CallObject *> & dict, std::string &callId);
	ConfObject* FindConfObjectByConfId(std::map<std::string, ConfObject *> & dict, std::string &confId);
	void ReleaseCallObjFromDict(CallObject *call_object, std::map<std::string, CallObject*> & dict);
	void ReleaseAllCallObjFromDict(std::map<std::string, CallObject*> & dict);
	void ReleaseConfObjFromDict(ConfObject *conf_object, std::map<std::string, ConfObject*> & dict);
	void ReleaseAllConfObjFromDict(std::map<std::string, ConfObject*> & dict);

	void StartTimer(const char *timer, TimerCallBack callback, void* param, int delay_ms, bool loop);
	void StopTimer(const char *timer);

	
	void ComponentsSeparatedByString(const char *str, char *substr,std::vector<std::string> & array);

private:
	rtc::CriticalSection wtk_tools_lock_;
};

class TimerManager
{
public:
    TimerManager();
    ~TimerManager();
    int AddTimer(timer_param* timer_params);
    int DeleteTimer(const char* timer_name);
    
private:
	void TryStartTimerThread();
    static bool ProcessTimerThread(void *param);
    void CountNextTime(timer_param* timer);

    bool is_start_;
    rtc::PlatformThread* timer_manager_thread_;
    std::map<std::string,timer_param*> timer_dict_;
    //lock
    rtc::CriticalSection wtk_timer_lock_;
};

#endif
