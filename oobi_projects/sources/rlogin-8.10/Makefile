#
# Makefile    Makefile for rlogin-8.10 for Linux.
#
# Version:    @(#)Makefile  1.00  10-Jul-1995  MvS.
#

CFLAGS += -I/usr/include/bsd -DTERMIOS -DBSD=43

PROG    = rlogin-radius

ifeq ($(IPv6enable),yes)
OBJS    = rlogin.o rcmd/rcmd6.o
else
OBJS    = rlogin.o rcmd/rcmd.o
endif

#ifneq ($(wildcard /usr/lib/libbsd.a),)
#LDLIBS += -lbsd
#endif
#ifneq ($(wildcard /usr/lib/libnsl.a),)
LDLIBS += -lnsl
#endif
#LDLIBS += -lkrb -ldes

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LDLIBS)

ifeq ($(IPv6enable),yes)
rcmd/rcmd6.o: rcmd/rcmd6.c
	cd rcmd && $(MAKE)
else
rcmd/rcmd.o: rcmd/rcmd.c
	cd rcmd && $(MAKE)
endif

install: $(PROG)
	install -s -m 4511 $(PROG) $(BINDIR)

clean:
	$(RM) -rf *.o *.a *.s rcmd/*.o

clobber:
	$(RM) -rf *.o *.a *.s rcmd/*.o $(PROG)

