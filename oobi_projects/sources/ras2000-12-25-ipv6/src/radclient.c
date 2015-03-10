/*
 * radclient	A client-side implementation of the RADIUS protocol.
 *
 * Version:	@(#)radclient.c  1.43  08-Nov-1997  miquels@cistron.nl
 *
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "radius.h"
#include "server.h"

#define ONE_UCHAR(ptr,offset) (*((unsigned char *)(ptr)+(offset)))

#define MSB_UINT4(ptr) ((ONE_UCHAR((ptr),0) << 24) + \
			(ONE_UCHAR((ptr),1) << 16) + \
			(ONE_UCHAR((ptr),2) << 8) + \
			(ONE_UCHAR((ptr),3)))

extern void radius_md5_calc(unsigned char *, unsigned char *, unsigned int);

/*
 *	This is an "attribute".
 */
typedef struct _attr_ {
	unsigned char type;
	unsigned char len;
	union {
		char str[254];
		unsigned int num;
	} val;
	unsigned char pwtype;
	struct _attr_ *next;
} ATTR;

/*
 *	This is a complete request or reply.
 */
typedef struct {
	unsigned char code;
	unsigned char id;
	unsigned char vector[AUTH_VECTOR_LEN];
	time_t timestamp;
	time_t lastsent;
	ATTR *list;
	unsigned char secret[100];
} RADPKT;


/*
 *	Create a new id.
 */
int rad_id()
{
	int fd, i, n;
	char id[8];

	if ((fd = open(RAD_ID_FILE, O_RDWR|O_CREAT, 0644)) < 0) {
		nsyslog(LOG_ERR, "%s: %m", RAD_ID_FILE);
		return -1;
	}
	for(i = 0; i < 10; i++) {
		if (i > 0) usleep(200000);
		if (flock(fd, LOCK_EX) == 0)
		        break;
	}
	if (i == 10) {
		nsyslog(LOG_ERR, "rad_id: failed to lock %s\n", RAD_ID_FILE);
		return -1;
	}

	n = read(fd, id, 7);
	if (n < 0) n = 0;
	id[n] = 0;
	i = (atoi(id) + 1) & 0xff;
	snprintf(id, sizeof (id), "%d\n", i);
	(void)ftruncate(fd, 0L);
	(void)lseek(fd, 0L, SEEK_SET);
	(void)write(fd, id, strlen(id));

	flock(fd, LOCK_UN);
	close(fd);

	return i;
}

/*
 *	Create a new session id.
 */
unsigned int rad_sessionid(char *s)
{
	char id[32];
#ifndef CYCLADES

	snprintf(id, 32, "%lu%u", time(NULL), getpid());

	if (s)
	{
		strncpy(s, id, 32);
		s[31] = 0;
	}
	return 0;
#else
	int fd, n;
	unsigned int i;

	if ((fd = open(RAD_SESSIONID_FILE, O_RDWR|O_CREAT, 0644)) < 0) {
		nsyslog(LOG_ERR, "%s: %m", RAD_SESSIONID_FILE);
		return -1;
	}
	for(i = 0; i < 10; i++) {
		if (i > 0) usleep(200000);
		if (flock(fd, LOCK_EX) == 0)
		        break;
	}
	if (i == 10) {
		nsyslog(LOG_ERR, "rad_sessionid: failed to lock %s\n",
			RAD_SESSIONID_FILE);
		return -1;
	}

	n = read(fd, id, 31);
	if (n < 0) n = 0;
	id[n] = 0;
	i = 0; n = 0;
	sscanf(id, "%x", &i);
	if ((i & 0x000fffff) == 0x000fffff) {
		i &= 0xfff00000;
		i += 0x00100000;
		if ((i &= 0xfff00000) == 0) {
			i = 0x00100000;
		}
		n = 1;
	}
	i++;
	sprintf(id, "%08x\n", i);
	(void)ftruncate(fd, 0L);
	(void)lseek(fd, 0L, SEEK_SET);
	(void)write(fd, id, strlen(id));

	flock(fd, LOCK_UN);
	close(fd);

	if (n) {
		system("sh /bin/updatefiles " RAD_SESSIONID_FILE " > /dev/ttyS0");
	}

	if (s) {
		strcpy(s, id);
		s[8] = 0;
	}
	return 0;
#endif
}

/*
 *	Create a random vector. We could use /dev/urandom
 *	for this under Linux for increased security, but..
 */
void rad_vector(char *vector)
{
	int	randno;
	int	i;

	srand(time(0) + getpid());
	for(i = 0;i < AUTH_VECTOR_LEN;) {
		randno = rand();
		memcpy(vector, &randno, sizeof(int));
		vector += sizeof(int);
		i += sizeof(int);
	}
}


/*
 *	Build an attribute.
 *	Integers are passed through without conversion...
 */
