#include "wtk-mixer.h"
#include "../wtkrtc_mixer_api/wtk_rtc_mixer_api.h"

struct channel_info  *p_chi = NULL;
static const struct option long_options[] = {
	{ "foreground",      no_argument,       NULL, 'f' },
	{ "mixer_port",      required_argument, NULL, 'l' },
	{ "mixer_ip",        required_argument, NULL, 'a' },
	{ "help"   ,         no_argument,       NULL, 'h' },
	{ "verbose",         no_argument,       NULL, 'v' },
	{ NULL,              0,                 NULL,  0  }
};
static void exit_help(int argc, char * const argv[])
{
	fprintf( stderr, "%s usage\n", argv[0] );
	fprintf( stderr, "-l <mixer_port>\tSet UDP main listen port to <lport>\n" );
	fprintf( stderr, "-a <mixer_ip>\tSet UDP main listen ip to <lip>\n" );  
	fprintf( stderr, "-f        \tRun in foreground.\n" );
	fprintf( stderr, "-v        \tIncrease verbosity. Can be used multiple times.\n" );
	fprintf( stderr, "-h        \tThis help message.\n" );
	fprintf( stderr, "\n" );
	exit(1);
}

static int init_ms_info( ms_info_t * ms_info )
{
	memset( ms_info, 0, sizeof(ms_info_t) );

	ms_info->daemon = 1; /* By defult run as a daemon. */
	ms_info->mixer_port = MS_PORT_DEFAULT;
	ms_info->mixer_fd = -1;
	memset(ms_info->mixer_ip, 0x00, sizeof(ms_info->mixer_ip));
	return 0; 
}
static void deinit_ms_info( ms_info_t * ms_info )
{
	if (ms_info->mixer_fd >= 0)
	{
		close(ms_info->mixer_fd);
	}
	ms_info->mixer_fd = -1;
	return;
}
static int send_to_audio_channel(char* buf, int len, int channel)
{
	struct channel_info  *p_ch = NULL;

	if ((channel >= 0 ) && (channel < MAX_PARTICIPANT )) 
	{
		p_ch = find_sockaddr_by_channelno(p_chi, channel);
		if (p_ch!=NULL)
		{
			//TraceEvent( TRACE_DEBUG, "Child(Pid=%u): channel=%d, len=%d", getpid(), channel, len );
			sendto(p_ch->sock, buf, len, 0, (struct sockaddr *)&(p_ch->addr), sizeof(struct sockaddr_in));
		}
	}
	
	return 0;	
}
static int send_to_video_channel(char* buf, int len, int channel)
{
	struct channel_info  *p_ch = NULL;
	
	if ((channel >= 0 ) && (channel < MAX_PARTICIPANT )) 
	{
		p_ch = find_sockaddr_by_channelno(p_chi, channel);
		if (p_ch!=NULL)
		{
			//TraceEvent( TRACE_DEBUG, "Child(Pid=%u):Video channel=%d, len=%d", getpid(), channel, len );
			sendto(p_ch->sock, buf, len, 0, (struct sockaddr *)&(p_ch->addr), sizeof(struct sockaddr_in));
		}
	}

	return 0;	
}
static int send_to_all_video_channel(char* buf, int len, int own_channel)
{
	struct channel_info  *p_ch = p_chi;
	while(p_ch != NULL)
	{
		if(p_ch->channel_num != own_channel)
		{
			sendto(p_ch->sock, buf, len, 0, (struct sockaddr *)&(p_ch->addr), sizeof(struct sockaddr_in));
		}
		p_ch = p_ch->next;
	}

	return 0;	
}

