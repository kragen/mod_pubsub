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

#if !defined(STATUSHANDLERADAPTER_H)
#define STATUSHANDLERADAPTER_H

#include "LibKNCom.h"
#include <LibKN\IRequestStatusHandler.h>
#include <LibKN\IListener.h>

class CConnector;

class RequestStatusHandlerAdapter : public IRequestStatusHandler
{
public:
	RequestStatusHandlerAdapter(CConnector* conn, IComRequestStatusHandler* sh);
	~RequestStatusHandlerAdapter();

	void OnStatus(const Message& msg);

	IComRequestStatusHandler* GetComHandler();

private:
	CConnector* m_Conn;
	IComRequestStatusHandler* m_Handler;
};

class ListenerHandlerAdapter : public IListener
{
public:
	ListenerHandlerAdapter(CConnector* conn, IComListener* l);
	~ListenerHandlerAdapter();

	void OnUpdate(const Message& msg);

	IComListener* GetComHandler();

private:
	CConnector* m_Conn;
	IComListener* m_Handler;
};

#endif
