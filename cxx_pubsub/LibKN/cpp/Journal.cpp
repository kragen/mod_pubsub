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
#include <LibKN\Journal.h>
#include <LibKN\Connector.h>
#include <LibKN\SimpleParser.h>
#include <LibKN\Message.h>
#include <LibKn\StrUtil.h>

extern void Seed();

Journal::Journal(Connector* conn)
{
	Seed();
	m_Connector = conn;

	if (m_Connector)
		m_Transport = &m_Connector->GetTransport();
	else
		m_Transport = 0;

	m_Tunnel = 0;
	m_JournalBase = L"/who/wms/";
	m_hReadThread = 0;
	m_ThreadDeathEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}

Journal::~Journal()
{
	if (m_Tunnel)
	{
//		delete m_Tunnel;
		m_Tunnel = 0;
	}

	CloseHandle(m_ThreadDeathEvent);
}

bool Journal::EnsureConnected()
{
	if (IsConnected())
		return true;
	else
		return RestartTunnel();
}

const wstring& Journal::GetJournalPath() const
{
	Lock autoLock(this);

	return m_JournalPath;
}

bool Journal::IsConnected()
{
	if (m_Tunnel == 0)
		return false;

	return !m_Tunnel->IsClosed();
}

void Journal::SetJournalPathImpl()
{
	Lock autoLock(this);

	if (m_JournalPath.length() == 0)
	{
		wstring url;

		if (m_Connector)
		{
			const ITransport::Parameters& p = m_Connector->GetParameters();
			url = ConvertToWide(p.m_ServerUrl);
		}

		// start with the path in jounalbase
		if (m_JournalBase.length() > 0)
		{
			m_JournalPath = url + m_JournalBase;

			wstring target = L"/kn_journal";

			// if it ends with "/kn_journal" then use it as is
			if (m_JournalPath.length() > target.length() && 
				m_JournalPath.compare(m_JournalPath.length() - target.length(), target.length(), target) == 0)
			{
				return;
			}
		}
		else
		{
			m_JournalPath = L"/";  // make sure that there is a leading slash
		}

		//put in a psudo-random number to attempt to avoid clashes
		DWORD ticks = GetTickCount();
		wchar_t temp[11];
		m_JournalPath += _ultow(ticks, &temp[0], 10);

		//add kn_journal
		m_JournalPath += L"/kn_journal";
	}

	return;
}

bool Journal::StartTunnel()
{
	Lock autoLock(this);

	// make sure that the tunnel is not already open
	if (m_Tunnel)
	{
		if (m_Tunnel->Success())
		{
			//already open!
			return false;
		}
		else
		{
			//clean up the old journal
			delete m_Tunnel;
			m_Tunnel = 0;
		}
	}

	// clear the params
	m_TunnelParams.Empty();

	// get the name of the journal
	SetJournalPathImpl();

	// build the params used to connect
	m_TunnelParams.Set("do_method", "route");
	m_TunnelParams.Set(L"kn_from", m_JournalPath.c_str());
	m_TunnelParams.Set("kn_response_format", "simple");

	if (m_Transport == 0)
		return false;

	m_Tunnel = m_Transport->OpenTunnel(m_TunnelParams);

	if (m_Tunnel == 0)
		return false;

	m_Connector->OnConnectionStatus(m_Tunnel->GetStatus());
	m_Connector->GetRandR().ConnectStatus(m_Tunnel->GetStatus());

	if (!m_Tunnel->Success())
	{
		m_Tunnel->Close();
		// Set the error
		//
		return false;
	}

	ResetEvent(m_ThreadDeathEvent);
	
	m_ExpectClosing = false;

	DWORD threadId = 0;

	if (m_hReadThread == 0)
		m_hReadThread = CreateThread(0, 0, ReadSimpleTunnelThread, static_cast<void*>(this), 0, &threadId);

	if (m_hReadThread == 0)
	{
		m_Tunnel->Close();
		SetEvent(m_ThreadDeathEvent);

		//!TH
		// set_error
		return false;
	}

	return true;
}

void Journal::ExpectClosing()
{
	Lock autoLock(this);
	m_ExpectClosing = true;
}