ATTR *rad_addattr(ATTR *list, int type, int val_len, unsigned int num, char *str)
{
	ATTR *a, *r;

	/*
	 * Ask for space.
	 */
	if ((r = (ATTR *)malloc(sizeof(ATTR))) == NULL) {
		nsyslog(LOG_ERR, "rad_addattr: Out of memory\n");
		return r;
	}
	memset(r, 0, sizeof(ATTR));

	/*
	 * Find end of the list (if any) and add us to it.
	 */
	for(a = list; a && a->next; a = a->next)
		;
	if (a) a->next = r;

	/*
	 * Fill out fields.
	 */
	if (str) {
		r->pwtype = PW_TYPE_STRING;
		r->len = val_len >= 0 ? val_len : strlen(str);
		memcpy(r->val.str, str, r->len);
	} else {
		r->pwtype = PW_TYPE_INTEGER;
		r->len = val_len > 0 ? val_len : 4;
		r->val.num = num;
	}
	r->len += 2;
	r->type = type;

	return r;
}

/*
 *	Build a request.
 */
int rad_buildreq(char *buf, int bufsz, RADPKT *pkt)
{
	char *p = buf;
	ATTR *a;
	int attr_len = 0;
	int tot_len;
	int tm;
	time_t now;

	/*
	 *	Find out how much space we need for the value pairs.
	 */
	for(a = pkt->list; a; a = a->next)
		attr_len += a->len;
	tot_len = attr_len + AUTH_HDR_LEN;
	if (pkt->timestamp) tot_len += 6;	
	if (tot_len > bufsz) {
		nsyslog(LOG_ERR, "rad_buildreq: buffer overflow\n");
		return -1;
	}
	memset(p, 0, tot_len);

	/*
	 *	See if we're going to use a new timestamp -
	 *	if so, we need a new ID.
	 */
	if (pkt->timestamp) {
		time(&now);
		if (pkt->lastsent != now) {
			pkt->id = rad_id();
			pkt->lastsent = now;
		}
	}

	/*
	 *	Fill in header fields.
	 */
	*p++ = pkt->code;
	*p++ = pkt->id;
	*(unsigned short *)p = htons(tot_len);
	p += 2;
	memcpy(p, pkt->vector, AUTH_VECTOR_LEN);
	p += AUTH_VECTOR_LEN;

	/*
	 *	Build complete request.
	 */
	for(a = pkt->list; a; a = a->next) {
		p[0] = a->type;
		p[1] = a->len;
		if (a->pwtype == PW_TYPE_STRING)
			memcpy(p+2, a->val.str, a->len - 2);
		else
			memcpy(p+2, &(a->val.num), 4);
		p += a->len;
	}

	/*
	 *	And add a time stamp.
	 */
	if (pkt->timestamp) {
		*p++ = PW_ACCT_DELAY_TIME;
		*p++ = 6;
		tm = htonl(now - pkt->timestamp);
		memcpy(p, &tm, 4);
	}

	return tot_len;
}

/*
 *	Free a list of attributes.
 */
void rad_attrfree(ATTR *l)
{
	ATTR *next;

	while (l != NULL) {
		next = l->next;
		free(l);
		l = next;
	}
}

/*
 *	Build an authentication request.
 */
