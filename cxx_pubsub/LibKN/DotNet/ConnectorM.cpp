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
#include "ConnectorM.h"
#include "Convert.h"
#include "StringToChar.h"

namespace LibKNDotNet
{

Connector::Connector()
{
	m_ConnectorImpl = new ::Connector();
	m_ConnectionStatusHandlers = new TBridgeCollection<ConnectionStatusHandlerBridge*>();
	m_Listeners = new TBridgeCollection<ListenerBridge*>();
	m_ListenerMap = new ListenerMap();
	m_CS = new CCriticalSection();
}

Connector::~Connector()
{
	delete m_ConnectorImpl;
	delete m_ConnectionStatusHandlers;
	delete m_Listeners;
	delete m_ListenerMap;
	delete m_CS;
}

bool Connector::IsConnected()
{
	return m_ConnectorImpl->IsConnected();
}

bool Connector::Open(Parameters* p)
{
	Lock autoLock(m_CS);

	::ITransport::Parameters* _p = M2N_Parameters(p);
	bool retval = m_ConnectorImpl->Open(*_p);
	delete _p;
	return retval;
}

Parameters* Connector::GetParameters()
{
	Lock autoLock(m_CS);
	Parameters* p = N2M_Parameters(m_ConnectorImpl->GetParameters());
	return p;
}

bool Connector::Close()
{
	return m_ConnectorImpl->Close();
}

bool Connector::EnsureConnected()
{
	return m_ConnectorImpl->EnsureConnected();
}

bool Connector::get_Queueing()
{
	return m_ConnectorImpl->GetQueueing();
}

void Connector::set_Queueing(bool on)
{
	return m_ConnectorImpl->SetQueueing(on);
}

bool Connector::SaveQueue(String* filename)
{
	StringToWChar* _filename = new StringToWChar(filename);
	return m_ConnectorImpl->SaveQueue(_filename->GetChar());
}

bool Connector::LoadQueue(String* filename)
{
	StringToWChar* _filename = new StringToWChar(filename);
	return m_ConnectorImpl->LoadQueue(_filename->GetChar());
}

bool Connector::Flush()
{
	return m_ConnectorImpl->Flush();
}

bool Connector::Clear()
{
	return m_ConnectorImpl->Clear();
}

bool Connector::HasItems()
{
	return m_ConnectorImpl->HasItems();
}

bool Connector::Publish(Message* msg, IRequestStatusHandler* sh)
{
	Lock autoLock(m_CS);
	RequestStatusHandlerBridge* _shb = new RequestStatusHandlerBridge(sh);
	const ::Message& m = *msg->GetImpl();
	bool retVal = m_ConnectorImpl->Publish(m, _shb);
	delete _shb;
	return retVal;
}

String* Connector::Subscribe(String* topic, IListener* listener, Message* options, IRequestStatusHandler* sh)
{
	Lock autoLock(m_CS);
	RequestStatusHandlerBridge* _shb = new RequestStatusHandlerBridge(sh);
	StringToWChar* _t = new StringToWChar(topic);
	ListenerBridge* _lb = Wrap(listener);

	::Message _options;
	
	if (options)
		_options = *options->GetImpl();

	wstring retVal = m_ConnectorImpl->Subscribe(_t->GetChar(), _lb, _options, _shb);

	delete _shb;
	
	(*m_ListenerMap)[retVal] = _lb;

	return new String(retVal.c_str());
}

bool Connector::Unsubscribe(String* rid, IRequestStatusHandler* sh)
{
	Lock autoLock(m_CS);
	RequestStatusHandlerBridge* _shb = new RequestStatusHandlerBridge(sh);
	StringToWChar* _rid = new StringToWChar(rid);
	ListenerMap::iterator it = m_ListenerMap->find(_rid->GetChar());

	if (it == m_ListenerMap->end())
		return false;

	ListenerBridge* _lb = (*it).second;
	m_Listeners->Remove(_lb);
	m_ListenerMap->erase(it);
	delete _lb;

	bool retval = m_ConnectorImpl->Unsubscribe(_rid->GetChar(), _shb);
	delete _shb;
	return retval;
}

void Connector::AddConnectionStatusHandler(IConnectionStatusHandler* sh)
{
	Lock autoLock(m_CS);
	ConnectionStatusHandlerBridge* _shb = new ConnectionStatusHandlerBridge(sh);
	m_ConnectionStatusHandlers->Add(_shb);
	m_ConnectorImpl->AddConnectionStatusHandler(_shb);
}

void Connector::RemoveConnectionStatusHandler(IConnectionStatusHandler* sh)
{
	Lock autoLock(m_CS);
	ConnectionStatusHandlerBridge* _shb = new ConnectionStatusHandlerBridge(sh);
	ConnectionStatusHandlerBridge* realBridge = m_ConnectionStatusHandlers->Remove(_shb);

	if (realBridge != 0)
		m_ConnectorImpl->RemoveConnectionStatusHandler(realBridge);
}

ListenerBridge* Connector::Wrap(IListener* listener)
{
	ListenerBridge* _lb = new ListenerBridge(listener);
	m_Listeners->Add(_lb);
	return _lb;
}

}

