/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/video_render/android/video_render_android_native_opengl2.h"
#include "rtc_base/criticalsection.h"
#include "system_wrappers/include/clock.h"
#include "api/video/i420_buffer.h"

namespace webrtc {

AndroidNativeOpenGl2Renderer::AndroidNativeOpenGl2Renderer(
    const int32_t id,
    const VideoRenderType videoRenderType,
    void* window,
    const bool fullscreen) :
    VideoRenderAndroid(id, videoRenderType, window, fullscreen),
    _javaRenderObj(NULL),
    _javaRenderClass(NULL) {
}

bool AndroidNativeOpenGl2Renderer::UseOpenGL2(void* window) {
  if (!g_jvm) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": RendererAndroid():UseOpenGL No JVM set.";
    return false;
  }
  bool isAttached = false;
  JNIEnv* env = NULL;
  if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
    // try to attach the thread and get the env
    // Attach this thread to JVM
    jint res = g_jvm->AttachCurrentThread(&env, NULL);

    // Get the JNI env for this thread
    if ((res < 0) || !env) {
	  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": RendererAndroid(): Could not attach thread to JVM";
      return false;
    }
    isAttached = true;
  }

  // get the renderer class
  jclass javaRenderClassLocal =
      env->FindClass("org/webrtc/videoengine/ViEAndroidGLES20");
  if (!javaRenderClassLocal) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not find ViEAndroidRenderer class";
    return false;
  }

  // get the method ID for UseOpenGL
  jmethodID cidUseOpenGL = env->GetStaticMethodID(javaRenderClassLocal,
                                                  "UseOpenGL2",
                                                  "(Ljava/lang/Object;)Z");
  if (cidUseOpenGL == NULL) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not get UseOpenGL ID";
    return false;
  }
  jboolean res = env->CallStaticBooleanMethod(javaRenderClassLocal,
                                              cidUseOpenGL, (jobject) window);

  // Detach this thread if it was attached
  if (isAttached) {
    if (g_jvm->DetachCurrentThread() < 0) {
	  RTC_LOG(INFO) << __FUNCTION__ << ": Could not detach thread from JVM";
    }
  }
  if(res)
    RTC_LOG(INFO) << __FUNCTION__ << ": UseOpenGL2():return is true";
  else
  	RTC_LOG(INFO) << __FUNCTION__ << ": UseOpenGL2():return is false";
  return res;
}

AndroidNativeOpenGl2Renderer::~AndroidNativeOpenGl2Renderer() {
  RTC_LOG(INFO) << __FUNCTION__ << ": AndroidNativeOpenGl2Renderer dtor";
  if (g_jvm) {
    // get the JNI env for this thread
    bool isAttached = false;
    JNIEnv* env = NULL;
    if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
      // try to attach the thread and get the env
      // Attach this thread to JVM
      jint res = g_jvm->AttachCurrentThread(&env, NULL);

      // Get the JNI env for this thread
      if ((res < 0) || !env) {
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not attach thread to JVM.";
        env = NULL;
      }
      else {
        isAttached = true;
      }
    }

    env->DeleteGlobalRef(_javaRenderObj);
    env->DeleteGlobalRef(_javaRenderClass);

    if (isAttached) {
      if (g_jvm->DetachCurrentThread() < 0) {
		RTC_LOG(INFO) << __FUNCTION__ << ": Could not detach thread from JVM";
      }
    }
  }
}

