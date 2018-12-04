/*
 * Porting third party mempool functions by xiaofan.lee@wattertek.com
 */
#ifndef _critical_section_posix_h_
#define _critical_section_posix_h_

#include "critical_section_wrapper.h"

#include <pthread.h>

namespace webrtc {

class CriticalSectionPosix : public CriticalSectionWrapper {
 public:
  CriticalSectionPosix();

  virtual ~CriticalSectionPosix();

  virtual void Enter() ;
  virtual void Leave() ;

 private:
  pthread_mutex_t mutex_;
  friend class ConditionVariablePosix;
};

}  // namespace webrtc

#endif
