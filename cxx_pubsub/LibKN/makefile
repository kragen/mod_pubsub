# @KNOWNOW_LICENSE_START@
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
# 
# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @KNOWNOW_LICENSE_END@

!include "paths.mak"

#
# Common dependencies - Every build depends on these
#
COMMON_DEP=makefile \
	   buildlogdir \
	   packagedir_target

#
# VC6 options
#
VC6_DSW=VC6\VC6.dsw
VC6_LOGDIR=.\BuildLogs\VC6
VC6_DEP=LibKNCpp_vc6 LibKNCom_vc6 \
	Functional_vc6 \
	PubTest_vc6 \
	PubTestMT_vc6 \
	QTest_vc6 \
	SubTest_vc6 \
	SubTestCounted_vc6 \
	SubTestMT_vc6 
	
#
# VC7 options
#
VC7_DSW=VC7\VC7.sln
VC7_LOGDIR=.\BuildLogs\VC7
VC7_DEP=LibKNCpp_vc7 LibKNCom_vc7 LibKNDotNet_vc7 \
	MessageTest1_vc7 \
	PubTest1_vc7 \
	PubTestMT_vc7 \
	RssPubNet_vc7 \
	SubTest1_vc7 \
	SubTest2_vc7 \
	SubTestMT_vc7 
	
#
# VC71 options
#
VC71_DSW=VC71\VC71.sln
VC71_LOGDIR=.\BuildLogs\VC71
VC71_DEP=LibKNCpp_vc71 LibKNCom_vc71 LibKNDotNet_vc71
	
#
# EVC3 options (PocketPC 2002)
#
EVC3_DSW=EVC3\EVC3.VCW
EVC3_LOGDIR=.\BuildLogs\EVC3
EVC3_DEP=LibKNCpp_evc3 LibKNCom_evc3

#
# EVC3_2K options (PocketPC 2000)
#
EVC3_2K_DSW=EVC3.2k\EVC3.VCW
EVC3_2K_LOGDIR=.\BuildLogs\EVC3.2k
EVC3_2K_DEP=LibKNCpp_evc3_2k LibKNCom_evc3_2k

!ifdef VC6
VC6_BUILDTARGET=vc6
!else
VC6_BUILDTARGET=
!endif

!ifdef VC7
VC7_BUILDTARGET=vc7
!else
VC7_BUILDTARGET=
!endif

!ifdef VC71
VC71_BUILDTARGET=vc71
!else
VC71_BUILDTARGET=
!endif

!ifdef EVC3
EVC3_BUILDTARGET=evc3
!else
EVC3_BUILDTARGET=
!endif

!ifdef EVC3_2K
EVC3_2K_BUILDTARGET=evc3_2k
!else
EVC3_2K_BUILDTARGET=
!endif

!ifdef DOXYGEN
DOCS_BUILDTARGET=docs
!else
DOCS_BUILDTARGET=
!endif

!ifdef WISE
INSTALLS_BUILDTARGET=installs
!else
INSTALLS_BUILDTARGET=
!endif


#
# By default, build everything
#
all: $(VC6_BUILDTARGET) $(VC7_BUILDTARGET) $(VC71_BUILDTARGET) $(EVC3_BUILDTARGET) \
	$(EVC3_2K_BUILDTARGET) $(DOCS_BUILDTARGET) \
	packages 
# $(INSTALLS_BUILDTARGET) # no longer build installs.

#
# Create the build log directories
#
buildlogdir:
!if !exist(BuildLogs) 
	@md BuildLogs
!endif
!if !exist(BuildLogs\VC6)
	@md BuildLogs\VC6
!endif
!if !exist(BuildLogs\VC7)
	@md BuildLogs\VC7
!endif
!if !exist(BuildLogs\VC71)
	@md BuildLogs\VC71
!endif
!if !exist(BuildLogs\EVC3)
	@md BuildLogs\EVC3
!endif
!if !exist(BuildLogs\EVC3.2k)
	@md BuildLogs\EVC3.2k
!endif


#
# The various environment builds
#
vc6: $(COMMON_DEP) $(VC6_DEP)

vc7: $(COMMON_DEP) $(VC7_DEP)

vc71: $(COMMON_DEP) $(VC71_DEP)

evc3: $(COMMON_DEP) $(EVC3_DEP)
	
evc3_2k: $(COMMON_DEP) $(EVC3_2K_DEP)


