#ifndef _wtk_rtc_api_h
#define _wtk_rtc_api_h

#include <memory>
#include <utility>
#include <vector>
#include <string>

#include "rtc_base/logging.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/audio_mixer/audio_mixer_impl.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
//Create video factory
#include "api/video_codecs/sdp_video_format.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "modules/video_coding/codecs/h264/include/h264.h"

//Create audio factory
#include "api/audio_codecs/audio_encoder_factory_template.h"
#include "api/audio_codecs/audio_decoder_factory_template.h"
#include "api/audio_codecs/opus/audio_encoder_opus.h"
#include "api/audio_codecs/opus/audio_decoder_opus.h"
#include "api/audio_codecs/ilbc/audio_encoder_ilbc.h"
#include "api/audio_codecs/ilbc/audio_decoder_ilbc.h"
#include "media/engine/internalencoderfactory.h"
#include "media/engine/webrtcvideoengine.h"

#include "call/call.h"
#include "system_wrappers/include/clock.h"

//Test For video renderer.
#include "test/video_renderer.h"
#include "test/video_capturer.h"
#include "test/vcm_capturer.h"

#endif
