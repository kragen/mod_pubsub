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
#include <LibKN\Message.h>
#include <LibKN\StrUtil.h>
#include <LibKN\SimpleParser.h>
#include <LibKN\Logger.h>

Message::Message() :
	m_ItemsHaveChanged(true)
{

}

Message::~Message()
{
}

Message::Message(const Message& rhs)
{
	Lock autoLock(this);

	*this = rhs;
	m_ItemsHaveChanged = true;
}

Message& Message::operator=(const Message& rhs)
{
	Lock autoLock(this);
	Lock autoLock2(&rhs);

	if (&rhs == this)
		return *this;

	m_Container = rhs.m_Container;
	m_ItemsHaveChanged = true;

	return *this;
}

bool Message::operator==(const Message& rhs)
{
	Lock autoLock(this);
	Lock autoLock2(&rhs);

	if (&rhs == this)
		return true;

	if (m_Container.size() != rhs.m_Container.size())
		return false;

	return m_Container == rhs.m_Container;
}

const string& Message::GetAsHttpParam() const
{
	Lock autoLock(this);

	if (m_ItemsHaveChanged)
		ConvertToHttpParam();

	return m_HttpParamString;
}
	
void Message::ConvertToHttpParam() const
{
	Lock autoLock(this);

	Message* This = const_cast<Message*>(this);
	This->m_HttpParamString.erase();

	for (Container::iterator it = This->m_Container.begin(); it != This->m_Container.end(); ++it)
	{
		This->m_HttpParamString.AddParam((*it).first, (*it).second);
	}

	This->m_ItemsHaveChanged = false;
}

void Message::Set(const wstring& f, const wstring& v)
{
	Lock autoLock(this);

	m_ItemsHaveChanged = true;

	// m_Container[f] = v;
	m_Container[f].assign(v);
}

void Message::Set(const string& f, const string& v)
{
	Set(ConvertToWide(f), ConvertToWide(v));
}

void Message::Remove(const wstring& f)
{
	Lock autoLock(this);

	m_ItemsHaveChanged = true;
	Container::iterator it = m_Container.find(f);

	if (it != m_Container.end())
		m_Container.erase(it);
}

void Message::Remove(const string& f)
{
	Remove(ConvertToWide(f));
}

bool Message::Get(const wstring& f, wstring& v) const
{
	Lock autoLock(this);

	Container::const_iterator it = m_Container.find(f);
	
	if (it == m_Container.end())
		return false;

	v = (*it).second;
	return true;
}

bool Message::Get(const string& f, wstring& v) const
{
	return Get(ConvertToWide(f), v);
}

void Message::Empty()
{
	Lock autoLock(this);

	m_ItemsHaveChanged = true;
	m_Container.clear();
}
	
bool Message::IsEmpty() const
{
	Lock autoLock(this);

	return m_Container.empty();
}

const Message::Container& Message::GetContainer() const
{
	Lock autoLock(this);

	return m_Container;
}

string Message::GetAsSimpleFormat() const
{
	Lock autoLock(this);

	string retVal;

	SimpleString simpleParamString;
	wstring payload;

	{
		Message* This = const_cast<Message*>(this);
		This->m_HttpParamString.erase();

		for (Container::iterator it = This->m_Container.begin(); it != This->m_Container.end(); ++it)
		{
			if ((*it).first.compare(L"kn_payload") != 0)
			{
				simpleParamString.AddParam((*it).first, (*it).second);
			}
			else
				payload = (*it).second;
		}
	}

	if (simpleParamString.length() > 0)
		retVal += simpleParamString;

	if (payload.length() > 0)
	{
		retVal += "\n";
		string s = ConvertToUtf8(payload);
		retVal.append(s);
	}

	return retVal;
}

bool Message::InitFromSimple(const string& str)
{
	Lock autoLock(this);

	SimpleParser sp(0);
	Message tmp;
	bool b = sp.make_map_from_simple(str, tmp);

	if (b)
	{
		Empty();
		*this = tmp;
	}

	return b;
}

