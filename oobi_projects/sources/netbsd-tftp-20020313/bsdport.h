#ifndef __BSDPORT_H
#define __BSDPORT_H

#include <string.h>

/*
 * [RGB Sep 13 2007]
 *
 * This file contains some functions to ease the task of porting from BSD to
 * Linux.
 *
 */

#if !HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif

#if !HAVE_SS_LEN
struct sockaddr;
size_t get_sockaddr_len(struct sockaddr *addr);
#endif

#if HAVE_SS_LEN
#define SS_LEN(ssptr) (ssptr)->ss_len
#define SA_LEN(saptr) (saptr)->sa_len
#define IF_HAVE_SS_LEN(txt) txt
#else
/* Must pass a pointer to an object. */
#define SS_LEN(ssptr) get_sockaddr_len((struct sockaddr *) ssptr)
#define SA_LEN(saptr) get_sockaddr_len((struct sockaddr *) saptr)
#define IF_HAVE_SS_LEN(txt)
#endif

#ifndef UID_MAX
#define UID_MAX 65535
#endif

#ifndef GID_MAX
#define GID_MAX 65535
#endif

#ifndef __dead
#define __dead
#endif

#endif
