#
# Simple Makefile to compile tftp + tftpd
#
# If you want to compile only one of them then you can override the
# variable PROGS in the 'make' command line.
#
# The location where each program will be installed is defined by
# the variables install-<program>
#

PROGS  := tftp in.tftpd
PREFIX :=
EXTRA_CFLAGS :=

install-in.tftpd := $(PREFIX)/usr/sbin/in.tftpd
install-tftp     := $(PREFIX)/usr/bin/tftp

CFLAGS := -DHAVE_STRLCPY=0 -Dlint

# Programs: usually overriden for cross-compiling.
CC	:= gcc
STRIP	:= strip
INSTALL	:= install

#Q:=@echo
Q:=

HEADERS:=tftpsubs.h bsdport.h

.PHONY: all clean install $(foreach prog,$(PROGS),install-$(prog))

all: $(PROGS)

install: $(foreach prog,$(PROGS),install-$(prog))

########################################
#  Install rules:

install-in.tftpd : in.tftpd
	$(Q) $(INSTALL) -v $< $($@)

install-tftp : tftp
	$(Q) $(INSTALL) -v $< $($@)

########################################
#  Build rules:

in.tftpd : tftpd.o tftpsubs.o bsdport.o
	$(Q) $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@.gdb $^
	$(Q) $(STRIP) -g --strip-unneeded -o $@ $@.gdb

tftp  : main.o tftp.o tftpsubs.o bsdport.o
	$(Q) $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@.gdb $^
	$(Q) $(STRIP) -g --strip-unneeded -o $@ $@.gdb

%.o : %.c $(HEADERS)
	$(Q) $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -Wall -o $@ -c $<

clean:
	-rm -fv *.o *.gdb in.tftpd tftp
