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

#if !defined(LIBKN_HTTPPARAMSTRING_H)
#define LIBKN_HTTPPARAMSTRING_H

#include <LibKN\Defs.h>

/**
 * Utility class for building encoded http post payload.
 */
class HttpParamString : public string
{
public:
	/**
	 * Constructor to create an empty HttpParamString.
	 */
	HttpParamString();
	~HttpParamString();

	/**
	 * Clear this object, making it ready for reuse.
	 */
	void Clear();

	/**
	 * Add parameters to this string.
	 */
	bool AddPostParam(const wstring& name, const wstring& value);

private:
	const string utf8_encode(const wstring& str_to_encode);
	const string url_encode(const string& str_to_encode);

	char* m_EncodeBuf;
	unsigned long m_BufLen;
};

#endif
