#ifndef _wtk_media_sdk_jni_h
#define _wtk_media_sdk_jni_h

#include <jni.h>

#undef JNIEXPORT
#define JNIEXPORT __attribute__((visibility("default")))

#define REPORT_EVENT_BUF_SIZE	512
#define LOG_TAG    "[JNI: WtkMediaSDK<chromium>]"
#define JNILOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define JNILOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

struct WtkJvmContextInfo
{
	jmethodID onReceiveCallEventId;
	jmethodID onCallStateEventId;
	jmethodID onRemoteVideoStateEventId;
	jmethodID onConferenceStateEventId;
	jmethodID onConferenceVadListEventId;
	jmethodID onSendSignalId;
	
	jobject listener;
};

JNIEXPORT JNICALL jboolean 	Java_com_wattertek_jni_WtkMediaSDKJNI_Initialize(JNIEnv* env,jobject obj,jobject context,jstring appkey,jobject jlistener);
JNIEXPORT JNICALL void 		Java_com_wattertek_jni_WtkMediaSDKJNI_SetUserProfile(JNIEnv *env,jobject obj,jstring profile,jstring listRS);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_SetParameter(JNIEnv *env,jobject obj,jstring key,jstring value);
JNIEXPORT JNICALL jstring 	Java_com_wattertek_jni_WtkMediaSDKJNI_GetParameter(JNIEnv *env,jobject obj,jstring key);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_ReceiveCallNotification(JNIEnv *env,jobject obj,jstring callNotification);
JNIEXPORT JNICALL jstring 	Java_com_wattertek_jni_WtkMediaSDKJNI_MakeCall(JNIEnv *env,jobject obj,jstring dstID,jstring srcID,jint media_type);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_AnswerCall(JNIEnv *env,jobject obj,jstring callID);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_HoldCall(JNIEnv *env,jobject obj,jstring callID,jint hold_state);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_MuteCall(JNIEnv *env,jobject obj,jstring callID,jint mute_state);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_HangupCall(JNIEnv *env,jobject obj,jstring callID);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_SetVideoDisplay(JNIEnv *env,jobject obj, jobject localView, jobject remoteView);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_StartVideoSend(JNIEnv *env,jobject obj, jstring callID);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_StopVideoSend(JNIEnv *env,jobject obj, jstring callID,jint reason);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_SetCamera(JNIEnv *env,jobject obj, jint device_id);
JNIEXPORT JNICALL jstring 	Java_com_wattertek_jni_WtkMediaSDKJNI_MakeOutboundCall(JNIEnv *env,jobject obj,jstring dstID,jstring srcID,jint media_type,jstring posInfo,jstring msInfo,jstring via);
JNIEXPORT JNICALL jstring 	Java_com_wattertek_jni_WtkMediaSDKJNI_JoinConference(JNIEnv *env,jobject obj,jstring dstIDs,jstring groupID,jint media_type,jstring srcID,jstring confID);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_HangupConference(JNIEnv *env,jobject obj,jstring confID);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_ListConference(JNIEnv *env,jobject obj,jstring confID);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_CtrlConference(JNIEnv *env,jobject obj,jstring confID,jint ctrl_action,jstring dstID);
JNIEXPORT JNICALL jstring 	Java_com_wattertek_jni_WtkMediaSDKJNI_GetServerCallID(JNIEnv *env,jobject obj,jstring callID);
JNIEXPORT JNICALL jint 		Java_com_wattertek_jni_WtkMediaSDKJNI_GetCallQualityLevel(JNIEnv *env,jobject obj,jstring callID);
#ifdef __cplusplus
}
#endif

#endif

