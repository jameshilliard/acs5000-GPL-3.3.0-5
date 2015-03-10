#ifndef __API_ADSAP2_H
#define __API_ADSAP2_H

#define UNIX_SOCKET_NAME "/dev/unixAdsap2"

#define UnixAdsap2FieldIdUsername		1
#define UnixAdsap2FieldIdPassword		2
#define UnixAdsap2FieldIdBlobKey		3
#define UnixAdsap2FieldIdStyle			4

struct _UnixAdsap2_field {
	unsigned short	flen;
	unsigned char	fid;
	unsigned char	fval[1];
};

typedef struct _UnixAdsap2_field UnixAdsap2_field;

struct _UnixAdsap2_auth_req {
	unsigned short	size;
	unsigned short	rtype;		// 0 --> request
	unsigned short	port_num;
	unsigned short	check_only;
	UnixAdsap2_field	fields[1];
};

typedef struct _UnixAdsap2_auth_req UnixAdsap2_auth_req;

struct _UnixAdsap2_auth_rep {
	unsigned short	size;
	unsigned short	rtype;		// 1 --> reply
	unsigned short	rcode;		// return code 0 --> OK
	unsigned short	port_num;
	unsigned short	logging_dla;
	unsigned char	privilege;	// 0 --> regular user
								// 1 --> admin
	unsigned char	preempt;	// 0 --> disconnect if port busy
								// 1 --> close the other session if port busy;
	unsigned char   style[32];	// should hold what comes with username after :
								// example "ssh <username>:<resource> ..."
	unsigned char	command[32];
};

typedef struct _UnixAdsap2_auth_rep UnixAdsap2_auth_rep;

union _UnixAdsap2_msg {
	UnixAdsap2_auth_rep rep;
	UnixAdsap2_auth_req req;
};

typedef union _UnixAdsap2_msg UnixAdsap2_msg;

#ifndef _ADSAP2_SERVER_

#define INITIAL_MSG_REQ_SIZE 2048
#define INCR_MSG_REQ_SIZE 1024

#ifdef  __API_ADSAP2_
static int msg_req_size;
static UnixAdsap2_msg *msg_req = NULL;
#endif

int unixAdsap2_connect(void);

UnixAdsap2_msg *unixAdsap2_new_req(void);
int inixAdsap2_put_field(unsigned short flen, unsigned char fid, unsigned char *fval);
void unixAdsap2_drop_req(void);

#endif

UnixAdsap2_field *unixAdsap2_get_field(int index, UnixAdsap2_msg *msg);
UnixAdsap2_msg *unixAdsap2_getmsg(int fd);
int unixAdsap2_putmsg(int fd, UnixAdsap2_msg *msg);

#endif // __API_ADSAP2_H

