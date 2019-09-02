# Microsoft Developer Studio Project File - Name="MMSwitch" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MMSwitch - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MMSwitch.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MMSwitch.mak" CFG="MMSwitch - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MMSwitch - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MMSwitch - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MMSwitch - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gz /MD /W3 /O1 /D "NDEBUG" /D "INC_OLE2" /D "STRICT" /D "WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D try=__try /D except=__except /D leave=__leave /D finally=__finally /U "DBG" /U "DEBUG" /FR /Fo".\Release/" /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 shell32.lib strmbase.lib quartz.lib vfw32.lib winmm.lib kernel32.lib advapi32.lib version.lib largeint.lib user32.lib gdi32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1d1c0000" /entry:"DllEntryPoint@12" /dll /pdb:none /machine:I386 /out:"MMSwitch.ax" /subsystem:windows,4.0 /OPT:NOWIN98
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
TargetPath=.\MMSwitch.ax
SOURCE="$(InputPath)"
PostBuild_Desc=UPX & Register ...
PostBuild_Cmds=REM ..\UPX\UPX -9 --best --strip-relocs=0 --compress-icons=1 $(TargetPath)	regsvr32.exe /s $(TargetPath)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "MMSwitch - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gz /MDd /W3 /Z7 /Gy /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /FR /Oid /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 shell32.lib strmbasd.lib quartz.lib msvcrtd.lib vfw32.lib winmm.lib kernel32.lib advapi32.lib version.lib largeint.lib user32.lib gdi32.lib comctl32.lib ole32.lib olepro32.lib oleaut32.lib uuid.lib /nologo /base:"0x1d1c0000" /entry:"DllEntryPoint@12" /dll /pdb:none /machine:I386 /nodefaultlib /out:"MMSwitch.ax" /align:0x1000 /debug:mapped,full /subsystem:windows,4.0
# Begin Special Build Tool
TargetPath=.\MMSwitch.ax
SOURCE="$(InputPath)"
PostBuild_Desc=Register filter ...
PostBuild_Cmds=regsvr32.exe /s $(TargetPath)
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "MMSwitch - Win32 Release"
# Name "MMSwitch - Win32 Debug"
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\iMMSwitch.h
# End Source File
# Begin Source File

SOURCE=.\MMSwitchprop.h
# End Source File
# Begin Source File

SOURCE=.\MMSwitchuids.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\IconStream.bmp
# End Source File
# Begin Source File

SOURCE=.\anim\IconStream0.png
# End Source File
# Begin Source File

SOURCE=.\anim\IconStream1.png
# End Source File
# Begin Source File

SOURCE=.\anim\IconStream2.png
# End Source File
# Begin Source File

SOURCE=.\anim\IconStream3.png
# End Source File
# Begin Source File

SOURCE=.\MMSwitch.cpp
DEP_CPP_MMSWI=\
	".\iMMSwitch.h"\
	".\MMSwitchprop.h"\
	".\MMSwitchuids.h"\
	".\wmp.h"\
	{$(INCLUDE)}"amextra.h"\
	{$(INCLUDE)}"amfilter.h"\
	{$(INCLUDE)}"audevcod.h"\
	{$(INCLUDE)}"cache.h"\
	{$(INCLUDE)}"combase.h"\
	{$(INCLUDE)}"cprop.h"\
	{$(INCLUDE)}"ctlutil.h"\
	{$(INCLUDE)}"dllsetup.h"\
	{$(INCLUDE)}"dsschedule.h"\
	{$(INCLUDE)}"fourcc.h"\
	{$(INCLUDE)}"measure.h"\
	{$(INCLUDE)}"msgthrd.h"\
	{$(INCLUDE)}"mtype.h"\
	{$(INCLUDE)}"outputq.h"\
	{$(INCLUDE)}"pstream.h"\
	{$(INCLUDE)}"refclock.h"\
	{$(INCLUDE)}"reftime.h"\
	{$(INCLUDE)}"renbase.h"\
	{$(INCLUDE)}"source.h"\
	{$(INCLUDE)}"streams.h"\
	{$(INCLUDE)}"strmctl.h"\
	{$(INCLUDE)}"sysclock.h"\
	{$(INCLUDE)}"transfrm.h"\
	{$(INCLUDE)}"transip.h"\
	{$(INCLUDE)}"videoctl.h"\
	{$(INCLUDE)}"vtrans.h"\
	{$(INCLUDE)}"winctrl.h"\
	{$(INCLUDE)}"winutil.h"\
	{$(INCLUDE)}"wxdebug.h"\
	{$(INCLUDE)}"wxlist.h"\
	{$(INCLUDE)}"wxutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MMSwitch.def
# End Source File
# Begin Source File

SOURCE=.\MMSwitchprop.cpp
DEP_CPP_MMSWIT=\
	".\iMMSwitch.h"\
	".\MMSwitchprop.h"\
	".\MMSwitchuids.h"\
	{$(INCLUDE)}"amextra.h"\
	{$(INCLUDE)}"amfilter.h"\
	{$(INCLUDE)}"audevcod.h"\
	{$(INCLUDE)}"cache.h"\
	{$(INCLUDE)}"combase.h"\
	{$(INCLUDE)}"cprop.h"\
	{$(INCLUDE)}"ctlutil.h"\
	{$(INCLUDE)}"dllsetup.h"\
	{$(INCLUDE)}"dsschedule.h"\
	{$(INCLUDE)}"fourcc.h"\
	{$(INCLUDE)}"measure.h"\
	{$(INCLUDE)}"msgthrd.h"\
	{$(INCLUDE)}"mtype.h"\
	{$(INCLUDE)}"outputq.h"\
	{$(INCLUDE)}"pstream.h"\
	{$(INCLUDE)}"refclock.h"\
	{$(INCLUDE)}"reftime.h"\
	{$(INCLUDE)}"renbase.h"\
	{$(INCLUDE)}"source.h"\
	{$(INCLUDE)}"streams.h"\
	{$(INCLUDE)}"strmctl.h"\
	{$(INCLUDE)}"sysclock.h"\
	{$(INCLUDE)}"transfrm.h"\
	{$(INCLUDE)}"transip.h"\
	{$(INCLUDE)}"videoctl.h"\
	{$(INCLUDE)}"vtrans.h"\
	{$(INCLUDE)}"winctrl.h"\
	{$(INCLUDE)}"winutil.h"\
	{$(INCLUDE)}"wxdebug.h"\
	{$(INCLUDE)}"wxlist.h"\
	{$(INCLUDE)}"wxutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MMSwitchprop.rc
# End Source File
# Begin Source File

SOURCE=.\Morgan.bmp
# End Source File
# Begin Source File

SOURCE=.\raw1.bin
# End Source File
# End Target
# End Project
