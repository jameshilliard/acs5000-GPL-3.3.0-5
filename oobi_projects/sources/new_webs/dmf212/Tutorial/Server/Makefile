# (c) 2002 Art & Logic, Inc. All Rights Reserved.
# $Id: Makefile,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $

#
#  Makefile for the Linux version of the Art & Logic Device Management
#  Framework. May be used as a starting point for other operating systems.
#  Written for GNU make, other makes may require some modifications.
#

SHELL = /bin/sh

DMFDIR = ../../AlFrame

EXPATDIR = ../../expat

GAWSDIR = ../../webs

MAINDIR = ../../LINUX

# Enable the DMF code.
DMF = -DqDmf

# enable support for 'rich error pages'
RICHERR = -DqRichErrorPage

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
# SSLINC = /RSASSL/library/include
# SSLLIB = /RSASSL/library/lib/libsslc.a
# SSLSW	= -DWEBS_SSL_SUPPORT -I$(SSLINC)
# SSLPATCHFILE = $(GAWSDIR)/websSSL.o

CFLAGS	= -DWEBS -DUEMF -DOS="LINUX" -DLINUX $(UMSW) $(DASW) $(SSLSW) $(IFMODSW)
OTHERS	= -DB_STATS -DB_FILL -DDEBUG 
DMFFLAGS = $(DMF) $(RICHERR) $(HIERACC)
DEBUG	= -g -Wall
IFLAGS 	=  -I$(DMFDIR) \
            -I$(EXPATDIR)/xmlparse \
            -I$(EXPATDIR)/xmltok \
            -I$(GAWSDIR)

LDFLAGS	= $(SSLLIB) $(IFMODLIB)

CC = gcc

all:	Tutorial

Tutorial:  $(MAINDIR)/main.o \
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
      $(CHAP)/UserFunc.o \
      $(CHAP)/Param.o\
      $(EXPATDIR)/xmlparse/xmlparse.o \
      $(EXPATDIR)/xmltok/xmlrole.o \
      $(EXPATDIR)/xmltok/xmltok.o 
		   $(CC) -o $@ $+


clean:
	rm -f Tutorial 
	rm -f $(MAINDIR)/main.o
	rm -f $(DMFDIR)/*.o
	rm -f $(GAWSDIR)/*.o
	rm -f $(CHAP)/*.o
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
	

$(CHAP)/Param.o : \
	$(DMFDIR)/AlTypes.h\
	$(DMFDIR)/Dmf.h\
	$(DMFDIR)/ErrorCodes.h\
	$(DMFDIR)/GetFuncs.h\
	$(DMFDIR)/Param.h\
	$(DMFDIR)/ParamCodes.h\
	$(DMFDIR)/SetFuncs.h\
	$(CHAP)/UserFunc.h\
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/emfdb.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/webs.h\
	

$(CHAP)/UserFunc.o : \
	$(DMFDIR)/Access.h\
	$(DMFDIR)/AlTypes.h\
	$(DMFDIR)/Dmf.h\
	$(DMFDIR)/ErrorCodes.h\
	$(DMFDIR)/Locals.h\
	$(DMFDIR)/Param.h\
	$(DMFDIR)/ParamCodes.h\
	$(CHAP)/UserFunc.h\
	$(GAWSDIR)/ej.h\
	$(GAWSDIR)/ejIntrn.h\
	$(GAWSDIR)/emfdb.h\
	$(GAWSDIR)/uemf.h\
	$(GAWSDIR)/um.h\
	$(GAWSDIR)/webs.h\
	
