#ifndef _TBCP_H_
#define _TBCP_H_

#include "util.h"
#include <stdlib.h>
#include <string.h>

#define RTCP_PT_APP 204
#define SOCKET int

#define PRE_PACKET_LEN 2
#define TBCP_HEAD_LEN 12


typedef enum _TBCP_Type
{
	TB_Request=0x00,	
	TB_Granted, 
	TB_Taken1, 
	TB_Deny,  
	TB_Release, 
	TB_Idle,  
	TB_Revoke, 
	TB_Ack, 
	TB_Position, 
	TB_Queued, 
	TB_Disconnect=0x0b , 
	TB_Connect=0x0f, 
	TB_Taken2=0x12,
	TB_Enter=0x18,
	TB_Exit
}TBCP_Type;

#define  AUDIO_TYPE 0X1A

typedef enum {
   RTCP_SDES_END   = 0,
   RTCP_SDES_CNAME = 1,
   RTCP_SDES_NAME  = 2,
   RTCP_SDES_EMAIL = 3,
   RTCP_SDES_PHONE = 4,
   RTCP_SDES_LOC   = 5,
   RTCP_SDES_TOOL  = 6,
   RTCP_SDES_NOTE  = 7,
   RTCP_SDES_PRIV  = 8
} rtcp_sdes_type_t;

typedef  struct _TBCP_Data
{
	char sessionid[33];
	char freeppid[31];
	char * data; 
	int   dataLen; 
	TBCP_Type type;	
}TBCP_Data;


typedef struct _TBCP_Info {
	TBCP_Type type;	
	char data[1024]; 
	int length; 
}TBCP_Info; 

typedef int (* TBCP_received_callback)(  TBCP_Data *  data); 

int TBCP_sendCmd(SOCKET socket , TBCP_Data *  data);
void  TBCP_reset_data(TBCP_Data *  data); 
void  TBCP_fill_data(TBCP_Data *  tbcpData, char * sessionID ,char * freeppID,TBCP_Type type); 
int TBCP_getInfo(TBCP_Info *info ,char * data ,int length); 
int TBCP_is (char * data , int length); 


#endif 
