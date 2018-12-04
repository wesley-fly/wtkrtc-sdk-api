/*
 * Porting third party mempool functions by xiaofan.lee@wattertek.com
 */

#include "critical_section_posix.h"

namespace webrtc {

CriticalSectionPosix::CriticalSectionPosix() {
  pthread_mutexattr_t attr;
  (void) pthread_mutexattr_init(&attr);
  (void) pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  (void) pthread_mutex_init(&mutex_, &attr);
}

CriticalSectionPosix::~CriticalSectionPosix() {
  (void) pthread_mutex_destroy(&mutex_);
}

void
CriticalSectionPosix::Enter() {
  (void) pthread_mutex_lock(&mutex_);
}

void
CriticalSectionPosix::Leave() {
  (void) pthread_mutex_unlock(&mutex_);
}

}  // namespace webrtc

