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
#include "RequestStatusHandler.h"
#include "StringToChar.h"

namespace LibKNDotNet
{
IRequestStatusHandler::IRequestStatusHandler()
{
}

IRequestStatusHandler::~IRequestStatusHandler()
{
}

void IRequestStatusHandler::OnStatus(Message* msg)
{
	String* v = 0;
	String* f = new String(StatusCodeField);

	if (!msg->Get(f, &v))
	{
		OnError(msg);
	}
	else
	{
		if (v->Length == 0)
			OnError(msg);

		int status_code = 0;
		StringToWChar* _v = new StringToWChar(v);
		
		if (swscanf(_v->GetChar(), L"%d", &status_code) != 1)
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

void IRequestStatusHandler::OnError(Message* msg)
{

}

void IRequestStatusHandler::OnSuccess(Message* msg)
{

}
}


