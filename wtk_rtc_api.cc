#include "wtk_rtc_api.h"


//webrtc::cricket::WebRtcVoiceEngine* g_VoiceEngine = nullptr;
rtc::scoped_refptr<webrtc::AudioDeviceModule> g_adm = nullptr;
rtc::scoped_refptr<webrtc::AudioProcessing> g_apm = nullptr;
rtc::scoped_refptr<webrtc::AudioMixerImpl> g_amm = nullptr;

std::unique_ptr<webrtc::RtcEventLog> event_log = nullptr;

/*webrtc::AudioDeviceModule* g_adm = nullptr;
webrtc::AudioProcessing* g_apm = nullptr;
webrtc::AudioMixerImpl* g_amm = nullptr;*/
webrtc::AudioCodingModule* g_acm = nullptr;
webrtc::Call* g_call = nullptr;

webrtc::AudioSendStream *g_audioSendStream = nullptr;
webrtc::AudioReceiveStream *g_audioReceiveStream = nullptr;

#define AUDIO_SEND_SSRC		1111
#define AUDIO_REC_SSRC		2222

class AudioLoopTransport:public webrtc::Transport{
public:
    bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options) override
    {
		RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";
		
        int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
        if(g_call != nullptr){
	        webrtc::PacketReceiver::DeliveryStatus status = webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK;
			rtc::CopyOnWriteBuffer buffer(packet, length);
			webrtc::PacketReceiver* rev = g_call->Receiver();
			rev->DeliverPacket(webrtc::MediaType::AUDIO, buffer, webrtc::PacketTime(send_time, -1));
			
	        assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
	        RTC_LOG(INFO) << __FUNCTION__ << " =============================>status = " << status ;
    	}
        return true;
    }

    bool SendRtcp(const uint8_t* packet, size_t length) override
    {
		RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";
		//int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
        webrtc::PacketReceiver::DeliveryStatus status = g_call->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, 
																					        rtc::CopyOnWriteBuffer(packet, length), 
																					        webrtc::PacketTime());
        assert(status == webrtc::PacketReceiver::DeliveryStatus::DELIVERY_OK);
        RTC_LOG(INFO) << __FUNCTION__ << " =============================>status = " << status ;
        return true;
    }
};
AudioLoopTransport* g_audioSendTransport = nullptr;

void CreateAudioSendStream(void)
{
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";
	g_audioSendTransport = new AudioLoopTransport();
    webrtc::AudioSendStream::Config config(g_audioSendTransport);
	config.rtp.ssrc = AUDIO_SEND_SSRC;
	//config.min_bitrate_bps = 100*1000;
	//config.max_bitrate_bps = 500*1000;
	config.send_codec_spec = webrtc::AudioSendStream::Config::SendCodecSpec(102, {"ILBC", 8000, 1});
	config.encoder_factory = webrtc::CreateBuiltinAudioEncoderFactory();

    g_audioSendStream = g_call->CreateAudioSendStream(config);
	

    assert(g_audioSendStream);
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>ended";
}

void CreateAudioReceiveStream(void)
{
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";

	rtc::scoped_refptr<webrtc::AudioDecoderFactory> audioDecoderFactory = webrtc::CreateBuiltinAudioDecoderFactory();
    webrtc::AudioReceiveStream::Config config;
    config.decoder_factory = audioDecoderFactory;
	config.rtp.remote_ssrc = AUDIO_SEND_SSRC;
	config.rtp.local_ssrc = AUDIO_REC_SSRC;
	config.decoder_map = {{102, {"ILBC", 8000, 1}}};

    g_audioReceiveStream = g_call->CreateAudioReceiveStream(config);

    assert(g_audioReceiveStream);
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>ended";
}

void StartCall(void)
{
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";
	g_call->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkUp);

    g_audioSendStream->Start();
    g_audioReceiveStream->Start();
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>ended";
}

void CallApi_Setup(void)
{
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";
	
	webrtc::AudioState::Config audioStateConfig;
	audioStateConfig.audio_device_module = g_adm;
	audioStateConfig.audio_mixer = g_amm;
	audioStateConfig.audio_processing = g_apm;
	
	event_log = webrtc::RtcEventLog::Create(webrtc::RtcEventLog::EncodingType::Legacy);
	webrtc::CallConfig callConfig(event_log.get());

	callConfig.audio_state = webrtc::AudioState::Create(audioStateConfig);
	//callConfig.bitrate_config.max_bitrate_bps = 500*1000;
	//callConfig.bitrate_config.min_bitrate_bps = 100*1000;
	//callConfig.bitrate_config.start_bitrate_bps = 250*1000;

	g_call = webrtc::Call::Create(callConfig);

	g_adm->RegisterAudioCallback(callConfig.audio_state->audio_transport());
	
	assert(g_call);

	RTC_LOG(INFO) << __FUNCTION__ << " =============================>ended";
}

