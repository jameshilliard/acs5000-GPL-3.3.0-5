#ifndef ROT_BUFFER_H
#define ROT_BUFFER_H

# ifdef __ROT_BUFFER__
# define EXTERN
# else
# define EXTERN extern
# endif

#include <stdio.h>   //[GY]2006/May/05

#define MAX_BUFFER     1024
#define MAX_XML_BUFFER 512
#define MAX_DLS_BUFFER 2048

#define DLS_PRI   166
#define DLS_TIMER 500

#define DLS_MAX_HLEN 256

#define DLS_ST_OK       0
#define DLS_ST_PIPEFULL 1
#define DLS_ST_TIMESEND 2

enum {
Xml_Search_FirstOpenBracket,    // <
Xml_Search_QuestionMark,        // ?
Xml_Search_x,                   // x
Xml_Search_m,                   // m
Xml_Search_l,                   // l
Xml_Search_OpenBracket,         // <
Xml_Search_TagFirstChar,        // ?, \,  or any other char
Xml_Search_CloseBracket         // >
};

struct log_data_buffer {
	int   read_buf;
	int   tot_buf;
	char  buffer[MAX_BUFFER+2];
	char  xml_buffer[MAX_XML_BUFFER];
	int   xml_buffer_pos;
	int   xml_state;
	int   xml_tag_cnt;
        char  alert_strings[ALERT_STRINGS_QUANTITY][ALERT_STRINGS_MAX_CHAR+1]; //string match for port alert [GY]2006/Apr/26
	char  dls_buffer[MAX_DLS_BUFFER];
	int   dls_buffer_pos;
	int   dls_status;
	int   msgtimer;
};

typedef struct log_data_buffer LOG_DATA_BUFFER;

/* Server authentication */

extern LOG_DATA_BUFFER *curr_log;

EXTERN int	fd_buf;
EXTERN int	terminal_lines;
EXTERN void	trunc_file(int fd, int lpos, int rpos, struct line_cfg *pconf);
EXTERN void	stop_start_buffering(struct line_cfg *pconf, int flag);
EXTERN void	check_file_size(int fd, char *buffer, int cnt, int flag, struct line_cfg *pconf, int port);
EXTERN int	open_data_buffering(struct line_cfg *pconf);
EXTERN int	check_buffering_retention_mode(struct line_cfg *pconf, LOG_DATA_BUFFER *lbuf, CY_SESS *sess);
EXTERN void	more(int, int, int (*rmethod)(), void (*wmethod)());
EXTERN int	get_a_line(int, char *, int);
EXTERN FILE     *snmp_pipe_FP;  //[GY]2006/May,05
EXTERN int      snmp_pipe_flag;  //[GY]2006/May,05
#ifdef DLSenable
EXTERN void handle_dls_buffer(char* buffer, int cnt, int port);
#endif

# undef EXTERN

#endif
