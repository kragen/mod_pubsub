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

#if !defined(LIBKN_RANDRHANDLER_H)
#define LIBKN_RANDRHANDLER_H

#include <LibKN\Defs.h>
#include <LibKN\Message.h>

typedef set<wstring> EventSetImpl;

class EventSet
{
public:
	EventSet();
	~EventSet();

	/**
	 * Sets the timeout for the handler.
	 * \param timeout Timeout in seconds.
	 */
	void SetTimeout(int timeout);

	/**
	 * Insert the new string into the event.
	 */
	bool Insert(const wstring& newString);

private:
	int m_Timeout;

	EventSetImpl m_Set1;
	EventSetImpl m_Set2;

	EventSetImpl* m_Current;
	EventSetImpl* m_Older;

	DWORD m_NextCount;
};

class RandRHandler : public CCriticalSection
{
public:
	typedef LockImpl<RandRHandler> Lock;

	RandRHandler();
	virtual ~RandRHandler();

	/** 
	 * Get the scheme from the journal connection status.
	 */
	void ConnectStatus(const Message& status);

	/**
	 * Check the event to record a checkpoint or to see if it is a duplicate.
	 */
	bool EventCheck(const Message& event);

	/**
	 * Set any necessary parameters for a reconnect.
	 */
	void SetReconnectParams(Message& msg);

private:
	enum ReconnectSchemes 
	{
		RouteCheckpoint,
		EventHash,
		None 
	} m_Scheme;

	wstring m_ReconnectTimeout;

	// checkpointing
	wstring m_Checkpoint;

	// event hashing
	EventSet m_Set;

private:
	RandRHandler& operator=(const RandRHandler& rhs);
};

#endif
