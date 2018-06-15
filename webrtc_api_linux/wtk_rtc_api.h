#ifndef _wtk_rtc_api_h
#define _wtk_rtc_api_h

#include <memory>
#include <utility>
#include <vector>
#include <string>


#define WTK_USE_BUILTIN_OPUS 1
#define WTK_USE_BUILTIN_ILBC 1

#include "rtc_base/logging.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/audio_mixer/audio_mixer_impl.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"

#include "api/video_codecs/sdp_video_format.h"

#include "api/audio_codecs/audio_encoder_factory_template.h"
#include "api/audio_codecs/audio_decoder_factory_template.h"
#if WTK_USE_BUILTIN_OPUS
#include "api/audio_codecs/opus/audio_encoder_opus.h"
#include "api/audio_codecs/opus/audio_decoder_opus.h"
#endif
#if WTK_USE_BUILTIN_ILBC
#include "api/audio_codecs/ilbc/audio_encoder_ilbc.h"
#include "api/audio_codecs/ilbc/audio_decoder_ilbc.h"
#endif
#include "media/engine/internalencoderfactory.h"

// call api mode
#include "call/call.h"
#include "system_wrappers/include/clock.h"
#include "test/video_renderer.h"

#endif
