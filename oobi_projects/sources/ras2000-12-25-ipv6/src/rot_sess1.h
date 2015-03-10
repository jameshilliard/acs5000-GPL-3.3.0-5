int EscWriteBuffer(int SockFd, unsigned char * Buffer, unsigned int BSize, unsigned int isOutBPos, int ttyFd, int sess);

int send_msg_rw_sess(char *buf, int cnt, int direction, int port);

void send_msg_this_sess(char *buf, int cnt, CY_SESS *sess);

int read_rw_msg(CY_SESS *sess, char *buf, int count);

int max_sess_free_buf(int cnt, int port);

void set_flow_off_all_session(int port);

