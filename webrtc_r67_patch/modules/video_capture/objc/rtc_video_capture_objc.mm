/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

#import <AVFoundation/AVFoundation.h>
#ifdef WEBRTC_IOS
#import <UIKit/UIKit.h>
#endif

#import "modules/video_capture/objc/device_info_objc.h"
#import "modules/video_capture/objc/rtc_video_capture_objc.h"

#include "rtc_base/logging.h"

using namespace webrtc;
using namespace webrtc::videocapturemodule;

@interface RTCVideoCaptureIosObjC (hidden)
- (int)changeCaptureInputWithName:(NSString*)captureDeviceName;
@end

@implementation RTCVideoCaptureIosObjC {
  webrtc::videocapturemodule::VideoCaptureIos* _owner;
  webrtc::VideoCaptureCapability _capability;
  AVCaptureSession* _captureSession;
  BOOL _orientationHasChanged;
  AVCaptureConnection* _connection;
  BOOL _captureChanging;  // Guarded by _captureChangingCondition.
  NSCondition* _captureChangingCondition;

  UIView* _preview_UIView;
  void* _local_View;
  AVCaptureVideoPreviewLayer* _preview_layer;
  unsigned char* _tmp_frame_buf;
}

@synthesize frameRotation = _framRotation;

- (id)initWithOwner:(VideoCaptureIos*)owner {
  if (self = [super init]) {
    _owner = owner;
    _captureSession = [[AVCaptureSession alloc] init];
#if defined(WEBRTC_IOS)
    _captureSession.usesApplicationAudioSession = NO;
#endif
    _captureChanging = NO;
    _captureChangingCondition = [[NSCondition alloc] init];

    if (!_captureSession || !_captureChangingCondition) {
      return nil;
    }
	_preview_UIView = nil;
	_preview_layer = nil;
	_local_View = NULL;
	_tmp_frame_buf = NULL;

    // create and configure a new output (using callbacks)
    AVCaptureVideoDataOutput* captureOutput = [[AVCaptureVideoDataOutput alloc] init];
	
    NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey;
    //kCVPixelFormatType_32BGRA
    NSNumber* val = [NSNumber numberWithUnsignedInt:kCVPixelFormatType_420YpCbCr8BiPlanarFullRange];
    NSDictionary* videoSettings = [NSDictionary dictionaryWithObject:val forKey:key];
    captureOutput.videoSettings = videoSettings;

    // add new output
    if ([_captureSession canAddOutput:captureOutput]) {
      [_captureSession addOutput:captureOutput];
    } else {
      RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Could not add output to AVCaptureSession";
    }
//Below for IOS auto rotation SDK not use it
/*
#ifdef WEBRTC_IOS
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];

    NSNotificationCenter* notify = [NSNotificationCenter defaultCenter];
    [notify addObserver:self
               selector:@selector(onVideoError:)
                   name:AVCaptureSessionRuntimeErrorNotification
                 object:_captureSession];
    [notify addObserver:self
               selector:@selector(deviceOrientationDidChange:)
                   name:UIDeviceOrientationDidChangeNotification
                 object:nil];
#endif
*/
  }

  return self;
}

- (void)directOutputToSelf {
  [[self currentOutput]
      setSampleBufferDelegate:self
                        queue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)];
}

- (void)directOutputToNil {
  [[self currentOutput] setSampleBufferDelegate:nil queue:NULL];
}

- (void)deviceOrientationDidChange:(NSNotification*)notification {
  _orientationHasChanged = YES;
  [self setRelativeVideoOrientation];
}

