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
 * $Id: test-parser.c,v 1.4 2004/04/19 05:39:15 bsittler Exp $
 **/

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "parser.h"

static unused char rcsid[] = "@(#) $Id: test-parser.c,v 1.4 2004/04/19 05:39:15 bsittler Exp $";

static int expect_id = 0;
static int failed = 0;
static int expected_bytes = 0;

int expects[] = {
    102, 
    3075, 
    1284, 
    25330,
    25330, 
    10111, 
    10112, 
    10113,
    10717,
    10718,
    10114
};

/* each of these is fed as a chunk to the parser. */
char *bufs[] = {
    /* all at once */
    "foo bar baz \n{\nname : \"kn_id\",\nvalue : \"102\"\n}, gork",
    /* split in the string before the kn_id */
    "goog bag \n{\nname : ",
    "\"kn_id\",\nvalue : \"3075\"\n}, znork",
    /* split in the string after the kn_id */
    "blorg \n{\nname : \"kn_id\",\nvalue : \"1284\"",
    "\n},",
    /* split in the middle of the id */
    "\n{\nname : \"kn_id\",\nvalue : \"253",
    "30\"\n},",
    /* split everywhere */
    "\n{\nname : \"kn_id\",\nvalue : \"253",
    "30\"\n},",
    /* several id's at once */
    "\n{\nname : \"kn_id\",\nvalue : \"10111\"\n}, "
    "\n{\nname : \"kn_id\",\nvalue : \"10112\"\n}, glork bronk boog"
    "\n{\nname : \"kn_id\",\nvalue : \"10113\"\n}, glork bronk boog",
    /* pubsub.py */
    "\n{name: \"kn_id\", value: \"10717\"},",
    /* ModPubSub.pm */
    "{name:\"kn_id\",value:\"10718\"}\n"
};

/* this one gets sent in one byte at a time */
char *otherstring = "aroiuwarou jarojraohearjoeaoioijoji joiaoijajoioijoji oji\n{\nname : \"kn_id\",\nvalue : \"10114\"\n}, ow jwoijiowaoihewrjioaewohaeoheaiojrejoaaheio oji ";

#define count(x) (sizeof(x)/sizeof(x[0]))

void stub(void *client_data, int ev_id, int bytes, int conn)
{
    if (expect_id >= count(expects)) {
        fprintf(stderr, "FAILED: got extra event %d\n", ev_id);
        failed = 1;
    } else if (ev_id != expects[expect_id]) {
        fprintf(stderr, "FAILED: got id %d expecting %d\n", ev_id, 
                expects[expect_id]);
        failed = 1;
    } else if (bytes != expected_bytes) {
        fprintf(stderr, "FAILED: got bytes %d expecting %d\n", bytes,
                expected_bytes);
    } else {
        expect_id ++;
    }
}

int main() {
    parser_t *p = new_simple_parser(stub, 0, 0);
    int i;

    for (i = 0; i < count(bufs); i++) {
        expected_bytes += strlen(bufs[i]);
        parse_data(p, 0, bufs[i], strlen(bufs[i]));
    }
    for (i = 0; i < strlen(otherstring); i++) {
        expected_bytes ++;
        parse_data(p, 0, otherstring + i, 1);
    }
    if (expect_id != count(expects)) {
        fprintf(stderr, "FAILED: missed last events (%ld of %ld)\n",
                (long int) (count(expects) - expect_id),
                (long int) (count(expects)));
    }
    return failed;
}
