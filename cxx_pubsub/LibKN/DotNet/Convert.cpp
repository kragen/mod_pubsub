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
#include "Convert.h"
#include "StringToChar.h"

namespace LibKNDotNet
{
Parameters* N2M_Parameters(const ::ITransport::Parameters& p)
{
	Parameters* retVal = new Parameters();

	retVal->ServerUrl = p.m_ServerUrl.c_str();
	retVal->Username = p.m_Username.c_str();
	retVal->Password = p.m_Password.c_str();
	retVal->UseProxy = p.m_UseProxy;
	retVal->ProxyServer = p.m_ProxyServer.c_str();
	retVal->ProxyUsername = p.m_ProxyUsername.c_str();
	retVal->ProxyPassword = p.m_ProxyPassword.c_str();
	retVal->ProxyExceptionList = p.m_ProxyExceptionList.c_str();
	retVal->CustomHeader = p.m_CustomHeader.c_str();
	retVal->ShowUI = p.m_ShowUI;

	return retVal;
}

tstring Convert(System::String* s)
{
	if (s == 0)
		return "";

	StringToChar* _s = new StringToChar(s);
	tstring t(_s->GetChar());
	return t;
}

::ITransport::Parameters* M2N_Parameters(Parameters* p)
{
	::ITransport::Parameters* retVal = new ::ITransport::Parameters();

	tstring t = Convert(p->get_ServerUrl());
	retVal->m_ServerUrl = t;

	t = Convert(p->get_Username());
	retVal->m_Username = t;

	retVal->m_Password = Convert(p->get_Password());
	retVal->m_UseProxy = p->get_UseProxy();
	retVal->m_ProxyServer = Convert(p->get_ProxyServer());
	retVal->m_ProxyUsername = Convert(p->get_ProxyPassword());
	retVal->m_ProxyExceptionList = Convert(p->get_ProxyExceptionList());
	retVal->m_CustomHeader = Convert(p->get_CustomHeader());
	retVal->m_ShowUI = p->get_ShowUI();

	return retVal;
}

}
