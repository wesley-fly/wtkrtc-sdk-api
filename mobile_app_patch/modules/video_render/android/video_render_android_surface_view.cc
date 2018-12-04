/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "modules/video_render/android/video_render_android_surface_view.h"
#include "rtc_base/criticalsection.h"
#include "system_wrappers/include/clock.h"
#include "common_types.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"

namespace webrtc {

AndroidSurfaceViewRenderer::AndroidSurfaceViewRenderer(
    const int32_t id,
    const VideoRenderType videoRenderType,
    void* window,
    const bool fullscreen) :
    VideoRenderAndroid(id,videoRenderType,window,fullscreen),
    _javaRenderObj(NULL),
    _javaRenderClass(NULL) {
}

AndroidSurfaceViewRenderer::~AndroidSurfaceViewRenderer() {
  RTC_LOG(INFO) << __FUNCTION__ << ": ~AndroidSurfaceViewRenderer dtor";
  if(g_jvm) {
    // get the JNI env for this thread
    bool isAttached = false;
    JNIEnv* env = NULL;
    if (g_jvm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
      // try to attach the thread and get the env
      // Attach this thread to JVM
      jint res = g_jvm->AttachCurrentThread(&env, NULL);

      // Get the JNI env for this thread
      if ((res < 0) || !env) {
        RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not attach thread to JVM";
        env=NULL;
      }
      else {
        isAttached = true;
      }
    }
    env->DeleteGlobalRef(_javaRenderObj);
    env->DeleteGlobalRef(_javaRenderClass);

    if (isAttached) {
      if (g_jvm->DetachCurrentThread() < 0) {
        RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not detach thread from JVM";
      }
    }
  }
}

int32_t AndroidSurfaceViewRenderer::Init() {
  RTC_LOG(INFO) << __FUNCTION__<< ": AndroidSurfaceViewRenderer::Init";
  if (!g_jvm) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Not a valid Java VM pointer.";
    return -1;
  }
  if(!_ptrWindow) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": No window have been provided";
    return -1;
  }

  // get the JNI env for this thread
  bool isAttached = false;
  JNIEnv* env = NULL;
  if (g_jvm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
    // try to attach the thread and get the env
    // Attach this thread to JVM
    jint res = g_jvm->AttachCurrentThread(&env, NULL);

    // Get the JNI env for this thread
    if ((res < 0) || !env) {
	  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not attach thread to JVM";
      return -1;
    }
    isAttached = true;
  }

  // get the ViESurfaceRender class
  jclass javaRenderClassLocal =
      env->FindClass("org/webrtc/videoengine/ViESurfaceRenderer");
  if (!javaRenderClassLocal) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not find ViESurfaceRenderer";
    return -1;
  }

  // create a global reference to the class (to tell JNI that
  // we are referencing it after this function has returned)
  _javaRenderClass =
      reinterpret_cast<jclass>(env->NewGlobalRef(javaRenderClassLocal));
  if (!_javaRenderClass) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not create Java ViESurfaceRenderer class reference";
    return -1;
  }

  // Delete local class ref, we only use the global ref
  env->DeleteLocalRef(javaRenderClassLocal);

  // get the method ID for the constructor
  jmethodID cid = env->GetMethodID(_javaRenderClass,
                                   "<init>",
                                   "(Landroid/view/SurfaceView;)V");
  if (cid == NULL) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not get constructor ID";
    return -1; /* exception thrown */
  }

  // construct the object
  jobject javaRenderObjLocal = env->NewObject(_javaRenderClass,
                                              cid,
                                              _ptrWindow);
  if (!javaRenderObjLocal) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not create Java Render";
    return -1;
  }

  // create a reference to the object (to tell JNI that we are referencing it
  // after this function has returned)
  _javaRenderObj = env->NewGlobalRef(javaRenderObjLocal);
  if (!_javaRenderObj) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not create Java SurfaceRender object reference";
    return -1;
  }

  // Detach this thread if it was attached
  if (isAttached) {
    if (g_jvm->DetachCurrentThread() < 0) {
	  RTC_LOG(INFO) << __FUNCTION__ << ": Could not detach thread from JVM";
    }
	RTC_LOG(INFO) << __FUNCTION__ << ": isAttached is ture, detach done";
  }
  RTC_LOG(INFO) << __FUNCTION__ << ": done";
  return 0;
}

AndroidStream*
AndroidSurfaceViewRenderer::CreateAndroidRenderChannel(
    int32_t streamId,
    int32_t zOrder,
    const float left,
    const float top,
    const float right,
    const float bottom,
    VideoRenderAndroid& renderer) {
  RTC_LOG(INFO) << __FUNCTION__ << ": AndroidSurfaceViewRenderer::CreateAndroidRenderChannel";
  AndroidSurfaceViewChannel* stream =
      new AndroidSurfaceViewChannel(streamId, g_jvm, renderer, _javaRenderObj);
  if(stream && stream->Init(zOrder, left, top, right, bottom) == 0)
    return stream;
  else
    delete stream;
  return NULL;
}

AndroidSurfaceViewChannel::AndroidSurfaceViewChannel(
    uint32_t streamId,
    JavaVM* jvm,
    VideoRenderAndroid& renderer,
    jobject javaRenderObj) :
    _bufferToRender(webrtc::VideoFrame(I420Buffer::Create(1280, 720),webrtc::kVideoRotation_0, 0)),
    _renderer(renderer),
    _jvm(jvm),
    _javaRenderObj(javaRenderObj),
#ifndef ANDROID_NDK_8_OR_ABOVE
	_javaByteBufferObj(NULL),
	_directBuffer(NULL),
#endif
    _bitmapWidth(0),
    _bitmapHeight(0)
{}

