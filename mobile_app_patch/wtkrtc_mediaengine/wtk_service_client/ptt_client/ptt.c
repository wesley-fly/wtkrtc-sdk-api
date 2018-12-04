/*
    Internal Engine of Push-to-Talk Client
    Created by Swenson.Liu
    @copyright BROWAN
 */
#include "ptt.h"

/* Create a new Push-to-Talk Client Object 
   return: Pointer of PTT Object
 */
PTT* PTT_create()
{
	PTT * ptt = (PTT *)malloc (sizeof (PTT));
	 
	if (ptt)
		memset(ptt, 0 , sizeof (PTT)); 

    MUTEXINIT(&(ptt->ptt_lock));
	ptt->tbcp_send_queue=NULL; 
	ptt->TB_Status = TB_Disconnect; 
	ptt->connectStatue = PTT_DISCONNECT;
	ptt->ptt_proc_flag = -1; 
	ptt->ptt_proc_thread = NULL; 
	ptt->connection = 0; 
	
#if defined(WIN32)  ||  defined(_WIN32_WCE)
	ptt->ptt_proc_event = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif 
	return ptt; 
}

/*
	reset ptt obj . 
	ptt : a ptt obj; 
   
*/

int PTT_reset(PTT * ptt){
	tbcp_event * temp = NULL;

	if (ptt){
		if(ptt->tbcp_send_queue)
			RELEASE_QUEUE(ptt->tbcp_send_queue,ptt->ptt_lock,temp); 


		ptt->TB_Status = TB_Disconnect; 
		ptt->connectStatue = PTT_DISCONNECT;
		ptt->ptt_proc_flag = -1; 
		ptt->ptt_proc_thread = NULL; 
	}
	else 
		return -1; 

	return 0; 
}

/*
	Analysis of received data, called by ptt pthread.  
*/
static int PTT_recv_parse(PTT * ptt ){

	char pttData[1024]={0,}; 
	//char dataLen[6]; 
	unsigned short dataLen =0; 
	TBCP_Info tbcp_info ; 
	int length = 0; 
	unsigned short packetlen =0;  
	int recvLen = 0;
	int socket = ptt->socket;
	//char tbcp_status = -1;
	char * content = NULL; 
	//int error_code;

	if (socket <0 )
		return -1; 

	recvLen = recv(socket ,(char *)(&dataLen),2, 0 ); 
	
	 //error_code = WSAGetLastError();
	if (recvLen<0)
		return -1; 
	else if (recvLen !=2)
		return -1; 

	//packetlen = ntohs((*(unsigned short *)dataLen));

	packetlen = ntohs(dataLen);
	if (packetlen <= 0)
		return -1;
	
	recvLen = recv(socket, pttData, packetlen, 0);	
	
	if(recvLen != packetlen)
	return -1; 

	if (recvLen<12 || recvLen >1292)
	return -1; 

	if (TBCP_is(pttData,recvLen))
    {
			// is  a tbcp data . 
			if (TBCP_getInfo(&tbcp_info ,pttData,packetlen)<0)
				return -1; 

			switch(tbcp_info.type){
				
				case TB_Ack:
					if (tbcp_info.length>0){ 
						unsigned int tmpData; 
						unsigned int subtype ; 
						unsigned int reason ; 
						tmpData = ntohl(*((unsigned int *)(tbcp_info.data)));
						subtype = tmpData >>27; 
						reason    = (tmpData>>16) &0x07FF; 
						
						if (subtype == TB_Enter){
							ptt->TB_Status =TB_Enter; 
							ptt->connectStatue = PTT_CONNECTED; 
						}
					}
				break; 
				case TB_Granted:
					ptt->TB_Status = TB_Granted;
				break; 

				case TB_Deny:
					ptt->TB_Status = TB_Deny; 
				break;
				
				case TB_Idle:
					ptt->TB_Status = TB_Idle; 
				break;

				case TB_Release:
					ptt->TB_Status = TB_Release; 
				break;
				case TB_Taken1:
					ptt->TB_Status = TB_Taken1;
					if (tbcp_info.length>0){
						content = pttData+2; 
						length = 34; 
					}
				break; 
				case TB_Queued:
					ptt->TB_Status = TB_Queued;
					if (tbcp_info.length>0){
						content = pttData+2; 
						length = 34; 
					}
				break; 
				case TB_Revoke:
					ptt->TB_Status = TB_Revoke;
				break; 
				
			}
			ptt->post_ptt_event(ptt->connection, ptt->TB_Status, content,length); 
	}else
		// is a rtp data. 
		ptt->post_ptt_event(ptt->connection, AUDIO_TYPE, pttData, recvLen);
    return 0;
}


