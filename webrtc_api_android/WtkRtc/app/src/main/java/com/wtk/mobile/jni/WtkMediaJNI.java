package com.wtk.mobile.jni;

public class WtkMediaJNI {
    public interface StatusListener
    {
        public void onWtkCallEvent(int type, String jsonString );
    }
    public native int       IaxInitialize(StatusListener listener);
    public native void      IaxShutdown();
    public native void      StartAudioPlayout();
    public native void      StopAudioPlayout();
    public native void      StartVideoPlayout();
    public native void      StopVideoPlayout();
    public native int       IaxRegister(String name, String number, String pass, String host, String port);
    public native void      IaxUnRegister(int regId);
    public native int       IaxDial(String dest, String host, String user, String cmd, String ext);
    public native void      IaxAnswer(int callNo);
    public native int       IaxHangup(int callNo);
    public native void      IaxSetHold(int callNo, int hold);
    public native void      IaxSetFormat(int callNo, int rtp_format);
}