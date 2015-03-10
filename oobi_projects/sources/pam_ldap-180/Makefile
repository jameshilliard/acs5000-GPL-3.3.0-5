#
# Makefile for pam_ldap
#

include $(PAM_DIR)/Make.Rules

CFLAGS = -O2 -Wall -fPIC $(OS_CFLAGS)

srcdir = .
top_srcdir = .
PREFIX = $(TARGET_DIR)
DESTDIR = $(TARGET_DIR)/lib/security

LDFLAGS = -shared -Wl,-Bdynamic -Wl,--version-script,$(srcdir)/exports.linux
CPPFLAGS =  -DLDAP_REFERRALS -D_REENTRANT -I$(LDAP_DIR)/include -I$(TARGET_DIR)/usr/include -I.
LFLAGS = -L$(LDAP_DIR)/lib -L$(TARGET_DIR)/lib -L$(OPEN_SSL_DIR)/lib -Wl,-rpath,/lib
LIBS = -lldap -llber -llutil -lssl -lcrypto -lnsl -lcrypt -lresolv -lpam -ldl

####################################################################
# No changes below this line

SRCS = pam_ldap.c pam_ldap.h md5.c md5.h

OBJS = pam_ldap.o md5.o

all: pam_ldap.so.unstripped
	$(CROSS_COMPILE)strip --remove-section=.note --remove-section=.comment \
	pam_ldap.so.unstripped -o pam_ldap.so

pam_ldap.so.unstripped: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(LFLAGS) $(OBJS) $(LIBS)

install:
	cp pam_ldap.so $(DESTDIR)/.
	#chown root:sys $(DESTDIR)/pam_ldap.so
	rm ./*.unstripped
	${INSTALL} -m 644 ldap.conf $(FAKEROOT)/etc

clean:
	rm -f *.so *.o *.unstripped

pam_ldap.o: pam_ldap.c pam_ldap.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

md5.o: md5.c md5.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<
