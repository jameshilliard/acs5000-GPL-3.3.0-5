#
# Created by Andy Berkheimer <andy@tho.org> Jul 30, 1997
# Based on the many other PAM module Makefiles
# Last modified Apr 19, 1998
#
# Copyright (C) 1997-1998, Andrew Berkheimer <andy@tho.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# This Makefile controls a build process of pam_opie module for
# Linux-PAM. You should not modify this Makefile (unless you know
# what you are doing!).
#

ifndef FULL_LINUX_PAM_SOURCE_TREE
export DYNAMIC=-DPAM_DYNAMIC
export CC=gcc
export CFLAGS=-O2 -Dlinux -DLINUX_PAM \
       -ansi -D_POSIX_SOURCE -Wall -Wwrite-strings \
       -Wpointer-arith -Wcast-qual -Wcast-align -Wtraditional \
       -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Winline \
       -Wshadow -pedantic -fPIC
export MKDIR=mkdir -p
export LD_D=gcc -shared -Xlinker -x
export SECUREDIR=/lib/security
export INSTALL=install
endif

TITLE=pam_opie

OPIELIB = -Llibopie -lopie -lc
OPIE = libopie/libopie.a

LIBSRC = $(TITLE).c
LIBOBJ = $(TITLE).o
LIBOBJD = $(addprefix dynamic/,$(LIBOBJ))
LIBOBJS = $(addprefix static/,$(LIBOBJ))

dynamic/%.o : %.c
	$(CC) $(CFLAGS) $(DYNAMIC) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@

static/%.o : %.c
	$(CC) $(CFLAGS) $(STATIC) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@


ifdef DYNAMIC
LIBSHARED = $(TITLE).so
endif

ifdef STATIC
LIBSTATIC = lib$(TITLE).o
endif

####################### don't edit below #######################

all: dirs $(LIBSHARED) $(LIBSTATIC) register

dirs:
ifdef DYNAMIC
	$(MKDIR) ./dynamic
endif
ifdef STATIC
	$(MKDIR) ./static
endif

register:
ifdef STATIC
	( cd .. ; ./register_static $(TITLE) $(TITLE)/$(LIBSTATIC) )
endif

ifdef DYNAMIC
$(LIBOBJD): $(LIBSRC)

$(LIBSHARED):	$(LIBOBJD) $(OPIE)
		$(LD_D) -o $@ $(LIBOBJD) $(OPIELIB)
endif

ifdef STATIC
$(LIBOBJS): $(LIBSRC)

$(LIBSTATIC): $(LIBOBJS) $(OPIE)
	$(LD) -r -o $@ $(LIBOBJS) $(OPIELIB)
endif

$(OPIE): libopie/*.c libopie/*.h
	(cd libopie; make)

install: all
	$(MKDIR) $(FAKEROOT)$(SECUREDIR)
ifdef DYNAMIC
	$(INSTALL) -m 644 $(LIBSHARED) $(FAKEROOT)$(SECUREDIR)
endif

remove:
	rm -f $(FAKEROOT)$(SECUREDIR)/$(TITLE).so

clean:
	rm -f $(LIBOBJD) $(LIBOBJS) core *~
	(cd libopie; make clean)

extraclean: 
	rm -rf core *~ *.a *.o *.so *.bak dynamic static
	(cd libopie; make realclean)

.c.o:	
	$(CC) $(CFLAGS) -c $<
