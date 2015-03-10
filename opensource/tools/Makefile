#
#  Makefile for Cyclades projects.
#

MD5      := MD5
APPLETS  := applets
SOURCES  := sources
PATCHES  := patches
TARBAL   := tarbal
BUILD    := build
DIST     := dist
TOOLS    := tools
RULES    := rules
LOGS     := logs
WORK_DIR := $(shell pwd)

define text_error

ERROR: eviroment variable 'OPENSOURCE_DIR' is not defined.

The command '. ./$(TOOLS)/mk_env -s' must be executed inside the
directory that will be value of 'OPENSOURCE_DIR'.

endef

ifndef OPENSOURCE_DIR
  $(error $(text_error))
endif

include $(OPENSOURCE_DIR)/$(TOOLS)/MakeChecks

include $(WORK_DIR)/packages_def

include $(WORK_DIR)/config_tools

export SOURCES BUILD DIST TOOLS RULES WORK_DIR APPLETS MD5 PATCHES TARBAL LOGS

.PHONY: all create_dirs src bld cookie

ifeq ($(filter cookie, $(MAKECMDGOALS)),cookie)
$(error Target cookie can not be directly called) 
endif

CCACHE:=/usr/local/bin/ccache
FCROSS_CC:=$(firstword $(CROSS_CC))
FCROSS_CPP:=$(firstword $(CROSS_CPP))

all: lclean src bld applets
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeCookie

dummy:
	@mkdir dummy
	@( \
		cd dummy; \
		ln -s $(CCACHE) $(FCROSS_CC) ; \
		if [ ! "x$(FCROSS_CPP)" = "x$(FCROSS_CC)" ]; then \
		  ln -s $(CCACHE) $(FCROSS_CPP) ; \
		fi \
	)

#
# make all sources targets for parallel compilation
#
bld: src

src bld applets: lclean

lclean: $(WORK_DIR)/$(LOGS)
	sudo rm -rf $(WORK_DIR)/$(LOGS)/*

applets:  create_dirs force
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeApplets

src: create_dirs force
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeSource

bld: src force
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeBuild

create_dirs: 	$(OPENSOURCE_DIR)/$(SOURCES) \
				$(OPENSOURCE_DIR)/$(MD5) \
				$(WORK_DIR)/$(BUILD) \
				$(OPENSOURCE_DIR)/$(APPLETS) \
				$(WORK_DIR)/$(DIST) \
				$(WORK_DIR)/$(LOGS) \
				dummy

$(OPENSOURCE_DIR)/$(SOURCES) \
$(OPENSOURCE_DIR)/$(MD5) \
$(WORK_DIR)/$(BUILD) \
$(OPENSOURCE_DIR)/$(APPLETS) \
$(WORK_DIR)/$(DIST) \
$(WORK_DIR)/$(LOGS):
	install -m u=wrxs,g=rxsw,o=rx -d $@

veryclean: clean_$(BUILD) clean_$(SOURCES) \
		clean_$(DIST) clean_$(APPLETS) clean_$(MD5) clean_$(LOGS) clean_cookie

clean: clean_$(BUILD) clean_$(DIST) clean_$(LOGS) clean_cookie clean_dummy

clean_dummy:
	rm -rf dummy

clean_cookie:
	sudo rm -f cookie.*

clean_$(BUILD) clean_$(DIST) clean_$(LOGS): clean_% : force
	sudo rm -rf $(WORK_DIR)/$*

clean_$(SOURCES) clean_$(APPLETS) clean_$(MD5): clean_% : force
	sudo rm -rf $(OPENSOURCE_DIR)/$*

clean_%: force
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeClean $@

$(SRC_SPC_PKGS): create_dirs force
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeSource $@

$(SRC_PKGS): create_dirs force
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeSource $@

$(BLD_PKGS): create_dirs force
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeSource $@
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeBuild $@

$(APPLET_PKGS): create_dirs force
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeApplets $@

%+: force
	@( \
	TARGET="$(filter $*%, $(SRC_SPC_PKGS) $(BLD_PKGS) $(SRC_PKGS) $(APPLET_PKGS))";\
	if [ "$$TARGET" = "" ] ; then \
		echo "Invalid target: $@" ; \
		exit -1 ; \
	else \
		$(MAKE) -f $(firstword $(MAKEFILE_LIST)) $$TARGET ; \
	fi )

$(MAKEFILE_LIST): ;

force: ;

