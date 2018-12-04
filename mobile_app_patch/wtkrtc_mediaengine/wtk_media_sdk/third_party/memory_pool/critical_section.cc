/*
 * Porting third party mempool functions by xiaofan.lee@wattertek.com
 */


#if defined(_WIN32)
    #include <windows.h>
    #include "critical_section_win.h"
#else
    #include "critical_section_posix.h"
#endif

namespace webrtc {

CriticalSectionWrapper* CriticalSectionWrapper::CreateCriticalSection() {
#ifdef _WIN32
  return new CriticalSectionWindows();
#else
  return new CriticalSectionPosix();
#endif
}

}  // namespace webrtc

