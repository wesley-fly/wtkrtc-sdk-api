#!/bin/sh

if [ $1 == "mixer" ]; then
  ninja -C out/Linux/ wtk-mixer
  #cp out/Linux/libWtkMediaEngineMixer.so /home/lee/Downloads/NewVoIP/wtk-mixserver-1.0/
elif [ $1 == "jni" ]; then
  ninja -C out/Android/ WtkMediaEngineJni 
  cp out/Android/libWtkMediaEngineJni.so ~/Github/WebRTC-API/webrtc_api_android/WtkRtc/app/src/main/jniLibs/armeabi-v7a/libWtkMediaEngineJni.so
elif [ $1 == "sdk" ]; then
  ninja -C out/Android/ WtkMediaSDKJni
  cp out/Android/libWtkMediaSDKJni.so ~/Github/WtkMediaSDK/app/src/main/jniLibs/armeabi-v7a/libWtkMediaSDKJni.so

  cp out/Android/libWtkMediaSDKJni.so /home/lee/Github/Loop_Android/app/src/main/jniLibs/armeabi/
  cp out/Android/libWtkMediaSDKJni.so /home/lee/Github/Loop_Android/app/src/main/jniLibs/armeabi-v7a/libWtkMediaSDKJni.so

  cp out/Android/libWtkMediaSDKJni.so /home/lee/Teemy_Android/app/src/main/jniLibs/armeabi/
  cp out/Android/libWtkMediaSDKJni.so /home/lee/Teemy_Android/app/src/main/jniLibs/armeabi-v7a/
else
  echo -e "At least 1 param should be [mixer|jni|sdk]"
fi

