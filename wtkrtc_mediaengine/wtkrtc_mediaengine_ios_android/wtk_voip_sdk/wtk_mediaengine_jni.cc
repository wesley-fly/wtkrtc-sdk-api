#include <string.h>
#include <stdio.h>
#include "wtk_mediaengine_jni.h"
#include "rtc_base/trace_event.h"
#include <android/log.h>
#include "wtk_voip_sdk_jni/wtk_service_client/wtkcall_lib.h"
#include "modules/utility/include/jvm_android.h"

struct WtkStateEvent	g_WtkStateEvent;
static JavaVM* g_WtkJvmContext;

jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved) {
  	g_WtkJvmContext = jvm;
	webrtc::JVM::Initialize(jvm);
	
	JNILOGI("JNI_OnLoad Success!");
	return JNI_VERSION_1_6;
}
void JNICALL JNI_OnUnLoad(JavaVM* jvm, void* reserved) {
	webrtc::JVM::Uninitialize();
	g_WtkJvmContext = NULL; 
	JNILOGI("JNI_OnUnLoad !");
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
			case EVENT_CONTROL:
			{
				ControlInfo *ctrl = NULL; 
				ctrl = (ControlInfo *)info ; 
				sprintf(msg ,"{callNo:%d, type:%d, value:%d}", ctrl->callNo,ctrl->type, ctrl->value);
				break; 
			}
			case EVENT_MESSAGE:
			{
				MessageInfo * msgInfo = NULL; 
				msgInfo = (MessageInfo *) info; 
				char* msgHeader;
				msgHeader=strstr(msgInfo->message,"SUV");
				if(msgHeader != NULL)
				{
					sprintf(msg,"SUV {callNo:%d,msginfo:%s}",msgInfo->callNo, msgInfo->message+4);
				}
				else
					sprintf(msg ,"%s",msgInfo->message);
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


jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxInitialize(JNIEnv* env,jobject obj,jobject jlistener)
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
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);

	ret = wtkcall_initialize_iax();
	
	return ret;
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxShutdown(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
	wtkcall_shutdown_iax();
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_StartAudioPlayout(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
	wtkcall_start_audio();
}

void Java_com_wtk_mobile_jni_WtkMediaJNI_StopAudioPlayout(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
	wtkcall_stop_audio();
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_StartVideoPlayout(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
	wtkcall_start_video();
}

void Java_com_wtk_mobile_jni_WtkMediaJNI_StopVideoPlayout(JNIEnv* env,jobject obj)
{	
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
	wtkcall_stop_video();
}

jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxRegister(JNIEnv* env,jobject obj,jstring name, jstring number, jstring pass, jstring host, jstring port)
{
	jint ret = -1;
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
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

	ret = wtkcall_register(pName,pNumber,pPass,pHost,pPort);
	
	(env)->ReleaseStringUTFChars(name, pName );
	(env)->ReleaseStringUTFChars(number, pNumber );
	(env)->ReleaseStringUTFChars(pass, pPass );
	(env)->ReleaseStringUTFChars(host, pHost );
	(env)->ReleaseStringUTFChars(port, pPort );
	return ret;
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxUnRegister(JNIEnv *env, jobject obj,jint regId)
{
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);

	wtkcall_unregister(regId); 
}
jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxDial(JNIEnv *env, jobject obj,jstring dest, jstring host, jstring user, jstring cmd, jstring ext)
{
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
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
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
	wtkcall_answer(callNo); 
	wtkcall_select(callNo);
}
jint Java_com_wtk_mobile_jni_WtkMediaJNI_IaxHangup(JNIEnv *env, jobject obj,jint callNo)
{
	jint ret = -1;
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
	ret = wtkcall_hangup(callNo); 
	return ret;
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxSetHold(JNIEnv *env, jobject obj,jint callNo,jint hold)
{
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
	wtkcall_set_hold(callNo,hold); 
}
void Java_com_wtk_mobile_jni_WtkMediaJNI_IaxSetFormat(JNIEnv *env, jobject obj,jint callNo,int rtp_format)
{
	JNILOGI("%s,%d", __FUNCTION__,__LINE__);
	wtkcall_set_format(callNo,rtp_format); 
}
