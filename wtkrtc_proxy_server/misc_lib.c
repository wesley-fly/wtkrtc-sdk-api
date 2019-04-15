#include "misc_lib.h"

//Log API

struct RT_Info_List* RtInfoListArray[ROUTETABLE_LIST_SIZE];
int traceLevel = 0;
int useSyslog = 0;
int syslog_opened = 0;
void TraceEvent(int level, char* file, int line, char* format, ...)
{
	va_list va_ap;
	if(level <= traceLevel)
	{
		char LogTime[TRACE_DATESIZE] = {0};
		char buf[1024];
		char out_buf[640];
		char* extra_msg = "";
		
		time_t timenow = time(NULL);

		memset(buf, 0, sizeof(buf));
		strftime(LogTime, TRACE_DATESIZE, "%d/%b/%Y %H:%M:%S", localtime(&timenow));
		
		va_start (va_ap, format);
		vsnprintf(buf, sizeof(buf)-1, format, va_ap);
		va_end(va_ap);
		
		if(level == 0)
			extra_msg = "ERROR: ";
		else if(level == 1)
			extra_msg = "WARNING: ";
		else if(level == 2)
			extra_msg = "NORMAL: ";
		else if(level == 3)
			extra_msg = "INFO: ";
		else if(level == 4)
			extra_msg = "DEBUG: ";

		while(buf[strlen(buf)-1] == '\n') 
			buf[strlen(buf)-1] = '\0';

		if(useSyslog) {
			if(!syslog_opened) {
				openlog("relayserver", LOG_PID, LOG_LOCAL6);
				syslog_opened = 1;
			}

			snprintf(out_buf, sizeof(out_buf), "%s%s", extra_msg, buf);
			syslog(LOG_INFO, "%s", out_buf);
		} else {
			snprintf(out_buf, sizeof(out_buf), "%s [%11s:%4d] %s%s", LogTime, file, line, extra_msg, buf);
			printf("%s\n", out_buf);
			fflush(stdout);
		}
	}
}
int inaddrcmp(const struct sockaddr_in *sin1, const struct sockaddr_in *sin2)
{
	return ((sin1->sin_addr.s_addr != sin2->sin_addr.s_addr) 
		|| (sin1->sin_port != sin2->sin_port));
}
int inonlyaddrcmp(const struct sockaddr_in *sin1, const struct sockaddr_in *sin2)
{
	return (sin1->sin_addr.s_addr != sin2->sin_addr.s_addr);
}
//Socket API
int setup_socket(int port, char* ipaddr, int bind_any)
{
	int socket_fd;
	struct sockaddr_in local_address;
	int sockopt = 1;
	int sndbuf=0;			/* Send buffer size */
	int rcvbuf=0;			/* Receive buffer size */
	socklen_t optlen;		/* Option length */

	if((socket_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		TraceEvent(TRACE_ERROR, "Unable to create socket [%s][%d]\n", strerror(errno), socket_fd);
		return -1;
	}
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&sockopt, sizeof(sockopt));

	optlen = sizeof(sndbuf);
	getsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, &optlen);
	optlen = sizeof(rcvbuf);
	getsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &optlen);

	memset(&local_address, 0, sizeof(local_address));
	local_address.sin_family = AF_INET;
	local_address.sin_port = htons(port);
	if (bind_any == 1)
		local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		local_address.sin_addr.s_addr = inet_addr(ipaddr);

	if(bind(socket_fd, (struct sockaddr*) &local_address, sizeof(local_address)) == -1) {
		TraceEvent(TRACE_ERROR, "Bind error [%s]\n", strerror(errno));
		return -1;
	}
	return socket_fd;
}
//IAX API
int iax_parse_ies(struct iax_ies *ies, unsigned char *data, int datalen)
{
	/* Parse data into information elements */
	int len;
	int ie;
	memset(ies, 0x00, (int)sizeof(struct iax_ies));
	while(datalen >= 2) {
		ie = data[0];
		len = data[1];
		if (len > datalen - 2) {
			TraceEvent(TRACE_ERROR, "Information element length exceeds message size");
			return -1;
		}
		switch(ie) {
			case IAX_IE_RELAY_TOKEN:
				memcpy(ies->relaytoken, (char *)data + 2, len);
				TraceEvent(TRACE_DEBUG, "ies->relaytoken=[%s], len=[%d]", ies->relaytoken, len);
			break;
			case IAX_IE_USERNAME:
				memcpy(ies->username, (char *)data + 2, len);
				TraceEvent(TRACE_DEBUG, "ies->username=[%s], len=[%d]", ies->username, len);
			break;								
			default:
			break;
		}
		datalen -= (len + 2);
		data += (len + 2);
	}
	if (datalen) {
		TraceEvent(TRACE_ERROR, "Invalid information element contents, strange boundary");
		return -1;
	}
	return 0;
}

