#ifndef _PTT_H_
#define _PTT_H_

#include "pttclient.h"
#include "tbcp.h"
#include "util.h"
#include "iaxclient_lib.h"

#if defined(WIN32)  ||  defined(_WIN32_WCE)
#include <winsock.h>
#else 
#include <pthread.h>
#include <sys/socket.h>
#include <fcntl.h>
#endif 

#ifdef MACOSX
#include "iaxclient_lib.h"
#define STRCPY(dst, src, size ) strncpy(dst,src,size)
#endif

#define PTT_DISCONNECT  0
#define PTT_CONNECTED 1
#define PTT_EVENT_NET_ERROR -1

typedef struct _tbcp_event{
	char data[512];
	int len;
	char type; 
	struct _tbcp_event* next;
	struct _tbcp_event* tail;
}
tbcp_event;

typedef struct _PTT {
	//base info 
	int pttID;
	int connection; 
	int pttToken;
	int channel; 
	char sessionid[256];
	char freeppid[256];
	
	//status info 
	unsigned char connectStatue; 
	unsigned char TB_Status; 
	
	//send TBCP queue . 
	tbcp_event * tbcp_send_queue;
	MUTEX ptt_lock;
	// recv TBCP queue . 
	//rtp_event * tbcp_recv_queue; 
	//MUTEX tbcp_recv_queue_lock;

	
	//send audio data queue
	// rtp_event* audio_queue;
	  //MUTEX audio_queue_lock;
	// server info 
	struct sockaddr_in  sin ; // ptt server address. 
	int socket; 
	
	// log func 
	ptt_output_callback_t logOutput; 
	//ptt thread. 
	ptt_event_callback_t post_ptt_event ; 		
    THREAD   		ptt_proc_thread ;
    THREADID		ptt_thread_id ; 
	 
#if defined(WIN32)  ||  defined(_WIN32_WCE)
	HANDLE               ptt_proc_event; 
#endif 
    int  ptt_proc_flag; 	
		
}PTT; 


PTT * PTT_create();
	
void PTT_set_event_observer(PTT * ptt,ptt_event_callback_t func);
int  PTT_set_server_addr(PTT * ptt,struct sockaddr_in * server_addr);

int  PTT_connect_to_server(PTT * ptt, struct  sockaddr_in * server_addr);
int  PTT_reset(PTT * ptt);

int  PTT_send_audio(PTT * ptt , unsigned char * audioData , int length );
int  PTT_release(PTT * ptt);

int  PTT_disconnected(PTT * ptt);

int  PTT_request(PTT * ptt);

#endif
