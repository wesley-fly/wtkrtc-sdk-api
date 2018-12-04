#include <memory>
#include <utility>
#include <vector>
#include <string>

#include "call/call.h"
#include "rtc_base/logging.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/audio_mixer/audio_mixer_impl.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "modules/video_render/video_render.h"
#include "api/video_codecs/sdp_video_format.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "api/audio_codecs/audio_encoder_factory_template.h"
#include "api/audio_codecs/audio_decoder_factory_template.h"
#include "api/audio_codecs/opus/audio_encoder_opus.h"
#include "api/audio_codecs/opus/audio_decoder_opus.h"
#include "api/audio_codecs/ilbc/audio_encoder_ilbc.h"
#include "api/audio_codecs/ilbc/audio_decoder_ilbc.h"
#include "media/engine/internalencoderfactory.h"
#include "media/engine/webrtcvideoengine.h"
#include "system_wrappers/include/clock.h"
#include "modules/rtp_rtcp/include/rtp_header_parser.h"

#include "wtk_rtc_api.h"
#include "wtkrtc_mediaengine/wtk_rtc_api/wtk_video_capturer/wtk_vcm_capturer.h"

#if defined(WEBRTC_ANDROID)
#include "sdk/android/src/jni/androidmediadecoder_jni.h"
#include "sdk/android/src/jni/androidmediaencoder_jni.h"
#elif defined(WEBRTC_IOS)
#endif
#include "modules/bitrate_controller/include/bitrate_controller.h"

#define MAX_VIDEO_PARTICIPANT 4
#define AV_SYNC_GROUP 			"wtk_av_sync"

static int g_audio_min_bps = 8 * 1000;
static int g_audio_max_bps = 32 * 1000;
static int g_video_min_bps = 64 * 1000;
static int g_video_max_bps = 512 * 1000;

static int g_call_min_bps = g_audio_min_bps + g_video_min_bps;
static int g_call_start_bps = g_audio_min_bps + g_video_min_bps;
static int g_call_max_bps = g_audio_max_bps + g_video_max_bps;

static bool g_use_rtp_exten = false;
static bool g_send_side_bwe = false;

static int g_used_video_codec = kWtkVideoCodecH264;
//capturer
static int g_used_video_width = 640;
static int g_used_video_height = 480;
//encode
static int g_used_video_fps = 15;
static int g_used_video_maxqp = 30;

static std::unique_ptr<webrtc::Call>					g_call = nullptr;

static webrtc::VideoCapturer* 							g_video_capturers = nullptr;
static rtc::VideoSinkInterface<webrtc::VideoFrame>* 	g_remote_display = nullptr;
static webrtc::AudioSendStream* 						g_audio_send_stream = nullptr;
static webrtc::AudioReceiveStream* 						g_audio_receive_stream = nullptr;
static webrtc::VideoSendStream* 						g_video_send_stream = nullptr;
static webrtc::VideoReceiveStream* 						g_video_receive_stream = nullptr;

static audio_transport_callback_t 						g_send_audio_packet = nullptr;
static video_transport_callback_t 						g_send_video_packet = nullptr;

//For conference
static rtc::VideoSinkInterface<webrtc::VideoFrame>* 	g_conf_display[MAX_VIDEO_PARTICIPANT] = {nullptr};
static webrtc::VideoSendStream* 						g_conf_send_stream = nullptr;
static webrtc::VideoReceiveStream* 						g_conf_receive_stream[MAX_VIDEO_PARTICIPANT] = {nullptr};

class AudioTransport:public webrtc::Transport{
public:
    bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options) override
    {		
        int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		rtc::SentPacket sent_packet(options.packet_id,send_time);
    	g_call->OnSentPacket(sent_packet);
		g_send_audio_packet(packet, length);
        return true;
    }

    bool SendRtcp(const uint8_t* packet, size_t length) override
    {
        g_send_audio_packet(packet, length);
        return true;
    }
};

class VideoTransport:public webrtc::Transport{
public:
    bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options) override
    {
        int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		rtc::SentPacket sent_packet(options.packet_id,send_time);
    	g_call->OnSentPacket(sent_packet);
		g_send_video_packet(packet, length);
		//RTC_LOG(LS_INFO) << __FUNCTION__ << ":: SendRtp length " << length;
        return true;
    }

    bool SendRtcp(const uint8_t* packet, size_t length) override
    {
		g_send_video_packet(packet, length);
		//RTC_LOG(LS_INFO) << __FUNCTION__ << ":: SendRtcp length " << length;
        return true;
    }
};

AudioTransport* g_audio_send_transport = new AudioTransport();
VideoTransport* g_video_send_transport = new VideoTransport();

#ifdef WEBRTC_ANDROID
int libwtk_init_AndroidVideoEnv(void* javaVM, void* context)
{
	if(webrtc::SetCaptureAndroidVM(javaVM, context) != 0)
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: SetCaptureAndroidVM error";
		return -1;
	}
	if(webrtc::SetRenderAndroidVM(javaVM) != 0)
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: SetRenderAndroidVM error";
		return -1;
	}

	return 0;
}
#endif

