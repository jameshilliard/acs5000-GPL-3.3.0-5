#
# Simple Makefile to compile a dummy tcpd program.
#
# The location where the program will be installed is defined
# by the variable install-<program>
#

PROGS  := tcpd
PREFIX :=
EXTRA_CFLAGS :=

install-tcpd := $(PREFIX)/usr/sbin/tcpd

# Programs: usually overriden for cross-compiling.
CC	:= gcc
STRIP	:= strip
INSTALL	:= install

# C compiler flags: not usually overriden.
CFLAGS  := -Wall -DFACILITY=LOG_MAIL

#Q:=@echo
Q:=

.PHONY: all clean install $(foreach prog,$(PROGS),install-$(prog))

all: $(PROGS)

install: $(foreach prog,$(PROGS),install-$(prog))

########################################
#  Install rules:

install-tcpd : tcpd
	$(Q) $(INSTALL) -v -m 755 $< $($@)

########################################
#  Build rules:

tcpd  : tcpd.o
	$(Q) $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@.gdb $^
	$(Q) $(STRIP) -g --strip-unneeded -o $@ $@.gdb

%.o : %.c
	$(Q) $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

clean:
	-rm -fv *.o *.gdb tcpd