RADPKT *rad_buildauth(struct auth *ai, int ppp)
{
	RADPKT *pkt;
	ATTR *list = NULL;
	char *secret_key;
	int secret_len;
	int pass_len;
	int padded_len;
	char  md5buf[256];
	char passtmp[256];
	int i, j;

	if ((pkt = malloc(sizeof(RADPKT))) == NULL)
		return NULL;
	memset(pkt, 0, sizeof(RADPKT));

	/*
	 *	Build the header.
	 */
	pkt->code = PW_AUTHENTICATION_REQUEST;
	pkt->id   = rad_id();
	rad_vector(pkt->vector);

	/*
	 *	Add the login name.
	 */
	if ((pkt->list = rad_addattr(NULL, PW_USER_NAME, -1, 0, ai->login)) == NULL)
	{
		free(pkt);
		return NULL;
	}

	/*
	 *	And the password - this one is hard.
	 */
	pass_len = strlen(ai->passwd); 
	memset(passtmp, 0, sizeof(passtmp));
	secret_key = lineconf[ai->nasport].secret;
	secret_len = strlen(secret_key);
	strcpy(md5buf, secret_key);
	memcpy(md5buf + secret_len, pkt->vector, AUTH_VECTOR_LEN);
	radius_md5_calc(passtmp, md5buf, secret_len + AUTH_VECTOR_LEN);

	for(i = 0; ai->passwd[i] && i < AUTH_PASS_LEN; i++)
		passtmp[i] ^= ai->passwd[i];
	while((i < pass_len) && ai->passwd[i]) {
		memcpy(md5buf + secret_len,passtmp + i - AUTH_PASS_LEN,AUTH_PASS_LEN);
		radius_md5_calc(passtmp + i,md5buf,secret_len + AUTH_PASS_LEN);
		for (j = AUTH_PASS_LEN;j && ai->passwd [i];--j,++i)
			passtmp [i] ^= ai->passwd [i];
	}
	//nsyslog(LOG_DEBUG, "passwd/secret: %s/%s", ai->passwd, secret_key);
	padded_len = (pass_len+(AUTH_VECTOR_LEN-1)) & ~(AUTH_VECTOR_LEN-1);
	
	if (padded_len == 0)
		padded_len = AUTH_VECTOR_LEN;
		
	if (rad_addattr(pkt->list, PW_PASSWORD, padded_len, 0, passtmp) == NULL) {
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}

	/*
	 *	Now we finish off with the client id (our ip address),
	 *	and the port id (tty number).
	 */
	if (rad_addattr(pkt->list, PW_NAS_IP_ADDRESS, 0, mainconf.ipno, NULL) == NULL) {
		rad_attrfree(list);
		free(pkt);
		return NULL;
	}
	if (rad_addattr(pkt->list, PW_NAS_PORT_ID, 0, htonl(ai->nasport + 1), NULL) == NULL) {
		rad_attrfree(list);
		free(pkt);
		return NULL;
	}
	if (rad_addattr(pkt->list, PW_NAS_PORT_TYPE, 0, htonl(ai->porttype), NULL) == NULL) {
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}

	/*
	 *	Add connection info if it is available.
	 */
	if (ai->conn_info[0]) {
		if (rad_addattr(pkt->list, PW_CONNECT_INFO, -1, 0, ai->conn_info) == NULL) {
			rad_attrfree(pkt->list);
			free(pkt);
			return NULL;
		}
	}


	/*
	 *	We add the protocol type if this is PPP.
	 */
	if (ppp &&
	rad_addattr(pkt->list, PW_FRAMED_PROTOCOL, 0, htonl(PW_PPP), NULL) == NULL) {
		rad_attrfree(list);
		free(pkt);
		return NULL;
	}

	if (ppp &&
	rad_addattr(pkt->list, PW_SERVICE_TYPE, 0, htonl(PW_FRAMED_USER), NULL) == NULL) {
		rad_attrfree(list);
		free(pkt);
		return NULL;
	}

	if (ai->len_state &&
		rad_addattr(pkt->list, PW_STATE, ai->len_state, 0, ai->state_info) == NULL) {
		rad_attrfree(list);
		free(pkt);
		return NULL;
	}

	return pkt;
}

/*
 *	Build an accounting request.
 */
