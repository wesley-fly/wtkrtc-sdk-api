#ifndef _wtk_rtc_api_h
#define _wtk_rtc_api_h

#include <memory>
#include <utility>


// org mode
#include "rtc_base/logging.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "modules/audio_coding/include/audio_coding_module.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/audio_mixer/audio_mixer_impl.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"

// call api mode
#include "call/call.h"
#include "system_wrappers/include/clock.h"

#include "media/engine/webrtcvoiceengine.h"

#endif
