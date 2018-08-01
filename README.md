# WebRTC API
WebRTC API based M67 release, will update&record code from time to time.
Sopport Linux & Mac platform video/audio loopback test.

##Note:
1. Sorry for my zero C++  skill, the code also need more test.

###Linux platform:
1. Get the WebRTC source code, please refer webrtc.org, or
    https://webrtc.org/native-code/android/ for add Android build tools, and
    https://webrtc.org/native-code/ios/ for add iOS build tools.
    *NOTE: For Linux platform loopback test, need gen project like: gn gen out/Linux --args='is_debug=false' or set RTC_DCHECK_IS_ON macro define to 0 at src/rtc_base/checks.h, about the side effect will check later.*
2. Compile the source code according the web site above.
3. Add this api code dircotry to WebRTC src/ dictory.
4. Modify src/BUILD.gn file for add compile the api code. eg. add this line deps += [ "webrtc_api_linux_mac:wtk_rtc_api" ] on some useful place.
5. Compile it, the bin file will gen at out/Linux/ dirctory, just run it
6. Have fun.

###iOS platform:
TBD
###Android platform:
First commit
