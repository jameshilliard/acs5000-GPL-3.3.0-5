#all: bootparamd
#include ../MCONFIG
#include ../MRULES

RPCGEN=rpcgen

# Warning, do NOT put this in the current directory under its own
# name without updating things and particularly the clean target.
#BOOTPARAMX = /usr/include/rpcsvc/bootparam_prot.x
# Actually the one that comes with linux libc appears to be broken
BOOTPARAMX = bootparam_prot.x.real

HEADER=bootparam_prot.h
DIST=README main.c rpc.bootparamd.c callbootd.c bootparamd.8 Makefile

all: bootparamd callbootd

bootparamd: bootparam_prot_svc.o bootparam_prot_xdr.o rpc.bootparamd.o main.o
	${CC} $(LDFLAGS) $^ $(LIBS) -o $@

callbootd: callbootd.o bootparam_prot_clnt.o bootparam_prot_xdr.o 
	${CC} $(LDFLAGS) $^ $(LIBS) -o $@

install: bootparamd callbootd
	install -s -m $(DAEMONMODE) bootparamd \
		$(INSTALLROOT)$(SBINDIR)/rpc.bootparamd
	install -s -m $(BINMODE) callbootd $(INSTALLROOT)$(BINDIR)/callbootd

clean:
	rm -f *.o bootparamd callbootd 
	rm -f bootparam_prot.h bootparam_prot_svc.c
	rm -f bootparam_prot_xdr.c bootparam_prot_clnt.c
	rm -f bootparam_prot.x

bootparam_prot_clnt.o bootparam_prot_xdr.o: bootparam_prot.h
bootparam_prot_svc.o: bootparam_prot.h
callbootd.o rpc.bootparamd.o main.o: bootparam_prot.h ../version.h

bootparam_prot.x: $(BOOTPARAMX)
	ln -s $(BOOTPARAMX) bootparam_prot.x

bootparam_prot.h: bootparam_prot.x
	$(RPCGEN) -h $< -o $@

bootparam_prot_xdr.c: bootparam_prot.x
	$(RPCGEN) -c $< -o $@

bootparam_prot_clnt.c: bootparam_prot.x
	$(RPCGEN) -l $< -o $@

bootparam_prot_svc.c: bootparam_prot.x
	$(RPCGEN) -m $< -o $@ 
