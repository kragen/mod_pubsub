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
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>
#include <stdio.h>

Connector::Connector() :
	m_Journal(this),
	m_Transport(this)
{
}

Connector::~Connector()
{
	m_Listeners.clear();
	m_ConnectionStatusHandlers.clear();
}

bool Connector::IsConnected()
{
	bool b = m_Transport.IsConnected();
	return b && m_Journal.IsConnected();
}

bool Connector::Open(const ITransport::Parameters& p)
{
	bool b = m_Transport.Open(p);
	return b;
}

const ITransport::Parameters& Connector::GetParameters() const
{
	return m_Transport.GetParameters();
}

bool Connector::Close()
{
//	m_Journal.CloseTunnel();
	return m_Transport.Close();
}

bool Connector::EnsureConnected()
{
	return m_Transport.EnsureConnected();
}

bool Connector::Publish(const Message& msg, IRequestStatusHandler* sh)
{
	Lock autoLock(this);

	bool b = m_Journal.EnsureConnected();

	bool sent = false;

	if (b && IsConnected())
	{
		FlushQueue();
		sent = Post(msg, sh);
	}

	if (!sent)
	{
		m_OfflineQueue.Add(msg.GetAsHttpParam());
	}

	return sent;
}

wstring Connector::Subscribe(const wstring& topic, IListener* listener, IRequestStatusHandler* sh)
{
	Lock autoLock(this);

	if (!m_Journal.EnsureConnected())
		return L"";

	Message msg;
	wstring rid = GetRouteId(topic, msg);

	if (SubscribeRouteId(msg, rid, topic, listener, sh))
		return rid;
	else
		return L"";
}

bool Connector::SubscribeRouteId(Message& msg, const wstring& rid, const wstring& topic, 
	IListener* listener, IRequestStatusHandler* sh)
{
	{
		Lock autoLock(this);
		m_Listeners[rid] = listener;
	}

	wstring temp = Route(msg, rid, topic, m_Journal.GetJournalPath(), listener, sh);

	bool b = temp == rid;

#if 0
	if (!b)
	{
		printf("[\ntemp = %S\n rid = %S\n]\n", temp.c_str(), rid.c_str());
	}
#endif

	return b;
}

wstring Connector::GetRouteIdImpl(const wstring& topic, Message& msg) const
{
	// String to hold the kn_id for the route
	wstring sub_id;

	{
		Lock autoLock(this);

		//get an existing id or make and add the id to the parameters
		Message::Container::const_iterator pos = msg.GetContainer().find(L"kn_id");

		if (msg.GetContainer().end() == pos)
		{
			//generate an id for the subscription
			sub_id = GetSubscriptionId();

			//no kn_id has been specified
			msg.Set(L"kn_id", sub_id);
		}
		else
		{
			//an id has been specified, use it
			sub_id = (*pos).second;
		}
	}

	//String to hold the route id
	wstring rid = ConvertToWide(m_Transport.GetUrl());

	//first part is the url of the server
	if (0 != rid.compare(rid.length() - 1, 1, L"/"))
	{
		rid.append(L"/");
	}

	//then the topic
	if (0 != topic.compare(0, 1, L"/"))
		rid += topic;
	else
		rid	+= topic.substr(1);

	//and the sub_id
	rid += L"/kn_routes/";
	rid += sub_id.c_str();

	return rid;
}

wstring Connector::GetRouteId(const wstring& topic, Message& msg) const
{
	bool uniqueRid = false;
	wstring rid;
	 
	while (!uniqueRid)
	{
		rid = GetRouteIdImpl(topic, msg);

		{
			Lock autoLock(this);
			uniqueRid = m_Listeners.find(rid) == m_Listeners.end();
		}
	}

	msg.Set(L"LIBKN_RID", rid);

	return rid;
}

pair<wstring, wstring> Connector::ParseRouteId(const wstring& _rid)
{
	wstring rid = _rid;

	pair<wstring, wstring> ret_val;

	//check to make sure that the url matches
	if (0 != rid.compare(0, m_Transport.GetUrl().length(), ConvertToWide(m_Transport.GetUrl())))
	{
		//not the same server that we are connected to
		return ret_val;
	}

	//get rid of the url portion
	rid.erase(0, m_Transport.GetUrl().length());

	//parse the from topic and id out of the rid
	wstring::size_type beginpos = rid.rfind(L"/kn_routes/");

	if (wstring::npos == beginpos)
	{
		//error, bad rid
		return ret_val;
	}

	//get the from topic out
	wstring from(rid.substr(0, beginpos)); //.c_str());

	//get the kn_id
	wstring kn_id( rid.substr(beginpos + wstring(L"/kn_routes/").length()) );

	ret_val.first = from;
	ret_val.second = kn_id;

	return ret_val;
}

