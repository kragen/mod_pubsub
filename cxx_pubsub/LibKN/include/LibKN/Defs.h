/*
Copyright (c) 2000-2003 KnowNow, Inc.  All rights reserved.

@KNOWNOW_LICENSE_START@

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
be used to endorse or promote any product without prior written
permission from KnowNow, Inc.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@KNOWNOW_LICENSE_END@
*/

#if !defined(LIBKN_DEFS_H)
#define LIBKN_DEFS_H
   
#   if !defined(LIBKN_INTERNAL_BUILD)
#		define LIBKN_PREFIX 	"LIBKN"
#		if !defined(_WIN32_WCE)
#			if (_MSC_VER >= 1300)
#				define LIBKN_VERSION	"10_7"
#			else
#				define LIBKN_VERSION	"10"
#			endif 
#		else
#	       define LIBKN_VERSION	"10"
#		endif
#		if defined(_DEBUG) || defined(DEBUG)
#		    define LIBKN_DEBUG	"D"
#		else
#		    define LIBKN_DEBUG  "ND"
#		endif
   
#		if defined(_WIN32_WCE)
#			if defined(_WIN32_WCE_EMULATION)
#				define LIBKN_PLATFORM		"CeEmul"
#			elif defined(CEF)
#				define LIBKN_PLATFORM		"CeCef"
#			elif defined(_ARM_)
#				define LIBKN_PLATFORM		"CeArm"
#			elif defined(_MIPS_)
#				define LIBKN_PLATFORM		"CeMips"
#			elif defined(_SH3_)
#				define LIBKN_PLATFORM		"CeSh3"
#			else
#				define LIBKN_PLATFORM		"Ce"
#			endif
//#		else if defined(_MANAGED)
#		else   // regular Windows
#			if !defined(_MT) || !defined(_DLL)
#				error "_MT and _DLL must be used on X86 platforms"
#			endif
#			if defined(_UNICODE)
#				define LIBKN_PLATFORM		"X86U"
#			else
#				define LIBKN_PLATFORM		"X86"
#			endif
#		endif
   
#		define LIBKN_LIB	LIBKN_PREFIX##""##LIBKN_VERSION##""##LIBKN_PLATFORM##""##LIBKN_DEBUG
#		pragma comment(lib, LIBKN_LIB)
#	endif

#include <string>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <algorithm>
#include <utility>

#if !defined(_WIN32_WCE)
#	define MYSTD std
#else
#	define MYSTD
#endif

using MYSTD::string;
using MYSTD::wstring;
using MYSTD::map;
using MYSTD::set;
using MYSTD::pair;
using MYSTD::list;
using MYSTD::queue;
using MYSTD::find;

typedef MYSTD::basic_string<TCHAR> tstring;

#endif

