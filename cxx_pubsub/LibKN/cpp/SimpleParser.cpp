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
#include <LibKN\SimpleParser.h>
#include <LibKN\Connector.h>
#include <stdlib.h>
#include <stdio.h>

SimpleParser::SimpleParser(Connector* connector) : 
	m_Connector(connector),
	len_found(false),
	w_buf_size(0),
	w_buf(0)
{
	//a buffer to hold the converted wide char
	w_buf_size = 512;
	w_buf = new wchar_t[w_buf_size];
}

SimpleParser::~SimpleParser()
{
	if (w_buf)
		delete[] w_buf;
}

/** @param buf_ptr the raw data from the simple tunnel connection
 *
 * Parses a journal subcription using simple tunnel format and
 * fires Events on the specified CKNServer object as they are
 * found.  Data is passed in in an mb_buf_ptr structure.  The object
 * copies any data it needs from the buffer.  It returns an 
 * mb_buf_ptr struct that points to the unparsed portion of the buffer.
 * it should be called again with the return value until the returned 
 * mb_buf_ptr.len == 0.
 *
 */
mb_buf_ptr SimpleParser::parse(mb_buf_ptr buf_ptr)
{
	//if it is not found, look for the length first
	if (!len_found)
		buf_ptr = parse_len(buf_ptr);

	if (len_found)
		buf_ptr = parse_event(buf_ptr);

	//return wherever we have gotten to in the buffer
	return buf_ptr;
}

mb_buf_ptr SimpleParser::parse_len(mb_buf_ptr buf_ptr)
{
	char* ch_ptr = (char*)buf_ptr.m_Ptr;
	for (unsigned int i = 0; (i < buf_ptr.m_Len && !len_found); ++i, ++ch_ptr)
	{
		//collect number characters
		if (*ch_ptr >= '0' && *ch_ptr <= '9')
		{
			len_string += *ch_ptr;
		}
		else if (*ch_ptr == '\n')
		{
			len = atoi(len_string.c_str());
			len_found = true;
			len_string.erase();
		}
	}

	buf_ptr.m_Ptr = (unsigned char*)ch_ptr;
	buf_ptr.m_Len -= i; 

	return buf_ptr;
}

mb_buf_ptr SimpleParser::parse_event(mb_buf_ptr buf_ptr)
{
	if (len == 0)
	{
		//try again
		len_found = false;
		return buf_ptr;
	}

	//just keep moving bytes into the event buffer until it reaches len number of bytes
	int num_bytes_to_copy = (buf_ptr.m_Len > len - event_str.length()) ? len - event_str.length() : buf_ptr.m_Len;
	event_str.append((char*)buf_ptr.m_Ptr, num_bytes_to_copy);

	if (event_str.length() == len)
		fire_event();

	//get rid of what we have used
	buf_ptr.m_Ptr += num_bytes_to_copy;
	buf_ptr.m_Len -= num_bytes_to_copy;

	return buf_ptr;
}

bool SimpleParser::fire_event()
{
	Message msg;
	make_map_from_simple(event_str, msg);

	//reliablity check
	if (m_Connector->GetRandR().EventCheck(msg))
	{
		//fire the event
		m_Connector->OnFireEvent(msg);
	}

	//clear things for the next round
	event_str.erase();
	len_found = false;

	return true;
}

//convert the simple tunnel fram format, already has the length take off by the event parser
bool SimpleParser::make_map_from_simple(const string& simple, Message& msg)
{
	string name;
	string value;

	wstring decoded_name;
	wstring decoded_value;

	string::size_type str_pos = 0;
	string::size_type mid_pos = 0;
	string::size_type end_pos = 0;

	string::size_type payload_pos = simple.find("\n\n");

	while (string::npos != (mid_pos =  simple.find(": ", str_pos)) && str_pos < payload_pos)
	{
		//look for the end
		end_pos = simple.find("\n", mid_pos);
		if (end_pos == string::npos)
		{
			//big trouble
			return false;
		}

		name = simple.substr(str_pos, mid_pos - str_pos);
		value = simple.substr(mid_pos + 2, end_pos - (mid_pos + 2));

		decoded_name = decode_simple_header_string(name);
		decoded_value = decode_simple_header_string(value);

		if (decoded_name.length() == 0)
			return false;

		msg.Set(decoded_name, decoded_value);

		str_pos = end_pos + 1;
	}

	mid_pos = simple.find("\n", str_pos);
	if (mid_pos == wstring::npos)
	{
		//no payload!?
		msg.Set("kn_payload", "");
	}
	else
	{
		msg.Set(L"kn_payload", decode_simple_payload_string(simple.substr(mid_pos + 1, (simple.length()) - (mid_pos + 1))));
	}

	return true;
}

wstring SimpleParser::decode_simple_header_string(string& str_to_decode)
{
	wstring ret_str;
	ret_str = L"";

	//find encoded characters
	string number;
	string::size_type str_pos = 0;

	int byte = 0;
	unsigned char new_char;

	while (string::npos != (str_pos = str_to_decode.find("=", str_pos)))
	{

		if (sscanf(str_to_decode.substr(str_pos, 3).c_str(), "=%x", &byte))
		{
			new_char = byte;
			//replace with the new stuff
			str_to_decode.replace(str_pos, 3, 1, new_char);
		}

		++str_pos;
	}

	//convert to wide string 
	int w_buf_len;
	if (!(w_buf_len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str_to_decode.c_str(), str_to_decode.length(), &w_buf[0], w_buf_size)))
	{
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError ())
		{
			int new_w_buf_size = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str_to_decode.c_str(), str_to_decode.length(), NULL, 0);
			delete[] w_buf;
			w_buf = new wchar_t[new_w_buf_size];
			w_buf_size = new_w_buf_size;

			if (!(w_buf_len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str_to_decode.c_str(), str_to_decode.length(), &w_buf[0], w_buf_size)))
				return ret_str;
		}
		else
			return ret_str;
	}

	ret_str.assign(w_buf, w_buf_len);

	return ret_str;
}

wstring SimpleParser::decode_simple_payload_string(string& str_to_decode)
{
	wstring ret_str;

	//convert to wide string 
	int w_buf_len;
	if (!(w_buf_len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str_to_decode.c_str(), str_to_decode.length(), &w_buf[0], w_buf_size)))
	{
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError ())
		{
			int new_w_buf_size = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str_to_decode.c_str(), str_to_decode.length(), NULL, 0);
			delete[] w_buf;
			w_buf = new wchar_t[new_w_buf_size];
			w_buf_size = new_w_buf_size;

			if (!(w_buf_len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str_to_decode.c_str(), str_to_decode.length(), &w_buf[0], w_buf_size)))
				return ret_str;
		}
		else
			return ret_str;
	}

	ret_str.assign(w_buf, w_buf_len);

	return ret_str;
}
