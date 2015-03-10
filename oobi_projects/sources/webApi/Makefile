ARTLOGIC_AI:=$(ARTLOGIC_DIR)/dmf212/AlFrame
ARTLOGIC_SRV:=$(ARTLOGIC_DIR)/Server
RASDIR    = $(RAS_DIR)/src
LIBOBJS   = $(RASDIR)/lib.o WebApi.o Public.o ArtLogic.o pam_auth.o
ifeq ($(PMDNGenable),yes)
LIBOBJS += webpm.o webpmcomm.o
endif

U_BOOT_OBJS   = $(UBOOT_DIR)/common/cyc_env.o $(UBOOT_DIR)/lib_generic/crc32.o
LIBS      = WebApi.a 

INSTALL=/usr/bin/install -c
PREFIX=$(TARGET_DIR)

CFLAGS += -DWebPagesDir='"$(WebPagesDir)"' -DSHADOWPASSWDS
LIBCYC_OBJS=$(LIBCYC_DIR)/src/libcyc_apctl.o \
		$(LIBCYC_DIR)/src/libcyc_ipc.o \
		$(LIBCYC_DIR)/src/libcyc_kvm.o \
		$(LIBCYC_DIR)/src/libcyc_ll.o \
		$(LIBCYC_DIR)/src/libcyc_thread.o \
		$(LIBCYC_DIR)/src/libcyc_main.o \
		$(LIBCYC_DIR)/src/libcyc_malloc.o \
		$(LIBCYC_DIR)/src/libcyc_util.o \
		$(LIBCYC_DIR)/src/md5.o


ifeq ($(PMDNGenable),yes)
CFLAGS+=-I$(PMCLIB_DIR)/include -I$(PMCLIB_DIR)/gsp-include
endif

ifeq ($(DEFTARG),OEM2)
CFLAGS+=-D$(DEFTARG) 
endif

ifeq ($(TARG),kvm)
CFLAGS+=-DKVM
LIBOBJS+=kvmdebug.o kvm_auxport.o kvm_cascade.o
endif

ifeq ($(TARG),kvmapc)
CFLAGS+=-DKVM
LIBOBJS+=kvmdebug.o kvm_auxport.o kvm_cascade.o
endif

ifeq ($(TARG),kvmnetv2)
CFLAGS+=-DKVM -DKVMNETP -I$(UBOOT_DIR)/include
LIBOBJS+=kvmdebug.o kvm_auxport.o kvm_cascade.o
endif

ifeq ($(TARG),ons)
CFLAGS+=-DONS -I$(UBOOT_DIR)/include
LIBOBJS+=kvmdebug.o kvm_auxport.o kvm_cascade.o
endif

LDFLAGS += -L.
TARGETS = libWebApi.a 

ifeq ($(KERNEL_VER),k26)
CFLAGS+=-DKERNEL_26 
endif

ifeq ($(RDPenable),yes)
CFLAGS+=-DRDPenable
endif

ifeq ($(ZIMAGE),uImage)
CFLAGS+=-DUBOOT_PARAM -I$(UBOOT_DIR)/include
U_BOOT_OBJS   = $(UBOOT_DIR)/common/cyc_env.o $(UBOOT_DIR)/lib_generic/crc32.o
endif

all:	$(TARGETS)

ifeq ($(filter $(TARG),ons kvmnetv2),$(TARG))
libWebApi.a:	$(LIBOBJS)
		$(AR) rv libWebApi.a $(LIBOBJS) $(U_BOOT_OBJS) $(LIBCYC_OBJS)
		$(RANLIB) libWebApi.a 
else
libWebApi.a:	$(LIBOBJS)
		$(AR) rv libWebApi.a $(LIBOBJS) $(LIBCYC_OBJS)
		$(RANLIB) libWebApi.a 
endif

#libWebApi.so: $(LIBOBJS)
#	$(CC) -shared -o $@ $(LIBOBJS) $(LIBCYC_OBJS)

$(LIBOBJS):	WebApi2.h $(ARTLOGIC_SRV)/webApi.h $(RAS_DIR)/src/lib.o

clean:
		rm -f *~ *.o *.a $(TARGETS) libWebApi.a

