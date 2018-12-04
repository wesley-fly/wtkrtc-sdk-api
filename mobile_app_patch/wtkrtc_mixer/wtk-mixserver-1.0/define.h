#ifndef _define_h_
#define _define_h_
//Trace def
#define TRACE_DATESIZE 	32
#define TRACE_ERROR     0, __FILE__, __LINE__
#define TRACE_WARNING   1, __FILE__, __LINE__
#define TRACE_NORMAL    2, __FILE__, __LINE__
#define TRACE_INFO      3, __FILE__, __LINE__
#define TRACE_DEBUG     4, __FILE__, __LINE__

#define MS_VERSION_NUM	"WTK-MIXER.V01"
#define MS_PORT_DEFAULT 	8585
#define MS_PKTBUF_SIZE		2048
#define MS_RECV_TIMEOUT		45

#define MS_CMD_LEN 			6
#define MS_CMD_NCF 			"NEWCNF" 
#define MS_CMD_HCF 			"HUPCNF"

#endif