- (void)dealloc {
  if(_tmp_frame_buf)
  {
    free(_tmp_frame_buf);
	_tmp_frame_buf = NULL;
  }
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (BOOL)setCaptureDeviceByUniqueId:(NSString*)uniqueId {
  [self waitForCaptureChangeToFinish];
  // check to see if the camera is already set
  if (_captureSession) {
    NSArray* currentInputs = [NSArray arrayWithArray:[_captureSession inputs]];
    if ([currentInputs count] > 0) {
      AVCaptureDeviceInput* currentInput = [currentInputs objectAtIndex:0];
      if ([uniqueId isEqualToString:[currentInput.device localizedName]]) {
        return YES;
      }
    }
  }

  return [self changeCaptureInputByUniqueId:uniqueId];
}

- (BOOL)startCaptureWithCapability:(const VideoCaptureCapability&)capability {
  [self waitForCaptureChangeToFinish];
  if (!_captureSession) {
    return NO;
  }

  // check limits of the resolution
  if (capability.maxFPS < 0 || capability.maxFPS > 60) {
    return NO;
  }

  if ([_captureSession canSetSessionPreset:AVCaptureSessionPreset1280x720]) {
    if (capability.width > 1280 || capability.height > 720) {
      return NO;
    }
  } else if ([_captureSession canSetSessionPreset:AVCaptureSessionPreset640x480]) {
    if (capability.width > 640 || capability.height > 480) {
      return NO;
    }
  } else if ([_captureSession canSetSessionPreset:AVCaptureSessionPreset352x288]) {
    if (capability.width > 352 || capability.height > 288) {
      return NO;
    }
  } else if ([_captureSession canSetSessionPreset:AVCaptureSessionPresetLow]) {
    if (capability.width > 176 || capability.height > 144) {
      return NO;
    }
  } else if (capability.width < 0 || capability.height < 0) {
    return NO;
  }

  _capability = capability;

  AVCaptureVideoDataOutput* currentOutput = [self currentOutput];
  if (!currentOutput) return NO;

  [self directOutputToSelf];

  _orientationHasChanged = NO;
  _captureChanging = YES;
  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
    [self startCaptureInBackgroundWithOutput:currentOutput];
  });
  return YES;
}

- (AVCaptureVideoDataOutput*)currentOutput {
  return [[_captureSession outputs] firstObject];
}

- (void)startCaptureInBackgroundWithOutput:(AVCaptureVideoDataOutput*)currentOutput {
  NSString* captureQuality = [NSString stringWithString:AVCaptureSessionPresetLow];
  if (_capability.width >= 1280 || _capability.height >= 720) {
    captureQuality = [NSString stringWithString:AVCaptureSessionPreset1280x720];
  } else if (_capability.width >= 640 || _capability.height >= 480) {
    captureQuality = [NSString stringWithString:AVCaptureSessionPreset640x480];
  } else if (_capability.width >= 352 || _capability.height >= 288) {
    captureQuality = [NSString stringWithString:AVCaptureSessionPreset352x288];
  } else if (_capability.width >= 176 || _capability.height >= 144) {
    captureQuality = [NSString stringWithString:AVCaptureSessionPresetLow];
  }

  // begin configuration for the AVCaptureSession
  [_captureSession beginConfiguration];

  // picture resolution
  [_captureSession setSessionPreset:captureQuality];

  _connection = [currentOutput connectionWithMediaType:AVMediaTypeVideo];
  //[self setRelativeVideoOrientation];

  // finished configuring, commit settings to AVCaptureSession.
  [_captureSession commitConfiguration];

  [_captureSession startRunning];
  [self signalCaptureChangeEnd];
}

- (void)setRelativeVideoOrientation {
  if (!_connection.supportsVideoOrientation) {
    return;
  }
#ifndef WEBRTC_IOS
  _connection.videoOrientation = AVCaptureVideoOrientationLandscapeRight;
  return;
#else
  switch ([UIDevice currentDevice].orientation) {
    case UIDeviceOrientationPortrait:
      _connection.videoOrientation = AVCaptureVideoOrientationPortrait;
      break;
    case UIDeviceOrientationPortraitUpsideDown:
      _connection.videoOrientation = AVCaptureVideoOrientationPortraitUpsideDown;
      break;
    case UIDeviceOrientationLandscapeLeft:
      _connection.videoOrientation = AVCaptureVideoOrientationLandscapeRight;
      break;
    case UIDeviceOrientationLandscapeRight:
      _connection.videoOrientation = AVCaptureVideoOrientationLandscapeLeft;
      break;
    case UIDeviceOrientationFaceUp:
    case UIDeviceOrientationFaceDown:
    case UIDeviceOrientationUnknown:
      if (!_orientationHasChanged) {
        _connection.videoOrientation = AVCaptureVideoOrientationPortrait;
      }
      break;
  }
#endif
}

