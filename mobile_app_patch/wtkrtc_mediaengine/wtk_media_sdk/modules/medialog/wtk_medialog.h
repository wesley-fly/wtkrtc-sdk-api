#ifndef _wtk_media_log_h_
#define _wtk_media_log_h_
#include <string>
#include <queue>
#include <map>
#include "rtc_base/platform_thread.h"

class WtkMediaLog
{
public:
	WtkMediaLog();
	~WtkMediaLog();
	void InitMediaLog();
	void TryStartMediaLogThread();

	std::string pid_cid_;
	std::string stats_json_string_;
	std::string core_server_callid_;
	int call_duration_;
private:
	static bool ProcessMediaLogThread(void* param);
	rtc::PlatformThread* submit_call_log_thread_;
};
#endif