void libwtk_set_audio_transport(audio_transport_callback_t func)
{
	g_send_audio_packet = func;
}
void libwtk_set_video_transport(video_transport_callback_t func)
{
	g_send_video_packet = func;
}
int libwtk_decode_audio(uint8_t* buf, int buflen)
{
	if( buflen && g_call != nullptr)
	{
		int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		g_call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, rtc::CopyOnWriteBuffer(buf, buflen), webrtc::PacketTime(send_time, -1));
	}
	return buflen;
}
int libwtk_decode_video(uint8_t* buf, int buflen)
{
	if( buflen && g_call != nullptr)
	{
		int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		g_call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, rtc::CopyOnWriteBuffer(buf, buflen), webrtc::PacketTime(send_time, -1));
	}
	return buflen;
}

void libwtk_config_video(int codec, int width, int height, int fps, int maxqp)
{
	g_used_video_codec = codec;
	g_used_video_width = width;
	g_used_video_height = height;
	g_used_video_fps = fps;
	g_used_video_maxqp = maxqp;
}
void libwtk_config_bitrate(int audio_min_bps, int audio_max_bps, int video_min_bps, int video_max_bps)
{
	g_audio_min_bps = audio_min_bps;
	g_audio_max_bps = audio_max_bps;
	g_video_min_bps = video_min_bps;
	g_video_max_bps = video_max_bps;

	g_call_min_bps = g_audio_min_bps + g_video_min_bps;
	g_call_start_bps = g_audio_min_bps + g_video_min_bps;
	g_call_max_bps = g_audio_max_bps + g_video_max_bps;
}

int libwtk_create_remote_render(void* surfaceView)
{
	int streamId = 0;
	int is_full_screen = 1;
	webrtc::VideoRender* g_remote_render = nullptr;
	if(surfaceView != nullptr)
	{
		g_remote_render = webrtc::VideoRender::CreateVideoRender(streamId,surfaceView,is_full_screen,webrtc::kRenderDefault);
		if(g_remote_render != nullptr)
		{
			g_remote_display = g_remote_render->AddIncomingRenderStream(streamId, 0, 0, 0, 1, 1);
			g_remote_render->StartRender(streamId);
			return 0;
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: CreateVideoRender error";
			return -1;
		}
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: surfaceView is null";
		return -1;
	}
}

int libwtk_create_capture(int is_front)
{
	g_video_capturers = webrtc::VcmCapturer::Create(g_used_video_width, g_used_video_height,g_used_video_fps,is_front);
	if(g_video_capturers != nullptr)
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: g_video_capturers create success";
		return 0;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << ":: g_video_capturers create failed, swap w:h";
		return -1;
	}
}
void libwtk_start_capture(void)
{
	if(g_video_capturers != nullptr)
	{
		g_video_capturers->Start();
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: g_video_capturers = null";
	}
}
void libwtk_stop_capture(void)
{
	if(g_video_capturers != nullptr)
	{
		g_video_capturers->Stop();
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: g_video_capturers = null";
	}
}
void libwtk_switch_camera(int device_id)
{
	if(g_video_capturers != nullptr){
		g_video_capturers->SetCaptureDevice(device_id);
		g_video_capturers->SetCaptureRotation(webrtc::kVideoRotation_270);
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: g_video_capturers = null";
	}
}

void libwtk_destory_capture()
{
	if(g_video_capturers != nullptr)
	{
		g_video_capturers->Destroy();
	}
}

void libwtk_set_capture_rotation(int rotation)
{
	webrtc::VideoRotation rotation_set;

	switch (rotation)
	{
		case 0:
			rotation_set = webrtc::kVideoRotation_0;
			break;
		case 90:
			rotation_set = webrtc::kVideoRotation_90;
			break;
		case 180:
			rotation_set = webrtc::kVideoRotation_180;
			break;
		case 270:
			rotation_set = webrtc::kVideoRotation_270;
			break;
		default:
			rotation_set = webrtc::kVideoRotation_270;
			break;
	}
	if(g_video_capturers != nullptr)
	{
		g_video_capturers->SetCaptureRotation(rotation_set);
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: g_video_capturers = null";
	}
}

int libwtk_create_call(void)
{
	if(g_call != nullptr)
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << " :g_Call already exsit, so return success!";
		return 0;
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << " :g_Call is nullprt, so start creat call object!";
		//audio device module
		rtc::scoped_refptr<webrtc::AudioDeviceModule> adm = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kPlatformDefaultAudio);
#ifdef WEBRTC_ANDROID
		if(adm->BuiltInAECIsAvailable())
			adm->EnableBuiltInAEC(1);
		if(adm->BuiltInAGCIsAvailable())
			adm->EnableBuiltInAGC(1);
		if(adm->BuiltInNSIsAvailable())
			adm->EnableBuiltInNS(1);
