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
#include <LibKN\HttpParamString.h>

HttpParamString::HttpParamString() :
	m_EncodeBuf(0)
{
	Clear();
}

HttpParamString::~HttpParamString()
{
	if (m_EncodeBuf)
		delete[] m_EncodeBuf;
}

void HttpParamString::Clear()
{
	if (m_EncodeBuf)
		delete[] m_EncodeBuf;

	m_BufLen = 512;
	m_EncodeBuf = new char[m_BufLen];
}

bool HttpParamString::AddPostParam(const wstring& name, const wstring& value)
{
	//convert to utf8 
	string encoded_name(utf8_encode(name));
	
	if (encoded_name.length() == 0)
		return false;

	//encode the name
	encoded_name = url_encode(encoded_name);

	//convert to utf8 and decode the value
	string encoded_value(utf8_encode(value));
	if (encoded_value.length() == 0 && value.length() != 0)
		return false;

	//encode the value
	encoded_value = url_encode(encoded_value);

	append(encoded_name);
	append("=");
	append(encoded_value);
	append(";");

	return true;
}

const string HttpParamString::utf8_encode(const wstring& str_to_encode)
{
	string ret_str("");
	int str_len;
	if (!(str_len = WideCharToMultiByte(CP_UTF8, 0, str_to_encode.c_str(), str_to_encode.length(), m_EncodeBuf, m_BufLen, NULL, NULL)))
	{
		//check if the buffer was too short
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			//get the required lenght
			int temp_len = WideCharToMultiByte(CP_UTF8, 0, str_to_encode.c_str(), str_to_encode.length(), NULL, 0, NULL, NULL);
			if (temp_len == 0)
				return ret_str;

			delete[] m_EncodeBuf;

			m_EncodeBuf = new char[temp_len];
			m_BufLen = temp_len;

			if (!(str_len = WideCharToMultiByte(CP_UTF8, 0, str_to_encode.c_str(), str_to_encode.length(), m_EncodeBuf, m_BufLen, NULL, NULL)))
				return ret_str;
		}
		else
			return ret_str;
	}

	return ret_str.assign(m_EncodeBuf, str_len);
}

#define is_safe(aCharacter)       \
	( isalnum(aCharacter)  || \
	  (aCharacter == '-' ) || \
	  (aCharacter == '_' ) || \
	  (aCharacter == '.' ) || \
	  (aCharacter == '!' ) || \
	  (aCharacter == '~' ) || \
	  (aCharacter == '*' ) || \
	  (aCharacter == '\'') || \
	  (aCharacter == '(' ) || \
	  (aCharacter == ')' ) )

const string HttpParamString::url_encode(const string& aString)
{
	const char anEscapeCharacter = '%';
	const char*  anInputBytes        = aString.c_str();
	size_t       anInputBytesLength  = aString.length();
	size_t       anOutputBytesLength = 0;
	size_t       anIterator;

	/* Calculate length of encoded string. */
	for (anIterator = 0; anIterator < anInputBytesLength; anIterator++)
	{
		unsigned char aCharacter = (unsigned char)anInputBytes[anIterator];

		/* Quoted characters take 3 bytes; others take 1. */
		if (is_safe(aCharacter))
			anOutputBytesLength += 1;
		else
			anOutputBytesLength	+= 3;
	}

	string aResult;
	aResult.reserve(anOutputBytesLength + 2);

	for (anIterator = 0; anIterator < anInputBytesLength; anIterator++)
	{
		unsigned char aCharacter = (unsigned char)anInputBytes[anIterator];

		if (is_safe(aCharacter))
		{
			aResult += aCharacter;
		}
		else
		{
			char aHex[4];
			sprintf(aHex, "%c%02X", anEscapeCharacter, aCharacter);
			aResult += aHex;
		}
	}

	return aResult;
}

