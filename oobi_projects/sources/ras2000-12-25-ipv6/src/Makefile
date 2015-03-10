OBJS:=main.o slip.o ctlp-subs.o

LIBRARIES:=-lcrypt -L$(PREFIX)/lib -lpam -lpam_misc

LIBOBJS:=radclient.o lib.o lib1.o chat.o md5.o syslog.o utmp.o auth.o rot_getty.o getty.o rot_shm.o rot_buffer.o rot_buf.o xwras.o rot_sess.o tstr.o xdebug.o rot_ttys.o rot_sess1.o rot_sess.o rot_sess2.o chkUsers.o config_lock.o check_serial.o event_lib.o rwconf.o do_login.o spawnit.o

LIBA:=librad.a

CFLAGS+=-D$(DEFTARG) -I. -I$(LIBCYC_DIR)/include -I$(WEBAPI_DIR)

ifeq ($(filter $(TARGET_PRODUCT),kvm kvmnetv2 kvmapc ons),$(TARGET_PRODUCT))
CFLAGS+=-I$(KVMNET_UTIL_DIR)/include
endif

LDFLAGS+=-L.

ifeq ($(filter $(TARG),kvm kvmapc kvmnetv2 kvmmax),$(TARG))
CFLAGS+=-DKVM
endif

ifeq ($(TARG),ts1h)
CFLAGS+=-DCONFIG_TS1H
endif

ifeq ($(TARG),ons)
CFLAGS+=-DONS
endif

ifeq ($(origin BIN_TARGETS),undefined)  
BIN_TARGETS:=portslave 
endif

LIB_TARGETS:=libsconf.so

ifeq ($(PMDenable),yes)
CFLAGS+=-I$(PMCLIB_DIR)/include -I$(PMCLIB_DIR)/gsp-include
LIBCYPMC:=libcypmc.so
LIB_TARGETS+=$(LIBCYPMC)
DEP_PMD:=$(PMCLIB_DIR)/libpmc.so
PMD_LIB:=-L$(PMCLIB_DIR) -lpmc
endif

ifeq ($(IPMIenable),yes)
LIBSRASIPMI=librasipmi.so
LIB_TARGETS+=$(LIBSRASIPMI)
endif

TARGETS:=$(LIBA) $(LIB_TARGETS) $(BIN_TARGETS) 

all: $(TARGETS)

install:
	@( for file in $(BIN_TARGETS) ;  do \
		install -m 755 $$file	$(BINDEST) ; \
		${STRIP} $(BINDEST)/$$file ; \
	done )
	@( for file in $(LIB_TARGETS) ; do \
		install -m 644 $$file $(LIBDEST) ; \
		${STRIP} $(LIBDEST)/$$file ; \
	done )

ifeq ($(DEFTARG),OEM3)
pmc.o:  CFLAGS+=-DOEM3
endif

$(LIBCYPMC): pmc.o $(DEP_PMD)
	$(CC) $(LDFLAGS) -shared pmc.o -o $@ $(PMD_LIB)

$(LIBSRASIPMI): libcyc_rasipmi.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ 

portslave:	$(OBJS) $(LIBA)
	$(CC) $(LDFLAGS) -o portslave $(OBJS) $(LIBA) $(LIBRARIES)

ctlportslave:	ctlportslave.o
		$(CC) $(LDFLAGS) -o ctlportslave ctlportslave.o ctlp-subs.o

radinit:	radinit.o
		$(CC) $(LDFLAGS) -o radinit radinit.o

librad.a:	$(LIBOBJS) 
		$(AR) rv librad.a $(LIBOBJS) 
		$(RANLIB) librad.a

libsconf.so: sconf.o
	$(CC) -shared sconf.o -o libsconf.so

clean:
		rm -f *~ *.o *.a *.so $(TARGETS)
