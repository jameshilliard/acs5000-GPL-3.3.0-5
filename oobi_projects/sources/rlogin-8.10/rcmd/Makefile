#
# Makefile	Creates a rcmd.o that retries a couple
#		of time on ECONNREFUSED - useful if there
#		are a few hanging rlogins on the target host.
#
#		Needed until this function is patched in the
#		Linux libc.
#
# Version:	@(#)rcmd.c.new-patched  0.01  02-Jan-1995  MvS.
#
ifeq ($(IPv6enable),yes)
rcmd6.o:	rcmd6.c inetprivate.h
		$(CC) -DYP -O2 -c rcmd6.c
else
rcmd.o:		rcmd.c inetprivate.h
		$(CC) -DYP -O2 -c rcmd.c
endif
