#ifndef _wtk_relay_h_
#define _wtk_relay_h_

#include "misc_lib.h"

struct mixservice_info
{
  int daemon;
  char mixer_ip[32];
  uint16_t mixer_port;
  int mixer_fd;
};
typedef struct mixservice_info ms_info_t;

struct mixservice_rep{
  char action[6+1];
  char session[32+1];
  struct sockaddr_in addr;
};

#endif