#endif
		adm->Init();
		//audio process module
		rtc::scoped_refptr<webrtc::AudioProcessing> apm = webrtc::AudioProcessingBuilder().Create();
		webrtc::AudioProcessing::Config config;
		config.high_pass_filter.enabled = true;
		config.gain_controller2.enabled = true;
		apm->ApplyConfig(config);
		apm->level_estimator()->Enable(true);
		apm->echo_cancellation()->enable_drift_compensation(true);
		apm->echo_cancellation()->Enable(true);
		apm->echo_cancellation()->enable_metrics(true);
		apm->noise_suppression()->set_level(webrtc::NoiseSuppression::kVeryHigh);
		apm->noise_suppression()->Enable(true);
		apm->gain_control()->set_analog_level_limits(0, 255);
		apm->gain_control()->set_mode(webrtc::GainControl::kAdaptiveAnalog);
		apm->gain_control()->Enable(true);
		apm->voice_detection()->Enable(true);
		apm->voice_detection()->set_likelihood( webrtc::VoiceDetection::kModerateLikelihood);
		apm->Initialize();

		webrtc::AudioState::Config audioStateConfig;
		audioStateConfig.audio_device_module = adm;
		audioStateConfig.audio_mixer = webrtc::AudioMixerImpl::Create();
		audioStateConfig.audio_processing = apm;

		webrtc::BitrateConstraints call_bitrate_config;
		call_bitrate_config.min_bitrate_bps = g_call_min_bps;
		call_bitrate_config.start_bitrate_bps = g_call_start_bps;
		call_bitrate_config.max_bitrate_bps = g_call_max_bps;
		
		static std::unique_ptr<webrtc::RtcEventLog> event_log =  webrtc::RtcEventLog::CreateNull();
		webrtc::CallConfig callConfig(event_log.get());
		callConfig.audio_state = webrtc::AudioState::Create(audioStateConfig);
		callConfig.bitrate_config = call_bitrate_config;
		
		adm->RegisterAudioCallback(callConfig.audio_state->audio_transport());
		
		g_call.reset(webrtc::Call::Create(callConfig));
		if(g_call != nullptr)
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << " :Init Call Success!";
			return 0;
		}else{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << " :Init Call Failed!";
			return -1;
		}
	}
}

static int64_t video_rec_avg_bps = 0; 

int libwtk_get_audio_stats(int* send_bps, int* rec_bps, int* package_lost)
{
	if((g_audio_send_stream != nullptr)&&(g_audio_receive_stream != nullptr))
	{
		webrtc::AudioSendStream::Stats audio_send_stats;
		audio_send_stats = g_audio_send_stream->GetStats();
		webrtc::AudioReceiveStream::Stats audio_rec_stats;
		audio_rec_stats = g_audio_receive_stream->GetStats();
		
		*package_lost = audio_send_stats.packets_lost + audio_rec_stats.packets_lost;
		*send_bps = (audio_send_stats.bytes_sent/(int64_t)audio_send_stats.total_input_duration + 1)*8;
		*rec_bps = (audio_rec_stats.bytes_rcvd/(int64_t)audio_send_stats.total_input_duration + 1)*8;

		return 0;
	}
	else
	{
		return -1;
	}
}
int libwtk_get_video_stats(int* send_bps, int* rec_bps, int* prefer_bps)
{
	if((g_video_send_stream != nullptr)&&(g_video_receive_stream != nullptr))
	{
		webrtc::VideoSendStream::Stats video_send_stats;
		video_send_stats = g_video_send_stream->GetStats();
		webrtc::VideoReceiveStream::Stats video_rec_stats;
		video_rec_stats = g_video_receive_stream->GetStats();
		
		*send_bps = video_send_stats.media_bitrate_bps;
		if(video_rec_avg_bps == 0)
		{
			*rec_bps = video_rec_stats.total_bitrate_bps;
		}
		else
		{
			*rec_bps = video_rec_avg_bps;
		}
		*prefer_bps = video_send_stats.preferred_media_bitrate_bps;

		return 0;
	}
	else
	{
		return -1;
	}
}

int libwtk_get_call_quality(int* audio_level, int* video_level)
{
	static int64_t last_bytes_rcvd = 0;
	static int64_t last_bytes_rcvd_time = 0;

	if(g_audio_send_stream != nullptr)
	{
		webrtc::AudioSendStream::Stats audio_send_stats;
		audio_send_stats = g_audio_send_stream->GetStats();

		webrtc::AudioReceiveStream::Stats audio_rec_stats;
		audio_rec_stats = g_audio_receive_stream->GetStats();

		int measured_bytes;
		int64_t measured_time;
		int cur_bps;
		if(last_bytes_rcvd_time == 0)
		{
			measured_time = 5000;
		}
		else
		{
			measured_time = webrtc::Clock::GetRealTimeClock()->TimeInMilliseconds() - last_bytes_rcvd_time;
		}
		measured_bytes = audio_rec_stats.bytes_rcvd - last_bytes_rcvd;

		cur_bps = (measured_bytes*8*1000)/measured_time;

		last_bytes_rcvd = audio_rec_stats.bytes_rcvd;
		last_bytes_rcvd_time = webrtc::Clock::GetRealTimeClock()->TimeInMilliseconds();

		*audio_level = cur_bps;
	}
	if(g_video_send_stream != nullptr)
	{
		webrtc::VideoReceiveStream::Stats video_rec_stats;
		video_rec_stats = g_video_receive_stream->GetStats();
		
		int tmp_total_bitrate_bps = video_rec_stats.total_bitrate_bps;
		if(video_rec_avg_bps == 0)
		{
			video_rec_avg_bps = tmp_total_bitrate_bps;
		}
		else
		{
			video_rec_avg_bps = (video_rec_avg_bps + tmp_total_bitrate_bps)/2;
		}
		
		*video_level = tmp_total_bitrate_bps;
	}

	return 0;
}

