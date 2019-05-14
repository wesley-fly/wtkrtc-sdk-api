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

#include "wtk_rtc_api.h"

#define USED_VIDEO_WIDTH 		352
#define USED_VIDEO_HEIGHT 		288
#define USED_VIDEO_FPS 			15
#define USED_MAX_VIDEO_QP 		48

#define SEND_SSRC 				11111111
#define RECV_SSRC 				22222222
#define RTX_SSRC 				33333333

#define AV_SYNC_GROUP 			"av_sync"

#define AUDIO_MIN_BPS			6 * 1000
#define AUDIO_MAX_BPS			12 * 1000

#define VIDEO_MIN_BPS			32 * 1000
#define VIDEO_MAX_BPS			64 * 1000

#define CALL_MIN_BPS			6 * 1000
#define CALL_START_BPS			8 * 1000
#define CALL_MAX_BPS			12 * 1000

bool g_Send_side_bwe = true;
bool g_Loop_test = false;

static std::unique_ptr<webrtc::Call>					g_Call = nullptr;
static std::unique_ptr<webrtc::RtcEventLog> 			Event_log = nullptr;
//static rtc::scoped_refptr<webrtc::AudioDeviceModule> 	Audio_DeviceModule = nullptr;

static webrtc::AudioSendStream* 						g_AudioSendStream = nullptr;
static webrtc::AudioReceiveStream* 						g_AudioReceiveStream = nullptr;
static webrtc::VideoSendStream* 						g_VideoSendStream = nullptr;
static webrtc::VideoReceiveStream* 						g_VideoReceiveStream = nullptr;

static audio_transport_callback_t 						send_audio_packet = nullptr;
static video_transport_callback_t 						send_video_packet = nullptr;

enum classPayloadTypes{
	kPayloadTypeRtx = 98,
    kPayloadTypeIlbc = 102,
    kPayloadTypeOpus = 109,
    kPayloadTypeH264 = 122,
    kPayloadTypeVP8 = 120,
    kPayloadTypeVP9 = 101,
};
enum classExtensionIds{
	kAudioLevelExtensionId = 5,
	kTOffsetExtensionId = 6,
	kAbsSendTimeExtensionId = 7,
	kTransportSequenceNumberExtensionId = 8,
	kVideoRotationExtensionId = 9,
	kVideoContentTypeExtensionId = 10,
	kVideoTimingExtensionId = 11,
};

class AudioTransport:public webrtc::Transport{
public:
    bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options) override
    {		
        int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		rtc::SentPacket sent_packet(options.packet_id,send_time);
    	g_Call->OnSentPacket(sent_packet);
		if(g_Loop_test)
			g_Call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));
		else
			send_audio_packet(packet,length);
        return true;
    }

    bool SendRtcp(const uint8_t* packet, size_t length) override
    {
    	if(g_Loop_test)
    	{
			int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
	        g_Call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));
    	}
		else
	        send_audio_packet(packet,length);

        return true;
    }
};

class VideoTransport:public webrtc::Transport{
public:
    bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options) override
    {
    	//RTC_LOG(LS_INFO) << "SendRtp started";
        int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		rtc::SentPacket sent_packet(options.packet_id,send_time);
    	g_Call->OnSentPacket(sent_packet);
		if(g_Loop_test)
			g_Call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));
		else
			send_video_packet(packet,length);
        return true;
    }

    bool SendRtcp(const uint8_t* packet, size_t length) override
    {
    	//RTC_LOG(LS_INFO) << "SendRtcp started";
	    if(g_Loop_test)
    	{
			int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
	        g_Call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));
        }
		else
			send_video_packet(packet,length);

        return true;
    }
};

AudioTransport* g_AudioSendTransport = new AudioTransport();
VideoTransport* g_VideoSendTransport = new VideoTransport();

void libwtk_set_audio_transport(audio_transport_callback_t func)
{
	send_audio_packet = func;
}
void libwtk_set_video_transport(video_transport_callback_t func)
{
	send_video_packet = func;
}
static bool IsUserIdPacket(uint8_t* buf,int len)
{
	return false;
}
int parserUserId(uint8_t *buf,int buflen)
{
	return 0;
}

int libwtk_decode_audio(uint8_t* buf, int buflen)
{
	if( buflen)
	{
		if (IsUserIdPacket(buf,buflen)==true) {
			parserUserId(buf,buflen);
		}
		else {
			int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
			g_Call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, rtc::CopyOnWriteBuffer(buf, buflen), webrtc::PacketTime(send_time, -1));
		}
	}
	return buflen;
}
int libwtk_decode_video(uint8_t* buf, int buflen)
{
	if( buflen)
	{
		if (IsUserIdPacket(buf,buflen)==true) {
			parserUserId(buf,buflen);
		}
		else {
			int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
			g_Call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, rtc::CopyOnWriteBuffer(buf, buflen), webrtc::PacketTime(send_time, -1));
		}
	}
	return buflen;
}