/*
	Send ptt data to ptt server , called by ptt pthread. 
*/


static int PTT_send_data(PTT * ptt ){

	TBCP_Data tbcpData;
	tbcp_event * event = NULL; 
	long ret =0;
	
	if (!ptt)
		return -1; 

	POP_QUEUE_HEAD(event,ptt->tbcp_send_queue,ptt->ptt_lock); 

	if (!event)
	return -1; 

	memset(&tbcpData, 0, sizeof(TBCP_Data));

	if (event->type != AUDIO_TYPE ){

		switch(event->type){
			case TB_Request:
				TBCP_fill_data(&tbcpData, ptt->sessionid, ptt->freeppid,TB_Request); 
			break; 

			case TB_Release:
				TBCP_fill_data(&tbcpData, ptt->sessionid, ptt->freeppid,TB_Release); 
			break; 

			case TB_Exit:
				TBCP_fill_data(&tbcpData, ptt->sessionid, ptt->freeppid, TB_Exit);
				ptt->ptt_proc_flag = -1; 
			break; 
		
		}
	
		ret = TBCP_sendCmd(ptt->socket,&tbcpData); 
		
		if (ret < 0){
			free(event);
			return -1;
		}
		
		ptt->TB_Status = event->type; 		
		ptt->post_ptt_event(ptt->connection, ptt->TB_Status, NULL,0);
	}
	else
    {
		ret = send(ptt->socket, event->data,event->len, 0);
		if (ret < 0){
			free(event);
			return -1; 
		}
	}
	free(event);
	return (int)ret;
}


/*
static unsigned long WINAPI proc_func(void * context){
	TBCP_Data tbcpData;
	PTT * ptt = NULL;
	ptt = (PTT *)context;
	if (!ptt)
		return -1;

	//SetEvent(ptt->ptt_proc_event);

	while (!ptt->ptt_proc_flag){
		unsigned long waitResult = WaitForSingleObject(ptt->ptt_proc_event, 500);
		
		if (waitResult == WAIT_OBJECT_0)
			ptt->ptt_proc_flag = 1; 
		
			//PTT_recv_parse(ptt);
			//PTT_send_data(ptt);
			Sleep(10); 
	}

	ptt->logOutput(3,"<----- ptt thread is stoped----->"); 
}
*/



/*
   pthread func. 
   
*/

static THREADFUNCDECL(ptt_proc_thread_func, args){
	PTT * ptt = NULL; 
	TBCP_Data tbcpData;

	THREADFUNCRET(ret);
	
	ptt = (PTT *)args;  
	if (!ptt )
	  	return -1; 

	// send TBCP CMD 'ENTER' to media server. 
	TBCP_reset_data(&tbcpData);
	TBCP_fill_data(&tbcpData, ptt->sessionid, ptt->freeppid, TB_Enter);
	if(TBCP_sendCmd(ptt->socket, &tbcpData)==-1) {
        return -1;
    }

	#if defined(WIN32) || defined(_WIN32_CE)
		ResetEvent(ptt->ptt_proc_event);
	#endif

	iaxci_prioboostbegin();
	
	while (!ptt->ptt_proc_flag)
    {
		PTT_recv_parse(ptt);
		PTT_send_data(ptt); 		
#if defined(WIN32) || defined(_WIN32_CE)
        Sleep(10);
#else
        iaxc_millisleep(10);
        //usleep(10000);
#endif
	}
	
	iaxci_prioboostend();
	return ret; 
}


