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

#include "stdafx.h"
#include <LibKN\Tunnel.h>

Tunnel::Tunnel(HINTERNET tunnelHandle, const Message& status)
{
	m_Available = 0;
	m_Closed = false;
	m_TunnelHandle = tunnelHandle;
	m_Status = status;
}

Tunnel::~Tunnel()
{
}

/** @param buf_ptr Pointer to the mb_buf_ptr into which a pointer to the data
 *                 will be added. The buffer is vaild until this function is
 *                 called again, or the Tunnel object is deleted.
 *
 *  @return The size of the data returned in the buffer, or zero (0) if the
 *          connection is closed intentionally, minus one (-1) if the 
 *          connection closes unexpectedly.
 *
 * This function will block until there is data avalable or the connection is 
 * closed. The buffer is vaild until this function is called again, or the 
 * Tunnel object is deleted.
 *
 */
int Tunnel::ReadData(mb_buf_ptr* buf_ptr)
{
	if (IsClosed())
		return 0;

	{
		Lock autoLock(this);
		if (!m_TunnelHandle)
		{
			StatusUnexpectedClose();
			return -1;
		}
	}

	DWORD dwSize = 0;			   // size of the data available
	DWORD dwDownloaded = 0;		   // size of the downloaded data

	if (m_Available == 0)
	{
		Lock autoLock(this);

		// This will block until there is data avalable or the connection is closed
		if (!InternetQueryDataAvailable(m_TunnelHandle, &dwSize, 0, 0))
		{
			//check for close
			if (IsClosed())
				return 0;
			else
			{
				StatusUnexpectedClose();
				return -1;
			}
		}

		{
			m_Available = dwSize;
		}

	}

	//setup for reading
	{
		Lock autoLock(this);

		unsigned char* buffer = buf_ptr->m_Ptr;
		DWORD to_read = (m_Available < buf_ptr->m_Max) ? m_Available : buf_ptr->m_Max;

		// Read the data from the HINTERNET handle.
		if (!InternetReadFile(m_TunnelHandle, (void*)buffer, to_read, &dwDownloaded))
		{
			buf_ptr->m_Len = 0;
			StatusUnexpectedClose();
			return -1;
		}
		else
		{
			//put the pointer and length into a mb_buf_ptr
			buf_ptr->m_Len = dwDownloaded;

			//lower the avalable amount by what we read
			m_Available -= dwDownloaded;
		}
	}

	// Check the size of the remaining data.  
	// If it is zero, the connection is finished.
	// This should not happen to a journal, unles we close it
	if (dwDownloaded == 0)
	{
		//check for close
		if (IsClosed())
			return 0;
		else
		{
			StatusUnexpectedClose();
			return -1;
		}
	}

	return dwDownloaded;
}

void Tunnel::Close()
{
	Lock autoLock(this);

	m_Closed = true;

	StatusClosed();

	if (m_TunnelHandle)
	{
		InternetCloseHandle(m_TunnelHandle);
		m_TunnelHandle = 0;
	}
}

bool Tunnel::IsClosed()
{
	return m_Closed;
}

bool Tunnel::Success()
{
	Lock autoLock(this);
	return m_TunnelHandle != 0;
}

const Message& Tunnel::GetStatus()
{
	Lock autoLock(this);
	return m_Status;
}

void Tunnel::StatusClosed()
{
	Lock autoLock(this);

	m_Status.Empty();

	char buffer[30];
	sprintf(buffer, "%d", INTERNET_STATE_DISCONNECTED_BY_USER);

	m_Status.Set("state", buffer);
	m_Status.Set("kn_payload", "Tunnel connection closed normally.");
}

void Tunnel::StatusUnexpectedClose()
{
	Lock autoLock(this);

	if (m_TunnelHandle)
	{
		InternetCloseHandle(m_TunnelHandle);
		m_TunnelHandle = 0;
	}

	m_Status.Empty();

	char buffer[30];
	sprintf(buffer, "%d", INTERNET_STATE_DISCONNECTED);

	m_Status.Set("state", buffer);
	m_Status.Set("kn_payload", "Tunnel connection closed unexpectedly.");
}


