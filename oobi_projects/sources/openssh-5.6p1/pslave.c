#include "includes.h"
#include "buffer.h"

#include <errno.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>

# define __PSLAVE__
#include <pslave_cfg.h>
#include <src/rwconf.h>
#include <src/rot_buf.h>
#include <src/cy_shm.h>
#include <src/auth.h>
#include <src/server1.h>
#include <src/rot_buffer.h>
#include <src/rot_sess.h>
#include <src/mysyslog.h>
#include <src/rot_shm.h>
#include <src/sconf.h>
#ifdef PMD
#include <src/pmc.h>
#endif

#include "openbsd-compat/sys-queue.h"
#include "ssh.h"
#include "ssh1.h"
#include "log.h"
#include "xmalloc.h"
#include "channels.h"
#include "compat.h"
#include "ssh2.h"
#include "dispatch.h"
#include "packet.h"
#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "session.h"
#include "servconf.h"

#include "pslave.h"

#ifdef USE_PAM
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_appdata.h>
#endif

#ifdef IPMI
#include <src/libcyc_rasipmi.h>
#endif

#if defined(CY_DLA_ALERT) || defined(IPv6enable)
#include <sys/socket.h>
#endif

#ifdef CY_DLA_ALERT
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* CY_DLA_ALERT */

#ifdef IPv6enable
#include <netdb.h>
#define EVENT_CONF_FILE   "/etc/event_notif.conf"
#endif /* IPv6 */

extern ServerOptions options;

static struct auth auth_ai;
struct auth *pai = &auth_ai;
static struct line_cfg  *pconf;
static char ch_pending[32];
static int  ix_pend = 0, cmd_pend = 0;
static int  last_was_cr = 0;
static int RetentionMode = 0;
static int multsess_flag = 0;
int DeviceFd = -1;

static LOG_DATA_BUFFER log_buffer = {0} ;

int got_signal_int = 0;

#define cyc_signal_int got_signal_int

#ifdef PMD
int checkPM_echo(Buffer *Buf, int fd);
#endif

static void cyc_write_data_ssh (char *p, int cnt);

Session *session_by_pid(pid_t pid);
void    session_set_fds(Session *, int, int, int, int, int);

static void cyc_pipe_pmssh(int * ptyfd, int * ttyfd);
static void cyc_send_buffer_ssh(char *buf, int len);
static void cyc_write_line_ssh (char *line);
static int cyc_check_buffering_option(char *buffer, int len, int fd, int DeviceFd, CY_SESS * sess);
static void ssh_get_a_package(int type, u_int32_t plen, void *ctxt);
static int write_buf_sess(char *buf, int len);
int ssh_read_a_char(int nonblock_flag);

extern int (*read_char_method)(int nonblock_flag);
extern void (*write_line_method)(char *line, int cnt);

extern void makeRawOrStty(int devFd, struct line_cfg *pconf);

//[RK]May/09/06 - event notificaiton
extern void writeevt(int, char*, ...);

#ifdef USE_PAM
int socket_tty_conv(int num_msg, const struct pam_message **msgm,
	struct pam_response **response, void *appdata_ptr)
{
	return PAM_CONV_ERR;
}
#endif

// treats signals 
static void cyc_treat_int(int sig)
{
	switch (sig) {
		case SIGHUP : // change the configuration
		case SIGIO :  // there is a buffer in cy_shm
		case SIGALRM :
		case SIGUSR2 : // CAS/PMD 
		case SIGUSR1 : // CAS/PMD
			cyc_signal_int = sig;
			break;
		case SIGINT :
		case SIGTERM :
		case SIGCHLD : 
			syslog(LOG_NOTICE,"Receive Signal %d", sig);
			cyc_signal_int = sig;
			break;
		default :
			syslog(LOG_NOTICE,"Receive Signal %d", sig);
	}
}

// Exit routines
void cyc_close_sess(void)
{
	CY_SESS *sess = pai->sess, *first_rw;
	int flag=0, pid = getpid(),flow_flag=0;
	int evt = 6;

	if (thisport < 0) {
		return;
	}
	debug("%s thisport=%x ",__func__,thisport);

	// the session has already closed
	if ((!sess && !pconf->fast) ||
	    (sess && (sess->vet_pid != pid))) {
		nsyslog(LOG_INFO, "Socket-ssh[%d] exit on port %d (%s)",
			pid, (thisport + 1), pconf->tty);
		return;
	}

	if (pconf->fast && (cy_shm->vet_pid[pai->nasport] != pid)) { 
		return;
	}

	// reset the control variables
	if (pconf->fast && (cy_shm->vet_pid[pai->nasport] == pid)) { //mp[Oct/2005] Bug 5159
		cy_shm->vet_pid[pai->nasport] = 0;
		if (cy_shm->tty_user[pai->nasport]) {
			shm_free_str(cy_shm->tty_user[pai->nasport]);
			cy_shm->tty_user[pai->nasport] = 0;
		}
		cy_shm->st_time[pai->nasport] = 0;
	} else {
		if (sess) {
			flag = sess->sess_flag;
			flow_flag = sess->flow_flag;
			sess->vet_pid = 0;
		}
	}

	if (cy_shm->vet_pool[pai->nasport] == pid) {
		cy_shm->vet_pool[pai->nasport] = 0;
	}

	//nsyslog(LOG_INFO, "Socket-ssh[%d] stopped on port %d (%s)",
	//	pid, (thisport+1),pconf->tty);

	if (pconf->fast) {
		close(DeviceFd);
		goto call_writeevt;
	}

	// treat multiple session 
	if (pai->st_sess == 1 || pai->st_sess == 3) {
		// RW session - the main session has been finished or the admin kill this session
		if (sess->sess_flag & 3) {
			cyc_write_line_ssh("\r\nSession killed by the administrator...\r\n");
		}
	}

	if (pai->st_sess == 2) { // RO session
		close_sniff_session(sess);
	}

	clean_session_entry (sess); // free the memory
	pai->sess = 0;

	if (pai->st_sess == 0 || pai->st_sess == 2) {
		// RO session or NONE
		evt = 42;
		goto call_writeevt;
	}

	// treats RW session
	close(DeviceFd);

	if (!(first_rw = first_rw_session(pai->nasport))) {
		//There are no more read/write sessions - kill all the sniff sessions
		if (!(flag & 1)) {
			cy_shm->vet_pool[thisport] = 0;
			//No one is assuming the main session - kill all the sniff sessions
			kill_session_list(pai->nasport, 2);
		}
	} else {
		evt = 42;
	}
	
	if (flow_flag && cy_shm->DB_pid) { 
		kill(cy_shm->DB_pid,SIGIO);
	}

call_writeevt:

	//[RK]May/09/06 - event notification
	writeevt(evt,"ssii",pai->login,
			pconf->alias,
			(thisport + 1),
			1);

}

void ExitFunction(void)
{
	debug("%s thisport=%d",__func__,thisport);
	if (thisport >= 0 && cy_shm->vet_pool[thisport] == getpid()) {
		cy_shm->vet_pool[thisport] = 0;
	}
#ifdef PMD
	if ((pconf->protocol == P_IPDU) &&
	    (pconf->pmsessions & PMSESS_SSH)) {
		cyc_close_sess();
		finish_pam(); //[RK]Mar/15/06
	}
#endif
}

// routines used during authentication phase
// called by SSHD 
void cyc_shm_init() 
{
	debug("%s thisport=%d",__func__,thisport);
	if (shm_init(-1) < 0) {
		syslog(LOG_ERR, "problems initializing shared memomy");
	} else {
		while (shm_get_set_val(0, &cy_shm->st_cy_ras, 0, CY_SEM_CONFIG) == 0)
			sleep(1);
	}
}

// verify if the console is configured as SOCKET_SSH 
// called in auth2.c and auth1.c
int cyc_check_term_config(char *pterm)
{
	char *tty = pterm;
	int  i;
#ifdef IPv6enable
	struct sockaddr_storage ipaddr;
#endif
	struct sockaddr *ipv6 = NULL;

	debug("cyc_check_term_config %s",pterm);
	if (pai && pai->sess)
		pai->sess->sess_flag &= CY_SESS_FLAG_MASK;

	if (thisport >= 0) { // thisport has already set
		return(0);
	}

	if (!memcmp(pterm, "ttyS", 4)) {
		thisport = -6;  // used ttySXX
		goto sintax_ok;
	}

	for(i=0; pterm[i]; i++) {
		if (pterm[i] < '0' || pterm[i] > '9') {
			goto hostname;
		}
	}

	thisport = -3; // TCP port number
	tty = (char *)atoi(pterm);
	goto sintax_ok;

hostname:
#ifdef IPv6enable
	i = asc_to_binary6(pterm, &ipaddr);
	if (!i) {
		/* pterm points to an IP address */
		thisport = -4; // IP address
		if (ipaddr.ss_family == AF_INET6) {
			struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)&ipaddr;
			if (IN6_IS_ADDR_V4MAPPED(sa6->sin6_addr.s6_addr)) {
				(int)tty = sa6->sin6_addr.s6_addr32[3];
			} else {
				ipv6 = (struct sockaddr *)&ipaddr;
				tty = 0;
			}
		} else if (ipaddr.ss_family == AF_INET) {
			struct sockaddr_in *sa4 = (struct sockaddr_in *)&ipaddr;
			(int)tty = sa4->sin_addr.s_addr;
		} else {
			thisport = -5; // ?? alias
			tty = pterm;
		}
		goto sintax_ok;
	}