int libwtk_create_audio_send_stream(uint32_t local_audio_ssrc)
{
	if(g_audio_send_stream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_audio_send_stream already exsit, so return success!";
		return 0;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_audio_send_stream is nullprt, so start creat audio stream!";
	
	    webrtc::AudioSendStream::Config audio_send_config(g_audio_send_transport);
		audio_send_config.send_codec_spec = webrtc::AudioSendStream::Config::SendCodecSpec(kWtkPayloadTypeOpus, {"OPUS", 48000, 2,{{"usedtx", "0"},{"stereo", "1"}}});
		audio_send_config.encoder_factory = webrtc::CreateAudioEncoderFactory<webrtc::AudioEncoderOpus>();
		audio_send_config.rtp.ssrc = local_audio_ssrc;
		//audio_send_config.rtp.nack.rtp_history_ms = 1000;
		audio_send_config.rtp.extensions.clear();
		audio_send_config.min_bitrate_bps = g_audio_min_bps;
		audio_send_config.max_bitrate_bps = g_audio_max_bps;

		if(g_send_side_bwe)
		{
			audio_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,webrtc::kRtpExtensionTransportSequenceNumber));
		}
	    g_audio_send_stream = g_call->CreateAudioSendStream(audio_send_config);
		if (g_audio_send_stream != nullptr)
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
			return 0;
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << " , Failed!";
			return -1;
		}
	}
}

int libwtk_create_audio_receive_stream(uint32_t remote_audio_ssrc)
{
	if(g_audio_receive_stream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_AudioReceiveStream already exsit, so return success!";
		return 0;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_AudioReceiveStream is nullprt, so go on!";
	
	    webrtc::AudioReceiveStream::Config audio_rev_config;
		audio_rev_config.rtcp_send_transport = g_audio_send_transport;
	    audio_rev_config.decoder_factory = webrtc::CreateAudioDecoderFactory<webrtc::AudioDecoderOpus>();
	    audio_rev_config.sync_group = AV_SYNC_GROUP;
		audio_rev_config.decoder_map = {{kWtkPayloadTypeOpus, {"OPUS", 48000, 2}}};
		audio_rev_config.rtp.remote_ssrc = remote_audio_ssrc;
		audio_rev_config.rtp.transport_cc = g_send_side_bwe;
		//audio_rev_config.rtp.nack.rtp_history_ms = 1000;
		audio_rev_config.rtp.extensions.clear();
		if(g_send_side_bwe)
		{
			audio_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,webrtc::kRtpExtensionTransportSequenceNumber));
		}
	    g_audio_receive_stream = g_call->CreateAudioReceiveStream(audio_rev_config);

		if (g_audio_receive_stream != nullptr)
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
			return 0;
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << " , Failed!";
			return -1;
		}
	}
}