RADPKT *rad_buildacct(struct auth *ai, int islogin)
{
	RADPKT *pkt;
	int i, s, p, c;
	unsigned int h, a;

	if ((pkt = malloc(sizeof(RADPKT))) == NULL)
		return NULL;
	memset(pkt, 0, sizeof(RADPKT));

	/*
	 *	Build the header.
	 */
	pkt->code = PW_ACCOUNTING_REQUEST;;
	pkt->id   = rad_id();
	memset(pkt->vector, 0, AUTH_VECTOR_LEN);
	strcpy(pkt->secret,lineconf[ai->nasport].secret);

	pkt->timestamp = time(NULL);

	/*
	 *	Tell the server what kind of request this is.
	 */
	i = islogin ? PW_STATUS_START : PW_STATUS_STOP;
	if ((pkt->list = rad_addattr(pkt->list, PW_ACCT_STATUS_TYPE, 0, htonl(i), NULL)) == NULL) {
		free(pkt);
		return NULL;
	}

	/*
	 *	Add the login name.
	 */
	if (ai->login && ai->login[0]) {
		if (rad_addattr(pkt->list, PW_USER_NAME, -1, 0, ai->login) == NULL) {
			rad_attrfree(pkt->list);
			free(pkt);
			return NULL;
		}
	}

	/*
	 *	Now we add the client id (our ip address),
	 *	and the port id (tty number).
	 */
	if (rad_addattr(pkt->list, PW_NAS_IP_ADDRESS, 0, mainconf.ipno, NULL) == NULL) {
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}

	if (rad_addattr(pkt->list, PW_NAS_PORT_ID, 0, htonl(ai->nasport + 1), NULL) == NULL) {
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}
	if (rad_addattr(pkt->list, PW_NAS_PORT_TYPE, 0, htonl(ai->porttype), NULL) == NULL) {
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}

	/*
	 *	Add input and output octets.
	 */
	if ((!islogin && (ai->sent_bytes || ai->recv_bytes)) &&
	     rad_addattr(pkt->list,
	     PW_ACCT_OUTPUT_OCTETS, 0, htonl(ai->sent_bytes), NULL) == NULL) {
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}
	if ((!islogin && (ai->sent_bytes || ai->recv_bytes)) &&
	     rad_addattr(pkt->list,
	     PW_ACCT_INPUT_OCTETS, 0, htonl(ai->recv_bytes), NULL) == NULL) {
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}

	/*
	 *	We have to add a unique identifier, the same
	 *	for login as for logout.
	 */
	if (rad_addattr(pkt->list, PW_ACCT_SESSION_ID, -1, 0, ai->session) == NULL) {
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}

	/*
	 *	Add connection info on login AND logout. That's what
	 *	a portmaster does too..
	 */
	if (ai->conn_info[0]) {
		if (rad_addattr(pkt->list, PW_CONNECT_INFO, -1, 0, ai->conn_info) == NULL) {
			rad_attrfree(pkt->list);
			free(pkt);
			return NULL;
		}
	}

	/*
	 *	Include session time if this is a logout.
	 */
	if (!islogin &&
	    rad_addattr(pkt->list, PW_ACCT_SESSION_TIME, 0,
	    htonl(time(NULL) - ai->start), NULL) == NULL) {
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}

	/*
	 *	We'll have to add some more fields here:
	 *	- service type
	 *	- login service
	 *	- framed protocol
	 *	- framed IP address
	 *	- framed compression
	 *	- login IP host
	 *
	 */
	i = -1;
	s = -1;
	p = -1;
	c = -1;
	h = 0;
	a = 0;
	switch(ai->proto) {
		case P_SHELL:
			s = PW_SHELL_USER;
			break;
		case P_TELNET:
			s = PW_LOGIN_USER;
			i = PW_TELNET;
			//h = ai->address; // IPv6 support - changed format of IP addrs
			// TODO - review Radius protocol for IPv6
			h =	((struct sockaddr_in *)(&ai->address))->sin_addr.s_addr;
			break;
		case P_RLOGIN:
			s = PW_LOGIN_USER;
			i = PW_RLOGIN;
			//h = ai->address; // IPv6 support - changed format of IP addrs
			// TODO - review Radius protocol for IPv6
			h =	((struct sockaddr_in *)(&ai->address))->sin_addr.s_addr;
			break;
		case P_SSH:
		case P_SSH2:
			s = PW_LOGIN_USER;
			i = PW_SSH;
			//h = ai->address; // IPv6 support - changed format of IP addrs
			// TODO - review Radius protocol for IPv6
			h =	((struct sockaddr_in *)(&ai->address))->sin_addr.s_addr;
			break;			
		case P_TCPCLEAR:
		case P_TCPLOGIN:
			s = PW_LOGIN_USER;
			i = PW_TCP_CLEAR;
			//h = ai->address; // IPv6 support - changed format of IP addrs
			// TODO - review Radius protocol for IPv6
			h =	((struct sockaddr_in *)(&ai->address))->sin_addr.s_addr;
			break;
		case P_PPP:
		case P_PPP_ONLY:
		case P_SLIP:
		case P_CSLIP:
			s = PW_FRAMED_USER;
			//a = ai->address; // IPv6 support - changed format of IP addrs
			// TODO - review Radius protocol for IPv6
			a =	((struct sockaddr_in *)(&ai->address))->sin_addr.s_addr;
			break;
	}
	switch(ai->proto) {
		case P_PPP:
		case P_PPP_ONLY:
			p = PW_PPP;
			c = PW_VAN_JACOBSEN_TCP_IP;
			break;
		case P_SLIP:
			p = PW_SLIP;
			break;
		case P_CSLIP:
			p = PW_SLIP;
			c = PW_VAN_JACOBSEN_TCP_IP;
	}
	if (s > 0 && rad_addattr(pkt->list, PW_SERVICE_TYPE, 0, htonl(s), NULL) == NULL) {
buildacct_error:
		rad_attrfree(pkt->list);
		free(pkt);
		return NULL;
	}
	if (i >= 0 && rad_addattr(pkt->list, PW_LOGIN_SERVICE, 0, htonl(i), NULL) == NULL)
		goto buildacct_error;

	if (p >= 0 && rad_addattr(pkt->list, PW_FRAMED_PROTOCOL, 0, htonl(p), NULL) == NULL)
		goto buildacct_error;

	if (a > 0 && rad_addattr(pkt->list, PW_FRAMED_IP_ADDRESS, 0, a, NULL) == NULL)
		goto buildacct_error;

	if (c >= 0 && rad_addattr(pkt->list, PW_FRAMED_COMPRESSION, 0, htonl(c), NULL) == NULL)
		goto buildacct_error;

	if (h > 0 && rad_addattr(pkt->list, PW_LOGIN_IP_HOST, 0, h, NULL) == NULL)
		goto buildacct_error;

	return pkt;
}

#define DEFAULT_TRIES	5