- (void)onVideoError:(NSNotification*)notification {
  NSLog(@"onVideoError: %@", notification);
  // TODO(sjlee): make the specific error handling with this notification.
  RTC_LOG(LS_ERROR) << __FUNCTION__ << ": [AVCaptureSession startRunning] error.";
}

- (BOOL)stopCapture {
#ifdef WEBRTC_IOS
  [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
#endif
  _orientationHasChanged = NO;
  [self waitForCaptureChangeToFinish];
  [self directOutputToNil];

  if (!_captureSession) {
    return NO;
  }

  _captureChanging = YES;
  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void) {
    [self stopCaptureInBackground];
  });

  /*_preview_UIView = nil;
  _preview_layer = nil;
  _local_View = NULL;*/
	
  return YES;
}

- (void)stopCaptureInBackground {
  [_captureSession stopRunning];
  [self signalCaptureChangeEnd];
}

- (BOOL)changeCaptureInputByUniqueId:(NSString*)uniqueId {
  [self waitForCaptureChangeToFinish];
  NSArray* currentInputs = [_captureSession inputs];
  // remove current input
  if ([currentInputs count] > 0) {
    AVCaptureInput* currentInput = (AVCaptureInput*)[currentInputs objectAtIndex:0];

    [_captureSession removeInput:currentInput];
  }

  // Look for input device with the name requested (as our input param)
  // get list of available capture devices
  int captureDeviceCount = [DeviceInfoIosObjC captureDeviceCount];
  if (captureDeviceCount <= 0) {
    return NO;
  }

  AVCaptureDevice* captureDevice = [DeviceInfoIosObjC captureDeviceForUniqueId:uniqueId];

  if (!captureDevice) {
    return NO;
  }

  // now create capture session input out of AVCaptureDevice
  NSError* deviceError = nil;
  AVCaptureDeviceInput* newCaptureInput =
      [AVCaptureDeviceInput deviceInputWithDevice:captureDevice error:&deviceError];

  if (!newCaptureInput) {
    const char* errorMessage = [[deviceError localizedDescription] UTF8String];

    RTC_LOG(LS_ERROR) << __FUNCTION__ << ": deviceInputWithDevice error:" << errorMessage;

    return NO;
  }

  // try to add our new capture device to the capture session
  [_captureSession beginConfiguration];

  BOOL addedCaptureInput = NO;
  if ([_captureSession canAddInput:newCaptureInput]) {
    [_captureSession addInput:newCaptureInput];
    addedCaptureInput = YES;
  } else {
    addedCaptureInput = NO;
  }

  [_captureSession commitConfiguration];

  return addedCaptureInput;
}

