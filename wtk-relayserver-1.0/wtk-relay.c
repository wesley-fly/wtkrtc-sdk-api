#include "wtk-relay.h"

int epoll_fd = -1;

static const struct option long_options[] = {
	{ "foreground",      no_argument,       NULL, 'f' },
	{ "local-port",      required_argument, NULL, 'l' },
	{ "local-ip",        required_argument, NULL, 'a' },
	{ "manager-port",    required_argument, NULL, 'p' },
	{ "manager-ip",      required_argument, NULL, 'm' },		
	{ "md5key",          required_argument, NULL, 'k' },
	{ "help"   ,         no_argument,       NULL, 'h' },
	{ "verbose",         no_argument,       NULL, 'v' },
	{ NULL,              0,                 NULL,  0  }
};

static void exit_help(int argc, char * const argv[])
{
	fprintf( stderr, "%s usage\n", argv[0] );
	fprintf( stderr, "-l <lport>\tSet UDP main listen port to <lport>\n" );
	fprintf( stderr, "-a <lip>\tSet UDP main listen ip to <lip>\n" );
	fprintf( stderr, "-p <lport>\tSet UDP manager listen port to <mport>\n" );
	fprintf( stderr, "-m <lip>\tSet UDP manager listen ip to <mip>\n" );    
	fprintf( stderr, "-k <md5key>\tSet md5 key <md5key>\n" );
	fprintf( stderr, "-f        \tRun in foreground.\n" );
	fprintf( stderr, "-v        \tIncrease verbosity. Can be used multiple times.\n" );
	fprintf( stderr, "-h        \tThis help message.\n" );
	fprintf( stderr, "\n" );
	exit(1);
}

