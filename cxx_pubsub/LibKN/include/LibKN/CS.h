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

#if !defined(LIBKN_CS_H)
#define LIBKN_CS_H

#include <LibKN\Defs.h>
#include <LibKN\Lock.h>

/**
 * This class is a wrapper for the Win32 CRITICAL_SECTION object.
 * Objects that want to be thread-safe should derive from this class.
 * To lock the resource, they can call Enter().
 * To release the resource, call Leave().
 */
class CCriticalSection : public ILockable
{
public:
	typedef LockImpl<CCriticalSection> Lock;

	/**
	 * Creates and initializes the CRITICAL_SECTION object.
	 */
	CCriticalSection();

	/**
	 * Destroys the CRITICAL_SECTION object.
	 */
	~CCriticalSection();

	/**
	 * Attempt to enter the critical section. 
	 * If another thread is using the object, this function blocks until no other thread has the resource.
	 */
	void Enter() const;

	/**
	 * Leave the critical section. Releases the shared resource.
	 */
	void Leave() const;

private:
	/** The real CRITICAL_SECTION object. */
	CRITICAL_SECTION m_CS;
};

/**
 * This class is a wrapper for the Win32 MUTEX object.
 */
class CMutex : public ILockable
{
public:
	CMutex(const TCHAR* name, BOOL initialOwner = FALSE);
	~CMutex();

	void Enter() const;
	void Leave() const;

private:
	HANDLE m_Mutex;
};

#endif
