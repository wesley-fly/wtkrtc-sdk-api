#include "wtk_ptt_ctrl.h"
#include "base/logging.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_instance.h"

WtkPttCtrl::WtkPttCtrl()
{
}
WtkPttCtrl::~WtkPttCtrl()
{
}

void WtkPttCtrl::CreateWtkPtt(const char *dstID, const char * srcID, int media, char *outCallID)
{
	rtc::CritScope lock(&wtk_ptt_lock_);
	WtkMediaInstance *wtkMediaInstance = WtkMediaInstance::SharedWtkMediaInstance();
	if(!wtkMediaInstance)
	{
		DLOG(ERROR) << __FUNCTION__ << ",Couldn't get SharedWtkMediaInstance!";
		return;
	}
}
bool WtkPttCtrl::IsExistCurrentPttObject(void)
{
	return false;
}

