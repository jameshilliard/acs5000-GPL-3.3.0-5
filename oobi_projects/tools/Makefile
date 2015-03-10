#
#  Makefile for Cyclades projects.
#

MD5:=MD5
LOGS:=logs
TOOLS:=tools
RULES:=rules
BUILD:=build
TARGET:=target
IMAGE:=image
SOURCES:=sources
APPLETS:=applets
WORK_DIR:=$(shell pwd)

ID_UN:=$(shell id -un)
ID_GN:=$(shell id -gn)

export TOOLS BUILD TARGET IMAGE WORK_DIR SOURCES RULES MD5 APPLETS LOGS
export ID_UN ID_GN

define text_error


ERROR: eviroment variable 'ROOT_PROJ' is not defined.

The command '. ./$(TOOLS)/mk_env -s' must be executed inside the
directory that will be the value of 'ROOT_PROJ'.

endef

ifndef ROOT_PROJ
  $(error $(text_error))
endif

#
# Checks make version and some environment variables
#
include $(ROOT_PROJ)/$(TOOLS)/MakeChecks

#
# Get all project variable definitions
#
include $(WORK_DIR)/config
ifdef ADMUSER
export ADMUSER
endif
export PCMCIAenable PMDenable NISenable

#
# Get all names of packages used in the project
#
include $(ROOT_PROJ)/$(PACKAGES_DEF)

#
# Get all cross compiler tool definitions
#
include $(CONFIG_TOOL_DIR)/config_tools

ifeq ($(BuildType),HostBuild)
$(call check_vars,\
OPENSOURCE_DIR APPL_DEBUG TARGET_PRODUCT \
CROSS_STRIP CROSS_PATH JAVA_CONF SOURCE_PKGS)
endif

ifeq ($(BuildType),HostBuild)
$(call check_vars,\
OPENSOURCE_DIR APPL_DEBUG TARGET_PRODUCT \
CROSS_STRIP CROSS_PATH SOURCE_PKGS BUILTIN_PKGS JAVA_CONF)
endif

ifeq ($(BuildType),BuildImage)
$(call check_vars,\
OPENSOURCE_DIR APPL_DEBUG TARGET_PRODUCT CROSS_DIR ZIMAGE \
CROSS_TARGET CROSS_STRIP CROSS_PATH MVL_LIB_FILES MVL_BIN_FILES \
CYCLADES_FILES SOURCE_PKGS BUILTIN_PKGS JAVA_CONF)
endif

include $(OPENSOURCE_DIR)/$(TOOLS)/$(JAVA_CONF)

#### Makefile targets begin here ....

all: $(BuildType)

HostBuild:	check_cookie \
			check_utilities \
			clean_target \
			bld

BuildImage:	check_cookie \
			check_utilities \
			clean_target \
			kernel_config \
			copy_files \
			build_extra \
			builtin \
			builtin_misc \
			bld \
			modules \
			$(IMAGE)

KernelOnly: check_cookie \
			check_utilities \
			clean_target \
			kernel_config \
			bld \
			$(IMAGE)

######### Define global dependences for parallel compilation #########

clean_target: check_cookie check_utilities

kernel_config \
build_extra: clean_target

builtin: build_extra
copy_files: builtin

# special treatment for some files for APC CPS OEM task
# (ugly but we don't wanna touch opensource files)
builtin_misc: builtin
	@( \
		rm -f $(WORK_DIR)/$(TARGET)/bin/libol-config; \
		file=$(WORK_DIR)/$(TARGET)/usr/local/lib/ipsec/_startklips; \
		if [ -f $${file} ] ; then \
			grep -v "Cyclades" $${file} > $${file}.tmp; \
			mv $${file}.tmp $${file}; \
			chmod 755 $${file}; \
		fi )

ifneq ($(BuildType),KernelOnly)
bld: build_extra kernel_config
endif

modules: kernel_config
 
zImage_kernel: modules

ifneq ($(BuildType),KernelOnly)
uImage_kernel: modules
endif

ifneq ($(BuildType),KernelOnly)
uImage_crc: make_image
endif

zImage_crc: bld zImage_kernel

ifneq ($(BuildType),KernelOnly)
ifeq ($(ZIMAGE),uImage)
make_image: uImage_kernel
else
zImage_kernel: make_image
endif
endif

make_image: copy_files bld modules

md5: $(ZIMAGE)_crc

$(IMAGE): $(ZIMAGE) $(ZIMAGE)_crc md5

ifneq ($(BuildType),KernelOnly)
uImage: $(ZIMAGE)_kernel make_image
else
uImage: $(ZIMAGE)_kernel
endif

zImage: make_image $(ZIMAGE)_kernel

create_dirs: 	$(WORK_DIR)/$(BUILD) \
		$(WORK_DIR)/$(MD5) \
		$(WORK_DIR)/$(TARGET) \
		$(WORK_DIR)/$(IMAGE) \
		$(WORK_DIR)/$(LOGS)

$(WORK_DIR)/$(BUILD) \
$(WORK_DIR)/$(MD5) \
$(WORK_DIR)/$(TARGET) \
$(WORK_DIR)/$(IMAGE) \
$(WORK_DIR)/$(LOGS):
	install -d $@

