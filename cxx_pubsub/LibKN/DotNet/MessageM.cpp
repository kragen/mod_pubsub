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
#include "MessageM.h"
#include "StringToChar.h"

namespace LibKNDotNet
{

Message::Message()
{
	m_MessageImpl = new ::Message();
	m_CS = new CCriticalSection();
}

Message::Message(const ::Message& rhs)
{
	m_MessageImpl = new ::Message(rhs);
	m_CS = new CCriticalSection();
}

Message::~Message()
{
	delete m_MessageImpl;
	delete m_CS;
}

void Message::Copy(Message* rhs)
{
	Lock autoLock(m_CS);
	*m_MessageImpl = *(rhs->m_MessageImpl);
}

void Message::CopyImpl(const ::Message& rhs)
{
	Lock autoLock(m_CS);
	*m_MessageImpl = rhs;
}

bool Message::IsEqual(Message* rhs)
{
	return *m_MessageImpl == *(rhs->m_MessageImpl);
}

void Message::Empty()
{
	m_MessageImpl->Empty();
}

void Message::Set(String* f, String* v)
{
	StringToWChar* _f = new StringToWChar(f);
	StringToWChar* _v = new StringToWChar(v);

	m_MessageImpl->Set(_f->GetChar(), _v->GetChar());
}

void Message::Remove(String* f)
{
	StringToWChar* _f = new StringToWChar(f);
	m_MessageImpl->Remove(_f->GetChar());
}

bool Message::Get(String* f, String** v)
{
	StringToWChar* _f = new StringToWChar(f);
	wstring _v;

	bool retVal = m_MessageImpl->Get(_f->GetChar(), _v);

	if (retVal)
	{
		*v = new String(_v.c_str());
	}

	return retVal;
}

bool Message::IsEmpty()
{
	return m_MessageImpl->IsEmpty();
}

String* Message::GetAsSimpleFormat()
{
	string s = m_MessageImpl->GetAsSimpleFormat();
	return new String(s.c_str());
}

bool Message::InitFromSimple(String* str)
{
	StringToChar* _str = new StringToChar(str);
	string s(_str->GetChar());
	bool retVal = m_MessageImpl->InitFromSimple(s);
	return retVal;
}

::Message* Message::GetImpl()
{
	return m_MessageImpl;
}

IEnumerator* Message::GetEnumerator()
{
	return new MessageEnumerator(this);
}

MessageEnumerator::MessageEnumerator(Message* m) :
	m_Message(m),
	m_Iter(0)
{
}

Object* MessageEnumerator::get_Current()
{
	if (m_Iter == 0)
		throw new InvalidOperationException();

	if ((*m_Iter) == m_Message->GetImpl()->GetContainer().end())
		throw new InvalidOperationException();

	MessageEntry* me = new MessageEntry();
	me->Field = new String((*(*m_Iter)).first.c_str());
	me->Value = new String((*(*m_Iter)).second.c_str());

	return me;
}

void MessageEnumerator::Reset()
{
	if (m_Iter)
	{
		delete m_Iter;
		m_Iter = 0;
	}
}

bool MessageEnumerator::MoveNext()
{
	if (m_Message->GetImpl()->GetContainer().empty())
		return false;

	if (m_Iter == 0)
	{
		m_Iter = new ::Message::Container::const_iterator();
		*m_Iter = m_Message->GetImpl()->GetContainer().begin();
		return true;
	}

	(*m_Iter)++;

	bool retVal = (*m_Iter) != m_Message->GetImpl()->GetContainer().end();

	return retVal;
}

}


