package com.wtk.mobile.WtkRtc;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;

import com.wtk.mobile.jni.WtkMediaJNIKit;

import org.webrtc.videoengine.ViERenderer;
import org.webrtc.videoengine.VideoCaptureAndroid;


public class VideoActivity extends AppCompatActivity implements View.OnClickListener {
    private String TAG="VideoActivity";
    private LinearLayout mLocalLinearLayout = null,mRemoteLinearLayout = null;
    private SurfaceView mLocalSurfaceView = null, mRemoteSurfaceView = null;
    private Button mIsStartVideo,mSwitchCameraBtn,mSwitchRotation;
    private Button mHangup,mBackAudio,mTest;

    private int mCurrentCamera = 1; //Front
    private int mIsCameraStarted = 1;
    private int mCurrentRotation = 0;

    private HangupReceiver hangupreceiver = null;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);

        mLocalLinearLayout = findViewById(R.id.ll_local_view);
        mRemoteLinearLayout = findViewById(R.id.ll_remote_view);

        mIsStartVideo = findViewById(R.id.btn_is_start_video);
        mSwitchCameraBtn = findViewById(R.id.btn_switch_camera);
        mSwitchRotation = findViewById(R.id.btn_switch_rotation);
        mHangup = findViewById(R.id.btn_hangup);
        mBackAudio = findViewById(R.id.btn_back_audio);
        mTest = findViewById(R.id.btn_back_audio1);

        mIsStartVideo.setOnClickListener(this);
        mSwitchCameraBtn.setOnClickListener(this);
        mSwitchRotation.setOnClickListener(this);
        mHangup.setOnClickListener(this);
        mBackAudio.setOnClickListener(this);
        mTest.setOnClickListener(this);

        PrepareVideoRender();
        WtkMediaJNIKit.getInstance().StartAudioPlayout();
        WtkMediaJNIKit.getInstance().StartVideoPlayout();

        WtkMediaJNIKit.getInstance().StartCapturer();
        WtkMediaJNIKit.getInstance().SetCapturerRotation(270);
        mSwitchRotation.setText("270");
    }
    @Override
    protected void onStart()
    {
        super.onStart();
        IntentFilter filter = new IntentFilter();
        filter.addAction("iax.hangup");
        filter.addAction("iax.video.start");
        filter.addAction("iax.video.stop");
        hangupreceiver = new HangupReceiver();
        registerReceiver(hangupreceiver, filter);
    }
    @Override
    public void onClick(View paramView){
        switch (paramView.getId())
        {
            case R.id.btn_is_start_video:
                mIsCameraStarted = (mIsCameraStarted+1)%2;
                if(mIsCameraStarted == 1) {
                    WtkMediaJNIKit.getInstance().StartCapturer();
                    mIsStartVideo.setText("Stop Video");
                }
                else {
                    WtkMediaJNIKit.getInstance().StopCapturer();
                    mIsStartVideo.setText("Start Video");
                }
                break;
            case R.id.btn_switch_camera:
                mCurrentCamera = (mCurrentCamera+1)%2;
                WtkMediaJNIKit.getInstance().SwitchCamera(mCurrentCamera);
                break;
            case R.id.btn_switch_rotation:
                mCurrentRotation = (mCurrentRotation+1)%4;
                if(mCurrentRotation == 0) {
                    WtkMediaJNIKit.getInstance().SetCapturerRotation(0);
                    mSwitchRotation.setText("0");
                }
                else if(mCurrentRotation == 1) {
                    WtkMediaJNIKit.getInstance().SetCapturerRotation(90);
                    mSwitchRotation.setText("90");
                }
                else if(mCurrentRotation == 2) {
                    WtkMediaJNIKit.getInstance().SetCapturerRotation(180);
                    mSwitchRotation.setText("180");
                }
                else if(mCurrentRotation == 3) {
                    WtkMediaJNIKit.getInstance().SetCapturerRotation(270);
                    mSwitchRotation.setText("270");
                }
                break;
            case R.id.btn_hangup:
                WtkMediaJNIKit.getInstance().IaxHangup();
                break;
            case R.id.btn_back_audio:
                WtkMediaJNIKit.getInstance().StopCapturer();
                WtkMediaJNIKit.getInstance().StopVideoPlayout();
                finish();
                break;
            case R.id.btn_back_audio1:
                break;
        }
    }
    @Override
    protected void onDestroy() {
        unregisterReceiver(hangupreceiver);
        super.onDestroy();
    }

    private void PrepareVideoRender()
    {
        mLocalSurfaceView = ViERenderer.CreateRenderer(VideoActivity.this, false);
        if(mLocalLinearLayout != null)
        {
            ViewGroup.LayoutParams params = mLocalLinearLayout.getLayoutParams();
            ViewGroup.LayoutParams mparams = new ViewGroup.LayoutParams(params.width, params.height);
            mLocalLinearLayout.setPadding(1, 1, 1, 1);
            mLocalSurfaceView.setZOrderMediaOverlay(true);
            mLocalLinearLayout.addView(mLocalSurfaceView,mparams);
            VideoCaptureAndroid.setLocalPreview(mLocalSurfaceView.getHolder());
        }
        mRemoteSurfaceView = ViERenderer.CreateRenderer(VideoActivity.this,true);
        if(mRemoteLinearLayout != null)
        {
            mRemoteLinearLayout.addView(mRemoteSurfaceView);
        }
        WtkMediaJNIKit.getInstance().SetVideoView(mLocalSurfaceView, mRemoteSurfaceView);
    }
    private class HangupReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if("iax.hangup".equals(action)) {
                finish();
            }
            else if("iax.video.start".equals(action))
            {
                //
            }
            else if("iax.video.stop".equals(action))
            {
                //finish();
            }
        }
    }
}
