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
#include <net/if.h>
#include <net/if_arp.h>

static inline int inaddrcmp(const struct sockaddr_in *sin1, const struct sockaddr_in *sin2)
{
  return ((sin1->sin_addr.s_addr != sin2->sin_addr.s_addr) || (sin1->sin_port != sin2->sin_port));
}

struct channel_info {
  int sock;
  int channel_num;
  time_t updateTime;
  struct sockaddr_in addr;
  struct channel_info *  next;
};

extern int traceLevel;
extern int useSyslog;
extern void TraceEvent(int level, char* file, int line, char* format, ...);
extern int setup_ms_socket(int local_port, char *ip, int bind_any);
extern struct channel_info * find_channel_info_by_sockaddr( struct channel_info *list, struct sockaddr_in *addr);
extern struct channel_info * find_sockaddr_by_channelno( struct channel_info *list, const int channelno);
#endif
