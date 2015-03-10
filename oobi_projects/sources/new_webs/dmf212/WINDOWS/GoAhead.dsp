# Microsoft Developer Studio Project File - Name="GoAhead" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GoAhead - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GoAhead.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GoAhead.mak" CFG="GoAhead - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GoAhead - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GoAhead - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GoAhead - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GoAhead - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GoAhead___Win32_Debug"
# PROP BASE Intermediate_Dir "GoAhead___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugLib"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /D "_LIB" /D "B_STATS" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "GoAhead - Win32 Release"
# Name "GoAhead - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\webs\asp.c
# End Source File
# Begin Source File

SOURCE=..\webs\balloc.c
# End Source File
# Begin Source File

SOURCE=..\webs\base64.c
# End Source File
# Begin Source File

SOURCE=..\webs\cgi.c
# End Source File
# Begin Source File

SOURCE=..\webs\default.c
# End Source File
# Begin Source File

SOURCE=..\webs\ejlex.c
# End Source File
# Begin Source File

SOURCE=..\webs\ejparse.c
# End Source File
# Begin Source File

SOURCE=..\webs\emfdb.c
# End Source File
# Begin Source File

SOURCE=..\webs\form.c
# End Source File
# Begin Source File

SOURCE=..\webs\h.c
# End Source File
# Begin Source File

SOURCE=..\webs\handler.c
# End Source File
# Begin Source File

SOURCE=..\webs\md5c.c
# End Source File
# Begin Source File

SOURCE=..\webs\mime.c
# End Source File
# Begin Source File

SOURCE=..\webs\misc.c
# End Source File
# Begin Source File

SOURCE=..\webs\page.c
# End Source File
# Begin Source File

SOURCE=..\webs\ringq.c
# End Source File
# Begin Source File

SOURCE=..\webs\rom.c
# End Source File
# Begin Source File

SOURCE=..\webs\security.c
# End Source File
# Begin Source File

SOURCE=..\webs\sock.c
# End Source File
# Begin Source File

SOURCE=..\webs\sockGen.c
# End Source File
# Begin Source File

SOURCE=..\webs\sym.c
# End Source File
# Begin Source File

SOURCE=..\webs\uemf.c
# End Source File
# Begin Source File

SOURCE=..\webs\um.c
# End Source File
# Begin Source File

SOURCE=..\webs\url.c
# End Source File
# Begin Source File

SOURCE=..\webs\value.c
# End Source File
# Begin Source File

SOURCE=..\webs\webrom.c
# End Source File
# Begin Source File

SOURCE=..\webs\webs.c
# End Source File
# Begin Source File

SOURCE=..\webs\websda.c
# End Source File
# Begin Source File

SOURCE=..\webs\websSSL.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\webs\websuemf.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\webs\ej.h
# End Source File
# Begin Source File

SOURCE=..\webs\ejIntrn.h
# End Source File
# Begin Source File

SOURCE=..\webs\emfdb.h
# End Source File
# Begin Source File

SOURCE=..\webs\md5.h
# End Source File
# Begin Source File

SOURCE=..\webs\uemf.h
# End Source File
# Begin Source File

SOURCE=..\webs\um.h
# End Source File
# Begin Source File

SOURCE=..\webs\webs.h
# End Source File
# Begin Source File

SOURCE=..\webs\websda.h
# End Source File
# Begin Source File

SOURCE=..\webs\websSSL.h
# End Source File
# Begin Source File

SOURCE=..\webs\wsIntrn.h
# End Source File
# End Group
# End Target
# End Project
