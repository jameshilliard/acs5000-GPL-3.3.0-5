/*
 * cbcp - Call Back Configuration Protocol.
 *
 * Copyright (c) 1995 Pedro Roque Marques.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names of the authors of this software must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Pedro Roque Marques
 *     <pedro_m@yahoo.com>"
 *
 * THE AUTHORS OF THIS SOFTWARE DISCLAIM ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define RCSID	"$Id: cbcp.c,v 1.2 2006/02/27 20:22:33 livio Exp $"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

#include "pppd.h"
#include "cbcp.h"
#include "fsm.h"
#include "lcp.h"

static const char rcsid[] = RCSID;

/*
 * Options.
 */
static int setcbcp __P((char **));
#ifdef CBCP_SERVER_SUPPORT
static void setservercbcp __P((char **));
#endif

static option_t cbcp_option_list[] = {
    { "callback", o_special, setcbcp,
      "Ask for callback", OPT_PRIO | OPT_A2STRVAL, &cbcp[0].us_number },
#ifdef CBCP_SERVER_SUPPORT
 	{ "cb-script", o_special, setservercbcp, 
		"Set pathname of callback script"},
#endif
   { NULL }
};

/*
 * Protocol entry points.
 */
static void cbcp_init      __P((int unit));
static void cbcp_open      __P((int unit));
static void cbcp_lowerup   __P((int unit));
static void cbcp_input     __P((int unit, u_char *pkt, int len));
static void cbcp_protrej   __P((int unit));
static int  cbcp_printpkt  __P((u_char *pkt, int len,
				void (*printer) __P((void *, char *, ...)),
				void *arg));
#ifdef CBCP_SERVER_SUPPORT
static void cbcp_lowerdown   __P((int unit));
static int cbcp_mount_data(cbcp_state *us, u_char *bufp, u_char cb_type);
static void cbcp_sendreq(cbcp_state *us);
static void cbcp_sendack(cbcp_state *us);
static void cbcp_recvresp(cbcp_state *us, char *pckt, int pcktlen);
static void cbcp_reqtmo(void * usv);
#endif

struct protent cbcp_protent = {
    PPP_CBCP,
    cbcp_init,
    cbcp_input,
    cbcp_protrej,
    cbcp_lowerup,
#ifdef CBCP_SERVER_SUPPORT
	cbcp_lowerdown,
#else
    NULL,
#endif
    cbcp_open,
    NULL,
    cbcp_printpkt,
    NULL,
    0,
    "CBCP",
    NULL,
    cbcp_option_list,
    NULL,
    NULL,
    NULL
};

cbcp_state cbcp[NUM_PPP];	

/* internal prototypes */

static void cbcp_recvreq __P((cbcp_state *us, u_char *pckt, int len));
static void cbcp_resp __P((cbcp_state *us));
static void cbcp_up __P((cbcp_state *us));
static void cbcp_recvack __P((cbcp_state *us, u_char *pckt, int len));
static void cbcp_send __P((cbcp_state *us, int code, u_char *buf, int len));

/* option processing */
#ifdef CBCP_SERVER_SUPPORT
static void setservercbcp (char **argv)
{
    lcp_allowoptions[0].neg_cbcp = 1;
    cbcp[0].us_cbscript = strdup(*argv);
}
#endif

static int
setcbcp(argv)
    char **argv;
{
    lcp_wantoptions[0].neg_cbcp = 1;
	
#ifdef CBCP_SERVER_SUPPORT
	cbcp[0].us_cbcp = 0;
#endif

    cbcp_protent.enabled_flag = 1;
    cbcp[0].us_number = strdup(*argv);
    if (cbcp[0].us_number == 0)
	novm("callback number");
    cbcp[0].us_type |= (1 << CB_CONF_USER);
    cbcp[0].us_type |= (1 << CB_CONF_ADMIN);
    return (1);
}

/* init state */
static void
cbcp_init(iface)
    int iface;
{
    cbcp_state *us;

	dbglog("cbcp_int");
    us = &cbcp[iface];
    memset(us, 0, sizeof(cbcp_state));
    us->us_unit = iface;
    us->us_type |= (1 << CB_CONF_NO);

#ifdef CBCP_SERVER_SUPPORT
	us->us_delay = 5;
	us->us_cbscript = 0;
#endif
	
}

