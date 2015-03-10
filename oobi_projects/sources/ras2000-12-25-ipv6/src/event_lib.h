/*****************************************************************
File: event_lib.h

- openevt, closeevt, writeevt  - send messages to the event 
notification (event_gen) daemon.

void openevt(void);
void writeevt(int event_number, char * format, va_list parameters);
void closeevt(void);

Description 

- closeevt() closes the descriptor being sued to write to
  the fifo device (/var/run/snmp_pipe)

- openevt() opens a connection to the fifo device

- writeevt generates a event notification message, which
  will be written to the fifo device.
  Parameters :
	. event_number : number of the event
	. format : type of the parameters : 'i' = interger
                                            's' = string
					    'u' = username
                   Ex. "iss" : interger, string, string
        . va_list : list of the parameters for the evnet
****************************************************************/

#ifndef EVENT_LIB_H
#define EVENT_LIB_H 1

#ifdef __cplusplus
extern "C" {
#endif

int openevt(void);
void closeevt(void);
void writeevt(int, char *,...);

#ifdef __cplusplus
}
#endif

#endif /* EVENT_LIB_H */
