#ifndef _wtk_rtc_api_h
#define _wtk_rtc_api_h

#define OPUS_SAMPLE_RATE_HZ 48000
#define OPUS_SAMPLES_PER_CHAN OPUS_SAMPLE_RATE_HZ/100
#define OPUS_NUMBER_OF_CHAN	1
#define MAX_PARTICIPANT 50
#define MAX_VIDEO_PARTICIPANT 4

enum videoCodec{
    kWtkVideoCodecVP8 = 0,
    kWtkVideoCodecVP9 = 1,
    kWtkVideoCodecH264 = 2,
    kWtkVideoCodecH264Auto = 3,
};

enum classPayloadTypes{
	kWtkPayloadTypeRtx = 98,
    kWtkPayloadTypeIlbc = 97,
    kWtkPayloadTypeOpus = 107,
    kWtkPayloadTypeH264 = 99,
    kWtkPayloadTypeVP8 = 100,
    kWtkPayloadTypeVP9 = 108,
};

#define RTC_EXPORT __attribute__((visibility("default")))

typedef int (*audio_transport_mixer_callback_t)(const uint8_t* buf, int len,int channel);
typedef int (*video_transport_mixer_callback_t)(const uint8_t* buf, int len,int channel);
#ifdef __cplusplus
extern "C" {
#endif
RTC_EXPORT extern void 	libwtk_set_mixer_audio_transport(audio_transport_mixer_callback_t func);
RTC_EXPORT extern void 	libwtk_set_mixer_video_transport(video_transport_mixer_callback_t func);
RTC_EXPORT extern int	libwtk_mixer_decode_audio(uint8_t* buf, int buflen,int channel);
RTC_EXPORT extern int 	libwtk_mixer_decode_video(uint8_t* buf, int buflen,int channel);
RTC_EXPORT extern void	libwtk_mixer_init(void);
RTC_EXPORT extern void	libwtk_mixer_deinit(void);
RTC_EXPORT extern void	libwtk_mixer_setup_mixer(int channel);
#ifdef __cplusplus
}
#endif

#endif
