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
#include "StatusHandlerAdapter.h"
#include "CConnector.h"

RequestStatusHandlerAdapter::RequestStatusHandlerAdapter(
	CConnector* conn, IComRequestStatusHandler* sh) :
	m_Conn(conn),
	m_Handler(sh)
{
	if (m_Handler)
		m_Handler->AddRef();
}

RequestStatusHandlerAdapter::~RequestStatusHandlerAdapter()
{
	if (m_Handler != 0)
	{
		m_Handler->Release();
		m_Handler = 0;
	}
}

IComRequestStatusHandler* RequestStatusHandlerAdapter::GetComHandler()
{
	return m_Handler;
}

void RequestStatusHandlerAdapter::OnStatus(const Message& msg)
{
	if (m_Conn)
		m_Conn->OnStatusFromHandler(this, msg);
}


ListenerHandlerAdapter::ListenerHandlerAdapter(CConnector* conn, IComListener* l) :
	m_Conn(conn),
	m_Handler(l)
{
	if (m_Handler)
		m_Handler->AddRef();
}

ListenerHandlerAdapter::~ListenerHandlerAdapter()
{
	if (m_Handler)
	{
		m_Handler->Release();
		m_Handler = 0;
	}
}

void ListenerHandlerAdapter::OnUpdate(const Message& msg)
{
	if (m_Conn)
		m_Conn->OnUpdateFromHandler(this, msg);
}

IComListener* ListenerHandlerAdapter::GetComHandler()
{
	return m_Handler;
}


