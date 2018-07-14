package com.wtk.mobile.WtkRtc;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.wtk.mobile.jni.CommonParams;
import com.wtk.mobile.jni.WtkMediaJNIKit;

import org.webrtc.ContextUtils;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

public class MainActivity extends AppCompatActivity implements OnClickListener {
    private String TAG="LoginActivity";
    private Button mDialButton,mAnswerButton,mHangupButton;
    private Button mIsSpeakerButton,mIsHoldButton,mIsMuteButton;
    private SharedPreferences m_sp = null;
    private int regID = 0;
    private int isCaller = 0;
    private String host = null;
    private String port = null;
    private String userid = null;
    private String password = null;
    private EditText editDest;

    private String mStateInfo = "空闲中";
    private TextView txtLocalIp,txtState;
    private int mCurrentSpeaker = 0;
    private int mCurrentMute = 0;
    private int mCurrentHold = 0;

    private CallReceiver receiver = null;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getSharedPreferences();

        editDest = findViewById(R.id.edit_number);

        mDialButton = findViewById(R.id.btn_dial);
        mAnswerButton = findViewById(R.id.btn_answer);
        mHangupButton = findViewById(R.id.btn_hangup);

        mIsSpeakerButton = findViewById(R.id.btn_is_speaker);
        mIsHoldButton = findViewById(R.id.btn_is_hold);
        mIsMuteButton = findViewById(R.id.btn_is_mute);

        mDialButton.setOnClickListener(this);
        mAnswerButton.setOnClickListener(this);
        mHangupButton.setOnClickListener(this);

        mIsSpeakerButton.setOnClickListener(this);
        mIsHoldButton.setOnClickListener(this);
        mIsMuteButton.setOnClickListener(this);

        txtLocalIp = findViewById(R.id.txtNetInfo);
        txtState = findViewById(R.id.txtState);

        txtLocalIp.setText(GetLocalIpAddress());
        txtState.setText(mStateInfo);

