package com.wtk.mobile.jni;

public class WtkMediaJNI {
    public interface StatusListener
    {
        public void onWtkCallEvent(int type, String jsonString );
    }
    public native int IaxInitialize(StatusListener listener);
    public native int MediaInitialize();
    public native void IaxShutdown();
    public native void MediaShutdown();
    public native int IaxRegister(String name, String number, String pass, String host, String port);
    public native void IaxUnRegister(int regId);
    public native int IaxDial(String dest, String host, String user, String cmd, String ext);
    public native void IaxAnswer(int callNo);
    public native void IaxHangup(int callNo);
    public native void IaxHold(int callNo, int hold);
    public native void IaxMute(int callNo, int mute);
    public native void IaxSetFormat(int callNo, int rtp_format);
}