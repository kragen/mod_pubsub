/**
 * pubsub_ping - send events to a PubSub Server
 **/

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
 *
 * $Id: pubsub_ping.h,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

#ifndef PUBSUB_PING_H_INCLUDED
#define PUBSUB_PING_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* print a string with special & non-printing characters escaped */
extern int fputs_quoted(const char *str, FILE *stream);

/** PingOptions interface **/

/* FIXME: the payload size *really* needs to be specified in
 *        characters (to avoid truncated UTF-8 sequences,) but that
 *        forces us to do UTF-8 decoding. */

/* options for a pubsub_ping run */
typedef struct {
    unsigned long limit; /* number of events to send (0 unlimits) */
    double wait; /* minimum inter-event interval in seconds */
    char *pattern; /* UTF-8 and URL-encoded pattern with which to fill the payload */
    unsigned verbose; /* verbosity level */
    int quiet; /* quiet output flag -- suppresses per-event reporting */
    size_t size; /* payload size in bytes */
    double maxwait; /* maximum inter-event interval in seconds */
} PingOptions;

/* allocator and initializer */
extern PingOptions *newPingOptions(void);

/* deallocator */
extern void deletePingOptions(PingOptions *options);

/* run pubsub_ping with the given options, returning a main()-style status
 * code; url is a string containing the server URL
 * (e.g. "http://mypubsubserver:8000/kn/", and topic is either a string
 * containing the topic URL (e.g. "/who/anonymous/test") or (char *)0,
 * indicating that pubsub_ping() should choose a random topic under
 * /what/ping (e.g. "/what/ping/215345".) */
extern int pubsub_ping(PingOptions *options, char *url, char *topic);

#ifdef __cplusplus
}
#endif

#endif /* ! defined(PUBSUB_PING_H_INCLUDED) */