static void init_rs_info(rs_info_t *rs_info)
{
	memset( rs_info, 0, sizeof(rs_info_t) );

	rs_info->daemon = 1;
	memset(rs_info->md5key, 0x00, sizeof(rs_info->md5key));
	
	memset(rs_info->relay_ip, 0x00, sizeof(rs_info->relay_ip));
	rs_info->relay_port = RELAY_PORT_DEFAULT;
	rs_info->relay_fd = -1;
	
	strcpy(rs_info->mgmt_ip, "127.0.0.1");
	rs_info->mgmt_port = MGMT_PORT_DEFAULT;
	rs_info->mgmt_fd = -1;

	rs_info->rti = NULL;
}
static void deinit_rs_info(rs_info_t *rs_info)
{
	if(rs_info->relay_fd >= 0)
		close(rs_info->relay_fd);
	rs_info->relay_fd = -1;
	
	if(rs_info->mgmt_fd >= 0)
		close(rs_info->mgmt_fd);
	rs_info->mgmt_fd = -1;
	//clear_relay_route_list();
}
static int process_udp(rs_info_t* rs_info,struct sockaddr_in* sender_sock,uint8_t* udp_buf,size_t udp_size)
{
	int flag=0;
	int res = udp_size;

	struct iax_ies ies;
	struct ast_iax2_full_hdr *fh = NULL;
	struct ast_iax2_mini_hdr *mh = NULL;
	struct ast_iax2_video_hdr *vh = NULL;
	struct RT_Info *scan = NULL;

	fh = (struct ast_iax2_full_hdr *) udp_buf;
	mh = (struct ast_iax2_mini_hdr *) udp_buf;
	vh = (struct ast_iax2_video_hdr *) udp_buf;

	if (res < sizeof(*mh)) {
		TraceEvent( TRACE_WARNING, "Too small packet received (%d of %d min), packet from '%s:%d'", res, sizeof(struct ast_iax2_mini_hdr), inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
		return -1;
	}
	/*video frame return immediately after deal*/
	if ((vh->zeros == 0) && (ntohs(vh->callno) & 0x8000))
	{
		if (res < sizeof(*vh)) {
			TraceEvent( TRACE_WARNING, "Rejecting packet from '%s.%d' that is flagged as a video frame but is too short", inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
			return 1;
		}
		scan = find_routeinfo_by_addr_and_callno(sender_sock, ntohs(vh->callno) & ~0x8000,  &flag);
		if (NULL != scan)
		{
			if (flag == LEFT_SIDE_FRAME)
			{
				sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->r_ipaddr), sizeof(struct sockaddr_in));
			}
			else
			{
				sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->l_ipaddr), sizeof(struct sockaddr_in));
			}
		}
		else
		{
			TraceEvent( TRACE_INFO, "Routing table is not established, Discard Mini Video Frame, packet from '%s:%d'", inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
		}
	}
	//Full Frame
	if (ntohs(fh->scallno) & IAX_FLAG_FULL) 
	{
		if (res < sizeof(*fh)) {
			TraceEvent( TRACE_WARNING, "Rejecting packet from '%s:%d' that is flagged as a full frame but is too short", inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
			return -1;
		}
		int subclass = -1;
		if ( fh->type == AST_FRAME_VIDEO) 
		{
			subclass = uncompress_subclass(fh->csub & ~0x40) | ((fh->csub >> 6) & 0x1);
		} else {
			subclass = uncompress_subclass(fh->csub);
		}
		if((subclass == IAX_COMMAND_TXCNT)&&(fh->type == AST_FRAME_IAX))
		{
			if(iax_parse_ies(&ies, udp_buf + sizeof(*fh), res - sizeof(*fh))) 
			{
				TraceEvent( TRACE_WARNING, "iax_parse_ies is fail, packet from '%s:%d'", inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
				return -1;
			}
			if (strlen(ies.relaytoken)==0)
			{
				TraceEvent( TRACE_WARNING, "ies.RelayToken is empty, packet from '%s:%d'", inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
				return -1;
			}
			/*
			//TODO:Verify TXCNT, TXREQ
			if (data_validity_check(ies.relaytoken,rs_info->md5key))
			{
				TraceEvent( TRACE_WARNING, "Data validation is not passed, RelayToken=[%s],packet from '%s:%d'", ies.RelayToken, inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
				return -1;
			}
			*/
			scan = find_routeinfo_by_relaytoken(rs_info->rti, ies.relaytoken);
			if ( NULL == scan )
			{
				TraceEvent( TRACE_INFO, "Can not find route ies.relaytoken=%s, begin create", ies.relaytoken);
				scan = (struct RT_Info*)calloc(1, sizeof(struct RT_Info));
				memcpy(scan->relaytoken, ies.relaytoken, strlen(ies.relaytoken));
				memcpy(scan->l_username, ies.username, strlen(ies.username));
				memcpy(&(scan->l_ipaddr), sender_sock, sizeof(struct sockaddr_in));
				scan->l_callno = ntohs(fh->scallno) & ~0x8000;
				scan->status = ROUTETABLE_SETTING;
				memcpy(scan->pktbuf, udp_buf, udp_size); 
				scan->pkt_len = udp_size;
				rs_info->rti = scan;

				add_route_to_RtInfoListArray(scan->l_callno, scan);
			}
			else
			{
				if ((scan->status == ROUTETABLE_SETTING)&&(scan->l_callno == (ntohs(fh->scallno) & ~0x8000)))
				{
					if(inaddrcmp(&scan->l_ipaddr, sender_sock))
					{
						TraceEvent( TRACE_INFO, "Frame update, l_ipaddr change to [%s:%d]", inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
						memcpy(&(scan->l_ipaddr), sender_sock, sizeof(struct sockaddr_in));
						memcpy(scan->pktbuf, udp_buf, udp_size); 
						scan->pkt_len = udp_size;
					}
					else
					{
						TraceEvent( TRACE_INFO, "Frame retransmissions, ies.relaytoken=%s", ies.relaytoken);
						memcpy(scan->pktbuf, udp_buf, udp_size);
						scan->pkt_len = udp_size;
					}
				}
				else if((scan->status == ROUTETABLE_SETTING)&&(scan->l_callno != (ntohs(fh->scallno) & ~0x8000)))
				{
					TraceEvent( TRACE_INFO, "Other leg frame transmissions, ies.relaytoken=%s", ies.relaytoken);
					scan->r_callno = ntohs(fh->scallno) & ~0x8000;
					memcpy(&(scan->r_ipaddr), sender_sock, sizeof(struct sockaddr_in));
					memcpy(scan->r_username, ies.username, strlen(ies.username));
					scan->status = ROUTETABLE_SETTED;

					sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->l_ipaddr), sizeof(struct sockaddr_in));
					sendto(rs_info->relay_fd, scan->pktbuf, scan->pkt_len, 0, (struct sockaddr*)&(scan->r_ipaddr), sizeof(struct sockaddr_in));

					add_route_to_RtInfoListArray(scan->r_callno, scan);
					
					TraceEvent( TRACE_INFO, "Route table create success, ies.relaytoken = %s", ies.relaytoken);

					return 0;
				}
				if ((scan->status == ROUTETABLE_SETTED)||(scan->status == ROUTETABLE_RELEASING))
				{
					int len = 0;
					if (scan->l_callno == (ntohs(fh->scallno) & ~0x8000))
					{
						if(inaddrcmp(&scan->l_ipaddr, sender_sock) && ies.txreason == IAX_TXREASON_HEARTBEAT)
						{
							TraceEvent( TRACE_INFO, "IAX_TXREASON_HEARTBEAT and left ipaddr changed from [%s:%d] to [%s:%d]",
								inet_ntoa(scan->l_ipaddr.sin_addr), ntohs(scan->l_ipaddr.sin_port),
								inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
							scan->l_txsequence = 0;
							scan->l_txmaxcount = 0;
							if (modify_txreason_ie(udp_buf + sizeof(*fh), udp_size - sizeof(*fh)))
							{
								TraceEvent(TRACE_INFO, "Modify modify_txreason_ie left fail!!!!!!!");
								len = udp_size;
							}
							else
							{
								memcpy(&(scan->l_ipaddr), sender_sock, sizeof(struct sockaddr_in));
								udp_buf[udp_size] = IAX_IE_APPARENT_ADDR;
								udp_buf[udp_size+1] = (int)sizeof(struct sockaddr_in);
								memcpy(udp_buf+udp_size+2, sender_sock, (int)sizeof(struct sockaddr_in));
								len = udp_size+2+(int)sizeof(struct sockaddr_in);
								
								udp_buf[len] = IAX_IE_TXSEQUENCE;
								udp_buf[len+1] = 1;
								udp_buf[len+2] = ++scan->l_txsequence;    	  							
								len = len + 3;

								scan->l_txstatus = 1;
								scan->l_txmaxcount++;
							}
							TraceEvent( TRACE_INFO, "Left r_txsequence=[%d], count = %d", scan->r_txsequence,scan->r_txmaxcount);
							sendto(rs_info->relay_fd, udp_buf, len, 0, (struct sockaddr*)&(scan->r_ipaddr), sizeof(struct sockaddr_in));
						}
						else if (scan->l_txstatus==1 && ies.txreason==IAX_TXREASON_HEARTBEAT && scan->l_txmaxcount < 4)
						{
							if (modify_txreason_ie(udp_buf + sizeof(*fh), udp_size - sizeof(*fh)))
							{
								TraceEvent(TRACE_INFO, "Modify modify_txreason_ie left fail!!!!!!!");
								len = udp_size;
							}
							else
							{
								udp_buf[udp_size] = IAX_IE_APPARENT_ADDR;
								udp_buf[udp_size+1] = (int)sizeof(struct sockaddr_in);
								memcpy(udp_buf+udp_size+2, sender_sock, (int)sizeof(struct sockaddr_in));
								len = udp_size+2+(int)sizeof(struct sockaddr_in);
								
								udp_buf[len] = IAX_IE_TXSEQUENCE;
								udp_buf[len+1] = 1;
								udp_buf[len+2] = scan->l_txsequence;    	  							
								len = len + 3;
								
								scan->l_txstatus = 1;
								scan->l_txmaxcount++;
							}
							TraceEvent( TRACE_INFO, "Re-Left r_txsequence=[%d], count = %d", scan->l_txsequence,scan->l_txmaxcount);
							
							sendto(rs_info->relay_fd, udp_buf, len, 0, (struct sockaddr*)&(scan->r_ipaddr), sizeof(struct sockaddr_in));
						}
						else if (scan->r_txstatus==1 && ies.txreason==IAX_TXREASON_NETCHANGE && ies.txsequence==scan->r_txsequence)
						{
							scan->r_txstatus = 0;
							scan->r_txmaxcount = 0;
							sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->r_ipaddr), sizeof(struct sockaddr_in));
							TraceEvent( TRACE_INFO, "Left IAX_TXREASON_NETCHANGE done!");
						}
						else
						{      	  					
							sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->r_ipaddr), sizeof(struct sockaddr_in));
						}
					}
					else
					{
						if (inaddrcmp(&scan->r_ipaddr, sender_sock) && ies.txreason == IAX_TXREASON_HEARTBEAT)
						{
							TraceEvent( TRACE_INFO, "IAX_TXREASON_HEARTBEAT and right ipaddr changed from [%s:%d] to [%s:%d]\r\n",
								inet_ntoa(scan->r_ipaddr.sin_addr), ntohs(scan->r_ipaddr.sin_port),
								inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
							scan->r_txsequence = 0;
							scan->r_txmaxcount = 0;
							if (modify_txreason_ie(udp_buf + sizeof(*fh), udp_size - sizeof(*fh)))
							{
								TraceEvent(TRACE_INFO, "Modify modify_txreason_ie right fail!!!!!!!");
								len = udp_size;
							}
							else
							{
								memcpy(&(scan->r_ipaddr), sender_sock, sizeof(struct sockaddr_in));
								udp_buf[udp_size] = IAX_IE_APPARENT_ADDR;
								udp_buf[udp_size+1] = (int)sizeof(struct sockaddr_in);
								memcpy(udp_buf+udp_size+2, sender_sock, (int)sizeof(struct sockaddr_in));
								len = udp_size+2+(int)sizeof(struct sockaddr_in);
								udp_buf[len] = IAX_IE_TXSEQUENCE;
								udp_buf[len+1] = 1;
								udp_buf[len+2] = ++scan->r_txsequence;    	  							
								len = len + 3;

								scan->r_txstatus = 1;
								scan->r_txmaxcount++; 
							}
							TraceEvent( TRACE_INFO, "Right r_txsequence=[%d], count = %d", scan->r_txsequence,scan->r_txmaxcount);
							sendto(rs_info->relay_fd, udp_buf, len, 0, (struct sockaddr*)&(scan->l_ipaddr), sizeof(struct sockaddr_in));
						}
						else if (scan->r_txstatus==1 && ies.txreason==IAX_TXREASON_HEARTBEAT && scan->r_txmaxcount < 4)
						{
							if (modify_txreason_ie(udp_buf + sizeof(*fh), udp_size - sizeof(*fh)))
							{
								TraceEvent(TRACE_INFO, "Modify modify_txreason_ie right fail!!!!!!!");
								len = udp_size;
							}
							else
							{
								udp_buf[udp_size] = IAX_IE_APPARENT_ADDR;
								udp_buf[udp_size+1] = (int)sizeof(struct sockaddr_in);
								memcpy(udp_buf+udp_size+2, sender_sock, (int)sizeof(struct sockaddr_in));
								len = udp_size+2+(int)sizeof(struct sockaddr_in);
								udp_buf[len] = IAX_IE_TXSEQUENCE;
								udp_buf[len+1] = 1;
								udp_buf[len+2] = scan->r_txsequence;
								len = len + 3;
								
								scan->r_txstatus = 1;
								scan->r_txmaxcount++;
							}
							TraceEvent( TRACE_INFO, "Re-Right r_txsequence=[%d], count = %d", scan->r_txsequence,scan->r_txmaxcount);
							sendto(rs_info->relay_fd, udp_buf, len, 0, (struct sockaddr*)&(scan->l_ipaddr), sizeof(struct sockaddr_in));
						}
						else if (scan->l_txstatus==1 && ies.txreason==IAX_TXREASON_NETCHANGE && ies.txsequence==scan->l_txsequence)
						{
							scan->l_txstatus = 0;
							scan->l_txmaxcount = 0;
							sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->l_ipaddr), sizeof(struct sockaddr_in));
							TraceEvent( TRACE_INFO, "Right IAX_TXREASON_NETCHANGE done!");
						}
						else
						{
							sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->l_ipaddr), sizeof(struct sockaddr_in));
						}
					}
					return 0;
				}
			}
		}
		else if((subclass == IAX_COMMAND_TXREQ)&&(fh->type == AST_FRAME_IAX))
		{
			TraceEvent( TRACE_INFO, "Other Full frame IAX_COMMAND_TXREQ");
		}
		else if((subclass == IAX_COMMAND_TXREADY)&&(fh->type == AST_FRAME_IAX))
		{
			TraceEvent( TRACE_INFO, "Other Full frame IAX_COMMAND_TXREADY");
			/*scan = find_routeinfo_by_addr_and_callno(sender_sock, ntohs(fh->scallno) & ~0x8000, &flag);
			if (NULL != scan)
			{
				if (flag == LEFT_SIDE_FRAME)
				{
					scan->txstatus = 1<<0;
				}
				else
				{
					scan->txstatus = 1<<1;
				}
			}*/
		}
		else if((subclass == IAX_COMMAND_TXREL)&&(fh->type == AST_FRAME_IAX))
		{
			TraceEvent( TRACE_INFO, "Other Full frame IAX_COMMAND_TXREL");
		}
		else
		{
			//TXACC:24/PING:2/PONG:3/ACK:4/HANGUP:5/TXREJ:27
			if((subclass == IAX_COMMAND_TXREADY)||(subclass == IAX_COMMAND_TXREL)||(subclass == IAX_COMMAND_TXREQ)||(subclass == IAX_COMMAND_TXACC))
				TraceEvent( TRACE_INFO, "Other Full frame subclass = %d, packet from [%s:%d], just forward it", subclass, inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
			scan = find_routeinfo_by_addr_and_callno(sender_sock, ntohs(fh->scallno) & ~0x8000, &flag);
			if (NULL != scan)
			{
				if (flag == LEFT_SIDE_FRAME)
				{
					sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->r_ipaddr), sizeof(struct sockaddr_in));
				}
				else
				{
					sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->l_ipaddr), sizeof(struct sockaddr_in));
				}

				if (fh->type == AST_FRAME_IAX)
				{
					if(subclass == IAX_COMMAND_HANGUP)
					{
						TraceEvent( TRACE_INFO, "IAX_COMMAND_HANGUP, packet from '%s:%d'", inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
						scan->status = ROUTETABLE_RELEASING;
						del_route_from_RtInfoListArray(&scan->l_ipaddr, scan->l_callno);
						del_route_from_RtInfoListArray(&scan->r_ipaddr, scan->r_callno);
						return 0;
					}
					else if(subclass == IAX_COMMAND_TXREJ)
					{
						TraceEvent( TRACE_INFO, "IAX_COMMAND_TXREJ, packet from '%s:%d'", inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
						scan->status = ROUTETABLE_RELEASED;
						del_route_from_RtInfoListArray(&scan->l_ipaddr, scan->l_callno);
						del_route_from_RtInfoListArray(&scan->r_ipaddr, scan->r_callno);
						return 0;
					}
				}
			}
			else
			{
				TraceEvent( TRACE_INFO, "Routing table is not established, Discard Full Frame=%d, packet from '%s:%d'", subclass, inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
			}
		}
	}
	//Mini Frame
	else
	{
		scan = find_routeinfo_by_addr_and_callno(sender_sock, ntohs(mh->callno) & ~0x8000, &flag);
		if(scan != NULL)
		{
			if(flag == LEFT_SIDE_FRAME)
			{
				sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->r_ipaddr), sizeof(struct sockaddr_in));
			}
			else
			{
				sendto(rs_info->relay_fd, udp_buf, udp_size, 0, (struct sockaddr*)&(scan->l_ipaddr), sizeof(struct sockaddr_in));
			}
		}
		else
		{
			TraceEvent( TRACE_DEBUG, "Routing table is not established, Discard Mini Frame, packet from '%s:%d'", inet_ntoa(sender_sock->sin_addr), ntohs(sender_sock->sin_port));
		}
	}
	return 0;
}
static int process_mgmt(rs_info_t* rs_info,struct sockaddr_in* sender_sock,uint8_t* mgmt_buf,size_t mgmt_size)
{
	struct mgmt_type* type=NULL;
	char resbuf[RELAY_PKTBUF_SIZE*10];

	memset(resbuf, 0x00, sizeof(resbuf));
	type = (struct mgmt_type *)mgmt_buf;

	if(type->type == MGMT_ROUTELIST)
	{
		if(type->csub == MGMT_ROUTELIST_ALL)
			list_all_detail_route( rs_info->rti,  resbuf);
		else if(type->csub == MGMT_ROUTELIST_CUR)
			list_detail_route(rs_info->rti, (char *)type->value, resbuf);
	}
	else if(type->type == MGMT_CONFIG)
	{
		if (type->csub==MGMT_CONFIG_TRACELEVEL)
		{
			TraceEvent( TRACE_WARNING, "Set log level from %d to %d", traceLevel, type->value[0] & 0xFF);
			sprintf(resbuf, "Set log level from %d to %d", traceLevel, type->value[0] & 0xFF);
			traceLevel = type->value[0] & 0xFF;
		}
		else
			return -1;
	}
	else
		return -1;
	
	sendto(rs_info->mgmt_fd, resbuf, strlen(resbuf), 0, (struct sockaddr *)sender_sock, sizeof(struct sockaddr_in));

	return 0;
}

