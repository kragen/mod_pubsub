/*
 * $Log: readme.txt,v $
 * Revision 1.5  2004/04/19 05:39:09  bsittler
 * Propagate updated license text and associated copyright date to all
 * affected files.
 *
 * Also replaced a few KNOWNOW_LICENSE_BEGINs with the correct
 * KNOWNOW_LICENSE_START.
 *
 * Revision 1.4  2003/08/15 23:46:14  ifindkarma
 * Added changes from August 2003.
 *
 * 
 * 6     8/04/03 3:13a Thui
 * Added VC7.1
 * 
 * 5     3/20/03 9:50p Thui
 * Updated for Unicode and PocketPC 2k builds
 * 
 * 4     3/18/03 12:17a Thui
 * 
 * 3     3/05/03 4:27p Thui
 * Updated for current directory structure.
 * 
 * 2     3/03/03 8:56p Thui
 */

Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

@KNOWNOW_LICENSE_START@

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither the name of the KnowNow, Inc., nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@KNOWNOW_LICENSE_END@




LIBKN

A Win32 C++/COM/.NET/CE library to talk to a notification server.


DIRECTORY STRUCTURE

Name         Description
----         -----------
include      The main header files (implemented in cpp)
  cestl      an STL for Embedded Visual C++ 3.0 (CE build only)
cpp          The main source
lib          Destination directory for built libraries and DLLs
VC6          Single project to build everything associated with Visual Studio 6
VC7          Single project to build everything associated with Visual Studio.NET 2002
VC71         Single project to build everything associated with Visual Studio.NET 2003
EVC3         Single project to build everything associated with PocketPC 2002 build
EVC3.2k      Single project to build everything associated with PocketPC 2000 build
ComCe        COM wrapper for PocketPC 2002 builds
ComCe.2k     COM wrapper for PocketPC 2000 builds
ComWin32     COM wrapper in Win32 (98, NT/2000) (built with VS6)
ComWin32_7   COM wrapper in Win32 (98, NT/2000) (built with VS7)
ComWin32_71  COM wrapper in Win32 (98, NT/2000) (built with VS71)
cpp7         C++ build with Visual Studio.NET 2002 (VS7)
cpp71        C++ build with Visual Studio.NET 2003 (VS71)
cpp_ce       C++ build with Embedded Visual C++ 3.0 using PocketPC SDK 2002
cpp_ce.2k    C++ build with Embedded Visual C++ 3.0 using PocketPC SDK 2000
DotNet       .NET wrapper build with VS.NET 2002
DotNet_71    .NET wrapper build with VS.NET 2003
docs         Generated docs based on info from the "include" and "cpp" directories
Tests        Test programs and small samples
Apps         Applications that uses LibKN


Building LIBKN

External Dependencies
	Visual Studio 6 SP5 (both VC++ and VB6 are needed)
	Visual Studio.NET 2002
	Visual Studio.NET 2003
	Embedded Visual Tools 3.0 with Pocket PC 2000 SDK and Pocket PC 2002 SDK
	Wise Professional 9.02 (http://www.wise.com)
	Doxygen and GraphViz (http://www.doxygen.org)
	cppunit (http://cppunit.sourceforge.net)
	stampver (http://www.elphin.com/products/stampver.html)
	
It is generally recommended to use the supplied makefile in the root directory.
Be sure to update the paths in paths.mak to reflect your build system. If you do
use the supplied makefiles, note that you will need all of the tools above for
the default build. If you don't have certain tools, you will need to update both
makefile and/or paths.mak to reflect that. If you modified the makefile or paths.mak,
you're on your own.

This will build most everything and in the right order. However, if you are
used to using the IDE, use either the project/solution files in the VC6, VC7, VC71
EVC3, or EVC3.2k directories.

In general, the main projects are either "debug" or "release". When building
the COM/ActiveX versions, build only the debug or release MinDependency
project. Testing has only been done with MinDependency and not MinSize. They
should work, but it hasn't been tested.


Building the documentation

The documentation is written into the source code itself. Use the 
supplied makefile to generate the documentation. The generated 
documentation is located in the docs directory. To view the
docs, use checkdocs.bat.

