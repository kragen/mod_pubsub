# Microsoft Developer Studio Project File - Name="LibKNCpp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LibKNCpp - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LibKNCpp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LibKNCpp.mak" CFG="LibKNCpp - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LibKNCpp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "LibKNCpp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "LibKNCpp - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "LibKNCpp - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LibKNCpp - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "LIBKN_INTERNAL_BUILD" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\LibKN10X86ND.lib"

!ELSEIF  "$(CFG)" == "LibKNCpp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "LIBKN_INTERNAL_BUILD" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\LibKN10X86D.lib"

!ELSEIF  "$(CFG)" == "LibKNCpp - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "LibKNCpp___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "LibKNCpp___Win32_Unicode_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Unicode_Debug"
# PROP Intermediate_Dir "Unicode_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "LIBKN_INTERNAL_BUILD" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "UNICODE" /D "_UNICODE" /D "LIBKN_INTERNAL_BUILD" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\LibKN10X86D.lib"
# ADD LIB32 /nologo /out:"..\lib\LibKN10X86UD.lib"

!ELSEIF  "$(CFG)" == "LibKNCpp - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "LibKNCpp___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "LibKNCpp___Win32_Unicode_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode_Release"
# PROP Intermediate_Dir "Unicode_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "LIBKN_INTERNAL_BUILD" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "UNICODE" /D "_UNICODE" /D "LIBKN_INTERNAL_BUILD" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\LibKN10X86ND.lib"
# ADD LIB32 /nologo /out:"..\lib\LibKN10X86UND.lib"

!ENDIF 

# Begin Target

# Name "LibKNCpp - Win32 Release"
# Name "LibKNCpp - Win32 Debug"
# Name "LibKNCpp - Win32 Unicode Debug"
# Name "LibKNCpp - Win32 Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Connector.cpp
# End Source File
# Begin Source File

SOURCE=.\CS.cpp
# End Source File
# Begin Source File

SOURCE=.\HttpParamString.cpp
# End Source File
# Begin Source File

SOURCE=.\IRequestStatusHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\Journal.cpp
# End Source File
# Begin Source File

SOURCE=.\Logger.cpp
# End Source File
# Begin Source File

SOURCE=.\Message.cpp
# End Source File
# Begin Source File

SOURCE=.\OfflineQ.cpp
# End Source File
# Begin Source File

SOURCE=.\RandRHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleParser.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StrUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Transport.cpp
# End Source File
# Begin Source File

SOURCE=.\Tunnel.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\LibKN\Connector.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\CS.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\Defs.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\HttpParamString.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\IConnectionStatusHandler.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\IListener.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\IRequestStatusHandler.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\IStatusHandler.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\ITransport.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\Journal.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\Lock.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\Logger.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\Message.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\OfflineQ.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\RandRHandler.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\SimpleParser.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\StrUtil.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\Transport.h
# End Source File
# Begin Source File

SOURCE=..\include\LibKN\Tunnel.h
# End Source File
# End Group
# End Target
# End Project
