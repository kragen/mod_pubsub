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

#if !defined(LIBKN_SIMPLEPARSER_H)
#define LIBKN_SIMPLEPARSER_H

#include <LibKN\Defs.h>
#include <LibKN\Tunnel.h>
#include <LibKN\Message.h>

class Connector;

/**
 * A class for parsing the events coming off the transport layer.
 */
class SimpleParser  
{
public:
	/**
	 * The constructor needs a callback for the events.
	 */
	SimpleParser(Connector* connector);
	virtual ~SimpleParser();

	/**
	 * Pass in the raw data from the simple tunnel format for parsing.
	 */
	mb_buf_ptr parse(mb_buf_ptr buf_ptr);

	/**
	 * Build a stringified ParamMap from the simple tunnel format event.
	 */
	bool make_map_from_simple(const string& simple, Message& param_map);

private:
	// parse for a length
	mb_buf_ptr parse_len(mb_buf_ptr buf_ptr);
	
	// once a length is found, parse out the rest of the event
	mb_buf_ptr parse_event(mb_buf_ptr buf_ptr);

	// once an event is complete fire the event on the CKNServer object
	bool fire_event();

	// decodes a header string of an event
	wstring decode_simple_header_string(string& str_to_decode);

	// decodes the payload of an event
	wstring decode_simple_payload_string(string& str_to_decode);

	// the length string that we find
	string len_string;

	// the lenght in bytes of the event to be read
	unsigned int len;

	// are we in the middle of filling an event
	bool len_found;

	// a place to hold the event
	string event_str;

	// pointer to the server object that events will be posted to
	Connector* m_Connector;

private:
	SimpleParser& operator=(const SimpleParser& rhs);
};

#endif
