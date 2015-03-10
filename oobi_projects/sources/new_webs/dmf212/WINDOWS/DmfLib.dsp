# Microsoft Developer Studio Project File - Name="DmfLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DmfLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DmfLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DmfLib.mak" CFG="DmfLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DmfLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DmfLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DmfLib - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "..\webs" /I "..\AlFrame" /I "..\expat\xmlparse" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DmfLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DmfLib___Win32_Debug"
# PROP BASE Intermediate_Dir "DmfLib___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugLib"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\expat\xmlparse" /I "..\webs" /I "..\AlFrame" /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /D "B_STATS" /YX /FD /GZ /c
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

# Name "DmfLib - Win32 Release"
# Name "DmfLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\AlFrame\Access.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Cookie.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Dmf.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\EjFuncs.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Error.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Forms.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\GetFuncs.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\GetSet.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Localize.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Locals.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Parse.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\RpcUtil.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Session.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\SetFuncs.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Soap.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\StringBuf.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\XmlRpc.c
# End Source File
# Begin Source File

SOURCE=..\AlFrame\XmlRpcFuncs.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\AlFrame\Access.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\AlTypes.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Cookie.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Dmf.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\EjFuncs.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Error.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\ErrorCodes.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Forms.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\GetFuncs.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\GetSet.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Localize.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Locals.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Param.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\ParamCodes.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Parse.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\RpcUtil.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Session.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\SetFuncs.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Smtp.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\Soap.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\StringBuf.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\XmlRpc.h
# End Source File
# Begin Source File

SOURCE=..\AlFrame\XmlRpcFuncs.h
# End Source File
# End Group
# End Target
# End Project