AndroidSurfaceViewChannel::~AndroidSurfaceViewChannel() {
  RTC_LOG(INFO) << __FUNCTION__ << ": ~AndroidSurfaceViewChannel dtor";
  if(_jvm) {
    // get the JNI env for this thread
    bool isAttached = false;
    JNIEnv* env = NULL;
    if ( _jvm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
      // try to attach the thread and get the env
      // Attach this thread to JVM
      jint res = _jvm->AttachCurrentThread(&env, NULL);

      // Get the JNI env for this thread
      if ((res < 0) || !env) {
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not attach thread to JVM";
        env=NULL;
      }
      else {
        isAttached = true;
      }
    }

    env->DeleteGlobalRef(_javaByteBufferObj);
    if (isAttached) {
      if (_jvm->DetachCurrentThread() < 0) {
		RTC_LOG(INFO) << __FUNCTION__ << ": Could not detach thread from JVM";
      }
    }
  }
}

int32_t AndroidSurfaceViewChannel::Init(
    int32_t /*zOrder*/,
    const float left,
    const float top,
    const float right,
    const float bottom) {

  RTC_LOG(INFO) << __FUNCTION__ << ": AndroidSurfaceViewChannel::Init";
  if (!_jvm) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ":  Not a valid Java VM pointer";
    return -1;
  }

  if( (top > 1 || top < 0) ||
      (right > 1 || right < 0) ||
      (bottom > 1 || bottom < 0) ||
      (left > 1 || left < 0)) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Wrong coordinates";
    return -1;
  }

  // get the JNI env for this thread
  bool isAttached = false;
  JNIEnv* env = NULL;
  if (_jvm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
    // try to attach the thread and get the env
    // Attach this thread to JVM
    jint res = _jvm->AttachCurrentThread(&env, NULL);

    // Get the JNI env for this thread
    if ((res < 0) || !env) {
	  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not attach thread to JVM";
      return -1;
    }
    isAttached = true;
  }

  jclass javaRenderClass =
      env->FindClass("org/webrtc/videoengine/ViESurfaceRenderer");
  if (!javaRenderClass) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not find ViESurfaceRenderer";
    return -1;
  }

  // get the method ID for the CreateIntArray
  _createByteBufferCid =
      env->GetMethodID(javaRenderClass,
                       "CreateByteBuffer",
                       "(II)Ljava/nio/ByteBuffer;");
  if (_createByteBufferCid == NULL) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not get CreateByteBuffer ID";
    return -1; /* exception thrown */
  }

  // get the method ID for the DrawByteBuffer function
  _drawByteBufferCid = env->GetMethodID(javaRenderClass,
                                        "DrawByteBuffer",
                                        "()V");
  if (_drawByteBufferCid == NULL) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not get DrawByteBuffer ID";
    return -1; /* exception thrown */
  }

  // get the method ID for the SetCoordinates function
  _setCoordinatesCid = env->GetMethodID(javaRenderClass,
                                        "SetCoordinates",
                                        "(FFFF)V");
  if (_setCoordinatesCid == NULL) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ":could not get SetCoordinates ID";
    return -1; /* exception thrown */
  }

  env->CallVoidMethod(_javaRenderObj, _setCoordinatesCid,
                      left, top, right, bottom);

  // Detach this thread if it was attached
  if (isAttached) {
    if (_jvm->DetachCurrentThread() < 0) {
	  RTC_LOG(INFO) << __FUNCTION__ << ":Could not detach thread from JVM";
    }
  }

  RTC_LOG(INFO) << __FUNCTION__ << ": AndroidSurfaceViewChannel::Init done";
  return 0;
}

void AndroidSurfaceViewChannel::OnFrame(const VideoFrame& videoFrame) {
  _renderCritSect.Enter();
  _bufferToRender = videoFrame;
  _renderCritSect.Leave();
  _renderer.ReDraw();
}


/*Implements AndroidStream
 * Calls the Java object and render the buffer in _bufferToRender
 */
void AndroidSurfaceViewChannel::DeliverFrame(JNIEnv* jniEnv) {
  _renderCritSect.Enter();

  if (_bitmapWidth != _bufferToRender.width() ||_bitmapHeight != _bufferToRender.height()) {

	RTC_LOG(INFO) << __FUNCTION__ << ":  New render size(w:h):" << _bufferToRender.width() << "x" << _bufferToRender.height();
    if (_javaByteBufferObj) {
      jniEnv->DeleteGlobalRef(_javaByteBufferObj);
      _javaByteBufferObj = NULL;
      _directBuffer = NULL;
    }

    jobject javaByteBufferObj =
        jniEnv->CallObjectMethod(_javaRenderObj, _createByteBufferCid,
                                 _bufferToRender.width(),
                                 _bufferToRender.height());
    _javaByteBufferObj = jniEnv->NewGlobalRef(javaByteBufferObj);
    if (!_javaByteBufferObj) {
	  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not create Java ByteBuffer object reference";
      _renderCritSect.Leave();
      return;
    } else {
      _directBuffer = static_cast<unsigned char*>
          (jniEnv->GetDirectBufferAddress(_javaByteBufferObj));
      _bitmapWidth = _bufferToRender.width();
      _bitmapHeight = _bufferToRender.height();
    }
  }

  if(_javaByteBufferObj && _bitmapWidth && _bitmapHeight) {
    const int conversionResult =
        ConvertFromI420(_bufferToRender, VideoType::kRGB565, 0, _directBuffer);

    if (conversionResult < 0)  {
	  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Color conversion failed.";
      _renderCritSect.Leave();
      return;
    }
  }
  _renderCritSect.Leave();
  // Draw the Surface
  jniEnv->CallVoidMethod(_javaRenderObj, _drawByteBufferCid);
}

}  // namespace webrtc
