/**
 * Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
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
 **/

/* $Id: ev_pubsub.h,v 1.2 2004/04/19 05:39:15 bsittler Exp $ */

#include "evdispatch.h"

/* Create a channel which listens for events on a given topic,
 * sending them back in javascript format on a given file descriptor.
 * 
 * Events that arrived within max_age seconds ago should be queued for
 * delivery (though max_age == 0 is the only supported mode right now).
 *
 * Other parameters are cookies of various kinds which are copied into
 * the JS output at different points --- inelegant, but so is the concept
 * of JS event delivery.
 */

int create_kn_channel (int fd, char *topic, int max_age, char *callback,
		       char *init, char *onload, char *onerror,
		       char *last_posn);

/* Destroy a channel created by create_kn_channel */

void destroy_kn_channel (ev_channel *chan);

/* Schedule "keepalive" output to kn channels --- another total kludge.
 * Should be scheduled at least every ten seconds or so.
 */

int schedule_kn_keepalives (void);

/* This routine queues events which have happened since the last time
 * it was called, returning the number of connections which have new
 * data to send.
 */
int process_kn_events(void);

/* General initialization */

void init_pubsub_evstreams (char *ev_log_name, char *ev_dir_name);


