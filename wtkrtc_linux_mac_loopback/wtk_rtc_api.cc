#include "wtk_rtc_api.h"

#define USED_AUDIO_DEVICE_ID 0
#define USED_VIDEO_DEVICE_ID 0

#define USED_VIDEO_WIDTH 	640
#define USED_VIDEO_HEIGHT 	480
#define USED_VIDEO_FPS 		30
#define USED_MAX_VIDEO_QP 	48

#define SEND_SSRC 	11111111
#define RECV_SSRC 	22222222
#define RTX_SSRC 	33333333

#define AV_SYNC_GROUP "av_sync"

#define AUDIO_MIN_BPS	6 * 1000
#define AUDIO_MAX_BPS	32 * 1000

#define VIDEO_MIN_BPS	50 * 1000
#define VIDEO_MAX_BPS	800 * 1000

#define CALL_MIN_BPS	60 * 1000
#define CALL_START_BPS	300 * 1000
#define CALL_MAX_BPS	800 * 1000

bool g_Send_side_bwe = false;

rtc::scoped_refptr<webrtc::AudioDeviceModule> g_Adm = nullptr;
std::unique_ptr<webrtc::RtcEventLog> g_Event_log = nullptr;

webrtc::Call* g_Call = nullptr;

webrtc::AudioSendStream *g_AudioSendStream = nullptr;
webrtc::AudioReceiveStream *g_AudioReceiveStream = nullptr;
webrtc::VideoSendStream *g_VideoSendStream = nullptr;
webrtc::VideoReceiveStream *g_VideoReceiveStream = nullptr;

std::unique_ptr<webrtc::test::VideoRenderer> g_Local_render = nullptr;
std::unique_ptr<webrtc::test::VideoRenderer> g_Remote_render = nullptr;
std::unique_ptr<webrtc::test::VideoCapturer> g_Video_capturers = nullptr;

enum classPayloadTypes{
	kPayloadTypeRtx = 98,
	kPayloadTypeIlbc = 102,
	kPayloadTypeOpus = 111,
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

class AudioLoopTransport:public webrtc::Transport{
public:
    bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options) override
    {		
        int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
				rtc::SentPacket sent_packet(options.packet_id,send_time);
				g_Call->OnSentPacket(sent_packet);
				g_Call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));
        return true;
    }

    bool SendRtcp(const uint8_t* packet, size_t length) override
    {
				int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
				g_Call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));
				return true;
    }
};

class VideoLoopTransport:public webrtc::Transport{
public:
    bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options) override
    {
			//RTC_LOG(LS_INFO) << "SendRtp started";
			int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
			rtc::SentPacket sent_packet(options.packet_id,send_time);
			g_Call->OnSentPacket(sent_packet);
			g_Call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));
			return true;
    }

    bool SendRtcp(const uint8_t* packet, size_t length) override
		{
			//RTC_LOG(LS_INFO) << "SendRtcp started";
			int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
			g_Call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));

			return true;
    }
};

AudioLoopTransport* g_AudioSendTransport = new AudioLoopTransport();//nullptr;
VideoLoopTransport* g_VideoSendTransport = new VideoLoopTransport();//nullptr;

void StartCapture(void)
{
	g_Video_capturers.reset(webrtc::test::VcmCapturer::Create(USED_VIDEO_WIDTH, USED_VIDEO_HEIGHT,USED_VIDEO_FPS,USED_VIDEO_DEVICE_ID));
	g_Video_capturers->Start();
}

void CreateAudioSendStream(void)
{
	webrtc::AudioSendStream::Config audio_send_config(g_AudioSendTransport);
	audio_send_config.send_codec_spec = webrtc::AudioSendStream::Config::SendCodecSpec(kPayloadTypeOpus, {"OPUS", 48000, 2,{{"usedtx", "0"},{"stereo", "1"}}});
	audio_send_config.encoder_factory = webrtc::CreateAudioEncoderFactory<webrtc::AudioEncoderOpus>();
	audio_send_config.rtp.ssrc = SEND_SSRC + 1;
	audio_send_config.rtp.extensions.clear();

	if(g_Send_side_bwe)
	{
		audio_send_config.min_bitrate_bps = AUDIO_MIN_BPS;
		audio_send_config.max_bitrate_bps = AUDIO_MAX_BPS;	
		
		audio_send_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,kTransportSequenceNumberExtensionId));
	}
	g_AudioSendStream = g_Call->CreateAudioSendStream(audio_send_config);
}
void CreateAudioReceiveStream(void)
{
	//webrtc::AudioDecoderIlbc
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
		audio_rev_config.rtp.extensions.push_back(webrtc::RtpExtension(webrtc::RtpExtension::kTransportSequenceNumberUri,kTransportSequenceNumberExtensionId));
	}
	g_AudioReceiveStream = g_Call->CreateAudioReceiveStream(audio_rev_config);
}