/* lower layer is up */
static void
cbcp_lowerup(iface)
    int iface;
{
    cbcp_state *us = &cbcp[iface];

    dbglog("cbcp_lowerup");
    dbglog("want: %d", us->us_type);

    if (us->us_type & (1 << CB_CONF_USER))
        dbglog("phone no: %s", us->us_number);

#ifdef CBCP_SERVER_SUPPORT
	if (lcp_hisoptions[0].neg_cbcp) {	/* callback negociated in LCP */
		cbcp_protent.enabled_flag = 1;
		cbcp[0].us_cbcp = 1;	/* callback server */
		dbglog(" his:neg_cbcp ");
		/* callback server */
		if (cbcp[0].us_cbscript == 0) {
			/* it doesn't have callback script, so stop callback */ 
			lcp_hisoptions[0].neg_cbcp = 0;
		} else {
			dbglog(" us_cbscript OK");
			cbcp[0].us_number = 0;
			cbcp[0].us_type = 1 << CB_CONF_NO;
		}
	}
#endif
}

static void
cbcp_open(unit)
    int unit;
{
    dbglog("cbcp_open");

#ifdef CBCP_SERVER_SUPPORT
	if (cbcp[unit].us_cbcp) { /* callback server */
		dbglog(" us_cbcp ok");
		cbcp_sendreq(&cbcp[unit]);	/* send CBCP request */
	}
#endif
}

/* process an incomming packet */
static void
cbcp_input(unit, inpacket, pktlen)
    int unit;
    u_char *inpacket;
    int pktlen;
{
    u_char *inp;
    u_char code, id;
    u_short len;

    cbcp_state *us = &cbcp[unit];

    inp = inpacket;

	dbglog("cbcp_input");

    if (pktlen < CBCP_MINLEN) {
        error("CBCP packet is too small");
	return;
    }

    GETCHAR(code, inp);
    GETCHAR(id, inp);
    GETSHORT(len, inp);

//????#if 0
    if (len > pktlen || len < CBCP_MINLEN) {
	if (debug)
            dbglog("CBCP packet: invalid length %d", len);
        return;
    }
//????#endif

    len -= CBCP_MINLEN;
 
    switch(code) {
    case CBCP_REQ:
#ifdef CBCP_SERVER_SUPPORT
	if (us->us_cbcp) { /* callback server */
		dbglog("CBCP_REQ received");
		break;
	}
#endif
	us->us_id = id;
	cbcp_recvreq(us, inp, len);
	break;

    case CBCP_RESP:
#ifdef CBCP_SERVER_SUPPORT
	if (us->us_cbcp) { /* callback server */
		if (debug && (id != us->us_id)) {
			dbglog("CBCP_RESP : id doesn't match : expected %d recev %d", us->us_id, id);
		}
		cbcp_recvresp(us,inp,len);
		break;
	}
#endif
	if (debug)
	    dbglog("CBCP_RESP received");
	break;

    case CBCP_ACK:
#ifdef CBCP_SERVER_SUPPORT
	if (us->us_cbcp) { /* callback server */
		if (debug)
		    dbglog("CBCP_ACK received");
		break;
	}
#endif
	if (debug && id != us->us_id)
	    dbglog("id doesn't match: expected %d recv %d",
		   us->us_id, id);

	cbcp_recvack(us, inp, len);
	break;

    default:
	break;
    }
}

/* protocol was rejected by foe */
void cbcp_protrej(int iface)
{
}

char *cbcp_codenames[] = {
    "Request", "Response", "Ack"
};

char *cbcp_optionnames[] = {
    "NoCallback",
    "UserDefined",
    "AdminDefined",
    "List"
};

