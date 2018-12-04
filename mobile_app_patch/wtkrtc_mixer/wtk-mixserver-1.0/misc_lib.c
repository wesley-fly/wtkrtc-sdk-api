#include "misc_lib.h"

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
				openlog("mixerserver", LOG_PID, LOG_LOCAL6);
				syslog_opened = 1;
			}

			snprintf(out_buf, sizeof(out_buf), "%s%s", extra_msg, buf);
			syslog(LOG_INFO, "%s", out_buf);
		} else {
			snprintf(out_buf, sizeof(out_buf), "%s [%11s:%4d] %s%s", LogTime, file, line, extra_msg, buf);
			snprintf(out_buf, sizeof(out_buf), "%s [Mixer Server] %s%s", LogTime, extra_msg, buf);
			printf("%s\n", out_buf);
			fflush(stdout);
		}
	}
}
int setup_ms_socket(int local_port, char *ip, int bind_any)
{
	int sock_fd;
	struct sockaddr_in local_address;
	//int sockopt = 1;

	if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0))  < 0) {
		TraceEvent(TRACE_ERROR, "(Pid=%u)Unable to create socket [%s][%d]", getpid(),strerror(errno), sock_fd);
		return(-1);
	}

	memset(&local_address, 0, sizeof(struct sockaddr_in));
	local_address.sin_family = AF_INET;
	local_address.sin_port = htons(local_port);
	
	if (bind_any == 1)
		local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		local_address.sin_addr.s_addr = inet_addr(ip);

	if(bind(sock_fd, (struct sockaddr*) &local_address, sizeof(struct sockaddr)) == -1) {
		TraceEvent(TRACE_ERROR, "(Pid=%u)Bind error [%s]",getpid(), strerror(errno));
		return(-1);
	}
	return(sock_fd);
}
struct channel_info * find_channel_info_by_sockaddr( struct channel_info *list, struct sockaddr_in *addr)
{
	while(list != NULL)
	{
		if (!inaddrcmp(&list->addr, addr))
		{
			return list;
		}
		list = list->next;
	}
	return NULL;
}
struct channel_info * find_sockaddr_by_channelno( struct channel_info *list, const int channelno)
{
	while(list != NULL)
	{
		if(channelno == list->channel_num)
		{
  			return list;
		}
		list = list->next;
	}
	return NULL;
}

