#define LD_DISCONNECTION 	0x8000 		//-q32768	
#define LD_CONNECTION 		0x4000		//-q16384
#define LD_CASCADING		0x2000		//-q8192
#define LD_QUEUE		0x1000		//-q4096
#define LD_PORTSHARING		0x0800		//-q2048
#define LD_POWER		0x0400		//-q1024
#define LD_SERIAL		0x0200		//-q512
#define LD_PROTOCOL		0x0100		//-q256
#define LD_WEB			0x0080		//-q128
#define LD_NETWORK		0x0040		//-q64
#define LD_OSD			0x0020		//-q32
#define LD_CONFIG		0x0010		//-q16
#define LD_USER			0x0008		//-q8
#define LD_THREAD		0x0004		//-q4
#define LD_STACK		0x0002		//-q2
#define LD_OTHER		0x0001		//-q1

#ifndef dolog
#define dolog(L, P...)	\
	if (kvmdebug) {\
		if (L <= kvmdebug) {\
			printf(##P);\
			printf("\r\n");\
			fflush(stdout);\
		}\
	} else {\
		syslog(L, ##P);\
	}
#endif

#define LOGDEBUG(L, P...) pmesg(L, __LINE__, __FUNCTION__ , ## P)

void pmesg(unsigned long, int, const char*, char*, ...);
unsigned long set_debug(unsigned long);
