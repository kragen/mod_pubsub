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

#include "stdafx.h"
#include <LibKN\CS.h>
#include <LibKN\Logger.h>

CCriticalSection::CCriticalSection()
{
	ZeroMemory(&m_CS, sizeof m_CS);
	InitializeCriticalSection(&m_CS);
}

CCriticalSection::~CCriticalSection()
{
	DeleteCriticalSection(&m_CS);
	ZeroMemory(&m_CS, sizeof m_CS);
}

void CCriticalSection::Enter() const
{
	EnterCriticalSection(const_cast<CRITICAL_SECTION*>(&m_CS));
}

void CCriticalSection::Leave() const
{
	LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&m_CS));
}


CMutex::CMutex(const TCHAR* name, BOOL initialOwner) : 
	m_Mutex(0)
{
#if !defined(_WIN32_WCE)
	m_Mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, name);
#endif

	if (m_Mutex == 0)
	{
		// Failed to open, let's try create
		//
		m_Mutex = CreateMutex(0, initialOwner, name);

		if (m_Mutex == 0 || m_Mutex == INVALID_HANDLE_VALUE)
		{
			m_Mutex = 0;
		}
	}
}

CMutex::~CMutex()
{
	if (m_Mutex)
	{
		CloseHandle(m_Mutex);
		m_Mutex = 0;
	}
}

void CMutex::Enter() const
{
	if (m_Mutex)
	{
		WaitForSingleObject(m_Mutex, INFINITE);
	}
}

void CMutex::Leave() const
{
	if (m_Mutex)
		ReleaseMutex(m_Mutex);
}