/*
	Start ptt pthread.
*/

static int PTT_start_proc_thread(PTT * ptt){

	void * _thread_handle = NULL;
	int _id = 0;
	unsigned long ret = -1; 

	ptt->ptt_proc_flag = 0;
	
	/*
     void * _thread_handle = NULL;
     int _id = 0;
     unsigned long ret = -1;

    _thread_handle = CreateThread(NULL, 0, proc_func,ptt,NULL,_id);

	if (_thread_handle == NULL){
	
		ptt->logOutput(3 ,"Error: create thread failed ."); 
		return -1; 
	}

	SetThreadPriority(_thread_handle, THREAD_PRIORITY_TIME_CRITICAL);

	ret = WaitForSingleObject(ptt->ptt_proc_event,1000); 

	if (ret != WAIT_OBJECT_0)
		ptt->logOutput(3,"error: ptt thread did not start up.");
    */
	
	if ( THREADCREATE(ptt_proc_thread_func, ptt, ptt->ptt_proc_thread,
			ptt ->ptt_thread_id ) == THREADCREATE_ERROR)
			return -1;

	#if defined(WIN32) || defined(_WIN32_CE)	
		ptt->ptt_proc_event = CreateEvent(NULL, TRUE, TRUE, NULL);	
	#endif
	
	return 0; 
}


int PTT_stop_proc_thread(PTT  * ptt){

	if (ptt->ptt_proc_flag >=0){

		ptt->ptt_proc_flag = -1; 
		
		#if defined(WIN32) || defined(WIN32_CE)
			THREADJOIN(ptt->ptt_proc_event);
			CloseHandle(ptt->ptt_proc_event);
		#else
			THREADJOIN(ptt->ptt_proc_thread);
		#endif
	}
	return 0;
}

/*
   Set the ptt add to ptt obj. 
*/


int  PTT_set_server_addr(PTT * ptt, struct sockaddr_in * server_addr){

	if (!ptt || ! server_addr)
		return -1; 
	
	memcpy(& ptt->sin, server_addr, sizeof(struct sockaddr_in)); 
	return 0; 	
}
/*
    To connect to the PTT sever. 
    ptt : ptt obj . 
    server_addr : ptt server. 
    
*/

int PTT_connect_to_server(PTT * ptt, struct sockaddr_in *server_addr){
	
	int fd = -1; 
	int ret = -1; 
	struct sockaddr_in sin; 
	int iMode = 1;
	

	// start  PTT pthread . 

	// if (PTT_start_proc_thread(ptt)<0)
	// return -1; 

	// try to connect to PTT server. 
	
	if (!ptt || ! server_addr)
	return -1; 

	//ptt->sin = server_addr; 
	//memcpy(& ptt->sin, server_addr, sizeof(struct sockaddr_in));

	sin.sin_family = AF_INET;
	sin.sin_port = server_addr->sin_port; 
	
	#if defined(WIN32)	||	defined(_WIN32_WCE)
		sin.sin_addr.S_un.S_addr = server_addr->sin_addr.s_addr;
	#else 
		sin.sin_addr.s_addr = server_addr->sin_addr.s_addr;
	#endif
	
	if (ptt->connectStatue == PTT_CONNECTED)
	 return 0; 

	fd = socket(AF_INET,SOCK_STREAM,0);
	
	if (fd <0 ){
		ptt->post_ptt_event(ptt->connection , PTT_EVENT_NET_ERROR, NULL,0); 			
		return -1; 
	}

	if (connect(fd, (struct sockaddr*)&sin, sizeof(struct sockaddr)) != 0) {
		//int error_num = 0;
		ptt->post_ptt_event(ptt->connection , PTT_EVENT_NET_ERROR, NULL,0);		
		//error_num = WSAGetLastError();
        PTT_CLOSE_SOCKET(fd);
        return -1;
	}
	ptt->socket = fd; 
	ptt->TB_Status = TB_Connect; 
	ptt->connectStatue = PTT_DISCONNECT;
	ptt->connection++;
	// start ptt pthread . 
	
#if defined(WIN32)  ||  defined(_WIN32_WCE)
    ioctlsocket(fd, FIONBIO, (u_long FAR*) &iMode);
#else
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif

    ret = PTT_start_proc_thread(ptt);
	if (ret < 0)
    {
        PTT_CLOSE_SOCKET(ptt->socket);
        return -1;
    }
	
	return 0; 
}