static int start_meetme(int sockfd, struct sockaddr_in *sender_sock, char *meetmekey)
{
	uint8_t pktbuf[MS_PKTBUF_SIZE];
	char conf_session[36];
	char conf_number[36];
	int sock = -1;
	int keep_running = 1;
	int rc;
	ssize_t bread;
	struct mixservice_rep ms_rep;
	socklen_t slen = sizeof(struct sockaddr_in);
	struct sockaddr_in local_sender_addr;
	int max_sock;
	fd_set socket_mask;
	struct timeval wait_time;
	int i_recv_timeout = 0;
	socklen_t i;
	struct channel_info  *p_ch = NULL;
	
	memcpy(&local_sender_addr, sender_sock, sizeof(struct sockaddr_in));
	memset(conf_session, 0x00, sizeof(conf_session));
	memset(conf_number, 0x00, sizeof(conf_number));
	strncpy(conf_session, meetmekey, 32);
	snprintf(conf_number, 32, "%s", &meetmekey[33]);
	TraceEvent( TRACE_DEBUG,"Child(Pid=%u): parse meetmekey::conf_session=%s,conf_number=%s",getpid(),conf_session, conf_number);
	TraceEvent( TRACE_DEBUG,"Child(Pid=%u): Mix Server start init lib WtkRTC Engine!");
	libwtk_set_mixer_audio_transport(send_to_audio_channel);
	//libwtk_set_mixer_video_transport(send_to_video_channel);

	libwtk_mixer_init();
	
	TraceEvent( TRACE_DEBUG,"Child(Pid=%u): Mix Server start init lib WtkRTC Engine end!");

	TraceEvent( TRACE_INFO,"Child(Pid=%u): conf_session=%s, Begin Createing New Meetme",getpid(), conf_session);
	sock = setup_ms_socket(0, NULL, 1 );/*bind ANY*/
	if(-1 == sock)
	{
		TraceEvent( TRACE_ERROR, "Child(Pid=%u): Failed to open main socket. %s, Meetme started Fail!",getpid(), strerror(errno));
		return -1;
	}

	memset(&ms_rep, 0x00, sizeof(struct mixservice_rep));
	strcpy(ms_rep.action, MS_CMD_NCF);
	strcpy(ms_rep.session, conf_session);
	getsockname(sock, (struct sockaddr *)&ms_rep.addr, &slen);
	TraceEvent(TRACE_DEBUG, "Child(Pid=%u):Meetme started Success at ip[%s], port[%d]",getpid(), inet_ntoa(ms_rep.addr.sin_addr), ntohs(ms_rep.addr.sin_port));
	
	sendto(sockfd, &ms_rep, sizeof(struct mixservice_rep), 0, (struct sockaddr *)&local_sender_addr, sizeof(struct sockaddr_in));
	sendto(sockfd, &ms_rep, sizeof(struct mixservice_rep), 0, (struct sockaddr *)&local_sender_addr, sizeof(struct sockaddr_in));
	usleep(200*1000);
	close(sockfd);
	i_recv_timeout = time(NULL);
	while(keep_running)
	{
		FD_ZERO(&socket_mask);
		max_sock = sock;
		FD_SET(sock, &socket_mask);

		wait_time.tv_sec = 10;
		wait_time.tv_usec = 0;

		rc = select(max_sock+1, &socket_mask, NULL, NULL, &wait_time);
		if(rc > 0)
		{
			if (FD_ISSET(sock, &socket_mask))
			{
				i = sizeof(sender_sock);
				bread = recvfrom(sock, pktbuf, MS_PKTBUF_SIZE, 0/*flags*/, (struct sockaddr *)&local_sender_addr, (socklen_t*)&i);
				if(bread < MS_CMD_LEN)
				{
					TraceEvent( TRACE_ERROR, "Child(Pid=%u): recvfrom() failed %d errno %d (%s)", getpid(), bread, errno, strerror(errno));
				}
				else
				{
					if(strncmp((const char *)pktbuf, MS_CMD_HCF, MS_CMD_LEN)==0)
					{
						keep_running = 0;
						TraceEvent(TRACE_NORMAL, "Child(Pid=%u): Meetme stop Success", getpid());
						break;
					}
					switch(pktbuf[1]&0x7f)
					{
						case kWtkPayloadTypeOpus:
						{
							//audio rtcp pt = 200(sender report Source description, 40 byte) 
							//201(reciver report, 32 byte), seems no effect?
							static int channel = 0;
							p_ch = find_channel_info_by_sockaddr(p_chi, &local_sender_addr);
							if(p_ch == NULL)
							{
								p_ch = (struct channel_info*)calloc(1, sizeof(struct channel_info));
								memcpy(&(p_ch->addr), &local_sender_addr, sizeof(struct sockaddr_in));
								p_ch->sock = sock;
								p_ch->channel_num = channel;
								p_ch->next = p_chi;
								p_chi = p_ch;
	
								TraceEvent( TRACE_INFO, "Child(Pid=%u): New Participant insert, bread=[%d], channel = %d", getpid(), bread,channel);
								libwtk_mixer_setup_mixer(channel);
								libwtk_mixer_decode_audio(pktbuf, bread, channel);
								channel++;
							}
							else
							{
								libwtk_mixer_decode_audio(pktbuf, bread, p_ch->channel_num);
							}
							
							p_ch->updateTime = time(NULL);
							i_recv_timeout = p_ch->updateTime;
						}
						break;
						/*
						case kWtkPayloadTypeVP8:
						case kWtkPayloadTypeVP9:
						case kWtkPayloadTypeH264:
						{
							p_ch = find_channel_info_by_sockaddr(p_chi, &local_sender_addr);
							if(p_ch == NULL)
							{
								TraceEvent( TRACE_INFO, "Child(Pid=%u): This video frame has no a exsit audio channel, so unknown where is to be forward!!!", getpid(), bread);
							}
							else
							{
								send_to_all_video_channel((char*)pktbuf, bread, p_ch->channel_num);
							}
						}
						break;
						default:
							TraceEvent( TRACE_INFO, "Child(Pid=%u): Does not support this data PT(%d), and the data len=[%d]", getpid(),pktbuf[1]&0x7f,bread);
						break;
						*/
						default:
						{
							p_ch = find_channel_info_by_sockaddr(p_chi, &local_sender_addr);
							if(p_ch == NULL)
							{
								TraceEvent( TRACE_INFO, "Child(Pid=%u): This video frame has no a exsit audio channel, so unknown where is to be forward!!!", getpid(), bread);
							}
							else
							{
								send_to_all_video_channel((char*)pktbuf, bread, p_ch->channel_num);
							}
						}
						break;
					}
				}
			}
		}
		else if(rc == 0)
		{
			TraceEvent( TRACE_INFO, "Child(Pid=%u): select timeout!!!", getpid());
		}
		else if(rc < 0)
		{
			TraceEvent( TRACE_ERROR, "Child(Pid=%u): select error!!!", getpid());
		}

		if((time(NULL)-i_recv_timeout) > MS_RECV_TIMEOUT)
		{
			keep_running = 0;
			TraceEvent(TRACE_NORMAL, "Child(Pid=%u): MeetMe Recv Timeout,May be network interruption.Meetme stop Success", getpid());
			break;
		}
	}
	TraceEvent( TRACE_DEBUG,"Child(Pid=%u): Mix Server start de-init lib WtkRTC Engine!");
	libwtk_mixer_deinit();
	TraceEvent( TRACE_DEBUG,"Child(Pid=%u): Mix Server start de-init lib WtkRTC Engine end!");

	return 0;
}
static int process_udp(int socket, struct sockaddr_in *sender_sock, uint8_t *udp_buf, size_t udp_size)
{
	char tmpkey[64];
	pid_t pid;
	struct sockaddr_in sockaddr;

	memset(tmpkey, 0x00, sizeof(tmpkey));
	memcpy(&sockaddr, sender_sock, sizeof(struct sockaddr_in));

	if (strlen((const char *)udp_buf) < 32+7)
	{
		TraceEvent( TRACE_WARNING,"Father(Pid=%u): The request data is abnormal[%s]!!!!",getpid(), udp_buf);
		return 0;
	}
	
	snprintf(tmpkey, 60, "%s", &udp_buf[MS_CMD_LEN+1]);
	TraceEvent( TRACE_NORMAL,"Father(Pid=%u): tmpkey=%s, Create New Meetme",getpid(), tmpkey);

	pid = fork();
	if(pid == 0)
	{
		start_meetme(socket, &sockaddr, tmpkey);
		TraceEvent(TRACE_NORMAL, "Child(Pid=%u): Exit MeetMe the process", getpid());
		exit(0);
	}
	else
	{
		TraceEvent(TRACE_NORMAL, "Father(Pid=%u): Returns the process to continue listening", getpid());
		return 0;
	}
}
static int run_loop( ms_info_t * ms_info )
{
	int keep_running=1;
	int max_sock,rc;
	fd_set socket_mask;
	struct timeval wait_time;
	struct sockaddr_in sender_sock;
	socklen_t i;
	ssize_t bread;
	uint8_t pktbuf[MS_PKTBUF_SIZE];
	
	TraceEvent(TRACE_NORMAL, "Father(Pid=%u): Mixing Server started", getpid());
	
	signal(SIGCLD, SIG_IGN);

	while(keep_running)
	{
		FD_ZERO(&socket_mask);
		max_sock = ms_info->mixer_fd;
		FD_SET(ms_info->mixer_fd, &socket_mask);

		wait_time.tv_sec = 10; 
		wait_time.tv_usec = 0;

		rc = select(max_sock+1, &socket_mask, NULL, NULL, &wait_time);
		if(rc > 0)
		{
			if(FD_ISSET(ms_info->mixer_fd, &socket_mask))
			{
				i = sizeof(sender_sock);
				bread = recvfrom( ms_info->mixer_fd, pktbuf, MS_PKTBUF_SIZE, 0/*flags*/, (struct sockaddr *)&sender_sock, (socklen_t*)&i);
				if(bread < MS_CMD_LEN)
				{
					TraceEvent( TRACE_ERROR, "Father(Pid=%u): recvfrom() failed %d errno %d (%s)", getpid(), bread, errno, strerror(errno));
					continue;
				}
				else
				{
					if (strncmp((const char *)pktbuf, MS_CMD_NCF, MS_CMD_LEN)==0)
					{
						process_udp(ms_info->mixer_fd, &sender_sock, pktbuf, bread);
					}
				}
			}
		}
		else if(rc == 0)
		{
			//TraceEvent( TRACE_INFO, "Father(Pid=%u): select timeout", getpid() );
		}
		else if(rc < 0)
		{
			TraceEvent( TRACE_INFO, "Father(Pid=%u): select error", getpid() );
		}
	}

	deinit_ms_info(ms_info);
	return 0;
}
int main( int argc, char * const argv[] )
{
  ms_info_t ms_info;
  int bind_any = 1;
  int opt;
  init_ms_info( &ms_info );
  //libvd_delete_video_channel(0);
  while((opt = getopt_long(argc, argv, "fl:a:vh", long_options, NULL)) != -1) 
  {
    switch (opt) 
    {
      case 'l': /* local-port */
        ms_info.mixer_port = atoi(optarg);
        break; 	  	      	  	    
      case 'a': /* local-ip */
        strcpy(ms_info.mixer_ip, optarg);
        bind_any = 0;
        break;	  	     	  	             
      case 'f': /* foreground */
        ms_info.daemon = 0;
        break;
      case 'h': /* help */
        exit_help(argc, argv);
        break;
      case 'v': /* verbose */
        ++traceLevel;
        break;
    }
  }
	if (ms_info.daemon)
  {
    useSyslog=1; /* traceEvent output now goes to syslog. */
    if ( -1 == daemon( 0, 0 ) )
    {
      TraceEvent( TRACE_ERROR, "Father(Pid=%u): Failed to become daemon.", getpid() );
      exit(-5);
    }
    else
    {
      TraceEvent( TRACE_NORMAL, "Father(Pid=%u): traceLevel is %d", getpid(), traceLevel);
    }
  }
  TraceEvent( TRACE_NORMAL, "Mixer Server version is [%s],data_hdr_len is 28", MS_VERSION_NUM);

  ms_info.mixer_fd = setup_ms_socket(ms_info.mixer_port, ms_info.mixer_ip, bind_any );
  if ( -1 == ms_info.mixer_fd )
  {
    TraceEvent( TRACE_ERROR, "Father(Pid=%u): Failed to open main socket. %s", getpid(), strerror(errno) );
    exit(-2);
  }
  else
  {
    TraceEvent( TRACE_NORMAL, "Father(Pid=%u): MixingServer is listening on UDP %u (main)", getpid(), ms_info.mixer_port);
  }
	
  return run_loop(&ms_info);
}
