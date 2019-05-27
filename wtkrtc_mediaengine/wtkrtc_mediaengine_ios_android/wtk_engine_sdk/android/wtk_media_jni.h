#ifndef _wtk_media_jni_h
#define _wtk_media_jni_h

#include <jni.h>

#undef JNIEXPORT
#define JNIEXPORT __attribute__((visibility("default")))

#define LOG_TAG    "***Xiaofan: WTK-WEBRTC***"
#define JNILOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define JNILOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

struct WtkStateEvent
{
	jmethodID	onWtkCallEventId;
	jobject		listener;
};

JNIEXPORT JNICALL jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxInitialize(JNIEnv* env,jobject obj,jobject jlistener,jobject context);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxShutdown(JNIEnv* env,jobject obj);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_StartAudioPlayout(JNIEnv* env,jobject obj);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_StopAudioPlayout(JNIEnv* env,jobject obj);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_StartVideoPlayout(JNIEnv* env,jobject obj);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_StopVideoPlayout(JNIEnv* env,jobject obj);
JNIEXPORT JNICALL jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxRegister(JNIEnv* env,jobject obj,jstring name, jstring number, jstring pass, jstring host, jstring port,jint refresh);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxUnRegister(JNIEnv *env, jobject obj,jint regId);
JNIEXPORT JNICALL jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxDial(JNIEnv *env, jobject obj,jstring dest, jstring host, jstring user, jstring cmd, jstring ext);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxAnswer(JNIEnv *env, jobject obj,jint callNo);
JNIEXPORT JNICALL jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxHangup(JNIEnv *env, jobject obj,jint callNo);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxSetHold(JNIEnv *env, jobject obj,jint callNo,jint hold);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_SetVideoView(JNIEnv *env, jobject obj,jobject LocalSurfaceView,jobject RemoteSurfaceView);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_SetVideoConfView(JNIEnv *env, jobject obj,jobject SurfaceView0,jobject SurfaceView1,jobject SurfaceView2,jobject SurfaceView3);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_SwitchCamera(JNIEnv *env, jobject obj,int deviceId);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_StartCapturer(JNIEnv *env, jobject obj);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_StopCapturer(JNIEnv *env, jobject obj);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_SetCapturerRotation(JNIEnv *env, jobject obj, int rotation);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_ConfigVideoParams(JNIEnv *env, jobject obj, int codec,int width,int height,int fps,int maxqp);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_ConfigStreamBitrate(JNIEnv *env, jobject obj,int audio_min_bps,int audio_max_bps,int video_min_bps,int video_max_bps);
JNIEXPORT JNICALL jint Java_com_wtk_mobile_jni_WtkMediaJNI_CtrlConference(JNIEnv *env, jobject obj,jint callNo,jint type);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_StartVideoConf(JNIEnv* env,jobject obj,int participant_ssrc);
JNIEXPORT JNICALL void Java_com_wtk_mobile_jni_WtkMediaJNI_StopVideoConf(JNIEnv* env,jobject obj);

#ifdef __cplusplus
}
#endif

#endif
