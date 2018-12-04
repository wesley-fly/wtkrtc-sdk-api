/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/video_render/android/video_render_android_impl.h"
#include "system_wrappers/include/event_wrapper.h"

#include "rtc_base/criticalsection.h"
#include "system_wrappers/include/clock.h"


namespace webrtc {

JavaVM* VideoRenderAndroid::g_jvm = NULL;

int32_t SetRenderAndroidVM(void* javaVM) {
  RTC_LOG(INFO) << __FUNCTION__;
  VideoRenderAndroid::g_jvm = static_cast<JavaVM*> (javaVM);
  return 0;
}

VideoRenderAndroid::VideoRenderAndroid(
    const int32_t id,
    const VideoRenderType videoRenderType,
    void* window,
    const bool /*fullscreen*/):
    _renderType(videoRenderType),
    _ptrWindow((jobject)(window)),
    _javaShutDownFlag(false),
    _javaShutdownEvent(*EventWrapper::Create()),
    _javaRenderEvent(*EventWrapper::Create()),
    _lastJavaRenderEvent(0),
    _javaRenderJniEnv(NULL) {
}

VideoRenderAndroid::~VideoRenderAndroid() {

  if (_javaRenderThread)
    StopRender();

  for (AndroidStreamMap::iterator it = _streamsMap.begin();
       it != _streamsMap.end();
       ++it) {
    delete it->second;
  }
  delete &_javaShutdownEvent;
  delete &_javaRenderEvent;
  delete &_critSect;
}

int32_t VideoRenderAndroid::ChangeWindow(void* /*window*/) {
  return -1;
}

rtc::VideoSinkInterface<VideoFrame>*
VideoRenderAndroid::AddIncomingRenderStream(const uint32_t streamId,
                                            const uint32_t zOrder,
                                            const float left, const float top,
                                            const float right,
                                            const float bottom) {
  rtc::CritScope lock(&_critSect);

  AndroidStream* renderStream = NULL;
  AndroidStreamMap::iterator item = _streamsMap.find(streamId);
  if (item != _streamsMap.end() && item->second != NULL) {
	RTC_LOG(INFO) << __FUNCTION__ << ": Render stream already exists";
    return renderStream;
  }

  renderStream = CreateAndroidRenderChannel(streamId, zOrder, left, top,
                                            right, bottom, *this);
  if (renderStream) {
    _streamsMap[streamId] = renderStream;
  }
  else {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": renderStream is NULL";
    return NULL;
  }
  return renderStream;
}

int32_t VideoRenderAndroid::DeleteIncomingRenderStream(
    const uint32_t streamId) {
  rtc::CritScope lock(&_critSect);

  AndroidStreamMap::iterator item = _streamsMap.find(streamId);
  if (item == _streamsMap.end()) {
    RTC_LOG(LS_ERROR) << __FUNCTION__ << ": renderStream is NULL";
    return -1;
  }
  delete item->second;
  _streamsMap.erase(item);
  return 0;
}

int32_t VideoRenderAndroid::GetIncomingRenderStreamProperties(
    const uint32_t streamId,
    uint32_t& zOrder,
    float& left,
    float& top,
    float& right,
    float& bottom) const {
  return -1;
}

int32_t VideoRenderAndroid::StartRender() {
  rtc::CritScope lock(&_critSect);

  if (_javaRenderThread) {
    // StartRender is called when this stream should start render.
    // However StopRender is not called when the streams stop rendering.
    // Thus the the thread  is only deleted when the renderer is removed.
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Render thread already exist";
    return 0;
  }

  _javaRenderThread.reset(new rtc::PlatformThread(JavaRenderThreadFun, this,
                                                  "AndroidRenderThread"));

  _javaRenderThread->Start();
  _javaRenderThread->SetPriority(rtc::kRealtimePriority);
  return 0;
}

int32_t VideoRenderAndroid::StopRender() {
  RTC_LOG(INFO) << __FUNCTION__ << ": StopRender";
  {
    rtc::CritScope lock(&_critSect);
    if (!_javaRenderThread)
    {
      return -1;
    }
    _javaShutDownFlag = true;
    _javaRenderEvent.Set();
  }

  _javaShutdownEvent.Wait(3000);
  rtc::CritScope lock(&_critSect);
  _javaRenderThread->Stop();
  _javaRenderThread.reset();

  return 0;
}

void VideoRenderAndroid::ReDraw() {
  rtc::CritScope lock(&_critSect);
  // Allow redraw if it was more than 20ms since last.
  if (_lastJavaRenderEvent < webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds() - 20) {
    _lastJavaRenderEvent = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
    _javaRenderEvent.Set();
  }
}

bool VideoRenderAndroid::JavaRenderThreadFun(void* obj) {
  return static_cast<VideoRenderAndroid*> (obj)->JavaRenderThreadProcess();
}

bool VideoRenderAndroid::JavaRenderThreadProcess()
{
  _javaRenderEvent.Wait(1000);

  rtc::CritScope lock(&_critSect);
  if (!_javaRenderJniEnv) {
    // try to attach the thread and get the env
    // Attach this thread to JVM
    jint res = g_jvm->AttachCurrentThread(&_javaRenderJniEnv, NULL);

    // Get the JNI env for this thread
    if ((res < 0) || !_javaRenderJniEnv) {
	  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not attach thread to JVM";
      return false;
    }
  }

  for (AndroidStreamMap::iterator it = _streamsMap.begin();
       it != _streamsMap.end();
       ++it) {
    it->second->DeliverFrame(_javaRenderJniEnv);
  }

  if (_javaShutDownFlag) {
    if (g_jvm->DetachCurrentThread() < 0)
      RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not detach thread from JVM";
    else {
      RTC_LOG(INFO) << __FUNCTION__ << ": detach thread from JVM";
    }
    _javaRenderJniEnv = NULL;
    _javaShutDownFlag = false;
    _javaShutdownEvent.Set();
    return false; // Do not run this thread again.
  }
  return true;
}

VideoRenderType VideoRenderAndroid::RenderType() {
  return _renderType;
}

VideoType VideoRenderAndroid::PerferedVideoType() {
  return VideoType::kI420;
}

bool VideoRenderAndroid::FullScreen() {
  return false;
}

int32_t VideoRenderAndroid::GetGraphicsMemory(
    uint64_t& /*totalGraphicsMemory*/,
    uint64_t& /*availableGraphicsMemory*/) const {
  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": - not supported on Android";
  return -1;
}

int32_t VideoRenderAndroid::GetScreenResolution(
    uint32_t& /*screenWidth*/,
    uint32_t& /*screenHeight*/) const {
  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": - not supported on Android";
  return -1;
}

uint32_t VideoRenderAndroid::RenderFrameRate(
    const uint32_t /*streamId*/) {
  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": - not supported on Android";
  return -1;
}

int32_t VideoRenderAndroid::SetStreamCropping(
    const uint32_t /*streamId*/,
    const float /*left*/,
    const float /*top*/,
    const float /*right*/,
    const float /*bottom*/) {
  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": - not supported on Android";
  return -1;
}

int32_t VideoRenderAndroid::SetTransparentBackground(const bool enable) {
  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": - not supported on Android";
  return -1;
}

int32_t VideoRenderAndroid::ConfigureRenderer(
    const uint32_t streamId,
    const unsigned int zOrder,
    const float left,
    const float top,
    const float right,
    const float bottom) {
  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": - not supported on Android";
  return -1;
}

int32_t VideoRenderAndroid::SetText(
    const uint8_t textId,
    const uint8_t* text,
    const int32_t textLength,
    const uint32_t textColorRef,
    const uint32_t backgroundColorRef,
    const float left, const float top,
    const float rigth, const float bottom) {
  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": - not supported on Android";
  return -1;
}

int32_t VideoRenderAndroid::SetBitmap(const void* bitMap,
                                      const uint8_t pictureId,
                                      const void* colorKey,
                                      const float left, const float top,
                                      const float right,
                                      const float bottom) {
  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": - not supported on Android";
  return -1;
}

}  // namespace webrtc
