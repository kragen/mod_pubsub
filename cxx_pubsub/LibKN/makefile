#
# Paths to compilers/IDEs
#
VC6="C:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin\MSDEV.EXE"
#VC6="C:\vse6\Common\MSDev98\Bin\MSDEV.EXE"
VC7="C:\Program Files\Microsoft Visual Studio .NET\Common7\IDE\devenv.exe"
EVC3="C:\Program Files\Microsoft eMbedded Tools\Common\EVC\Bin\EVC.EXE"


#
# Doc build depends on doxygen
#
DOXYGEN=doxygen.exe


#
# Common dependencies - Every build depends on these
#
COMMON_DEP=	makefile \
		buildlogdir

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
# EVC3 options
#
EVC3_DSW=EVC3\EVC3.VCW
EVC3_LOGDIR=.\BuildLogs\EVC3
EVC3_DEP=LibKNCpp_evc3 LibKNCom_evc3


#
# By default, build everything
#
all: vc6 vc7 evc3 docs

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
!if !exist(BuildLogs\EVC3)
	@md BuildLogs\EVC3
!endif

#
# The various environment builds
#
vc6: $(COMMON_DEP) $(VC6_DEP)

vc7: $(COMMON_DEP) $(VC7_DEP)

evc3: $(COMMON_DEP) $(EVC3_DEP)
	
#
# VC6 Build 
#	
LibKNCpp_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "LibKNCpp - Win32 Release" /OUT $(VC6_LOGDIR)\LibKnCppRelease.log
	$(VC6) $(VC6_DSW) /MAKE "LibKNCpp - Win32 Debug" /OUT $(VC6_LOGDIR)\LibKnCppDebug.log

LibKNCom_vc6: $(COMMON_DEP)
	$(VC6) $(VC6_DSW) /MAKE "LibKNCom - Win32 Release MinDependency" /OUT $(VC6_LOGDIR)\LibKnComReleaseMinDep.log
	$(VC6) $(VC6_DSW) /MAKE "LibKNCom - Win32 Debug" /OUT $(VC6_LOGDIR)\LibKnComDebug.log

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

LibKNCom_vc7: $(COMMON_DEP)
	$(VC7) /project LibKNCom $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNComRelease.log /build "Release MinDependency"
	$(VC7) /project LibKNCom $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNComDebug.log /build Debug

LibKNDotNet_vc7: $(COMMON_DEP)
	$(VC7) /project LibKNDotNet $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNDotNetRelease.log /build Release
	$(VC7) /project LibKNDotNet $(VC7_DSW) /OUT $(VC7_LOGDIR)\LibKNDotNetDebug.log /build Debug

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
# EVC3 Build
#
LibKNCpp_evc3: $(COMMON_DEP)
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCpp - Win32 (WCE x86) Release" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNCppX86Release.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCpp - Win32 (WCE ARM) Release" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNCppARMRelease.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCpp - Win32 (WCE x86) Debug" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNCppX86Debug.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCpp - Win32 (WCE ARM) Debug" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNCppARMDebug.log

LibKNCom_evc3: $(COMMON_DEP)
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCom - Win32 (WCE x86) Release MinDependency" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNComX86Release.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCom - Win32 (WCE ARM) Release MinDependency" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNComARMRelease.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCom - Win32 (WCE x86) Debug" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNComX86Debug.log
	$(EVC3) $(EVC3_DSW) /MAKE "LibKNCom - Win32 (WCE ARM) Debug" /ceconfig="Pocket PC 2002" /OUT $(EVC3_LOGDIR)\LibKNComARMDebug.log


#
# The docs
#
docs: $(COMMON_DEP)
	$(DOXYGEN) doxygen.cfg
	
	
