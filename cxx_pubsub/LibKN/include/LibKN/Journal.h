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

#if !defined(LIBKN_JOURNAL_H)
#define LIBKN_JOURNAL_H

#include <LibKN\Defs.h>
#include <LibKN\Tunnel.h>
#include <LibKN\CS.h>

class Transport;

/**
 * The Journal class provides a high-level object of dealing with a journal connection to the server.
 * This class uses a tunnel to provide the journal capability.
 * The Connector class automatically creates a journal as needed.
 */
class Journal : public CCriticalSection
{
friend class Connector;

public:
	typedef LockImpl<Journal> Lock;

	/**
	 * This function returns the connection status.
	 * \return True if connected. False otherwise.
	 */
	bool IsConnected();

	/**
	 * This function ensures the journal is connected (i.e. the tunnel is created and connected).
	 * \return True if connected. False otherwise.
	 */
	bool EnsureConnected();

	/**
	 * This function starts the tunnel.
	 * \return True if connected. False otherwise.
	 */
	bool StartTunnel();

	/**
	 * This function restarts the tunnel.
	 * \return True if connected. False otherwise.
	 */
	bool RestartTunnel();
	
	/**
	 * This function stops and closes the tunnel.
	 * \return True if successfully disconnected. False otherwise.
	 */
	bool CloseTunnel();

	/**
	 * Return the path of the journal.
	 */
	const wstring& GetJournalPath() const;

private:
	static void SafeConnectionStatus(Tunnel* t, Connector* c);
	void ExpectClosing();

	Journal(Connector* conn);
	~Journal();

	Journal& operator=(const Journal& rhs);

	bool StartTunnelImp();
	void SetJournalPathImpl();

	bool m_ExpectClosing;
	Connector* m_Connector;
	Transport* m_Transport;
	Tunnel* m_Tunnel;
	Message m_TunnelParams;
	
	wstring m_JournalBase;
	wstring m_JournalPath;

	// handle to the reader thread
	HANDLE m_hReadThread;

	// Event that lets us know the reader thread is running
	// HANDLE m_ThreadDeathEvent;

	// Reader thread control function 
	static DWORD WINAPI ReadSimpleTunnelThread(void* args);
};

#endif
