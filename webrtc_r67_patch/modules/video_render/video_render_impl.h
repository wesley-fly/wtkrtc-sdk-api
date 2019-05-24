/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_VIDEO_RENDER_IMPL_H_
#define WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_VIDEO_RENDER_IMPL_H_

#include <map>

#include "modules/video_render/video_render.h"
#include "rtc_base/criticalsection.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {
class IVideoRender;
class AndroidStream;

// Class definitions
class ModuleVideoRenderImpl: public VideoRender
{
public:
    /*
     *   VideoRenderer constructor/destructor
     */
    ModuleVideoRenderImpl(const int32_t id,
                          const VideoRenderType videoRenderType,
                          void* window, const bool fullscreen);

    virtual ~ModuleVideoRenderImpl();

    virtual int64_t TimeUntilNextProcess() override;
    virtual void Process() override;

    /*
     *   Returns the render window
     */
    virtual void* Window() override;

    /*
     *   Change render window
     */
    virtual int32_t ChangeWindow(void* window) override;
	
	//virtual void OnFrame(const webrtc::VideoFrame& frame) override;

    /*
     *   Returns module id
     */
    int32_t Id();

    /**************************************************************************
     *
     *   Incoming Streams
     *
     ***************************************************************************/

    /*
     *   Add incoming render stream
     */
    virtual rtc::VideoSinkInterface<VideoFrame>
            * AddIncomingRenderStream(const uint32_t streamId,
                                      const uint32_t zOrder,
                                      const float left, const float top,
                                      const float right, const float bottom) override;
    /*
     *   Delete incoming render stream
     */
    virtual int32_t
            DeleteIncomingRenderStream(const uint32_t streamId) override;

    /*
     *   Add incoming render callback, used for external rendering
     */
    virtual int32_t
            AddExternalRenderCallback(const uint32_t streamId,
                                      rtc::VideoSinkInterface<VideoFrame>* renderObject) override;

    /*
     *   Get the porperties for an incoming render stream
     */
    virtual int32_t
            GetIncomingRenderStreamProperties(const uint32_t streamId,
                                              uint32_t& zOrder,
                                              float& left, float& top,
                                              float& right, float& bottom) const override;
    /*
     *   Incoming frame rate for the specified stream.
     */
    virtual uint32_t GetIncomingFrameRate(const uint32_t streamId) override;

    /*
     *   Returns the number of incoming streams added to this render module
     */
    virtual uint32_t GetNumIncomingRenderStreams() const override;

    /*
     *   Returns true if this render module has the streamId added, false otherwise.
     */
    virtual bool HasIncomingRenderStream(const uint32_t streamId) const override;

    /*
     *
     */
    virtual int32_t
            RegisterRawFrameCallback(const uint32_t streamId,
                                     rtc::VideoSinkInterface<VideoFrame>* callbackObj) override;

    virtual int32_t SetExpectedRenderDelay(uint32_t stream_id,
                                           int32_t delay_ms) override;

    /**************************************************************************
     *
     *   Start/Stop
     *
     ***************************************************************************/

    /*
     *   Starts rendering the specified stream
     */
    virtual int32_t StartRender(const uint32_t streamId) override;

    /*
     *   Stops the renderer
     */
    virtual int32_t StopRender(const uint32_t streamId) override;

    /*
     *   Sets the renderer in start state, no streams removed.
     */
    virtual int32_t ResetRender() override;

    /**************************************************************************
     *
     *   Properties
     *
     ***************************************************************************/

    /*
     *   Returns the prefered render video type
     */
    virtual VideoType PreferredVideoType() const override;

    /*
     *   Returns true if the renderer is in fullscreen mode, otherwise false.
     */
    virtual bool IsFullScreen() override;

    /*
     *   Gets screen resolution in pixels
     */
    virtual int32_t
            GetScreenResolution(uint32_t& screenWidth,
                                uint32_t& screenHeight) const override;

    /*
     *   Get the actual render rate for this stream. I.e rendered frame rate,
     *   not frames delivered to the renderer.
     */
    virtual uint32_t RenderFrameRate(const uint32_t streamId) override;

    /*
     *   Set cropping of incoming stream
     */
    virtual int32_t SetStreamCropping(const uint32_t streamId,
                                      const float left, const float top,
                                      const float right, const float bottom) override;

    virtual int32_t ConfigureRenderer(const uint32_t streamId,
                                      const unsigned int zOrder,
                                      const float left, const float top,
                                      const float right, const float bottom) override;

    virtual int32_t SetTransparentBackground(const bool enable) override;

    virtual int32_t FullScreenRender(void* window, const bool enable) override;

    virtual int32_t SetBitmap(const void* bitMap,
                              const uint8_t pictureId,
                              const void* colorKey,
                              const float left, const float top,
                              const float right, const float bottom) override;

    virtual int32_t SetText(const uint8_t textId,
                            const uint8_t* text,
                            const int32_t textLength,
                            const uint32_t textColorRef,
                            const uint32_t backgroundColorRef,
                            const float left, const float top,
                            const float right, const float bottom) override;

    virtual int32_t SetStartImage(const uint32_t streamId,
                                  const VideoFrame& videoFrame) override;

    virtual int32_t SetTimeoutImage(const uint32_t streamId,
                                    const VideoFrame& videoFrame,
                                    const uint32_t timeout) override;

private:
	int32_t _id;
    rtc::CriticalSection _moduleCrit;
    void* _ptrWindow;
    bool _fullScreen;

    IVideoRender* _ptrRenderer;
    typedef std::map<uint32_t, IncomingVideoStream*> IncomingVideoStreamMap;
    IncomingVideoStreamMap _streamRenderMap;
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_VIDEO_RENDER_IMPL_H_