#
# VC6 Build 
#	
LibKNCpp_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "LibKNCpp - Win32 Release" /OUT $(VC6_LOGDIR)\LibKnCppRelease.log
	$(VC6) $(VC6_DSW) /MAKE "LibKNCpp - Win32 Debug" /OUT $(VC6_LOGDIR)\LibKnCppDebug.log
	$(VC6) $(VC6_DSW) /MAKE "LibKNCpp - Win32 Unicode Release" /OUT $(VC6_LOGDIR)\LibKnCppUnicodeRelease.log
	$(VC6) $(VC6_DSW) /MAKE "LibKNCpp - Win32 Unicode Debug" /OUT $(VC6_LOGDIR)\LibKnCppUnicodeDebug.log

ForExcelTarget: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "LibKNCom - Win32 Release MinDependency" /OUT $(VC6_LOGDIR)\LibKnComReleaseMinDep.log
	$(STAMPVER) -vStampVer.inf ComWin32\ReleaseMinDependency\LibKNCom.dll > $(VC6_LOGDIR)\LibKnComReleaseMinDepStampVer.log

LibKNCom_vc6: $(COMMON_DEP) ForExcelTarget
	$(VC6) $(VC6_DSW) /MAKE "LibKNCom - Win32 Debug" /OUT $(VC6_LOGDIR)\LibKnComDebug.log
	$(STAMPVER) -vStampVer.inf ComWin32\Debug\LibKNCom.dll > $(VC6_LOGDIR)\LibKnComDebugStampVer.log
	$(VC6) $(VC6_DSW) /MAKE "LibKNCom - Win32 Unicode Release MinDependency" /OUT $(VC6_LOGDIR)\LibKnComUnicodeReleaseMinDep.log
	$(STAMPVER) -vStampVer.inf ComWin32\ReleaseUMinDependency\LibKNCom.dll > $(VC6_LOGDIR)\LibKnComUnicodeReleaseMinDepStampVer.log
	$(VC6) $(VC6_DSW) /MAKE "LibKNCom - Win32 Unicode Debug" /OUT $(VC6_LOGDIR)\LibKnComUnicodeDebug.log
	$(STAMPVER) -vStampVer.inf ComWin32\DebugU\LibKNCom.dll > $(VC6_LOGDIR)\LibKnComUnicodeDebugStampVer.log

Functional_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "Functional - Win32 Release" /OUT $(VC6_LOGDIR)\FunctionalRelease.log
	$(VC6) $(VC6_DSW) /MAKE "Functional - Win32 Debug" /OUT $(VC6_LOGDIR)\FunctionalDebug.log

PubTest_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "PubTest - Win32 Release" /OUT $(VC6_LOGDIR)\PubTestRelease.log
	$(VC6) $(VC6_DSW) /MAKE "PubTest - Win32 Debug" /OUT $(VC6_LOGDIR)\PubTestDebug.log

PubTestMT_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "PubTestMT - Win32 Release" /OUT $(VC6_LOGDIR)\PubTestMTRelease.log
	$(VC6) $(VC6_DSW) /MAKE "PubTestMT - Win32 Debug" /OUT $(VC6_LOGDIR)\PubTestMTDebug.log

QTest_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "QTest - Win32 Release" /OUT $(VC6_LOGDIR)\QTestRelease.log
	$(VC6) $(VC6_DSW) /MAKE "QTest - Win32 Debug" /OUT $(VC6_LOGDIR)\QTestDebug.log

SubTest_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "SubTest - Win32 Release" /OUT $(VC6_LOGDIR)\SubTestRelease.log
	$(VC6) $(VC6_DSW) /MAKE "SubTest - Win32 Debug" /OUT $(VC6_LOGDIR)\SubTestDebug.log

SubTestCounted_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "SubTestCounted - Win32 Release" /OUT $(VC6_LOGDIR)\SubTestCountedRelease.log
	$(VC6) $(VC6_DSW) /MAKE "SubTestCounted - Win32 Debug" /OUT $(VC6_LOGDIR)\SubTestCountedDebug.log

SubTestMT_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "SubTestMT - Win32 Release" /OUT $(VC6_LOGDIR)\SubTestMTRelease.log
	$(VC6) $(VC6_DSW) /MAKE "SubTestMT - Win32 Debug" /OUT $(VC6_LOGDIR)\SubTestMTDebug.log


#
# VC7 Build
#
LibKNCpp_vc7: $(COMMON_DEP)
	$(VC7) /project LibKNCpp $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNCppRelease.log /build Release
	$(VC7) /project LibKNCpp $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNCppDebug.log /build Debug
	$(VC7) /project LibKNCpp $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNCppUnicodeRelease.log /build "Unicode Release MinDependency"
	$(VC7) /project LibKNCpp $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNCppUnicodeDebug.log /build "Unicode Debug"

