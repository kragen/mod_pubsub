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

#include "stdafx.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>
#include <LibKN\Logger.h>
#include <stdio.h>

Connector::Connector() :
	m_Journal(this),
	m_Transport(this)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Connector()"));
}

Connector::~Connector()
{
	AutoMethod(g_cmpCppConnector, _T("Connector::~Connector()"));
	m_Listeners.clear();
	m_ConnectionStatusHandlers.clear();
}

bool Connector::IsConnected()
{
	AutoMethod(g_cmpCppConnector, _T("Connector::IsConnected()"));
	bool b = m_Transport.IsConnected();
	return b; // && m_Journal.IsConnected();
}

bool Connector::Open(const ITransport::Parameters& p)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Open"));
	bool b = m_Transport.Open(p);
	
	if (b)
		b = EnsureConnected();

	return b;
}

const ITransport::Parameters& Connector::GetParameters() const
{
	AutoMethod(g_cmpCppConnector, _T("Connector::GetParameters()"));
	return m_Transport.GetParameters();
}

bool Connector::Close()
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Close()"));
	m_Journal.ExpectClosing();
	m_Journal.CloseTunnel();
	return m_Transport.Close();
}

bool Connector::EnsureConnected()
{
	AutoMethod(g_cmpCppConnector, _T("Connector::EnsureConnected()"));
	return m_Transport.EnsureConnected();
}


bool Connector::GetQueueing()
{
	AutoMethod(g_cmpCppConnector, _T("Connector::GetQueueing()"));
	return m_OfflineQueue.GetQueueing();
}

void Connector::SetQueueing(bool on)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::SetQueueing()"));
	m_OfflineQueue.SetQueueing(on);
}

bool Connector::SaveQueue(const wstring& filename)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::SaveQueue()"));
	return m_OfflineQueue.SaveQueue(filename);
}

bool Connector::LoadQueue(const wstring& filename)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::LoadQueue()"));
	return m_OfflineQueue.LoadQueue(filename);
}

bool Connector::Clear()
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Clear()"));
	return m_OfflineQueue.Clear();
}

bool Connector::Flush()
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Flush()"));
	Lock autoLock(this);

	if (!m_OfflineQueue.m_Queue.empty())
	{
		Message m;
		m.Set(L"FlushQueue", L"Start");
	
		OnConnectionStatusImpl(m);
	
		bool b = m_OfflineQueue.FlushImp(this);
	
		m.Empty();
		m.Set(L"FlushQueue", L"End");
		m.Set(L"FlushStatus", b ? L"Done" : L"Fail");
	
		OnConnectionStatusImpl(m);
	}

	return true;
}

bool Connector::HasItems()
{
	AutoMethod(g_cmpCppConnector, _T("Connector::HasItems()"));
	return m_OfflineQueue.HasItems();
}


bool Connector::Publish(const Message& msg, IRequestStatusHandler* sh)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Publish()"));

	bool b = m_Journal.EnsureConnected();

	bool sent = false;

	if (b && IsConnected())
	{
		Flush();
		sent = Post(msg, sh);
	}

	if (!sent)
	{
		Lock autoLock(this);

		bool authFailed = false;
		wstring status;

		if (m_LastConnectionStatus.Get("status_code", status))
		{
			if (status == L"403" || status == L"401")
			{
				authFailed = true;
			}
		}

		if (!authFailed)
		{
			m_OfflineQueue.Add(msg.GetAsHttpParam());
		}
	}

	if (!sent)
	{
		if (sh)
		{
			Lock autoLock(this);
			Message m = m_LastConnectionStatus;

			wstring status;

			if (m_LastConnectionStatus.Get("status_code", status))
			{
				if (status == L"200")
				{
					char buffer[32];
					sprintf(buffer, "%d", HTTP_STATUS_SERVER_ERROR);
					m.Set("status_code", buffer);
				}
			}

			sh->OnStatus(m);
		}
	}

	return sent;
}

