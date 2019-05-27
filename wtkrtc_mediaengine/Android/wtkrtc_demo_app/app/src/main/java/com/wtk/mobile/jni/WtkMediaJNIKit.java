package com.wtk.mobile.jni;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.SurfaceView;
import com.wtk.mobile.jni.WtkMediaJNI.StatusListener;

import org.json.JSONException;
import org.json.JSONObject;

public class WtkMediaJNIKit implements StatusListener {
    private static final String TAG = "WtkMediaJNIKit";

    private static WtkMediaJNIKit instance = null;
    private static WtkMediaJNI wtkMediaJNI = null;

    private String callNumber = null;
    private int callNo = -1;
    private Context m_context = null;
    static
    {
        System.loadLibrary("WtkMediaEngineJni");
    }

    public static WtkMediaJNIKit getInstance()
    {
        if (instance == null) {
            instance = new WtkMediaJNIKit();
        }

        return instance;
    }
    public void SetBroadCast(Context context)
    {
        m_context = context;
    }
    public int IaxInitialize(Context context)
    {
        return wtkMediaJNI.IaxInitialize(this, context);
    }
    public void IaxShutdown()
    {
        wtkMediaJNI.IaxShutdown();
    }
    public void StartAudioPlayout()
    {
        wtkMediaJNI.StartAudioPlayout();
    }
    public void StopAudioPlayout()
    {
        wtkMediaJNI.StopAudioPlayout();
    }
    public void StartVideoPlayout()
    {
        wtkMediaJNI.StartVideoPlayout();
    }
    public void StopVideoPlayout()
    {
        wtkMediaJNI.StopVideoPlayout();
    }
    public void SetVideoView(SurfaceView local, SurfaceView remote)
    {
        wtkMediaJNI.SetVideoView(local, remote);
    }
    public void SetVideoConfView(SurfaceView remote0,SurfaceView remote1,SurfaceView remote2,SurfaceView remote3)
    {
        wtkMediaJNI.SetVideoConfView(remote0,remote1,remote2,remote3);
    }
    public void StartVideoConf(int participant_ssrc)
    {
        wtkMediaJNI.StartVideoConf(participant_ssrc);
    }
    public void StopVideoConf()
    {
        wtkMediaJNI.StopVideoConf();
    }
    public int IaxRegister(String name, String number, String pass, String host, String port, int refresh)
    {
        return wtkMediaJNI.IaxRegister(name, number, pass, host, port, refresh);
    }


    public void IaxUnRegister(int regId)
    {
        wtkMediaJNI.IaxUnRegister(regId);
    }
    public int IaxDial(String dest, String host, String user, String cmd, String ext)
    {
        callNo = wtkMediaJNI.IaxDial(dest, host, user, cmd, ext);
        return callNo;
    }
    public void IaxAnswer()
    {
        Log.d(TAG, "IaxAnswer callNo = "+callNo);
        wtkMediaJNI.IaxAnswer(callNo);
    }
    public void CtrlConference(int type)
    {
        wtkMediaJNI.CtrlConference(callNo,type);
    }
    public void IaxHangup()
    {
        wtkMediaJNI.IaxHangup(callNo);
    }
    public void IaxSetHold(int hold)
    {
        wtkMediaJNI.IaxSetHold(callNo, hold);
    }
    public void SwitchCamera(int devideId)
    {
        wtkMediaJNI.SwitchCamera(devideId);
    }
    public void StartCapturer()
    {
        wtkMediaJNI.StartCapturer();
    }
    public void StopCapturer()
    {
        wtkMediaJNI.StopCapturer();
    }
    public void SetCapturerRotation(int rotation)
    {
        wtkMediaJNI.SetCapturerRotation(rotation);
    }
    public void ConfigVideoParams(int codec,int width,int height,int fps,int maxqp)
    {
        wtkMediaJNI.ConfigVideoParams(codec, width, height, fps, maxqp);
    }
    public void ConfigStreamBitrate(int audio_min_bps,int audio_max_bps, int video_min_bps, int video_max_bps)
    {
        wtkMediaJNI.ConfigStreamBitrate(audio_min_bps, audio_max_bps, video_min_bps, video_max_bps);
    }
    private WtkMediaJNIKit() {
        wtkMediaJNI = new WtkMediaJNI();
    }