LibKNCom_vc7: $(COMMON_DEP)
	$(VC7) /project LibKNCom $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNComRelease.log /build "Release MinDependency"
	$(STAMPVER) -vStampVer.inf ComWin32_7\ReleaseMinDependency\LibKNCom.dll > $(VC7_LOGDIR)\LibKNComReleaseStampVer.log
	$(VC7) /project LibKNCom $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNComDebug.log /build Debug
	$(STAMPVER) -vStampVer.inf ComWin32_7\Debug\LibKNCom.dll > $(VC7_LOGDIR)\LibKNComDebug.StampVerlog 
	$(VC7) /project LibKNCom $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNComUnicodeRelease.log /build "Unicode Release MinDependency"
	$(STAMPVER) -vStampVer.inf ComWin32_7\ReleaseUMinDependency\LibKNCom.dll > $(VC7_LOGDIR)\LibKNComUnicodeReleaseStampVer.log
	$(VC7) /project LibKNCom $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNComUnicodeDebug.log /build "Unicode Debug"
	$(STAMPVER) -vStampVer.inf ComWin32_7\DebugU\LibKNCom.dll > $(VC7_LOGDIR)\LibKNComUnicodeDebugStampVer.log

LibKNDotNet_vc7: $(COMMON_DEP)
	$(VC7) /project LibKNDotNet $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNDotNetRelease.log /build Release
	$(STAMPVER) -vStampVer.inf Lib\LibKNDotNet.dll > $(VC7_LOGDIR)\LibKNDotNetRelease.StampVerLog
	$(VC7) /project LibKNDotNet $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNDotNetDebug.log /build Debug
	$(STAMPVER) -vStampVer.inf Lib\LibKNDotNetD.dll > $(VC7_LOGDIR)\LibKNDotNetDebug.StampVerLog

MessageTest1_vc7: $(COMMON_DEP)
	$(VC7) /project MessageTest1 $(VC7_DSW) /OUT $(VC7_LOGDIR)\MessageTest1Release.log /build Release
	$(VC7) /project MessageTest1 $(VC7_DSW) /OUT $(VC7_LOGDIR)\MessageTest1Debug.log /build Debug
	
PubTest1_vc7: $(COMMON_DEP)	
	$(VC7) /project PubTest1 $(VC7_DSW) /OUT $(VC7_LOGDIR)\PubTest1Release.log /build Release
	$(VC7) /project PubTest1 $(VC7_DSW) /OUT $(VC7_LOGDIR)\PubTest1Debug.log /build Debug

PubTestMT_vc7: $(COMMON_DEP)	
	$(VC7) /project PubTestMT $(VC7_DSW) /OUT $(VC7_LOGDIR)\PubTestMTRelease.log /build Release
	$(VC7) /project PubTestMT $(VC7_DSW) /OUT $(VC7_LOGDIR)\PubTestMTDebug.log /build Debug

RssPubNet_vc7: $(COMMON_DEP)	
	$(VC7) /project RssPub.Net $(VC7_DSW) /OUT $(VC7_LOGDIR)\RssPubNetRelease.log /build Release
	$(VC7) /project RssPub.Net $(VC7_DSW) /OUT $(VC7_LOGDIR)\RssPubNetDebug.log /build Debug
	
SubTest1_vc7: $(COMMON_DEP)
	$(VC7) /project SubTest1 $(VC7_DSW) /OUT $(VC7_LOGDIR)\SubTest1Release.log /build Release
	$(VC7) /project SubTest1 $(VC7_DSW) /OUT $(VC7_LOGDIR)\SubTest1Debug.log /build Debug
	
SubTest2_vc7: $(COMMON_DEP)
	$(VC7) /project SubTest2 $(VC7_DSW) /OUT $(VC7_LOGDIR)\SubTest2Release.log /build Release
	$(VC7) /project SubTest2 $(VC7_DSW) /OUT $(VC7_LOGDIR)\SubTest2Debug.log /build Debug
	
SubTestMT_vc7: $(COMMON_DEP)
	$(VC7) /project SubTestMT $(VC7_DSW) /OUT $(VC7_LOGDIR)\SubTestMTRelease.log /build Release
	$(VC7) /project SubTestMT $(VC7_DSW) /OUT $(VC7_LOGDIR)\SubTestMTDebug.log  /build Debug


#
# VC71 Build
#
LibKNCpp_vc71: $(COMMON_DEP)
	$(VC71) /project LibKNCpp $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNCppRelease.log /build Release
	$(VC71) /project LibKNCpp $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNCppDebug.log /build Debug
	$(VC71) /project LibKNCpp $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNCppUnicodeRelease.log /build "Unicode Release MinDependency"
	$(VC71) /project LibKNCpp $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNCppUnicodeDebug.log /build "Unicode Debug"