int32_t AndroidNativeOpenGl2Renderer::Init() {
  RTC_LOG(INFO) << __FUNCTION__;
  if (!g_jvm) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Not a valid Java VM pointer.";
    return -1;
  }
  if (!_ptrWindow) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": No window have been provided.";
    return -1;
  }

  // get the JNI env for this thread
  bool isAttached = false;
  JNIEnv* env = NULL;
  if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
    // try to attach the thread and get the env
    // Attach this thread to JVM
    jint res = g_jvm->AttachCurrentThread(&env, NULL);

    // Get the JNI env for this thread
    if ((res < 0) || !env) {
	  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not attach thread to JVM ";
      return -1;
    }
    isAttached = true;
  }

  // get the ViEAndroidGLES20 class
  jclass javaRenderClassLocal =
      env->FindClass("org/webrtc/videoengine/ViEAndroidGLES20");
  if (!javaRenderClassLocal) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not find ViEAndroidGLES20";
    return -1;
  }

  // create a global reference to the class (to tell JNI that
  // we are referencing it after this function has returned)
  _javaRenderClass =
      reinterpret_cast<jclass> (env->NewGlobalRef(javaRenderClassLocal));
  if (!_javaRenderClass) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not create Java SurfaceHolder class reference";
    return -1;
  }

  // Delete local class ref, we only use the global ref
  env->DeleteLocalRef(javaRenderClassLocal);

  // create a reference to the object (to tell JNI that we are referencing it
  // after this function has returned)
  _javaRenderObj = env->NewGlobalRef(_ptrWindow);
  if (!_javaRenderObj) {
    RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not create Java SurfaceRender object reference";
    return -1;
  }

  // Detach this thread if it was attached
  if (isAttached) {
    if (g_jvm->DetachCurrentThread() < 0) {
	  RTC_LOG(INFO) << __FUNCTION__ << ": Could not detach thread from JVM";
    }
  }

  RTC_LOG(INFO) << __FUNCTION__ << ": done";
  return 0;

}
AndroidStream*
AndroidNativeOpenGl2Renderer::CreateAndroidRenderChannel(
    int32_t streamId,
    int32_t zOrder,
    const float left,
    const float top,
    const float right,
    const float bottom,
    VideoRenderAndroid& renderer) {
  RTC_LOG(INFO) << __FUNCTION__ << ": CreateAndroidRenderChannel dtor";
  AndroidNativeOpenGl2Channel* stream =
      new AndroidNativeOpenGl2Channel(streamId, g_jvm, renderer,
                                      _javaRenderObj);
  if (stream && stream->Init(zOrder, left, top, right, bottom) == 0)
    return stream;
  else {
    delete stream;
  }
  return NULL;
}

AndroidNativeOpenGl2Channel::AndroidNativeOpenGl2Channel(
    uint32_t streamId,
    JavaVM* jvm,
    VideoRenderAndroid& renderer,jobject javaRenderObj):
    _bufferToRender(webrtc::VideoFrame(I420Buffer::Create(1280, 720),webrtc::kVideoRotation_0, 0)),
    _renderer(renderer), _jvm(jvm), _javaRenderObj(javaRenderObj),
    _registerNativeCID(NULL), _deRegisterNativeCID(NULL),
    _openGLRenderer(streamId) {

}
AndroidNativeOpenGl2Channel::~AndroidNativeOpenGl2Channel() {
  RTC_LOG(INFO) << __FUNCTION__ << ": AndroidNativeOpenGl2Channel dtor";
  if (_jvm) {
    // get the JNI env for this thread
    bool isAttached = false;
    JNIEnv* env = NULL;
    if (_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
      // try to attach the thread and get the env
      // Attach this thread to JVM
      jint res = _jvm->AttachCurrentThread(&env, NULL);

      // Get the JNI env for this thread
      if ((res < 0) || !env) {
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not attach thread to JVM";
        env = NULL;
      } else {
        isAttached = true;
      }
    }
    if (env && _deRegisterNativeCID) {
      env->CallVoidMethod(_javaRenderObj, _deRegisterNativeCID);
    }

    if (isAttached) {
      if (_jvm->DetachCurrentThread() < 0) {
		RTC_LOG(INFO) << __FUNCTION__ << ": Could not detach thread from JVM";
      }
    }
  }

}