    public String getCallNumber() {
        return callNumber;
    }
    public int getCallIaxNo() {
        return callNo;
    }
    private class MyHandler extends Handler {
        public void handleMessage(Message msg) {
            String jsonString = (String) msg.obj;
            jsonString = jsonString.replace("[", "^");
            jsonString = jsonString.replace("]", "*");
            try {
                JSONObject obj = new JSONObject(jsonString);
                switch (msg.what) {
                    case CommonParams.EVENT_REGISTRATION:
                        int regState = obj.getInt("reply");
                        Intent regintent = new Intent();
                        switch (regState){
                            case CommonParams.REGISTRATION_ACCEPT:
                                regintent.setAction("iax.reg.success");
                                Log.d(TAG, "Iax Registered Success!");
                                break;
                            case CommonParams.REGISTRATION_REJECT:
                                regintent.setAction("iax.reg.reject");
                                Log.d(TAG, "Iax Registered Reject!");
                                break;
                            case CommonParams.REGISTRATION_TIMEOUT:
                                regintent.setAction("iax.reg.timeout");
                                Log.d(TAG, "Iax Registered Timeout!");
                                break;
                        }
                        m_context.sendBroadcast(regintent);
                        break;

                    case CommonParams.EVENT_STATE:
                        int curState = obj.getInt("activity");
                        Intent stateintent = new Intent();
                        switch (curState){
                            case CommonParams.CALL_FREE:
                                stateintent.setAction("iax.hangup");
                                callNo = obj.getInt("callNo");
                                callNumber = obj.getString("peernum");
                                break;
                            case CommonParams.CALL_OUTGOING:
                                callNumber = obj.getString("peernum");
                                break;
                            case CommonParams.CALL_RINGIN:
                                callNo = obj.getInt("callNo");
                                callNumber = obj.getString("peernum");
                                stateintent.setAction("iax.new.call");
                                break;
                            case CommonParams.CALL_RINGBACK:
                                stateintent.setAction("iax.ringback");
                                break;
                            case CommonParams.CALL_ANSWERED:
                                stateintent.setAction("iax.answer");
                                callNumber = obj.getString("peernum");
                                break;
                            case CommonParams.CALL_TRANSFERED_RS:
                                stateintent.setAction("iax.transfer.rs");
                                callNumber = obj.getString("peernum");
                                break;
                            case CommonParams.CALL_TRANSFERED_NAT:
                                stateintent.setAction("iax.transfer.nat");
                                callNumber = obj.getString("peernum");
                                break;
                            case CommonParams.CALL_TRANSFERED_P2P:
                                stateintent.setAction("iax.transfer.p2p");
                                callNumber = obj.getString("peernum");
                                break;
                        }
                        m_context.sendBroadcast(stateintent);
                        break;
                    case CommonParams.EVENT_MESSAGE:
                        String message = obj.getString("msginfo");
                        Intent messageintent = new Intent();
                        switch (message){
                            case CommonParams.MSG_VIDEO_START:
                                messageintent.setAction("iax.video.start");
                                break;
                            case CommonParams.MSG_VIDEO_STOP:
                                messageintent.setAction("iax.video.stop");
                                break;
                        }
                        m_context.sendBroadcast(messageintent);
                        break;
                    default:
                        Log.d(TAG, "Unknow Message! ");
                        break;
                }
            }catch (JSONException e) {
                Log.e(TAG, "Wtk Call Error Message! ");
            }
        }
    }
    MyHandler myEventHandler = new MyHandler();
    @Override
    public void onWtkCallEvent(int type, String jsonString) {
        Log.d(TAG,"onWtkCallEvent type = " +type+" , json string = "+jsonString);
        Message msg = new Message();
        msg.what = type;
        msg.obj = jsonString;

        myEventHandler.sendMessage(msg);
    }
}