LibKNCom_vc71: $(COMMON_DEP)
	$(VC71) /project LibKNCom $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNComRelease.log /build "Release MinDependency"
	$(STAMPVER) -vStampVer.inf ComWin32_71\ReleaseMinDependency\LibKNCom.dll > $(VC71_LOGDIR)\LibKNComRelease.StampVerlog 
	$(VC71) /project LibKNCom $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNComDebug.log /build Debug
	$(STAMPVER) -vStampVer.inf ComWin32_71\Debug\LibKNCom.dll > $(VC71_LOGDIR)\LibKNComDebugStampVer.log
	$(VC71) /project LibKNCom $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNComUnicodeRelease.log /build "Unicode Release MinDependency"
	$(STAMPVER) -vStampVer.inf ComWin32_71\ReleaseUMinDependency\LibKNCom.dll > $(VC71_LOGDIR)\LibKNComUnicodeReleaseStampVer.log
	$(VC71) /project LibKNCom $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNComUnicodeDebug.log /build "Unicode Debug"
	$(STAMPVER) -vStampVer.inf ComWin32_71\DebugU\LibKNCom.dll > $(VC71_LOGDIR)\LibKNComUnicodeDebugStampVer.log

LibKNDotNet_vc71: $(COMMON_DEP)
	$(VC71) /project LibKNDotNet $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNDotNetRelease.log /build Release
	$(STAMPVER) -vStampVer.inf Lib\LibKNDotNet_71.dll > $(VC71_LOGDIR)\LibKNDotNetRelease.StampVerLog
	$(VC71) /project LibKNDotNet $(VC71_DSW) /OUT $(VC71_LOGDIR)\LibKNDotNetDebug.log /build Debug
	$(STAMPVER) -vStampVer.inf Lib\LibKNDotNet_71D.dll > $(VC71_LOGDIR)\LibKNDotNetDebug.StampVerLog
	

#
# EVC3 Build
#
LibKNCpp_evc3: $(COMMON_DEP)
!if defined(SUPPORT_X86)
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCpp - Win32 (WCE x86) Release" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNCppX86Release.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCpp - Win32 (WCE x86) Debug" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNCppX86Debug.log
!endif	
!if defined(SUPPORT_ARM)
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCpp - Win32 (WCE ARM) Release" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNCppARMRelease.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCpp - Win32 (WCE ARM) Debug" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNCppARMDebug.log
!endif	

LibKNCom_evc3: $(COMMON_DEP)
!if defined(SUPPORT_X86)
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCom - Win32 (WCE x86) Release MinDependency" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNComX86Release.log
	$(STAMPVER) -vStampVer.inf ComCe\X86RelMinDependency\LibKNCom.dll > $(EVC3_LOGDIR)\LibKNComX86ReleaseStampVer.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCom - Win32 (WCE x86) Debug" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNComX86Debug.log
	$(STAMPVER) -vStampVer.inf ComCe\X86Dbg\LibKNCom.dll > $(EVC3_LOGDIR)\LibKNComX86DebugStampVer.log
!endif	
!if defined(SUPPORT_ARM)
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCom - Win32 (WCE ARM) Release MinDependency" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNComARMRelease.log
	$(STAMPVER) -vStampVer.inf ComCe\ARMRelMinDependency\LibKNCom.dll > $(EVC3_LOGDIR)\LibKNComARMReleaseStampVer.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCom - Win32 (WCE ARM) Debug" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNComARMDebug.log
	$(STAMPVER) -vStampVer.inf ComCe\ARMDbg\LibKNCom.dll > $(EVC3_LOGDIR)\LibKNComARMDebugStampVer.log
!endif	


#
# EVC3_2k Build
#
LibKNCpp_evc3_2k: $(COMMON_DEP)
!if defined(SUPPORT_X86)
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCpp - Win32 (WCE x86) Release" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNCppX86Release.log
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCpp - Win32 (WCE x86) Debug" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNCppX86Debug.log
!endif	
!if defined(SUPPORT_ARM)
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCpp - Win32 (WCE ARM) Release" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNCppARMRelease.log
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCpp - Win32 (WCE ARM) Debug" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNCppARMDebug.log
!endif
!if defined(SUPPORT_SH3)
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCpp - Win32 (WCE SH3) Release" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNCppSH3Release.log
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCpp - Win32 (WCE SH3) Debug" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNCppSH3Debug.log
!endif
!if defined(SUPPORT_MIPS)
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCpp - Win32 (WCE MIPS) Release" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNCppMIPSRelease.log
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCpp - Win32 (WCE MIPS) Debug" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNCppMIPSDebug.log
!endif	