- (void)captureOutput:(AVCaptureOutput*)captureOutput
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
           fromConnection:(AVCaptureConnection*)connection {
  const int kFlags = 0;
  CVImageBufferRef videoFrame = CMSampleBufferGetImageBuffer(sampleBuffer);

  if (CVPixelBufferLockBaseAddress(videoFrame, kFlags) != kCVReturnSuccess) {
    return;
  }

  const int kYPlaneIndex = 0;
  const int kUVPlaneIndex = 1;
  
  uint8_t* baseAddress = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(videoFrame,kYPlaneIndex);
  if(baseAddress == NULL)
  {
  	return;
  }

  size_t yPlaneBytesPreRow = (int)CVPixelBufferGetBytesPerRowOfPlane(videoFrame,kYPlaneIndex);
  

  const size_t width = CVPixelBufferGetWidth(videoFrame);
  const size_t height = CVPixelBufferGetHeight(videoFrame);
  const size_t frameSize = width * height * 3 >> 1;
  
  VideoCaptureCapability tempCaptureCapability;
  tempCaptureCapability.width = width;
  tempCaptureCapability.height = height;
  tempCaptureCapability.maxFPS = _capability.maxFPS;
  tempCaptureCapability.videoType = VideoType::kNV12;

  if(yPlaneBytesPreRow == width)
  {
	_owner->IncomingFrame(baseAddress, frameSize, tempCaptureCapability, 0);
  }
  else
  {
    size_t yPlaneHeight = (int)CVPixelBufferGetHeightOfPlane(videoFrame,kYPlaneIndex);
    size_t uvPlaneHeight = (int)CVPixelBufferGetHeightOfPlane(videoFrame,kUVPlaneIndex);
    RTC_LOG(LS_ERROR) << __FUNCTION__ << ": Un-Normal capture image size, will deal it later!!!";
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": yPlaneHeight = " << yPlaneHeight << ",uvPlaneHeight = " << uvPlaneHeight;
	/*for(int i = 0; i < yPlaneHeight + uvPlaneHeight; i++)
	{
	  if(_tmp_frame_buf != NULL)
	  {
	    memcpy(_tmp_frame_buf, baseAddress, width);
		baseAddress += yPlaneBytesPreRow;
		_tmp_frame_buf += width;
	  }
	}
	_tmp_frame_buf -= frameSize;
	_owner->IncomingFrame(_tmp_frame_buf, frameSize, tempCaptureCapability, 0);*/
  }

  /*uint8_t* baseAddress = (uint8_t*)CVPixelBufferGetBaseAddress(videoFrame);
  const size_t width = CVPixelBufferGetWidth(videoFrame);
  const size_t height = CVPixelBufferGetHeight(videoFrame);
  const size_t frameSize = width * height * 3 >> 1;//width * height * 2;

  VideoCaptureCapability tempCaptureCapability;
  tempCaptureCapability.width = width;
  tempCaptureCapability.height = height;
  tempCaptureCapability.maxFPS = _capability.maxFPS;
  tempCaptureCapability.videoType = VideoType::kNV12;

  _owner->IncomingFrame(baseAddress, frameSize, tempCaptureCapability, 0);*/

  CVPixelBufferUnlockBaseAddress(videoFrame, kFlags);
}

- (void)signalCaptureChangeEnd {
  [_captureChangingCondition lock];
  _captureChanging = NO;
  [_captureChangingCondition signal];
  [_captureChangingCondition unlock];
}

- (void)waitForCaptureChangeToFinish {
  [_captureChangingCondition lock];
  while (_captureChanging) {
    [_captureChangingCondition wait];
  }
  [_captureChangingCondition unlock];
}

- (void)addPreviewDisplayLayer
{
  if(_preview_layer && _preview_UIView)
  {
    _preview_layer.frame = _preview_UIView.bounds;

	[_preview_UIView.layer addSublayer:_preview_layer];
  }
  else
  {
    RTC_LOG(LS_ERROR) << __FUNCTION__ << ": _preview_layer or _preview_UIView is nil";
  }
}
- (void)setPreviewView:(void*) view
{
  if(!view)
  {
    RTC_LOG(LS_ERROR) << __FUNCTION__ << ": localView is nil";
    _local_View = view;
	return;
  }
  else
  {
    _local_View = view;
	_preview_UIView = nil;
	_preview_UIView = (__bridge UIView*)_local_View;
	_preview_layer = [AVCaptureVideoPreviewLayer layerWithSession:_captureSession];
	_preview_layer.videoGravity = AVLayerVideoGravityResizeAspectFill;
	[self addPreviewDisplayLayer];
  }
}

- (void)regDisplayPreviewData
{
  _preview_layer = [AVCaptureVideoPreviewLayer layerWithSession:_captureSession];
  _preview_layer.videoGravity = AVLayerVideoGravityResizeAspectFill;
  [self addPreviewDisplayLayer];
}
- (void)unregDisplayPreviewData
{
  if(_preview_layer && _preview_UIView)
  {
    CGRect tmp_frame = {{0,0},{0,0}};
	_preview_layer.frame = tmp_frame;
  }
}

@end
