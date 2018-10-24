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

import static com.wtk.mobile.jni.CommonParams.VIDEO_CODEC_H264;
import static com.wtk.mobile.jni.CommonParams.VIDEO_CODEC_H264_AUTO;

public class VideoConfActivity extends AppCompatActivity implements View.OnClickListener{
    private String TAG="VideoConfActivity";
    private LinearLayout mLocalLinearLayout = null,mRemoteLinearLayout0 = null,mRemoteLinearLayout1 = null,mRemoteLinearLayout2 = null,mRemoteLinearLayout3 = null;
    private SurfaceView mLocalSurfaceView = null, mRemoteSurfaceView0 = null,mRemoteSurfaceView1 = null,mRemoteSurfaceView2 = null,mRemoteSurfaceView3 = null;
    private HangupReceiver hangupreceiver = null;
    private Button mIsStartVideo,mSwitchCameraBtn,mHangup;
    private int mCurrentCamera = 1; //Front
    private int mIsCameraStarted = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_videoconf);
        mLocalLinearLayout = findViewById(R.id.ll_local_view);
        mRemoteLinearLayout0 = findViewById(R.id.ll_remote_view_0);
        mRemoteLinearLayout1 = findViewById(R.id.ll_remote_view_1);
        mRemoteLinearLayout2 = findViewById(R.id.ll_remote_view_2);
        mRemoteLinearLayout3 = findViewById(R.id.ll_remote_view_3);

        mIsStartVideo = findViewById(R.id.btn_conf_is_start_video);
        mSwitchCameraBtn = findViewById(R.id.btn_conf_switch_camera);
        mHangup = findViewById(R.id.btn_conf_hangup);

        mIsStartVideo.setOnClickListener(this);
        mSwitchCameraBtn.setOnClickListener(this);
        mHangup.setOnClickListener(this);
        WtkMediaJNIKit.getInstance().ConfigVideoParams(VIDEO_CODEC_H264,176,144,15,50);
        WtkMediaJNIKit.getInstance().ConfigStreamBitrate(16*1000,32*1000,64*1000,800*1000);
        PrepareVideoConfRender();

        WtkMediaJNIKit.getInstance().StartVideoConf(20000000);

        //local start capture
        WtkMediaJNIKit.getInstance().StartCapturer();
        WtkMediaJNIKit.getInstance().SetCapturerRotation(270);
    }
    @Override
    public void onClick(View paramView){
        switch (paramView.getId())
        {
            case R.id.btn_conf_is_start_video:
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
            case R.id.btn_conf_switch_camera:
                mCurrentCamera = (mCurrentCamera+1)%2;
                WtkMediaJNIKit.getInstance().SwitchCamera(mCurrentCamera);
                break;
            case R.id.btn_conf_hangup:
                WtkMediaJNIKit.getInstance().IaxHangup();
                break;
        }
    }
    @Override
    protected void onStart()
    {
        super.onStart();
        IntentFilter filter = new IntentFilter();
        filter.addAction("iax.hangup");
        hangupreceiver = new VideoConfActivity.HangupReceiver();
        registerReceiver(hangupreceiver, filter);
    }
    @Override
    protected void onDestroy() {
        unregisterReceiver(hangupreceiver);
        super.onDestroy();
    }
    private class HangupReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if("iax.hangup".equals(action)) {
                finish();
            }
        }
    }
    private void PrepareVideoConfRender()
    {
        mLocalSurfaceView = ViERenderer.CreateRenderer(VideoConfActivity.this, false);
        if(mLocalLinearLayout != null)
        {
            ViewGroup.LayoutParams params = mLocalLinearLayout.getLayoutParams();
            ViewGroup.LayoutParams mparams = new ViewGroup.LayoutParams(params.width, params.height);
            mLocalLinearLayout.setPadding(1, 1, 1, 1);
            mLocalSurfaceView.setZOrderMediaOverlay(true);
            mLocalLinearLayout.addView(mLocalSurfaceView,mparams);
            VideoCaptureAndroid.setLocalPreview(mLocalSurfaceView.getHolder());
        }
        mRemoteSurfaceView0 = ViERenderer.CreateRenderer(VideoConfActivity.this,true);
        if(mRemoteLinearLayout0 != null)
        {
            mRemoteLinearLayout0.addView(mRemoteSurfaceView0);
        }
        mRemoteSurfaceView1 = ViERenderer.CreateRenderer(VideoConfActivity.this,true);
        if(mRemoteLinearLayout1 != null)
        {
            mRemoteLinearLayout1.addView(mRemoteSurfaceView1);
        }
        mRemoteSurfaceView2 = ViERenderer.CreateRenderer(VideoConfActivity.this,true);
        if(mRemoteLinearLayout2 != null)
        {
            mRemoteLinearLayout2.addView(mRemoteSurfaceView2);
        }
        mRemoteSurfaceView3 = ViERenderer.CreateRenderer(VideoConfActivity.this,true);
        if(mRemoteLinearLayout3 != null)
        {
            mRemoteLinearLayout3.addView(mRemoteSurfaceView3);
        }
        WtkMediaJNIKit.getInstance().SetVideoConfView(mRemoteSurfaceView0,mRemoteSurfaceView1,mRemoteSurfaceView2,mRemoteSurfaceView3);
    }
}