bool Journal::RestartTunnel()
{
	Lock autoLock(this);

	// make sure there is an existing journal and tunnel to restart
	if (m_TunnelParams.IsEmpty() || m_Tunnel == NULL)
		return StartTunnel();

	// if the tunnel has been closed then just pass it back so that the reader thread knows to quit
	if (m_Tunnel->IsClosed())
		return true;

	// if the tunnel is open(?) then close it
	if (m_Tunnel->Success()) 
	{
		m_Tunnel->Close();
	}

	//delete the tunnel, we will make a new one below
	delete m_Tunnel;

	//Lock autoLock(this);

	if (m_Transport == 0)
		return false;

	m_Tunnel = m_Transport->OpenTunnel(m_TunnelParams);

	if (m_Tunnel == 0)
		return false;

	m_Connector->OnConnectionStatus(m_Tunnel->GetStatus());
	m_Connector->GetRandR().ConnectStatus(m_Tunnel->GetStatus());

	if (!m_Tunnel->Success())
	{
		m_Tunnel->Close();
		// Set the error
		//
		return false;
	}

	return true;
}

bool Journal::CloseTunnel()
{
	Lock autoLock(this);

	// close the tunnel
	if (m_Tunnel) 
		m_Tunnel->Close();

	// make sure the reader thread stops
	DWORD ret = WaitForSingleObject(m_ThreadDeathEvent, 60000);

	if (m_hReadThread) 
	{
		CloseHandle(m_hReadThread);
		m_hReadThread = 0;
	}

	//check that the reader thread died
	if (ret == WAIT_OBJECT_0) 
	{
		delete m_Tunnel;
		return true;
	}

	//the reader thread may still be running
	return false;
}

int ReadFromJournal(Tunnel* t, mb_buf_ptr* buf_ptr, bool& isException)
{
	if (t == 0)
		return -2;

	int nRead = 0;
	isException = false;
	
	__try
	{
		nRead = t->ReadData(buf_ptr);
	}
	__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? 
		EXCEPTION_EXECUTE_HANDLER : 
		EXCEPTION_CONTINUE_SEARCH)
	{
		nRead = -2;
		isException = true;
	}

	return nRead;
}

void Journal::SafeConnectionStatus(Tunnel* t, Connector* c)
{
	if (IsBadReadPtr(t, sizeof Tunnel))
		return;

	if (IsBadReadPtr(c, sizeof Connector))
		return;

	__try
	{
		c->OnConnectionStatus(t->GetStatus());
	}
	__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? 
		EXCEPTION_EXECUTE_HANDLER : 
		EXCEPTION_CONTINUE_SEARCH)
	{
	}
}

DWORD WINAPI Journal::ReadSimpleTunnelThread(void* args)
{
	Journal* journal = static_cast<Journal*>(args);
	if (journal == 0)
		return 0;

	SimpleParser ep(journal->m_Connector);

	mb_buf_ptr buf_ptr;
	const int buffer_size = 16384;			// size of the buffer
	unsigned char read_buffer[buffer_size];	// buffer for the data

	int nRead = 0;
	int fib[] = 
	{
		1, 1, 2, 3, 5, 8, 13, 21
	};

	int fibSize = sizeof fib/sizeof (fib[0]);
	int fibIndex = 0;


	// This loop handles reading the data.
read:
	do
	{
		//put the pointer and length into a mb_buf_ptr
		buf_ptr.m_Ptr = &read_buffer[0];
		buf_ptr.m_Max = buffer_size;
		buf_ptr.m_Len = 0;

		// Read the data from the tunnel.
		bool isException = false;
		nRead = ReadFromJournal(journal->m_Tunnel, &buf_ptr, isException);

		if (isException)
			goto LeaveThread;

		if (nRead <= 0)
		{
			if (!IsBadReadPtr(journal, sizeof Journal))
				SafeConnectionStatus(journal->m_Tunnel, journal->m_Connector);
			break;
		}
		else
		{
			//parse the converted buffer
			while (buf_ptr.m_Len > 0)
				buf_ptr = ep.parse(buf_ptr);
		}
	} while (true);

	fibIndex = 0;

	//Check to see if this was an intended close
	if (nRead == -1 && journal->m_Tunnel != 0)
	{

		//we did not intend for the connection to close
		//keep trying until closed
		while (!journal->m_ExpectClosing)
		{
			//try and reconnect
			if (journal->RestartTunnel())
				goto read;	//continue to read 

			Message m;
			char buffer[32];

			sprintf(buffer, "%d", fib[fibIndex]);
			m.Set("delay", buffer);

			if (journal->m_Connector)
				journal->m_Connector->OnConnectionStatus(m);

			//give it a second before trying again
			Sleep(fib[fibIndex] * 1000);

			fibIndex = min(fibIndex + 1, fibSize - 1);
		}
	}

LeaveThread:
	//Once we get here we are leaving the thread
	SetEvent(journal->m_ThreadDeathEvent);

	return 1;
}


