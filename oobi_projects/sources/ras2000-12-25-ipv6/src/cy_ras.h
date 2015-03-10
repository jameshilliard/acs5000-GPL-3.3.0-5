/*
 * cy_ras.h Definitions for the common routines shared in 
 *		cy_ras.c and updateconf.c.
 *
 */

#ifndef __updconf_h
#define __updconf_h

extern int last_ip;
void init_socket_server(void);
void process_usr1(void);
void process_usr2(void);
void ip_to_asc(char *p, int ipno);
void kill_session_list_nowait (int port_num, int flag);
void kill_process(struct line_cfg *pconf, int flag);
int getIP6fromFile(char *str_addr);
#endif /*__updconf_h */