int uncompress_subclass(unsigned char csub)
{
	/* If the SC_LOG flag is set, return 2^csub otherwise csub */
	if (csub & IAX_FLAG_SC_LOG) {
		/* special case for 'compressed' -1 */
		if (csub == 0xff)
			return -1;
		else
			return 1 << (csub & ~IAX_FLAG_SC_LOG & IAX_MAX_SHIFT);
	}
	else
		return csub;
}
//Route table API
//#define DEBUG_ROUTETABLE
struct RT_Info* find_routeinfo_by_relaytoken(struct RT_Info *list, const char *token)
{
	while(list != NULL)
	{
		if( 0 == memcmp(token, list->relaytoken, strlen(list->relaytoken)) )
		{
			return list;
		}
		list = list->next;
	}
	return NULL;
}
struct RT_Info* find_routeinfo_by_addr_and_callno(struct sockaddr_in *addr, unsigned short scallno, int *flag)
{
	struct RT_Info_List* rt_info_list = NULL;
	struct RT_Info * rt_info =NULL;
#ifdef DEBUG_ROUTETABLE
	TraceEvent(TRACE_ERROR, "To find [%s:%d] scallno:%d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port),scallno);
#endif
	for(rt_info_list = RtInfoListArray[scallno]; rt_info_list != NULL; rt_info_list = rt_info_list->next)
	{
		rt_info = (struct RT_Info*)rt_info_list->RT_Info;
#ifdef DEBUG_ROUTETABLE
		TraceEvent(TRACE_ERROR, "L find_routeinfo_by_addr_and_callno [%s:%d] scallno:%d", inet_ntoa(rt_info->l_ipaddr.sin_addr), ntohs(rt_info->l_ipaddr.sin_port),rt_info->l_callno);
		TraceEvent(TRACE_ERROR, "R find_routeinfo_by_addr_and_callno [%s:%d] scallno:%d", inet_ntoa(rt_info->r_ipaddr.sin_addr), ntohs(rt_info->r_ipaddr.sin_port),rt_info->r_callno);
#endif
		if ((rt_info->status == ROUTETABLE_SETTED)||(rt_info->status == ROUTETABLE_RELEASING))
		{
			if(!inaddrcmp(&rt_info->l_ipaddr, addr) && (rt_info->l_callno == scallno))
			{
				*flag = LEFT_SIDE_FRAME;
#ifdef DEBUG_ROUTETABLE
				TraceEvent(TRACE_ERROR, "Find left over");
#endif
				return rt_info;
			}
			if(!inaddrcmp(&rt_info->r_ipaddr, addr) && (rt_info->r_callno == scallno))
			{
				*flag = RIGHT_SIDE_FRAME;
#ifdef DEBUG_ROUTETABLE
				TraceEvent(TRACE_ERROR, "Find right over");
#endif
				return rt_info;
			}
		}
	}
#ifdef DEBUG_ROUTETABLE
	TraceEvent(TRACE_ERROR, "Not Find over");
#endif
	return NULL;
}
void add_route_to_RtInfoListArray(unsigned short scallno, struct RT_Info *rti)
{
	struct RT_Info_List* rt_info_list;

	rt_info_list = (struct RT_Info_List *)malloc(sizeof(struct RT_Info_List));
	rt_info_list->next = RtInfoListArray[scallno];

	RtInfoListArray[scallno] = rt_info_list;
	RtInfoListArray[scallno]->RT_Info = (void *)rti;
#ifdef DEBUG_ROUTETABLE
	TraceEvent(TRACE_ERROR, "Add [%s:%d] scallno:%d", inet_ntoa(rti->l_ipaddr.sin_addr), ntohs(rti->l_ipaddr.sin_port),scallno);
#endif
}
void del_route_from_RtInfoListArray(struct sockaddr_in *addr, unsigned short scallno)
{
	struct RT_Info_List *cur_rt_info_list, *prev;
	struct RT_Info *rti;
	
	cur_rt_info_list = RtInfoListArray[scallno];
	prev = NULL;
	while(cur_rt_info_list != NULL)
	{
		rti = (struct RT_Info*)cur_rt_info_list->RT_Info;
		if((!inaddrcmp(&rti->l_ipaddr,addr)&&(rti->l_callno == scallno))||(!inaddrcmp(&rti->r_ipaddr,addr)&&(rti->r_callno == scallno)))
		{
			struct RT_Info_List *cur_next = cur_rt_info_list->next;
			if(prev == NULL)
			{
				RtInfoListArray[scallno] = cur_next;
			}
			else
			{
				prev->next = cur_next;
			}
			free(cur_rt_info_list);
			cur_rt_info_list = cur_next;
		}
		else
		{
			prev = cur_rt_info_list;
			cur_rt_info_list = cur_rt_info_list->next;
		}
	}
}

//Mgmt API
void list_all_detail_route( struct RT_Info *list,  char *resbuf)
{
	int len = 0;
	while(list != NULL)
	{
		sprintf(resbuf+len, "RelayToken=[%s],Status=[%d]\n", list->relaytoken, list->status);
		len = strlen(resbuf);
		
		list = list->next;
	}
}
void list_detail_route( struct RT_Info *list,  char *relaytoken, char *resbuf)
{
	int len = 0;
	while(list != NULL)
	{
		if(0 == memcmp(relaytoken, list->relaytoken, strlen(list->relaytoken)))
		{
			sprintf(resbuf+len, "RelayToken=[%s],Status=[%d]\n", list->relaytoken, list->status);
			break;
		}
		list = list->next;
	}
}