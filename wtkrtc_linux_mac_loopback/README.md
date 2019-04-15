### usage:
+ Copy this directory to WebRTC source code directory
```$cp -rf wtkrtc_linux_mac_loopback path/to/webrtc/src/```
+ Edit WebRTC's BUILD.gn files
```
 91     } else {
 92       deps += [
 93         "wtkrtc_linux_mac_loopback:wtkrtc_api_test"
 94       ]
 95     }
```
+ Generate build project
```gn gen out/Linux```
+ Build project
```ninja -C out/Linux wtkrtc_api_test```
+ The generated bin file at out/Linux/, file name is wtkrtc_api_test.
+ Run this bin file.

