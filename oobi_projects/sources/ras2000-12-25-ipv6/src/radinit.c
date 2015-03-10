/*
 * radinit	Initialize the /var/log/radsession.id file
 *		on each boot.
 *
 * Version:	@(#)radinit  1.00  15-Jul-1996  miquels@cistron.nl
 *
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <time.h>

#include "../pslave_cfg.h"

/*
 *	Create a new session id.
 */
unsigned int rad_sessionid(char *s)
{
	int fd, n;
	unsigned int i;
	char id[32];

	if ((fd = open(RAD_SESSIONID_FILE, O_RDWR|O_CREAT, 0644)) < 0) {
		perror(RAD_SESSIONID_FILE);
		return -1;
	}
	for(i = 0; i < 10; i++) {
		if (i > 0) usleep(200000);
		if (flock(fd, LOCK_EX) == 0)
		        break;
	}
	if (i == 10) {
		fprintf(stderr, "rad_sessionid: failed to lock %s\n",
			RAD_SESSIONID_FILE);
		return -1;
	}

	n = read(fd, id, 31);
	if (n < 0) n = 0;
	id[n] = 0;
	i = 0;
	sscanf(id, "%x", &i);
#ifndef CYCLADES
	if (s == NULL)
		i += 0x01000000;
	else
		i++;
	i &= 0xFFFFFFFF;
	if (i == 0) i++;
#else
	i |= 0x000fffff; 
#endif
	snprintf(id, sizeof (id), "%08x\n", i);
	(void)ftruncate(fd, 0L);
	(void)lseek(fd, 0L, SEEK_SET);
	(void)write(fd, id, strlen(id));

	flock(fd, LOCK_UN);
	close(fd);

	if (s) {
		strcpy(s, id);
		s[8] = 0;
	}
	return 0;
}

int main(void)
{
	rad_sessionid(NULL);
	return 0;
}