/* pretty print a packet */
static int
cbcp_printpkt(p, plen, printer, arg)
    u_char *p;
    int plen;
    void (*printer) __P((void *, char *, ...));
    void *arg;
{
    int code, opt, id, len, olen, delay;
    u_char *pstart;

    if (plen < HEADERLEN)
	return 0;
    pstart = p;
    GETCHAR(code, p);
    GETCHAR(id, p);
    GETSHORT(len, p);
    if (len < HEADERLEN || len > plen)
	return 0;

    if (code >= 1 && code <= sizeof(cbcp_codenames) / sizeof(char *))
	printer(arg, " %s", cbcp_codenames[code-1]);
    else
	printer(arg, " code=0x%x", code); 

    printer(arg, " id=0x%x", id);
    len -= HEADERLEN;

    switch (code) {
    case CBCP_REQ:
    case CBCP_RESP:
    case CBCP_ACK:
        while(len >= 2) {
	    GETCHAR(opt, p);
	    GETCHAR(olen, p);

	    if (olen < 2 || olen > len) {
	        break;
	    }

	    printer(arg, " <");
	    len -= olen;

	    if (opt >= 1 && opt <= sizeof(cbcp_optionnames) / sizeof(char *))
	    	printer(arg, " %s", cbcp_optionnames[opt-1]);
	    else
	        printer(arg, " option=0x%x", opt); 

	    if (olen > 2) {
	        GETCHAR(delay, p);
		printer(arg, " delay = %d", delay);
	    }

	    if (olen > 3) {
	        int addrt;
		char str[256];

		GETCHAR(addrt, p);
		memcpy(str, p, olen - 4);
		str[olen - 4] = 0;
		printer(arg, " number = %s", str);
	    }
	    printer(arg, ">");
	    break;
	}

    default:
	break;
    }

    for (; len > 0; --len) {
	GETCHAR(code, p);
	printer(arg, " %.2x", code);
    }

    return p - pstart;
}

/* received CBCP request */
static void
cbcp_recvreq(us, pckt, pcktlen)
    cbcp_state *us;
    u_char *pckt;
    int pcktlen;
{
    u_char type, opt_len, addr_type;
#ifndef CBCP_SERVER_SUPPORT
    u_char delay;
#endif

    char address[256];
    int len = pcktlen;

    address[0] = 0;

    while (len >= 2) {
        dbglog("length: %d", len);

	GETCHAR(type, pckt);
	GETCHAR(opt_len, pckt);

	if (opt_len < 2 || opt_len > len)
	    break;

	if (opt_len > 2)
#ifdef CBCP_SERVER_SUPPORT
	    GETCHAR(us->us_delay, pckt);
#else
	    GETCHAR(delay, pckt);
#endif

	us->us_allowed |= (1 << type);

	switch(type) {
	case CB_CONF_NO:
	    dbglog("no callback allowed");
	    break;

	case CB_CONF_USER:
	    dbglog("user callback allowed");
	    if (opt_len > 4) {
	        GETCHAR(addr_type, pckt);
		memcpy(address, pckt, opt_len - 4);
		address[opt_len - 4] = 0;
		if (address[0])
		    dbglog("address: %s", address);
	    }
	    break;

	case CB_CONF_ADMIN:
	    dbglog("user admin defined allowed");
	    break;

	case CB_CONF_LIST:
	    break;
	}
	len -= opt_len;
    }
    if (len != 0) {
       if (debug)
           dbglog("cbcp_recvreq: malformed packet (%d bytes left)", len);
       return;
    }

    cbcp_resp(us);
}