int libwtk_create_video_send_stream(uint32_t local_video_ssrc)
{
	if(g_video_send_stream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoSendStream already exsit, so return success!";
		return 0;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoSendStream is nullprt, so go on!";
	
	    webrtc::VideoSendStream::Config video_send_config(g_video_send_transport);
		video_send_config.rtp.ssrcs.push_back(local_video_ssrc);
		if(g_used_video_codec == kWtkVideoCodecVP8)
		{
			video_send_config.rtp.payload_name = "VP8";
			video_send_config.rtp.payload_type = kWtkPayloadTypeVP8;
			video_send_config.encoder_settings.encoder = webrtc::VP8Encoder::Create().release();
		}
		else if(g_used_video_codec == kWtkVideoCodecVP9)
		{
			video_send_config.rtp.payload_name = "VP9";
			video_send_config.rtp.payload_type = kWtkPayloadTypeVP9;
			video_send_config.encoder_settings.encoder = webrtc::VP9Encoder::Create().release();
		}
		else if(g_used_video_codec == kWtkVideoCodecH264)
		{
			video_send_config.rtp.payload_name = "H264";
			video_send_config.rtp.payload_type = kWtkPayloadTypeH264;
			cricket::VideoCodec codec("H264");
			video_send_config.encoder_settings.encoder = webrtc::H264Encoder::Create(codec).release();
		}
		else if(g_used_video_codec == kWtkVideoCodecH264Auto)
		{
			webrtc::jni::MediaCodecVideoEncoderFactory* encoder_factory = new webrtc::jni::MediaCodecVideoEncoderFactory();
			video_send_config.rtp.payload_name = "H264";
			video_send_config.rtp.payload_type = kWtkPayloadTypeH264;
			cricket::VideoCodec codec("H264");
			
			webrtc::VideoEncoder* is_encoder_support = encoder_factory->CreateVideoEncoder(codec);
			if(is_encoder_support != nullptr)
			{
				video_send_config.encoder_settings.encoder = is_encoder_support;
				RTC_LOG(LS_INFO) << __FUNCTION__ << " , HW H264 Supported, Use it!";
			}
			else
			{
				video_send_config.encoder_settings.encoder = webrtc::H264Encoder::Create(codec).release();
				RTC_LOG(LS_INFO) << __FUNCTION__ << " , HW H264 Not Supported, roll back to SW H264!";
			}
		}

		video_send_config.rtp.max_packet_size = 1200;
		video_send_config.rtp.extensions.clear();
		if(g_send_side_bwe){
			video_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,webrtc::kRtpExtensionTransportSequenceNumber));
		}
		if(g_use_rtp_exten)
		{
			if(!g_send_side_bwe){
				video_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kAbsSendTimeUri,webrtc::kRtpExtensionAbsoluteSendTime));
			}
			video_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kVideoContentTypeUri,webrtc::kRtpExtensionVideoContentType));
		    video_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kVideoTimingUri,webrtc::kRtpExtensionVideoTiming));
		}
		webrtc::VideoEncoderConfig encoder_config;
		
		encoder_config.number_of_streams = 1;
		encoder_config.min_transmit_bitrate_bps = g_video_min_bps;
		encoder_config.max_bitrate_bps = g_video_max_bps;
		encoder_config.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;

		if(g_used_video_codec == kWtkVideoCodecVP8)
		{
			encoder_config.codec_type = webrtc::kVideoCodecVP8;

			webrtc::VideoCodecVP8 vp8_settings = webrtc::VideoEncoder::GetDefaultVp8Settings();
			encoder_config.encoder_specific_settings = new rtc::RefCountedObject<webrtc::VideoEncoderConfig::Vp8EncoderSpecificSettings>(vp8_settings);
			encoder_config.video_stream_factory = new rtc::RefCountedObject<cricket::EncoderStreamFactory>("VP8", g_used_video_maxqp, g_used_video_fps, false, false);
		}
		else if(g_used_video_codec == kWtkVideoCodecVP9)
		{
			encoder_config.codec_type = webrtc::kVideoCodecVP9;

			webrtc::VideoCodecVP9 vp9_settings = webrtc::VideoEncoder::GetDefaultVp9Settings();
			encoder_config.encoder_specific_settings = new rtc::RefCountedObject<webrtc::VideoEncoderConfig::Vp9EncoderSpecificSettings>(vp9_settings);
			encoder_config.video_stream_factory = new rtc::RefCountedObject<cricket::EncoderStreamFactory>("VP9", g_used_video_maxqp, g_used_video_fps, false, false);
		}
		else if((g_used_video_codec == kWtkVideoCodecH264)||(g_used_video_codec == kWtkVideoCodecH264Auto))
		{
			encoder_config.codec_type = webrtc::kVideoCodecH264;

			webrtc::VideoCodecH264 h264_settings = webrtc::VideoEncoder::GetDefaultH264Settings();
			encoder_config.encoder_specific_settings = new rtc::RefCountedObject<webrtc::VideoEncoderConfig::H264EncoderSpecificSettings>(h264_settings);
			encoder_config.video_stream_factory = new rtc::RefCountedObject<cricket::EncoderStreamFactory>("H264", g_used_video_maxqp, g_used_video_fps, false, false);
		}

		g_video_send_stream = g_call->CreateVideoSendStream(std::move(video_send_config), std::move(encoder_config));
		if (g_video_send_stream != nullptr)
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
			//kDegradationDisabled,kMaintainResolution,kMaintainFramerate,kBalanced,
			g_video_send_stream->SetSource(g_video_capturers, webrtc::VideoSendStream::DegradationPreference::kBalanced);
			return 0;
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << " , Failed!";
			return -1;
		}
	}
}

int libwtk_create_video_receive_stream(uint32_t remote_video_ssrc)
{
	if(g_video_receive_stream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoReceiveStream already exsit, so return success!";
		return 0;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoReceiveStream is nullprt, so go on!";
	
	    webrtc::VideoReceiveStream::Config video_rev_config(g_video_send_transport);
		video_rev_config.renderer = g_remote_display;
	    video_rev_config.sync_group = AV_SYNC_GROUP;
		video_rev_config.rtp.remote_ssrc = remote_video_ssrc;

		video_rev_config.rtp.transport_cc = g_send_side_bwe;
		video_rev_config.rtp.remb = g_send_side_bwe;
		
		video_rev_config.rtp.rtcp_mode = webrtc::RtcpMode::kReducedSize;
		
		video_rev_config.rtp.extensions.clear();
		if(g_send_side_bwe){
			video_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,webrtc::kRtpExtensionTransportSequenceNumber));
		}
		if(g_use_rtp_exten)
		{
			if(!g_send_side_bwe){
				video_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kAbsSendTimeUri,webrtc::kRtpExtensionAbsoluteSendTime));
			}
			video_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kVideoContentTypeUri,webrtc::kRtpExtensionVideoContentType));
		    video_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kVideoTimingUri,webrtc::kRtpExtensionVideoTiming));
		}
		webrtc::VideoReceiveStream::Decoder decoder_config;
		
		decoder_config.payload_name = "VP8";
		decoder_config.payload_type = kWtkPayloadTypeVP8;
		decoder_config.decoder = webrtc::VP8Decoder::Create().release();
		video_rev_config.decoders.push_back(decoder_config);
		
		decoder_config.payload_name = "VP9";
		decoder_config.payload_type = kWtkPayloadTypeVP9;
		decoder_config.decoder = webrtc::VP9Decoder::Create().release();
		video_rev_config.decoders.push_back(decoder_config);
				
		if(g_used_video_codec == kWtkVideoCodecH264)
		{
			decoder_config.payload_name = "H264";
			decoder_config.payload_type = kWtkPayloadTypeH264;
			decoder_config.decoder = webrtc::H264Decoder::Create().release();
			video_rev_config.decoders.push_back(decoder_config);
		}
		else// if(g_used_video_codec == kWtkVideoCodecH264Auto)
		{
			webrtc::jni::MediaCodecVideoDecoderFactory* decoder_factory = new webrtc::jni::MediaCodecVideoDecoderFactory();
			webrtc::VideoDecoder* is_decoder_support = decoder_factory->CreateVideoDecoder(webrtc::kVideoCodecH264);
			if(is_decoder_support != nullptr)
			{
				decoder_config.payload_name = "H264";
				decoder_config.payload_type = kWtkPayloadTypeH264;
				decoder_config.decoder = is_decoder_support;
				video_rev_config.decoders.push_back(decoder_config);
				RTC_LOG(LS_INFO) << __FUNCTION__ << " , HW H264 Supported, Use it!";
			}
			else
			{
				decoder_config.payload_name = "H264";
				decoder_config.payload_type = kWtkPayloadTypeH264;
				decoder_config.decoder = webrtc::H264Decoder::Create().release();
				video_rev_config.decoders.push_back(decoder_config);
				RTC_LOG(LS_INFO) << __FUNCTION__ << " ,No HW H264 Supporte, Use SW H264 it!";
			}
		}
		
	    g_video_receive_stream = g_call->CreateVideoReceiveStream(std::move(video_rev_config));
		if (g_video_receive_stream != nullptr)
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
			return 0;
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << " , Failed!";
			return -1;
		}
	}
}
void libwtk_destroy_audio_send_stream(void)
{
	if (g_audio_send_stream != nullptr && g_call != nullptr)
	{
		g_call->DestroyAudioSendStream(g_audio_send_stream);
		g_audio_send_stream = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
	}
}