static int run_loop(rs_info_t *rs_info)
{
	struct epoll_event events[MAXEPOLLSIZE];
	int event_fds = -1;
	int id = 0;
	struct sockaddr_in sender_sock;
	socklen_t i;
	uint8_t pktbuf[RELAY_PKTBUF_SIZE];
	ssize_t numread; 
	
	TraceEvent(TRACE_NORMAL, "Relayserver started");

	while(1)
	{
		event_fds = epoll_wait(epoll_fd, events, MAXEPOLLSIZE, -1);
		if(event_fds <= 0)
		{
			TraceEvent(TRACE_ERROR, "epoll_wait return value=%d(0 == timeout) fail!!!!!", event_fds);
			continue;
		}
		for(id=0; id<event_fds; id++)
		{
			if(-1 == events[id].data.fd)
				continue;
			if(events[id].events & EPOLLIN){ 
				i = sizeof(sender_sock);
				memset(pktbuf, 0x00, sizeof(pktbuf));
				numread = recvfrom( events[id].data.fd, pktbuf, RELAY_PKTBUF_SIZE, 0/*flags*/, (struct sockaddr *)&sender_sock, (socklen_t*)&i);
				if ( numread <= 0 )
				{
					TraceEvent( TRACE_ERROR, "recvfrom() failed %d errno %d (%s)", numread, errno, strerror(errno) );
					continue;
				}
				if ( numread > 0 )
				{
					if (events[id].data.fd == rs_info->relay_fd)
						process_udp(rs_info, &sender_sock, pktbuf, numread);
					else if	(events[id].data.fd == rs_info->mgmt_fd)
						process_mgmt(rs_info, &sender_sock, pktbuf, numread);	
				}
			}
		}
	}
	deinit_rs_info(rs_info);
	close(epoll_fd);
	return 0;
}
int main(int argc, char* const argv[])
{
	rs_info_t rs_info;
	int bind_any = 1;
	struct epoll_event ev;

	init_rs_info(&rs_info);
	int opt;
	while((opt = getopt_long(argc, argv, "fl:a:p:m:k:vh", long_options, NULL)) != -1) 
	{
		switch (opt) 
		{
			case 'l': /* relay_port */
				rs_info.relay_port = atoi(optarg);
			break;
			case 'a': /* relay_ip */
				strcpy(rs_info.relay_ip, optarg);
				bind_any = 0;
			break;
			case 'p': /* manager-port */
				rs_info.mgmt_port= atoi(optarg);
			break;
			case 'm': /* manager-ip */
				strcpy(rs_info.mgmt_ip, optarg);
			break;				
			case 'k': /* md5key */
				strcpy(rs_info.md5key, optarg);
			break;					 
			case 'f': /* foreground */
				rs_info.daemon = 0;
			break;
			case 'h': /* help */
				exit_help(argc, argv);
			break;
			case 'v': /* verbose */
				++traceLevel;
			break;
		}
	}
	if (rs_info.daemon)
	{
		useSyslog=1; /* traceEvent output now goes to syslog. */
		if ( -1 == daemon( 0, 0 ) )
		{
			TraceEvent( TRACE_ERROR, "Failed to become daemon." );
			exit(-5);
		}
	}
	TraceEvent( TRACE_ERROR, "TraceLevel is %d", traceLevel);
	
	rs_info.relay_fd = setup_socket(rs_info.relay_port, rs_info.relay_ip, bind_any );/*bind ANY*/
	if ( -1 == rs_info.relay_fd )
	{
		TraceEvent( TRACE_ERROR, "Failed to open Relayserver socket. %s", strerror(errno) );
		exit(-2);
	}
	else
	{
		TraceEvent( TRACE_NORMAL, "Relayserver is listening on UDP %u (main)", rs_info.relay_port );
	}
	
	rs_info.mgmt_fd = setup_socket(rs_info.mgmt_port, rs_info.mgmt_ip, 0 /* bind LOOPBACK */ );
	if ( -1 == rs_info.mgmt_fd )
	{
		TraceEvent( TRACE_ERROR, "Failed to open management socket. %s", strerror(errno) );
		exit(-2);
	}
	else
	{
		TraceEvent( TRACE_NORMAL, "Relayserver is listening on UDP %u (management)", rs_info.mgmt_port);
	}

	epoll_fd = epoll_create(MAXEPOLLSIZE);
	ev.events = EPOLLIN;
	ev.data.fd = rs_info.relay_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, rs_info.relay_fd, &ev) < 0) 
	{
		TraceEvent( TRACE_ERROR, "epoll set EPOLL_CTL_ADD error: rs_info.relay_fd=%d, errno %d (%s)", rs_info.relay_fd, errno, strerror(errno) );
		exit(-3);
	}
	else
	{
		TraceEvent( TRACE_NORMAL, "relay_fd added in epoll success");
	}
	
	ev.events = EPOLLIN;
	ev.data.fd = rs_info.mgmt_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, rs_info.mgmt_fd, &ev) < 0) 
	{
		TraceEvent( TRACE_ERROR, "epoll set EPOLL_CTL_ADD error: rs_info.mgmt_fd=%d, errno %d (%s)", rs_info.mgmt_fd, errno, strerror(errno) );
		exit(-3);
	}
	else
	{
		TraceEvent( TRACE_NORMAL, "mgmt_fd added in epoll success");
	}

	return run_loop(&rs_info);
}
