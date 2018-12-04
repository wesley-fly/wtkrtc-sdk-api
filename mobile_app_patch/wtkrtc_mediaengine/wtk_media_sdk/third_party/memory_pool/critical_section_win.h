/*
 * Porting third party mempool functions by xiaofan.lee@wattertek.com
 */

#ifndef _critical_section_win_h_
#define _critical_section_win_h_

#include <windows.h>
#include "critical_section_wrapper.h"
#include <stdint.h>


namespace webrtc {

class CriticalSectionWindows : public CriticalSectionWrapper {
 public:
  CriticalSectionWindows();

  virtual ~CriticalSectionWindows();

  virtual void Enter();
  virtual void Leave();

 private:
  CRITICAL_SECTION crit;

  friend class ConditionVariableEventWin;
  friend class ConditionVariableNativeWin;
};

}  // namespace webrtc

#endif
