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

#if !defined(LIBKN_IREQUESTSTATUSHANDLER_H)
#define LIBKN_IREQUESTSTATUSHANDLER_H

#include <LibKN\IStatusHandler.h>

/**
 * Interface for getting various request statuses.
 * All interested parties must provide implementations of the pure virtual methods.
 */
class IRequestStatusHandler : public IStatusHandlerBase
{
public:
	/**
	 * This method gets called whenever the server returns a status.
	 * The default implementation calls either OnSuccess or OnError.
	 * Override this implementation to do something different.
	 */
	virtual void OnStatus(const Message& msg);

	/**
	 * This method gets called whenever there is an error from the server.
	 * \param msg The error as a message.
	 */
	virtual void OnError(const Message& msg);

	/**
	 * This method gets called whenever the status is successful from the server.
	 * \param msg The status as a message.
	 */
	virtual void OnSuccess(const Message& msg);
};

extern const TCHAR* const StatusCodeField;

#endif