#ifdef CBCP_SERVER_SUPPORT
static void
cbcp_resp(cbcp_state *us)
{
	u_char buf[256], cb_type;
	int len = 0;

	dbglog("cbcp_resp");
	cb_type = us->us_allowed & us->us_type;
	len = cbcp_mount_data(us, buf, cb_type);
	cbcp_send(us, CBCP_RESP, buf, len);

	if (cb_type & (1 << CB_CONF_NO)) {
		start_networks(us->us_unit);	/* no callback */
	}
}
#else
static void
cbcp_resp(us)
    cbcp_state *us;
{
    u_char cb_type;
    u_char buf[256];
    u_char *bufp = buf;
    int len = 0;
    int slen;

    cb_type = us->us_allowed & us->us_type;
    dbglog("cbcp_resp cb_type=%d", cb_type);

#if 0
    if (!cb_type)
        lcp_down(us->us_unit);
#endif

    if (cb_type & ( 1 << CB_CONF_USER ) ) {
	dbglog("cbcp_resp CONF_USER");
	slen = strlen(us->us_number);
	if (slen > 250) {
	    warn("callback number truncated to 250 characters");
	    slen = 250;
	}
	PUTCHAR(CB_CONF_USER, bufp);
	len = 3 + 1 + slen + 1;
	PUTCHAR(len , bufp);
	PUTCHAR(5, bufp); /* delay */
	PUTCHAR(1, bufp);
	BCOPY(us->us_number, bufp, slen + 1);
	cbcp_send(us, CBCP_RESP, buf, len);
	return;
    }

    if (cb_type & ( 1 << CB_CONF_ADMIN ) ) {
	dbglog("cbcp_resp CONF_ADMIN");
        PUTCHAR(CB_CONF_ADMIN, bufp);
	len = 3;
	PUTCHAR(len, bufp);
	PUTCHAR(5, bufp); /* delay */
	cbcp_send(us, CBCP_RESP, buf, len);
	return;
    }

    if (cb_type & ( 1 << CB_CONF_NO ) ) {
        dbglog("cbcp_resp CONF_NO");
	PUTCHAR(CB_CONF_NO, bufp);
	len = 2;
	PUTCHAR(len , bufp);
	cbcp_send(us, CBCP_RESP, buf, len);
	start_networks(us->us_unit);
	return;
    }
}
#endif

static void
cbcp_send(us, code, buf, len)
    cbcp_state *us;
    int code;
    u_char *buf;
    int len;
{
    u_char *outp;
    int outlen;

    outp = outpacket_buf;

    outlen = 4 + len;
    
    MAKEHEADER(outp, PPP_CBCP);

    PUTCHAR(code, outp);
    PUTCHAR(us->us_id, outp);
    PUTSHORT(outlen, outp);
    
    if (len)
        BCOPY(buf, outp, len);

    output(us->us_unit, outpacket_buf, outlen + PPP_HDRLEN);
}

static void
cbcp_recvack(us, pckt, len)
    cbcp_state *us;
    u_char *pckt;
    int len;
{
    u_char type, delay, addr_type;
    int opt_len;
    char address[256];

    if (len >= 2) {
        GETCHAR(type, pckt);
	GETCHAR(opt_len, pckt);
        if (opt_len >= 2 && opt_len <= len) {
		if (opt_len > 2)
		    GETCHAR(delay, pckt);

		if (opt_len > 4) {
	    		GETCHAR(addr_type, pckt);
	    		memcpy(address, pckt, opt_len - 4);
	    		address[opt_len - 4] = 0;
	    		if (address[0])
	        		dbglog("peer will call: %s", address);
		}
		if (type == CB_CONF_NO)
	    		return;
    		cbcp_up(us);
	} else if (debug)
	    dbglog("cbcp_recvack: malformed packet");
    }

}

/* ok peer will do callback */
static void
cbcp_up(us)
    cbcp_state *us;
{
#ifdef CBCP_SERVER_SUPPORT
	extern u_char *callback_script;

	dbglog("cbcp_up ");

	if (us->us_cbcp) {	/* callback server */
		if (us->us_type & (1 << CB_CONF_NO)) {
			start_networks(us->us_unit);	/* no callback */
			return;
		}
		do_callback = 1;
		callback_script = us->us_cbscript;
		return;
	}
#endif
	persist = 0;
    lcp_close(0, "Call me back, please");
    status = EXIT_CALLBACK;
}

#ifdef CBCP_SERVER_SUPPORT
/* lower layer is down */
static void
cbcp_lowerdown(iface)
    int iface;
{
    cbcp_state *us = &cbcp[iface];
	extern u_char *callback_script;

    dbglog("cbcp_lowerdown");
	UNTIMEOUT(cbcp_reqtmo, (void *)us);	/* stop request timer */
	us->us_cbcp = 0;
	if (doing_callback) {
		callback_script = 0;
	}
}

static void
cbcp_sendreq(cbcp_state *us)
{
	unsigned char buf[256];
	int len = 0;

	len = cbcp_mount_data(us, buf, us->us_type);
	cbcp_send(us,CBCP_REQ, buf, len);
	us->us_retry = 0;
	TIMEOUT(cbcp_reqtmo, (void *)us, CBCP_TIMEOUT);
}

