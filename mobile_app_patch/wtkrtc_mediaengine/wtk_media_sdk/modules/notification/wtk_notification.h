#ifndef _wtk_notification_h_
#define _wtk_notification_h_
#include <string>
#include <queue>
#include <map>
#include "rtc_base/platform_thread.h"

class WtkNotification
{
public:
	WtkNotification();
	~WtkNotification();
	void InitNotification();
	bool PushMessageToServer(std::string msgType,
                             std::string callID,
                             std::string srcID,
                             std::string dstID,
                             std::string isConf,
							 std::string coreServerCallid,
                             int causeCode,
                             int callType);
	void TryStartNotificationThread();

	std::queue<std::string> received_notification_queue_;
	
	std::map<std::string, std::string> callid_to_msg_type_;
private:
	static bool ProcessNotificationThread(void* param);
	void ProcessNotificationTask(void);
	void ReceivedNCLMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type);
	void ReceivedRCLMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type);
	void ReceivedRCKMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type);
	void ReceivedRCAMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type);
	void ReceivedMLGMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type);
	void ReceivedRCFMessage(std::string str_msg_json,std::string str_msg_call_id, int msg_media_type);
	
	rtc::PlatformThread* deal_notification_thread_;
};
#endif
