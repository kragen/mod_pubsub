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
#include <LibKN\OfflineQ.h>
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>
#include <LibKN\Logger.h>

IOfflineQueue::~IOfflineQueue()
{
}


OfflineQueue::OfflineQueue()
{
	m_On = false;
}

OfflineQueue::~OfflineQueue()
{
	Clear();
}

bool OfflineQueue::GetQueueing()
{
	return m_On;
}

void OfflineQueue::SetQueueing(bool on)
{
	m_On = on;
}
	
bool OfflineQueue::SaveQueue(const wstring& filename)
{
	if (filename.empty())
		return false;

	tstring tfn = ConvertToTString(filename);

	HANDLE file = CreateFile(tfn.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		DWORD num_written = 0;

		DWORD pos = SetFilePointer(file, 0, NULL, FILE_END);

		if (!SetEndOfFile(file))
		{
			return false;
		}

		while (!m_Queue.empty())
		{
			//save the item to the file
			WriteFile(file, m_Queue.front().c_str(), m_Queue.front().length(), &num_written, NULL);

			if (num_written != m_Queue.front().length())
			{
				CloseHandle(file);
				return false;
			}

			//put a newline after every event
			WriteFile(file, "\n", 1, &num_written, NULL);

			if (num_written != 1)
			{
				CloseHandle(file);
				return false;
			}

			//remove the item from the queue
			m_Queue.pop();
		}
	}
	else
	{
		//can't open file
		return false;
	}

	CloseHandle(file);

	return true;
}

bool OfflineQueue::LoadQueue(const wstring& filename)
{
	if (filename.empty())
		return false;

	Clear();

	tstring tfn = ConvertToTString(filename);
	HANDLE file = CreateFile(tfn.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		//fill the queue
		string payload;
		char c;

		DWORD num_read = 0;

		//try and get the first char
		if (!ReadFile(file, &c, 1, &num_read, NULL))
		{
			//file error
			CloseHandle(file);
			return false;
		}

		//continue as long as we can read
		while (num_read != 0)
		{
			if (c != '\n')
				payload += c;
			else
			{
				m_Queue.push(payload);
				payload = "";
			}

			//try and read the next char
			if (!ReadFile(file, &c, 1, &num_read, NULL))
			{
				//file error
				CloseHandle(file);
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	CloseHandle(file);
	return true;
}

bool OfflineQueue::Flush()
{
	return false;
}

bool OfflineQueue::HasItems()
{
	return !m_Queue.empty();
}

bool OfflineQueue::FlushImp(Connector* c)
{
	while (!m_Queue.empty())
	{
		if (c)
		{
			Transport& t = c->GetTransport();
	
			bool b = t.Send(m_Queue.front(), 0);
	
			if (b)
			{
				Message m;
				m.Set("FlushMsg", m_Queue.front());
				c->OnConnectionStatusImpl(m);
			}
			else
			{
				return false;
			}
		}

		m_Queue.pop();
	}

	return true;
}

bool OfflineQueue::Clear()
{
	while (!m_Queue.empty())
		m_Queue.pop();

	return true;
}

void OfflineQueue::Add(const string& data)
{
	if (GetQueueing())
		m_Queue.push(data);
}



