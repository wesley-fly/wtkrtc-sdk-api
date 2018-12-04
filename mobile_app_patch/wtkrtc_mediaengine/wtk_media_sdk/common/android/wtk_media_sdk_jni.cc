#include <string.h>
#include <stdio.h>
#include "rtc_base/trace_event.h"
#include <android/log.h>
#include "modules/utility/include/jvm_android.h"
#include "sdk/android/native_api/base/init.h"
#include "wtkrtc_mediaengine/wtk_service_client/wtkcall_lib.h"
#include "wtk_media_sdk_jni.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk_define.h"
#include "wtkrtc_mediaengine/wtk_media_sdk/common/wtk_media_sdk.h"
#include "base/json/json_writer.h"
#include "base/values.h"

static JavaVM* g_JvmContext;
struct WtkJvmContextInfo g_WtkJvmContextInfo;

jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved) {
  	g_JvmContext = jvm;
	webrtc::JVM::Initialize(jvm);
	webrtc::InitAndroid(jvm);

	return JNI_VERSION_1_6;
}
void JNICALL JNI_OnUnLoad(JavaVM* jvm, void* reserved) {
	webrtc::JVM::Uninitialize();
	g_JvmContext = NULL; 
}

class AppMediaEventImpl : public WtkMediaSDKEvent
{
public:
	AppMediaEventImpl() {}
	virtual ~AppMediaEventImpl(){}
	virtual void onReceiveCallEvent(char *callId, char *callerId, char *calleeId, char *callerName, int media, int callType)
	{
		JNIEnv *env = NULL;
		int is_attached = 0;
		std::string json_string;
		char json_string_char[REPORT_EVENT_BUF_SIZE] = {0}; 

		base::DictionaryValue dict;
		dict.SetKey("callId", base::Value(callId));
		dict.SetKey("callerId", base::Value(callerId));
		dict.SetKey("calleeId", base::Value(calleeId));
		dict.SetKey("callerName", base::Value(callerName));
		dict.SetKey("media", base::Value(media));
		dict.SetKey("callType", base::Value(callType));
		base::JSONWriter::Write(dict, &json_string);
		JNILOGE("%s,%d, json_string:%s", __FUNCTION__,__LINE__,json_string.c_str());
		strcpy(json_string_char, json_string.c_str());

		if(g_JvmContext->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) 
		{
			jint result = g_JvmContext->AttachCurrentThread(&env,NULL); 

			if (result != 0)
				return;
			else 
				is_attached = 1;
		}
		jstring j_event_json_string = env->NewStringUTF(json_string_char);
		env->CallVoidMethod(g_WtkJvmContextInfo.listener, g_WtkJvmContextInfo.onReceiveCallEventId,j_event_json_string);
		env->DeleteLocalRef(j_event_json_string);

		if (is_attached)
		{
			g_JvmContext->DetachCurrentThread(); 
			env = NULL;
		}
	}
	virtual void onCallStateEvent(char *callId, int state, int reason)
	{
		JNIEnv *env = NULL;
		int is_attached = 0;
		std::string json_string;
		char json_string_char[REPORT_EVENT_BUF_SIZE] = {0}; 
		
		base::DictionaryValue dict;
		dict.SetKey("callID", base::Value(callId));
		dict.SetKey("state", base::Value(state));
		dict.SetKey("reason", base::Value(reason));
		base::JSONWriter::Write(dict, &json_string);
		JNILOGE("%s,%d, json_string:%s", __FUNCTION__,__LINE__,json_string.c_str());
		
		strcpy(json_string_char, json_string.c_str());

		if(g_JvmContext->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) 
		{
			jint result = g_JvmContext->AttachCurrentThread(&env,NULL); 

			if (result != 0)
				return;
			else 
				is_attached = 1;
		}
		
		jstring j_event_json_string = env->NewStringUTF(json_string_char);
		env->CallVoidMethod(g_WtkJvmContextInfo.listener, g_WtkJvmContextInfo.onCallStateEventId,j_event_json_string);
		env->DeleteLocalRef(j_event_json_string);

		if (is_attached)
		{
			g_JvmContext->DetachCurrentThread(); 
			env = NULL;
		}
	}
	virtual void onRemoteVideoStateEvent(char *callId, int state)
	{
		JNIEnv *env = NULL;
		int is_attached = 0;
		std::string json_string;
		char json_string_char[REPORT_EVENT_BUF_SIZE] = {0}; 
		
		base::DictionaryValue dict;
		dict.SetKey("callId", base::Value(callId));
		dict.SetKey("state", base::Value(state));
		base::JSONWriter::Write(dict, &json_string);
		JNILOGE("%s,%d, json_string:%s", __FUNCTION__,__LINE__,json_string.c_str());
		
		strcpy(json_string_char, json_string.c_str());
		
		if(g_JvmContext->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) 
		{
			jint result = g_JvmContext->AttachCurrentThread(&env,NULL); 

			if (result != 0)
				return;
			else 
				is_attached = 1;
		}
		jstring j_event_json_string = env->NewStringUTF(json_string_char);
		env->CallVoidMethod(g_WtkJvmContextInfo.listener, g_WtkJvmContextInfo.onRemoteVideoStateEventId,j_event_json_string);
		env->DeleteLocalRef(j_event_json_string);

		if (is_attached)
		{
			g_JvmContext->DetachCurrentThread(); 
			env = NULL;
		}
	}
	virtual void onConferenceStateEvent(char *conferenceId, char *callId, char *callerId, int state, int reason)
	{
		JNIEnv *env = NULL;
		int is_attached = 0;
		std::string json_string;
		char json_string_char[REPORT_EVENT_BUF_SIZE] = {0}; 
		
		base::DictionaryValue dict;
		dict.SetKey("conferenceId", base::Value(conferenceId));
		dict.SetKey("callId", base::Value(callId));
		dict.SetKey("callerId", base::Value(callerId));
		dict.SetKey("state", base::Value(state));
		dict.SetKey("reason", base::Value(reason));
		base::JSONWriter::Write(dict, &json_string);
		JNILOGE("%s,%d, json_string:%s", __FUNCTION__,__LINE__,json_string.c_str());
		
		strcpy(json_string_char, json_string.c_str());

		if(g_JvmContext->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) 
		{
			jint result = g_JvmContext->AttachCurrentThread(&env,NULL); 

			if (result != 0)
				return;
			else 
				is_attached = 1;
		}
		jstring j_event_json_string = env->NewStringUTF(json_string_char);
		env->CallVoidMethod(g_WtkJvmContextInfo.listener, g_WtkJvmContextInfo.onConferenceStateEventId,j_event_json_string);
		env->DeleteLocalRef(j_event_json_string);

		if (is_attached)
		{
			g_JvmContext->DetachCurrentThread(); 
			env = NULL;
		}
	}
	virtual void onConferenceVadListEvent(char *conferenceId, char *callerId, char *listinfo)
	{
		JNIEnv *env = NULL;
		int is_attached = 0;
		std::string json_string;
		char json_string_char[REPORT_EVENT_BUF_SIZE*3] = {0}; 
		
		base::DictionaryValue dict;
		dict.SetKey("conferenceId", base::Value(conferenceId));
		dict.SetKey("callerId", base::Value(callerId));
		dict.SetKey("listInfo", base::Value(listinfo));

		base::JSONWriter::Write(dict, &json_string);
		JNILOGE("%s,%d, json_string:%s", __FUNCTION__,__LINE__,json_string.c_str());
		
		strcpy(json_string_char, json_string.c_str());
		
		if(g_JvmContext->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) 
		{
			jint result = g_JvmContext->AttachCurrentThread(&env,NULL); 

			if (result != 0)
				return;
			else 
				is_attached = 1;
		}
		jstring j_event_json_string = env->NewStringUTF(json_string_char);
		env->CallVoidMethod(g_WtkJvmContextInfo.listener, g_WtkJvmContextInfo.onConferenceVadListEventId,j_event_json_string);
		env->DeleteLocalRef(j_event_json_string);

		if (is_attached)
		{
			g_JvmContext->DetachCurrentThread(); 
			env = NULL;
		}
	}
	virtual void onSendSignal(char* targetId,char* isConf, char*srcID, char* jsonContent,char* signalType,char * signalCause,char *coreServerCallID)
	{
		JNIEnv *env = NULL;
		int is_attached = 0;
		std::string json_string;
		char json_string_char[REPORT_EVENT_BUF_SIZE*3] = {0}; 
		
		base::DictionaryValue dict;
		dict.SetKey("targetId", base::Value(targetId));
		dict.SetKey("isConfEvent", base::Value(isConf));
		dict.SetKey("srcID", base::Value(srcID));
		dict.SetKey("jsonContent", base::Value(jsonContent));
		dict.SetKey("signalType", base::Value(signalType));
		dict.SetKey("signalCause", base::Value(signalCause));
		dict.SetKey("coreServerCallID", base::Value(coreServerCallID));

		base::JSONWriter::Write(dict, &json_string);
		//JNILOGE("%s,%d, json_string:%s", __FUNCTION__,__LINE__,json_string.c_str());
		
		strcpy(json_string_char, json_string.c_str());

		if(g_JvmContext->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) 
		{
			jint result = g_JvmContext->AttachCurrentThread(&env,NULL); 

			if (result != 0)
				return;
			else 
				is_attached = 1;
		}
		jstring j_event_json_string = env->NewStringUTF(json_string_char);
		env->CallVoidMethod(g_WtkJvmContextInfo.listener, g_WtkJvmContextInfo.onSendSignalId,j_event_json_string);
		env->DeleteLocalRef(j_event_json_string);

		if (is_attached)
		{
			g_JvmContext->DetachCurrentThread(); 
			env = NULL;
		}
	}
};

