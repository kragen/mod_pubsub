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

#if !defined(BRIDGES_H)
#define BRIDGES_H

#include <list>

//
// T signifies the c++ class
//
template <class T>
class TBridgeCollection
{
	typedef std::list<T> Collection;

public:
	TBridgeCollection()
	{
	}
	
	~TBridgeCollection()
	{
		Clear();
	}

	T Add(T i)
	{
		m_Bridges.push_back(i);
		return i;
	}

	T Find(T t)
	{
		for (Collection::iterator it = m_Bridges.begin(); it != m_Bridges.end(); it++)
		{
			T i = *it;
			if (i->GetHandler() == t->GetHandler())
				return i;
		}

		return 0;
	}

	T Remove(T t)
	{
		for (Collection::iterator it = m_Bridges.begin(); it != m_Bridges.end(); it++)
		{
			T i = *it;
			if (i->GetHandler() == t->GetHandler())
			{
				m_Bridges.erase(it);
				return i;
			}
		}

		return 0;
	}

	void Clear()
	{
		for (Collection::iterator it = m_Bridges.begin(); it != m_Bridges.end(); it++)
		{
			T i = *it;
			delete i;
		}
		
		m_Bridges.clear();
	}

private:
	// Not implemented to prevent class from being copied.
	//
	TBridgeCollection& operator=(const TBridgeCollection& rhs);

	Collection m_Bridges;
};

#endif