int libwtk_initialize(void)
{
	int retval = SUCESS_RET;
	RTC_LOG(LS_INFO) << __FUNCTION__;
	return retval;
}
void libwtk_init_local_render(void)
{
	RTC_LOG(LS_INFO) << __FUNCTION__;
}

void libwtk_init_remote_render(void)
{
	RTC_LOG(LS_INFO) << __FUNCTION__;
}

int libwtk_init_capture(int device_id)
{
	int retval = SUCESS_RET;
	RTC_LOG(LS_INFO) << __FUNCTION__;

	return retval;
}
void libwtk_set_audio_codec(void)
{
	RTC_LOG(LS_INFO) << __FUNCTION__;
}

void libwtk_set_video_codec(void)
{
	RTC_LOG(LS_INFO) << __FUNCTION__;
}

void libwtk_start_capture(void)
{
	RTC_LOG(LS_INFO) << __FUNCTION__;
}
void libwtk_stop_capture(void)
{
	RTC_LOG(LS_INFO) << __FUNCTION__;
}
void libwtk_switch_camera(int device_id)
{
	RTC_LOG(LS_INFO) << __FUNCTION__;
}
int libwtk_create_call(void)
{
	RTC_LOG(LS_INFO) << __FUNCTION__;
	int retval = SUCESS_RET;
	if(g_Call != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_Call already exsit, so return success!";
		retval = SUCESS_RET;
		return retval;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_Call is nullprt, so go on!";
	}
	rtc::scoped_refptr<webrtc::AudioDeviceModule> Audio_DeviceModule = nullptr;
	Audio_DeviceModule = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kPlatformDefaultAudio);
	Audio_DeviceModule->Init();
	if(Audio_DeviceModule->BuiltInAECIsAvailable())
		Audio_DeviceModule->EnableBuiltInAEC(1);
	if(Audio_DeviceModule->BuiltInAGCIsAvailable())
		Audio_DeviceModule->EnableBuiltInAGC(1);
	if(Audio_DeviceModule->BuiltInNSIsAvailable())
		Audio_DeviceModule->EnableBuiltInNS(1);

	webrtc::AudioState::Config audioStateConfig;
	audioStateConfig.audio_device_module = Audio_DeviceModule;
	audioStateConfig.audio_mixer = webrtc::AudioMixerImpl::Create();
	audioStateConfig.audio_processing = webrtc::AudioProcessingBuilder().Create();

	webrtc::BitrateConstraints call_bitrate_config;
	call_bitrate_config.min_bitrate_bps = CALL_MIN_BPS;
	call_bitrate_config.start_bitrate_bps = CALL_START_BPS;
	call_bitrate_config.max_bitrate_bps = CALL_MAX_BPS;
	
	Event_log = webrtc::RtcEventLog::Create(webrtc::RtcEventLog::EncodingType::Legacy);
	webrtc::CallConfig callConfig(Event_log.get());
	callConfig.audio_state = webrtc::AudioState::Create(audioStateConfig);
	callConfig.bitrate_config = call_bitrate_config;
	
	Audio_DeviceModule->RegisterAudioCallback(callConfig.audio_state->audio_transport());
	
	//g_Call = webrtc::Call::Create(callConfig);
	g_Call.reset(webrtc::Call::Create(callConfig));
	if(g_Call != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :Init Call Success!";
		retval = SUCESS_RET;
	}else{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :Init Call Failed!";
		retval = FAILED_RET;
	}

	return retval;
}
void libwtk_destroy_call(void)
{
	if(g_Call != nullptr)
	{
		g_Call.reset();
		g_Call = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :Destroy Call Success!";
	}else{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_Call is nullptr, no stream to destroy!";
	}

}

int libwtk_create_audio_send_stream(int rtp_format)
{
	int retval = SUCESS_RET;
	if(g_AudioSendStream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_AudioSendStream already exsit, so return success!";
		return retval;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_AudioSendStream is nullprt, so go on!";
	}
    webrtc::AudioSendStream::Config audio_send_config(g_AudioSendTransport);
	audio_send_config.send_codec_spec = webrtc::AudioSendStream::Config::SendCodecSpec(kPayloadTypeOpus, {"OPUS", 48000, 2,{{"usedtx", "0"},{"stereo", "1"}}});
	audio_send_config.encoder_factory = webrtc::CreateAudioEncoderFactory<webrtc::AudioEncoderOpus>();
	audio_send_config.rtp.ssrc = SEND_SSRC + 1;
	audio_send_config.rtp.extensions.clear();

	if(g_Send_side_bwe)
	{
		audio_send_config.min_bitrate_bps = AUDIO_MIN_BPS;
		audio_send_config.max_bitrate_bps = AUDIO_MAX_BPS;	
		
		audio_send_config.rtp.extensions.push_back(
	        webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,
	        			kTransportSequenceNumberExtensionId));
	}
    g_AudioSendStream = g_Call->CreateAudioSendStream(audio_send_config);
	if (g_AudioSendStream != nullptr)
	{
		retval = SUCESS_RET;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		retval = FAILED_RET;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Failed!";
	}

	return retval;
}

