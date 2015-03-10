#ifndef __server1_h
#define __server1_h 1
#include <syslog.h>

#define P_IDLE  0
#define P_INCOM 5
#define P_LOGIN 10

#define TELNET_PORT	23

/* Range of normal, non escape ascii chars */
#define Fascii	32	//Space
#define Lascii	126	//Tilde

/* Some syntatic sugar */
#define TRUE	1
#define FALSE	0
#define noerror  0

typedef int (*HANDLECHAR)(unsigned char);

extern HANDLECHAR checkIAC;

/*
 * Global functions for the portslave.
 */
void title(int all, char **argv, char *fmt, ...);
int tty_copy(int crnl, int in, int out);
void block(int sig);
void unblock(int sig);
int get_line(int ppp, const char *prompt, int echo, char *buffer, int size, int flag_telnet);
int chatarray(int fd, int argc, char **argv, struct auth *ai);
int chat_expect(int fd, char *p, char *matchpart, int matchlen);
int chat(int fd, char *str, struct auth *ai);
void unescape(char *);
char *dotted(unsigned int a);
char *dotted6(struct sockaddr *ipaddr, char *buffer, int b_len);
int asc_to_binary6(char *ipstring, struct sockaddr *ipaddr);
int ipaddr_cmp (struct sockaddr_storage *sa1, struct sockaddr_storage *sa2);
int getty(struct auth *ai, int location);
int get_bidirect(struct auth *ai);
int emumodem(struct auth *ai);
void maketermsane(int fd);
int spawnit(struct auth *ai);
void expand_format(char *buffer, size_t size, char *format, struct auth *ai);
int do_slip(struct auth *ai);
int update_utmp(char *login_fmt, char *from_fmt, struct auth *ai, int wtmp);
int update_framed_route(struct auth *ai, int islogin);
int update_filter_id(struct auth *ai, int islogin);
int get_traffic_stats(struct auth *ai);
void tstr(int sending, char *s);
void serial_close(int fd);
int serial_open(struct line_cfg *pconf, int location);
char *term_in_use(struct line_cfg *lc, int location);
char *get_tty_text(struct line_cfg *lc);
void nsyslog(int pri, const char *fmt, ...);
void do_next_token(struct auth *ai, int flag);
char *num(int i);
void serial_unlock(int port);
void serial_lock(int port, char *name);

int check_users_access(char *users, char *user, int null_valid);

//[RK]May/31/05 Group Authorization Support
#ifdef GRPAUTHenable
int check_group_name_access(char * username, char * group_name, char * pusers, char * padmin, int pnull);
#endif
int check_admin_user(char * username, char * group_name, char * padmin);

/* Global variables. */
extern struct termios old_tio;
extern int chat_timeout;
extern int chat_send_delay;
extern void (*chat_debug_printf)(const char *, ...);
extern void set_remote_address(int s, struct auth *ai);

#endif