static int rad_send(int port, unsigned int host1, unsigned int host2, int try,
	time_t timeout,	char *recvbuf, int size, RADPKT *req, int acctpkt)
{
	char sendbuf[4096];
	int sendlen;
	struct sockaddr_in salocal, saremote, sa_replied;
	int local_port;
	int s, len, i;
	unsigned int remote;
	int ret;
	struct pollfd read_fd;

	if (try == 0) {
		try = DEFAULT_TRIES;
	}

	sendlen = rad_buildreq(sendbuf, sizeof(sendbuf), req);
	if (sendlen < 0) {
		return (-2);
	}

	/* if an acct pkt, create the authenticator */

	if (acctpkt == 1) {
		i = strlen(req->secret);
		memcpy(&sendbuf[sendlen],req->secret,i);
		radius_md5_calc(&sendbuf[4],sendbuf,sendlen+i);
	}

	/*
	 *	We have to bind the socket in order to
	 *	receive an answer back...
	 */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		nsyslog(LOG_ERR, "socket: %m");
		return -2;
	}

	memset(&salocal, 0, sizeof(salocal));
	salocal.sin_family = AF_INET;
	salocal.sin_addr.s_addr = INADDR_ANY;
	local_port = 1025;
	do {
		local_port++;
		salocal.sin_port = htons((unsigned short)local_port);
	} while ((bind(s, (struct sockaddr *)&salocal, sizeof (salocal)) < 0)
			&& local_port < 64000);

	if (local_port >= 64000) {
		close(s);
		nsyslog(LOG_ERR, "bind: %m");
		return -2;
	}

	/*
	 *	saremote is the remote radius server address.
	 */
	memset(&saremote, 0, sizeof(saremote));
	saremote.sin_family = AF_INET;
	saremote.sin_port = htons(port);

	if (!host1 && host2) {
		host1 = host2;
		host2 = 0;
	}
	remote = host1;
	/*
	 *	Try at most 10 times. We time out after specified number of
	 *	seconds, and after 3 tries we switch to an alternate server,
	 *	and keep switching after every timeout.
	 */
	for(i = 0; ; i++) {
		if (i >= try) {
			nsyslog(LOG_INFO, "radius@%s:%d not responding",
				dotted(remote), port);
			if (host2 && remote != host2) {
				remote = host2;
				i = 0;
			} else {
				break;
			}
		}

		/*
		 *	Connect to the radius server, and
		 *	send the request.
		 */
		saremote.sin_addr.s_addr = remote;
		sendto(s, sendbuf, sendlen, 0,
			(struct sockaddr *)&saremote, sizeof(saremote));

		/*
		 *	Now wait for an answer.
		 */
		read_fd.fd = s;
		read_fd.events = POLLIN;
		read_fd.revents = 0;
		switch (ret = poll (&read_fd, 1, timeout * 1000)) {
			case -1:
				nsyslog (LOG_DEBUG, "rad_send: poll: %d(%m)", errno);
				close(s);
				return (-1);
			case 0:
				nsyslog (LOG_DEBUG, "radius@%s:%d timed out in poll",
					dotted(remote), port);
				break;
			case 1:
				break;
			default:
				nsyslog (LOG_DEBUG, "rad_send: poll returned %d", ret);
				close(s);
				return (-1);
		}
		if (ret == 1) {
			len = sizeof(saremote);
			ret = recvfrom (s, recvbuf, size, 0, (struct sockaddr *) &sa_replied, &len);
			if (ret == -1) {
				nsyslog (LOG_DEBUG, "rad_send: recvfrom: %d(%m)", errno);
			} else {
				if (ret == 0) {
					nsyslog (LOG_DEBUG, "rad_send: recvfrom: read 0 bytes");
				} else {
					break;
				}
			}
		}
	}
	close(s);

	if (i >= try) {
		if (host2) {
			i = snprintf(sendbuf, sizeof (sendbuf),
				"Radius servers %s and ",
				dotted(host1));
			snprintf(&sendbuf[i], sizeof (sendbuf) - i,
				"%s not responding",
				dotted(host2));
		} else {
			snprintf(sendbuf, sizeof (sendbuf),
				"Radius server %s not responding", dotted(host1));
		}
		nsyslog(LOG_ERR, sendbuf);
		return -1;
	}

	return len;
}

/*
 *	Ask a radius server to authenticate us,
 */