void CreateVideoSendStream(void)
{
	webrtc::VideoSendStream::Config video_send_config(g_VideoSendTransport);

	video_send_config.rtp.ssrcs.push_back(SEND_SSRC);
	video_send_config.rtp.payload_name = "VP8";
	video_send_config.rtp.payload_type = kPayloadTypeVP8;
	video_send_config.pre_encode_callback = g_Local_render.get();
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
	
	/*webrtc::VideoStream stream;
	std::vector<webrtc::VideoStream> streams;
	stream.width = 352;//USED_VIDEO_WIDTH;
	stream.height = 288;//USED_VIDEO_HEIGHT;
	stream.max_framerate = USED_VIDEO_FPS;
	
	stream.min_bitrate_bps = 50 * 1000;
	stream.max_bitrate_bps = 800 * 1000;
	stream.target_bitrate_bps = 500 * 1000;
	
	stream.max_qp = USED_MAX_VIDEO_QP;
	
	streams.push_back(stream);
	encoder_config.video_stream_factory = new rtc::RefCountedObject<VideoStreamFactory>(streams);
	*/

	g_VideoSendStream = g_Call->CreateVideoSendStream(std::move(video_send_config), std::move(encoder_config));
	g_VideoSendStream->SetSource(g_Video_capturers.get(), webrtc::VideoSendStream::DegradationPreference::kBalanced);
}
void CreateVideoReceiveStream(void)
{
	webrtc::VideoReceiveStream::Config video_rev_config(g_VideoSendTransport);
	video_rev_config.renderer = g_Remote_render.get();
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
}

void SetupLocalRender(void)
{
	g_Local_render.reset(webrtc::test::VideoRenderer::Create("Video Local Preview Render #1", USED_VIDEO_WIDTH, USED_VIDEO_HEIGHT));
}

void SetupRomoteRender(void)
{
	g_Remote_render.reset(webrtc::test::VideoRenderer::Create("Video Remote Render #2 ", USED_VIDEO_WIDTH, USED_VIDEO_HEIGHT));
}

void StartAudioCallTest(void)
{	
	g_AudioSendStream->Start();
	g_AudioReceiveStream->Start();
}
void StartVideoCallTest(void)
{
	g_VideoSendStream->Start();
	g_VideoReceiveStream->Start();
}

void CreateCall(void)
{
	webrtc::AudioState::Config audioStateConfig;
	audioStateConfig.audio_device_module = g_Adm;
	audioStateConfig.audio_mixer = webrtc::AudioMixerImpl::Create();
	audioStateConfig.audio_processing = webrtc::AudioProcessingBuilder().Create();

	webrtc::BitrateConstraints call_bitrate_config;
	call_bitrate_config.min_bitrate_bps = CALL_MIN_BPS;
	call_bitrate_config.start_bitrate_bps = CALL_START_BPS;
	call_bitrate_config.max_bitrate_bps = CALL_MAX_BPS;
	
	g_Event_log = webrtc::RtcEventLog::Create(webrtc::RtcEventLog::EncodingType::Legacy);
	webrtc::CallConfig callConfig(g_Event_log.get());
	callConfig.audio_state = webrtc::AudioState::Create(audioStateConfig);
	callConfig.bitrate_config = call_bitrate_config;
	
	g_Adm->RegisterAudioCallback(callConfig.audio_state->audio_transport());

	g_Call = webrtc::Call::Create(callConfig);

	g_Call->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkUp);
	g_Call->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkUp);
}

void AudioDeviceModule_Setup(void)
{	
	bool stereo_playout = false;
	bool stereo_record = false;

	if(!g_Adm)
		g_Adm = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kPlatformDefaultAudio);
	g_Adm->Init();
	g_Adm->SetPlayoutDevice(USED_AUDIO_DEVICE_ID);
	g_Adm->InitSpeaker();
	g_Adm->SetRecordingDevice(USED_AUDIO_DEVICE_ID);
	g_Adm->InitMicrophone();
	
	g_Adm->StereoPlayoutIsAvailable(&stereo_playout);
	g_Adm->SetStereoPlayout(stereo_playout);
	g_Adm->StereoRecordingIsAvailable(&stereo_record);
	g_Adm->SetStereoRecording(stereo_record);
	g_Adm->InitPlayout();
	g_Adm->InitRecording();
}

void RunLoopTest(void)
{
	AudioDeviceModule_Setup();
	
	CreateCall();
	
	CreateAudioSendStream();
	CreateAudioReceiveStream();

	SetupLocalRender();
	SetupRomoteRender();
	StartCapture(); 
	
	CreateVideoSendStream();
	CreateVideoReceiveStream();
		
	StartAudioCallTest();
	StartVideoCallTest();

	while(1);
	/*for(int i = 0; i < 1000; i++)
	{
		webrtc::VideoSendStream::Stats send_stats;
		send_stats = g_VideoSendStream->GetStats();
		RTC_LOG(LS_INFO) << "GetVideoSendStats =====> " << send_stats.ToString(111111);
		sleep(30);
	}*/
}
	
int main(void)
{
	webrtc::test::RunTest(RunLoopTest);
	return 0;
}