        ContextUtils.initialize(MainActivity.this);
        //WtkMediaJNIKit.getInstance().MediaInitialize();
        WtkMediaJNIKit.getInstance().SetBroadCast(MainActivity.this);
    }

    @Override
    public void onClick(View paramView){
        switch (paramView.getId())
        {
            case R.id.btn_dial:
                String calleeId = editDest.getText().toString();
                String userIdPass = userid + ":" + password;
                String cmd;
                isCaller = 1;
                if(calleeId.contains("80")||calleeId.equals("8888")) {
                    cmd = "/nortp/forward";
                } else {
                    cmd = "/forward";
                }
                String ext = "testExt";
                Log.d(TAG, "IaxDial params is " +calleeId+",host="+host+"userIdPass="+userIdPass+"cmd="+cmd+"ext="+ext);
                WtkMediaJNIKit.getInstance().IaxDial(calleeId,host,userIdPass,cmd,ext);
                mStateInfo = "向 "+calleeId+"发起呼叫，正在接通中...";
                setActivityMode(CommonParams.STATERINGIN);
                break;
            case R.id.btn_answer:
                WtkMediaJNIKit.getInstance().IaxAnswer();
                mStateInfo = "正在和 "+WtkMediaJNIKit.getInstance().getCallNumber()+"语音通话中...";
                setActivityMode(CommonParams.STATETALKING);
                break;
            case R.id.btn_hangup:
                WtkMediaJNIKit.getInstance().IaxHangup();
                mStateInfo = "空闲中";
                setActivityMode(CommonParams.STATEIDEL);
                break;
            case R.id.btn_is_speaker:
                mCurrentSpeaker = (mCurrentSpeaker+1)%2;
                if (mCurrentSpeaker == 0) {
                    mIsSpeakerButton.setText("Speaker");
//                WtkMediaJNIKit.getInstance().setspeaker(mCurrentSpeaker);
                } else {
                    mIsSpeakerButton.setText("Headphone");
                }
                break;
            case R.id.btn_is_hold:
                mCurrentHold = (mCurrentHold+1)%2;
                if(mCurrentHold == 0) {
                    mIsHoldButton.setText("Hold");
//                WtkMediaJNIKit.getInstance().IaxHold();
                } else {
                    mIsHoldButton.setText("UnHold");
                }
                break;
            case R.id.btn_is_mute:
                mCurrentMute = (mCurrentMute+1)%2;
                if(mCurrentMute == 0) {
                    mIsMuteButton.setText("Mute");
//                WtkMediaJNIKit.getInstance().IaxMute();
                } else {
                    mIsMuteButton.setText("UnMute");
                }
                break;
        }
    }

    @Override
    protected void onStart()
    {
        super.onStart();
        IntentFilter filter = new IntentFilter();
        filter.addAction("iax.new.call");
        filter.addAction("iax.hangup");
        filter.addAction("iax.answer");

        receiver = new CallReceiver();
        registerReceiver(receiver, filter);
    }
    private void getSharedPreferences()
    {
        m_sp = getSharedPreferences("setting", Context.MODE_PRIVATE);
        if(m_sp == null)
            return;

        regID = m_sp.getInt("iax_regid", 0);
        host = m_sp.getString("iax_host", null);
        port = m_sp.getString("iax_port", null);
        userid = m_sp.getString("iax_user", null);
        password = m_sp.getString("iax_pass", null);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "IaxUnRegister, ret = " + regID);
        WtkMediaJNIKit.getInstance().IaxUnRegister(regID);
        super.onDestroy();
    }
    private void setActivityMode(int mode)
    {
        switch(mode)
        {
            case CommonParams.STATEIDEL:
                mDialButton.setEnabled(true);
                mAnswerButton.setEnabled(false);
                mHangupButton.setEnabled(false);
                mIsHoldButton.setEnabled(false);
                mIsSpeakerButton.setEnabled(false);
                mIsMuteButton.setEnabled(false);
                txtState.setText(mStateInfo);
                //Init some init params
                isCaller = 0;
                mCurrentHold = 0;
                mIsHoldButton.setText("Hold");
                mCurrentMute = 0;
                mIsMuteButton.setText("Mute");
                mCurrentSpeaker = 0;
                mIsSpeakerButton.setText("Speaker");
                break;
            case CommonParams.STATERINGIN:
                mDialButton.setEnabled(false);
                if(isCaller == 1)
                    mAnswerButton.setEnabled(false);
                else
                    mAnswerButton.setEnabled(true);
                mHangupButton.setEnabled(true);
                mIsHoldButton.setEnabled(false);
                mIsSpeakerButton.setEnabled(false);
                mIsMuteButton.setEnabled(false);
                txtState.setText(mStateInfo);
                break;
            case CommonParams.STATETALKING:
                mDialButton.setEnabled(false);
                mAnswerButton.setEnabled(false);
                mHangupButton.setEnabled(true);
                mIsHoldButton.setEnabled(true);
                mIsSpeakerButton.setEnabled(true);
                mIsMuteButton.setEnabled(true);
                txtState.setText(mStateInfo);
                break;
        }
    }
    private class CallReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if("iax.hangup".equals(action)) {
                mStateInfo = "空闲中";
                setActivityMode(CommonParams.STATEIDEL);
            }
            else if("iax.new.call".equals(action)) {
                mStateInfo = "收到来自 "+WtkMediaJNIKit.getInstance().getCallNumber()+"的电话，是否接听...";
                setActivityMode(CommonParams.STATERINGIN);
            }
            else if("iax.answer".equals(action))
            {
                mStateInfo = "正在和 "+WtkMediaJNIKit.getInstance().getCallNumber()+"语音通话中...";
                setActivityMode(CommonParams.STATETALKING);
            }
            else
            {
                mStateInfo = "空闲中";
                setActivityMode(CommonParams.STATEIDEL);
            }
        }
    }
    public String GetLocalIpAddress() {
        String localIPs = "";
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface
                    .getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf
                        .getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        localIPs += inetAddress.getHostAddress().toString() + " ";
                    }
                }
            }
        } catch (SocketException ex) {
            Log.e(TAG, ex.toString());
        }
        return localIPs;
    }
}