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
#include <LibKN\StrUtil.h>
#include <LibKN\Logger.h>

BaseEncodingString::BaseEncodingString() 
{
}

BaseEncodingString::~BaseEncodingString()
{
}

bool BaseEncodingString::AddParamImpl(const wstring& name, const wstring& value, const string& fvSep, const string& evtSep)
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
	append(fvSep);
	append(encoded_value);
	append(evtSep);

	return true;
}

const string BaseEncodingString::utf8_encode(const wstring& str_to_encode)
{
	return ConvertToUtf8(str_to_encode);
}

bool is_safe(unsigned char aCharacter)
{
	return isalnum(aCharacter) ||
		(aCharacter == '-' ) || 
		(aCharacter == '_' ) || 
		(aCharacter == '.' ) || 
		(aCharacter == '!' ) || 
		(aCharacter == '~' ) || 
		(aCharacter == '*' ) || 
		(aCharacter == '\'') || 
		(aCharacter == '(' ) || 
		(aCharacter == ')' );
}

const string BaseEncodingString::url_encode(const string& aString)
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



HttpParamString::HttpParamString() :
	BaseEncodingString()
{
}

HttpParamString::~HttpParamString()
{
}

bool HttpParamString::AddParam(const wstring& name, const wstring& value)
{
	AddParamImpl(name, value, "=", ";");
	return true;
}



SimpleString::SimpleString() :
	BaseEncodingString()
{
}

SimpleString::~SimpleString()
{
}

bool SimpleString::AddParam(const wstring& name, const wstring& value)
{
	AddParamImpl(name, value, ": ", "\n");
	return true;
}


