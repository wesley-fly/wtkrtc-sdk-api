package com.wtk.mobile.WtkRtc;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import com.wtk.mobile.jni.WtkMediaJNIKit;


public class LoginActivity extends AppCompatActivity implements OnClickListener{
    private String TAG="LoginActivity";
    private SharedPreferences m_sp = null;
    private Button mSignInButton;
    private EditText txtServer,txtPort,txtName,txtPass;
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        mSignInButton = findViewById(R.id.sign_in_button);
        txtServer = findViewById(R.id.iaxserver);
        txtPort = findViewById(R.id.iaxport);
        txtName = findViewById(R.id.iaxuser);
        txtPass = findViewById(R.id.iaxpass);

        mSignInButton.setOnClickListener(this);
        WtkMediaJNIKit.getInstance().IaxInitialize();
    }

    @Override
    public void onClick(View paramView){
        int retId;
        switch (paramView.getId())
        {
            case R.id.sign_in_button:
                retId = signInProcess(txtServer.getText().toString(),txtPort.getText().toString(),txtName.getText().toString(),txtPass.getText().toString());
                if(retId > 0) {
                    Intent MainInterface = new Intent(LoginActivity.this, MainActivity.class);
                    MainInterface.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    startActivity(MainInterface);
                }
                break;
        }
    }

    private int signInProcess(String server, String port, String name, String pass)
    {
        int retId;
        if(server == null || port == null || name == null || pass == null)
        {
            Log.e(TAG, "Sign in info is empty!");
        }

        retId = WtkMediaJNIKit.getInstance().IaxRegister("029-88"+name, name, pass, server,port);
        if(retId > 0)
        {
            setSharedPreferences(retId,server,name,port,pass);
            Log.d(TAG, "Sign in Success, retId = " + retId);
        }
        return retId;
    }

    private void setSharedPreferences(int redId, String host, String name, String port, String passwprd)
    {
        m_sp = getSharedPreferences("setting", Context.MODE_PRIVATE );
        if(m_sp == null)
            return;
        SharedPreferences.Editor ed = m_sp.edit();
        ed.putInt("iax_regid",redId);
        ed.putString("iax_host", host);
        ed.putString("iax_port", port);
        ed.putString("iax_user", name);
        ed.putString("iax_pass", passwprd);
        ed.commit();
    }
}