############ Check ip pre-builtin packages are OK ##########

check_cookie: force
	$(MAKE) -f $(OPENSOURCE_DIR)/$(TOOLS)/MakeCookie \
		-C $(CONFIG_TOOL_DIR) check

####################### source targets #######################

src: create_dirs force
	$(MAKE) -f $(ROOT_PROJ)/$(TOOLS)/MakeBuild $@

src_%: create_dirs force
	$(MAKE) -f $(ROOT_PROJ)/$(TOOLS)/MakeBuild $@

src_linux: create_dirs force
	$(MAKE) -f $(ROOT_PROJ)/$(RULES)/linux_kernel src

####################### build targets #########################

bld: create_dirs builtin force
	$(MAKE) -f $(ROOT_PROJ)/$(TOOLS)/MakeBuild $@

bld_linux: create_dirs kernel_config modules $(IMAGE)
	@echo

bld_%: create_dirs force
	$(MAKE) -f $(ROOT_PROJ)/$(TOOLS)/MakeBuild $@

#######################  check utilities #########################

# Java utilities to check
JAVAC:=bin/javac
JARSIGNER:=bin/jarsigner

check_utilities: dummy force
ifneq ($(words $(APPLET_PKGS)),0)
	@test -x $(JAVA_DIR)/$(JAVAC) || \
		( echo "ERROR: The Java Compiler was not found"; \
		  echo "Please install the java compiler in your machine"; \
		  exit 1 )
	@test -x $(JAVA_DIR)/$(JARSIGNER) || \
		( echo "ERROR: The jarsigner utility was not found"; \
		  echo "Please install jarsigner in your machine"; \
		  exit 1)
endif

dummy:
	@mkdir dummy
	@( \
		cd dummy; \
		ln -sf $(CROSS_BIN)/$(CROSS_STRIP) strip; \
	)

####################### builtin targets #########################

builtin: force
	$(call EXEC_ACTION,$@,sudo $(MAKE) -f $(ROOT_PROJ)/$(TOOLS)/MakeBuiltin)

####################### clean targets #########################

clean: force
	sudo rm -rf $(WORK_DIR)/$(BUILD)
	rm -rf $(WORK_DIR)/$(IMAGE)
	rm -rf $(WORK_DIR)/$(MD5)
	sudo rm -rf $(WORK_DIR)/$(TARGET)
	rm -rf $(WORK_DIR)/$(LOGS)
	rm -rf $(WORK_DIR)/dummy

clean_target: force
	rm -rf $(WORK_DIR)/$(LOGS)
	mkdir $(WORK_DIR)/$(LOGS)
	rm -rf $(WORK_DIR)/$(IMAGE)
	mkdir $(WORK_DIR)/$(IMAGE)
	sudo rm -rf $(WORK_DIR)/$(TARGET)
	mkdir $(WORK_DIR)/$(TARGET)
	mkdir $(WORK_DIR)/$(TARGET)/man
ifeq ($(GPL_ENV),yes)
	sudo tar -xzf $(WORK_DIR)/target.gz -C $(WORK_DIR)
endif

clean_linux: force
	$(MAKE) -f $(ROOT_PROJ)/$(RULES)/linux_kernel clean

clean_%: force
	$(MAKE) -f $(ROOT_PROJ)/$(TOOLS)/MakeClean $@


################# specific call to build target appl ######################

build_extra: force
	$(call EXEC_ACTION,$@,$(MAKE) -f $(firstword $(MAKEFILE_LIST)) build_extra_rdir)

build_extra_rdir: force
	# creates directories and copy files
	MODULES_DIR=$(MODULES_DIR) \
	KERNEL_VERSION=$(KERNEL_VERSION) \
	TARGET_DIR=$(WORK_DIR)/$(TARGET) \
	$(ROOT_PROJ)/$(TOOLS)/build_extra

	# creates devices
	TARG=$(TARGET_PRODUCT) \
	TARGET_DIR=$(WORK_DIR)/$(TARGET) \
	sudo $(ROOT_PROJ)/$(TOOLS)/mk_devs

ifdef isdn4k-utils
	# build devices for isdn
	sudo $(BUILTIN_BUILD)/$(isdn4k-utils)/scripts/makedev.sh \
		$(WORK_DIR)/$(TARGET)
endif

copy_files:
	$(call EXEC_ACTION,$@,$(MAKE) -f $(firstword $(MAKEFILE_LIST)) copy_files_rdir)

ifeq ($(GPL_ENV),yes)
CP_PCMCIA_FILES:=sudo $(ROOT_PROJ)/$(TOOLS)/cp_pcmcia_files
else
CP_PCMCIA_FILES:=$(ROOT_PROJ)/$(TOOLS)/cp_pcmcia_files
endif

copy_files_rdir:

ifneq ($(MVL_BIN_FILES),)
	#copy binary files from montavista
	PATH=$(CROSS_PATH) \
	MVL_TARGET_DIR=$(CROSS_TARGET) \
	MVL_FILES=$(MVL_BIN_FILES) \
	TARGET_DIR=$(WORK_DIR)/$(TARGET) \
	STRIP=$(CROSS_STRIP) \
	$(ROOT_PROJ)/$(TOOLS)/cp_files

	#[RK]Nov/26/02 - W-CAS
	mv -f $(WORK_DIR)/$(TARGET)/usr/bin/w  $(WORK_DIR)/$(TARGET)/usr/bin/w_ori

endif

ifneq ($(MVL_LIB_FILES),)
	#copy library files from montavista
	PATH=$(CROSS_PATH) \
	MVL_TARGET_DIR=$(CROSS_TARGET) \
	MVL_FILES=$(MVL_LIB_FILES) \
	TARGET_DIR=$(WORK_DIR)/$(TARGET) \
	STRIP=$(CROSS_STRIP) \
	$(ROOT_PROJ)/$(TOOLS)/cp_files
endif

ifneq ($(PCMCIA_FILES),)
	#copy library files from montavista
	PATH=$(CROSS_PATH) \
	MVL_TARGET_DIR=$(CROSS_TARGET) \
	MVL_FILES=$(PCMCIA_FILES) \
	TARGET_DIR=$(WORK_DIR)/$(TARGET) \
	STRIP=$(CROSS_STRIP) \
	$(CP_PCMCIA_FILES)
endif

#################### specific call to build kernel #########################

modules: force
	$(call EXEC_ACTION,$@,$(MAKE) -f $(ROOT_PROJ)/$(RULES)/linux_kernel -j1 modules)

kernel_config: force
	$(call EXEC_ACTION,$@,$(MAKE) -f $(ROOT_PROJ)/$(RULES)/linux_kernel -j1 config)

uImage_kernel: force
	$(call EXEC_ACTION,$@,$(MAKE) -f $(ROOT_PROJ)/$(RULES)/linux_kernel -j1 uImage)

zImage_kernel: force
	$(call EXEC_ACTION,$@,$(MAKE) -f $(ROOT_PROJ)/$(RULES)/linux_kernel -j1 zImage.initrd)

################# specific call to build image ######################

uImage_crc: force
ifneq ($(BuildType),KernelOnly)
	$(call EXEC_ACTION,$@,$(MAKE) -f $(ROOT_PROJ)/$(RULES)/linux_kernel -j1 uImage.initrd)
	cp $(KERNEL_IMAGE).initrd $(WORK_DIR)/$(IMAGE)/$(TARGET_IMAGENAME)
else
	cp $(KERNEL_IMAGE) $(WORK_DIR)/$(IMAGE)/$(TARGET_IMAGENAME)
endif

zImage_crc: force
	$(call EXEC_ACTION,$@,sudo crclinux $(KERNEL_IMAGE) 32 $(TARGET_BOOTCODE) \
		$(WORK_DIR)/$(IMAGE)/$(TARGET_IMAGENAME) 65536)

md5: $(ZIMAGE)_crc
	md5sum $(WORK_DIR)/$(IMAGE)/$(TARGET_IMAGENAME) > \
		$(WORK_DIR)/$(IMAGE)/$(TARGET_IMAGENAME).md5
	if [ -n "$(PROJ_SFX)" ]; then \
		sudo cp $(WORK_DIR)/$(IMAGE)/$(TARGET_IMAGENAME) /tftpboot/$(TARGET_IMAGENAME).$(PROJ_SFX); \
	fi

ifdef squashfs
SQUASH_DIR:=SQUASH_DIR=$(BUILTIN_DIST)/$(squashfs)
else
SQUASH_DIR:=
endif

make_image: force
	$(call EXEC_ACTION,$@,$(MAKE) -f $(firstword $(MAKEFILE_LIST)) make_image_rdir)

make_image_rdir: force
	# copy debug tools
ifeq ($(APPL_DEBUG),1)
	@( \
		for file in usr/bin/gdbserver usr/bin/strace \
					lib/libthread_db-1.0.so lib/libthread_db.so.1 ; do \
			$(CROSS_BIN)/$(CROSS_STRIP) $(CROSS_TARGET)/$$file \
				-o $(WORK_DIR)/$(TARGET)/$$file ; \
		done; \
	)
endif

	$(SQUASH_DIR) \
	KERNEL_IMAGE=$(KERNEL_IMAGE) \
	RD_IMAGE=$(RD_IMAGE) \
	RD_NINODES=$(RD_NINODES) \
	RD_SIZE=$(RD_SIZE) \
	VERSION_FILE=$(VERSION_FILE) \
	ROOT_PROJ=$(ROOT_PROJ) \
	TARG=$(TARGET_PRODUCT) \
	TARGET_DIR=$(WORK_DIR)/$(TARGET) \
	sudo $(ROOT_PROJ)/$(TOOLS)/mk_image

NEW_GPL: $(BuildType) force
	$(MAKE) -f $(ROOT_PROJ)/$(TOOLS)/MakeGPL GPL_TARGET=$@

OLD_GPL: force
	$(MAKE) -f $(ROOT_PROJ)/$(TOOLS)/MakeGPL GPL_TARGET=$@

$(MAKEFILE_LIST): ;

force: ;