void Amm_Setup(void)
{
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";

	if(!g_amm)
		g_amm =  webrtc::AudioMixerImpl::Create();
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>ended";
}
void Apm_Setup(void)
{
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";

	if(!g_apm)
		g_apm = webrtc::AudioProcessingBuilder().Create();

	RTC_LOG(INFO) << __FUNCTION__ << " =============================>ended";
}
void Adm_Setup(void)
{
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";
	
	if(!g_adm)
		g_adm = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kLinuxAlsaAudio);//kPlatformDefaultAudio
	//g_adm->Init();
	
	webrtc::AudioDeviceModule::AudioLayer audio_layer;
	int got_platform_audio_layer = g_adm->ActiveAudioLayer(&audio_layer);
	bool requirements_satisfied_ = true;
	bool stereo_playout_ = false;
	// First, ensure that a valid audio layer can be activated.
	if (got_platform_audio_layer != 0) {
		requirements_satisfied_ = false;
	}
	// Next, verify that the ADM can be initialized.
	if (requirements_satisfied_) {
		requirements_satisfied_ = (g_adm->Init() == 0);
	}
	// Finally, ensure that at least one valid device exists in each direction.
#define USED_DEVICE_ID 0
    if (requirements_satisfied_) {
		if (g_adm->SetPlayoutDevice(USED_DEVICE_ID) != 0) {
		    RTC_LOG(INFO) << __FUNCTION__ << ", SetPlayoutDevice error";
		}
		if (g_adm->InitSpeaker() != 0) {
		    RTC_LOG(INFO) << __FUNCTION__ << ", InitSpeaker error";
		}
		if (g_adm->SetRecordingDevice(USED_DEVICE_ID) != 0) {
		    RTC_LOG(INFO) << __FUNCTION__ << ", SetRecordingDevice error";
		}
		if (g_adm->InitMicrophone() != 0) {
		    RTC_LOG(INFO) << __FUNCTION__ << ", InitMicrophone error";
		}
		if (g_adm->StereoPlayoutIsAvailable(&stereo_playout_) != 0) {
		    RTC_LOG(INFO) << __FUNCTION__ << ", StereoPlayoutIsAvailable error";
		}
		if (g_adm->SetStereoPlayout(stereo_playout_) != 0) {
		    RTC_LOG(INFO) << __FUNCTION__ << ", SetStereoPlayout error";
		}
		if (g_adm->SetStereoRecording(false) != 0) {
		    RTC_LOG(INFO) << __FUNCTION__ << ", SetStereoRecording error";
		}

		if (g_adm->InitPlayout() != 0) {
		    RTC_LOG(INFO) << __FUNCTION__ << ", InitPlayout error";
		}
		/*if (g_adm->StartPlayout() != 0) {
			RTC_LOG(INFO) << __FUNCTION__ << ", StartPlayout error";
		}*/
		// start record
		if (g_adm->InitRecording() != 0) {
		    RTC_LOG(INFO) << __FUNCTION__ << ", InitRecording error";
		}
		/*if (g_adm->StartRecording() != 0) {
			RTC_LOG(INFO) << __FUNCTION__ << ", StartRecording error";
		}*/
    }
	//while(1);
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>ended";
}
void Acm_Setup(void)
{
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>start";

	webrtc::CodecInst dummyCodec;
	webrtc::CodecInst usedCodec;
	if(!g_acm)
			g_acm = webrtc::AudioCodingModule::Create(webrtc::AudioCodingModule::Config(webrtc::CreateBuiltinAudioDecoderFactory()));

	uint8_t numCodecs = g_acm->NumberOfCodecs();
	for (uint8_t n = 0; n < numCodecs; n++) {
		g_acm->Codec(n, &dummyCodec);
		if (STR_CASE_CMP(dummyCodec.plname, "ILBC") == 0)
		{
			usedCodec = dummyCodec;
		}
	}
	g_acm->RegisterSendCodec(usedCodec);
	g_acm->RegisterReceiveCodec(usedCodec);
	
	RTC_LOG(INFO) << __FUNCTION__ << " used coded: " << usedCodec;
	RTC_LOG(INFO) << __FUNCTION__ << " =============================>ended";
}
int main(void)
{
	RTC_LOG(INFO) << "++++++++++++++++++++Test main start+++++++++++++++++++++++";
	//Acm_Setup();
	Adm_Setup();
	Apm_Setup();
	Amm_Setup();
	
	CallApi_Setup();
	CreateAudioSendStream();
	CreateAudioReceiveStream();
	StartCall();
	while(1);
	RTC_LOG(INFO) << "++++++++++++++++++++Test main ended+++++++++++++++++++++++";
	return 0;
}
