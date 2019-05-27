#include "pttclient.h"
#include "ptt.h"

#include <stdio.h>
#ifdef WIN32
#include <stdlib.h>
#endif

static  PTT * ptt = NULL; 

void ptt_set_observer( ptt_event_callback_t func)
{
	PTT_set_event_observer( ptt, func); 
}

int ptt_initialize(ptt_output_callback_t  logOutput)
{

	ptt = PTT_create(); 

	if (!ptt)
		return -1; 

	ptt->logOutput = logOutput; 
	
    return 0;
}

void ptt_shutdown(void)
{
	PTT_release(ptt); 
}

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

int  ptt_disconnect_server(int connection)
{
	PTT_disconnected(  ptt); 
    return 0;
}
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
int  ptt_send_audio(int connection, unsigned char*data, int length)
{
	return PTT_send_audio(ptt, data,length);
}
