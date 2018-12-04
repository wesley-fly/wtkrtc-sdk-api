#include <stdbool.h>
#include <memory>
#include <utility>
#include <vector>
#include <string>
#include "call/call.h"
#include "rtc_base/logging.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_device/include/fake_audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/audio_mixer/audio_mixer_impl.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
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
#include "audio/audio_receive_stream.h"
#include "system_wrappers/include/clock.h"
#include "modules/rtp_rtcp/include/rtp_header_parser.h"
#include "api/call/audio_sink.h"
#include "rtc_base/bufferqueue.h"
#include "wtk_rtc_mixer_api.h"
#include "fakeaudiocapturemodule.h"

static std::unique_ptr<webrtc::Call> g_mixer_call[MAX_PARTICIPANT] = {nullptr};
static rtc::scoped_refptr<webrtc::AudioMixer> g_mixer_audio_mixer = nullptr;
rtc::scoped_refptr<webrtc::AudioDeviceModule> g_mixer_adm[MAX_PARTICIPANT] = {nullptr};

static webrtc::AudioReceiveStream* mixer_audio_receive_stream[MAX_PARTICIPANT] = {nullptr};
static webrtc::AudioSendStream* mixer_audio_send_stream[MAX_PARTICIPANT] = {nullptr};

static video_transport_mixer_callback_t send_mixer_audio_packet = nullptr;
static video_transport_mixer_callback_t send_mixer_video_packet = nullptr;


webrtc::AudioFrame mixed_audio_frame;
uint32_t wtk_audio_ssrc = 10000000;
uint32_t wtk_video_ssrc = 10000001;

void start_mix_and_send(int channel);

class AudioMixerTransport:public webrtc::Transport{
public:
	explicit AudioMixerTransport(int channel):channel_(channel){}
    bool SendRtp(const uint8_t* packet,size_t length,const webrtc::PacketOptions& options) override
    {		
        int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		rtc::SentPacket sent_packet(options.packet_id,send_time);
    	g_mixer_call[channel_]->OnSentPacket(sent_packet);
		send_mixer_audio_packet(packet, length, channel_);
        return true;
    }

    bool SendRtcp(const uint8_t* packet, size_t length) override
    {
        send_mixer_audio_packet(packet, length, channel_);
        return true;
    }
private:
	int channel_;
};

class ParticipantSinkSource:public webrtc::AudioMixer::Source,public webrtc::AudioSinkInterface {
public:
	explicit ParticipantSinkSource(int channel):channel_(channel){}
	void OnData(const Data& audio) override {
		memcpy(audio_data, audio.data, sizeof(int16_t) * OPUS_SAMPLES_PER_CHAN);
		start_mix_and_send(channel_);
	}
	AudioFrameInfo GetAudioFrameWithInfo(int target_rate_hz,webrtc::AudioFrame* frame) override {
		frame->samples_per_channel_ = OPUS_SAMPLES_PER_CHAN;
		frame->num_channels_ = OPUS_NUMBER_OF_CHAN;
		frame->sample_rate_hz_ = target_rate_hz;
		memcpy(frame->mutable_data(), audio_data, sizeof(int16_t) * OPUS_SAMPLES_PER_CHAN);
		
		return AudioFrameInfo::kNormal;
	}
	int Ssrc() const override { return 0; }
	int PreferredSampleRate() const override { return OPUS_SAMPLE_RATE_HZ; }
private:
	int channel_;
	int16_t audio_data[sizeof(int16_t) * OPUS_SAMPLES_PER_CHAN];
};
static ParticipantSinkSource* audio_sink_source[MAX_PARTICIPANT] = {nullptr};

void libwtk_set_mixer_audio_transport(audio_transport_mixer_callback_t func)
{
	send_mixer_audio_packet = func;
}
void libwtk_set_mixer_video_transport(video_transport_mixer_callback_t func)
{
	send_mixer_video_packet = func;
}
int libwtk_mixer_decode_audio(uint8_t* buf, int buflen, int channel)
{
	if( buflen && g_mixer_call[channel] != nullptr )
	{
		int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		g_mixer_call[channel]->Receiver()->DeliverPacket(webrtc::MediaType::AUDIO, rtc::CopyOnWriteBuffer(buf, buflen), webrtc::PacketTime(send_time, -1));
	}
	return buflen;
}
int libwtk_mixer_decode_video(uint8_t* buf, int buflen, int channel)
{
	if( buflen && g_mixer_call[channel] != nullptr )
	{
		int64_t send_time = webrtc::Clock::GetRealTimeClock()->TimeInMicroseconds();
		g_mixer_call[channel]->Receiver()->DeliverPacket(webrtc::MediaType::VIDEO, rtc::CopyOnWriteBuffer(buf, buflen), webrtc::PacketTime(send_time, -1));
	}
	return buflen;
}