#else
	if ((tty = (char *)inet_addr(pterm)) != (char *)-1) {
		thisport = -4; // IP address
		goto sintax_ok;
	}
#endif

	thisport = -5; // ?? alias
	tty = pterm;

sintax_ok:
#ifdef IPv6enable
	if ((thisport = rad_init(thisport, pai, tty, 2, ipv6)) < 0) 
#else
	/* TODO - this code must be reviewed when all products start using ras-ipv6 
	 */
	if ((thisport = rad_init(thisport, pai, tty, 2)) < 0) 
#endif
	{
		if (thisport == -1) {
			nsyslog(LOG_ERR, "Radinit could not find the terminal, Param:%s\n", pterm);
		} else {
			nsyslog(LOG_INFO, "Radinit could not find a free pool port, Param:%s\n", pterm);
		}
		return(1);
	} 
	atexit(ExitFunction);
	pconf = &lineconf[thisport];
	cy_shm->flag_dcd[thisport] = 1; //[RK]Jan/04/06 - check the status of DCD signal.
	do_server_auth = pconf->authtype;
	debug("cyc_check_term_config %s thisport=%d",pterm,thisport);
	return(0);
}

// called in auth2.c and auth1.c
int cyc_check_local_ip(void)
{
	struct sockaddr_storage from;
	struct sockaddr *ipv6 = NULL;
	socklen_t fromlen;
	int socket, ip_local;
#ifdef IPv6enable
	char addr[INET6_ADDRSTRLEN];
#endif

	if (thisport >= 0) { // port has already configured
		return(0);
	}
	debug("%s thisport=%d",__func__,thisport);

	/* Get client socket. */
	socket = packet_get_connection_in();

	/* Get IP address of client. */
	fromlen = sizeof(from);
	memset(&from, 0, sizeof(from));
	if (getsockname(socket, (struct sockaddr *) & from, &fromlen) < 0) {
		debug("getsockname failed: %.100s", strerror(errno));
		return(0);
	}

#ifdef IPv6enable
	if (from.ss_family == AF_INET6) {
		struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)&from;
		if (IN6_IS_ADDR_V4MAPPED(sa6->sin6_addr.s6_addr)) {
			(int)ip_local = sa6->sin6_addr.s6_addr32[3];
		} else {
			ipv6 = (struct sockaddr *)&from;
			ip_local = 0;
		}
	} else if (from.ss_family == AF_INET) {
		struct sockaddr_in *sa4 = (struct sockaddr_in *)&from;
		(int)ip_local = sa4->sin_addr.s_addr;
	} else {
		debug("cyc_check_local_ip failed: invalid IP address");
		return(0);
	}
	if ((thisport = rad_init(-4, pai, (char *)ip_local, 2, ipv6)) < 0)
#else
	ip_local = ((struct sockaddr_in *)&from)->sin_addr.s_addr;
	debug("getsockname ip: %.100s", dotted(ip_local));
	/* TODO - this code must be reviewed when all products start using ras-ipv6
	 */
	if ((thisport = rad_init(-4, pai, (char *)ip_local, 2)) < 0)
#endif
	{
		if (thisport == -2) {
#ifdef IPv6enable
			nsyslog(LOG_INFO, "Radinit could not find a free pool port, Param:%s\n", dotted6((struct sockaddr *)&from, addr, INET6_ADDRSTRLEN));
#else
			nsyslog(LOG_INFO, "Radinit could not find a free pool port, Param:%s\n", dotted(ip_local));
#endif
			return(1);
                }
		if (thisport == -3) {
			return(1);
		}
		thisport = -1; // not console
		return(0);
	}
	atexit(ExitFunction);
	pconf = &lineconf[thisport];
	cy_shm->flag_dcd[thisport] = 1; //[RK]Jan/04/06 - check the status of DCD signal.
	do_server_auth = pconf->authtype;
	return(0);
}

// copy the username to AUTH struct
// called in auth2.c and auth1.c
void cyc_copy_user_pai(char *name)
{
	debug("cyc_copy_user_pai [%s]",name);
	//[RK]May/16/06 - bug#7119 
	if (name == NULL || *name==0x00) {
		strcpy(pai->login,"none");
	} else {
		strcpy(pai->login, name);
	}
	pai->do_acct = 0;
	pai->proto = P_SOCKET_SSH;
}

// set the service and conv 
// called in auth-pam.c
#define CYC_NPAM_SERV  26
struct {
	int code;
	char name[30];
} cyc_pam[CYC_NPAM_SERV] = {
	{AUTH_NONE,"none"},

	{AUTH_LOCAL,"local"},
	{AUTH_LOCAL_RADIUS,"localradius"},
	{AUTH_LOCAL_TacacsPlus,"localtacplus"},
#ifdef NISenable
	{AUTH_LOCAL_NIS,"local"},
#endif

	{AUTH_LDAP,"ldap"},
	{AUTH_LDAP_LOCAL,"ldaplocal"},
	{AUTH_LDAP_DOWN_LOCAL,"ldapdownlocal"},
#ifdef RADIUS2FACTORenable
	{AUTH_LDAP_DOWN_LOCAL_AND_RADIUS,"ldapdownlocal-radius"},
#endif

#ifdef KRBenable
	{AUTH_KERBEROS,"kerberos"},
	{AUTH_KERBEROS_LOCAL,"kerberoslocal"},
	{AUTH_KERBEROS_DOWN_LOCAL,"kerberosdownlocal"},
#endif

	{AUTH_RADIUS,"radius"},
	{AUTH_RADIUS_LOCAL,"radiuslocal"},
	{AUTH_RADIUS_DOWN_LOCAL,"radiusdownlocal"},

	{AUTH_TacacsPlus,"tacplus"},
	{AUTH_TacacsPlus_LOCAL,"tacpluslocal"},
	{AUTH_TacacsPlus_DOWN_LOCAL,"tacplusdownlocal"},

#ifdef NISenable
	{AUTH_NIS,"local"},
	{AUTH_NIS_LOCAL,"local"},
	{AUTH_NIS_DOWN_LOCAL,"local"},
#endif

#ifdef OTPenable
	{AUTH_OTP,"otp"},	//[RK]Jan/18/06
	{AUTH_OTP_LOCAL,"otplocal"},
#endif

#ifdef ADSAP2
	{AUTH_DSVIEW, "dsview"}, //[RK]Jun/12/06
	{AUTH_DSVIEW_LOCAL, "dsviewlocal"},
	{AUTH_DSVIEW_DOWN_LOCAL, "dsviewdownlocal"},
#endif
};

// define the PAM service 
char *cyc_get_pam_service()
{
	int i;

	for (i=0; i < CYC_NPAM_SERV;i++) {
		if(cyc_pam[i].code == do_server_auth) {
			break;
		}
	}
	if (i == CYC_NPAM_SERV) i=0;
	debug("cyc_get_pam_service service=%s",cyc_pam[i].name);
	return(cyc_pam[i].name);
}

// set global variables for monitor - mm_register_thisport
void cyc_session_set_global(int port)
{
	debug("%s port=%d",__func__,port);
#ifdef IPv6enable
	thisport = rad_init(port, pai, NULL, 0, NULL);
#else
	/* TODO - this code must be reviewed when all products start using ras-ipv6 
	 */
	thisport = rad_init(port, pai, NULL, 0);
#endif
	pconf = &lineconf[thisport];
	do_server_auth = pconf->authtype;
}

void cyc_set_pconf(char *name)
{
	pconf = &lineconf[thisport];
	cyc_copy_user_pai(name);
}

#ifdef GRPAUTHenable
// Group Name 
// this routine is called by auth-pam (monitor)
void cyc_set_group_name(char *group_name)
{
	struct pam_pwd_ctrl *aux;
	int ppid = getppid();
	char * pgrp;

	debug("%s ppid=%d group_name[%s]",__func__,ppid,group_name);

	aux=cy_shm->pam_passwd;
	while (aux) {
		if(aux->pid==ppid) {
			if ((pgrp = shm_malloc((strlen(group_name) + 1), TYPE_SHM_STR))){
				strcpy(pgrp,group_name);
				aux->group_name = pgrp;
			}
			break;
		}
		else 
			aux=aux->next;
	}
}
#endif

#define NULL_NOT_VALID 0

