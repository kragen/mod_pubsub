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
#include <LibKN\IRequestStatusHandler.h>
#include <LibKN\Logger.h>

const TCHAR* const StatusCodeField = _T("status_code");

void IRequestStatusHandler::OnStatus(const Message& msg)
{
	wstring v;

	if (!msg.Get(StatusCodeField, v))
	{
		OnError(msg);
	}
	else
	{
		if (v.length() == 0)
			OnError(msg);

		int status_code = 0;

		if (swscanf(v.c_str(), L"%d", &status_code) != 1)
			status_code = 0;

		if (status_code == 0)
		{
			OnSuccess(msg);
		}
		else if (HTTP_STATUS_FIRST <= status_code && status_code <= HTTP_STATUS_LAST)
		{
			if (status_code < 300)
				OnSuccess(msg);
			else
				OnError(msg);
		}
		else
			OnError(msg);
	}
}

void IRequestStatusHandler::OnError(const Message& msg)
{
}

void IRequestStatusHandler::OnSuccess(const Message& msg)
{
}


