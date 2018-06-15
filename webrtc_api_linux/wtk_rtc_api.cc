#include "wtk_rtc_api.h"

#define USED_AUDIO_DEVICE_ID 0
#define USED_VIDEO_DEVICE_ID 0

#define USED_VIDEO_WIDTH 640
#define USED_VIDEO_HEIGHT 480
#define USED_VIDEO_FPS 30

rtc::scoped_refptr<webrtc::AudioDeviceModule> g_Adm = nullptr;
std::unique_ptr<webrtc::RtcEventLog> g_Event_log = nullptr;

webrtc::Call* g_Call = nullptr;

webrtc::AudioSendStream *g_AudioSendStream = nullptr;
webrtc::AudioReceiveStream *g_AudioReceiveStream = nullptr;
webrtc::VideoSendStream *g_VideoSendStream = nullptr;
webrtc::VideoReceiveStream *g_VideoReceiveStream = nullptr;

std::unique_ptr<webrtc::test::VideoRenderer> local_render = nullptr;
std::unique_ptr<webrtc::test::VideoRenderer> remote_render = nullptr;

enum classPayloadTypes{
    kPayloadTypeIlbc = 102,
    kPayloadTypeOpus = 111,
    kPayloadTypeH264 = 122,
    kPayloadTypeVP8 = 120,
    kPayloadTypeVP9 = 101,
};

class AudioLoopTransport:public webrtc::Transport{
public:
    bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options) override
    {		
        int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
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
        int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		g_Call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));
		
        return true;
    }

    bool SendRtcp(const uint8_t* packet, size_t length) override
    {
		int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
        g_Call->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, rtc::CopyOnWriteBuffer(packet, length), webrtc::PacketTime(send_time, -1));

        return true;
    }
};
class TestVideoEncoderFactory:public webrtc::VideoEncoderFactory {
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;

    CodecInfo QueryVideoEncoder(const webrtc::SdpVideoFormat& format) const override;

    std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(
        const webrtc::SdpVideoFormat& format) override;
private:
    webrtc::InternalEncoderFactory internal_encoder_factory_;
};

AudioLoopTransport* g_AudioSendTransport = nullptr;
VideoLoopTransport* g_VideoSendTransport = nullptr;

rtc::scoped_refptr<webrtc::AudioEncoderFactory> CreateWtkAudioEncoderFactory() {
  return webrtc::CreateAudioEncoderFactory<
#if WTK_USE_BUILTIN_OPUS
      webrtc::AudioEncoderOpus,
#endif
#if WTK_USE_BUILTIN_ILBC
      webrtc::AudioEncoderIlbc
#endif
>();
}
rtc::scoped_refptr<webrtc::AudioDecoderFactory> CreateWtkAudioDecoderFactory() {
  return webrtc::CreateAudioDecoderFactory<
#if WTK_USE_BUILTIN_OPUS
      webrtc::AudioDecoderOpus,
#endif
#if WTK_USE_BUILTIN_ILBC
      webrtc::AudioDecoderIlbc
#endif
>();
}

void CreateAudioSendStream(void)
{
	g_AudioSendTransport = new AudioLoopTransport();
    webrtc::AudioSendStream::Config config(g_AudioSendTransport);
	config.send_codec_spec = webrtc::AudioSendStream::Config::SendCodecSpec(kPayloadTypeOpus, {"OPUS", 48000, 2});
	config.encoder_factory = CreateWtkAudioEncoderFactory();

    g_AudioSendStream = g_Call->CreateAudioSendStream(config);
}
void CreateAudioReceiveStream(void)
{
	rtc::scoped_refptr<webrtc::AudioDecoderFactory> audioDecoderFactory = CreateWtkAudioDecoderFactory();
    webrtc::AudioReceiveStream::Config config;
	config.rtcp_send_transport = g_AudioSendTransport;
    config.decoder_factory = audioDecoderFactory;
	config.sync_group = "AudioVideoLoopbackTest";
	config.decoder_map = {{kPayloadTypeOpus, {"OPUS", 48000, 2}}};

    g_AudioReceiveStream = g_Call->CreateAudioReceiveStream(config);
}