void libwtk_destroy_audio_send_stream(void)
{
	if (g_AudioSendStream != nullptr && g_Call != nullptr)
	{
		g_Call->DestroyAudioSendStream(g_AudioSendStream);
		g_AudioSendStream = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
	}
}
int libwtk_create_audio_receive_stream(int rtp_format)
{
	int retval = SUCESS_RET;
	if(g_AudioReceiveStream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_AudioReceiveStream already exsit, so return success!";
		return retval;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_AudioReceiveStream is nullprt, so go on!";
	}

    webrtc::AudioReceiveStream::Config audio_rev_config;
	audio_rev_config.rtcp_send_transport = g_AudioSendTransport;
    audio_rev_config.decoder_factory = webrtc::CreateAudioDecoderFactory<webrtc::AudioDecoderOpus>();
    audio_rev_config.sync_group = AV_SYNC_GROUP;
	audio_rev_config.decoder_map = {{kPayloadTypeOpus, {"OPUS", 48000, 2}}};
	audio_rev_config.rtp.remote_ssrc = SEND_SSRC + 1;
	audio_rev_config.rtp.local_ssrc = RECV_SSRC + 1;
	audio_rev_config.rtp.transport_cc = g_Send_side_bwe;
	audio_rev_config.rtp.extensions.clear();

	if(g_Send_side_bwe){
		audio_rev_config.rtp.extensions.push_back(
	        webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,
	        			kTransportSequenceNumberExtensionId));
	}
    g_AudioReceiveStream = g_Call->CreateAudioReceiveStream(audio_rev_config);

	if (g_AudioReceiveStream != nullptr)
	{
		retval = SUCESS_RET;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		retval = FAILED_RET;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Failed!";
	}

	return retval;
}
void libwtk_destroy_audio_receive_stream(void)
{
	if (g_AudioReceiveStream != nullptr && g_Call != nullptr)
	{
		g_Call->DestroyAudioReceiveStream(g_AudioReceiveStream);
		g_AudioReceiveStream = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
	}
}

int libwtk_create_video_send_stream(int rtp_format)
{
	int retval = SUCESS_RET;
	if(g_VideoSendStream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoSendStream already exsit, so return success!";
		return retval;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoSendStream is nullprt, so go on!";
	}
	
	webrtc::VideoSendStream::Config video_send_config(g_VideoSendTransport);
	video_send_config.rtp.ssrcs.push_back(SEND_SSRC);
	video_send_config.rtp.payload_name = "VP8";
	video_send_config.rtp.payload_type = kPayloadTypeVP8;
	//video_send_config.pre_encode_callback = g_Local_render.get();
	video_send_config.encoder_settings.encoder = webrtc::VP8Encoder::Create().release();
	video_send_config.rtp.nack.rtp_history_ms = 1000;
	video_send_config.rtp.rtx.ssrcs.push_back(RTX_SSRC);
	video_send_config.rtp.rtx.payload_type = kPayloadTypeRtx;

	video_send_config.rtp.extensions.clear();
	if(g_Send_side_bwe){
		video_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,kTransportSequenceNumberExtensionId));
	}else{
		video_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kAbsSendTimeUri,kAbsSendTimeExtensionId));
	}
	video_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kVideoContentTypeUri,kVideoContentTypeExtensionId));
	video_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kVideoTimingUri,kVideoTimingExtensionId));
	webrtc::VideoEncoderConfig encoder_config;
	
	encoder_config.codec_type = webrtc::kVideoCodecVP8;
	encoder_config.number_of_streams = 1;
	encoder_config.min_transmit_bitrate_bps = VIDEO_MIN_BPS;
	encoder_config.max_bitrate_bps = VIDEO_MAX_BPS;
	encoder_config.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;
	
	webrtc::VideoCodecVP8 vp8_settings = webrtc::VideoEncoder::GetDefaultVp8Settings();
	encoder_config.encoder_specific_settings = new rtc::RefCountedObject<webrtc::VideoEncoderConfig::Vp8EncoderSpecificSettings>(vp8_settings);
	encoder_config.video_stream_factory = new rtc::RefCountedObject<cricket::EncoderStreamFactory>("VP8", USED_MAX_VIDEO_QP, USED_VIDEO_FPS, false, false);
	
	g_VideoSendStream = g_Call->CreateVideoSendStream(std::move(video_send_config), std::move(encoder_config));
	if (g_VideoSendStream != nullptr)
	{
		retval = SUCESS_RET;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
		//g_VideoSendStream->SetSource(g_Video_capturers.get(), webrtc::VideoSendStream::DegradationPreference::kBalanced);
	}
	else
	{
		retval = FAILED_RET;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Failed!";
	}

	return retval;
}
void libwtk_destroy_video_send_stream(void)
{
	if (g_VideoSendStream != nullptr && g_Call != nullptr)
	{
		g_Call->DestroyVideoSendStream(g_VideoSendStream);
		g_VideoSendStream = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
	}
}