void libwtk_destroy_audio_receive_stream(void)
{
	if (g_audio_receive_stream != nullptr && g_call != nullptr)
	{
		g_call->DestroyAudioReceiveStream(g_audio_receive_stream);
		g_audio_receive_stream = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
	}
}

void libwtk_destroy_video_send_stream(void)
{
	if (g_video_send_stream != nullptr && g_call != nullptr)
	{
		g_call->DestroyVideoSendStream(g_video_send_stream);
		g_video_send_stream = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
	}
}
void libwtk_destroy_video_receive_stream(void)
{
	if (g_video_receive_stream != nullptr && g_call != nullptr)
	{
		g_call->DestroyVideoReceiveStream(g_video_receive_stream);
		g_video_receive_stream = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
	}
}
void libwtk_destroy_call(void)
{
	if(g_call != nullptr)
	{
		g_call.reset();
		g_call = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :Destroy Call Success!";
	}else{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_Call is nullptr, no call object to destroy!";
	}
}

void libwtk_set_mute(bool muted)
{
	if(g_audio_send_stream != nullptr)
    {
    	g_audio_send_stream->SetMuted(muted);
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << " , g_audio_send_stream or g_audio_receive_stream is null";
	}
}

void libwtk_start_audio_stream(void)
{	
	if(g_audio_send_stream != nullptr && g_audio_receive_stream != nullptr)
    {
    	g_audio_send_stream->Start();
	    g_audio_receive_stream->Start();
		g_call->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkUp);
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << " , g_audio_send_stream or g_audio_receive_stream is null";
	}
}

void libwtk_start_video_stream(void)
{
	if(g_video_send_stream != nullptr && g_video_receive_stream != nullptr)
    {
    	g_video_send_stream->Start();
		g_video_receive_stream->Start();
		g_call->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkUp);
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << " , g_video_send_stream or g_video_receive_stream is null";
	}
}
void libwtk_stop_audio_stream(void)
{
	if(g_audio_send_stream != nullptr && g_audio_receive_stream != nullptr)
    {
    	g_audio_send_stream->Stop();
		g_audio_receive_stream->Stop();
		g_call->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkDown);
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << " , g_AudioSendStream or g_AudioReceiveStream is null";
	}
}
void libwtk_stop_video_stream(void)
{
	if(g_video_send_stream != nullptr && g_video_receive_stream != nullptr)
    {
		g_video_send_stream->Stop();
		g_video_receive_stream->Stop();
		g_call->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkDown);
	}
	else
	{
		RTC_LOG(LS_ERROR) << __FUNCTION__ << " , g_AudioSendStream or g_AudioReceiveStream is null";
	}
}