//[RK]May/20/05 - Group Authorization Support
// verify the user authorization : sXX.users 
// called by auth1 and auth2 
// return : 0 = deny access, 1 = allow access
int cyc_check_access(char * name, int ppid)
{
	struct pam_pwd_ctrl *aux;
	int ret = 0;
	char * cyc_group_name=NULL;

	debug("%s : ppid=%d user[%s] ",__func__,ppid,name);

#ifdef GRPAUTHenable
	aux=cy_shm->pam_passwd;
	while (aux) {
		if(aux->pid==ppid) {
			cyc_group_name = aux->group_name;
			break;
		}
		else 
			aux=aux->next;
	}

	if (cyc_group_name) {
		debug("%s : user[%s] cyc_group_name[%s]",__func__,name,cyc_group_name);
		if (!check_group_name_access(name, cyc_group_name, pconf->users, pconf->admin_users, NULL_NOT_VALID)) {
			pai->flag_admin_user = check_admin_user(name, cyc_group_name,pconf->admin_users);
			ret = 1;
		}
	} else 
#endif
	{
		debug("%s : user[%s] pusers[%s]",__func__,name,pconf->users);
		if (!check_users_access(pconf->users, name, NULL_NOT_VALID)) {
			if (check_users_access(pconf->admin_users, name, 1)) {
				pai->flag_admin_user = 0;
			} else {
				pai->flag_admin_user = 1;
			}
			ret = 1;
		} else {
			if (!check_users_access(pconf->admin_users, name, 1)) {
				pai->flag_admin_user = 1; 
				ret =  1;
			}
		}
	}

	return(ret);
}

// write the event notificaiton : authentication fail 
void cyc_wrtevt_authfail(char * username)
{
	if (thisport < 0) {
		if (logging_dla) writeevt(4,"s",username);
	} else {
		writeevt(7,"ssii",username,pconf->alias,(thisport+1),1);
	}
}

// save the user password - ts_menu second authentication
// called in auth-pam.c
void cyc_save_userp(char * p)
{
	struct pam_pwd_ctrl *pwd_ctrl,*aux;
	shm_signal(CY_SEM_SHM);
	pid_t ppid = getppid();

	debug("%s ",__func__);

	if ((aux=cy_shm->pam_passwd)) {
		while (aux && (aux->pid != ppid)) aux=aux->next;
		if (aux) {
			strncpy(aux->passwd, p, sizeof(pwd_ctrl->passwd)-1);
			goto had_pwd_ctrl;
		}
	}

	if (!(pwd_ctrl = (struct pam_pwd_ctrl *)shm_malloc(sizeof(struct pam_pwd_ctrl), TYPE_SHM_STR)))
		goto mem_problem;
	memset(pwd_ctrl, 0, sizeof(struct pam_pwd_ctrl));
	pwd_ctrl->pid = ppid; //save pid now, replace for shell later
	strncpy(pwd_ctrl->passwd, p, sizeof(pwd_ctrl->passwd)-1);
	if ((aux=cy_shm->pam_passwd)) {
		while (aux->next) aux=aux->next;
		aux->next = pwd_ctrl;
	} else {
		cy_shm->pam_passwd = pwd_ctrl;
	}

had_pwd_ctrl:
mem_problem:
	shm_unsignal(CY_SEM_SHM);
}

// set the correct PID
void cyc_pam_pwd_ctrl(int ppid)
{
	struct pam_pwd_ctrl *aux;

	debug("%s ",__func__);
	 aux=cy_shm->pam_passwd;
	 while (aux) {
		if(aux->pid==ppid) {
			aux->pid=getpid();
		}
		else aux=aux->next;
	 }
}

//=========================================================================
// OPEN - CAS session

// allocate the console port
// called in session.c (do_exec_pty routine)
int cyc_allocate_port(int *ptyfd, int *ttyfd)
{
	struct sigaction sa = {0};
	char *p;
	int  res, count;

	debug("%s ",__func__);

	pconf = &lineconf[thisport];
	// change the cmdline
	setproctitle("-RW %s",get_tty_text(pconf));

	// fill the AI struct with remote address and port
	set_remote_address(packet_get_connection_in(), pai);

	sa.sa_handler = cyc_treat_int;
	for (count=1; count < 32; count++) {
		if (count == SIGPIPE) continue;
		if (count == SIGWINCH) continue;
		if (count == SIGSEGV) continue;
		if (count == SIGFPE) continue;
		if (count == SIGILL) continue;
		if (count == SIGKILL) continue;
		if (count == SIGCHLD) continue;
		sigaction(count,  &sa, NULL);
	}

	//[RK]June/14/04 - SSH/Telnet to IPDU
#ifdef PMD
	if ((pconf->protocol == P_IPDU) &&
	    (pconf->pmsessions & PMSESS_SSH)) {
		cyc_pipe_pmssh(ptyfd, ttyfd); // create a pipe for pmCommand
		pai->st_sess = 1; // read/write session
		pconf->fast = 1;
		setproctitle("-RW %s",get_tty_text(pconf));
		cy_shm->vet_pid[pai->nasport] = getpid();
		cy_shm->tty_user[pai->nasport] = shm_strdup(pai->login);
		cy_shm->st_time[pai->nasport] = time(0);
		atexit(ExitFunction); //[RK]Jul/28/04 - problem with close
		return(1);
	}
#endif

check_again:

	multsess_flag = 0;
	if ((p = term_in_use(pconf, 2))) {
		// there is another session openned in this port
		if (pconf->fast) {
			// show the error message
			cyc_write_line_ssh(p);
			return(0);
		}

		if ((pconf->sniff_mode == 3) &&
		    (pconf->multiple_sessions == NO_MULTIPLE_SESSIONS)) { 
			// NO multiple session and this the second session
			cyc_write_line_ssh(p);
			return(0);
		}
		
		// check permission
		if ((pconf->authtype == AUTH_NONE &&
		    (!pconf->admin_users || !*pconf->admin_users)) ||
		    (pconf->authtype != AUTH_NONE &&
		     !pai->flag_admin_user)) {
			cyc_write_line_ssh(p);
			return(0);
		}

		// check configuration
		//[RK]Sep/09 - fixed problem with sniff i/o without multiple session
		if ((pconf->multiple_sessions == NO_MULTIPLE_SESSIONS) ||
		    (pconf->multiple_sessions == SNIFF_SESSIONS_ONLY)) {
			// open sniff session
			res = check_mult_session(2, p, ssh_read_a_char, cyc_write_data_ssh, NULL);
			if (res == 1) return(0);
			if (res == 2) {
				pai->st_sess = 2; // sniff session;
				*ptyfd = -1;
				*ttyfd = -1;
				setproctitle("-RO %s",get_tty_text(pconf));
				return(1);
			}
			if (res != 3 ) goto check_again;
		}	
		if (pconf->multiple_sessions != RW_SESSIONS_ONLY) {
			write_line_method = cyc_write_data_ssh;
			read_char_method = ssh_read_a_char; 
			multsess_flag = 1;
		}
	}

	if (!pconf->fast) { // change buffers with cy_buffering
		// get sess struct
		if(open_rw_session(2, ssh_read_a_char, cyc_write_data_ssh, NULL) == 1) {
			return(0); // error
		}

		if (pconf->virtual) { // virtual port
			pai->st_sess = 3; // read/write virtual session;
			if ((DeviceFd = open_virtual_session(2,ssh_read_a_char, cyc_write_data_ssh)) < 0) {
				return(0);
                        }
		} else { // physical port
			pai->st_sess = 1; // read/write session;
			DeviceFd = -1;
			setproctitle("-RW %s",get_tty_text(pconf));

			//mp[Sep2003] update multiuser notification status
			if (cy_shm->line_conf[pai->nasport].multiuser_notif && pai->sess->prev) {
				CY_SESS *saux = pai->sess;
				shm_signal(CY_SEM_SHM);
				while (saux) {
					saux->sess_flag &= ~CY_SESS_FLAG_MUNTF_M;
					saux->sess_flag |= CY_SESS_FLAG_MUNTF_IN;
					saux = saux->prev;
				}
				shm_unsignal(CY_SEM_SHM);
			}
			//mp[Nov2003] set data buffer user logs control
			if (cy_shm->line_conf[pai->nasport].DB_user_logs) {
				shm_signal(CY_SEM_SHM);
				pai->sess->sess_flag |= CY_SESS_FLAG_DB_USRLOG;
				shm_unsignal(CY_SEM_SHM);
			}
			// used by cy_buffering
			shm_signal(CY_SEM_SHM);
			pai->sess->sess_flag |= CY_SESS_FLAG_SSHD;
			shm_unsignal(CY_SEM_SHM);
		}
	} else { // open and control the serial port 
		pai->st_sess = 1; // read/write session
		if ((DeviceFd = getty(pai,2)) < 0) {
			error("Failed to allocate pty.");
			return(0);
		}
		setproctitle("-RW %s",get_tty_text(pconf));
		cy_shm->vet_pid[pai->nasport] = getpid();
		cy_shm->tty_user[pai->nasport] = shm_strdup(pai->login);
		cy_shm->st_time[pai->nasport] = time(0);
	}
	
	//[RK]May/09/06 - event notification
	writeevt(5,"ssii",pai->login,pconf->alias,pai->nasport+1,1);
	//syslog(LOG_INFO, "%s: User [%s] connected to port [%d] (%s) via socket ssh",
	//	TAG_APPLICATION,pai->login, pai->nasport+1, pconf->tty);

	*ptyfd = DeviceFd;
	*ttyfd = -1;

	return(1);
}

