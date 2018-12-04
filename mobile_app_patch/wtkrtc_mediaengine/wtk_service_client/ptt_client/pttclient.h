/*
 * Push-to-Talk Client Module
 * 
 *  Feature:
 *  1) reliable tansmission of audio packet
 *  2) TBCP processror and notification
 *
 *  Created by mark
 */

#ifndef _PTTCLIENT_H_
#define _PTTCLIENT_H_


/* Prototype of Observer Function
     connection - which connection a PTT event or RTP packet received
     event - type of event (-1=RTP, Other refer to _PTT_EVENT)
     content  - content of event
     length - content length
    Note: The calling process should return as quickly as possible
*/
typedef int(* ptt_event_callback_t)(int connection, int ptt_event, void* content, int length); 

typedef void (* ptt_output_callback_t)(int type, const char * out); 
 
#include "ptt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Set a callback function to observer the status of talk */
extern void ptt_set_observer( ptt_event_callback_t func);


/* Initialize PTT client module */
extern int  ptt_initialize(ptt_output_callback_t  output);
    
/* Shutdown PTT client module */
extern void ptt_shutdown(void);

/* Connect PTT media server */
extern int  ptt_connect_server(struct sockaddr_in* sin, char * sessionID, char * freeppID);
/* Disconnect PTT media server */
extern int  ptt_disconnect_server(int connection);

/* Request or release token asynchronously
    Note: the result should be returned by observer function
*/
extern int  ptt_request_token(int connection);
extern int  ptt_release_token(int connection);

/* Send Voice Stream to Server*/
extern int  ptt_send_audio(int connection, unsigned char*data, int length);

extern void ptt_set_output(void(* func)(const char *)); 

#ifdef __cplusplus
}
#endif


#endif 


