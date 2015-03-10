#ifndef ROT_SESS_H
#define ROT_SESS_H 1

#define SNIF_ESCAPE 0x1A

#define MaxMsgSize 2044

struct MsgBuf {
    union {
        long    mtype;
        char    ctype[4];
    } mt;
    char mtext[MaxMsgSize+4];
};

struct TmpBuf {
	char tmp_buf[400];
	int ix_buf;
	int cnt_buf;
};

extern int DeviceFd;
extern int got_signal_int;

int read_sniff_msg(CY_SESS *sess);
void close_sniff_session(CY_SESS *sess);
int check_mult_session(int location, char *msg, int rmethod(int nonblock_flag), void wmethod(char *line, int cnt), int main_routine(struct auth *p));
int check_sniff_options(char opt, CY_SESS *sess);

void do_virtual_session(int location,
			int rmethod(int nonblock_flag), void wmethod(char *line, int cnt));
int open_virtual_session(int location,
			int rmethod(int nonblock_flag), void wmethod(char *line, int cnt));

int count_sessions (int port_num);
CY_SESS *first_rw_session (int port_num);
CY_SESS *first_sniff_session (int port_num);

void kill_sniff_session(int sess, int flag, int clean);
void kill_session(CY_SESS *sess, int flag);
void kill_session_nowait(CY_SESS *sess, int flag);
void kill_session_list (int port_num, int flag);
void kill_session_list_nowait (int port_num, int flag);
void kill_session_pid (int port_num, int pid);
void clean_session_entry (CY_SESS *sess);
void free_session_entry (CY_SESS *sess);
CY_SESS *alloc_session_entry (int port, int flag, char *username);

void send_msg_this_sess(char *buf, int cnt, CY_SESS *sess);
int open_rw_session(int location, int rmethod(int nonblock_flag), void wmethod(char *line, int cnt), int main_routine(struct auth *p));
int read_rw_msg(CY_SESS *sess, char *buf, int count);
int write_rw_msg(CY_SESS * sess, char *buf, int len);

char *sock_show_sess(int ind);
int get_param_method(char *f, int n, int *p);

#endif