// open one SSH session
// called in serverloop.c (server_loop and server_loop2 routines)
int cyc_create_session_tty(void)
{
	Session *s;

	debug("%s thisport=%d",__func__,thisport);
	// get SSH session struct
	s = session_by_pid(getpid());

	/* Allocate a pty and open it. */
	if (!cyc_allocate_port(&s->ptyfd, &s->ttyfd)) {
		fatal("do_exec_pty: failed to allocate ttyS");
	}

#ifdef PMD
	//[RK] Jun/14/04 - SSH/Telnet to IPDU
	if (pconf->protocol == P_IPDU) {
		if (compat20) {
			//session_set_fds(s,s->ptyfd,s->ttyfd,-1,1,2);
			session_set_fds(s,s->ptyfd,s->ttyfd,-1,0,2);
		}
	} else 
#endif
	{
		if (s->ptyfd >= 0) {
			// change the TTY attributes
			makeRawOrStty(s->ptyfd, pconf);
			s->ptymaster = s->ptyfd;
		}

		if (s->ptyfd < 0 && (pai->st_sess == 3)) // RW virtual session without connection
			packet_disconnect("dup #1 failed: %.100s", strerror(errno));

		/* Enter interactive session. */
		if (compat20) {
			//session_set_fds(s, s->ptyfd, s->ptyfd, -1,1,2);
			session_set_fds(s, s->ptyfd, s->ptyfd, -1,0,2);
		}
		// check data buffering 
		if (!pconf->fast) cyc_check_RetentionMode(0); 
        }
	return(s->ptyfd);
}

int cyc_set_fdout(int ptyfd,int ttyfd)
{
	debug("%s thisport=%d",__func__,thisport);
#ifdef PMD
	if (pconf->protocol == P_IPDU) {
		return(ttyfd);
	} else 
#endif
	{
		if (ptyfd >= 0) {
			makeRawOrStty(ptyfd, pconf);
		}
		return(ptyfd);
	}
}

void cyc_set_vet_pool(void)
{
	debug("%s thisport=%d",__func__,thisport);
	cy_shm->vet_pool[thisport] = getpid();
}

#ifdef PMD
//[RK]June/14/04 - SSH/Telnet to IPDU
// open pipe for pmCommand
static void cyc_pipe_pmssh(int * ptyfd, int * ttyfd)
{
	int pin[2], pout[2],perr[2];

	debug("%s thisport=%d",__func__,thisport);
	/* Allocate pipes for communicating with the program. */
	if (pipe(pin) < 0 || pipe(pout) < 0 || pipe(perr) < 0) {
		nsyslog(LOG_NOTICE,"Could not create pipes: %.100s",strerror(errno));
		exit(1);
	}

	/* Fork the child. */
	if ((cyc_pid_child = fork()) == 0) {
		char * argv[4];
		char cmd[20];
		int i;
		uid_t uid;
		struct passwd *pwui;

		pwui = getpwnam(pai->login);

		if (pwui) {
			uid = pwui->pw_uid;
		} else {
			exit(1);
		}

		// Redirect stdin.  We close the parent side of the socket
		// pair, and make the child side the standard input.
		close(pin[1]);
		if (dup2(pin[0], 0) < 0)
			perror("dup2 stdin");
		close(pin[0]);
		/* Redirect stdout. */
		close(pout[0]);
		if (dup2(pout[1], 1) < 0)
			perror("dup2 stdout");
		close(pout[1]);
		/* Redirect stderr. */
		close(perr[0]);
		if (dup2(perr[1], 2) < 0)
			perror("dup2 stderr");
		close(perr[1]);
		// call pmCommand : execve
		for (i=3;i < 64; i++)
			close(i);

		setuid(uid);

		tcflush(0,TCIOFLUSH);
		tcflush(1,TCIOFLUSH);

		argv[0] = "/bin/pmCommand";
#ifdef ONS
		sprintf(cmd,"%1s%d",(pai->nasport+1 > AUX_OFFSET) ? "a" : "s",
			(pai->nasport+1 > AUX_OFFSET) ? (pai->nasport+1-AUX_OFFSET) : (pai->nasport+1));
#else
		sprintf(cmd,"%d",pai->nasport+1);
#endif
		argv[1] = strdup(cmd);
		argv[2] = NULL;
		signal (SIGPIPE, SIG_DFL);
		execve("/bin/pmCommand", argv, NULL);
		exit(1);
	}

	// Close the child sides of the pipes.
	close(pin[0]);
	close(pout[1]);
	close(perr[1]);

	*ptyfd = pin[1]; // data to pmCommand
	*ttyfd = pout[0]; // data from pmCommand
}
#endif

// ================================================================
// send buffer to ssh client
// used by multiple session and data buffering menu
static void cyc_send_buffer_ssh(char *buf, int len)
{
	debug("%s thisport=%d len=[%d]",__func__,thisport,len);
	if (compat20) {
		packet_start(SSH2_MSG_CHANNEL_DATA);
		packet_put_int(ssh_remote_id);
		packet_put_string(buf, len);
	} else {
		packet_start(SSH_SMSG_STDOUT_DATA);
		packet_put_string(buf, len);
	}
	packet_send();
	packet_write_wait();
}

// send one string to ssh client
static void cyc_write_line_ssh (char *line)
{
	//debug("%s thisport=%d",__func__,thisport);
	cyc_send_buffer_ssh(line, strlen(line));
}

// send data to ssh client
static void cyc_write_data_ssh (char *p, int cnt)
{
	if (cnt) {
		//debug("%s thisport=%d cnt=[%d]",__func__,thisport,cnt);
		cyc_send_buffer_ssh(p, cnt);
	} else {
		//debug("%s thisport=%d strlen=[%d]",__func__,thisport,strlen(p));
		cyc_send_buffer_ssh(p, strlen(p));
	}
}

//==================================================================
// send break to tty 
// called in session.c file
void session_set_sendbreak(int break_interval)
{
	debug("%s thisport=%d",__func__,thisport);
	nsyslog(LOG_INFO,"Client sent a break to %s (interval=%d)", pconf->tty,break_interval);
	if (pconf->fast) {
		tcsendbreak(DeviceFd,break_interval);		
	} else {
		pai->sess->sendbreak = break_interval;
		if (cy_shm->DB_pid) { //[RK]Nov/24/03 Bug#1075
			kill(cy_shm->DB_pid,SIGIO);
		}
	}
}

//======================================================================
// Data Buffering treatment

void cyc_check_RetentionMode(int ver)
{
	LOG_DATA_BUFFER *aux;

	debug("%s thisport=%d ver=[%d]",__func__,thisport,ver);
	if (ver == 1)
		aux = (LOG_DATA_BUFFER *)&fd_buf;
	else
		aux = &log_buffer;

	if (multsess_flag && pai->sess &&
            (pai->sess->sess_flag & CY_SESS_FLAG_MENU)) {
		check_sniff_options(pconf->escape_char, pai->sess);
		pai->sess->sess_flag |= CY_SESS_FLAG_MENU;
		pai->sess->sess_flag |= 4;
		cmd_pend = 1;
		multsess_flag = 0;
		return;
	}

	if (!pconf->fast && (pai->st_sess == 1) && // RW session
	    // check data_buffering configuration and if there is data
	    (RetentionMode = check_buffering_retention_mode(pconf, 
				aux, pai->sess))) {
		pai->sess->sess_flag |= 4; // Data Buffering MENU
		RetentionMode = cyc_check_buffering_option(NULL, 0, fd_buf, DeviceFd, pai->sess);
		if (!RetentionMode)
			pai->sess->sess_flag &= CY_SESS_FLAG_MASK;
	}

	//if (pconf->fast) 
	//	makeRawOrStty(DeviceFd, pconf);

	if (ver) log_buffer.tot_buf = 0;
}

static char op3_txt[] =
"\r\nA non-empty Data Buffering File was found. Choose which action \r\n"
"should be performed ( (I)gnore or (D)isplay ) : ";
static char op0_txt[] =
"\r\nA non-empty Data Buffering File was found. Choose which action \r\n"
"should be performed ( (I)gnore, (D)isplay, (E)rase or (S)how and erase ) : ";
static char inv_txt[] = "\r\n'?' is an invalid option";
static char disp_txt[] = "?\r\nFile content:\r\n";
static char ignore_txt[] = "\r\nFile content was ignored\r\n";
static char del_txt[] = "?\r\nFile erased !\r\n";

