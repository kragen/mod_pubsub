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
#include <LibKN\RandRHandler.h>
#include <LibKN\Logger.h>
#include <stdlib.h>
#include <limits.h>

EventSet::EventSet()
{
	m_NextCount = 0;
	m_Timeout = 60000;

	m_Current = &m_Set1;
	m_Older = &m_Set2;
}

EventSet::~EventSet()
{
}

void EventSet::SetTimeout(int timeout)
{
	m_Timeout = timeout * 1000;
}

bool EventSet::Insert(const wstring& new_string)
{
	DWORD this_count = GetTickCount();

	if (m_NextCount < this_count)
	{
		//empty older set
		m_Older->clear();

		//swap the sets
		EventSetImpl* temp = m_Current;
		m_Current = m_Older;
		m_Older = temp;

		//figure out the next tick count to clear
		if (_UI32_MAX - m_NextCount < (unsigned) m_Timeout)
			m_NextCount = m_Timeout - (_UI32_MAX - m_NextCount);
		else
			m_NextCount = this_count + m_Timeout;
	}


	if (!m_Current->insert(new_string).second)
		return false;

	if (m_Older->find(new_string) != m_Older->end())
		return false;

	return true;
}


RandRHandler::RandRHandler()
{
	m_Scheme = RouteCheckpoint;
	m_ReconnectTimeout = L"60";
}

RandRHandler::~RandRHandler()
{
	Lock autoLock(this);
}

void RandRHandler::ConnectStatus(const Message& status)
{
	Lock autoLock(this);

	Message::Container::const_iterator pos = status.GetContainer().find(L"kn_journal_reconnect_scheme");
	if (pos != status.GetContainer().end())
	{
		wstring scheme = (*pos).second;
		if (0 == scheme.compare(L"kn_route_checkpoint"))
			m_Scheme = RouteCheckpoint;
		else if (0 == scheme.compare(L"kn_event_hash"))
			m_Scheme = EventHash;
		else
			m_Scheme = None;
	}
	else
		m_Scheme = RouteCheckpoint;

	pos = status.GetContainer().find(L"kn_journal_reconnect_timeout");
	if (pos != status.GetContainer().end())
		m_ReconnectTimeout = (*pos).second;

	m_Set.SetTimeout(_wtoi(m_ReconnectTimeout.c_str()));
}

bool RandRHandler::EventCheck(const Message& event)
{
	Lock autoLock(this);

	switch (m_Scheme)
	{
		case RouteCheckpoint: 
		{
			Message::Container::const_iterator pos = event.GetContainer().find(L"kn_route_checkpoint");
			
			if (pos != event.GetContainer().end())
				m_Checkpoint = (*pos).second;
			
			return true;
		}

		case EventHash: 
		{
			//kn_route_location + kn_event_hash uniquely identify the event
			wstring event_hash;
			Message::Container::const_iterator pos = event.GetContainer().find(L"kn_route_location");

			if (pos != event.GetContainer().end())
			{
				event_hash = (*pos).second;
				pos = event.GetContainer().find(L"kn_event_hash");
				if (pos != event.GetContainer().end())
					event_hash += (*pos).second;
				else
					return true;
			}
			else
				return true;

			//check event against the set of events already seen and throw-out if duplicate
			if (!m_Set.Insert(event_hash))
				return false;

			return true;
		}

		case None:
			return true;
	}

	return true;
}

void RandRHandler::SetReconnectParams(Message& msg)
{
	Lock autoLock(this);

	switch (m_Scheme)
	{
		case RouteCheckpoint:
			msg.Set(L"do_since_checkpoint", m_Checkpoint);
			return;

		case EventHash:
			msg.Set(L"do_max_age", m_ReconnectTimeout);
			return;

		case None:
			return;
	}
}

