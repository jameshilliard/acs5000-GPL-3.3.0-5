#ifndef BIDIRECT_H
#define BIDIRECT_H
#include "rwconf.h"
#include "cy_shm.h"
#include "rot_shm.h"
#include "auth.h"

/* connection mode for bidirect telnet */
#define ENTER_TCP		8
#define TCP_CONNECT     4
#define TERM_CONNECT    2
#define IDLE_CONNECT    1
#define DEF_CONNECT     0


#ifdef BIDIRECT
/* macros for bidirect telnet */
#define GET_BIDIR_TCP(x,y, semn)  \
   ((x == P_TELNET_BIDIRECT) && (shm_get_set_val(0, &y, 0, semn) & TCP_CONNECT))
#define SET_BIDIR_TCP(x,y, semn) (shm_get_set_val(TCP_CONNECT, &y, 1, semn))

#define GET_BIDIR_ENTTCP(x,y, semn)  \
   ((x == P_TELNET_BIDIRECT) && (shm_get_set_val(0, &y, 0, semn) & ENTER_TCP))
#define SET_BIDIR_ENTTCP(x,y, semn) (shm_get_set_val(ENTER_TCP, &y, 1, semn))

#define GET_BIDIR_IDLE(x,y,semn) \
  ((x == P_TELNET_BIDIRECT) && (shm_get_set_val(0, &y, 0, semn) & IDLE_CONNECT))
#define SET_BIDIR_IDLE(x,y, semn) (shm_get_set_val(IDLE_CONNECT, &y, 1, semn))

#define GET_BIDIR_LOGIN(x,y,semn) \
  ((x == P_TELNET_BIDIRECT) && (shm_get_set_val(0, &y, 0, semn) & TERM_CONNECT))
#endif
#define SET_BIDIR_LOGIN(x,y,semn) \
   (shm_get_set_val(TERM_CONNECT, &y, 1, semn))

#ifdef BIDIRECT
#define GET_BIDIR_INIT(x,y,semn) \
  ((x == P_TELNET_BIDIRECT) && (shm_get_set_val(0, &y, 0, semn) == DEF_CONNECT))
#define SET_BIDIR_INIT(x,y,semn) \
  (shm_get_set_val(DEF_CONNECT, &y, 1, semn))

#define NOT_BIDIR_TCP(x,y,semn) \
	((x != P_TELNET_BIDIRECT) || GET_BIDIR_INIT(x,y,semn) || \
	GET_BIDIR_LOGIN(x,y,semn) || GET_BIDIR_IDLE(x,y,semn))
#endif
#endif /* BIDIRECT_H */	
