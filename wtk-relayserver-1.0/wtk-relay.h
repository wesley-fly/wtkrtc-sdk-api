#ifndef _wtk_relay_h_
#define _wtk_relay_h_

#include "misc_lib.h"
#include <sys/epoll.h>

//
struct relayservice_info
{
	int daemon;
	char md5key[32];
	
	char relay_ip[32];
	int relay_port;
	int relay_fd;
	
	char mgmt_ip[32];
	int mgmt_port;
	int mgmt_fd;

	struct RT_Info *rti;
};
typedef struct relayservice_info rs_info_t;


#endif