AppMediaEventImpl *g_WtkSDKEventImpl;

jboolean Java_com_wattertek_jni_WtkMediaSDKJNI_Initialize(JNIEnv* env,jobject obj,jobject context,jstring appkey,jobject jlistener)
{
	jclass listenClass = NULL;
	const char *pAppKey = NULL;
	
	g_WtkJvmContextInfo.listener = env->NewGlobalRef(jlistener);
	listenClass = env->GetObjectClass(jlistener);
	if (!listenClass){
		JNILOGE("%s,%d, can not get listenClass", __FUNCTION__,__LINE__);
		return false; 
	}

	g_WtkJvmContextInfo.onReceiveCallEventId = env->GetMethodID(listenClass,"onReceiveCallEvent","(Ljava/lang/String;)V");
	if (!g_WtkJvmContextInfo.onReceiveCallEventId){
		JNILOGE("%s,%d, can not find onReceiveCallEventId", __FUNCTION__,__LINE__);
		return false; 
	}
	g_WtkJvmContextInfo.onCallStateEventId = env->GetMethodID(listenClass,"onCallStateEvent","(Ljava/lang/String;)V");
	if (!g_WtkJvmContextInfo.onCallStateEventId){
		JNILOGE("%s,%d, can not find onCallStateEventId", __FUNCTION__,__LINE__);
		return false; 
	}
	g_WtkJvmContextInfo.onRemoteVideoStateEventId = env->GetMethodID(listenClass,"onRemoteVideoStateEvent","(Ljava/lang/String;)V");
	if (!g_WtkJvmContextInfo.onRemoteVideoStateEventId){
		JNILOGE("%s,%d, can not find onRemoteVideoStateEventId", __FUNCTION__,__LINE__);
		return false; 
	}
	g_WtkJvmContextInfo.onConferenceStateEventId = env->GetMethodID(listenClass,"onConferenceStateEvent","(Ljava/lang/String;)V");
	if (!g_WtkJvmContextInfo.onConferenceStateEventId){
		JNILOGE("%s,%d, can not find onConferenceStateEventId", __FUNCTION__,__LINE__);
		return false; 
	}
	g_WtkJvmContextInfo.onConferenceVadListEventId = env->GetMethodID(listenClass,"onConferenceVadListEvent","(Ljava/lang/String;)V");
	if (!g_WtkJvmContextInfo.onConferenceVadListEventId){
		JNILOGE("%s,%d, can not find onConferenceVadListEventId", __FUNCTION__,__LINE__);
		return false; 
	}
	g_WtkJvmContextInfo.onSendSignalId = env->GetMethodID(listenClass,"onSendSignal","(Ljava/lang/String;)V");
	if (!g_WtkJvmContextInfo.onSendSignalId){
		JNILOGE("%s,%d, can not find onSendSignalId", __FUNCTION__,__LINE__);
		return false; 
	}

	if(wtkcall_init_AndroidEnv(g_JvmContext, context) != 0)
	{
		JNILOGE("%s,%d, wtkcall_init_AndroidEnv error", __FUNCTION__,__LINE__);
		return false; 
	}
	
	g_WtkSDKEventImpl = new AppMediaEventImpl();
	pAppKey = (env)->GetStringUTFChars(appkey, NULL);

	if(!WtkMediaSDK::Initialize(pAppKey, g_WtkSDKEventImpl))
	{
		JNILOGE("%s,%d, WtkMediaSDK::Initialize false", __FUNCTION__,__LINE__);
		(env)->ReleaseStringUTFChars(appkey, pAppKey);
		return false;
	}
	else
	{
		(env)->ReleaseStringUTFChars(appkey, pAppKey);
		JNILOGI("Wattertek MediaEngine SDK Start success with SDK version:%s", WTK_MEDIA_SDK_VERSION);
		return true;
	}
}
void Java_com_wattertek_jni_WtkMediaSDKJNI_SetUserProfile(JNIEnv *env,jobject obj,jstring profile,jstring listRS)
{
	const char *pProfile = NULL;
	const char *pListRS = NULL;
	
	pProfile = (env)->GetStringUTFChars(profile , NULL);
	pListRS = (env)->GetStringUTFChars(listRS , NULL);

	JNILOGI("%s,line %d, Params  = %s, %s", __FUNCTION__,__LINE__,pProfile,pListRS);
	WtkMediaSDK::SetUserProfile(pProfile, pListRS);
	
	(env)->ReleaseStringUTFChars(profile, pProfile );
	(env)->ReleaseStringUTFChars(listRS, pListRS );
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_SetParameter(JNIEnv *env,jobject obj,jstring key,jstring value)
{
	const char *pKey = NULL;
	const char *pValue = NULL;
	jint res = -1;
	
	pKey = (env)->GetStringUTFChars(key , NULL);
	pValue = (env)->GetStringUTFChars(value , NULL);

	JNILOGI("%s,line %d, Params  = %s, %s", __FUNCTION__,__LINE__,pKey,pValue);
	res = WtkMediaSDK::SetParameter(pKey, pValue);
	
	(env)->ReleaseStringUTFChars(key, pKey );
	(env)->ReleaseStringUTFChars(value, pValue );

	return res; 
}

jstring Java_com_wattertek_jni_WtkMediaSDKJNI_GetParameter(JNIEnv *env,jobject obj,jstring key)
{
	const char *pKey = NULL;
	char pValue[32] = {0};
	
	pKey = (env)->GetStringUTFChars(key , NULL);

	WtkMediaSDK::GetParameter(pKey, pValue);
	JNILOGI("%s,line %d, Params  = %s,%s", __FUNCTION__,__LINE__,pKey,pValue);
	(env)->ReleaseStringUTFChars(key, pKey);
	
	return (env)->NewStringUTF(pValue);
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_ReceiveCallNotification(JNIEnv *env,jobject obj,jstring callNotification)
{
	const char *pNotification = NULL;

	pNotification = (env)->GetStringUTFChars(callNotification, NULL);
	JNILOGI("%s,line %d, Params  = %s", __FUNCTION__,__LINE__,pNotification);

	int res = WtkMediaSDK::ReceiveCallNotification(pNotification);

	(env)->ReleaseStringUTFChars(callNotification, pNotification);

	return res;
}
jstring Java_com_wattertek_jni_WtkMediaSDKJNI_MakeCall(JNIEnv *env,jobject obj,jstring dstID,jstring srcID,jint media_type)
{
	const char *pDstId = NULL;
	const char *pSrcId = NULL;
	char outCallID[32] = {0};
	
	pDstId = (env)->GetStringUTFChars(dstID , NULL);
	pSrcId = (env)->GetStringUTFChars(srcID , NULL);
	JNILOGI("%s,line %d, Params  = %s,%s,%d,", __FUNCTION__,__LINE__,pDstId,pSrcId,media_type);

	WtkMediaSDK::MakeCall(pDstId,pSrcId,media_type,outCallID);
	
	JNILOGI("%s,line %d, out call id = %s", __FUNCTION__,__LINE__,outCallID);
	(env)->ReleaseStringUTFChars(dstID, pDstId );
	(env)->ReleaseStringUTFChars(srcID, pSrcId );

	return (env)->NewStringUTF(outCallID);
}

jint Java_com_wattertek_jni_WtkMediaSDKJNI_AnswerCall(JNIEnv *env,jobject obj,jstring callID)
{
	const char *pCallID = NULL;
	pCallID = (env)->GetStringUTFChars(callID , NULL);
	JNILOGI("%s,line %d, Params  = %s", __FUNCTION__,__LINE__,pCallID);	
	
	int ret = WtkMediaSDK::AnswerCall(pCallID);

	(env)->ReleaseStringUTFChars(callID, pCallID);

	return ret;
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_HoldCall(JNIEnv *env,jobject obj,jstring callID,jint hold_state)
{
	const char *pCallID = NULL;
	pCallID = (env)->GetStringUTFChars(callID , NULL);
	JNILOGI("%s,line %d, Params  = %s,%d", __FUNCTION__,__LINE__,pCallID, hold_state);	
	
	int ret = WtkMediaSDK::HoldCall(pCallID, hold_state);

	(env)->ReleaseStringUTFChars(callID, pCallID);

	return ret;
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_MuteCall(JNIEnv *env,jobject obj,jstring callID,jint mute_state)
{
	const char *pCallID = NULL;
	pCallID = (env)->GetStringUTFChars(callID , NULL);
	JNILOGI("%s,line %d, Params  = %s,%d", __FUNCTION__,__LINE__,pCallID, mute_state);	
	
	int ret = WtkMediaSDK::MuteCall(pCallID, mute_state);

	(env)->ReleaseStringUTFChars(callID, pCallID);

	return ret;
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_HangupCall(JNIEnv *env,jobject obj,jstring callID)
{
	const char *pCallID = NULL;
	pCallID = (env)->GetStringUTFChars(callID , NULL);
	JNILOGI("%s,line %d, Params  = %s", __FUNCTION__,__LINE__,pCallID);	
	
	int ret = WtkMediaSDK::HangupCall(pCallID);

	(env)->ReleaseStringUTFChars(callID, pCallID);

	return ret;
}

jint Java_com_wattertek_jni_WtkMediaSDKJNI_SetVideoDisplay(JNIEnv *env,jobject obj, jobject localView, jobject remoteView)
{
	jint ret = -1; 

	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	ret = WtkMediaSDK::SetVideoDisplay(localView,remoteView); 
	
	return ret; 
}

jint Java_com_wattertek_jni_WtkMediaSDKJNI_StartVideoSend(JNIEnv *env,jobject obj, jstring callID)
{
	int ret = -1; 
	const char* pCallID = NULL; 
	pCallID = (env)->GetStringUTFChars(callID,NULL);
	
	JNILOGI("%s,line %d, Params  = %s", __FUNCTION__,__LINE__,pCallID);
	ret = WtkMediaSDK::StartVideoSend(pCallID); 
	
	(env)->ReleaseStringUTFChars(callID, pCallID);
	return ret ; 
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_StopVideoSend(JNIEnv *env,jobject obj, jstring callID,jint reason)
{
	int ret = -1; 
	const char* pCallID = NULL; 
	pCallID = (env)->GetStringUTFChars(callID,NULL);
	
	JNILOGI("%s,line %d, Params  = %s", __FUNCTION__,__LINE__,pCallID);
	ret = WtkMediaSDK::StopVideoSend(pCallID,reason); 

	(env)->ReleaseStringUTFChars(callID, pCallID);
	
	return ret ;
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_SetCamera(JNIEnv *env,jobject obj, jint device_id)
{	
	JNILOGI("%s,line %d, Params  = %d", __FUNCTION__,__LINE__,device_id);
	return WtkMediaSDK::SetCamera(device_id); 
}

jstring Java_com_wattertek_jni_WtkMediaSDKJNI_MakeOutboundCall(JNIEnv *env,jobject obj,jstring dstID,jstring srcID,jint media_type,jstring posInfo,jstring msInfo,jstring via)
{
	const char* pDstID = NULL;
	const char* pSrcID = NULL;
	const char* pPosInfo = NULL;
	const char* pMsInfo = NULL;
	const char* pVia = NULL;
	char outCallID[256] = {0};

	pDstID = (env)->GetStringUTFChars(dstID,NULL);
	pSrcID = (env)->GetStringUTFChars(srcID,NULL);
	pPosInfo = (env)->GetStringUTFChars(posInfo,NULL);
	pMsInfo = (env)->GetStringUTFChars(msInfo,NULL);
	pVia = (env)->GetStringUTFChars(via,NULL);

	JNILOGI("%s,line %d, Params  = %s,%s,%s,%s,%s,%d", __FUNCTION__,__LINE__,pDstID,pSrcID,pPosInfo,pMsInfo,pVia,media_type);
	WtkMediaSDK::MakeOutboundCall(pDstID,pSrcID,pPosInfo,pMsInfo,pVia,media_type,outCallID);
	JNILOGI("%s,line %d, Params  out conf id = %s", __FUNCTION__,__LINE__,outCallID);

	(env)->ReleaseStringUTFChars(dstID, pDstID);
	(env)->ReleaseStringUTFChars(srcID, pSrcID);
	(env)->ReleaseStringUTFChars(posInfo, pPosInfo);
	(env)->ReleaseStringUTFChars(msInfo, pMsInfo);
	(env)->ReleaseStringUTFChars(via, pVia);

	return (env)->NewStringUTF(outCallID);
}
jstring Java_com_wattertek_jni_WtkMediaSDKJNI_JoinConference(JNIEnv *env,jobject obj,jstring dstIDs,jstring groupID,jint media_type,jstring srcID,jstring confID)
{
	const char* pDstIDs = NULL;
	const char* pGroupID = NULL;
	const char* pSrcID = NULL;
	const char* pConfID = NULL;

	char outConfID[256] = {0};
	pDstIDs = (env)->GetStringUTFChars(dstIDs,NULL);
	pGroupID = (env)->GetStringUTFChars(groupID,NULL);
	pSrcID = (env)->GetStringUTFChars(srcID,NULL);
	pConfID = (env)->GetStringUTFChars(confID,NULL);

	JNILOGI("%s,line %d, Params  = %s,%s,%s,%s,%d", __FUNCTION__,__LINE__,pDstIDs,pGroupID,pSrcID,pConfID,media_type);
	WtkMediaSDK::JoinConference(pDstIDs, pGroupID, media_type, pSrcID,pConfID,outConfID);
	JNILOGI("%s,line %d, Params  out conf id = %s", __FUNCTION__,__LINE__,outConfID);

	(env)->ReleaseStringUTFChars(dstIDs, pDstIDs);
	(env)->ReleaseStringUTFChars(groupID, pGroupID);
	(env)->ReleaseStringUTFChars(srcID, pSrcID);
	(env)->ReleaseStringUTFChars(confID, pConfID);

	return (env)->NewStringUTF(outConfID);
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_HangupConference(JNIEnv *env,jobject obj,jstring confID)
{
	const char* pConfID = NULL;
	pConfID = (env)->GetStringUTFChars(confID,NULL);
	
	JNILOGI("%s,line %d, Params  conf id = %s", __FUNCTION__,__LINE__,pConfID);

	int ret = WtkMediaSDK::HangupConference(pConfID);
	
	(env)->ReleaseStringUTFChars(confID, pConfID);

	return ret;
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_ListConference(JNIEnv *env,jobject obj,jstring confID)
{
	const char* pConfID = NULL;
	
	pConfID = (env)->GetStringUTFChars(confID,NULL);
	
	JNILOGI("%s,line %d, Params = %s", __FUNCTION__,__LINE__,pConfID);

	int ret = WtkMediaSDK::ListConference(pConfID);
	
	(env)->ReleaseStringUTFChars(confID, pConfID);

	return ret;
}
jint Java_com_wattertek_jni_WtkMediaSDKJNI_CtrlConference(JNIEnv *env,jobject obj,jstring confID,jint ctrl_action,jstring dstID)
{
	const char* pConfID = NULL;
	const char* pDstID = NULL;
	
	pConfID = (env)->GetStringUTFChars(confID,NULL);
	pDstID = (env)->GetStringUTFChars(dstID,NULL);

	JNILOGI("%s,line %d, Params = %s,%d,%s", __FUNCTION__,__LINE__,pConfID,ctrl_action,pDstID);
	int res = WtkMediaSDK::CtrlConference(pConfID, ctrl_action, pDstID);
	
	(env)->ReleaseStringUTFChars(confID, pConfID);
	(env)->ReleaseStringUTFChars(dstID, pDstID);

	return res;
}
jstring Java_com_wattertek_jni_WtkMediaSDKJNI_GetServerCallID(JNIEnv *env,jobject obj,jstring callID)
{
	const char* pCallID = NULL;
	
	char CoreServerId[256] = {0};

	pCallID = (env)->GetStringUTFChars(callID,NULL);

	WtkMediaSDK::GetServerCallID(pCallID, CoreServerId);
	
	JNILOGI("%s,line %d, Params = %s, %s", __FUNCTION__,__LINE__,pCallID,CoreServerId);
	(env)->ReleaseStringUTFChars(callID, pCallID);

	return (env)->NewStringUTF(CoreServerId);
}

jint Java_com_wattertek_jni_WtkMediaSDKJNI_GetCallQualityLevel(JNIEnv *env,jobject obj,jstring callID)
{
	const char* pCallID = NULL;
	int level = -1;

	pCallID = (env)->GetStringUTFChars(callID,NULL);

	level = WtkMediaSDK::GetCallQualityLevel(pCallID);
	
	JNILOGI("%s,line %d, Params = %s, %x", __FUNCTION__,__LINE__,pCallID,level);
	(env)->ReleaseStringUTFChars(callID, pCallID);

	return level;
}