//conference, just deal receive video stream
int libwtk_create_conf_render(void* surfaceView0,void* surfaceView1,void* surfaceView2,void* surfaceView3)
{
	int streamId = 1;
	int is_full_screen = 0;
	webrtc::VideoRender* g_remote_render = nullptr;
	
	if(surfaceView0 != nullptr)
	{
		g_remote_render = webrtc::VideoRender::CreateVideoRender(streamId,surfaceView0,is_full_screen,webrtc::kRenderDefault);
		if(g_remote_render != nullptr)
		{
			g_conf_display[0] = g_remote_render->AddIncomingRenderStream(streamId, 0, 0, 0, 1, 1);
			g_remote_render->StartRender(streamId);
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: Create conference 0 VideoRender error";
		}
	}
	streamId++;
	
	if(surfaceView1 != nullptr)
	{
		g_remote_render = webrtc::VideoRender::CreateVideoRender(streamId,surfaceView1,is_full_screen,webrtc::kRenderDefault);
		if(g_remote_render != nullptr)
		{
			g_conf_display[1] = g_remote_render->AddIncomingRenderStream(streamId, 0, 0, 0, 1, 1);
			g_remote_render->StartRender(streamId);
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: Create conference 1 VideoRender error";
		}
	}
	streamId++;
	
	if(surfaceView2 != nullptr)
	{
		g_remote_render = webrtc::VideoRender::CreateVideoRender(streamId,surfaceView2,is_full_screen,webrtc::kRenderDefault);
		if(g_remote_render != nullptr)
		{
			g_conf_display[2] = g_remote_render->AddIncomingRenderStream(streamId, 0, 0, 0, 1, 1);
			g_remote_render->StartRender(streamId);
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: Create conference 2 VideoRender error";
		}
	}
	streamId++;
	
	if(surfaceView3 != nullptr)
	{
		g_remote_render = webrtc::VideoRender::CreateVideoRender(streamId,surfaceView3,is_full_screen,webrtc::kRenderDefault);
		if(g_remote_render != nullptr)
		{
			g_conf_display[3] = g_remote_render->AddIncomingRenderStream(streamId, 0, 0, 0, 1, 1);
			g_remote_render->StartRender(streamId);
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << ":: Create conference 3 VideoRender error";
		}
	}
	streamId++;

	return 0;
}
int libwtk_create_video_conf_send_stream(uint32_t local_video_ssrc)
{
	if(g_conf_send_stream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoSendStream already exsit, so return success!";
		return 0;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoSendStream is nullprt, so go on!";
	
	    webrtc::VideoSendStream::Config video_send_config(g_video_send_transport);
		video_send_config.rtp.ssrcs.push_back(local_video_ssrc);
		if(g_used_video_codec == kWtkVideoCodecVP8)
		{
			video_send_config.rtp.payload_name = "VP8";
			video_send_config.rtp.payload_type = kWtkPayloadTypeVP8;
			video_send_config.encoder_settings.encoder = webrtc::VP8Encoder::Create().release();
		}
		else if(g_used_video_codec == kWtkVideoCodecVP9)
		{
			video_send_config.rtp.payload_name = "VP9";
			video_send_config.rtp.payload_type = kWtkPayloadTypeVP9;
			video_send_config.encoder_settings.encoder = webrtc::VP9Encoder::Create().release();
		}
		else if(g_used_video_codec == kWtkVideoCodecH264)
		{
			video_send_config.rtp.payload_name = "H264";
			video_send_config.rtp.payload_type = kWtkPayloadTypeH264;
			cricket::VideoCodec codec("H264");
			video_send_config.encoder_settings.encoder = webrtc::H264Encoder::Create(codec).release();
		}
		else if(g_used_video_codec == kWtkVideoCodecH264Auto)
		{
			webrtc::jni::MediaCodecVideoEncoderFactory* encoder_factory = new webrtc::jni::MediaCodecVideoEncoderFactory();
			video_send_config.rtp.payload_name = "H264";
			video_send_config.rtp.payload_type = kWtkPayloadTypeH264;
			cricket::VideoCodec codec("H264");
			
			webrtc::VideoEncoder* is_encoder_support = encoder_factory->CreateVideoEncoder(codec);
			if(is_encoder_support != nullptr)
			{
				video_send_config.encoder_settings.encoder = is_encoder_support;
			}
			else
			{
				video_send_config.encoder_settings.encoder = webrtc::H264Encoder::Create(codec).release();
			}
		}

		video_send_config.rtp.max_packet_size = 1200;
		video_send_config.rtp.extensions.clear();
		
		webrtc::VideoEncoderConfig encoder_config;
		encoder_config.number_of_streams = 1;
		encoder_config.min_transmit_bitrate_bps = g_video_min_bps;
		encoder_config.max_bitrate_bps = g_video_max_bps;
		encoder_config.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;

		if(g_used_video_codec == kWtkVideoCodecVP8)
		{
			encoder_config.codec_type = webrtc::kVideoCodecVP8;

			webrtc::VideoCodecVP8 vp8_settings = webrtc::VideoEncoder::GetDefaultVp8Settings();
			encoder_config.encoder_specific_settings = new rtc::RefCountedObject<webrtc::VideoEncoderConfig::Vp8EncoderSpecificSettings>(vp8_settings);
			encoder_config.video_stream_factory = new rtc::RefCountedObject<cricket::EncoderStreamFactory>("VP8", g_used_video_maxqp, g_used_video_fps, false, false);
		}
		else if(g_used_video_codec == kWtkVideoCodecVP9)
		{
			encoder_config.codec_type = webrtc::kVideoCodecVP9;

			webrtc::VideoCodecVP9 vp9_settings = webrtc::VideoEncoder::GetDefaultVp9Settings();
			encoder_config.encoder_specific_settings = new rtc::RefCountedObject<webrtc::VideoEncoderConfig::Vp9EncoderSpecificSettings>(vp9_settings);
			encoder_config.video_stream_factory = new rtc::RefCountedObject<cricket::EncoderStreamFactory>("VP9", g_used_video_maxqp, g_used_video_fps, false, false);
		}
		else if((g_used_video_codec == kWtkVideoCodecH264)||(g_used_video_codec == kWtkVideoCodecH264Auto))
		{
			encoder_config.codec_type = webrtc::kVideoCodecH264;

			webrtc::VideoCodecH264 h264_settings = webrtc::VideoEncoder::GetDefaultH264Settings();
			encoder_config.encoder_specific_settings = new rtc::RefCountedObject<webrtc::VideoEncoderConfig::H264EncoderSpecificSettings>(h264_settings);
			encoder_config.video_stream_factory = new rtc::RefCountedObject<cricket::EncoderStreamFactory>("H264", g_used_video_maxqp, g_used_video_fps, false, false);
		}

		g_conf_send_stream = g_call->CreateVideoSendStream(std::move(video_send_config), std::move(encoder_config));
		if (g_conf_send_stream != nullptr)
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
			//kDegradationDisabled,kMaintainResolution,kMaintainFramerate,kBalanced,
			g_conf_send_stream->SetSource(g_video_capturers, webrtc::VideoSendStream::DegradationPreference::kBalanced);
			return 0;
		}
		else
		{
			RTC_LOG(LS_ERROR) << __FUNCTION__ << " , Failed!";
			return -1;
		}
	}
}

