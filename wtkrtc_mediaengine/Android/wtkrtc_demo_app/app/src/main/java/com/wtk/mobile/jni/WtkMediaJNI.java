package com.wtk.mobile.jni;

import android.content.Context;

public class WtkMediaJNI {
    public interface StatusListener
    {
        public void onWtkCallEvent(int type, String jsonString );
    }
    public native int       IaxInitialize(StatusListener listener,Context context);
    public native void      IaxShutdown();
    public native void      StartAudioPlayout();
    public native void      StopAudioPlayout();
    public native void      StartVideoPlayout();
    public native void      StopVideoPlayout();
    public native int       IaxRegister(String name, String number, String pass, String host, String port, int refresh);
    public native void      IaxUnRegister(int regId);
    public native int       IaxDial(String dest, String host, String user, String cmd, String ext);
    public native void      IaxAnswer(int callNo);
    public native int       IaxHangup(int callNo);
    public native void      IaxSetHold(int callNo, int hold);
    public native void      SetVideoView(Object LocalSurface, Object RemoteSurface);
    public native void      SwitchCamera(int devideId);
    public native void      StartCapturer();
    public native void      StopCapturer();
    public native void      SetCapturerRotation(int rotation);
    public native void      ConfigVideoParams(int codec,int width,int height,int fps,int maxqp);
    public native void      ConfigStreamBitrate(int audio_min_bps,int audio_max_bps, int video_min_bps, int video_max_bps);
    public native int       CtrlConference(int callNo, int type);
    public native void      SetVideoConfView(Object RemoteSurface0,Object RemoteSurface1,Object RemoteSurface2,Object RemoteSurface3);
    public native void      StartVideoConf(int participant_ssrc);
    public native void      StopVideoConf();
}