static int cyc_check_buffering_option(char *buffer, int len, int fd, int DeviceFd, CY_SESS * sess)
{
	int rtn = 0;
	char ch_op, flag = 0;

	debug("%s thisport=%d",__func__,thisport);
	// check the port configuration 
	if (pconf->dont_show_db_op == 1 && pconf->DB_mode != 1) { 
		// no display menu and ignore buffer
		cyc_send_buffer_ssh(ignore_txt, strlen(ignore_txt));
		goto exit_rot;
	}

	//[RK]Feb/15/06
	if ((pconf->dont_show_db_op == 2) ||
#ifdef DB_ENH
	    (pconf->dont_show_db_op == 4) ||
#endif
	    (pconf->DB_mode == 1)) { // no display menu and display buffer
		if (pconf->DB_mode != 1) {
			if (pconf->dont_show_db_op == 2) { // show the buffer
				cyc_send_buffer_ssh(&disp_txt[1], strlen(disp_txt)-1);
			}
			lseek(fd, 0, SEEK_SET);
			more(fd, DeviceFd, ssh_read_a_char, cyc_write_line_ssh);
#ifdef DB_ENH
			if (pconf->dont_show_db_op == 4) { // erase the buffer
				lseek(fd, 0, SEEK_SET);
				ftruncate(fd, 0);
				goto exit_rot;
			}
#endif
			cyc_send_buffer_ssh("\r\n", 2);
			lseek(fd, 0, SEEK_END);
		}
		goto exit_rot;
	}

	// treats the option choosed by client
	while (len--) {
		ch_op = *buffer++;
		flag = 1;
		switch (ch_op &= 0xdf) {
			case 'S': // show the data and erase it
				if (pconf->dont_show_db_op == 3) {
					// options : Ignore and Display are valids
					inv_txt[3] = ch_op;
					cyc_send_buffer_ssh(inv_txt, strlen(inv_txt));
					break;
				}
						
			case 'D': // show the data
				disp_txt[0] = ch_op;
				cyc_send_buffer_ssh(disp_txt, strlen(disp_txt));
				// file with Data Buffering
				lseek(fd, 0, SEEK_SET);
				// display the data
				more(fd, DeviceFd, ssh_read_a_char, cyc_write_line_ssh);

				if (ch_op == 'D') {
					cyc_send_buffer_ssh("\r\n", 2);
					lseek(fd, 0, SEEK_END);
					goto exit_rot;
				}
				// erase the data
				cyc_send_buffer_ssh(del_txt+1, strlen(del_txt)-1);
				lseek(fd, 0, SEEK_SET);
				ftruncate(fd, 0); 
				goto exit_rot;

			case 'E': // erase the data
				if (pconf->dont_show_db_op == 3) {
					// options : Ignore and Display are valids
					inv_txt[3] = ch_op;
					cyc_send_buffer_ssh(inv_txt, strlen(inv_txt));
					break;
				}
				del_txt[0] = ch_op;
				cyc_send_buffer_ssh(del_txt, strlen(del_txt));
				lseek(fd, 0, SEEK_SET);
				ftruncate(fd, 0); 
				goto exit_rot;

			case 'I': // ignore the data
				disp_txt[0] = ch_op;
				cyc_send_buffer_ssh(disp_txt, 3);
				goto exit_rot;

			default:
				inv_txt[3] = ch_op;
				cyc_send_buffer_ssh(inv_txt, strlen(inv_txt));
				break;
		}
	}
	if (buffer == NULL || flag) {
		if (pconf->dont_show_db_op == 0) { 
			cyc_send_buffer_ssh(op0_txt, strlen(op0_txt));
		} else {
			cyc_send_buffer_ssh(op3_txt, strlen(op3_txt));
		}
	}
    rtn = 1;
exit_rot:
	if (sess) {
		if (rtn)
			sess->sess_flag |= CY_SESS_FLAG_DBMENU;
		else
			sess->sess_flag &= ~CY_SESS_FLAG_DBMENU;
#ifdef DB_ENH
		//[RK]Mar/01/06
		if ((sess->flow_flag == 2) && cy_shm->DB_pid) {
			// restart the flow
			kill(cy_shm->DB_pid, SIGIO);
		}
#endif
	}
	return(rtn);
}

//========================================================================
// SERVERLOOP routines

// check DCD signal
// ret 1 - DCD ON ret 0 - DCD OFF

int cyc_check_dcd(void)
{
	int ret;

	if (!pconf->dcd) // DCD is not controlled
		return (1); 

	if (pconf->fast) {
		int signals;
		// port is not OK
		if (DeviceFd == -1) return 1;
		ioctl(DeviceFd,TIOCMGET,&signals);
		if (!(signals & TIOCM_CD) ) { // DCD is OFF
			cyc_write_line_ssh ("Error reading from device, DCD is off.");
			nsyslog(LOG_NOTICE,"Error reading from device, DCD is off.");
			return(0);
		} else {
			return(1);
		}
	}
	// get the information from cy_buffering
	if(!cy_shm->flag_dcd[thisport]) {
		cy_shm->flag_dcd[thisport] = 1;
		ret = cy_shm->dcd_status[thisport];
		if (!ret) {
			cyc_write_line_ssh ("Error reading from device, DCD is off.");
			nsyslog(LOG_NOTICE,"Error reading from device, DCD is off.");
		}
		return (ret);
	}
	cy_shm->flag_dcd[thisport] = 1;
	return 1;
}

// ignore the data from TTY to SSH client if in Retention mode
// called by make_packets_from_stdout_data routine
int cyc_ignore_data_buffer(int len)
{
	if (RetentionMode) {
		return(0);
	}
	return(len);
}

// check signal -- after select command
// called by wait_until_can_do_something routine
int cyc_check_signal(int selret)
{
	if (pai->sess && cyc_signal_int) { // session with Cy_buffering

		if (cyc_signal_int == SIGIO) { // there is data in cy_shm 
			cyc_signal_int = 0;
			return(1);
		}

#ifdef PMD
		if (pai->sess->sess_flag & CY_SESS_FLAG_PMD) {
			// CAS - PM menu
			if (cyc_signal_int == SIGUSR1) { // PMD is not OK
				cyc_signal_int = 0;
				pmcas_logout(pai->sess); // exit PM session
				cyc_write_data_ssh(servernotready,0); // send to user PMD not ready
				return(1);
			}
			//[RK]Jan/10 - receive SIGCHLD after IPMI command
			if ((pai->sess->sess_flag & CY_SESS_FLAG_IPMI) &&
			    (cyc_signal_int == SIGCHLD)) {
				cyc_signal_int = 0;
				return(1);
			}
		}
#endif
		return(-1); //[RK]Sep/15/05 - fixed the problem with interruption in this routine
	}

	//[RK]Sep/22/05 - fixed problem with SIGINT
        if (pconf->fast) {
        	if (selret > 0) {
			cyc_check_idle_time(0, 1); // reset the idle timer
		}
		if (cyc_signal_int) { //[RK]Sep/15/05 - fixed the problem with interruption in this routine
			return(-1);
		}
        }
		return(0);
} 

// check sniff session and calculate the max_time_milliseconds
int cyc_sniff_max_time()
{
	if (!pconf->fast &&
	    (pai->st_sess == 2 || pai->st_sess == 1)) {
		// CAS session - local sess RW/RO
		// copy the data from cy_shm to temporary buffer 
		// send the data to SSH client
		if (read_sniff_msg(pai->sess)) { 
			cyc_check_idle_time(0, 1); // reset the idle timer
		}
		if ((pai->sess->flow_flag == 2) && cy_shm->DB_pid) {
			// restart the flow
			kill(cy_shm->DB_pid, SIGIO);
		}
		return(50);
	} else {
		return(1000);
	}
}

//check serial port is fast or virtual
int cyc_check_fast()
{
	if (pconf->fast || (pai->st_sess == 3)) return(1);
	return(0);
}
 
// finish the SSH section
void cyc_finish_ssh()
{
	debug("%s thisport=%d",__func__,thisport);
	if (!pconf->fast) {
		if (pai->st_sess == 1 || pai->st_sess == 2) {
			int sec = 10;
			/* Wait until cy_buffering writes everything in the tty port */
			while (!IsBufferEmpty(&pai->sess->InBuffer) && sec-- > 0) {
				sleep(1);
			}
		}
		if (pai->sess && (pai->sess->sess_flag & CY_SESS_FLAG_SCONF)) {
			sconf_logout(pai->sess);
		}
#ifdef PM
		if (pai->sess && pai->sess->pmc) {
			pmcas_logout(pai->sess);
		}
#endif
#ifdef IPMI
		if (pai->sess && (pai->sess->sess_flag & CY_SESS_FLAG_IPMI)) {
			ipmi_logout(pai->sess);
		}
#endif
	}
}


//====================================================================
// analyse the data from SSH client to TTY
// called : process_output routine when SSHV1

