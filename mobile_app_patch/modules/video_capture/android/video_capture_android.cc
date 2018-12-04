/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "modules/video_capture/android/video_capture_android.h"
#include "modules/utility/include/helpers_android.h"
#include "modules/video_capture/android/device_info_android.h"

#include "rtc_base/criticalsection.h"
#include "rtc_base/logging.h"
#include "rtc_base/refcount.h"
#include "rtc_base/refcountedobject.h"
#include "rtc_base/timeutils.h"
#include "rtc_base/trace_event.h"

static JavaVM* g_jvm = NULL;
static jclass g_java_capturer_class = NULL;  // VideoCaptureAndroid.class.
static jobject g_context = NULL;  // Owned android.content.Context.

namespace webrtc {

// Called by Java to get the global application context.
jobject JNICALL GetContext(JNIEnv* env, jclass) {
  assert(g_context);
  return g_context;
}

// Called by Java when the camera has a new frame to deliver.
void JNICALL ProvideCameraFrame(
    JNIEnv* env,
    jobject,
    jbyteArray javaCameraFrame,
    jint length,
    jint rotation,
    jlong timeStamp,
    jlong context) {
  webrtc::videocapturemodule::VideoCaptureAndroid* captureModule =
      reinterpret_cast<webrtc::videocapturemodule::VideoCaptureAndroid*>(
          context);
  jbyte* cameraFrame = env->GetByteArrayElements(javaCameraFrame, NULL);
  captureModule->OnIncomingFrame(
      reinterpret_cast<uint8_t*>(cameraFrame), length, rotation, 0);
  env->ReleaseByteArrayElements(javaCameraFrame, cameraFrame, JNI_ABORT);
}

int32_t SetCaptureAndroidVM(void* javaVM, void* context) {
  if (javaVM) {
    //assert(!g_jvm);
    g_jvm = static_cast<JavaVM*> (javaVM);
    AttachThreadScoped ats(g_jvm);
    g_context = ats.env()->NewGlobalRef(static_cast<jobject>(context));

    videocapturemodule::DeviceInfoAndroid::Initialize(ats.env());

    jclass j_capture_class =
        ats.env()->FindClass("org/webrtc/videoengine/VideoCaptureAndroid");
    assert(j_capture_class);
    g_java_capturer_class =
        reinterpret_cast<jclass>(ats.env()->NewGlobalRef(j_capture_class));
    assert(g_java_capturer_class);

    JNINativeMethod native_methods[] = {
        {"GetContext",
         "()Landroid/content/Context;",
         reinterpret_cast<void*>(&GetContext)},
        {"ProvideCameraFrame",
         "([BIIJJ)V",
         reinterpret_cast<void*>(&ProvideCameraFrame)}};
    if (ats.env()->RegisterNatives(g_java_capturer_class,
                                   native_methods, 2) != 0)
      assert(false);
  } else {
    if (g_jvm) {
      AttachThreadScoped ats(g_jvm);
      ats.env()->UnregisterNatives(g_java_capturer_class);
      ats.env()->DeleteGlobalRef(g_java_capturer_class);
      g_java_capturer_class = NULL;
      ats.env()->DeleteGlobalRef(g_context);
      g_context = NULL;
      videocapturemodule::DeviceInfoAndroid::DeInitialize();
      g_jvm = NULL;
    }
  }

  return 0;
}

namespace videocapturemodule {

rtc::scoped_refptr<VideoCaptureModule> VideoCaptureImpl::Create(
    const char* deviceUniqueIdUTF8) {
  rtc::scoped_refptr<videocapturemodule::VideoCaptureAndroid> implementation(
  	new rtc::RefCountedObject<videocapturemodule::VideoCaptureAndroid>());
  if (implementation->Init(deviceUniqueIdUTF8) != 0) {
    return nullptr;
  }
  return implementation;
}

int32_t VideoCaptureAndroid::OnIncomingFrame(uint8_t* videoFrame,
                                             size_t videoFrameLength,
                                             int32_t degrees,
                                             int64_t captureTime) {
  if (!_captureStarted)
    return 0;
  VideoRotation current_rotation =
      (degrees <= 45 || degrees > 315) ? kVideoRotation_0 :
      (degrees > 45 && degrees <= 135) ? kVideoRotation_90 :
      (degrees > 135 && degrees <= 225) ? kVideoRotation_180 :
      (degrees > 225 && degrees <= 315) ? kVideoRotation_270 :
      kVideoRotation_0;  // Impossible.
  if (_rotation != current_rotation) {
    RTC_LOG(LS_INFO) << "New camera rotation: " << degrees;
    _rotation = current_rotation;
    int32_t status = VideoCaptureImpl::SetCaptureRotation(_rotation);
    if (status != 0)
      return status;
  }
  return IncomingFrame(
      videoFrame, videoFrameLength, _captureCapability, captureTime);
}

VideoCaptureAndroid::VideoCaptureAndroid()
    : VideoCaptureImpl(),
      _deviceInfo(),
      _jCapturer(NULL),
      _captureStarted(false),
      _curDeviceId(1){
}

int32_t VideoCaptureAndroid::Init(const char* deviceUniqueIdUTF8) {
  const int nameLength = strlen(deviceUniqueIdUTF8);
  if (nameLength >= kVideoCaptureUniqueNameLength)
    return -1;

  // Store the device name
  RTC_LOG(LS_INFO) << "VideoCaptureAndroid::Init: " << deviceUniqueIdUTF8;
  size_t camera_id = 0;
  if (!_deviceInfo.FindCameraIndex(deviceUniqueIdUTF8, &camera_id))
    return -1;
  _deviceUniqueId = new char[nameLength + 1];
  memcpy(_deviceUniqueId, deviceUniqueIdUTF8, nameLength + 1);

  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  jmethodID ctor = env->GetMethodID(g_java_capturer_class, "<init>", "(IJ)V");
  assert(ctor);
  jlong j_this = reinterpret_cast<intptr_t>(this);
  _jCapturer = env->NewGlobalRef(
      env->NewObject(g_java_capturer_class, ctor, camera_id, j_this));
  assert(_jCapturer);
  _rotation = kVideoRotation_0;
  return 0;
}

VideoCaptureAndroid::~VideoCaptureAndroid() {
  // Ensure Java camera is released even if our caller didn't explicitly Stop.
  if (_captureStarted)
    StopCapture();
  AttachThreadScoped ats(g_jvm);
  ats.env()->DeleteGlobalRef(_jCapturer);
}

int32_t VideoCaptureAndroid::StartCapture(
    const VideoCaptureCapability& capability) {
  rtc::CritScope cs(&_apiCs);
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();

  if (_deviceInfo.GetBestMatchedCapability(
          _deviceUniqueId, capability, _captureCapability) < 0) {

	RTC_LOG(LS_INFO) << "GetBestMatchedCapability failed:" <<__FUNCTION__<<", w = "<<capability.width<<",h="<<capability.height;
    return -1;
  }

  RTC_LOG(LS_INFO) << "GetBestMatchedCapability:" <<__FUNCTION__<<", w = "<<capability.width<<",h="<<capability.height;

  jmethodID j_start =
      env->GetMethodID(g_java_capturer_class, "startCapture", "(IIII)Z");
  assert(j_start);
  int min_mfps = 0;
  int max_mfps = 0;
  _deviceInfo.GetMFpsRange(_deviceUniqueId, _captureCapability.maxFPS,
                           &min_mfps, &max_mfps);
  bool started = env->CallBooleanMethod(_jCapturer, j_start,
                                        _captureCapability.width,
                                        _captureCapability.height,
                                        min_mfps, max_mfps);
  if (started) {
    _requestedCapability = capability;
    _captureStarted = true;
  }
  return started ? 0 : -1;
}

int32_t VideoCaptureAndroid::StopCapture() {
  _apiCs.Enter();
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();

  memset(&_requestedCapability, 0, sizeof(_requestedCapability));
  memset(&_captureCapability, 0, sizeof(_captureCapability));
  _captureStarted = false;
  // Exit critical section to avoid blocking camera thread inside
  // onIncomingFrame() call.
  _apiCs.Leave();

  jmethodID j_stop =
      env->GetMethodID(g_java_capturer_class, "stopCapture", "()Z");
  return env->CallBooleanMethod(_jCapturer, j_stop) ? 0 : -1;
}

bool VideoCaptureAndroid::CaptureStarted() {
  rtc::CritScope cs(&_apiCs);
  return _captureStarted;
}

int32_t VideoCaptureAndroid::CaptureSettings(
    VideoCaptureCapability& settings) {
  rtc::CritScope cs(&_apiCs);
  settings = _requestedCapability;
  return 0;
}

int32_t VideoCaptureAndroid::SetCaptureRotation(VideoRotation rotation) {
  int32_t status = VideoCaptureImpl::SetCaptureRotation(rotation);
  if (status != 0)
    return status;

  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();

  jmethodID j_spr =
      env->GetMethodID(g_java_capturer_class, "setPreviewRotation", "(I)V");
  assert(j_spr);
  int rotation_degrees;
  if (RotationInDegrees(rotation, &rotation_degrees) != 0) {
    assert(false);
  }
  env->CallVoidMethod(_jCapturer, j_spr, rotation_degrees);
  return 0;
}

int32_t VideoCaptureAndroid::GetCaptureDevice()
{
    return _curDeviceId;
}

int32_t VideoCaptureAndroid::SetCaptureDevice(int32_t deviceId){
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env(); 
  RTC_LOG(LS_INFO) << "SetCaptureDevice : start " << deviceId;

  jmethodID j_spr =
      env->GetMethodID(g_java_capturer_class, "SetCamera", "(IIIII)I");
  assert(j_spr);
  
  int min_mfps = 0;
  int max_mfps = 0;
  _deviceInfo.GetMFpsRange(_deviceUniqueId, _captureCapability.maxFPS,
							 &min_mfps, &max_mfps);

  env->CallIntMethod(_jCapturer, j_spr, _captureCapability.width,
                                        _captureCapability.height,
                                        min_mfps, max_mfps,deviceId);
  _curDeviceId = deviceId;
  return 0;
}

}  // namespace videocapturemodule
}  // namespace webrtc