wstring Connector::Subscribe(const wstring& topic, IListener* listener, const Message& options, IRequestStatusHandler* sh)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Subscribe()"));
	Lock autoLock(this);

	if (!m_Journal.EnsureConnected())
	{
		if (sh)
		{
			Message m = m_LastConnectionStatus;
			sh->OnStatus(m);
		}

		return L"";
	}

	if (IsConnected())
		Flush();

	Message msg(options);
	wstring rid = GetRouteId(topic, msg);

	if (SubscribeRouteId(msg, rid, topic, listener, sh))
		return rid;
	else
		return L"";
}

bool Connector::SubscribeRouteId(Message& msg, const wstring& rid, const wstring& topic, 
	IListener* listener, IRequestStatusHandler* sh)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::SubscribeRouteId()"));
	{
		Lock autoLock(this);
		m_Listeners[rid] = listener;
	}

	if (!m_Journal.EnsureConnected())
	{
		if (sh)
		{
			Message m = m_LastConnectionStatus;
			sh->OnStatus(m);
		}

		return false;
	}

	if (IsConnected())
		Flush();

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
	AutoMethod(g_cmpCppConnector, _T("Connector::GetRouteIdImpl()"));

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
	AutoMethod(g_cmpCppConnector, _T("Connector::GetRouteId()"));

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
	AutoMethod(g_cmpCppConnector, _T("Connector::ParseRouteId()"));

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
	AutoMethod(g_cmpCppConnector, _T("Connector::GetSubscriptionId()"));

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
	AutoMethod(g_cmpCppConnector, _T("Connector::Post()"));

	return m_Transport.Send(msg, sh);
}

wstring Connector::Route(Message& msg, const wstring& rid, const wstring& from, const wstring& to, 
	IListener* listener, IRequestStatusHandler* sh)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Route()"));

	msg.Set(L"do_method", L"route");
	msg.Set(L"kn_from", from);
	msg.Set(L"kn_to", to);
	msg.Set(L"kn_uri", rid);
	msg.Set(L"kn_response_format", L"simple");

	if (!Post(msg, sh))
	{
		if (sh)
		{
			Message m = m_LastConnectionStatus;
			sh->OnStatus(m);
		}

		return L"";
	}

	return rid;
}

wstring Connector::Route(const wstring& from, const wstring& to, IListener* listener, IRequestStatusHandler* sh)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Route()"));

	wstring rid;
	Message msg;
	rid = GetRouteId(from, msg);

	return Route(msg, rid, from, to, listener, sh);
}

bool Connector::Unsubscribe(const wstring& rid, IRequestStatusHandler* sh)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::Unsubscribe()"));

	if (IsConnected())
		Flush();

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

	bool b = Post(msg, sh);

	if (!b)
	{
		if (sh)
		{
			Message m = m_LastConnectionStatus;
			sh->OnStatus(m);
		}
	}

	return b;
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
	AutoMethod(g_cmpCppConnector, _T("Connector::AddConnectionStatusHandler()"));

	if (sh == 0)
		return;

	Lock autoLock(this);
	m_ConnectionStatusHandlers.push_back(sh);
}

void Connector::RemoveConnectionStatusHandler(IConnectionStatusHandler* sh)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::RemoveConnectionStatusHandler()"));

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
	AutoMethod(g_cmpCppConnector, _T("Connector::OnFireEvent()"));

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
	AutoMethod(g_cmpCppConnector, _T("Connector::OnConnectionStatusImpl()"));
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

void Connector::OnConnectionStatus(const Message& msg)
{
	AutoMethod(g_cmpCppConnector, _T("Connector::OnConnectionStatus()"));

	m_LastConnectionStatus = msg;

	OnConnectionStatusImpl(msg);

	wstring v;
	if (msg.Get(L"journal", v))
	{
		// Reconnected
		//
		Flush();
	}
}


