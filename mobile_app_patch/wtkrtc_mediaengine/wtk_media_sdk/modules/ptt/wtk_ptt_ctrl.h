#ifndef _wtk_ptt_ctrl_h_
#define _wtk_ptt_ctrl_h_

#include "rtc_base/criticalsection.h"


class WtkPttCtrl
{
public:
	WtkPttCtrl();
	~WtkPttCtrl();
	void CreateWtkPtt(const char *dstID, const char * srcID, int media, char *outCallID);
	bool IsExistCurrentPttObject(void);

private:
	rtc::CriticalSection wtk_ptt_lock_;
};
#endif
