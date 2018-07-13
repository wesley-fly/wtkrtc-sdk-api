#ifndef _wtk_mediaengine_jni_h
#define _wtk_mediaengine_jni_h

#include <jni.h>
#include "rtc_base/trace_event.h"
#include <android/log.h>
#include "wtk_voip_sdk_jni/wtk_service_client/wtkcall_lib.h"
#include "modules/utility/include/jvm_android.h"

#undef JNIEXPORT
#define JNIEXPORT __attribute__((visibility("default")))


#define WEBRTC_TRACE(a,b,c,...)  	__android_log_print(ANDROID_LOG_DEBUG, "***Xiaofan: WTK-WEBRTC***", __VA_ARGS__)
#ifdef __cplusplus
extern "C" {
#endif

struct WtkStateEvent
{
	jmethodID	onWtkCallEventId;
	jobject		listener;
};

JNIEXPORT JNICALL jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxInitialize(JNIEnv* env,jobject obj,jobject jlistener);
JNIEXPORT JNICALL jint Java_com_wtk_mobile_jni_WtkMediaJNI_MediaInitialize(JNIEnv* env,jobject obj);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxShutdown(JNIEnv* env,jobject obj);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_MediaShutdown(JNIEnv* env,jobject obj);
JNIEXPORT JNICALL jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxRegister(JNIEnv* env,jobject obj,jstring name, jstring number, jstring pass, jstring host, jstring port);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxUnRegister(JNIEnv *env, jobject obj,jint regId);
JNIEXPORT JNICALL jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxDial(JNIEnv *env, jobject obj,jstring dest, jstring host, jstring user, jstring cmd, jstring ext);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxAnswer(JNIEnv *env, jobject obj,jint callNo);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxHangup(JNIEnv *env, jobject obj,jint callNo);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxHold(JNIEnv *env, jobject obj,jint callNo,jint hold);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxMute(JNIEnv *env, jobject obj,jint callNo,jint mute);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxSetFormat(JNIEnv *env, jobject obj,jint callNo,int rtp_format);

#ifdef __cplusplus
}
#endif

#endif
