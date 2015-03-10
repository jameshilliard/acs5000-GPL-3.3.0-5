# Microsoft Developer Studio Project File - Name="tutorial" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=tutorial - Win32 Ch15
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tutorial.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tutorial.mak" CFG="tutorial - Win32 Ch15"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tutorial - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch03" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch04" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch05" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch06" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch07" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch08" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch09" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch10" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch16" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch11" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch12" (based on "Win32 (x86) Application")
!MESSAGE "tutorial - Win32 Ch15" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tutorial - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "tutorial___Win32_Release"
# PROP BASE Intermediate_Dir "tutorial___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "tutorial___Win32_Release"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Ch03"
# PROP BASE Intermediate_Dir "Ch03"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch04"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch04"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch05"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch05"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch06"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch06"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch07"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch07"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch08"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch08"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch09"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch09"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch10"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch10"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch16"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch16"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib advapi32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch11"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch11"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch12"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch12"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tutorial___Win32_Ch150"
# PROP BASE Intermediate_Dir "tutorial___Win32_Ch150"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\expat\xmlparse" /I "..\..\webs" /I "..\..\AlFrame" /I "ch03" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "USER_MANAGEMENT_SUPPORT" /D "qRichErrorPage" /D "qDebug" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\expat\lib" /libpath:"..\..\Windows\DebugLib"

!ENDIF 

# Begin Target

# Name "tutorial - Win32 Release"
# Name "tutorial - Win32 Debug"
# Name "tutorial - Win32 Ch03"
# Name "tutorial - Win32 Ch04"
# Name "tutorial - Win32 Ch05"
# Name "tutorial - Win32 Ch06"
# Name "tutorial - Win32 Ch07"
# Name "tutorial - Win32 Ch08"
# Name "tutorial - Win32 Ch09"
# Name "tutorial - Win32 Ch10"
# Name "tutorial - Win32 Ch16"
# Name "tutorial - Win32 Ch11"
# Name "tutorial - Win32 Ch12"
# Name "tutorial - Win32 Ch15"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "DMF"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\AlFrame\Access.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\Cookie.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\Dmf.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\EjFuncs.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\Error.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\Forms.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\GetFuncs.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\GetSet.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\Localize.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\Locals.c
# End Source File
# Begin Source File

SOURCE=..\..\WINDOWS\main.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\Parse.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\RpcUtil.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\Session.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\SetFuncs.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\Soap.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\StringBuf.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\XmlRpc.c
# End Source File
# Begin Source File

SOURCE=..\..\AlFrame\XmlRpcFuncs.c
# End Source File
# End Group
# Begin Group "GoAhead"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\webs\asp.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\balloc.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\base64.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\cgi.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\default.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\ejlex.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\ejparse.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\emfdb.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\form.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\h.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\handler.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\md5c.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\mime.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\page.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\ringq.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\rom.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\security.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\sock.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\sockGen.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\sym.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\uemf.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\um.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\url.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\value.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\webrom.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\webs.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\websda.c
# End Source File
# Begin Source File

SOURCE=..\..\webs\websuemf.c
# End Source File
# End Group
# Begin Group "ch03"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch03\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch03\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch03\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch03\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "ch04"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch04\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch04\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP BASE Intermediate_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch04\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch04\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP BASE Intermediate_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "ch05"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch05\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch05\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch05\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch05\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch05\user\Values.asp
# End Source File
# End Group
# Begin Group "ch06"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch06\admin\EditUser.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch06\home.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch06\user\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch06\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch06\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch06\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch06\admin\UserList.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch06\user\Values.asp
# End Source File
# End Group
# Begin Group "ch07"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch07\tech\ConfigValues.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch07\admin\EditUser.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch07\home.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch07\user\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch07\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP BASE Intermediate_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch07\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch07\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch07\admin\UserList.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch07\user\Values.asp
# End Source File
# End Group
# Begin Group "ch08"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch08\admin\Config.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch08\tech\ConfigValues.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch08\admin\EditUser.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch08\home.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch08\user\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch08\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch08\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch08\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Exclude_From_Build 1
# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch08\admin\UserList.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch08\user\Values.asp
# End Source File
# End Group
# Begin Group "ch09"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch09\admin\Config.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch09\tech\ConfigValues.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch09\admin\EditUser.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch09\home.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch09\user\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch09\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch09\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch09\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch09\admin\UserList.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch09\user\Values.asp
# End Source File
# End Group
# Begin Group "ch10"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch10\admin\Config.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch10\tech\ConfigValues.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch10\admin\EditUser.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch10\error.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch10\error400.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch10\error404.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch10\home.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch10\user\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch10\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch10\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch10\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch10\admin\UserList.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch10\user\Values.asp
# End Source File
# End Group
# Begin Group "ch11"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch11\user\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch11\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch11\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch11\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch11\user\Values.asp
# End Source File
# End Group
# Begin Group "ch12"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch12\admin\Config.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch12\tech\ConfigValues.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch12\admin\EditUser.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch12\error.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch12\error400.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch12\error404.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch12\home.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch12\user\home.asp
# End Source File
# Begin Source File

SOURCE=.\ch12\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch12\user\SystemInfo.asp
# End Source File
# Begin Source File

SOURCE=.\ch12\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP Intermediate_Dir "Debug"
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch12\admin\UserList.asp
# End Source File
# Begin Source File

SOURCE=..\Web\ch12\user\Values.asp
# End Source File
# End Group
# Begin Group "ch15"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ch15\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP Intermediate_Dir "Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ch15\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP Intermediate_Dir "Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "ch16"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Web\ch16\admin\Config.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\tech\ConfigValues.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\admin\EditUser.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\error.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\error400.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\error404.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\home.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\user\home.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ch16\Param.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\user\SystemInfo.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ch16\UserFunc.c

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\admin\UserList.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Web\ch16\user\Values.asp

!IF  "$(CFG)" == "tutorial - Win32 Release"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch03"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch04"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch05"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch06"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch07"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch08"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch09"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch10"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch16"

# PROP Intermediate_Dir "Debug"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch11"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch12"

!ELSEIF  "$(CFG)" == "tutorial - Win32 Ch15"

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
