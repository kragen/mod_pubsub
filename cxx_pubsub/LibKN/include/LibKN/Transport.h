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

#if !defined(LIBKN_TRANSPORT_H)
#define LIBKN_TRANSPORT_H

#include <LibKN\Defs.h>
#include <LibKN\ITransport.h>
#include <LibKN\Message.h>
#include <LibKN\CS.h>
#include <LibKN\IRequestStatusHandler.h>

class Tunnel;
class Connector;

/**
 * This class provides the interface to the OS network layer.
 */
class Transport : public ITransport, public CCriticalSection
{
friend class Connector;
friend class OfflineQueue;

public:
	typedef LockImpl<Transport> Lock;

	/**
	 * \return True if successfully connected.
	 */
	bool IsConnected();

	/**
	 * Opens the connection to the server indicated by the parameters.
	 * \return True if successfully connected.
	 */
	bool Open(const ITransport::Parameters& p);
	
	/**
	 * \return The parameters this class is using.
	 */
	const ITransport::Parameters& GetParameters() const;
	
	/**
	 * Closes the current connection.
	 * \return True if successfully disconnected.
	 */
	bool Close();
	
	/**
	 * Ensures the connection is fully connected.
	 * \return True if the connection was successfully connected. False otherwise.
	 */
	bool EnsureConnected();

	/**
	 * Sends the message to the server.
	 * \return True if successfully sent.
	 */
	bool Send(const Message& msg, IRequestStatusHandler* sh);
	
	/**
	 * Opens a tunnel connection to the server.
	 */
	Tunnel* OpenTunnel(const Message& msg);

	/**
	 * \return The URL for the connection.
	 */
	const tstring& GetUrl() const;

private:
	bool Send(const string& data, IRequestStatusHandler* sh);
	HINTERNET Send(const string& data, Message& status_map);
	bool SendImpl(const string& data, const Message& msg, IRequestStatusHandler* sh);

	bool Connect();
	bool Disconnect();

	bool HandleAuth(HINTERNET http_request, bool& triedAuth);
	bool HandleProxyAuth(HINTERNET http_request, bool& triedProxyStuff);
	bool HandleSecurity(HINTERNET http_request);
	void HandleError(DWORD error, Message& status_map);

	DWORD GetINetFlags();
	void SetupHttpRequest(HINTERNET http_request);

	bool CheckStatus(HINTERNET http_request, Message& status_map);

	bool GetTunnelStatus(HINTERNET hTunnel, Message& status_map);

private:
	Transport(Connector* conn);
	~Transport();

	Connector* m_Connector;
	HINTERNET m_Connect;
	HINTERNET m_Session;
	ITransport::Parameters m_Parameters;

	// Members to save values from InternetCrackUrl()
	//
	tstring m_ServerPath;
	tstring m_Server;
	bool m_Https;

private:
	Transport& operator=(const Transport& rhs);
};

#endif
