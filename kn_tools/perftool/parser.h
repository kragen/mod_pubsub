/**
 * Copyright 2001-2004 KnowNow, Inc.  All rights reserved.
 *
 * @KNOWNOW_LICENSE_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the KnowNow, Inc., nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 * 
 *
 * $Id: parser.h,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

#ifndef __PARSER_H
#define __PARSER_H

#include "pipefit.h"

/* This guy has only one purpose in life: to extract kn_ids from
 * javascript-formatted events and call report_event with them. */

typedef struct parser parser_t;

/* use this to instantiate a new parser that calls a callback function
 * for each event */
typedef void (*parser_event_handler)(void *client_data, int id, 
                                     int bytes, int conn);
/* on_event gets called with client_data as its first arguments, the
 * kn_id of the event as id, the number of bytes received on the
 * connection in bytes, and the connection number in 'conn'.
 * simple_format is 1 to tell the parser to "parse" the simple format
 * (quoted-printable RFC-822 messages preceded by byte counts in
 * decimals and followed by newlines), or 0 to tell the parser to
 * "parse" the JavaScript format.*/
parser_t *new_simple_parser(parser_event_handler on_event, void *client_data,
                            int simple_format);
/* Call this with new data to add */
void parse_data(parser_t *parser, int conn, char *data, int len);
/* Call this when you're done with the parser */
void parser_free(parser_t *parser);

#endif