LibKNCom_evc3_2k: $(COMMON_DEP)
!if defined(SUPPORT_X86)
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCom - Win32 (WCE x86) Release MinDependency" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNComX86Release.log
	$(STAMPVER) -vStampVer.inf ComCe.2k\X86RelMinDependency\LibKNCom.dll > $(EVC3_2K_LOGDIR)\LibKNComX86ReleaseStampVer.log
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCom - Win32 (WCE x86) Debug" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNComX86Debug.log
	$(STAMPVER) -vStampVer.inf ComCe.2k\X86Dbg\LibKNCom.dll > $(EVC3_2K_LOGDIR)\LibKNComX86DebugStampVer.log
!endif	
!if defined(SUPPORT_ARM)
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCom - Win32 (WCE ARM) Release MinDependency" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNComARMRelease.log
	$(STAMPVER) -vStampVer.inf ComCe.2k\ARMRelMinDependency\LibKNCom.dll > $(EVC3_2K_LOGDIR)\LibKNComARMReleaseStampVer.log
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCom - Win32 (WCE ARM) Debug" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNComARMDebug.log
	$(STAMPVER) -vStampVer.inf ComCe.2k\ARMDbg\LibKNCom.dll > $(EVC3_2K_LOGDIR)\LibKNComARMDebugStampVer.log
!endif	
!if defined(SUPPORT_SH3)
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCom - Win32 (WCE SH3) Release MinDependency" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNComSH3Release.log
	$(STAMPVER) -vStampVer.inf ComCe.2k\SH3RelMinDependency\LibKNCom.dll > $(EVC3_2K_LOGDIR)\LibKNComSH3ReleaseStampVer.log
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCom - Win32 (WCE SH3) Debug" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNComSH3Debug.log
	$(STAMPVER) -vStampVer.inf ComCe.2k\SH3Dbg\LibKNCom.dll > $(EVC3_2K_LOGDIR)\LibKNComSH3DebugStampVer.log
!endif	
!if defined(SUPPORT_MIPS)
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCom - Win32 (WCE MIPS) Release MinDependency" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNComMIPSRelease.log
	$(STAMPVER) -vStampVer.inf ComCe.2k\MIPSRelMinDependency\LibKNCom.dll > $(EVC3_2K_LOGDIR)\LibKNComMIPSReleaseStampVer.log
	$(EVC3_2K) $(EVC3_2K_DSW) /MAKE "LibKNCom - Win32 (WCE MIPS) Debug" /ceconfig="Pocket PC" /OUT $(EVC3_2K_LOGDIR)\LibKNComMIPSDebug.log
	$(STAMPVER) -vStampVer.inf ComCe.2k\MIPSDbg\LibKNCom.dll > $(EVC3_2K_LOGDIR)\LibKNComMIPSDebugStampVer.log
!endif	


#
# The docs
#
docs: $(COMMON_DEP)
	$(DOXYGEN) doxygen.cfg > BuildLogs\doxygen.log
	
	
#
# Installs
#	
installs: wise_desktop wise_ppc2k wise_ppc2k2

wise_desktop: $(COMMON_DEP)
	$(WISE) /c /d Install\Desktop\Desktop.wse
	
wise_ppc2k: $(COMMON_DEP)
	$(WISE) /c /d Install\PPC2k\PocketPC2k.wse

wise_ppc2k2: $(COMMON_DEP)
	$(WISE) /c /d Install\PPC2k2\PocketPC2k2.wse
	

#
# Create the packages directories
#
packagedir_target: packagedir_root packagedir_cpp packagedir_com packagedir_dotnet packagedir_ppc2k packagedir_ppc2k2

#
packagedir_root:
!if !exist(packages)
	@md packages
!endif

#
packagedir_cpp: packagedir_root
!if !exist(packages\cpp)
	@md packages\cpp
!endif
!if !exist(packages\cpp\docs)
	@md packages\cpp\docs
!endif
!if !exist(packages\cpp\docs\doxygen)
	@md packages\cpp\docs\doxygen
!endif
!if !exist(packages\cpp\include)
	@md packages\cpp\include
!endif
!if !exist(packages\cpp\include\libkn)
	@md packages\cpp\include\libkn
!endif
!if !exist(packages\cpp\include\cestl)
	@md packages\cpp\include\cestl
!endif
!if !exist(packages\cpp\include\cewtl)
	@md packages\cpp\include\cewtl
