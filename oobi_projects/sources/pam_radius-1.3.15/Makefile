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

TITLE=pam_radius_auth
CONFD=$(CONFIGED)/security
export CONFD
CONFILE=$(CONFD)/radius.conf
export CONFILE

#ifeq ($(HAVE_LIBPWDB),yes)

#

LIBSRC = $(TITLE).c
LIBOBJ = $(TITLE).o

LIBOBJD = $(addprefix dynamic/,$(LIBOBJ)) md5.o
#LIBOBJS = $(addprefix static/,$(LIBOBJ))

dynamic/%.o : %.c
	$(CC) $(CFLAGS) $(DYNAMIC) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@

#static/%.o : %.c
#	$(CC) $(CFLAGS) $(STATIC) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@


ifdef DYNAMIC
LIBSHARED = $(TITLE).so
endif

#ifdef STATIC
#LIBSTATIC = lib$(TITLE).o
#endif

####################### don't edit below #######################

all: dirs $(LIBSHARED) $(LIBSTATIC) register

dirs:
ifdef DYNAMIC
	$(MKDIR) ./dynamic
endif
#ifdef STATIC
#	$(MKDIR) ./static
#endif

register:
#ifdef STATIC
#	( cd .. ; ./register_static $(TITLE) $(TITLE)/$(LIBSTATIC) )
#endif

ifdef DYNAMIC
$(LIBOBJD): $(LIBSRC)

$(LIBSHARED):	$(LIBOBJD)
	$(LD_D) -o $@ $(LIBOBJD)

md5.o: md5.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DHIGHFIRST -c $< -o $@

endif

#ifdef STATIC
#$(LIBOBJS): $(LIBSRC)
#
#$(LIBSTATIC): $(LIBOBJS)
#	$(LD) -r -o $@ $(LIBOBJS) -lpwdb
#endif

install: all
ifdef DYNAMIC
	$(INSTALL) -m $(SHLIBMODE) $(LIBSHARED) $(FAKEROOT)$(SECUREDIR)
endif
	${INSTALL} -D -m 600 pam_radius_auth.conf $(FAKEROOT)/etc/raddb/server

remove:
	rm -f $(FAKEROOT)$(SECUREDIR)/$(TITLE).so

clean:
	rm -f $(LIBOBJD) $(LIBOBJS) core *~
	rm -f *.a *.o *.so *.bak dynamic/* static/*
	rm -rf dynamic static

.c.o:	
	$(CC) $(CFLAGS) -c $<

#else
#
#include ../dont_makefile
#
#endif
