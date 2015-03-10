#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define __API_ADSAP2_
#include "api_adsap2.h"

/*
** Connects to the socket unixAdsap2
**
** Returns -1 if could not connect, otherwise the fd.
**
** Daemon adsap2 does not need it.
*/ 
int unixAdsap2_connect(void)
{
	int fd;
	struct sockaddr_un addr = {0};

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, UNIX_SOCKET_NAME);
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		return (-1);
	}
	return(fd);
}

/*
** get a message from the socket
**
** The first 2 bytes is the total message length. The returneds
**  message is malloc-ed. The caller must free it. 
*/
UnixAdsap2_msg *unixAdsap2_getmsg(int fd)
{
	unsigned short size;
	int len;
	UnixAdsap2_msg *msg;
	unsigned char *pdata;

	if (read(fd, &size, 2) != 2) {
		return (NULL);
	}

	if ((msg = (UnixAdsap2_msg *)malloc(size)) == NULL) {
		return(NULL);
	}

	msg->req.size = size;
	size -= 2;
	pdata = (unsigned char *)&msg->req.rtype;

	while (size > 0) {
		if ((len = read(fd, pdata, size)) <= 0) {
			free(msg);
			return(NULL);
		}
		pdata += len;
		size -= len;
	}
	return(msg);
}

/*
** writes a message to the socket.
*/
int unixAdsap2_putmsg(int fd, UnixAdsap2_msg *msg)
{
	int cnt;
	int len = msg->req.size;
	unsigned char *pdata = (unsigned char *)msg;

	while (len > 0) {
		if ((cnt = write(fd, pdata, len)) <= 0) {
			return(-1);
		}
		len -= cnt;
		pdata += cnt;
	}
	return(len);
}

/*
** gets the field by index from message. A new struct UnixAdsap2_field is malloc-ed. 
** The caller must free it.
*/
UnixAdsap2_field *unixAdsap2_get_field(int index, UnixAdsap2_msg *msg)
{
	int index_field, len_field;
	int cnt_field = 0;
	UnixAdsap2_field *pf, *pr, pa;

	if (msg == NULL) {
		return(NULL);
	}

	index_field = sizeof(UnixAdsap2_auth_req) - sizeof(UnixAdsap2_field);

	while (index_field < msg->req.size) {
		pf = (UnixAdsap2_field *)((char *)msg + index_field);
		memcpy(&pa, pf, sizeof(pa));
		if (cnt_field++  == index) {
			len_field = pa.flen + sizeof(UnixAdsap2_field) - 1;
			if ((pr = (UnixAdsap2_field *)malloc(len_field+1)) == NULL) {
				return(NULL);
			}
			memset(pr,0, len_field+1);
			memcpy(pr, pf, len_field);
			return(pr);
		}
		index_field += pa.flen + sizeof(UnixAdsap2_field) - 1;
	}
	return(NULL);
}

/*
** Allocates a new request auth message
*/
UnixAdsap2_msg *unixAdsap2_new_req(void)
{
	if (msg_req != NULL) free(msg_req);

	if ((msg_req = (UnixAdsap2_msg *)malloc(INITIAL_MSG_REQ_SIZE)) == NULL) {
		return (NULL);
	}

	memset(msg_req, 0, INITIAL_MSG_REQ_SIZE);

	msg_req_size = INITIAL_MSG_REQ_SIZE;
	msg_req->req.size = sizeof(UnixAdsap2_auth_req) - sizeof(UnixAdsap2_field);

	return(msg_req);
}

/*
** Add a field in the message req. If there is no space in the message it
** be reallocated. 
**
*/
int inixAdsap2_put_field(unsigned short flen, unsigned char fid, unsigned char *fval)
{
	int len_field, avail;
	UnixAdsap2_field *pf, *pa;

	if (msg_req == NULL) {
		return(-1);
	}

	len_field = flen + sizeof(UnixAdsap2_field) - 1;
	avail = msg_req_size - msg_req->req.size;

	if ((pa = (UnixAdsap2_field *)malloc(len_field)) == NULL) {
		return(-1);
	}

	if (len_field > avail) {
	}

	pf = (UnixAdsap2_field *)((char *)msg_req + msg_req->req.size);

	pa->flen = flen;
	pa->fid = fid;
	memcpy(pa->fval, fval, flen);
	memcpy(pf, pa, len_field);
	msg_req->req.size += len_field;

	return(0);
}

/*
** free the request auth message
*/
void unixAdsap2_drop_req(void)
{
	if (msg_req != NULL) free(msg_req);

	msg_req = NULL;
}