!endif
!if !exist(packages\cpp\lib)
	@md packages\cpp\lib
!endif
!if !exist(packages\cpp\samples)
	@md packages\cpp\samples
!endif
!if !exist(packages\cpp\samples\PubTest)
	@md packages\cpp\samples\PubTest
!endif
!if !exist(packages\cpp\samples\PubTestMT)
	@md packages\cpp\samples\PubTestMT
!endif
!if !exist(packages\cpp\samples\QTest)
	@md packages\cpp\samples\QTest
!endif
!if !exist(packages\cpp\samples\SubTest)
	@md packages\cpp\samples\SubTest
!endif
!if !exist(packages\cpp\samples\SubTestCounted)
	@md packages\cpp\samples\SubTestCounted
!endif
!if !exist(packages\cpp\samples\SubTestMT)
	@md packages\cpp\samples\SubTestMT
!endif

#
packagedir_com: packagedir_root
!if !exist(packages\com)
	@md packages\com
!endif
!if !exist(packages\com\docs)
	@md packages\com\docs
!endif
!if !exist(packages\com\docs\doxygen)
	@md packages\com\docs\doxygen
!endif
!if !exist(packages\com\bin)
	@md packages\com\bin
!endif
!if !exist(packages\com\bin\vs6)
	@md packages\com\bin\vs6
!endif
!if !exist(packages\com\bin\vs7)
	@md packages\com\bin\vs7
!endif
!if !exist(packages\com\bin\vs71)
	@md packages\com\bin\vs71
!endif
!if !exist(packages\com\samples)
	@md packages\com\samples
!endif
!if !exist(packages\com\samples\html)
	@md packages\com\samples\html
!endif
!if !exist(packages\com\samples\office)
	@md packages\com\samples\office
!endif
!if !exist(packages\com\samples\office\excel)
	@md packages\com\samples\office\excel
!endif
!if !exist(packages\com\samples\vb)
	@md packages\com\samples\vb
!endif
!if !exist(packages\com\samples\vb\MsgViewer)
	@md packages\com\samples\vb\MsgViewer
!endif
!if !exist(packages\com\samples\vb\PubSub)
	@md packages\com\samples\vb\PubSub
!endif
!if !exist(packages\com\samples\wsh)
	@md packages\com\samples\wsh
!endif

#
packagedir_dotnet: packagedir_root
!if !exist(packages\dotnet)
	@md packages\dotnet
!endif
!if !exist(packages\dotnet\bin)
	@md packages\dotnet\bin
!endif
!if !exist(packages\dotnet\docs)
	@md packages\dotnet\docs
!endif
!if !exist(packages\dotnet\docs\doxygen)
	@md packages\dotnet\docs\doxygen
!endif
!if !exist(packages\dotnet\samples)
	@md packages\dotnet\samples
!endif
!if !exist(packages\dotnet\samples\MessageTest1)
	@md packages\dotnet\samples\MessageTest1
!endif
!if !exist(packages\dotnet\samples\PubTest1)
	@md packages\dotnet\samples\PubTest1
!endif
!if !exist(packages\dotnet\samples\PubTestMT)
	@md packages\dotnet\samples\PubTestMT
!endif
!if !exist(packages\dotnet\samples\SubTest1)
	@md packages\dotnet\samples\SubTest1
!endif
!if !exist(packages\dotnet\samples\SubTest2)
	@md packages\dotnet\samples\SubTest2
!endif
!if !exist(packages\dotnet\samples\SubTestMT)
	@md packages\dotnet\samples\SubTestMT
!endif


#
packagedir_ppc2k: packagedir_root
!if !exist(packages\pocketpc2k)
	@md packages\pocketpc2k
!endif
!if !exist(packages\pocketpc2k\bin)
	@md packages\pocketpc2k\bin
!endif
!if !exist(packages\pocketpc2k\bin\ARM)
	@md packages\pocketpc2k\bin\ARM
!endif
!if !exist(packages\pocketpc2k\bin\MIPS)
	@md packages\pocketpc2k\bin\MIPS
!endif
!if !exist(packages\pocketpc2k\bin\SH3)
	@md packages\pocketpc2k\bin\SH3
!endif
!if !exist(packages\pocketpc2k\bin\X86EM)
	@md packages\pocketpc2k\bin\X86EM
!endif
!if !exist(packages\pocketpc2k\docs)
	@md packages\pocketpc2k\docs
!endif
!if !exist(packages\pocketpc2k\docs\doxygen)
	@md packages\pocketpc2k\docs\doxygen
