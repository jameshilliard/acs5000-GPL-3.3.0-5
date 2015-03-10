# (c) 2002 Art & Logic, Inc. All Rights Reserved.

#
#  Makefile for the Linux version of AcsWeb.
#  Written for GNU make, other makes may require some modifications.
#

SHELL = /bin/sh

ACSDIR = .

WEBDIR = ../Web

DMFROOT = ../dmf212

ACSWEB = $(TARGET_DIR)/$(WebPagesDir)

DMFDIR = $(DMFROOT)/AlFrame

EXPATDIR = $(DMFROOT)/expat

GAWSDIR = $(DMFROOT)/webs

MAINDIR = .

SERVERDIR = $(PWD)

# Enable the DMF code.
DMF = -DqDmf

# enable support for 'rich error pages'
#RICHERR = -DqRichErrorPage

# enable support for hierarchical access control
HIERACC = -DqHierarchicalAccess

# enable the user management code...
UMSW = -DUSER_MANAGEMENT_SUPPORT

# turn on the digest authentication code
DASW = -DDIGEST_ACCESS_SUPPORT


# if-Modified-Support switches (requires math library, libm.a)
# IFMODSW = -DWEBS_IF_MODIFIED_SUPPORT
# # IFMODLIB = /usr/lib/libm.a

# SSL switches (enable all to activate)

# Libcrypto paths
LIBCRYPTO_DIR = $(OPEN_SSL_DIR)/lib
LIBCRYPTO = $(LIBCRYPTO_DIR)/libcrypto.so

# Libssl paths
LIBSSL_DIR = $(OPEN_SSL_DIR)/lib
LIBSSL = $(LIBSSL_DIR)/libssl.so

# SSLINC = /RSASSL/library/include
# SSLLIB = /RSASSL/library/lib/libsslc.a
SSLINC = $(OPEN_SSL_DIR)/include
SSLLIB = $(LIBSSL) $(LIBCRYPTO)
SSLSW	= -DWEBS_SSL_SUPPORT -DOPENSSL -I$(SSLINC)
SSLPATCHFILE = $(GAWSDIR)/websSSL.o

CFLAGS	+= -DWEBS -DUEMF -DOS="LINUX" -DLINUX $(UMSW) $(DASW) $(SSLSW) $(IFMODSW)
OTHERS	= -DCYC_FIX
#OTHERS	= -DDEBUG -DCYC_FIX
DMFFLAGS = $(DMF) $(RICHERR) $(HIERACC)
#DEBUG	= -Wall
IFLAGS 	=  -I$(DMFDIR) \
            -I$(EXPATDIR)/xmlparse \
            -I$(EXPATDIR)/xmltok \
            -I$(GAWSDIR) \
            -I$(WEBAPI_DIR) \
	    -I$(SERVERDIR) \
	    -I$(LIBCYC_DIR)/include \
	    -I$(RAS_DIR)/src 
ifeq ($(PMDNGenable),yes)
IFLAGS += -I$(PMCLIB_DIR)/include -I$(PMCLIB_DIR)/gsp-include
endif

LDFLAGS += $(SSLLIB) $(IFMODLIB) -Wl,-Map,web.map -lm

RASDIR    = $(RAS_DIR)/src
LIBS      = $(WEBAPI_DIR)/libWebApi.a
LIBRARIES = -L$(TARGET_DIR)/lib -lpam -lpam_misc -lcrypt
ifeq ($(PMDNGenable),yes)
LIBRARIES += -L$(PMCLIB_DIR) -lpmc
endif

LIBOTHERS    = $(RASDIR)/rwconf.o $(RASDIR)/librad.a

INSTALL=/usr/bin/install
PREFIX=$(TARGET_DIR)

OTHERS += -DASSERT

ifeq ($(ZIMAGE),uImage)
CFLAGS+=-DUBOOT_PARAM
endif

all:	AcsWeb

