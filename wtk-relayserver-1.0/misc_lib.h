#ifndef _misc_lib_h_
#define _misc_lib_h_

#include "define.h"

#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <syslog.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/epoll.h>

struct RT_Info {
	char relaytoken[64];
	
	struct sockaddr_in l_ipaddr;
	struct sockaddr_in r_ipaddr;
	unsigned short l_callno;
	unsigned short r_callno;
	char l_username[USERNAME_SIZE];
	char r_username[USERNAME_SIZE];
	unsigned char l_txsequence;
	unsigned char r_txsequence;
	unsigned char l_txstatus;
	unsigned char r_txstatus;
	unsigned char l_txmaxcount;
	unsigned char r_txmaxcount;

	unsigned char status;
	unsigned char txstatus;

	int pkt_len;		
	uint8_t pktbuf[RELAY_PKTBUF_SIZE];

	struct RT_Info* next;
};
struct RT_Info_List{
	void *RT_Info;
	struct RT_Info_List* next;			
};

/* Full frames are always delivered reliably */
struct ast_iax2_full_hdr {
	unsigned short scallno;	/* Source call number -- high bit must be 1 */
	unsigned short dcallno;	/* Destination call number -- high bit is 1 if retransmission */
	unsigned int ts;		/* 32-bit timestamp in milliseconds (from 1st transmission) */
	unsigned char oseqno;	/* Packet number (outgoing) */
	unsigned char iseqno;	/* Packet number (next incoming expected) */
	unsigned char type;		/* Frame type */
	unsigned char csub;		/* Compressed subclass */
	unsigned char iedata[0];
} __attribute__ ((__packed__));


/* Mini header is used only for voice frames -- delivered unreliably */
struct ast_iax2_mini_hdr {
	unsigned short callno;	/* Source call number -- high bit must be 0, rest must be non-zero */
	unsigned short ts;		/* 16-bit Timestamp (high 16 bits from last ast_iax2_full_hdr) */
							/* Frametype implicitly VOICE_FRAME */
							/* subclass implicit from last ast_iax2_full_hdr */
	unsigned char data[0];
} __attribute__ ((__packed__));

struct ast_iax2_video_hdr {
	unsigned short zeros;			/* Zeros field -- must be zero */
	unsigned short callno;			/* Video call number */
	unsigned short ts;				/* Timestamp and mark if present */
	unsigned char data[0];
} __attribute__ ((__packed__));

struct iax_ies {
	char relaytoken[64];
	char username[USERNAME_SIZE];
	unsigned char txreason;
	unsigned char txsequence;	
};

struct mgmt_type {
	unsigned char type;	
	unsigned char csub;
	unsigned char value[64];
};

/*static inline int inaddrcmp(const struct sockaddr_in *sin1, const struct sockaddr_in *sin2)
{
	return ((sin1->sin_addr.s_addr != sin2->sin_addr.s_addr) 
		|| (sin1->sin_port != sin2->sin_port));
}*/

//Lib API
extern int traceLevel;
extern int useSyslog;
extern int inaddrcmp(const struct sockaddr_in *sin1, const struct sockaddr_in *sin2);
extern void TraceEvent(int level, char* file, int line, char* format, ...);
extern int setup_socket(int port, char* ipaddr, int bind_any);
extern int iax_parse_ies(struct iax_ies *ies, unsigned char *data, int datalen);
extern int modify_txreason_ie(unsigned char *data, int datalen);
extern int uncompress_subclass(unsigned char csub);
extern struct RT_Info* find_routeinfo_by_relaytoken(struct RT_Info *list, const char *token);
extern struct RT_Info* find_routeinfo_by_addr_and_callno(struct sockaddr_in *addr, unsigned short scallno, int *flag);
extern void add_route_to_RtInfoListArray(unsigned short scallno, struct RT_Info *rti);
extern void del_route_from_RtInfoListArray(struct sockaddr_in *addr, unsigned short scallno);
extern void list_all_detail_route( struct RT_Info *list,  char *resbuf);
extern void list_detail_route( struct RT_Info *list,  char *relaytoken, char *resbuf);
#endif
