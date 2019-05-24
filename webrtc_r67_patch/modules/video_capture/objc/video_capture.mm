/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif
#include "rtc_base/logging.h"
#include "modules/video_capture/objc/device_info_objc.h"
#include "modules/video_capture/objc/rtc_video_capture_objc.h"
#include "rtc_base/refcount.h"
#include "rtc_base/refcountedobject.h"
#include "rtc_base/scoped_ref_ptr.h"

using namespace webrtc;
using namespace videocapturemodule;

rtc::scoped_refptr<VideoCaptureModule> VideoCaptureImpl::Create(
    const char* deviceUniqueIdUTF8) {
  return VideoCaptureIos::Create(deviceUniqueIdUTF8);
}

VideoCaptureIos::VideoCaptureIos()
    : is_capturing_(false),
	cur_device_id_(1) {
  capability_.width = kDefaultWidth;
  capability_.height = kDefaultHeight;
  capability_.maxFPS = kDefaultFrameRate;
  capture_device_ = nil;
}

VideoCaptureIos::~VideoCaptureIos() {
  if (is_capturing_) {
    [capture_device_ stopCapture];
    capture_device_ = nil;
  }
}

rtc::scoped_refptr<VideoCaptureModule> VideoCaptureIos::Create(
    const char* deviceUniqueIdUTF8) {
  if (!deviceUniqueIdUTF8[0]) {
    return NULL;
  }

  rtc::scoped_refptr<VideoCaptureIos> capture_module(
      new rtc::RefCountedObject<VideoCaptureIos>());

  const int32_t name_length = strlen(deviceUniqueIdUTF8);
  if (name_length > kVideoCaptureUniqueNameLength)
    return nullptr;

  capture_module->_deviceUniqueId = new char[name_length + 1];
  strncpy(capture_module->_deviceUniqueId, deviceUniqueIdUTF8, name_length + 1);
  capture_module->_deviceUniqueId[name_length] = '\0';

  capture_module->capture_device_ = [[RTCVideoCaptureIosObjC alloc] initWithOwner:capture_module];
  if (!capture_module->capture_device_) {
    return nullptr;
  }

  NSString* deviceUniqueId = [[NSString alloc] initWithCString:deviceUniqueIdUTF8 encoding:NSUTF8StringEncoding];
  if (![capture_module->capture_device_ setCaptureDeviceByUniqueId:deviceUniqueId]) {
    return nullptr;
  }
  
  return capture_module;
}

int32_t VideoCaptureIos::StartCapture(const VideoCaptureCapability& capability) {
  capability_ = capability;

  if (![capture_device_ startCaptureWithCapability:capability]) {
    return -1;
  }

  is_capturing_ = true;

  return 0;
}

int32_t VideoCaptureIos::StopCapture() {
  if (![capture_device_ stopCapture]) {
    return -1;
  }

  is_capturing_ = false;
  return 0;
}

bool VideoCaptureIos::CaptureStarted() {
  return is_capturing_;
}

int32_t VideoCaptureIos::CaptureSettings(VideoCaptureCapability& settings) {
  settings = capability_;
  settings.videoType = VideoType::kNV12;
  return 0;
}

int32_t VideoCaptureIos::GetCaptureDevice()
{
    return cur_device_id_;
}

int32_t VideoCaptureIos::SetCaptureDevice(int32_t deviceId)
{
  NSArray *deviceArray = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
  int deviceCount =[deviceArray count];
  if (deviceCount == 0)
  {
	RTC_LOG(LS_ERROR) << "SetCaptureDevice :: VIDEO_CAMETA_ERROR: camera count = 0 ";
	return -1;
  }
  else if(deviceCount == 1)
  {
    deviceId = 0;
  }
  AVCaptureDevice * currentDevice = [[AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo] objectAtIndex:deviceId];
  NSString* deviceUniqueId =currentDevice.uniqueID;
  //NSString *deviceName = currentDevice.localizedName;
  AVCaptureDevicePosition position = currentDevice.position;
	
  if (![capture_device_ setCaptureDeviceByUniqueId:deviceUniqueId]) {
	return -1;
  }
  if (position == AVCaptureDevicePositionBack) {
	cur_device_id_ = 0;
  }
  else if (position == AVCaptureDevicePositionFront) {
	cur_device_id_ = 1;
  }

  return 0;
}
int32_t VideoCaptureIos::SetVideoPreviewRender(void* view)
{
	[capture_device_ setPreviewView:view];
    return 0;
}
int32_t VideoCaptureIos::RegDisplayPreviewData(void)
{
    [capture_device_ regDisplayPreviewData];
	return 0;
}
int32_t VideoCaptureIos::UnRegDisplayPreviewData(void)
{
    [capture_device_ unregDisplayPreviewData];
	return 0;
}

