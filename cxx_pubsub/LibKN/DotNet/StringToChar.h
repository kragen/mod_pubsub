/*
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

*/

#if !defined(STRINGTOCHAR_H)
#define STRINGTOCHAR_H

using namespace System::Runtime::InteropServices;
using namespace System::Reflection;
using System::String;
using System::IntPtr;

__gc class StringToChar
{
public:
	StringToChar(String* str) : 
		m_Ptr(0)
	{
//		if (str->get_Length())
		{
			IntPtr iPtr = Marshal::StringToCoTaskMemAnsi(str);
			m_Ptr = iPtr;
		}
	}

	~StringToChar()
	{
		Marshal::FreeCoTaskMem(m_Ptr);
		m_Ptr = 0;
	}

	char* GetChar() 
	{
		if (m_Ptr == 0)
			return 0;
		return (char*)m_Ptr.ToPointer();
	}

private:
	IntPtr m_Ptr;
};

__gc class StringToWChar
{
public:
	StringToWChar(String* str) : 
		m_Ptr(0)
	{
//		if (str->get_Length())
		{
			IntPtr iPtr = Marshal::StringToCoTaskMemUni(str);
			m_Ptr = iPtr;
		}
	}

	~StringToWChar()
	{
		Marshal::FreeCoTaskMem(m_Ptr);
		m_Ptr = 0;
	}

	wchar_t* GetChar() 
	{
		if (m_Ptr == 0)
			return 0;
		return (wchar_t*)m_Ptr.ToPointer();
	}

private:
	IntPtr m_Ptr;
};



#endif
