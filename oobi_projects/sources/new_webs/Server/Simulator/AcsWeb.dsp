# Microsoft Developer Studio Project File - Name="AcsWeb" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=AcsWeb - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AcsWeb.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AcsWeb.mak" CFG="AcsWeb - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AcsWeb - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "AcsWeb___Win32_Debug"
# PROP BASE Intermediate_Dir "AcsWeb___Win32_Debug"
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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\..\Dmf2.1.2\expat\xmlparse" /I "..\..\..\..\Dmf2.1.2\webs" /I "..\..\..\..\Dmf2.1.2\AlFrame" /I "..\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "qHierarchicalAccess" /D "qDmf" /D "WIN" /D "WEBS" /D "UEMF" /D "DIGEST_ACCESS_SUPPORT" /D "qDebug" /D qPort=8085 /FD /GZ /c
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
# ADD LINK32 wsock32.lib expat.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\..\Dmf2.1.2\expat\lib" /libpath:"..\..\..\..\Dmf2.1.2\Windows\DebugLib"
# Begin Target

# Name "AcsWeb - Win32 Debug"
# Begin Group "Notes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\ReadMe.txt
# End Source File
# End Group
# Begin Group "AcsWeb"

# PROP Default_Filter ""
# Begin Group "Server"

# PROP Default_Filter ""
# Begin Group "Engine Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\webApi.h
# End Source File
# End Group
# Begin Group "Simulator Implementation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Acs.c
# End Source File
# End Group
# Begin Group "Middleware"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Param.c
# End Source File
# Begin Source File

SOURCE=..\UserFunc.c
# End Source File
# Begin Source File

SOURCE=..\UserFunc.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Bounds.h
# End Source File
# Begin Source File

SOURCE=..\ResultCodes.h
# End Source File
# Begin Source File

SOURCE=..\SystemData.h
# End Source File
# End Group
# Begin Group "Web"

# PROP Default_Filter ""
# Begin Group "Wizard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Web\wizard\access.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\addModGroup.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\addModUser.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\authSecurity.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\CASProSettings.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\controls.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\dataBuffering.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\main.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\menu.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\menuBottom.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\netSettings.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\wizard\systemLog.asp
# End Source File
# End Group
# Begin Group "Normal (Expert)"

# PROP Default_Filter ""
# Begin Group "Access"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Web\normal\access\accAuth.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\access\accAuthentication.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\access\accessMenu.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\access\accPortSessions.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\access\accTS_ACSSettings.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\access\accUsersGroups.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\access\addModGroup.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\access\addModUser.asp
# End Source File
# End Group
# Begin Group "Administration"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Web\normal\administration\adminBootConfig.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\administration\adminMenu.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\administration\adminNotifications.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\administration\adminReboot.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\administration\adminSystemInfo.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\administration\adminTimeDate.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\administration\adminUpgradeFirm.asp
# End Source File
# End Group
# Begin Group "Applications"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsConnect.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsConnectACS.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsMenu.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPhysPorts.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPorts.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPortsAccess.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPortsDataBuff.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPortSettings.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPortsMUS.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPortsOther.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPortsSysLog.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPortsSysParam.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPowerMan.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPowerManAdmin.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPowerManConfig.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPowManConfigOutlet.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsPowManConfigPort.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsTSParam.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsVirtPorts.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\appsVirtPortsClustSet.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\physPorts.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\tabsPhysPorts.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\tabsVirtPorts.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\virtPorts.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\applications\virtPortsSettings.asp
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Web\normal\network\netAccessMeth.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netAccessMethConfig.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netAccessMethod.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netClustSettings.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netHostSettings.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netHostTable.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netIPChains.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netIPTables.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netRoutingTable.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netServices.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netSNMP.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netStaticRoutes.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\netVPNConnect.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\network\networkMenu.asp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Web\normal\controls.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\main.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\menu.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\menuBottom.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\menuBottomPlain.html
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\noControls.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\normal\topMenu.asp
# End Source File
# End Group
# Begin Group "Supplementary"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Web\scripts.js
# End Source File
# Begin Source File

SOURCE=..\..\Web\stylesLayout.css
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Web\login.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\loginConfirm.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\redirect.asp
# End Source File
# Begin Source File

SOURCE=..\..\Web\top.asp
# End Source File
# End Group
# Begin Group "Locale"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Locale\language.en
# End Source File
# End Group
# End Group
# Begin Group "Dmf"

# PROP Default_Filter ""
# Begin Group "Main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\WINDOWS\main.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Access.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Cookie.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Dmf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\EjFuncs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Error.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Forms.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\GetFuncs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\GetSet.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Localize.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Locals.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Parse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\RpcUtil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Session.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\SetFuncs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\Soap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\StringBuf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\XmlRpc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\AlFrame\XmlRpcFuncs.c
# End Source File
# End Group
# Begin Group "GoAhead"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\asp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\balloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\base64.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\cgi.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\default.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\ejlex.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\ejparse.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\emfdb.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\form.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\h.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\handler.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\md5c.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\mime.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\page.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\ringq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\rom.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\security.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\sock.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\sockGen.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\sym.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\uemf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\um.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\url.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\value.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\webrom.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\webs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\websda.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Dmf2.1.2\webs\websuemf.c
# End Source File
# End Group
# End Target
# End Project