int cyc_ssh_tty_data(Buffer *Buf, int fd, fd_set * writeset)
{
	int len;
	unsigned int st;
	char *p;
	int count=0;	//[RK]Oct/14/06

	if ((thisport < 0) || (pai->st_sess == 3)) {
		if ((fd != -1) && FD_ISSET(fd,writeset))
			return -1;
		else 
			return 0;
	}

#ifdef PMD
	if (pconf->protocol == P_IPDU) {
		// pipe with pmCommand
		return(checkPM_echo(Buf, fd));
	}
#endif

	if (pai->st_sess == 2) { // RO session
		if (pconf->multiple_sessions != MULTIPLE_RW_AND_SNIFF_SESSIONS){
			return(0);
		}
		while (Buf->end != Buf->offset) {
			// verify mult session menu
			if (check_sniff_options(*(Buf->buf+Buf->offset), pai->sess)) {
				cyc_alloc_tty = 1;
				Buf->offset++;
				break;
			}
			Buf->offset++;
		}
		return(0);
	}

	if (RetentionMode) { // Data Buffering Menu
		len = buffer_len(Buf);
		RetentionMode = cyc_check_buffering_option(buffer_ptr(Buf),
			len, fd_buf, DeviceFd, pai->sess);
		buffer_consume(Buf, len);
		if (!RetentionMode)
			pai->sess->sess_flag &= CY_SESS_FLAG_MASK;
		return(0);
	} 

	if (pconf->break_seq == NULL || *pconf->break_seq == 0) {
		// not configured break sequence 
		if (pconf->fast ||  
		    (pai->sess && !(pai->sess->sess_flag & CY_SESS_FLAG_MENU)
#ifdef IPMI
		     && !pconf->IPMIkey
#endif
#ifdef PMD
		     && !pconf->pmkey
#endif
				)) {
			// FAST or not has PMKey and not in mult sess menu
			len = buffer_len(Buf);
			goto exit_check_break;
		}
	}
	
	// CAS session 
	if (pai->sess) {
#ifdef PMD
		if (pai->sess->sess_flag & CY_SESS_FLAG_PMD) {
		 	// CAS PM menu
			len = buffer_len(Buf);
			goto exit_check_break;
		}
#endif
#ifdef IPMI	
		if (pai->sess->sess_flag & CY_SESS_FLAG_IPMI) {
			// CAS IPMI menu
			len = buffer_len(Buf);
			goto exit_check_break;
		}
#endif
	}

	p = Buf->buf + Buf->offset;
	st = Buf->offset;

	last_was_cr = (ix_pend || last_was_cr);
	if (ix_pend == -1) {
		ix_pend = 0;
	}

	for (len=0; st < Buf->end; st++, p++) {
		if (pai->sess) {
			if (pai->sess->sess_flag & CY_SESS_FLAG_MENU) {
				// mult session menu
				check_sniff_options(*p, pai->sess);
				if (pai->sess->sess_flag & 4) {
					if (!cmd_pend) {
						//*p is the escape char
						cmd_pend = 1;
						if (len) {
							// We have to send some bytes before treating the menu
							count = len;
							goto exit_check_break;
						}
					}
					Buf->offset = st+1;
					continue;
				} else {
					if (cmd_pend) {
						if (pai->sess->sess_flag & CY_SESS_FLAG_W) {
							setproctitle("-RW %s",get_tty_text(pconf));
						} else {
							setproctitle("-RO %s",get_tty_text(pconf));
						}
						// Has just finished the treatment of the menu
						cmd_pend = 0;
						Buf->offset = st+1;
						continue;
					}
				}
			}
			if (pconf->sconfkey && (pconf->sconfkey == *p)) {
				// client typed SCONFKey -> enter serial configuration mode
				sconf_login(pai->sess, pconf, cyc_write_data_ssh);
				return(buffer_len(Buf));
			}
#ifdef PMD
			if (pconf->pmkey && (pconf->pmkey == *p)) {
				// client typed PMKey -> start CAS PM menu
				pmcas_login(pai->sess, pconf->tty, cyc_write_data_ssh);
				return(buffer_len(Buf));
			}
#endif
#ifdef IPMI
			if (pconf->IPMIkey && (pconf->IPMIkey == *p)) {
				// client typed IPMIkey -> start CAS IPMI menu
				ipmi_login(pai->sess,pconf->IPMIServer,0,cyc_write_data_ssh);
				return(buffer_len(Buf));
			}
#endif	
		}

		if (pconf->break_seq == NULL || *pconf->break_seq == 0) {
			len++;
			continue;
		}

		// the char before the break sequence need to be NL 
		// ix_pend != 0 => start one break sequence
		if ((*p == '\r' || *p == '\n') && !ix_pend) { 
			last_was_cr = 1;
			len++;
			continue;
		}

		if (!last_was_cr) {
			// last char is not NL
			len++;
			continue;
		}
		// last char is NL -> check break sequence
		if (*p == pconf->break_seq[ix_pend]) {
			ch_pending[ix_pend++] = *p;

			if (!pconf->break_seq[ix_pend]) {
				// a break sequence was found
				if (len) {
					ix_pend = -1;
					// We have to send some bytes before the break
					goto exit_check_break;
				}
				ix_pend = 0;
				Buf->offset = st+1;

				if (pai->st_sess == 1) {// RW session 
					session_set_sendbreak((pconf->break_interval)? pconf->break_interval : 500);
				}
				if (pai->st_sess == 3) { // virtual session
					write(fd, "\xff\xf3", 2);
				}
			}
		} else { // is not break sequence, clear variables
			last_was_cr = 0;
			if (ix_pend) {
				if (Buf->offset + ix_pend <= st) {
					// a piece of break seq is in this Buf
					len += ix_pend;
				} else {
					// a piece of break seq is only in ch_pending
					if (pconf->fast || (pai->st_sess == 3)) {
						if (write(fd, ch_pending, ix_pend) != ix_pend) {
							return(-1);
						}
					} else {
						if (write_buf_sess(ch_pending, ix_pend) != ix_pend) {
							return(-1);
						}
					}
					Buf->offset = st;
				}
				ix_pend = 0;
			}
			++len;
		}
	}

exit_check_break:

	count = len;

	//[RK]Oct/14/06
	// Linefeed suppression. If the last bytes consist of \r\n, eliminate
	// the \n. Solves problem where telnet from Win98 gives extra prompt
	// line when you press Enter.
	// Nei: added raw data for this feature
	// 08/25/03 Nei: lf_suppression = 1 (CRLF expected; LF suppressed)
	if ((pconf->lf_suppress == 1) && (count >= 2) &&
		(Buf->buf[Buf->offset + count-2] == '\r') &&
		(Buf->buf[Buf->offset + count-1] == '\n')) {
			count--;
	} else { // 08/25/03 Nei: lf_suppression = 2 (CRNULL expected; NULL suppressed)
		if ((pconf->lf_suppress == 2) && (count >= 2) &&
		    (Buf->buf[Buf->offset + count-2] == '\r') &&
		    (Buf->buf[Buf->offset + count-1] == 0)) {
			count--;
		}
	}

	if (ix_pend) {
		// the end of the buffer us the beginning of a break sequence
		// it is now in the ch_pending
		if (count) {
			Buf->end -= ix_pend;
		} else {
			Buf->end = Buf->offset;
		}
	}

	if (count) {
		if (pconf->fast || (pai->st_sess == 3)) {
			// fast or virtual session
			write(fd, buffer_ptr(Buf), count);
		} else { // copy the data to cy_shm (cy_buffering)
			write_buf_sess(buffer_ptr(Buf), count);
		}
	}
	return(len);
}

