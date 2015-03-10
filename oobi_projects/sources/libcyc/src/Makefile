#
# File: Makefile
#
# Copyright (C) 2004-2005 Cyclades Corporation
#
# www.cyclades.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
#
# Author: Erik Lotspeich <erik.lotspeich@cyclades.com>
#
.SUFFIXES:
.SUFFIXES: .c .cc .d .o .so .a
.PHONY:

SHELL=/bin/sh
RANLIB=ranlib

PKG_INSTALL_DIR=$(TARGET_DIR)

include libcyc.mk

SHELL=/bin/sh
CFLAGS+=-I. -I../include -D_REENTRANT -D$(DEFTARG)
CXXFLAGS+=-I. -I../include -D_REENTRANT -D$(DEFTARG)
LDFLAGS=-lpthread

LIB_DSO=libcyc.so
LIB_A=libcyc.a
sources=libcyc_main.c libcyc_malloc.c libcyc_ll.c \
	libcyc_ipc.c libcyc_util.c libcyc_kvm.c \
	libcyc_thread.c libcyc_apctl.c md5.c
objects=$(sources:.c=.o)

LIBPP_DSO=libcyc++.so
LIBPP_A=libcyc++.a
pp_sources=libcyc++_main.cc libcyc++_signals.cc \
	libcyc++_ipc.cc libcyc++_thread.cc
pp_objects=$(pp_sources:.cc=.o)

all: $(LIB_A) $(LIB_DSO) $(LIBPP_A) $(LIBPP_DSO)

debug: CFLAGS+=-g -ggdb
debug: all

-include $(sources:.c=.d)
-include $(pp_sources:.cc=.d)

$(LIB_DSO): $(objects)
$(LIB_A): $(objects)
$(LIBPP_DSO): $(pp_objects)
$(LIBPP_A): $(pp_objects)

test: test.c ../include/cyc.h $(LIB_A)
	$(CC) $(CFLAGS) -o $@ $< $(LIB_A) $(LDFLAGS)

clean:
	-rm -f *.so *.o *.a *~ test

distclean: clean
	-rm -f *.d

install: all
	mkdir -p $(PKG_INSTALL_DIR)/usr/lib
	mkdir -p $(PKG_INSTALL_DIR)/usr/bin
	$(STRIP) $(LIB_DSO)
	$(STRIP) $(LIBPP_DSO)
	cp $(LIB_DSO) $(PKG_INSTALL_DIR)/usr/lib/$(LIB_DSO).3.0
	cp $(LIBPP_DSO) $(PKG_INSTALL_DIR)/usr/lib/$(LIBPP_DSO).0.0
	if [ ! -h $(PKG_INSTALL_DIR)/usr/lib/$(LIB_DSO) ]; then \
		cd $(PKG_INSTALL_DIR)/usr/lib ; ln -s $(LIB_DSO).3.0 \
		$(LIB_DSO); \
	fi
	if [ ! -h $(PKG_INSTALL_DIR)/usr/lib/$(LIBPP_DSO) ]; then \
		cd $(PKG_INSTALL_DIR)/usr/lib ; ln -s $(LIBPP_DSO).0.0 \
		$(LIBPP_DSO); \
	fi
