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
#define IAX_COMMAND_HEARTBEAT     	41

#define IAX_IE_USERNAME				6
#define IAX_IE_APPARENT_ADDR		18		/* Apparent address of peer - struct sockaddr_in */
#define IAX_IE_TXEVENT				216
#define IAX_IE_RELAY_TOKEN			222			/*relay token generet by asterisk*/

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
#define ROUTETABLE_NATTED			3
#define ROUTETABLE_P2PED			4
#define ROUTETABLE_RELEASING		5
#define ROUTETABLE_RELEASED			6

#define TX_STATUS_EVENT_INIT_NAT	0
#define TX_STATUS_EVENT_INIT_P2P	1
#define TX_STATUS_EVENT_RS			2
#define TX_STATUS_EVENT_NAT			3
#define TX_STATUS_EVENT_P2P			4
#define TX_STATUS_EVENT_NONE		5

#define LEFT_SIDE_FRAME				1
#define RIGHT_SIDE_FRAME			2

//Mgmt def
#define MGMT_ROUTELIST			1
#define MGMT_ROUTELIST_ALL		1
#define MGMT_ROUTELIST_CUR		2

#define MGMT_CONFIG				2
#define MGMT_CONFIG_TRACELEVEL	1

#endif
