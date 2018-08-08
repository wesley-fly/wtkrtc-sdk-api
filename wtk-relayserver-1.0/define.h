#ifndef _define_h_
#define _define_h_
//Trace def
#define TRACE_DATESIZE 	32
#define TRACE_ERROR     0, __FILE__, __LINE__
#define TRACE_WARNING   1, __FILE__, __LINE__
#define TRACE_NORMAL    2, __FILE__, __LINE__
#define TRACE_INFO      3, __FILE__, __LINE__
#define TRACE_DEBUG     4, __FILE__, __LINE__

//IAX def
#define IAX_FLAG_SC_LOG				0x80
#define IAX_MAX_SHIFT				0x3F
#define IAX_FLAG_FULL				0x8000

#define AST_FRAME_VIDEO				0x03
#define AST_FRAME_IAX				0x06

#define IAX_COMMAND_HANGUP			5
#define IAX_COMMAND_TXREQ  			22
#define IAX_COMMAND_TXCNT			23
#define IAX_COMMAND_TXACC			24
#define IAX_COMMAND_TXREADY			25
#define IAX_COMMAND_TXREL			26
#define IAX_COMMAND_TXREJ			27

#define IAX_IE_USERNAME				6
#define IAX_IE_APPARENT_ADDR		18		/* Apparent address of peer - struct sockaddr_in */
#define IAX_IE_TXSEQUENCE			216		/* NAT change time squeue number*/
#define IAX_IE_TXREASON				217		/* 0:Normal, 1:Heart beat, 2:Nat change, 3:Nat change, */
#define IAX_IE_RELAY_TOKEN			222			/*relay token generet by asterisk*/

#define IAX_TXREASON_HEARTBEAT      1
#define IAX_TXREASON_NETCHANGE      2

//Relay common def
#define RELAY_PORT_DEFAULT			4579
#define MGMT_PORT_DEFAULT			4580
#define RELAY_PKTBUF_SIZE			2048
#define MAXEPOLLSIZE 				512
#define USERNAME_SIZE				80

//Route Table def.
#define ROUTETABLE_LIST_SIZE		65537  
#define ROUTETABLE_IDEL				0
#define ROUTETABLE_SETTING			1
#define ROUTETABLE_SETTED			2
#define ROUTETABLE_RELEASING		3
#define ROUTETABLE_RELEASED			4
#define LEFT_SIDE_FRAME				1
#define RIGHT_SIDE_FRAME			2

//Mgmt def
#define MGMT_ROUTELIST			1
#define MGMT_ROUTELIST_ALL		1
#define MGMT_ROUTELIST_CUR		2

#define MGMT_CONFIG				2
#define MGMT_CONFIG_TRACELEVEL	1




#endif