int libwtk_create_video_receive_stream(int rtp_format)
{
	int retval = SUCESS_RET;
	if(g_VideoReceiveStream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoReceiveStream already exsit, so return success!";
		return retval;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_VideoReceiveStream is nullprt, so go on!";
	}
	webrtc::VideoReceiveStream::Config video_rev_config(g_VideoSendTransport);
	//video_rev_config.renderer = g_Remote_render.get();
	video_rev_config.sync_group = AV_SYNC_GROUP;
	video_rev_config.rtp.remote_ssrc = SEND_SSRC;
	video_rev_config.rtp.local_ssrc = RECV_SSRC;
	video_rev_config.rtp.rtx_ssrc = RTX_SSRC;
	video_rev_config.rtp.rtx_associated_payload_types[kPayloadTypeRtx] = kPayloadTypeVP8;
	video_rev_config.rtp.rtcp_xr.receiver_reference_time_report = true;
	video_rev_config.rtp.nack.rtp_history_ms = 1000; 
	video_rev_config.rtp.transport_cc = g_Send_side_bwe;
	video_rev_config.rtp.remb = !g_Send_side_bwe;
	
	video_rev_config.rtp.extensions.clear();
	if(g_Send_side_bwe){
		video_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,kTransportSequenceNumberExtensionId));
	}else{
		video_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kAbsSendTimeUri,kAbsSendTimeExtensionId));
	}
	video_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kVideoContentTypeUri,kVideoContentTypeExtensionId));
  video_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kVideoTimingUri,kVideoTimingExtensionId));
	webrtc::VideoReceiveStream::Decoder decoder_config;
	decoder_config.payload_name = "VP8";
	decoder_config.payload_type = kPayloadTypeVP8;
	decoder_config.decoder = webrtc::VP8Decoder::Create().release();
	video_rev_config.decoders.push_back(decoder_config);

	g_VideoReceiveStream = g_Call->CreateVideoReceiveStream(std::move(video_rev_config));
	if (g_VideoReceiveStream != nullptr)
	{
		retval = SUCESS_RET;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		retval = FAILED_RET;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Failed!";
	}

	return retval;
}
void libwtk_destroy_video_receive_stream(void)
{
	if (g_VideoReceiveStream != nullptr && g_Call != nullptr)
	{
		g_Call->DestroyVideoReceiveStream(g_VideoReceiveStream);
		g_VideoReceiveStream = nullptr;
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , no stream to destroy!";
	}
}
void libwtk_audio_stream_mute(bool mute)
{	
	RTC_LOG(LS_INFO) << __FUNCTION__ << " , mute = " << mute;
}

void libwtk_start_audio_stream(void)
{	
	if((g_AudioSendStream != nullptr) && g_AudioReceiveStream != nullptr)
	{
		g_AudioSendStream->Start();
		g_AudioReceiveStream->Start();
		g_Call->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkUp);
	}
	else
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , g_AudioSendStream or g_AudioReceiveStream is null";
}
void libwtk_start_video_stream(void)
{
	if((g_VideoSendStream != nullptr) && g_VideoReceiveStream != nullptr)
	{
		g_VideoSendStream->Start();
		g_VideoReceiveStream->Start();
		g_Call->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkUp);
	}
	else
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , g_AudioSendStream or g_AudioReceiveStream is null";
}
void libwtk_stop_audio_stream(void)
{
	RTC_LOG(LS_INFO) << __FUNCTION__ << " , g_AudioSendStream or g_AudioReceiveStream ";

	if((g_AudioSendStream != nullptr) && g_AudioReceiveStream != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , g_AudioReceiveStream stop";
		g_AudioReceiveStream->Stop();
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , g_AudioSendStream stop";
		g_AudioSendStream->Stop();
		
		g_Call->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkDown);
	}
	else
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , g_AudioSendStream or g_AudioReceiveStream is null";
}
void libwtk_stop_video_stream(void)
{
	if((g_VideoSendStream != nullptr) && g_VideoReceiveStream != nullptr)
	{
		g_VideoSendStream->Stop();
		g_VideoReceiveStream->Stop();
		g_Call->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkDown);
	}
	else
		RTC_LOG(LS_INFO) << __FUNCTION__ << " , g_AudioSendStream or g_AudioReceiveStream is null";
}
