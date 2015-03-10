###############################################################################
#        MAKEFILE CONFIGURATION
###############################################################################
.EXPORT_ALL_VARIABLES:

CFLAGS += -Werror -Iinclude -Igsp-include -I/opt/opensource/MVL-3.1/ppc_8xx/include

CFLAGS += -DSERVERTECH_LICENSE

#CFLAGS += -DDEBUG
#CFLAGS += -DNOISY_DEBUG
#CFLAGS += -DONLY_POWER_API

INSTALL:=install

OPT:=-O2 -fomit-frame-pointer
PCILIB_VERSION:=2.0
SUFFIX:=

export

###############################################################################
#        VARIABLES
###############################################################################

TARGETS:=libpmc.so
#TARGETS += pmclibtest

PMCLIB_OBJ:=pmclib.o pmc_validate.o pmc_sendcmd.o pmc_show.o debug.o

#PMCLIBTEST_OBJ:=pmclibtest.o libpmc.so

# Whenever necessary, gsp-include can be checked out from:
#   https://fre-svn.corp.avocent.com/svn/avocent-gsp/sources/generic/fm-power-mgmt-2.0.0/trunk/include
#    or
#   https://fre-svn.corp.avocent.com/svn/avocent-gsp/sources/generic/fm-power-mgmt-2.0.0/branches/<branch>/include
# Also available is tools/get_svn_src.sh
GSPHDRS_DIR = gsp-include

SVN_SIGNATURE = $(GSPHDRS_DIR)/.svn

#this file must be in synch with sources/pmd-gsp/prebuiltGSP.tgz
GSPTARBALL = prebuiltGSP.tgz

###############################################################################
#        RULES
###############################################################################
all: ckgsphdrs $(TARGETS) trytarball

libpmc.so: $(PMCLIB_OBJ)
	$(CC) $(LDFLAGS) -shared -o $@ $(PMCLIB_OBJ) -lm

#pmclibtest: $(PMCLIBTEST_OBJ)
#	$(CC) $(LDFLAGS) -o $@ $(PMCLIBTEST_OBJ) -lm

install:
	${INSTALL} -m 755 libpmc.so $(TARGET_DIR)/usr/lib

ckgsphdrs:
ifneq ($(shell ls -d $(SVN_SIGNATURE)),$(SVN_SIGNATURE))
	rm -rf $(GSPHDRS_DIR)
	tar -xzvf $(GSPTARBALL)
endif

trytarball:
ifeq ($(shell ls -d $(SVN_SIGNATURE)),$(SVN_SIGNATURE))
	tar -hcvzf $(ROOT_PROJ)/sources/pmclib/$(GSPTARBALL) $(GSPHDRS_DIR)/*.h
endif

clean:
	rm -rf *.o *~ $(TARGETS)
