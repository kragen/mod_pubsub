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

#if !defined(LIBKN_TUNNEL_H)
#define LIBKN_TUNNEL_H

#include <LibKN\Defs.h>
#include <LibKN\Message.h>
#include <LibKN\CS.h>

/**
 * A structure for parsing the events from the transport layer.
 */
struct mb_buf_ptr
{
public:
	/** Initializes everything to empty. */
	mb_buf_ptr()
	{
		m_Ptr = 0;
		m_Len = 0;
		m_Max = 0;
	}

	/** The beginning of the pointer. */
	unsigned char* m_Ptr;

	/** The length of the buffer. */
	unsigned int m_Len;

	/** The maximum size of the buffer. */
	unsigned int m_Max;
};


/**
 * This class maintains the tunnel connection between the client and
 * the router.
 */
class Tunnel : public CCriticalSection
{
public:
	typedef LockImpl<Tunnel> Lock;

	/**
	 * The constructor requires a handle to the connection.
	 */
	Tunnel(HINTERNET tunnelHandle, const Message& status);
	~Tunnel();

	/**
	 * This function is called by the tunnel thread to read data.
	 * \return The number of bytes read from the tunnel. -1 if there was an error.
	 */
	int ReadData(mb_buf_ptr* buf_ptr);

	/**
	 * Close the tunnel.
	 */
	void Close();

	/**
	 * \return True if the tunnel is closed. False otherwise.
	 */
	bool IsClosed();

	/**
	 * \return True if the tunnel was successfully created. False otherwise.
	 */
	bool Success();

	/**
	 * \return The status message from the tunnel.
	 */
	const Message& GetStatus();

private:
	void StatusClosed();
	void StatusUnexpectedClose();

	unsigned int m_Available;
	bool m_Closed;
	HINTERNET m_TunnelHandle;
	Message m_Status;

private:
	Tunnel& operator=(const Tunnel& rhs);
};

#endif
