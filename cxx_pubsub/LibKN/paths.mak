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
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 
# 3. Neither the name of the KnowNow, Inc., nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @KNOWNOW_LICENSE_END@
# 

#
# !!!!NOTE!!!!
# You must update the paths here to reflect your current build system.
# 

#
# Paths to compilers/IDEs
#
VC6="C:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin\MSDEV.EXE"
#VC6="C:\vse6\Common\MSDev98\Bin\MSDEV.EXE"
VB6="C:\Program Files\Microsoft Visual Studio\VB98\VB6.EXE"
VC7="C:\Program Files\Microsoft Visual Studio .NET\Common7\IDE\devenv.exe"
VC71="C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.exe"
EVC3="C:\Program Files\Microsoft eMbedded Tools\Common\EVC\Bin\EVC.EXE"
EVC3_2K=$(EVC3)

!if defined(EVC3) || defined(EVC3_2K)
SUPPORT_X86=1
SUPPORT_ARM=1
SUPPORT_SH3=1
SUPPORT_MIPS=1
!else
SUPPORT_X86=
SUPPORT_ARM=
SUPPORT_SH3=
SUPPORT_MIPS=
!endif


WISE="C:\Program Files\Wise Installation System\Wise32.exe"
WISE9_DIR="C:\Program Files\Wise Installation System"

#
# Stampver utility to stamp the version number of binaries with file resources
#
STAMPVER="c:\bin\stampver.exe"

#
# Doc build depends on doxygen
#
DOXYGEN=doxygen.exe


