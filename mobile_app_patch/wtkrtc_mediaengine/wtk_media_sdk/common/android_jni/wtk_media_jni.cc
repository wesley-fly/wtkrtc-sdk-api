#include <string.h>
#include <stdio.h>
#include "rtc_base/trace_event.h"
#include <android/log.h>
#include "modules/utility/include/jvm_android.h"
#include "sdk/android/native_api/base/init.h"
#include "../../../wtk_service_client/wtkcall_lib.h"
#include "wtk_media_jni.h"


struct WtkStateEvent	g_WtkStateEvent;
static JavaVM* g_WtkJvmContext;

jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved) {
  	g_WtkJvmContext = jvm;
	webrtc::JVM::Initialize(jvm);
	webrtc::InitAndroid(jvm);
	JNILOGI("JNI_OnLoad Success!");
	return JNI_VERSION_1_6;
}
void JNICALL JNI_OnUnLoad(JavaVM* jvm, void* reserved) {
	webrtc::JVM::Uninitialize();
	g_WtkJvmContext = NULL; 
}

int AppAllEventCallback(int type, void* info)
{
	char msg[1024],peername[256];
	int isAttached = 0;
	if (type == EVENT_LOG)
	{
		JNILOGI("IAX2 Callback Log: %s\n",(char*)info);
	}
	else
	{
		switch(type)	
		{
			case EVENT_REGISTRATION:
			{
				RegistrationInfo* ri = NULL;
				ri = (RegistrationInfo*)info;
				sprintf(msg, "{id:%d,reply:%d }", ri->id,ri->reply);
				break;
			}
			case EVENT_STATE:
			{
				CallInfo * ci = NULL; 
				ci = (CallInfo *) info; 
				
				char * ptr = NULL; 
				memcpy(peername,ci->peer_name,strlen(ci->peer_name)+1);
				ptr = strchr(peername,':');
				if(ptr)
					*ptr = '\0'; 

				sprintf(msg, "{callNo:%d, peername:%s, peernum :%s,  activity : %d ,start : %d ,  duration :%d ,type :%d }",
						  ci->callNo,peername, ci->peer_number,  ci->activity,ci->start, ci->duration, ci->type);
				break;
			}
			case EVENT_MESSAGE:
			{
				MessageInfo * msgInfo = NULL; 
				msgInfo = (MessageInfo *) info; 

				sprintf(msg,"{callNo:%d,msginfo:%s}",msgInfo->callNo, msgInfo->message);

				break; 
			}
			case EVENT_PTT:
			{
				PttInfo * pttInfo = NULL; 
				pttInfo = (PttInfo *)info; 
				sprintf(msg ,"{callNo:%d, event:%d, speaker_id:\"%s\", position:%d}",pttInfo->callNo,pttInfo->event,pttInfo->speaker_id,pttInfo->position);
				
				break; 
			}
			default:
			break;
		}

		JNIEnv *env = NULL;
		if(g_WtkJvmContext->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) 
		{
			jint result = g_WtkJvmContext->AttachCurrentThread(&env,NULL); 

			if (result != 0)
				return 0;
			else 
				isAttached = 1;
		}
		jstring jMsg = env->NewStringUTF(msg);
		//JNILOGE("%s,%d, type:%d, msg:%s", __FUNCTION__,__LINE__,type,msg);
		env->CallVoidMethod(g_WtkStateEvent.listener, g_WtkStateEvent.onWtkCallEventId,(jint)type,jMsg);
		env->DeleteLocalRef(jMsg);

		if (isAttached)
		{
			g_WtkJvmContext->DetachCurrentThread(); 
			env = NULL;
		}
	}
	
	return 0;
}

jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxInitialize(JNIEnv* env,jobject obj,jobject jlistener,jobject context)
{
	jint ret = -1;
	jclass listenClass = NULL;
	
	g_WtkStateEvent.listener = env->NewGlobalRef(jlistener);
	listenClass = env->GetObjectClass(jlistener);

	g_WtkStateEvent.onWtkCallEventId = env->GetMethodID(listenClass, "onWtkCallEvent", "(ILjava/lang/String;)V");
	
	if (!g_WtkStateEvent.onWtkCallEventId){
		JNILOGE("%s,%d, can not find onWtkCallEvent", __FUNCTION__,__LINE__);
		return false; 
	}
	
	wtkcall_set_jni_event_callback(AppAllEventCallback);
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	if(wtkcall_init_AndroidEnv(g_WtkJvmContext, context) != 0)
	{
		JNILOGE("%s,%d, wtkcall_init_AndroidEnv error", __FUNCTION__,__LINE__);
		return false; 
	}
	
	ret = wtkcall_initialize_iax();
	
	return ret;
}

jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxRegister(JNIEnv* env,jobject obj,jstring name, jstring number, jstring pass, jstring host, jstring port,jint refresh)
{
	jint ret = -1;
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	const char *pName = NULL;
	const char *pNumber = NULL;
	const char *pPass = NULL;
	const char *pHost = NULL;
	const char *pPort = NULL;
	pName = (env)->GetStringUTFChars(name , NULL);
	pNumber = (env)->GetStringUTFChars(number , NULL);
	pPass = (env)->GetStringUTFChars(pass , NULL);
	pHost = (env)->GetStringUTFChars(host , NULL);
	pPort = (env)->GetStringUTFChars(port , NULL);

	ret = wtkcall_register(pName,pNumber,pPass,pHost,pPort,refresh);
	
	(env)->ReleaseStringUTFChars(name, pName );
	(env)->ReleaseStringUTFChars(number, pNumber );
	(env)->ReleaseStringUTFChars(pass, pPass );
	(env)->ReleaseStringUTFChars(host, pHost );
	(env)->ReleaseStringUTFChars(port, pPort );
	return ret;
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxUnRegister(JNIEnv *env, jobject obj,jint regId)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);

	wtkcall_unregister(regId); 
}
jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxDial(JNIEnv *env, jobject obj,jstring dest, jstring host, jstring user, jstring cmd, jstring ext)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	jint callNo = -1;
	const char *pdest = NULL;
	const char *pHost = NULL;
	const char *pUser = NULL;
	const char *pCmd = NULL;
	const char *pExt = NULL;
	pdest = (env)->GetStringUTFChars( dest , NULL );
	pHost = (env)->GetStringUTFChars( host , NULL );
	pUser = (env)->GetStringUTFChars( user , NULL );
	pCmd = (env)->GetStringUTFChars( cmd , NULL );
	pExt = (env)->GetStringUTFChars( ext , NULL );
	
	JNILOGI("%s,dest = %s,phost=%s,puser=%s,pcmd=%s,pext=%s", __FUNCTION__,pdest,pHost,pUser,pCmd,pExt);
	callNo = wtkcall_dial(pdest,pHost,pUser,pCmd,pExt);

	(env)->ReleaseStringUTFChars( dest, pdest );
	(env)->ReleaseStringUTFChars( host, pHost );
	(env)->ReleaseStringUTFChars( user, pUser );
	(env)->ReleaseStringUTFChars( cmd, pCmd );
	(env)->ReleaseStringUTFChars( ext, pExt );

	return callNo;
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxAnswer(JNIEnv *env, jobject obj,jint callNo)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_answer(callNo); 
	wtkcall_select(callNo);
}
jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxHangup(JNIEnv *env, jobject obj,jint callNo)
{
	jint ret = -1;
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	ret = wtkcall_hangup(callNo); 
	return ret;
}
jint Java_com_wtk_mobile_jni_WtkMediaJNI_CtrlConference(JNIEnv *env, jobject obj,jint callNo,jint type)
{
	jint ret = -1;
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	ret = wtkcall_control_conference(callNo,type); 
	return ret;
}

void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxSetHold(JNIEnv *env, jobject obj,jint callNo,jint hold)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_set_hold(callNo,hold); 
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxShutdown(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_shutdown_iax();
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_StartAudioPlayout(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_start_audio();
}

void Java_com_wtk_mobile_jni_WtkMediaJNI_StopAudioPlayout(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_stop_audio();
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_StartVideoPlayout(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_start_video();
}

void Java_com_wtk_mobile_jni_WtkMediaJNI_StopVideoPlayout(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_stop_video();
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_SetVideoView(JNIEnv *env, jobject obj,jobject RemoteSurfaceView)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_set_render(RemoteSurfaceView); 
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_SwitchCamera(JNIEnv *env, jobject obj,jint callNo,int deviceId)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_switch_camera(callNo,deviceId); 
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_StartCapturer(JNIEnv *env, jobject obj)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_start_capturer(); 
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_StopCapturer(JNIEnv *env, jobject obj)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_stop_capturer(); 
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_SetCapturerRotation(JNIEnv *env, jobject obj, int rotation)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_set_capture_rotation(rotation); 
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_ConfigVideoParams(JNIEnv *env, jobject obj, int codec,int width,int height,int fps,int maxqp)
{
	JNILOGI("%s,line %d: %d,%d,%d,%d,%d", __FUNCTION__,__LINE__,codec,width,height,fps,maxqp);
	wtkcall_config_video(codec,width,height,fps,maxqp); 
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_ConfigStreamBitrate(JNIEnv *env, jobject obj,int audio_min_bps,int audio_max_bps,int video_min_bps,int video_max_bps)
{
	JNILOGI("%s,line %d: %d,%d,%d,%d", __FUNCTION__,__LINE__,audio_min_bps,audio_max_bps,video_min_bps,video_max_bps);
	wtkcall_config_bitrate(audio_min_bps,audio_max_bps,video_min_bps,video_max_bps); 
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_SetVideoConfView(JNIEnv *env, jobject obj,jobject SurfaceView0,jobject SurfaceView1,jobject SurfaceView2,jobject SurfaceView3)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_set_conf_render(SurfaceView0,SurfaceView1,SurfaceView2,SurfaceView3); 
}

void Java_com_wtk_mobile_jni_WtkMediaJNI_StartVideoConf(JNIEnv* env,jobject obj,int participant_ssrc)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_start_conf_video(participant_ssrc); 
}

void Java_com_wtk_mobile_jni_WtkMediaJNI_StopVideoConf(JNIEnv* env,jobject obj)
{
	JNILOGI("%s,line %d", __FUNCTION__,__LINE__);
	wtkcall_stop_conf_video(); 
}