int32_t AndroidNativeOpenGl2Channel::Init(int32_t zOrder,
                                          const float left,
                                          const float top,
                                          const float right,
                                          const float bottom)
{
  RTC_LOG(INFO) << __FUNCTION__ << ": AndroidNativeOpenGl2Channel";
  if (!_jvm) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Not a valid Java VM pointer";
    return -1;
  }

  // get the JNI env for this thread
  bool isAttached = false;
  JNIEnv* env = NULL;
  if (_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
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
      env->FindClass("org/webrtc/videoengine/ViEAndroidGLES20");
  if (!javaRenderClass) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not find ViESurfaceRenderer";
    return -1;
  }

  // get the method ID for the ReDraw function
  _redrawCid = env->GetMethodID(javaRenderClass, "ReDraw", "()V");
  if (_redrawCid == NULL) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not get ReDraw ID";
    return -1;
  }

  _registerNativeCID = env->GetMethodID(javaRenderClass,
                                        "RegisterNativeObject", "(J)V");
  if (_registerNativeCID == NULL) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not get RegisterNativeObject ID";
    return -1;
  }

  _deRegisterNativeCID = env->GetMethodID(javaRenderClass,
                                          "DeRegisterNativeObject", "()V");
  if (_deRegisterNativeCID == NULL) {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": could not get DeRegisterNativeObject ID";
    return -1;
  }

  JNINativeMethod nativeFunctions[2] = {
    { "DrawNative",
      "(J)V",
      (void*) &AndroidNativeOpenGl2Channel::DrawNativeStatic, },
    { "CreateOpenGLNative",
      "(JII)I",
      (void*) &AndroidNativeOpenGl2Channel::CreateOpenGLNativeStatic },
  };
  if (env->RegisterNatives(javaRenderClass, nativeFunctions, 2) == 0) {
	RTC_LOG(INFO) << __FUNCTION__ << ": Registered native functions";
  }
  else {
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Failed to register native functions";
    return -1;
  }

  env->CallVoidMethod(_javaRenderObj, _registerNativeCID, (jlong) this);

  // Detach this thread if it was attached
  if (isAttached) {
    if (_jvm->DetachCurrentThread() < 0) {
	  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not detach thread from JVM";
    }
  }

  if (_openGLRenderer.SetCoordinates(zOrder, left, top, right, bottom) != 0) {
    return -1;
  }

  RTC_LOG(INFO) << __FUNCTION__ << ": AndroidNativeOpenGl2Channel done";
  return 0;
}

void AndroidNativeOpenGl2Channel::OnFrame(const VideoFrame& videoFrame) {
  _renderCritSect.Enter();
  _bufferToRender = videoFrame;
  _renderCritSect.Leave();
  _renderer.ReDraw();
}


/*Implements AndroidStream
 * Calls the Java object and render the buffer in _bufferToRender
 */
void AndroidNativeOpenGl2Channel::DeliverFrame(JNIEnv* jniEnv) {
  //Draw the Surface
  jniEnv->CallVoidMethod(_javaRenderObj, _redrawCid);
}

/*
 * JNI callback from Java class. Called when the render
 * want to render a frame. Called from the GLRenderThread
 * Method:    DrawNative
 * Signature: (J)V
 */
void JNICALL AndroidNativeOpenGl2Channel::DrawNativeStatic(
    JNIEnv * env, jobject, jlong context) {
  AndroidNativeOpenGl2Channel* renderChannel =
      reinterpret_cast<AndroidNativeOpenGl2Channel*>(context);
  renderChannel->DrawNative();
}

void AndroidNativeOpenGl2Channel::DrawNative() {
  _renderCritSect.Enter();
  _openGLRenderer.Render(_bufferToRender);
  _renderCritSect.Leave();
}

/*
 * JNI callback from Java class. Called when the GLSurfaceview
 * have created a surface. Called from the GLRenderThread
 * Method:    CreateOpenGLNativeStatic
 * Signature: (JII)I
 */
jint JNICALL AndroidNativeOpenGl2Channel::CreateOpenGLNativeStatic(
    JNIEnv * env,
    jobject,
    jlong context,
    jint width,
    jint height) {
  AndroidNativeOpenGl2Channel* renderChannel =
      reinterpret_cast<AndroidNativeOpenGl2Channel*> (context);
  RTC_LOG(INFO) << __FUNCTION__;
  return renderChannel->CreateOpenGLNative(width, height);
}

jint AndroidNativeOpenGl2Channel::CreateOpenGLNative(
    int width, int height) {
  return _openGLRenderer.Setup(width, height);
}

}  // namespace webrtc
