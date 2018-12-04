#ifndef _conf_object_h_
#define _conf_object_h_

#include <string>
#include <vector>
#include <queue>
#include <map>
#include "rtc_base/criticalsection.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_define.h"


class ConfObject
{
public:
	ConfObject();
	~ConfObject();
	
	char confid_[MAX_CALLID_SIZE];

private:
	rtc::CriticalSection conf_object_lock_;
};

#endif