void CreateVideoSendStream(void)
{
	g_VideoSendTransport = new VideoLoopTransport();
    webrtc::VideoSendStream::Config send_config(g_VideoSendTransport);
	/*webrtc::VideoEncoderFactory* encoder_factory;
	webrtc::SdpVideoFormat video_format;
	video_format.name = "VP8";
	encoder_factory->CreateVideoEncoder(video_format);
	for (webrtc::SdpVideoFormat& format : encoder_factory->GetSupportedFormats()) {
		RTC_LOG(INFO) << __FUNCTION__ << ", format name = " << format.name;
	}*/
	TestVideoEncoderFactory g_video_encoder_factory;
	//g_video_encoder_factory = new TestVideoEncoderFactory();
	send_config.encoder_settings.encoder_factory = &g_video_encoder_factory;
	send_config.rtp.payload_name = "VP8";
	send_config.rtp.payload_type = kPayloadTypeVP8;
	
	webrtc::VideoEncoderConfig encoder_config;
	encoder_config.codec_type = webrtc::kVideoCodecVP8;
	encoder_config.number_of_streams = 1;
	encoder_config.max_bitrate_bps = 100000;
	encoder_config.simulcast_layers = std::vector<webrtc::VideoStream>(1);
	encoder_config.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;

    g_VideoSendStream = g_Call->CreateVideoSendStream(std::move(send_config), std::move(encoder_config));
}
void CreateVideoReceiveStream(void)
{
    webrtc::VideoReceiveStream::Config config(g_VideoSendTransport);
	config.renderer = remote_render.get();
    config.sync_group = "AudioVideoLoopbackTest";

	webrtc::VideoReceiveStream::Decoder decoder;
	decoder.payload_name = "VP8";
	decoder.payload_type = kPayloadTypeVP8;
	config.decoders.push_back(decoder);

    g_VideoReceiveStream = g_Call->CreateVideoReceiveStream(std::move(config));
}

void SetupLocalRender(void)
{
	local_render.reset(webrtc::test::VideoRenderer::Create("Video Local Render", USED_VIDEO_WIDTH, USED_VIDEO_HEIGHT));
}

void SetupRomoteRender(void)
{
	remote_render.reset(webrtc::test::VideoRenderer::Create("Video Remote Render", USED_VIDEO_WIDTH, USED_VIDEO_HEIGHT));
}

void StartCapture(void)
{
	std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> device_info(webrtc::VideoCaptureFactory::CreateDeviceInfo());

	char device_name[256]={0};
    char unique_name[256]={0};
	//int number_of_devices = device_info->NumberOfDevices();

	device_info->GetDeviceName(USED_VIDEO_DEVICE_ID, device_name, sizeof(device_name), unique_name, sizeof(unique_name));

	rtc::scoped_refptr<webrtc::VideoCaptureModule> device_module(webrtc::VideoCaptureFactory::Create(unique_name));
	
	webrtc::VideoCaptureCapability capability;
	capability.width = USED_VIDEO_WIDTH;
	capability.height = USED_VIDEO_HEIGHT;
	capability.maxFPS = USED_VIDEO_FPS;
	capability.videoType = webrtc::VideoType::kUnknown;

	device_module->StartCapture(capability);
}

void StartCallTest(void)
{
	g_Call->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkUp);
	g_Call->SignalChannelNetworkState(webrtc::MediaType::VIDEO, webrtc::kNetworkUp);

    g_AudioSendStream->Start();
    g_AudioReceiveStream->Start();
	g_VideoSendStream->Start();
    g_VideoReceiveStream->Start();
}

void CreateCall(void)
{
	webrtc::AudioState::Config audioStateConfig;
	audioStateConfig.audio_device_module = g_Adm;
	audioStateConfig.audio_mixer = webrtc::AudioMixerImpl::Create();
	audioStateConfig.audio_processing = webrtc::AudioProcessingBuilder().Create();
	
	g_Event_log = webrtc::RtcEventLog::Create(webrtc::RtcEventLog::EncodingType::Legacy);
	webrtc::CallConfig callConfig(g_Event_log.get());
	callConfig.audio_state = webrtc::AudioState::Create(audioStateConfig);
	g_Adm->RegisterAudioCallback(callConfig.audio_state->audio_transport());

	g_Call = webrtc::Call::Create(callConfig);
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

int main(void)
{
	AudioDeviceModule_Setup();
	
	CreateCall();

	CreateAudioSendStream();
	CreateAudioReceiveStream();

	SetupLocalRender();
	SetupLocalRender();
	StartCapture();	
	
	CreateVideoSendStream();
	CreateVideoReceiveStream();
	
	StartCallTest();
	
	while(1);

	return 0;
}
