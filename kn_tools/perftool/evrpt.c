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
 * $Id: evrpt.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "dstring.h"
#include "dstr_interp.h"
#include "pipefit.h"
#include "doubletime.h"

static unused char rcsid[] = "@(#) $Id: evrpt.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $";

/* code to report on the arrival of events */

/* probably take 32 bytes per record, including malloc overhead and padding.
 * That's 1K per 32 events sent.  Since we never free these records, this 
 * could be a significant memory user --- one meg per 32768 events.
 */
typedef struct event_timestamp {
    struct event_timestamp *next;
    int serial;
    double when_sent;
} event_timestamp;

static event_timestamp *events = NULL;

void event_was_just_sent(int serial)
{
    event_timestamp *new_event = (event_timestamp*)malloc(sizeof(*new_event));
    if (!new_event) abort();
    new_event->next = events;
    new_event->serial = serial;
    new_event->when_sent = gettimeofday_double();
    events = new_event;
}

/* on x86, minimal path length of this routine is 12 insns --- that's
 * when the event being retrieved was the most recent one; 17 or 22
 * insns if it's the previous one or the one before
 * that. -fomit-frame-pointer saves two insns. */
double when_event_was_sent(int serial)
{
    event_timestamp *evtp = events;
    while (evtp) {
        if (evtp->serial == serial) return evtp->when_sent;
        evtp = evtp->next;
    }
    fprintf(stderr, "Couldn't find event serial number %d\n", serial);
    return 0.0; /* abort(); */
}

void report_event(int serial, int bytes, int conn, pipe_fitting_t *output)
{
    double now = gettimeofday_double();  
    double when_sent;
    double latency;
    dstring *outstr;
    when_sent = when_event_was_sent(serial);
    latency = now - when_sent;
    outstr = dstring_interp_s("id: %d received: %f latency: %g"
                              " conn: %d bytes: %d\n",
                           serial, now, latency, conn, bytes);
    if (!outstr) abort();  /* what else can we do? */
    if (!pipe_fitting_write(output, outstr->start, outstr->len)) abort();
    dstring_free(outstr);
}
