/**
 * pubsub_ping - send events to a PubSub Server
 **/

/**
 * Copyright (c) 2000-2003 KnowNow, Inc.  All rights reserved.
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
 * $Id: _pubsub_ping.h,v 1.1 2003/03/22 06:59:18 ifindkarma Exp $
 **/

#ifndef _PUBSUB_PING_H_INCLUDED
#define _PUBSUB_PING_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PUBSUB_PING_INTERNAL

/*** INTERNAL ***/

/* (it's OK to use <kn/kn.h> stuff here) */

/* we only correctly handle round trips inside a +- SEQ_FIELD/2 window */
#define SEQ_BITS 16
#define SEQ_FIELD (1L << SEQ_BITS)

static int fput_knobj(kn_ObjectRef obj, FILE *stream);

/** PingContext interface **/

/* context for a pubsub_ping session */
typedef struct {
    /* user-supplied and defaulted values */
    PingOptions *options;

    int retval; /* main()-style return value */
    kn_ServerRef server; /* server object */
    kn_RouteRef route; /* subscription */
    int running; /* is pubsub_ping running? */
    int done; /* have we finished? */
    double timer; /* when to send the next ping [only while running] */
    long seq; /* next sequence number to send */
    long last_seq; /* last sequence number received */

    /* statistics */
    struct {
        unsigned long transmitted; /* number of pings sent */
        unsigned long received; /* total number of transmitted pings we get back */
        unsigned long received_ok; /* number of non-duplicate transmitted pings we get back */

        /* round-trip info for all transmitted pings we get back */
        struct {
            double min; /* minimum observed round-trip time */
            double max; /* maximum observed round-trip time */
            double sum; /* sum of observed round-trip times */
            double squared_sum; /* sum of squares of observed round-trip times */
        } rt;
    } stats;

    /* file descriptor sets and handlers */
    int nfds; /* number of used file descriptors */
    fd_set readfds; /* set of file descriptors we check for readability */
    fd_set writefds; /* set of file descriptors we check for writability */
    fd_set exceptfds; /* set of file descriptors we check for exceptions */
    kn_ServerProcessCallback onReadable[FD_SETSIZE]; /* callbacks for readable file descriptors */
    kn_ServerProcessCallback onWritable[FD_SETSIZE]; /* callbacks for writable file descriptors */
    kn_ServerProcessCallback onException[FD_SETSIZE]; /* callbacks for file descriptor exceptions */

    /* dynamically-allocated strings */
    kn_StringRef url; /* server URL */
    kn_StringRef topic; /* topic */
    kn_StringRef payload; /* transmitted payload */
    kn_StringRef expiry; /* event expiration time */
    kn_StringRef waitingFor; /* event ID we're waiting for */
    kn_StringRef sid; /* session ID */
} PingContext;

/* allocator and initializer */
static PingContext *newPingContext(PingOptions *options, char *url, char *topic);

/* deallocator */
static void deletePingContext(PingContext *context);

#endif /* defined(PUBSUB_PING_INTERNAL) */

#ifdef __cplusplus
}
#endif

#endif /* ! defined(_PUBSUB_PING_H_INCLUDED) */