void libwtk_mixer_init(void)
{
	if(g_mixer_audio_mixer == nullptr)
	{
		g_mixer_audio_mixer = webrtc::AudioMixerImpl::Create();
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :Init g_mixer_audio_mixer Success!";
	}
	else
	{
		RTC_LOG(LS_INFO) << " g_mixer_audio_mixer already exsit, so just use it success!";
	}
}
void libwtk_mixer_deinit(void)
{
	if(g_mixer_audio_mixer == nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :DeInit g_mixer_audio_mixer Success!";
	}
	else
	{
		g_mixer_audio_mixer.release();
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :DeInit g_mixer_audio_mixer.release Success!";
	}
}

void libwtk_mixer_setup_mixer(int channel)
{
	if(g_mixer_call[channel] != nullptr)
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_call already exsit, so return success!";
		return;
	}
	else
	{
		RTC_LOG(LS_INFO) << __FUNCTION__ << " :g_call is nullprt, so start creat call object!";
	}

	//rtc::scoped_refptr<webrtc::AudioDeviceModule> adm = FakeAudioCaptureModule::Create();

	g_mixer_adm[channel] = FakeAudioCaptureModule::Create();

	static std::unique_ptr<webrtc::RtcEventLog> event_log = webrtc::RtcEventLog::Create(webrtc::RtcEventLog::EncodingType::Legacy);
	webrtc::CallConfig call_config(event_log.get());
	
	webrtc::AudioState::Config audio_state_config;
	audio_state_config.audio_device_module = g_mixer_adm[channel];
	audio_state_config.audio_mixer = webrtc::AudioMixerImpl::Create();
	audio_state_config.audio_processing = webrtc::AudioProcessingBuilder().Create();
	call_config.audio_state = webrtc::AudioState::Create(audio_state_config);
	
	g_mixer_adm[channel]->RegisterAudioCallback(call_config.audio_state->audio_transport());
	
	g_mixer_call[channel].reset(webrtc::Call::Create(call_config));

	//audio send
	webrtc::AudioSendStream::Config audio_send_config(new AudioMixerTransport(channel));
	audio_send_config.send_codec_spec = webrtc::AudioSendStream::Config::SendCodecSpec(kWtkPayloadTypeOpus, {"OPUS", 48000, 2,{{"usedtx", "0"},{"stereo", "1"}}});
	audio_send_config.encoder_factory = webrtc::CreateAudioEncoderFactory<webrtc::AudioEncoderOpus>();
	audio_send_config.rtp.ssrc = wtk_audio_ssrc;
	audio_send_config.rtp.nack.rtp_history_ms = 1000; 
	audio_send_config.min_bitrate_bps = 16*1000;
	audio_send_config.max_bitrate_bps = 32*1000;
	audio_send_config.rtp.extensions.clear();
	mixer_audio_send_stream[channel] = g_mixer_call[channel]->CreateAudioSendStream(audio_send_config);

	//audio receive
	webrtc::AudioReceiveStream::Config audio_rev_config;
	audio_rev_config.rtcp_send_transport = new AudioMixerTransport(channel);
    audio_rev_config.decoder_factory = webrtc::CreateAudioDecoderFactory<webrtc::AudioDecoderOpus>();
    audio_rev_config.sync_group = "wtk_av_sync";
	audio_rev_config.decoder_map = {{kWtkPayloadTypeOpus, {"OPUS", 48000, 2}}};
	audio_rev_config.rtp.remote_ssrc = wtk_audio_ssrc;
	audio_rev_config.rtp.local_ssrc = wtk_audio_ssrc;
	audio_rev_config.rtp.transport_cc = true;
	audio_rev_config.rtp.extensions.clear();
    mixer_audio_receive_stream[channel] = g_mixer_call[channel]->CreateAudioReceiveStream(audio_rev_config);

	//Add source and mix	
	audio_sink_source[channel] = new ParticipantSinkSource(channel);
	mixer_audio_receive_stream[channel]->SetSink(audio_sink_source[channel]);
	g_mixer_audio_mixer->AddSource(audio_sink_source[channel]);
	
	//start audio send/recive
	mixer_audio_send_stream[channel]->Start();
    mixer_audio_receive_stream[channel]->Start();

	g_mixer_call[channel]->SignalChannelNetworkState(webrtc::MediaType::AUDIO, webrtc::kNetworkUp);
}

void start_mix_and_send(int channel)
{
	if(channel == 0)
	{
		g_mixer_audio_mixer->RemoveSource(audio_sink_source[channel]);
		g_mixer_audio_mixer->Mix(OPUS_NUMBER_OF_CHAN, &mixed_audio_frame);
		g_mixer_audio_mixer->AddSource(audio_sink_source[channel]);
	}
	g_mixer_adm[channel]->SpeakerVolume((uint32_t*)mixed_audio_frame.data());
}

