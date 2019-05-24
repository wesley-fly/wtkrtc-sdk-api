/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_EXTERNAL_VIDEO_RENDER_EXTERNAL_IMPL_H_
#define WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_EXTERNAL_VIDEO_RENDER_EXTERNAL_IMPL_H_

#include "modules/include/module_common_types.h"
#include "modules/video_render/i_video_render.h"
#include "rtc_base/criticalsection.h"


namespace webrtc {

// Class definitions
class VideoRenderExternalImpl: IVideoRender, public rtc::VideoSinkInterface<VideoFrame>
{
public:
    /*
     *   Constructor/destructor
     */

    VideoRenderExternalImpl(const int32_t id,
                            const VideoRenderType videoRenderType,
                            void* window, const bool fullscreen);

    virtual ~VideoRenderExternalImpl();

    virtual int32_t Init() override;

    virtual int32_t ChangeWindow(void* window) override;

    /**************************************************************************
     *
     *   Incoming Streams
     *
     ***************************************************************************/

    virtual rtc::VideoSinkInterface<VideoFrame>
            * AddIncomingRenderStream(const uint32_t streamId,
                                      const uint32_t zOrder,
                                      const float left, const float top,
                                      const float right, const float bottom) override;

    virtual int32_t
            DeleteIncomingRenderStream(const uint32_t streamId) override;

    virtual int32_t
            GetIncomingRenderStreamProperties(const uint32_t streamId,
                                              uint32_t& zOrder,
                                              float& left, float& top,
                                              float& right, float& bottom) const override;

    /**************************************************************************
     *
     *   Start/Stop
     *
     ***************************************************************************/

    virtual int32_t StartRender() override;

    virtual int32_t StopRender() override;

    /**************************************************************************
     *
     *   Properties
     *
     ***************************************************************************/

    virtual VideoRenderType RenderType() override;

    virtual VideoType PerferedVideoType() override;

    virtual bool FullScreen() override;

    virtual int32_t
            GetGraphicsMemory(uint64_t& totalGraphicsMemory,
                              uint64_t& availableGraphicsMemory) const override;

    virtual int32_t
            GetScreenResolution(uint32_t& screenWidth,
                                uint32_t& screenHeight) const override;

    virtual uint32_t RenderFrameRate(const uint32_t streamId) override;

    virtual int32_t SetStreamCropping(const uint32_t streamId,
                                      const float left, const float top,
                                      const float right, const float bottom) override;

    virtual int32_t ConfigureRenderer(const uint32_t streamId,
                                      const unsigned int zOrder,
                                      const float left, const float top,
                                      const float right, const float bottom) override;

    virtual int32_t SetTransparentBackground(const bool enable) override;

    virtual int32_t SetText(const uint8_t textId,
                            const uint8_t* text,
                            const int32_t textLength,
                            const uint32_t textColorRef,
                            const uint32_t backgroundColorRef,
                            const float left, const float top,
                            const float right, const float bottom) override;

    virtual int32_t SetBitmap(const void* bitMap,
                              const uint8_t pictureId,
                              const void* colorKey, const float left,
                              const float top, const float right,
                              const float bottom) override;

    void OnFrame(const VideoFrame& videoFrame) override;

private:
    rtc::CriticalSection _critSect;
    bool _fullscreen;
};

}  // namespace webrtc


#endif  // WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_EXTERNAL_VIDEO_RENDER_EXTERNAL_IMPL_H_