int libwtk_create_video_conf_receive_stream(uint32_t remote_video_ssrc)
{
	int i = 0;
	for(i=0;i<MAX_VIDEO_PARTICIPANT;i++)
	{
		if(g_conf_receive_stream[i] != nullptr)
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoReceiveStream already exsit, so return success!";
			continue;
		}
		else
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoReceiveStream is nullprt, so go on!";
		
		    webrtc::VideoReceiveStream::Config video_rev_config(g_video_send_transport);
			video_rev_config.renderer = g_conf_display[i];
		    video_rev_config.sync_group = AV_SYNC_GROUP;
			video_rev_config.rtp.remote_ssrc = remote_video_ssrc++;
			video_rev_config.rtp.transport_cc = false;
			video_rev_config.rtp.remb = false;
			video_rev_config.rtp.rtcp_mode = webrtc::RtcpMode::kReducedSize;
			video_rev_config.rtp.extensions.clear();
			
			webrtc::VideoReceiveStream::Decoder decoder_config;
			decoder_config.payload_name = "VP8";
			decoder_config.payload_type = kWtkPayloadTypeVP8;
			decoder_config.decoder = webrtc::VP8Decoder::Create().release();
			video_rev_config.decoders.push_back(decoder_config);
			decoder_config.payload_name = "VP9";
			decoder_config.payload_type = kWtkPayloadTypeVP9;
			decoder_config.decoder = webrtc::VP9Decoder::Create().release();
			video_rev_config.decoders.push_back(decoder_config);
			webrtc::jni::MediaCodecVideoDecoderFactory* decoder_factory = new webrtc::jni::MediaCodecVideoDecoderFactory();
			webrtc::VideoDecoder* is_decoder_support = decoder_factory->CreateVideoDecoder(webrtc::kVideoCodecH264);
			if(is_decoder_support != nullptr)
			{
				decoder_config.payload_name = "H264";
				decoder_config.payload_type = kWtkPayloadTypeH264;
				decoder_config.decoder = is_decoder_support;
				video_rev_config.decoders.push_back(decoder_config);
			}
			else
			{
				decoder_config.payload_name = "H264";
				decoder_config.payload_type = kWtkPayloadTypeH264;
				decoder_config.decoder = webrtc::H264Decoder::Create().release();
				video_rev_config.decoders.push_back(decoder_config);
			}
		    g_conf_receive_stream[i] = g_call->CreateVideoReceiveStream(std::move(video_rev_config));
			if (g_conf_receive_stream[i] != nullptr)
			{
				RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
			}
			else
			{
				RTC_LOG(LS_ERROR) << __FUNCTION__ << " , Failed!";
			}
		}
	}

	g_call->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkUp);
	return 0;
}
void libwtk_start_video_conf_stream(void)
{
	int i=0;
	if(g_conf_send_stream != nullptr)
    {
    	g_conf_send_stream->Start();
	}

	for(i=0;i<MAX_VIDEO_PARTICIPANT;i++)
	{
		if(g_conf_receive_stream[i] != nullptr)
			g_conf_receive_stream[i]->Start();
	}
	g_call->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkUp);
}
void libwtk_stop_video_conf_stream(void)
{
	int i=0;
	if(g_conf_send_stream != nullptr)
    {
    	g_conf_send_stream->Stop();		
	}
	for(i=0;i<MAX_VIDEO_PARTICIPANT;i++)
	{
		if(g_conf_receive_stream[i] != nullptr)
			g_conf_receive_stream[i]->Stop();
	}
	g_call->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkDown);
}
void libwtk_destroy_video_conf_stream(void)
{
	int i=0;
	for(i=0;i<MAX_VIDEO_PARTICIPANT;i++)
	{
		if (g_conf_receive_stream[i] != nullptr && g_call != nullptr)
		{
			g_call->DestroyVideoReceiveStream(g_conf_receive_stream[i]);
			g_conf_receive_stream[i] = nullptr;
			RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
		}
		else
		{
			RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
		}
	}

	if (g_conf_send_stream != nullptr && g_call != nullptr)
	{
		g_call->DestroyVideoSendStream(g_conf_send_stream);
		g_conf_send_stream = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
	}
}

