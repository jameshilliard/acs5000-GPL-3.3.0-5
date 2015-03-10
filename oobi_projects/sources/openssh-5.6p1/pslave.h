#ifndef EXTERN
#  define EXTERN extern
#endif

# ifndef __PSLAVE__
extern int do_server_auth;
extern int thisport;
extern int logging_dla;
extern int terminal_lines;
extern int cyc_alloc_tty;
extern int ssh2_channel_id;
extern int ssh_remote_id;
extern int cyc_pid_child;
#else
int do_server_auth = AUTH_LOCAL;
int thisport = -1;
int logging_dla = 1;
int terminal_lines;
int cyc_alloc_tty=0;
int ssh2_channel_id;
int ssh_remote_id;
int cyc_pid_child=0;
# endif

// reset the shared memory field of this session
void cyc_close_sess(void);

// init the shared memory
void cyc_shm_init();

// verify if the console is configured as SOCKET_SSH
int cyc_check_term_config(char *pterm);
int cyc_check_local_ip(void);

// copy the username to AUTH struct
void cyc_copy_user_pai(char *name);

// return the service name for PAM authentication
char *cyc_get_pam_service();

// set global variables for monitor - mm_register_thisport
void cyc_session_set_global(int port);

#ifdef GRPAUTHenable
// verify the user authorization : sXX.users
void cyc_set_group_name(char * group_name);
int cyc_check_access(char * name,int ppid);
#endif

// save the user password - ts_menu second authentication
void cyc_save_userp(char * p);
// set the correct PID
void cyc_pam_pwd_ctrl();

// allocate the console port
int cyc_allocate_port(int *ptyfd, int *ttyfd);

// open one SSH session
int cyc_create_session_tty(void);

// send break to tty
void session_set_sendbreak(int break_interval);

// check data buffering
void cyc_check_RetentionMode(int ver);

int cyc_set_fdout(int ptyfd,int ttyfd);
void cyc_set_vet_pool(void);

// SERVERLOOP routines
// check DCD signal
int cyc_check_dcd(void);
// ignore the data from TTY to SSH client if in Retention mode
int cyc_ignore_data_buffer(int len);
// check signal 
int cyc_check_signal(int selret);
// calculate the max_time_milliseconds
int cyc_sniff_max_time();
//check serial port is fast or virtual
int cyc_check_fast();
// finish the SSH section
void cyc_finish_ssh();

// analyse the data from SSH client to TTY
int cyc_ssh_tty_data(Buffer *Buf, int fd, fd_set * writeset);

// control the SSH session idle timeout
int cyc_check_idle_time(int milliseconds, int reset);

// write the event notif : authentication fail
void cyc_wrtevt_authfail(char *);

