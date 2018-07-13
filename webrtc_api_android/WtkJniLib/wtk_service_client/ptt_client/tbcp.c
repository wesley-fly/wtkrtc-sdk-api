
#include "tbcp.h"

#ifdef ANDROID 
	#include <android/log.h>
	#include<errno.h>
#endif 

//static TBCP_received_callback  TBCPRecvr = NULL; 

int TBCP_recv_callback( int channel, unsigned char subType,
        unsigned int name, const unsigned char* data,
        unsigned short dataLengthInBytes){

	if (subType>15)
		return -1; 
	
}


void TBCP_init(){

	//libve_set_PTT_RTCP_callback(TBCP_recv_callback); 
}



static int TBCP_generate_ssrc (char * freeppId){
	int id ; 
	
	if (!freeppId)
		return -1; 
	id = atoi(freeppId);
	
	return ntohl(id ); 
}

static int  TBCP_create_send_data (char * tbcp_cmd , TBCP_Data * data){
	unsigned int ssrc = 0;
	unsigned int header = 0; 
	unsigned int length  = 0; 
	unsigned int  tmp = 0; 

	if (!tbcp_cmd || !data)
		return -1; 
	
	ssrc = TBCP_generate_ssrc(data->freeppid); 
	header  =  2 << 30; // v
	header = header | (0<<29);//P
	header = header | (data->type << 24);// subtype
	header = header | (RTCP_PT_APP <<16);
	header = header |((TBCP_HEAD_LEN+sizeof(TBCP_Data))&0xFFFF); //length.
	
	// all length. 
	length = TBCP_HEAD_LEN + sizeof(TBCP_Data); 
	*((unsigned short* )tbcp_cmd) = htons(length);
	//header
	*((unsigned int*)(tbcp_cmd+2))= htonl(header); 
	*((unsigned int*)(tbcp_cmd+6))  = htonl(ssrc);

	memcpy(tbcp_cmd+10, "poc1",4);
	memcpy(tbcp_cmd+14, data, sizeof(TBCP_Data)); 


	return 0; 
}



void TBCP_reset_data(TBCP_Data *  tbcpData){

	if (tbcpData)
		memset(tbcpData, 0 , sizeof(TBCP_Data)); 
}



void  TBCP_fill_data(TBCP_Data *  tbcpData, char * sessionID ,char * freeppID,TBCP_Type type){

		if (!tbcpData)
			return ; 
		STRCPY(tbcpData->sessionid,sessionID, 33);
		STRCPY(tbcpData->freeppid,freeppID, 31);
	
		tbcpData->type = type; 
			
		switch(tbcpData->type){
			case TB_Enter :
			case TB_Release:
			case TB_Exit:
			default :
			break;
		}
		
	return; 
}


int TBCP_sendCmd( SOCKET socket , TBCP_Data *  tbcpData ){

		char tbcp_cmd[512] ; 
		int ret = 0; 

		if (socket <0 || !tbcpData )
		return -1; 

		TBCP_create_send_data(tbcp_cmd, tbcpData); 
		ret = send (socket,tbcp_cmd,TBCP_HEAD_LEN+2+sizeof(TBCP_Data),0) ;
		if (ret <= 0){
			//int error = 0;
			
			//error = WSAGetLastError();

			return -1;
		}
		//TBCP_reset_data(tbcpData); 
		return ret;
}



int TBCP_getInfo(TBCP_Info *info ,char * data ,int length){
	unsigned int header;
	int version = 0, pt = 0, packetlen = 0, subtype = 0;

	if (!info )
		return -1 ; 

	header = ntohl(((unsigned int *)(data))[0]);

	version = (header & 0xa0000000)>>30;
    	subtype = (header & 0x1f000000)>>24;
    	pt = (header & 0xff0000)>>16;

		info->type = (TBCP_Type)subtype;
		info->length  = length-TBCP_HEAD_LEN; 

		if (info->length>0)
			memcpy(info->data , data+TBCP_HEAD_LEN,info->length);
    return 0;
}


int TBCP_is (char * data , int length){
	unsigned short header;
	int  pt =0; 

	
	if (!data)
		return -1; 
	
	header = ntohs(((unsigned short *)(data))[0]);
	pt = (header & 0x00FF);

	if (pt == 204)
		return 1; 
	else 
		return 0; 
}