WEBS_OBJS=$(MAINDIR)/main.o \
      $(DMFDIR)/Access.o \
      $(DMFDIR)/Cookie.o \
      $(DMFDIR)/Dmf.o \
      $(DMFDIR)/EjFuncs.o \
      $(DMFDIR)/Error.o \
      $(DMFDIR)/Forms.o \
      $(DMFDIR)/GetFuncs.o \
      $(DMFDIR)/GetSet.o \
      $(DMFDIR)/Localize.o \
      $(DMFDIR)/Locals.o \
      $(DMFDIR)/Parse.o \
      $(DMFDIR)/RpcUtil.o \
      $(DMFDIR)/Session.o \
      $(DMFDIR)/SetFuncs.o \
      $(DMFDIR)/Soap.o \
      $(DMFDIR)/StringBuf.o \
      $(DMFDIR)/XmlRpc.o \
      $(DMFDIR)/XmlRpcFuncs.o\
      $(GAWSDIR)/asp.o \
      $(GAWSDIR)/balloc.o \
      $(GAWSDIR)/base64.o \
      $(GAWSDIR)/cgi.o \
      $(GAWSDIR)/default.o \
      $(GAWSDIR)/ejlex.o \
      $(GAWSDIR)/ejparse.o \
      $(GAWSDIR)/emfdb.o \
      $(GAWSDIR)/form.o \
      $(GAWSDIR)/h.o \
      $(GAWSDIR)/handler.o \
      $(GAWSDIR)/md5c.o \
      $(GAWSDIR)/mime.o \
      $(GAWSDIR)/misc.o \
      $(GAWSDIR)/page.o \
      $(GAWSDIR)/ringq.o \
      $(GAWSDIR)/rom.o \
      $(GAWSDIR)/security.o \
      $(GAWSDIR)/sock.o \
      $(GAWSDIR)/sockGen.o \
      $(GAWSDIR)/sym.o \
      $(GAWSDIR)/uemf.o \
      $(GAWSDIR)/um.o \
      $(GAWSDIR)/url.o \
      $(GAWSDIR)/value.o \
      $(GAWSDIR)/webrom.o \
      $(GAWSDIR)/webs.o \
      $(GAWSDIR)/websda.o \
      $(SSLPATCHFILE) \
      $(GAWSDIR)/websuemf.o \
      $(ACSDIR)/UserFunc.o \
      $(ACSDIR)/PortsFunc.o \
      $(ACSDIR)/cyc_aspFunc.o \
      $(ACSDIR)/Param.o\
      $(EXPATDIR)/xmlparse/xmlparse.o \
      $(EXPATDIR)/xmltok/xmlrole.o \
      $(EXPATDIR)/xmltok/xmltok.o

ifeq ($(filter $(TARG),kvm kvmapc kvmnetv2 ons),$(TARG))
WEBS_OBJS+= $(ACSDIR)/KVMFunc.o $(ACSDIR)/KVMPorts.o $(ACSDIR)/KVMUser.o
endif

AcsWeb: $(WEBS_OBJS) $(LIBS) $(LIBOTHERS)
		$(CC) -o $@ $(WEBS_OBJS) $(LDFLAGS) $(LIBRARIES) $(LIBS) $(LIBOTHERS) -lcrypt 

install:
		$(INSTALL) -m 755 AcsWeb    $(TARGET_DIR)/bin
		$(INSTALL) -d ${ACSWEB}
		$(INSTALL) -d ${ACSWEB}/Locale
	
		$(INSTALL) -d ${ACSWEB}/Images
		$(INSTALL) -d ${ACSWEB}/wizard

ifeq ($(AVWORKSenable),yes)
		$(INSTALL) -d ${ACSWEB}/Images/AVWorks
endif

#		$(INSTALL) -d ${ACSWEB}/wizard/helpPages
		$(INSTALL) -d ${ACSWEB}/normal
#		$(INSTALL) -d ${ACSWEB}/normal/helpPages
		$(INSTALL) -d ${ACSWEB}/normal/regularUser
ifeq ($(filter $(TARG),kvm kvmapc kvmnetv2 ons),$(TARG))
		$(INSTALL) -d ${ACSWEB}/normal/auxport
		$(INSTALL) -d ${ACSWEB}/normal/kvm
