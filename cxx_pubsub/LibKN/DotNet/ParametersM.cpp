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
#include "ParametersM.h"
#include "StringToChar.h"

namespace LibKNDotNet
{

Parameters::Parameters()
{
	m_ParametersImpl = new ::ITransport::Parameters();
	m_CS = new CCriticalSection();
}

Parameters::~Parameters()
{
	delete m_ParametersImpl;
	delete m_CS;
}

String* Parameters::get_ServerUrl()
{
	return new String(m_ParametersImpl->m_ServerUrl.c_str());
}

void Parameters::set_ServerUrl(String* val)
{
	StringToChar* v = new StringToChar(val);
	m_ParametersImpl->m_ServerUrl = v->GetChar();
}

String* Parameters::get_Username()
{
	return new String(m_ParametersImpl->m_Username.c_str());
}

void Parameters::set_Username(String* val)
{
	StringToChar* v = new StringToChar(val);
	m_ParametersImpl->m_Username = v->GetChar();
}

String* Parameters::get_Password()
{
	return new String(m_ParametersImpl->m_Password.c_str());
}

void Parameters::set_Password(String* val)
{
	StringToChar* v = new StringToChar(val);
	m_ParametersImpl->m_Password = v->GetChar();
}

bool Parameters::get_UseProxy()
{
	return m_ParametersImpl->m_UseProxy;
}

void Parameters::set_UseProxy(bool val)
{
	m_ParametersImpl->m_UseProxy = val;
}

String* Parameters::get_ProxyServer()
{
	return new String(m_ParametersImpl->m_ProxyServer.c_str());
}

void Parameters::set_ProxyServer(String* val)
{
	StringToChar* v = new StringToChar(val);
	m_ParametersImpl->m_ProxyServer = v->GetChar();
}

String* Parameters::get_ProxyUsername()
{
	return new String(m_ParametersImpl->m_ProxyUsername.c_str());
}

void Parameters::set_ProxyUsername(String* val)
{
	StringToChar* v = new StringToChar(val);
	m_ParametersImpl->m_ProxyUsername = v->GetChar();
}

String* Parameters::get_ProxyPassword()
{
	return new String(m_ParametersImpl->m_ProxyPassword.c_str());
}

void Parameters::set_ProxyPassword(String* val)
{
	StringToChar* v = new StringToChar(val);
	m_ParametersImpl->m_ProxyPassword = v->GetChar();
}

String* Parameters::get_ProxyExceptionList()
{
	return new String(m_ParametersImpl->m_ProxyExceptionList.c_str());
}

void Parameters::set_ProxyExceptionList(String* val)
{
	StringToChar* v = new StringToChar(val);
	m_ParametersImpl->m_ProxyExceptionList = v->GetChar();
}

String* Parameters::get_CustomHeader()
{
	return new String(m_ParametersImpl->m_CustomHeader.c_str());
}

void Parameters::set_CustomHeader(String* val)
{
	StringToChar* v = new StringToChar(val);
	m_ParametersImpl->m_CustomHeader = v->GetChar();
}

bool Parameters::get_ShowUI()
{
	return m_ParametersImpl->m_ShowUI;
}

void Parameters::set_ShowUI(bool val)
{
	m_ParametersImpl->m_ShowUI = val;
}

}
