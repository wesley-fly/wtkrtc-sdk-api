package com.wtk.mobile.jni;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import com.wtk.mobile.WtkRtc.MainActivity;
import com.wtk.mobile.jni.WtkMediaJNI.StatusListener;

import org.json.JSONException;
import org.json.JSONObject;

import static org.webrtc.ContextUtils.getApplicationContext;

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
    public int IaxInitialize()
    {
        return wtkMediaJNI.IaxInitialize(this);
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
    public int IaxRegister(String name, String number, String pass, String host, String port)
    {
        return wtkMediaJNI.IaxRegister(name, number, pass, host, port);
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
    public void IaxHangup()
    {
        wtkMediaJNI.IaxHangup(callNo);
    }
    public void IaxSetHold(int hold)
    {
        wtkMediaJNI.IaxSetHold(callNo, hold);
    }
    public void IaxSetFormat(int rtp_format)
    {
        wtkMediaJNI.IaxSetFormat(callNo, rtp_format);
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
                        }
                        m_context.sendBroadcast(stateintent);
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
