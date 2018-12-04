#ifndef _wtk_media_sdk_h_
#define _wtk_media_sdk_h_

#include "wtk_media_sdk_define.h"

class WtkMediaSDKEvent
{
public:
	virtual void onReceiveCallEvent(char *callId, char *callerId, char *calleeId, char *callerName, int media, int callType) = 0;
	virtual void onCallStateEvent(char *callId, int state, int reason) = 0;
	virtual void onRemoteVideoStateEvent(char *callId, int state) = 0;
	virtual void onConferenceStateEvent(char *conferenceId, char *callId, char *callerId, int state, int reason) = 0;
	virtual void onConferenceVadListEvent(char *conferenceId, char *callerId, char *listinfo) = 0;
	virtual void onSendSignal(char* targetId,char* isConf, char*srcID, char* jsonContent,char * signalType,char * signalCause,char *coreServerCallID) = 0;
protected:
    virtual ~WtkMediaSDKEvent() {}
    WtkMediaSDKEvent() {}	
};

class WtkMediaSDK
{
public:
	static bool Initialize(const char *appKey, void * sdkEventImpl);
	static void SetUserProfile(const char* profile, const char* ms_info);
	static int SetParameter(const char *key, const char *value);
	static void GetParameter(const char *key, char *outValue);
	static int ReceiveCallNotification(const char *jsonCallNotification);
	static int MakeCall(const char *dstID, const char *srcID, int media, char *outCallID);
	static int AnswerCall(const char *CallID);
	static int HoldCall(const char *CallID, int hold_state);
	static int MuteCall(const char *CallID, int mute_state);
	static int HangupCall(const char *CallID);
	static int SetVideoDisplay(void *localView, void *remoteView);
	static int StartVideoSend(const char *CallID);
	static int StopVideoSend(const char *CallID, int reason);
	static int SetCamera(int device_id);
	static int MakeOutboundCall(const char *dstID, const char *srcID, const char *posInfo, const char *msInfo, const char *via, int media_type, char *outConfID);
	static int JoinConference(const char *dstIDs, const char *groupID, int media_type, const char* srcID, const char* confID, char *outConfID);
	static int HangupConference(const char* ConfID);
	static int ListConference(const char* ConfID);
	static int CtrlConference(const char* ConfID, int ctrl_action, const char *dstID);
	static int GetServerCallID(const char* callID, char *coreServerId);
	static int GetCallQualityLevel(const char* callID);
};
#endif
