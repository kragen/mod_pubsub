/*
 * $Log: readme.txt,v $
 * Revision 1.3  2003/03/08 04:38:08  ifindkarma
 * Bug fixes and reorganization, plus some additions.  Note: package needs testing.
 *
 * 
 * 3     3/05/03 4:27p Thui
 * Updated for current directory structure.
 * 
 * 2     3/03/03 8:56p Thui
 */

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



LIBKN

A Win32 C++/COM/.NET/CE library to talk to a notification server.


DIRECTORY STRUCTURE

Name         Description
----         -----------
include      The main header files (implemented in cpp)
cpp          The main source
lib          Destination for built libraries and DLLs
VC6          Single project to build everything associated with Visual Studio 6
VC7          Single project to build everything associated with Visual Studio.NET 2002
EVC3         Single project to build everything associated with PocketPC build
docs         Generated docs based on info from the "include" and "cpp" directories
Tests        Test programs and small samples
Apps         Applications that uses 
cestl        an STL for Embedded Visual C++ 3.0 (CE build only)
ComPPC       COM wrapper for PocketPC builds
ComWin32     COM wrapper in Win32 (98, NT/2000) (built with VS6)
ComWin32_7   COM wrapper in Win32 (98, NT/2000) (built with VS7)
cpp7         C++ build with Visual Studio.NET 2002 (VS7)
cpp_ce       C++ build with Embedded Visual C++ 3.0
DotNet       .NET wrapper build with VS.NET 2002


Building LIBKN

External Dependencies
	Doxygen and GraphViz (http://www.doxygen.org)
	cppunit (http://cppunit.sourceforge.net)
	Visual Studio 6 SP5
	Visual Studio.NET 2002
	Embedded Visual Tools 3.0 with Pocket PC 2002 SDK

It is generally recommend to use the projects in either the VC6, VC7 or 
EVC3 directories as these projects will build most everything in the right order.

Alternatively, you can use the supplied makefile. Be sure to change the paths
at the beginning of the file to reflect your current system.

Building the documentation

The documentation is written into the source code itself. Use the 
supplied makefile to generate the documentation. The generated 
documentation is located in the docs directory. To view the
docs, use checkdocs.bat.

