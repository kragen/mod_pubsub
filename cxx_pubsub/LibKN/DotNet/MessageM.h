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

#if !defined(MESSAGEM_H)
#define MESSAGEM_H

using namespace System;
using namespace System::Collections;

#include <LibKN\Message.h>

namespace LibKNDotNet
{
	public __gc struct MessageEntry
	{
		String* Field;
		String* Value;
	};

	public __gc class Message : public IEnumerable
	{
	public:
		Message();
		Message(const ::Message& rhs);
		~Message();

		void Copy(Message* rhs);
		void CopyImpl(const ::Message& rhs);
		bool IsEqual(Message* rhs);

		void Empty();
		bool IsEmpty();

		void Set(String* f, String* v);
		void Remove(String* f);
		bool Get(String* f, String** v);

		String* GetAsSimpleFormat();
		bool InitFromSimple(String* str);

		IEnumerator* GetEnumerator();

		::Message* GetImpl();

	private:
		::Message* m_MessageImpl;
		CCriticalSection* m_CS;
		typedef LockImpl<CCriticalSection> Lock;
	};

	__gc class MessageEnumerator : public IEnumerator
	{
	public:
		MessageEnumerator(Message* m);

		Object* get_Current();
		void Reset();
		bool MoveNext();

	private:
		Message* m_Message;
		::Message::Container::const_iterator* m_Iter;
	};
}

#endif
