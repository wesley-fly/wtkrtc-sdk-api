#ifndef _wtk_rtc_api_h
#define _wtk_rtc_api_h

#include <stdbool.h>

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

typedef int (*audio_transport_callback_t)(const uint8_t* buf, int len);
typedef int (*video_transport_callback_t)(const uint8_t* buf, int len);
#ifdef __cplusplus
extern "C" {
#endif

#ifdef WEBRTC_ANDROID
extern int libwtk_init_AndroidVideoEnv(void* javaVM, void* context);
#endif
extern void libwtk_set_audio_transport(audio_transport_callback_t func);
extern void libwtk_set_video_transport(video_transport_callback_t func);
extern int libwtk_decode_audio(uint8_t* buf, int buflen);
extern int libwtk_decode_video(uint8_t* buf, int buflen);
extern void libwtk_config_video(int codec, int width, int height, int fps, int maxqp);
extern void libwtk_config_bitrate(int audio_min_bps, int audio_max_bps, int video_min_bps, int video_max_bps);
extern int libwtk_create_remote_render(void* surfaceView);
extern int libwtk_create_capture(int is_front);
extern void libwtk_start_capture(void);
extern void libwtk_stop_capture(void);
extern void libwtk_switch_camera(int device_id);
extern void libwtk_destory_capture();
extern void libwtk_set_capture_rotation(int rotation);
extern int libwtk_create_call(void);
extern int libwtk_get_audio_stats(int* send_bps, int* rec_bps, int* package_lost);
extern int libwtk_get_video_stats(int* send_bps, int* rec_bps, int* prefer_bps);
extern int libwtk_get_call_quality(int* audio_level, int* video_level);
extern int libwtk_create_audio_send_stream(uint32_t local_audio_ssrc);
extern int libwtk_create_audio_receive_stream(uint32_t remote_audio_ssrc);
extern int libwtk_create_video_send_stream(uint32_t local_video_ssrc);
extern int libwtk_create_video_receive_stream(uint32_t remote_video_ssrc);
extern void libwtk_destroy_audio_send_stream(void);
extern void libwtk_destroy_audio_receive_stream(void);
extern void libwtk_destroy_video_send_stream(void);
extern void libwtk_destroy_video_receive_stream(void);
extern void libwtk_destroy_call(void);
extern void libwtk_set_mute(bool muted);
extern void libwtk_start_audio_stream(void);
extern void libwtk_start_video_stream(void);
extern void libwtk_stop_audio_stream(void);
extern void libwtk_stop_video_stream(void);

extern int libwtk_create_conf_render(void* surfaceView0,void* surfaceView1,void* surfaceView2,void* surfaceView3);
extern int libwtk_create_video_conf_send_stream(uint32_t local_video_ssrc);
extern int libwtk_create_video_conf_receive_stream(uint32_t remote_video_ssrc);
extern void libwtk_start_video_conf_stream(void);
extern void libwtk_stop_video_conf_stream(void);
extern void libwtk_destroy_video_conf_stream(void);

#ifdef __cplusplus
}
#endif

#endif
