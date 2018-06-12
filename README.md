# WebRTC API
WebRTC SDK base M67 release, will update&record code from time to time.
#Note:
1. Video will add later;
2. Sorry for my NONE C++  skill, the code also need more test.
3. Also will improve the SDK API according my VoIP.

Use mothod
Linux platform:
1. Get the WebRTC source code, please refer webrtc.org, or 
    https://webrtc.org/native-code/android/
    https://webrtc.org/native-code/ios/
    NOTE: For Linux platform loopback test, need gen project like: gn gen out/Linux --args='is_debug=false'
2. Compile the source code according the web site above.
3. Add this api code dircotry to WebRTC src/ dictory.
4. Modify src/BUILD.gn file for add compile the api code. eg. add this line deps += [ "webrtc_api_linux:wtk_rtc_api" ] on some useful place.
5. Compile it, the bin file will gen at out/Linux/ dirctory, just run it
6. Have fun.

iOS platform:
TBD
Android platform:
TBD
