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

#if !defined(LIBKN_OFFLINEQ_H)
#define LIBKN_OFFLINEQ_H

#include <LibKN\Defs.h>
#include <LibKN\IOfflineQueue.h>

class OfflineQueue : public IOfflineQueue
{
typedef queue<string> Container;
friend class Connector;

public:
	OfflineQueue();
	~OfflineQueue();

	bool GetQueueing();
	void SetQueueing(bool on);

	bool SaveQueue(const wstring& filename);
	bool LoadQueue(const wstring& filename);
	bool Flush();
	bool Clear();
	bool HasItems();

private:
	void Add(const string& data);
	bool FlushImp(Connector* c);

private:
	bool m_On;
	Container m_Queue;

private:
	OfflineQueue& operator=(const OfflineQueue& rhs);
};

#endif
