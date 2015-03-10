#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"

#ifdef WINEMSenable
void initXml(LOG_DATA_BUFFER *buffer)
{
 	buffer->xml_state = Xml_Search_FirstOpenBracket;
}

int find_xw(char *buffer, int cnt)
{
	int pos = curr_log->xml_buffer_pos;

	while (cnt && (pos < MAX_XML_BUFFER-1)) {
		switch(curr_log->xml_state) {
		case Xml_Search_FirstOpenBracket:
			if (*buffer != '<') {
				break;
			}
			curr_log->xml_state = Xml_Search_QuestionMark;
			break;
		case Xml_Search_QuestionMark:
			if (*buffer != '?') {
				curr_log->xml_state = Xml_Search_FirstOpenBracket;
				break;	
			}
			curr_log->xml_state = Xml_Search_x;
			break;
		case Xml_Search_x:
			if (*buffer != 'x') {
				curr_log->xml_state = Xml_Search_FirstOpenBracket;
				break;
			}
			curr_log->xml_state = Xml_Search_m;
			break;
		case Xml_Search_m:
			if (*buffer != 'm') {
				curr_log->xml_state = Xml_Search_FirstOpenBracket;
				break;
			}
			curr_log->xml_state = Xml_Search_l;
			break;
		case Xml_Search_l:
			if (*buffer != 'l') {
				curr_log->xml_state = Xml_Search_FirstOpenBracket;
				break;
			}
			curr_log->xml_buffer[0] = '<';
			curr_log->xml_buffer[1] = '?';
			curr_log->xml_buffer[2] = 'x';
			curr_log->xml_buffer[3] = 'm';
			curr_log->xml_buffer[4] = 'l';
			pos = 5;
			curr_log->xml_state = Xml_Search_OpenBracket;
			curr_log->xml_tag_cnt = 0;
			break;
		case Xml_Search_OpenBracket:
			if (*buffer == '<') {
				curr_log->xml_state = Xml_Search_TagFirstChar;
			}
			if (isprint(*buffer)) {
				curr_log->xml_buffer[pos] = *buffer;
				pos++;
			}
			break;
		case Xml_Search_TagFirstChar:
			if (*buffer == '?') { // <?x
				curr_log->xml_state = Xml_Search_x;
				break;
			} else if (*buffer == '/') {
				curr_log->xml_tag_cnt--;
			} else {
				curr_log->xml_tag_cnt++;
			}
			if (isprint(*buffer)) {
				curr_log->xml_buffer[pos] = *buffer;
				pos++;
			}
			curr_log->xml_state = Xml_Search_CloseBracket;
			break;
		case Xml_Search_CloseBracket:
			if (*buffer == '<') {
				curr_log->xml_state = Xml_Search_FirstOpenBracket;
				break;
			} else if (*buffer == '>') {
				if (curr_log->xml_tag_cnt == 0) {
					// got everything
					curr_log->xml_buffer[pos] = *buffer;
					pos++;
					curr_log->xml_buffer[pos] = 0;
					curr_log->xml_buffer_pos = 0;
					curr_log->xml_state = Xml_Search_FirstOpenBracket;
					return 1;
				} else {
					curr_log->xml_state = Xml_Search_OpenBracket;
				}
			}
			if (isprint(*buffer)) {
				curr_log->xml_buffer[pos] = *buffer;
				pos++;
			}
			break;
		}
		buffer++;
		cnt--;
	}
	if (pos == MAX_XML_BUFFER-1) {
		XPRINTF("xml data exceeded 511 bytes.\n");
		pos = 0;
		curr_log->xml_state = Xml_Search_FirstOpenBracket;
	}
	curr_log->xml_buffer_pos = pos;
	return 0;
}
#endif
