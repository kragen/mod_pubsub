/**
 * Copyright (c) 2001-2003 KnowNow, Inc.  All rights reserved.
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
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * 
 * 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
 * be used to endorse or promote any product without prior written
 * permission from KnowNow, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 *
 * $Id: parser.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

#include <stdlib.h>
#include "util.h"
#include "dstring.h"
#include "parser.h"

static unused char rcsid[] = "@(#) $Id: parser.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $";

#define PARSER_MAGIC 0x3dbeef
struct parser {
    int magic;
    dstring *parsebuf;
    int bytes;
    void *client_data;
    void (*on_event)(void *client_data, int ev_id, int bytes, int conn);
    int simple_format;
};

/* this string always occurs before a non-Unicode kn_id value in the
 * 1.1.7 server*/
char *js_kn_id_finder =     "\n{\nname: \"kn_id\",\nvalue: \"";
/* but it's this string in the 1.2.x server */
char *new_js_kn_id_finder = "\n{\nname : \"kn_id\",\nvalue : \"";
/* and this one after */
char *js_kn_id_end_finder = "\"\n}";

/* these strings are from the simple format */
char *simple_kn_id_finder = "\nkn_id: ";
char *simple_kn_id_end_finder = "\n";

/* FIXME: this routine is really ugly and nasty */
void parse_data(parser_t *parser, int conn, char *data, int len)
{
    int lastindex = 0, index;
    int endindex;
    char *kn_id_finder, *new_kn_id_finder, *kn_id_end_finder;
    int finderlen, newfinderlen;
    int maxremaining;
    int start_offset;
    dstring *buf = parser->parsebuf;

    if (parser->simple_format)
    {
        kn_id_finder = simple_kn_id_finder;
        new_kn_id_finder = simple_kn_id_finder;
        kn_id_end_finder = simple_kn_id_end_finder;
    }
    else
    {
        kn_id_finder = js_kn_id_finder;
        new_kn_id_finder = new_js_kn_id_finder;
        kn_id_end_finder = simple_kn_id_end_finder;
    }

    finderlen = strlen(kn_id_finder);
    newfinderlen = strlen(new_kn_id_finder);

    /* maximum number of characters to leave from the end of the
     * buffer in case a partial kn_id_finder is left there. */
    maxremaining = (finderlen > newfinderlen ? 
                    finderlen : 
                    newfinderlen) - 1;
    if (parser->magic != PARSER_MAGIC) abort();

    parser->bytes += len;
    if (!dstring_append_cstr(buf, data, len)) abort();
    for (;;) {
        index = dstring_find_s(buf, lastindex, kn_id_finder);
        /* look for new_kn_id_finder and use that instead if it's
         * there and kn_id_finder is not.  The assumption is that they
         * will never both be in the same buffer (or, in the simple
         * format, that they are the same). */
        if (index != -1) {
            start_offset = finderlen;
        } else {
            index = dstring_find_s(buf, lastindex, new_kn_id_finder);
            start_offset = newfinderlen;
        }
    if (index == -1) break;
        endindex = dstring_find_s(buf, index + start_offset, kn_id_end_finder);
        if (endindex == -1) {
            /* the buffer ends in the middle of the kn_id */
            if (!dstring_shift(buf, index)) abort();
            return;
        }

        parser->on_event(parser->client_data, 
                         atoi(buf->start + index + start_offset),
                         parser->bytes, conn);
        lastindex = endindex + strlen(kn_id_end_finder);
    }
    /* at this point, we know that the buffer contains neither
     * kn_id_finder nor new_kn_id_finder, except possibly before
     * 'lastindex'.  But it might contain a proper prefix of one of
     * them.  So we shift out at least 'lastindex' characters, leaving
     * a maximum of the longest possible proper prefix of one of
     * these. */
    if (lastindex < buf->len - maxremaining) 
        lastindex = buf->len - maxremaining;
    if (!dstring_shift(buf, lastindex)) abort();
}

parser_t *new_simple_parser(parser_event_handler on_event, void *client_data,
                            int simple_format)
{
    parser_t *rv = (parser_t*)malloc(sizeof(parser_t));
    rv->parsebuf = new_dstring();
    if (!rv->parsebuf) {
        free(rv);
        return NULL;
    }
    rv->magic = PARSER_MAGIC;
    rv->bytes = 0;
    rv->on_event = on_event;
    rv->client_data = client_data;
    rv->simple_format = simple_format;
    return rv;
}

void parser_free(parser_t *parser)
{
    dstring_free(parser->parsebuf);
    free(parser);
}
