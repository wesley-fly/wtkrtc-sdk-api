#ifndef _wtk_rtc_api_h
#define _wtk_rtc_api_h

#include <stdbool.h>

#define SUCESS_RET	0
#define FAILED_RET	1

typedef int (*audio_transport_callback_t)(const uint8_t* buf, int len);
typedef int (*video_transport_callback_t)(const uint8_t* buf, int len);
#ifdef __cplusplus
extern "C" {
#endif
extern void 	libwtk_set_audio_transport(audio_transport_callback_t func);
extern void 	libwtk_set_video_transport(video_transport_callback_t func);
extern int 		libwtk_decode_audio(uint8_t* buf, int buflen);
extern int 		libwtk_decode_video(uint8_t* buf, int buflen);
extern int 		libwtk_initialize(void);
extern void 	libwtk_init_local_render(void);
extern void 	libwtk_init_remote_render(void);
extern int 		libwtk_create_call(void);
extern void 	libwtk_destroy_call(void);
extern int 		libwtk_init_capture(int device_id);
extern void 	libwtk_set_audio_codec(void);
extern void 	libwtk_set_video_codec(void);
extern void 	libwtk_start_capture(void);
extern void 	libwtk_stop_capture(void);
extern void 	libwtk_switch_camera(int device_id);
extern int 		libwtk_create_audio_send_stream(int rtp_format);
extern void 	libwtk_destroy_audio_send_stream(void);
extern int 		libwtk_create_audio_receive_stream(int rtp_format);
extern void 	libwtk_destroy_audio_receive_stream(void);
extern int 		libwtk_create_video_send_stream(int rtp_format);
extern void 	libwtk_destroy_video_send_stream(void);
extern int 		libwtk_create_video_receive_stream(int rtp_format);
extern void 	libwtk_destroy_video_receive_stream(void);
extern void 	libwtk_audio_stream_mute(bool mute);
extern void 	libwtk_start_audio_stream(void);
extern void 	libwtk_start_video_stream(void);
extern void 	libwtk_stop_audio_stream(void);
extern void 	libwtk_stop_video_stream(void);

#ifdef __cplusplus
}
#endif

#endif
