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
 * $Id: master.h,v 1.3 2004/04/19 05:39:15 bsittler Exp $
 **/

#ifndef __MASTER_H
#define __MASTER_H

typedef struct masterparms {
    int origin_topics;
    int journals_per_topic;
    int tunnels_per_journal;
    char *basetopic;
    float requests_per_sec;
    char *servername;
    int port;
    float runfor;
    int debug;
    int payloadsize;
    int simple_format;  /* boolean: use simple format? */
    int batchsize;  /* zero for no batching, otherwise a number of events */
    int max_opening_conns;
    float conns_per_sec;
} masterparms;

void run_master(masterparms *parms);

/* call this with your argc and argv to get them parsed as perftool
 * would.  It will print an error message and exit if it can't parse
 * it or return true (if master) or false (if slave) otherwise.  Not
 * an elegant interface. */
int parse_masterparms(int argc, char **argv, struct masterparms *parms);

/* and if you link that in, you'd better define this */
void add_slave(char *name);

#endif