int rad_client(struct auth *ai, int ppp)
{
	int ret, len;
	int islogin = P_IDLE;
	int isframed = 0;
	AUTH_HDR *reply;
	RADPKT *req;
	unsigned int a_len, a_type;
	unsigned char *a_val;
	char recvbuf[4096];
	char *ptr = recvbuf, *maxptr;
	int oldproto;
	int auth_port;
	struct servent *svp;
	struct realm_def *r;
	unsigned int realm_authhost1 = 0, realm_authhost2 = 0;

	if ((ai->passwd [0] == '\0') && (lineconf[ai->nasport].radnullpass == 0)) {
		return (-2);
	}

	//Get radauth port, or use default
	svp = getservbyname ("radius", "udp");
	if (svp == (struct servent *) 0)
		auth_port = PW_AUTH_UDP_PORT;
	else
		auth_port = ntohs(svp->s_port);
	
	/*
	 *	Set the message to some error in case we fail.
	 */
	strcpy(ai->message[0], "Internal error\r\n");

        /*
         *      First, build the request.
         */
	if ((req = rad_buildauth(ai, ppp)) == NULL) 
		return -1;

	/*
	 *	Now connect to the server.
	 */
	if ((r = ckrealm(ai)) != NULL) {
		realm_authhost1 = r->authhost1;
		realm_authhost2 = r->authhost2;
	} else {
		realm_authhost1 = lineconf[ai->nasport].authhost1;
		realm_authhost2 = lineconf[ai->nasport].authhost2;
	}

	len = rad_send(auth_port, realm_authhost1, realm_authhost2,
			lineconf[ai->nasport].retries,
			lineconf[ai->nasport].timeout,
			recvbuf, sizeof(recvbuf), req,0);
			
	rad_attrfree(req->list);
	free(req);

	if (len < 0) {
		if (len == -1) strcpy(ai->message[0],
			"RADIUS server not responding.\r\n");
		return -1;
	}

	reply = (AUTH_HDR *)ptr;
	nsyslog(LOG_DEBUG, "Got a reply, code %d, length %d", reply->code, ntohs(reply->length));
	switch (reply->code) {
	case PW_AUTHENTICATION_ACK:
		ret = 0;
		break;
	case PW_ACCESS_CHALLENGE:
		ai->next_token = 1;
		ret = 0;
		break;
	default:
		ret = -2;
		break;
	}
	ai->message[0][0] = 0;
	ai->filterid[0][0] = 0;
	oldproto = ai->proto;
	ai->proto = 0;

#ifdef CBCPenable
#ifdef CYCLADES
	ai->sess_type = 0;	/* normal session */
	ai->callback_number[0] = 0;	
#endif
#endif
	
	/*
	 *	Put the reply into our auth struct.
	 */
	maxptr = ptr + ntohs(reply->length);
	ptr += 20;

	while(ptr < maxptr) {
		a_type = ptr[0];
		a_len = ptr[1];
		a_val = &ptr[2];
		ptr += a_len;
#if 0 /* XXX - Debug */
		nsyslog(LOG_DEBUG, "Attribute type %d, length %d", a_type, a_len);
#endif
		if (a_type == 0 || a_len < 2) break;
		a_len -= 2;
		switch(a_type) {
			case PW_SERVICE_TYPE:
				/* Framed or login. */
				switch(MSB_UINT4(a_val)) {
					case PW_SHELL_USER:
						ai->proto = P_SHELL;
						break;
					case PW_LOGIN_USER:
						islogin = P_LOGIN;
						break;
					case PW_FRAMED_USER:
						isframed = P_LOGIN;
						break;
#ifdef CBCPenable
#ifdef CYCLADES
					case PW_DIALBACK_FRAMED_USER:
						isframed = P_LOGIN;
						ai->sess_type = 1;	/* callback session */
						break;
#endif
#endif
					default:
						break;
				}
				break;
			case PW_LOGIN_SERVICE:
				islogin = P_LOGIN;
				switch(MSB_UINT4(a_val)) {
					case PW_TELNET:
						ai->proto = P_TELNET;
						break;
					case PW_RLOGIN:
						ai->proto = P_RLOGIN;
						break;
					case PW_SSH:
						ai->proto = P_SSH;
						break;						
					case PW_TCP_CLEAR:
						ai->proto = P_TCPCLEAR;
						break;
					case PW_PORTMASTER:
					default:
						islogin = P_IDLE;
						/* Unknown to us. */
						break;
				}
				break;
			case PW_LOGIN_IP_HOST:
				// IPv6 support - changed format of IP addrs
				// TODO - review Radius protocol for IPv6
				//ai->address = htonl(MSB_UINT4(a_val));
				((struct sockaddr_in *)(&ai->address))->sin_addr.s_addr = htonl(MSB_UINT4(a_val));
				((struct sockaddr_in *)(&ai->address))->sin_family = AF_INET;
				//ai->host = ai->address; 
				memcpy(&ai->host, &ai->address, sizeof(ai->host));
				break;
			case PW_LOGIN_PORT:
				ai->loginport = MSB_UINT4(a_val);
				break;
			case PW_FRAMED_IP_ADDRESS:
				isframed = 1;
				if (MSB_UINT4(a_val) != 0xFFFFFFFE) {
					// IPv6 support - changed format of IP addrs
					// TODO - review Radius protocol for IPv6
					//ai->address = htonl (MSB_UINT4 (a_val));
					((struct sockaddr_in *)(&ai->address))->sin_addr.s_addr = htonl(MSB_UINT4(a_val));
					((struct sockaddr_in *)(&ai->address))->sin_family = AF_INET;
					//ai->host = ai->address;
					memcpy(&ai->host, &ai->address, sizeof(ai->host));
				}
				break;
			case PW_FRAMED_IP_NETMASK:
				ai->netmask = htonl (MSB_UINT4 (a_val));
				break;
			case PW_FRAMED_MTU:
				ai->mtu = MSB_UINT4 (a_val);
				break;
			case PW_IDLE_TIMEOUT:
				ai->idletime = MSB_UINT4 (a_val);
				break;
			case PW_SESSION_TIMEOUT:
				ai->sessiontime = MSB_UINT4 (a_val);
				break;				
			case PW_FRAMED_COMPRESSION:
				if (MSB_UINT4 (a_val) != PW_VAN_JACOBSEN_TCP_IP)
					break;
				if (ai->proto == 0 || ai->proto == P_SLIP)
					ai->proto = P_CSLIP;
				break;
			case PW_FRAMED_PROTOCOL:
				isframed = 1;
				if (MSB_UINT4 (a_val) == PW_PPP)
					ai->proto = P_PPP;
				else if (ai->proto == 0)
					ai->proto = P_SLIP;
				break;
			case PW_FILTER_ID:
				if (ai->fln > (sizeof(ai->filterid) / sizeof(ai->filterid[0])))
					break;			
				len = a_len;
				if (len >= sizeof(ai->filterid[ai->fln]))
					len = sizeof(ai->filterid[ai->fln]) - 1;
				memcpy(ai->filterid[ai->fln], a_val, len);
				ai->filterid[ai->fln][len] = 0;
				ai->fln++;
				break;
			case PW_FRAMED_ROUTE:
				if (ai->frn > (sizeof(ai->framed_route) / sizeof(ai->framed_route[0])))
					break;
				len = a_len;
				if (len >= sizeof(ai->framed_route[ai->frn]))
					len = sizeof(ai->framed_route[ai->frn]) - 1;
				memcpy(ai->framed_route[ai->frn], a_val, len);
				ai->framed_route[ai->frn][len] = 0;
				ai->frn++;
				break;				
			case PW_REPLY_MESSAGE:
				if (ai->msn > (sizeof(ai->message) / sizeof(ai->message[0])))
					break;
				len = a_len;
				if (len >= sizeof(ai->message[ai->msn]))
					len = sizeof(ai->message[ai->msn]) - 1;
				memcpy(ai->message[ai->msn], a_val, len);
				ai->message[ai->msn][len] = 0;
				ai->msn++;
				break;
			case PW_STATE:
				len = a_len;
				if (len >= sizeof(ai->state_info))
					len = sizeof(ai->state_info) - 1;
				memcpy(ai->state_info, a_val, len);
				ai->len_state = len;
				ai->state_info[len] = 0;
				break;
#ifdef CBCPenable
#ifdef CYCLADES				
			case PW_CALLBACK_NUMBER:
				memcpy(ai->callback_number, a_val, a_len);
				ai->callback_number[a_len] = 0;
				break;
#endif
#endif
		}
	}

	if (isframed && 
		(ai->address.ss_family == 0) && 
		lineconf[ai->nasport].ipno) {
		// IPv6 support - changed format of IP addrs
		//ai->address = lineconf[ai->nasport].ipno;
		//TODO - ipno will also be a sockaddr_storage
		memcpy(&ai->address, &lineconf[ai->nasport].ipno, sizeof(ai->address));
	}

#ifdef IPv6enable
	if (islogin && 
		(ai->address.ss_family == 0) && 
		(lineconf[ai->nasport].host.ss_family)) {
		// IPv6 support - changed format of IP addrs
		//ai->address = lineconf[ai->nasport].host;
		memcpy(&ai->address, &lineconf[ai->nasport].host, sizeof(ai->address));
	}
#else
    if (islogin && ai->address.ss_family == 0 && lineconf[ai->nasport].host)
		memcpy(&ai->address, &lineconf[ai->nasport].host, sizeof(lineconf[ai->nasport].host));
#endif

	if (ai->proto == 0)
		ai->proto = oldproto;

	if (ret == 0) {
		ai->authenticated = 2;
		rad_sessionid(ai->session);
		ai->start = time(NULL);
	} else {
		nsyslog(LOG_INFO, "authentication failed for (%s) %s",
			ai->login, ai->message[0]);
	}

	return ret;
}