!endif
!if !exist(packages\pocketpc2k\include)
	@md packages\pocketpc2k\include
!endif
!if !exist(packages\pocketpc2k\include\libkn)
	@md packages\pocketpc2k\include\libkn
!endif
!if !exist(packages\pocketpc2k\include\cestl)
	@md packages\pocketpc2k\include\cestl
!endif
!if !exist(packages\pocketpc2k\include\cewtl)
	@md packages\pocketpc2k\include\cewtl
!endif
!if !exist(packages\pocketpc2k\lib)
	@md packages\pocketpc2k\lib
!endif
!if !exist(packages\pocketpc2k\samples)
	@md packages\pocketpc2k\samples
!endif
!if !exist(packages\pocketpc2k\samples\Chat)
	@md packages\pocketpc2k\samples\Chat
!endif
!if !exist(packages\pocketpc2k\samples\MsgViewer)
	@md packages\pocketpc2k\samples\MsgViewer
!endif
!if !exist(packages\pocketpc2k\samples\PubSub)
	@md packages\pocketpc2k\samples\PubSub
!endif

#
packagedir_ppc2k2: packagedir_root
!if !exist(packages\pocketpc2k2)
	@md packages\pocketpc2k2
!endif
!if !exist(packages\pocketpc2k2\bin)
	@md packages\pocketpc2k2\bin
!endif
!if !exist(packages\pocketpc2k2\bin\ARM)
	@md packages\pocketpc2k2\bin\ARM
!endif
!if !exist(packages\pocketpc2k2\bin\X86EM)
	@md packages\pocketpc2k2\bin\X86EM
!endif
!if !exist(packages\pocketpc2k2\docs)
	@md packages\pocketpc2k2\docs
!endif
!if !exist(packages\pocketpc2k2\docs\doxygen)
	@md packages\pocketpc2k2\docs\doxygen
!endif
!if !exist(packages\pocketpc2k2\include)
	@md packages\pocketpc2k2\include
!endif
!if !exist(packages\pocketpc2k2\include\libkn)
	@md packages\pocketpc2k2\include\libkn
!endif
!if !exist(packages\pocketpc2k2\include\cestl)
	@md packages\pocketpc2k2\include\cestl
!endif
!if !exist(packages\pocketpc2k2\include\cewtl)
	@md packages\pocketpc2k2\include\cewtl
!endif
!if !exist(packages\pocketpc2k2\lib)
	@md packages\pocketpc2k2\lib
!endif
!if !exist(packages\pocketpc2k2\samples)
	@md packages\pocketpc2k2\samples
!endif
!if !exist(packages\pocketpc2k2\samples\Chat)
	@md packages\pocketpc2k2\samples\Chat
!endif
!if !exist(packages\pocketpc2k2\samples\PubSub)
	@md packages\pocketpc2k2\samples\PubSub
!endif


#
# Packages
#	
packages: $(COMMON_DEP) package_cpp package_com package_dotnet package_ppc2k package_ppc2k2

package_cpp: $(COMMON_DEP) 
	@copy license.txt packages\cpp
	@copy docs\html\*.* packages\cpp\docs\doxygen
	@copy include\libkn\*.* packages\cpp\include\libkn
	@copy include\cewtl\*.* packages\cpp\include\cewtl
	@copy include\cestl\*.* packages\cpp\include\cestl
	@copy lib\*.lib packages\cpp\lib
	@copy tests\cpp\PubTest packages\cpp\samples\PubTest
	@copy tests\cpp\PubTestMT packages\cpp\samples\PubTestMT
	@copy tests\cpp\QTest packages\cpp\samples\QTest
	@copy tests\cpp\SubTest packages\cpp\samples\SubTest
	@copy tests\cpp\SubTestCounted packages\cpp\samples\SubTestCounted
	@copy tests\cpp\SubTestMT packages\cpp\samples\SubTestMT
	
package_com: $(COMMON_DEP)
	@copy license.txt packages\com
	@copy docs\html\*.* packages\com\docs\doxygen
	@copy ComWin32\ReleaseMinDependency\LibKNCom.dll packages\com\bin\vs6
	@copy ComWin32\ReleaseMinDependency\LibKNCom.map packages\com\bin\vs6
	@copy ComWin32\LibKNCom.tlb packages\com\bin\vs6
	@copy ComWin32_7\ReleaseMinDependency\LibKNCom.dll packages\com\bin\vs7
	@copy ComWin32_7\ReleaseMinDependency\LibKNCom.map packages\com\bin\vs7
	@copy ComWin32_7\ReleaseMinDependency\LibKNCom.tlb packages\com\bin\vs7
	@copy ComWin32_71\ReleaseMinDependency\LibKNCom.dll packages\com\bin\vs71
	@copy ComWin32_71\ReleaseMinDependency\LibKNCom.map packages\com\bin\vs71
	@copy ComWin32_71\ReleaseMinDependency\LibKNCom.tlb packages\com\bin\vs71
	@copy tests\html\*.* packages\com\samples\html
	@copy tests\office\excel\*.* packages\com\samples\office\excel
	@copy tests\vb\MsgViewer\*.* packages\com\samples\vb\MsgViewer
	@copy tests\vb\PubSub\*.* packages\com\samples\vb\PubSub
	@copy tests\wsh\*.* packages\com\samples\wsh

