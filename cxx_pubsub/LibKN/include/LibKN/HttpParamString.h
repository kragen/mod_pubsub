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

#if !defined(LIBKN_HTTPPARAMSTRING_H)
#define LIBKN_HTTPPARAMSTRING_H

#include <LibKN\Defs.h>

/**
 * Base class for building encoded messages.
 */
class BaseEncodingString : public string
{
public:
	BaseEncodingString();
	virtual ~BaseEncodingString();

	virtual bool AddParam(const wstring& name, const wstring& value) = 0;

protected:
	bool AddParamImpl(const wstring& name, const wstring& value, const string& fvSep, const string& evtSep);

	const string utf8_encode(const wstring& str_to_encode);
	const string url_encode(const string& str_to_encode);
};

/**
 * Utility class for building encoded http post payload.
 */
class HttpParamString : public BaseEncodingString
{
public:
	/**
	 * Constructor to create an empty HttpParamString.
	 */
	HttpParamString();
	~HttpParamString();

	/**
	 * Add parameters to this string.
	 */
	bool AddParam(const wstring& name, const wstring& value);
};

class SimpleString : public BaseEncodingString
{
public:
	SimpleString();
	~SimpleString();

	/**
	 * Add parameters in simple format.
	 */
	bool AddParam(const wstring& name, const wstring& value);
};

#endif
