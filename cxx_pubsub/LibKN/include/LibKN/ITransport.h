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

#if !defined(LIBKN_ITRANSPORT_H)
#define LIBKN_ITRANSPORT_H

#include <LibKN\Defs.h>

/**
 * An abstract class that defines the methods a transport provides.
 */
class ITransport
{
public:
	/**
	 * This structure contains transport specific information.
	 * The information is used to connect to the server.
	 */
	struct Parameters
	{
	public:
		Parameters();

		/** The URL to the server. */
		tstring m_ServerUrl;

		/** The username to use for the connection. This field is used only when the server has authentication turned on. */
		tstring m_Username;

		/** The password to use for the connection. This field is used only when the server has authentication turned on. */
		tstring m_Password;

		/** 
		 * Set this field to true to indicate an HTTP proxy is required to connect to a server. 
		 * Otherwise, set it to false to use direct connection.
		 * If this field is true, then the other proxy fields should also be set.
		 * The default for this value is false for all platforms.
		 * On Windows, the default value is determined by the Internet connection settings.
		 */
		bool m_UseProxy;

		/** Set this field to the proxy server and port. An example is <em>myproxyserver:myproxyport</em>. */
		tstring m_ProxyServer;

		/** The username to use for the proxy connection. This field is used only when the proxy server requires authentication. */
		tstring m_ProxyUsername;
		
		/** The password to use for the proxy connection. This field is used only when the proxy server requires authentication. */
		tstring m_ProxyPassword;

		/** The exception list for bypassing the proxy connection. The list is a semicolon-separated list of addresses. */
		tstring m_ProxyExceptionList;

		/** When this field is set, it will be passed as part of every HTTP request sent to the server. */
		tstring m_CustomHeader;

		/** Set this field to True to indicate the transport should show any UI. This is false by default. */
		bool m_ShowUI;

		/** 
		 * Set the Hwnd to use as parent when bringing up the dialogs from WinINet. The default value is 0.
		 * If this value is 0, then WinInet will <b>NOT</b> display any dialogs.
		 */
		HWND m_ParentHwnd;
	};

	virtual ~ITransport() {}


	/**
	 * This function indicates the connection status of the transport.
	 * \return True if connected. False otherwise.
	 */
	virtual bool IsConnected() = 0;

	/**
	 * This method connects to the server based on the connection parameters passed.
	 * \return True if successfully connected. False otherwise.
	 */
	virtual bool Open(const ITransport::Parameters& p) = 0;

	/**
	 * This method returns the connection parameters used by the transport.
	 */
	virtual const Parameters& GetParameters() const = 0;

	/**
	 * This method closes the server connection.
	 * \return True if successfully disconnected. False otherwise.
	 */
	virtual bool Close() = 0;

	/**
	 * This method ensures the server is connected.
	 * \return True if successfully connected. False otherwise.
	 */
	virtual bool EnsureConnected() = 0;
};

#endif