package_dotnet: $(COMMON_DEP)
	@copy license.txt packages\dotnet
	@copy docs\html\*.* packages\dotnet\docs\doxygen
	@copy lib\LibKNDotNet.dll packages\dotnet\bin
	@copy lib\LibKNDotNet_71.dll packages\dotnet\bin
	@copy tests\dotnet\MessageTest1\*.* packages\dotnet\samples\MessageTest1
	@copy tests\dotnet\PubTest1\*.* packages\dotnet\samples\PubTest1
	@copy tests\dotnet\PubTestMT\*.* packages\dotnet\samples\PubTestMT
	@copy tests\dotnet\SubTest1\*.* packages\dotnet\samples\SubTest1
	@copy tests\dotnet\SubTest2\*.* packages\dotnet\samples\SubTest2
	@copy tests\dotnet\SubTestMT\*.* packages\dotnet\samples\SubTestMT

package_ppc2k: $(COMMON_DEP)
	@copy license.txt packages\pocketpc2k
	@copy docs\html\*.* packages\pocketpc2k\docs\doxygen
	@copy include\libkn\*.* packages\pocketpc2k\include\libkn
	@copy include\cewtl\*.* packages\pocketpc2k\include\cewtl
	@copy include\cestl\*.* packages\pocketpc2k\include\cestl
	@copy lib\LibKN10Ce*.lib packages\pocketpc2k\lib
	@copy ComCe.2k\ARMRelMinDependency\LibKNCom.dll packages\pocketpc2k\bin\ARM
	@copy ComCe.2k\LibKNCom.tlb packages\pocketpc2k\bin\ARM
	@copy ComCe.2k\MIPSRelMinDependency\LibKNCom.dll packages\pocketpc2k\bin\MIPS
	@copy ComCe.2k\LibKNCom.tlb packages\pocketpc2k\bin\MIPS
	@copy ComCe.2k\SH3RelMinDependency\LibKNCom.dll packages\pocketpc2k\bin\SH3
	@copy ComCe.2k\LibKNCom.tlb packages\pocketpc2k\bin\SH3
	@copy ComCe.2k\X86RelMinDependency\LibKNCom.dll packages\pocketpc2k\bin\X86EM
	@copy ComCe.2k\LibKNCom.tlb packages\pocketpc2k\bin\X86EM
	@copy Tests\PocketPC.2k\Chat\*.* packages\pocketpc2k\Samples\Chat
	@copy Tests\PocketPC.2k\MsgViewer\*.* packages\pocketpc2k\Samples\MsgViewer
	@copy Tests\PocketPC.2k\PubSub\*.* packages\pocketpc2k\Samples\PubSub
	
package_ppc2k2: $(COMMON_DEP)
	@copy license.txt packages\pocketpc2k2
	@copy docs\html\*.* packages\pocketpc2k2\docs\doxygen
	@copy include\libkn\*.* packages\pocketpc2k2\include\libkn
	@copy include\cewtl\*.* packages\pocketpc2k2\include\cewtl
	@copy include\cestl\*.* packages\pocketpc2k2\include\cestl
	@copy lib\LibKN10CeArm*.lib packages\pocketpc2k2\lib
	@copy lib\LibKN10CeEmul*.lib packages\pocketpc2k2\lib
	@copy ComCe\ARMRelMinDependency\LibKNCom.dll packages\pocketpc2k2\bin\ARM
	@copy ComCe\LibKNCom.tlb packages\pocketpc2k2\bin\ARM
	@copy ComCe\X86RelMinDependency\LibKNCom.dll packages\pocketpc2k2\bin\X86EM
	@copy ComCe\LibKNCom.tlb packages\pocketpc2k2\bin\X86EM
	@copy Tests\PocketPC\Chat\*.* packages\pocketpc2k2\Samples\Chat
	@copy Tests\PocketPC\PubSub\*.* packages\pocketpc2k2\Samples\PubSub
	


