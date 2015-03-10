#ifndef __server_h
#define __server_h 1

/* These system headers are needed by
 * other Cyclades headers below [EL] */
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#include "rot_buf.h"
#include "../pslave_cfg.h"
#include "rwconf.h"
#include "cy_shm.h"
#include "auth.h"

#include "server1.h"

#include "rot_sess.h"
#include "rot_sess1.h"
#include "rot_buffer.h"
#include "rot_shm.h"
#include "xdebug.h"
#include "bidirect.h"

#endif	/* __server_h */