/*
   Disconnect from the PTT server. 

*/

int PTT_disconnected(PTT * ptt)
{
	TBCP_Data tbcpData;
	int ret = 0;
	
	if (!ptt || ptt->socket <0 )
		return -1;
    

	ptt->ptt_proc_flag = -1; 

	#if defined(WIN32) || defined(_WIN32_CE)
		Sleep(10);
	#else
		iaxc_millisleep(10);
	//usleep(10000);
	#endif


	// stop ptt pthread. 
	PTT_stop_proc_thread(ptt); 	
	TBCP_fill_data(&tbcpData, ptt->sessionid, ptt->freeppid,TB_Exit); 	
	ret = TBCP_sendCmd(ptt->socket,&tbcpData); 
	PTT_CLOSE_SOCKET(ptt->socket); 
	PTT_reset(ptt); 
	//iaxci_usermsg(1, "PTT: PTT_request message TB_Request ");
	//PUSH_QUEUE_TAIL(event, ptt->tbcp_send_queue,ptt->ptt_lock);
	ptt->connection --; 
	return 0; 
}

/*
    Sets the callback function of log information. 
*/

void PTT_set_event_observer(PTT * ptt,ptt_event_callback_t func){
	ptt->post_ptt_event = func; 	
}

/*
    Request to speak . 
*/

int PTT_request(PTT * ptt){

	tbcp_event * event; 

	if (!ptt)
		return -1; 
	event = (tbcp_event *)malloc(sizeof(tbcp_event));
	if (!event)
		return -1; 
	memset(event, 0, sizeof(tbcp_event));
	event->type = TB_Request;
    
	iaxci_usermsg(1, "PTT: PTT_request message TB_Request ");
	PUSH_QUEUE_TAIL(event, ptt->tbcp_send_queue,ptt->ptt_lock);
    return 0;

}

/*
	Send audio data to PTT server. 
	audioData: RTP voice data packets. 
*/

int PTT_send_audio(PTT * ptt , unsigned char * audioData , int length ){
	tbcp_event * event = NULL;
	
	if (!ptt || !audioData || length <=0)
		return -1; 	

	event = (tbcp_event *)malloc(sizeof(tbcp_event));
	if (!event)
		return -1;
	memset(event, 0, sizeof(tbcp_event));
	event->type = AUDIO_TYPE;
	event->len = length+2; 
	*((unsigned short*)event->data) = htons(length); //audio data length. 
	memcpy(event->data + 2, audioData, length);
	PUSH_QUEUE_TAIL(event, ptt->tbcp_send_queue, ptt->ptt_lock);

	return 0; 
	
}

/* Release a Push-to-Talk Object 
Input : Push-to-Talk Object
 */
int PTT_release(PTT * ptt)
{
	tbcp_event * event = NULL;
    iaxci_usermsg(1, "PTT: PTT_release send TB_Release ");
	if(!ptt)
		return -1; 
	
    event =(tbcp_event *) malloc (sizeof(tbcp_event));
	if (!event)
        return -1;
	
    memset(event, 0, sizeof(tbcp_event));

    event->type = TB_Release;
	
     PUSH_QUEUE_TAIL(event, ptt->tbcp_send_queue,ptt->ptt_lock);
    return 0;
}


