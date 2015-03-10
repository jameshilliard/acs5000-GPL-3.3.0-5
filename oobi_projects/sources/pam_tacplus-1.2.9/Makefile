
#
# This Makefile controls a build process of $(TITLE) module for
# Linux-PAM. You should not modify this Makefile (unless you know
# what you are doing!).
#
# Created by Cristian Gafton <gafton@redhat.com> 1996/09/10
#
# STATIC modules are not supported
#

include $(Linux-PAM)/Make.Rules

TITLE = pam_tacplus
LIBTAC=./libtac
LIBOBJ= support.o pam_tacplus.o
LIBOBJD = $(addprefix dynamic/,$(LIBOBJ))

CFLAGS += -I$(LIBTAC)/include
LIBS= -ltac -L$(LIBTAC)/lib $(LDFLAGS)

dynamic/%.o : %.c
	$(CC) $(CFLAGS) $(DYNAMIC) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@

LIBSHARED = $(TITLE).so

####################### don't edit below #######################

all: dirs $(LIBSHARED)

dirs:
	$(MKDIR) ./dynamic

$(LIBOBJD): $(LIBSRC)

$(LIBSHARED): $(LIBTAC) $(LIBOBJD) $(LIBTAC)/lib/libtac.a
	$(LD_D) -o $@ $(LIBOBJD) $(LIBS)

clean:
	rm -f $(LIBOBJD) $(LIBOBJS) core *~
	rm -f *.a *.o *.so *.bak dynamic/*
	rm -rf dynamic
	make -C $(LIBTAC) clean

$(LIBTAC): FORCE
	make -C $(LIBTAC)

FORCE: ;   

install: all
ifdef DYNAMIC
	$(INSTALL) -m $(SHLIBMODE) $(LIBSHARED) $(FAKEROOT)$(SECUREDIR)
endif

