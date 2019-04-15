/*
 * Push-to-Talk Client Module
 *
 *  Feature:
 *  1) reliable tansmission of audio packet
 *  2) TBCP processror
 *
 *  Created by mark
 */
#include "pttclient.h"
#include "ptt.h"

#include <stdio.h>
#ifdef WIN32
#include <stdlib.h>
#endif

/* function to send event to iaxclient */ 

static  PTT * ptt = NULL; 

/* Set a callback function to observer the status of talk
    Samples to use this callback
    
    1) Receive a RTP Audio packet from server
       post_ptt_event(connection, PTT_EVENT_AUDIO, rtp_packet, rtp_length);
 
    2) Receive a TBCP PTT event from server
       post_ptt_event(connection, PTT_EVENT_NOTIFYMIN+TB_GRANTED, NULL, 0);
       »ò
       post_ptt_event(connection, PTT_EVENT_NOTIFYMIN+TB_TAKEN, speaker_id, sizeof(speaker_id));
 */
void ptt_set_observer( ptt_event_callback_t func)
{
	PTT_set_event_observer( ptt, func); 
}

/* Initialize PTT client module
Input:
 logOutput - pointer of log function to output debug message
Return: 0=succ; -1=failed
 */
int ptt_initialize(ptt_output_callback_t  logOutput)
{

	ptt = PTT_create(); 

	if (!ptt)
		return -1; 

	ptt->logOutput = logOutput; 
	
    return 0;
}

/* shudown PTT client module */
void ptt_shutdown(void)
{
	PTT_release(ptt); 
}

/* Connect PTT media server 
   sin - address and port of PTT media server
   Return:
      identifier of connection to media server
   Note:
      Other module will use the return value for further operation
 */
int  ptt_connect_server(struct sockaddr_in* sin, char * sessionID, char * freeppID)
{
    if(!sin ||!sessionID || !freeppID){
         return -1;
    }
	  

	STRCPY(ptt->sessionid,sessionID,256); 
	
	STRCPY(ptt->freeppid,freeppID, 256);

    if (PTT_connect_to_server(ptt,sin)<0){
        return -1;
    }
	   
    return 1;
}

/* Disconnect PTT media server 
    connection - identifier of connection
 */
int  ptt_disconnect_server(int connection)
{
	PTT_disconnected(  ptt); 
    return 0;
}

/* Request or Release speaker token
    connection - identifier of connection
*/
int  ptt_request_token(int connection)
{
	PTT_request(ptt);
    return 0;
}

int  ptt_release_token(int connection)
{
	PTT_release(ptt);
    	return 0;
}

/* Send Voice Stream
   connection - identifier of connection
   data - RTP packet buffer
   length - length of data
*/
int  ptt_send_audio(int connection, unsigned char*data, int length)
{

	return PTT_send_audio(ptt, data,length);
}