//=====================================================================
// write data from SSH client to shared memory (cy_buffering)
static int write_buf_sess(char *buf, int len)
{
	int count;
	CY_SESS * sess = pai->sess;
	BufferType *pInBuffer = &sess->InBuffer;

	// free space in the buffer
	if ((count = MaxContiguosFree(pInBuffer)) > 0) {
		count = MIN(count, len);
		// copy the data to shared memory
		memcpy(&pInBuffer->Buffer[pInBuffer->WrPos], buf, count);
		// increase the offset of the write pos in the buffer.
		pInBuffer->WrPos = (pInBuffer->WrPos + count) % BufferSize;
		if (sess->sess_flag & CY_SESS_FLAG_SCONF) {
			// we are in serial configuration mode
			sconf_treat_input(sess, pconf, cyc_write_data_ssh);
		} else
#ifdef PMD
		if (sess->sess_flag & CY_SESS_FLAG_PMD) {
			// send message to PMD
			pmcas_treat_input(sess, cyc_write_data_ssh);
#else
		if (0) {
#endif
#ifdef IPMI
		} else if (sess->sess_flag & CY_SESS_FLAG_IPMI) {
			// send message to ipmi
			ipmi_treat_input(sess,cyc_write_data_ssh);
#endif
		} else {
			if (cy_shm->DB_pid) { // cy_buffering is OK
				kill(cy_shm->DB_pid,SIGIO);
			}
		}
		return (count);
	}
	return (0);
}

//=================================================================
// internal routines to read data from SSH client
# define MY_BUF_SIZE	16
static int 	got_package;
static char	my_buf[MY_BUF_SIZE];
static int	my_len;
static int	my_off;

static void ssh_get_a_package(int type, u_int32_t plen, void *ctxt)
{
	int id;
	char *data;
	unsigned int data_len;
	Channel *c;

	if (compat20){
		/* Get the channel number and verify it. */
		id = packet_get_int();
		if (id != ssh2_channel_id) return;

		c = channel_lookup(id);
		if (c == NULL)
			packet_disconnect("Received data for nonexistent channel %d.", id);

		/* Ignore any data for non-open channels (might happen on close) */
		if (c->type != SSH_CHANNEL_OPEN &&
			c->type != SSH_CHANNEL_X11_OPEN)
			return;

	}
	/* Get the data. */
	data = packet_get_string(&data_len);
	packet_check_eom();

	if ((my_len = data_len) > MY_BUF_SIZE) {
		nsyslog(LOG_WARNING, "Stdin package too big (%d)", data_len);
		my_len = MY_BUF_SIZE;
	}
	my_off = 0;
	memcpy (my_buf, data, my_len);
	memset(data, 0, data_len);
	xfree(data);
	got_package = 1;
}

int ssh_read_a_char(int nonblock_flag)
{
	dispatch_fn *save_fn;
	int		msg;
	extern dispatch_fn * dispatch_get(int);

	if (my_len == my_off) {
		if (compat20) {
			msg = SSH2_MSG_CHANNEL_DATA;
		}else {
			msg = SSH_CMSG_STDIN_DATA;
		}
		got_package = 0;
		save_fn = (dispatch_fn *)dispatch_get (msg);
		dispatch_set (msg, &ssh_get_a_package);
		dispatch_run (DISPATCH_BLOCK, &got_package, NULL);
		dispatch_set (msg, save_fn);
	}
	return (got_package && my_off < my_len? my_buf[my_off++] : -1);
}

//======================================================================
// control the SSH session idle timeout
// called in serverloop.c
static int cyc_idletime = 0;

int cyc_check_idle_time(int milliseconds, int reset)
{
	if (!pconf->idletimeout) {
		return(0);
	}

	if (reset) {
		cyc_idletime = 0;
		return(0);
	}

	cyc_idletime += milliseconds;

	if (pconf->idletimeout * 60 * 1000 <= cyc_idletime) {
		nsyslog(LOG_NOTICE,"Inactivity Timeout");
		cyc_signal_int = 1; //[RK]Dec/01/05
		return(1);
	}

	return(0);
}

#ifdef PMD
//==================================================================
//[RK]Jun/15/04 - SSH/Telnet to IPDU
int checkPM_echo(Buffer *Buf, int fd)
{
	int len;
	unsigned int st;
	char *p;
	
	p = Buf->buf + Buf->offset;
	st = Buf->offset;

	for (len=Buf->offset; st < Buf->end; st++, p++) {
		if (*p == 0x7f) { 
			*p = '\b';
		}
		len++;
	}

	write(fd, buffer_ptr(Buf), len);
	return(len);
}
#endif

#ifdef CY_DLA_ALERT

// Also defined in krb[version]/src/appl/telnet/telnetd/utility.c
#define MAX_STR_LOG_LEN 1024
extern Buffer outgoing_pipe;
static int dla_lost_data = 0;

static int dla_init_sock_ip = 0;

#ifdef IPv6enable
static char dla_ip_local_str[INET6_ADDRSTRLEN];	//46 bytes (from <netinet/in.h)
#else
static char dla_ip_local_str[INET_ADDRSTRLEN];	//16 bytes (from <netinet/in.h)	
#endif

#define MAX_TIMESTAMP_STR 21
#define MAX_DLA_ALERT_STRINGS 10
#define MAX_DLA_ALERT_LENGTH 40

#define DLA_PIPE_NAME "/var/run/DB/dla_pipe"
#define ALERT_PIPE_NAME "/var/run/snmp_pipe"

static int dla_init_alert = 0;
static unsigned int dla_num_alerts = 0;
static char *dla_alert_string[MAX_DLA_ALERT_STRINGS];

static unsigned char *dla_log_data = NULL;
static unsigned int dla_log_max = 0;

extern int dla_log_pipe;
extern int dla_alert_pipe;
extern Authctxt *the_authctxt;


#ifdef IPv6enable
//---------------------------------------------------------------------
//  FUNCTION: readLocalAddress
//
//  DESCRIPTION: Read the local address to connect to DSVIEW from
//               EVENT_CONF_FILE.
//
//  PARAMETERS: local_address
//
//  RETURNS:  0 = local_address read
//           -1 = error reading local_address
//
//  NOTES:
//
//---------------------------------------------------------------------
static int readLocalAddress(char *local_address)
{
	FILE *fd;
	char *line, *pli, *pe;
	int i, ret = -1;

	// default value
	local_address[0] = 0;

	// open file
	if ((fd = fopen(EVENT_CONF_FILE, "r")) == NULL) {
		return(ret);
	}

	// allocate memory for one line with 80 chars
	line = calloc(81, 1);

	// read a line
	while (fgets(line, 81, fd) != NULL) {

		if (line[0] == '#') continue;

		if (strncmp("localAddress", line, 12) == 0) {
			pe = strstr(line,"=");
			if (!pe) continue;
			pe++;
			while (*pe && (isspace(*pe))) pe++;
			pli=pe;
			while (*pe && !(isspace(*pe))) pe++;
			if (*pe) *pe=0x00;
			strcpy(local_address, pli);
			ret = 0;
			break;
		}
	}

	fclose (fd);

	return(ret);
}
#endif

void
cy_dla_alert(Buffer *Buf)
{
	unsigned char *pipe_offset, *buf_offset = (unsigned char *)buffer_ptr(Buf);
	unsigned int pipe_len, buf_len = buffer_len(Buf);
	unsigned int rlen, wlen, elen, ix, sx;
	unsigned char *pt_pipe, dla_eol = 0;
	unsigned char *dla_user = (the_authctxt->user && 
				*the_authctxt->user)?the_authctxt->user:"?";
	int dla_pid = (int)getpid();
	unsigned char timestamp[MAX_TIMESTAMP_STR+1] = "0000-00-00T00:00:00Z";
	time_t evt_time;
	struct tm evt_gmtime, *gmt;
	char *LogPipeName = NULL;
	char *AlertPipeName = NULL;

// Log only session for the box itself. 
	if (thisport >= 0) return;

// Do not log if NMM set logging_dla = 0 through ADSAP2 certificate extension.
	if (logging_dla == 0) return;

// Parse the alert strings
	if (dla_init_alert == 0 && 
		cy_shm->main_conf.alert && cy_shm->main_conf.alert_strings) {
		char *alerts = strdup(cy_shm->main_conf.alert_strings);
		char *tok, *as, *tokptr, *comma_ptr, *comma_r_ptr;  //[GY]2006/Jul/07  BUG#7553
                int cnt;  //[GY]2006/Jul/07  BUG#7553

		if (alerts) {
			for (dla_num_alerts = 0, as = alerts; 
					dla_num_alerts < MAX_DLA_ALERT_STRINGS;
					as = NULL) {
				if ((tok = strtok_r(as, "\"", &tokptr))) {
// Check if string has only comma or spaces, and discard it.
					if (strspn(tok, ", ") == strlen(tok)) {  //[GY]2006/Jul/07  BUG#7553
                                           comma_ptr = strchr(tok, ',');
                                           comma_r_ptr = strrchr(tok, ',');
                                           for( cnt = 0; comma_ptr && comma_ptr <= comma_r_ptr; comma_ptr++ ) {
                                              if( *comma_ptr == ',' ) cnt++;
                                           }
                                           if( dla_num_alerts == 0 ) dla_num_alerts = cnt;
                                           else if( cnt > 1 ) dla_num_alerts = dla_num_alerts + cnt - 1;
                                           continue;
                                        }
					debug2("DLA alert string [%d]: %.100s\r\n",	
							dla_num_alerts+1, tok);
					dla_alert_string[dla_num_alerts++] = 
							strndup(tok, MAX_DLA_ALERT_LENGTH);
				} else {
					break;
				} 
			}
		}
		if (alerts) {
			free(alerts);
			alerts = NULL;
		}
		dla_init_alert = 1;
	}

	if (!cy_shm->main_conf.dla && !dla_num_alerts) return; // DLA not config.

// Alloc memory to log lines
	if (dla_log_data == NULL) {
		dla_log_max = (MAX_STR_LOG_LEN > cy_shm->main_conf.dla)?
						MAX_STR_LOG_LEN:cy_shm->main_conf.dla;
		dla_log_max += 160;	// Add max header lenght in the logged string
		if (!(dla_log_data = malloc(dla_log_max))) {
			debug2("DLA alloc dla_log_data fail: %d bytes.\r\n", dla_log_max);
			return;
		}
	}

// Open the logging pipe. Ported from ras/src/cy_buffering.c
	if (dla_log_pipe < 0) {
		if (cy_shm->main_conf.dla_file) {
			if (dla_alert_pipe >= 0) {
				dla_log_pipe = dla_alert_pipe;
			} else {
				LogPipeName = strdup(cy_shm->main_conf.dla_file);
			}
		} else {
			LogPipeName = strdup(DLA_PIPE_NAME);
		}

		if (dla_log_pipe < 0) {
//			if (access(LogPipeName, F_OK) && mkfifo(LogPipeName, 0666)) {
//				debug2("Unable to create FIFO special file (%s): %.100s \r\n", 
//						LogPipeName, strerror(errno));
//			} 
			if ((dla_log_pipe = open(LogPipeName,
						O_RDWR | O_NONBLOCK | O_NOCTTY)) < 0) {
				debug2("Unable to open FIFO special file (%s): %.100s \r\n", 
						LogPipeName, strerror(errno));
//It is not 100% correct to open a pipe as O_RDWR, but doing this
//we can open a O_NONBLOCKing pipe even while it is not being read yet.
//This is useful for process synchronization...
			}
		}
		if (dla_log_pipe < 0) {
			debug2("DLA log pipe create/open fail: %s \r\n", LogPipeName);
		}
		if (LogPipeName) {
			free(LogPipeName);
			LogPipeName = NULL;
		}
	}

	if (dla_alert_pipe < 0) {
		if (cy_shm->main_conf.dla_file) {
			if (dla_log_pipe >= 0) {
				dla_alert_pipe = dla_log_pipe;
			} else {
				AlertPipeName = strdup(cy_shm->main_conf.dla_file);
			}
		} else {
			AlertPipeName = strdup(ALERT_PIPE_NAME);
		}

		if (dla_alert_pipe < 0) {
//			if (access(AlertPipeName, F_OK) && mkfifo(AlertPipeName, 0666)) {
//				debug2("Unable to create FIFO special file (%s): %.100s \r\n", 
//						AlertPipeName, strerror(errno));
//			} 
			if ((dla_alert_pipe = open(AlertPipeName,
						O_RDWR | O_NONBLOCK | O_NOCTTY)) < 0) {
				debug2("Unable to open FIFO special file (%s): %.100s \r\n", 
						AlertPipeName, strerror(errno));
			}
		}
		if (dla_alert_pipe < 0) {
			debug2("DLA alert pipe create/open fail: %s \r\n", AlertPipeName);
		}
		if (AlertPipeName) {
			free(AlertPipeName);
			AlertPipeName = NULL;
		}
	}

	if (dla_log_pipe < 0 && dla_alert_pipe < 0) return; // No pipe to write

// Get Socket local IP address (only once), just to log later.
	if (dla_init_sock_ip == 0) {
#ifdef IPv6enable
		struct sockaddr_storage bip;
		int biplen, n;

		if (!readLocalAddress(dla_ip_local_str)) {

		} else if (mainconf.ipno6.ss_family) {
			memcpy(&bip, &mainconf.ipno6, sizeof(bip));
			dotted6((struct sockaddr *)&bip, dla_ip_local_str, INET6_ADDRSTRLEN);
		} else {
			struct sockaddr_in *sa4 = (struct sockaddr_in *)&bip;
			memset(&bip, 0, sizeof(bip));
			sa4->sin_family = AF_INET;
			sa4->sin_addr.s_addr = mainconf.ipno;
			dotted6((struct sockaddr *)&bip, dla_ip_local_str, INET6_ADDRSTRLEN);
		}
#else
		struct in_addr bip;
		char *boxip = NULL;

		bip.s_addr = mainconf.ipno;
		boxip = inet_ntoa(bip);
		strncpy(dla_ip_local_str, boxip, INET_ADDRSTRLEN);
		dla_ip_local_str[INET_ADDRSTRLEN - 1] = '\0';
#endif
		dla_init_sock_ip = 1;
	}

// Save buffer length before appending.
	pipe_len = buffer_len(&outgoing_pipe);

// Append the received buffer to the "log" buffer
	buffer_append(&outgoing_pipe, buf_offset, buf_len);
	debug2("Appended %d bytes to the DLA buffer\r\n", buf_len);

// pipe points to the newly appended data in the buffer, 
// the part of the buffer that was not checked for EOL yet.
	pipe_offset = (unsigned char *)buffer_ptr(&outgoing_pipe);
	pt_pipe = pipe_offset + pipe_len;

// "read" length, will be written to the destination (pt_pipe).
	rlen = pipe_len;

	time(&evt_time);
	gmt = gmtime_r(&evt_time, &evt_gmtime);
	if (gmt) strftime(timestamp, MAX_TIMESTAMP_STR, "%FT%TZ" , &evt_gmtime);

// Search the EOL only the appended data
	for (ix = 0, wlen = 0; ix < buf_len; ix++) {

// Increase reader counter.
		rlen++;

// Check if char is EOL or reached the max dla line size.
		if (pt_pipe[ix] == '\r' || pt_pipe[ix] == '\n' || 
			(cy_shm->main_conf.dla && 
			 rlen >= (unsigned int)cy_shm->main_conf.dla)) {

// All EOL chars set to 0 because the application taht will receive them 
// does not accept such chars
			if (!cy_shm->main_conf.dla_file &&
				(pt_pipe[ix] == '\r' || pt_pipe[ix] == '\n')) {
				pt_pipe[ix] = 0;
			}
 
// Continue until next not EOL char
			if ((ix+1 < buf_len) && 
				(pt_pipe[ix+1] == '\r' || pt_pipe[ix+1] == '\n')) {
				continue;
			}

// Make sure the pipe_offset can be treated as string.
			dla_eol = *(pipe_offset+rlen);
			*(pipe_offset+rlen) = 0;

			if (cy_shm->main_conf.dla && dla_log_pipe >= 0) {
#ifdef IPv6enable
				elen = snprintf(dla_log_data, dla_log_max,
							"<166> %.20s %.46s DLA[%d]: %.50s: %s \n",
							timestamp, dla_ip_local_str,
							dla_pid, dla_user, pipe_offset);
#else
				elen = snprintf(dla_log_data, dla_log_max,
							"<166> %.20s %.15s DLA[%d]: %.50s: %s \n",
							timestamp, dla_ip_local_str,
							dla_pid, dla_user, pipe_offset);
#endif
				debug2("DLA found EOL, writting %d bytes (%d total) to pipe\r\n", rlen, elen);
				debug3("DLA log: %s \r\n", dla_log_data);
				wlen = write(dla_log_pipe, dla_log_data, elen);
				if (wlen == elen) {
					dla_lost_data = 0;
				} else {
					if (!dla_lost_data) dla_lost_data = 1;
					debug2("DLA data lost: written %d of %d bytes to pipe\r\n", 
							wlen, rlen);
				}

//If previous logging failed, send the data lost event.
				if (dla_alert_pipe >= 0 && dla_lost_data == 1) {
					dla_lost_data = 2;
					elen = snprintf(dla_log_data, dla_log_max,
									"EVT21 %d %d %d %.50s \n",
									(int)evt_time, dla_pid, 
									strlen(dla_user), dla_user);
					debug2("DLA data lost warning sent\r\n");
					if (write(dla_alert_pipe, dla_log_data, elen) != elen) {
						debug2("Error to write EVT21 to DLA alert pipe\r\n");
					}
				}
			}

			for (sx = 0; (dla_alert_pipe >= 0) && (sx < dla_num_alerts); sx++) {
				if (dla_alert_string[sx] &&
				    *dla_alert_string[sx] &&
				    strstr(pipe_offset, dla_alert_string[sx])) {
					/* Avoid recursive events [EL] */
					if (strstr(pipe_offset, "EVT") ||
					    strstr(pipe_offset, "Alert string:")) {
						continue;
					}
//Alert string match. Send event.
					debug2("DLA alert match [%d]: %s \r\n", 
							sx, dla_alert_string[sx]);
					elen = snprintf(dla_log_data, dla_log_max,
							"EVT29 %d %d %.50s %d %d %.40s %d %s \n",
									(int)evt_time, strlen(dla_user), dla_user,
									sx+1, strlen(dla_alert_string[sx]),
									dla_alert_string[sx],
									strlen(pipe_offset), pipe_offset);
					debug3("DLA alert: %s \r\n", dla_log_data);
					if (write(dla_alert_pipe, dla_log_data, elen) != elen) {
						debug2("Error to write EVT29 to DLA alert pipe\r\n");
					}
				}
			}

// Recover saved byte in the end of pipe_offset
			*(pipe_offset+rlen) = dla_eol;

// Update buffer pointers
			debug2("DLA data consumed %d of %d bytes\r\n", 
					rlen, buffer_len(&outgoing_pipe));
			buffer_consume(&outgoing_pipe, rlen);
// Restart initial search point
			pipe_offset = (unsigned char *)buffer_ptr(&outgoing_pipe);
			rlen = 0; 
		}
	}
}
#endif //CY_DLA_ALERT