/*
 *	Send accounting info to a RADIUS server.
 */
int rad_acct(struct auth *ai, int islogin)
{
	int len;
	AUTH_HDR *reply;
	RADPKT *req;
	char recvbuf[4096];
	char *ptr = recvbuf;
	int acct_port;
	struct servent *svp;
	struct realm_def *r;
	unsigned int realm_accthost1 = 0, realm_accthost2 = 0;

	//Get radacct port, or use default	
	svp = getservbyname ("radacct", "udp");
	if (svp == (struct servent *) 0)
		acct_port = PW_ACCT_UDP_PORT;
	else
		acct_port = ntohs(svp->s_port);

	/*
	 *	Update utmp/wtmp (ctlportslave)
	 */
	if (lineconf[ai->nasport].sysutmp) {
		update_utmp("%L", lineconf [ai->nasport].utmp_from, ai, lineconf[ai->nasport].syswtmp);
	}
	
	/*
	 *	Update Framed-Routes
	 */
	update_framed_route(ai, islogin);

	/*
	 *	Run filter backend
	 */
	update_filter_id(ai, islogin);
	
        /*
         *      First, build the request.
         */
	if ((req = rad_buildacct(ai, islogin)) == NULL)
		return -1;

	/*
	 *	Now connect to the server.
	 */
	if ((r = ckrealm(ai)) != NULL) {
		realm_accthost1 = r->accthost1;
		realm_accthost2 = r->accthost2;
	} else {
		realm_accthost1 = lineconf[ai->nasport].accthost1;
		realm_accthost2 = lineconf[ai->nasport].accthost2;
	}

	if (!realm_accthost1 && !realm_accthost2) {
		return(0);
	}

	len = rad_send(acct_port, realm_accthost1, realm_accthost2,
			lineconf[ai->nasport].retries,
			lineconf[ai->nasport].timeout,
			recvbuf, sizeof(recvbuf), req,1);
			
	rad_attrfree(req->list);
	free(req);

	if (len < 0)
		return -1;

	reply = (AUTH_HDR *)ptr;
	nsyslog(LOG_DEBUG, "Got a reply, code %d, length %d", reply->code, ntohs(reply->length));

	return 0;
}