void Seed()
{
	DWORD d = GetTickCount();
	DWORD t = GetCurrentThreadId() << 2;

	d += t;

	srand(d);
}

wstring Connector::GetSubscriptionId() const
{
	Seed();

	//generate an id for the subscription
	int id = rand();
	//make a string out of it
	wchar_t formated[11];
	swprintf(&formated[0], L"%08i", id);

	return wstring(&formated[0]);
}

bool Connector::Post(const Message& msg, IRequestStatusHandler* sh)
{
	return m_Transport.Send(msg, sh);
}

wstring Connector::Route(Message& msg, const wstring& rid, const wstring& from, const wstring& to, 
	IListener* listener, IRequestStatusHandler* sh)
{
	msg.Set(L"do_method", L"route");
	msg.Set(L"kn_from", from);
	msg.Set(L"kn_to", to);
	msg.Set(L"kn_uri", rid);
	msg.Set(L"kn_response_format", L"simple");

	if (!Post(msg, sh))
	{
		return L"";
	}

	return rid;
}

wstring Connector::Route(const wstring& from, const wstring& to, IListener* listener, IRequestStatusHandler* sh)
{
	wstring rid;
	Message msg;
	rid = GetRouteId(from, msg);

	return Route(msg, rid, from, to, listener, sh);
}

bool Connector::Unsubscribe(const wstring& rid, IRequestStatusHandler* sh)
{
	{
		Lock autoLock(this);

		Listeners::iterator it = m_Listeners.find(rid);

		if (it == m_Listeners.end())
			return true;
		else
			m_Listeners.erase(it);
	}

	pair<wstring, wstring> route_info = ParseRouteId(rid);

	if (route_info.first.length() == 0 || route_info.second.length() == 0) 
	{
		//bad rid
		return false;
	}

	//delete the route to the journal
	Message msg;

	msg.Set(L"do_method", L"route");
	msg.Set(L"kn_from", route_info.first);
	msg.Set(L"kn_id", route_info.second);
  	msg.Set(L"kn_to", L"");
  	msg.Set(L"kn_response_format", L"simple");
  	msg.Set(L"LIBKN_RID", rid);

	return Post(msg, sh);
}

RandRHandler& Connector::GetRandR()
{
	Lock autoLock(this);
	return m_RandR;
}

Transport& Connector::GetTransport()
{
	Lock autoLock(this);
	return m_Transport;
}

void Connector::AddConnectionStatusHandler(IConnectionStatusHandler* sh)
{
	if (sh == 0)
		return;
	
	Lock autoLock(this);
	m_ConnectionStatusHandlers.push_back(sh);
}

void Connector::RemoveConnectionStatusHandler(IConnectionStatusHandler* sh)
{
	if (sh == 0)
		return;

	Lock autoLock(this);
		
	ConnectionStatusHandlers::iterator it = find(m_ConnectionStatusHandlers.begin(), m_ConnectionStatusHandlers.end(), sh);
	if (it == m_ConnectionStatusHandlers.end())
		return;

	m_ConnectionStatusHandlers.erase(it);
}

void Connector::OnFireEvent(const Message& msg)
{
	Lock autoLock(this);

	if (m_Listeners.empty())
		return;

	wstring rid;
	if (!msg.Get(L"kn_route_location", rid))
		return;

	Listeners::iterator it = m_Listeners.find(rid);
	if (it == m_Listeners.end())
		return;

	IListener* listener = (*it).second;

	if (listener == 0)
		return;

	listener->OnUpdate(msg);
}

void Connector::OnConnectionStatusImpl(const Message& msg)
{
	Lock autoLock(this);

	if (m_ConnectionStatusHandlers.empty())
		return;

	for (ConnectionStatusHandlers::iterator it = m_ConnectionStatusHandlers.begin(); it != m_ConnectionStatusHandlers.end(); ++it)
	{
		IConnectionStatusHandler* sh = *it;

		if (sh)
		{
			sh->OnConnectionStatus(msg);
		}
	}
}

void Connector::FlushQueue()
{
	Lock autoLock(this);

	if (!m_OfflineQueue.m_Queue.empty())
	{
		Message m;
		m.Set(L"FlushQueue", L"Start");
	
		OnConnectionStatusImpl(m);
	
		bool b = m_OfflineQueue.Flush(this);
	
		m.Empty();
		m.Set(L"FlushQueue", L"End");
		m.Set(L"FlushStatus", b ? L"Done" : L"Fail");
	
		OnConnectionStatusImpl(m);
	}
}

void Connector::OnConnectionStatus(const Message& msg)
{
	OnConnectionStatusImpl(msg);

	wstring v;
	if (msg.Get(L"journal", v))
	{
		// Reconnected
		//
		FlushQueue();
	}
}