static void
cbcp_recvresp(cbcp_state *us, char *pckt, int pcktlen)
{
    u_char type, opt_len, delay=0, addr_type, err;
    char address[34];

    address[0] = 0;
	err = 0;
	
	UNTIMEOUT(cbcp_reqtmo, (void *)us);	/* stop request timer */

	GETCHAR(type, pckt);
	GETCHAR(opt_len, pckt);

	if (opt_len > 2) {
	    GETCHAR(delay, pckt);
	}
	
	dbglog("cbcp_recresp: pcktlen %d type %d opt_len %d",pcktlen, type, opt_len);

	if (us->us_type & (1 << type)) { /* type is correct */
		switch(type) {
			case CB_CONF_NO:
	    		dbglog("no callback");
	    		break;

			case CB_CONF_USER:
	    		dbglog("user callback ");
	    		if (opt_len > 4) {
	        		GETCHAR(addr_type, pckt);
					memcpy(address, pckt, opt_len - 4);
					address[opt_len - 4] = 0;
					if (address[0]) {
		    			dbglog("address: %s", address);
	    			}
				} else {
					err = 1;
				}
	    		break;

			case CB_CONF_ADMIN:
	    		dbglog("user admin defined");
	    		break;
		}
    } else {
		err = 1;
	}

	if (err) {	/* response is invalid */
		cbcp_reqtmo((void *) us);
		return;
	}

	/* response is valid => send ACK */
	us->us_delay = delay;
	if (address[0]) {
		us->us_number = strdup(address);
	}
	cbcp_sendack(us);
}

static void
cbcp_sendack(cbcp_state *us) 
{
	unsigned char buf[256];
	int len = 0;

	len = cbcp_mount_data(us, buf, us->us_type);
	cbcp_send(us,CBCP_ACK,buf,len);
	cbcp_up(us);
}

static int
cbcp_mount_data(cbcp_state *us, u_char *bufp, u_char cb_type)
{
    int len = 0;

    dbglog("cbcp_mount_data cb_type=%d", cb_type);

    if (cb_type & ( 1 << CB_CONF_USER ) ) {
		dbglog("cbcp_mount_data CONF_USER");
		PUTCHAR(CB_CONF_USER, bufp);
		if (us->us_number) { /* callback client */
			len = 3 + 1 + strlen(us->us_number) + 1;
			PUTCHAR(len , bufp);
			PUTCHAR(us->us_delay, bufp); /* delay */
			PUTCHAR(1, bufp);
			BCOPY(us->us_number, bufp, strlen(us->us_number) + 1);
		} else { 	/* callback server */
			len = 5;
			PUTCHAR(len, bufp);
			PUTCHAR(us->us_delay, bufp); /* delay */
			PUTCHAR(1, bufp);
			PUTCHAR(0, bufp);
		}
		return len;
    }

    if (cb_type & ( 1 << CB_CONF_ADMIN ) ) {
		dbglog("cbcp_mount_data  CONF_ADMIN");
        PUTCHAR(CB_CONF_ADMIN, bufp);
		len = 3;
		PUTCHAR(len, bufp);
		PUTCHAR(us->us_delay, bufp); /* delay */
		return len;
    }

    if (cb_type & ( 1 << CB_CONF_NO ) ) {
        dbglog("cbcp_mount_data  CONF_NO");
		PUTCHAR(CB_CONF_NO, bufp);
		len = 2;
		PUTCHAR(len , bufp);
		return len;
    }
    return(0);
}

static void
cbcp_reqtmo(void * usv)
{
	cbcp_state *us = (cbcp_state *) usv;
	unsigned char buf[256];
	int len = 0;

	if (++us->us_retry >= CBCP_MAXRETRY) {
		/* close LCP */
		persist = 0;
		lcp_close(0, "Callback Negotiation Failed");
		status = EXIT_CALLBACK_NEGOTIATION_FAILED;
	} else {
		us->us_id++;
		len = cbcp_mount_data(us, buf, us->us_type);
		cbcp_send(us, CBCP_REQ, buf, len);
		TIMEOUT(cbcp_reqtmo, (void*)us, CBCP_TIMEOUT);
	}
}
#endif