/*
 *     Check for realm in the login name.
 */
struct realm_def *ckrealm(struct auth *ai) {
	struct realm_def *r;
	int l, n;

	l = strlen(ai->login);
	for (r = lineconf[ai->nasport].realm; r != NULL; r = r->next) {
		n = strlen(r->name);
		if (r->prefix && strncmp(ai->login, r->name, n) == 0)
			break;
		
		if (!r->prefix) {
			if (l < n)	continue;
			if (strcmp(ai->login + l - n, r->name) == 0)	break;
		}
	}		
	
	return r;
}

/*
 * Invoke 'route' command to update "Framed-Route".
 */
int update_framed_route(struct auth *ai, int islogin)
{
	char cmd[1024], buf[sizeof(ai->framed_route[0])];
	char *net, *gw, *metric;
	int x;

	if (ai->frn == 0) return 0;
	
	if (islogin) {
		nsyslog(LOG_INFO, "Adding routes: %d.", ai->frn);
		x = 0;
	} else {
		nsyslog(LOG_INFO, "Deleting routes: %d.", ai->frn);	
		x = ai->frn - 1;
	}
		
	while (x < ai->frn && x >= 0) {	
		memcpy(buf, ai->framed_route[x], sizeof(buf));
		net = strtok (buf, " ");
		gw = strtok ((char *) 0, " ");
		metric = strtok ((char *) 0, " ");
							
		snprintf(cmd, sizeof(cmd) - 1, "exec %s %s -net %s gw %s metric %s >/dev/null 2>&1",
			PATH_ROUTE, islogin ? "add" : "del", net, gw, metric);
		system(cmd);
		
		islogin ? x++ : x--;		//FIFO add, LIFO del
	} 

	return 0;
}

/*
 * Invoke the filter script defined by "Framed-Filter-Id".
 */
int update_filter_id(struct auth *ai, int islogin)
{
	char cmd[1024];
	int x;
	
	if (ai->fln == 0) return 0;
	
	if (islogin) {
		nsyslog(LOG_INFO, "Starting filters: %d.", ai->fln);
		x = 0;
	} else {
		nsyslog(LOG_INFO, "Stopping filters: %d.", ai->fln);
		x = ai->fln - 1;
	}
		//Format: path/script <start:stop> <remote ip> <local ip> <remote netmask>
	while (x < ai->fln && x >= 0) {	
		snprintf(cmd, sizeof(cmd)-1,"exec %s/%s %s %s %s %s >/dev/null 2>&1",
			mainconf.filterdir, ai->filterid[x], islogin ? "start" : "stop",
			strdup(dotted(((struct sockaddr_in *)(&ai->address))->sin_addr.s_addr)), 
			strdup(dotted(ai->localip)), strdup(dotted(ai->netmask)));
		//TODO - check necessary IPv6 changes here
		system(cmd);

		islogin ? x++ : x--;		//FIFO add, LIFO del
	}

	return 0;
}
