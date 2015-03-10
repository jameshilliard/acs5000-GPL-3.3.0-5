#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <termios.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"

int check_serial_in_use(struct line_cfg *lc)
{
	if (lc->fast) { //[RK]May/22/03 - Performance improvement
		if (!cy_shm->vet_pid[lc->this]) {
			return(0);
		}
		if ((kill(cy_shm->vet_pid[lc->this],0) < 0) && (errno == ESRCH)) {
			return(0);
		}
		return(1);
	}
	return (count_sessions_tst(lc->this));
}

void serial_unlock(int port)
{
	shm_free_str(cy_shm->tty_user[port]);
	cy_shm->tty_user[port] = NULL;
	cy_shm->vet_pid[port] = 0;
}

void serial_lock(int port, char *name)
{
	cy_shm->vet_pid[port] = getpid();
	cy_shm->tty_user[port] = shm_strdup(name);
}