endif
		$(INSTALL) -d ${ACSWEB}/normal/information
		$(INSTALL) -d ${ACSWEB}/normal/management
		$(INSTALL) -d ${ACSWEB}/normal/administration
		$(INSTALL) -d ${ACSWEB}/normal/applications
		$(INSTALL) -d ${ACSWEB}/normal/applications/appl
		$(INSTALL) -d ${ACSWEB}/normal/applications/appl/profiles  #Temperature Sensor
		$(INSTALL) -d ${ACSWEB}/normal/network
		$(INSTALL) -d ${ACSWEB}/normal/ports
		$(INSTALL) -d ${ACSWEB}/normal/security

		cp  ${WEBDIR}/*.asp ${ACSWEB}
		cp  ${WEBDIR}/*.html ${ACSWEB}
		cp  ${WEBDIR}/*.js ${ACSWEB}
		cp  ${WEBDIR}/stylesLayout.css ${ACSWEB}
		cp  ${WEBDIR}/Images/*.gif ${ACSWEB}/Images
		cp  ${WEBDIR}/Images/*.jpg ${ACSWEB}/Images
		cp  ${ACSDIR}/Locale/language.en ${ACSWEB}/Locale

ifeq ($(AVWORKSenable),yes)
		cp  ${WEBDIR}/discovery.dtd ${ACSWEB}
		cp  ${WEBDIR}/Images/AVWorks/*.gif ${ACSWEB}/Images/AVWorks
ifeq ($(TARG),kvmapc)	
		ln -sf /$(WebPagesDir)/normal/applications/appl/tmp/discovery.xml ${ACSWEB}/discovery.xml
endif
endif

		cp  ${WEBDIR}/wizard/*.asp ${ACSWEB}/wizard
ifneq ($(SECPROFILEenable),yes)
		rm -f ${ACSWEB}/wizard/secProfile.asp
endif
#		cp  ${WEBDIR}/wizard/helpPages/*.asp ${ACSWEB}/wizard/helpPages

		cp  ${WEBDIR}/normal/*.asp ${ACSWEB}/normal
		cp  ${WEBDIR}/normal/*.html ${ACSWEB}/normal
#		cp  ${WEBDIR}/normal/helpPages/*.asp ${ACSWEB}/normal/helpPages
ifeq ($(filter $(TARG),kvm kvmapc kvmnetv2 ons),$(TARG))
		cp  ${WEBDIR}/normal/auxport/*.asp ${ACSWEB}/normal/auxport
		cp  ${WEBDIR}/normal/kvm/*.asp ${ACSWEB}/normal/kvm
endif
		cp  ${WEBDIR}/normal/information/*.asp ${ACSWEB}/normal/information
		cp  ${WEBDIR}/normal/management/*.asp ${ACSWEB}/normal/management
		cp  ${WEBDIR}/normal/administration/*.asp ${ACSWEB}/normal/administration
		cp  ${WEBDIR}/normal/applications/*.asp ${ACSWEB}/normal/applications
		cp  ${WEBDIR}/normal/applications/appl/*.asp ${ACSWEB}/normal/applications/appl
		cp  ${WEBDIR}/normal/applications/appl/*.conf ${ACSWEB}/normal/applications/appl
		cp  ${WEBDIR}/normal/applications/appl/*.jar ${ACSWEB}/normal/applications/appl
ifneq ($(WINEMSenable),yes)
		rm -f ${ACSWEB}/normal/applications/appl/utf8key.conf
		rm -f ${ACSWEB}/normal/applications/appl/winApplet.conf
		rm -f ${ACSWEB}/normal/applications/appl/winbuttonbar.conf
endif
ifeq ($(MINDTERMenable),yes)
		rm -f ${ACSWEB}/normal/applications/appl/*.conf
endif
ifneq ($(PMDenable),yes)
		rm -f ${ACSWEB}/normal/applications/*PMOut*.asp
		rm -f ${ACSWEB}/normal/applications/*PM_IPDU*.asp
		rm -f ${ACSWEB}/normal/applications/*PMconfig*.asp
		rm -f ${ACSWEB}/normal/applications/*PMsoft*.asp
		rm -f ${ACSWEB}/normal/applications/*PMD*.asp
		rm -f ${ACSWEB}/normal/applications/*PMMulti*.asp
		rm -f ${ACSWEB}/normal/applications/*PMuser*.asp
endif
ifneq ($(IPMIenable),yes)
		rm -f ${ACSWEB}/normal/applications/*IPMI*.asp
endif

		cp  ${WEBDIR}/normal/network/*.asp ${ACSWEB}/normal/network
ifneq ($(PCMCIAenable),yes)
		rm -f ${ACSWEB}/normal/network/netAccessMeth*.asp
endif
ifneq ($(IPSECenable),yes)
		rm -f ${ACSWEB}/normal/network/*VPN*.asp
endif

		cp  ${WEBDIR}/normal/ports/*.asp ${ACSWEB}/normal/ports
ifneq ($(PMDenable),yes)
		rm -f ${ACSWEB}/normal/ports/*Power*.asp
endif

		cp  ${WEBDIR}/normal/security/*.asp ${ACSWEB}/normal/security
ifneq ($(SECPROFILEenable),yes)
		rm -f ${ACSWEB}/normal/security/*Profile*.asp
else
		rm -f ${ACSWEB}/normal/security/*Services*.asp
endif
ifneq ($(KRBenable),yes)
		rm -f ${ACSWEB}/normal/security/*Kerb*.asp
endif
ifneq ($(NISenable),yes)
		rm -f ${ACSWEB}/normal/security/*Nis*.asp
endif
ifneq ($(SMBenable),yes)
		rm -f ${ACSWEB}/normal/security/*Smb*.asp
endif

ifeq ($(RDPenable),yes)
		$(INSTALL) -d ${ACSWEB}/normal/rdp
		cp  ${WEBDIR}/normal/rdp/*.asp ${ACSWEB}/normal/rdp
		cp  ${WEBDIR}/normal/applications/appl/msrdp.cab ${ACSWEB}/normal/applications/appl
else
#		rm -f ${ACSWEB}/normal/helpPages/RDP*.asp
endif

ifeq ($(filter $(TARG),kvm kvmapc),$(TARG))
	rm -f ${ACSWEB}/normal/applications/appl/*.conf
	rm -f ${ACSWEB}/Images/{acs.jpg,acs1.gif,controls.jpg,kvmp.jpg,ons.jpg,TS110.jpg,kvm{.gif,16.jpg,32.jpg}}
endif

ifeq ($(TARG),cyc)
	rm -f ${ACSWEB}/Images/{{acs1,kvm}.gif,{acs,ons,TS100,kvm{16,32,p}.jpg,loginBackground{,800},top,controls,leftMenu}.jpg}
endif

myclean:
	rm -f $(ACSDIR)/*.o

clean:
	rm -f AcsWeb 
	rm -f $(MAINDIR)/main.o
	rm -f $(DMFDIR)/*.o
	rm -f $(GAWSDIR)/*.o
	rm -f $(ACSDIR)/*.o
	rm -rf $(EXPATDIR)/*.o
	echo clean: make complete
   
%.o:%.c
		$(CC) -c $(DEBUG) $(CFLAGS) $(DMFFLAGS) $(OTHERS) $(IFLAGS) -o $@ $<

$(DMFDIR)/Access.o : \
   $(DMFDIR)/Access.h\
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/Parse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/um.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/Cookie.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Cookie.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/Parse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   $(GAWSDIR)/wsIntrn.h\
   

$(DMFDIR)/Dmf.o : \
   $(DMFDIR)/Access.h\
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Cookie.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/EjFuncs.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Forms.h\
   $(DMFDIR)/Localize.h\
   $(DMFDIR)/Locals.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/Parse.h\
   $(DMFDIR)/RpcUtil.h\
   $(DMFDIR)/Soap.h\
   $(DMFDIR)/StringBuf.h\
   $(DMFDIR)/XmlRpc.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/um.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/EjFuncs.o : \
   $(DMFDIR)/Access.h\
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/EjFuncs.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/GetSet.h\
   $(DMFDIR)/Locals.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/Parse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/um.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/Error.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/GetSet.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/Parse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   $(GAWSDIR)/wsIntrn.h\
   

$(DMFDIR)/Forms.o : \
   $(DMFDIR)/Access.h\
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Forms.h\
   $(DMFDIR)/GetSet.h\
   $(DMFDIR)/Locals.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/Parse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/um.h\
   $(GAWSDIR)/webs.h\
   $(GAWSDIR)/wsIntrn.h\
   

$(DMFDIR)/GetFuncs.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/GetFuncs.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/GetSet.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/GetSet.h\
   $(DMFDIR)/Locals.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/Localize.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Localize.h\
   $(DMFDIR)/Locals.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/Parse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/Locals.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Locals.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/Parse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/Parse.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/Parse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/RpcUtil.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/GetSet.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/RpcUtil.h\
   $(DMFDIR)/StringBuf.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/Session.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   $(GAWSDIR)/wsIntrn.h\
   

$(DMFDIR)/SetFuncs.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/SetFuncs.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/Soap.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/GetSet.h\
   $(DMFDIR)/Localize.h\
   $(DMFDIR)/Locals.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/RpcUtil.h\
   $(DMFDIR)/Soap.h\
   $(DMFDIR)/StringBuf.h\
   $(DMFDIR)/XmlRpcFuncs.h\
   $(EXPATDIR)/xmlparse/xmlparse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   $(GAWSDIR)/wsIntrn.h\
   

$(DMFDIR)/StringBuf.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/StringBuf.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   

$(DMFDIR)/XmlRpc.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/GetSet.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/RpcUtil.h\
   $(DMFDIR)/StringBuf.h\
   $(DMFDIR)/XmlRpc.h\
   $(DMFDIR)/XmlRpcFuncs.h\
   $(EXPATDIR)/xmlparse/xmlparse.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   $(GAWSDIR)/wsIntrn.h\
   

$(DMFDIR)/XmlRpcFuncs.o : \
   $(DMFDIR)/AlTypes.h\
   $(DMFDIR)/Dmf.h\
   $(DMFDIR)/ErrorCodes.h\
   $(DMFDIR)/GetSet.h\
   $(DMFDIR)/Localize.h\
   $(DMFDIR)/Locals.h\
   $(DMFDIR)/Param.h\
   $(DMFDIR)/ParamCodes.h\
   $(DMFDIR)/RpcUtil.h\
   $(DMFDIR)/StringBuf.h\
   $(DMFDIR)/XmlRpc.h\
   $(DMFDIR)/XmlRpcFuncs.h\
   $(GAWSDIR)/ej.h\
   $(GAWSDIR)/ejIntrn.h\
   $(GAWSDIR)/emfdb.h\
   $(GAWSDIR)/uemf.h\
   $(GAWSDIR)/webs.h\
   

$(EXPATDIR)/xmlparse/xmlparse.o : \
	$(EXPATDIR)/xmlparse/xmlparse.h\
	$(EXPATDIR)/xmltok/xmldef.h\
	$(EXPATDIR)/xmltok/xmlrole.h\
	$(EXPATDIR)/xmltok/xmltok.h\
	

$(EXPATDIR)/xmltok/xmlrole.o : \
	$(EXPATDIR)/xmltok/ascii.h\
	$(EXPATDIR)/xmltok/xmldef.h\
	$(EXPATDIR)/xmltok/xmlrole.h\
	$(EXPATDIR)/xmltok/xmltok.h\
	

$(EXPATDIR)/xmltok/xmltok.o : \
	$(EXPATDIR)/xmltok/ascii.h\
	$(EXPATDIR)/xmltok/asciitab.h\
	$(EXPATDIR)/xmltok/iasciitab.h\
	$(EXPATDIR)/xmltok/latin1tab.h\
	$(EXPATDIR)/xmltok/nametab.h\
	$(EXPATDIR)/xmltok/utf8tab.h\
	$(EXPATDIR)/xmltok/xmldef.h\
	$(EXPATDIR)/xmltok/xmltok.h\
	$(EXPATDIR)/xmltok/xmltok_impl.c\
	$(EXPATDIR)/xmltok/xmltok_impl.h\
	$(EXPATDIR)/xmltok/xmltok_ns.c\
	


$(GAWSDIR)/asp.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/balloc.o : $(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/base64.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/cgi.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/default.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/ejlex.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/ejparse.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/emfdb.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/emfdb.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/form.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/h.o : $(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/handler.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/md5c.o : $(GAWSDIR)/md5.h\
	

$(GAWSDIR)/mime.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/misc.o : $(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/page.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/ringq.o : $(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/rom.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/security.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/um.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/websda.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/sock.o : $(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/sockGen.o : $(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/sym.o : $(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/uemf.o : $(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/um.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/emfdb.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/um.h\
	$(GAWSDIR)/webs.h\
	

$(GAWSDIR)/url.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/value.o : $(GAWSDIR)/uemf.h\
	

$(GAWSDIR)/webrom.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/webs.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/websda.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(GAWSDIR)/websda.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/md5.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/websda.h\
	

$(GAWSDIR)/websuemf.o : \
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	


$(MAINDIR)/main.o : \
	$(DMFDIR)/AlTypes.h\
	$(DMFDIR)/Dmf.h\
	$(DMFDIR)/ErrorCodes.h\
	$(DMFDIR)/Param.h\
	$(DMFDIR)/ParamCodes.h\
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/emfdb.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/um.h\
	$(GAWSDIR)/webs.h\
	$(GAWSDIR)/wsIntrn.h\
	

$(ACSDIR)/Param.o : \
	$(DMFDIR)/AlTypes.h\
	$(DMFDIR)/Dmf.h\
	$(DMFDIR)/ErrorCodes.h\
	$(DMFDIR)/GetFuncs.h\
	$(DMFDIR)/Param.h\
	$(DMFDIR)/ParamCodes.h\
	$(DMFDIR)/SetFuncs.h\
	$(ACSDIR)/UserFunc.h\
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/emfdb.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	

$(ACSDIR)/UserFunc.o : \
	$(DMFDIR)/Access.h\
	$(DMFDIR)/AlTypes.h\
	$(DMFDIR)/Dmf.h\
	$(DMFDIR)/ErrorCodes.h\
	$(DMFDIR)/Locals.h\
	$(DMFDIR)/Param.h\
	$(DMFDIR)/ParamCodes.h\
	$(ACSDIR)/UserFunc.h\
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/emfdb.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/um.h\
	$(GAWSDIR)/webs.h\


$(ACSDIR)/cyc_aspFunc.o : \
	$(DMFDIR)/Access.h\
	$(DMFDIR)/AlTypes.h\
	$(DMFDIR)/Dmf.h\
	$(DMFDIR)/ErrorCodes.h\
	$(DMFDIR)/Locals.h\
	$(DMFDIR)/Param.h\
	$(DMFDIR)/ParamCodes.h\
	$(ACSDIR)/UserFunc.h\
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/emfdb.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/um.h\
	$(GAWSDIR)/webs.h\
	
$(ACSDIR)/PortsFunc.o : \
	$(DMFDIR)/Access.h\
	$(DMFDIR)/AlTypes.h\
	$(DMFDIR)/Dmf.h\
	$(DMFDIR)/ErrorCodes.h\
	$(DMFDIR)/Locals.h\
	$(DMFDIR)/Param.h\
	$(DMFDIR)/ParamCodes.h\
	$(ACSDIR)/UserFunc.h\
	$(ACSDIR)/PortsFunc.h\
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/emfdb.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/um.h\
	$(GAWSDIR)/webs.h\
	
