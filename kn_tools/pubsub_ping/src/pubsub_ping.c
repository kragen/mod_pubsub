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
 * $Id: pubsub_ping.c,v 1.1 2003/03/22 06:59:18 ifindkarma Exp $
 **/

/* this file is part of the pubsub_ping implementation */
#define PUBSUB_PING_INTERNAL

/* for isprint, etc. */
#include <ctype.h>

/* for malloc() and strod() */
#include <stdlib.h>

/* for stderr, fprintf(), fflush(), sprintf(), etc. */
#include <stdio.h>

/* for bzero() [used by FD_ZERO] and memcpy() */
#include <string.h>

/* for EAGAIN */
#include <errno.h>

/* for signal() */
#include <signal.h>

/* for fd_set and pid_t */
#include <sys/types.h>

/* for gettimeofday() and struct timeval */
#include <sys/time.h>

/* for select() and getpid() */
#include <unistd.h>

/* for floor() and sqrt() */
#include <math.h>

/* for the C PubSub Client Library */
#include <kn/kn.h>

/* external interface */
#include "../include/pubsub_ping.h"

/* internal interface */
#include "../include/_pubsub_ping.h"

/* flag for SIGINT */
static volatile sig_atomic_t whacked;

/* SIGINT handler */
static void
whack(int signum)
{
    whacked = signum;
}

/* print a perror-style error message using prefix and set *retval,
   unless we just got SIGINT */
static void
error_nowhack(char *prefix, int *retval)
{
    if ((errno != EINTR) || ! whacked)
    {
        perror(prefix);
        *retval = 1;
    }
}

/* status event dispatcher -- shouldn't this be part of libkn? */
static void
statusDispatcher(kn_EventRef evt, void *void_ctx,
                 kn_EventHandler onSuccess,
                 kn_EventHandler onError)
{
    PingContext *ctx = (PingContext *) void_ctx;
    kn_StringRef lkn8_status = KNSTR("status");
    kn_StringRef lkn8MyStatus;
    size_t status_len;

    if (! lkn8_status)
    {
        error_nowhack("KNSTR", &(ctx -> retval));
        return;
    }
    if ((! (lkn8MyStatus = kn_EventGetValue(evt, lkn8_status))) ||
        ! (status_len = kn_StringGetSize(lkn8MyStatus)))
    {
        if (ctx -> options -> verbose)
        {
            fprintf(stderr,
                    "%s:%d: warning: malformed status event lacks 'status' header: ",
                    __FILE__,
                    __LINE__);
            fput_knobj(evt, stderr);
            fprintf(stderr, "\n");
        }
        (*onError)(evt, void_ctx);
        return;
    }
    switch (*(kn_StringGetBytes(lkn8MyStatus)))
    {
    case '1':
    case '2':
    case '3':
        (*onSuccess)(evt, void_ctx);
        break;
    default:
        (*onError)(evt, void_ctx);
    }
}

/* what time is it? [should gettimeofday() fail, this would set *retval to 1] */
static double
getTime(int *retval)
{
    struct timeval now_tv;
    double now = 0.0;

    if (gettimeofday(&now_tv, 0))
    {
        error_nowhack("gettimeofday", retval);
    }
    else
    {
        now =
            (double) now_tv . tv_sec +
            1e-6 * (double) now_tv . tv_usec;
    }
    return now;
}

/* forward reference */
static void
doPing(PingContext *ctx);

/* pubsub_ping success handler */
static void
onPingSuccess(kn_EventRef evt, void *void_ctx)
{
    PingContext *ctx = (PingContext *) void_ctx;

    if (ctx -> options -> verbose > 2)
    {
        fprintf(stderr,
                "%s <%p>: got successful status event: ",
                __FUNCTION__,
                ctx);
        fput_knobj(evt, stderr);
        fprintf(stderr, "\n");
    }
}

/* pubsub_ping error handler */

/* FIXME: this closure doesn't know enough to reschedule the next
 *     ping, since it doesn't know whether the status event refers to
 *     the latest outstanding request or to an earlier request which
 *     took more thak *maxwait* seconds to post a status event. Any
 *     solution to this problem must be extremely careful to avoid
 *     an uncontrolled memory leak in the case of status events which
 *     frequently take longer than *maxwait* seconds to appear (i.e.
 *     "flood ping" situations.) Basically, the program would need to
 *     use an arena to allocate all the tracking data structures, and
 *     destroy the arena when destroying the PingContext. */

static void
onPingError(kn_EventRef evt, void *void_ctx)
{
    PingContext *ctx = (PingContext *) void_ctx;
    kn_StringRef lkn8_status = KNSTR("status");
    kn_StringRef lkn8_kn_payload = KNSTR("kn_payload");
    kn_StringRef lkn8MyStatus;
    kn_StringRef lkn8MyPayload;
    
    if ((! lkn8_status) ||
        ! lkn8_kn_payload)
    {
        error_nowhack("KNSTR", &(ctx -> retval));
        return;
    }
    if (ctx -> options -> verbose > 2)
    {
        fprintf(stderr,
                "%s <%p>: got error status event: ",
                __FUNCTION__,
                ctx);
        fput_knobj(evt, stderr);
        fprintf(stderr, "\n");
    }
    fprintf(stderr,
            "Error: failed to publish to ");
    kn_StringWriteToStream(ctx -> topic, stderr);
    fprintf(stderr,
            ": ");
    if ((lkn8MyStatus = kn_EventGetValue(evt, lkn8_status)))
        kn_StringWriteToStream(lkn8MyStatus, stderr);
    else
        fprintf(stderr, "(null)");
    fprintf(stderr,
            "\n");
    if ((lkn8MyPayload = kn_EventGetValue(evt, lkn8_kn_payload)))
        kn_StringWriteToStream(lkn8MyPayload, stderr);
    else
        fprintf(stderr, "(null)");
    fprintf(stderr,
            "\n");
}

/* pubsub_ping status handler */
static void
onPingStatus(kn_EventRef evt, void *void_ctx)
{
    statusDispatcher(evt, void_ctx, &onPingSuccess, &onPingError);
}

/* long enough for a floating-point textual kn_time_t with a prefix */
#define TIME_BUFLEN 45

/* long enough for a decimal sequence number */
#define SEQ_BUFLEN (SEQ_BITS / 3 + 2)

/* send an event */
static void
doPing(PingContext *ctx)
{
    char now_buf[TIME_BUFLEN];
    char seq_buf[SEQ_BUFLEN];
    kn_MutableEventRef evt = 0;
    kn_StringRef lkn8_kn_time_t = 0;
    kn_StringRef lkn8_kn_payload = 0;
    kn_StringRef lkn8_kn_expires = 0;
    kn_StringRef lkn8_kn_id = 0;
    kn_StringRef lkn8_seq = 0;
    kn_StringRef lkn8_sid = 0;
    kn_StringRef lkn8MyTime = 0;
    kn_StringRef lkn8MySeq = 0;
    double now = 0.0;

    if (ctx -> waitingFor)
    {
        kn_Release(ctx -> waitingFor);
        ctx -> waitingFor = 0;
    }
    if (! ctx -> running)
    {
        return;
    }
    if (ctx -> options -> limit &&
        (ctx -> stats . transmitted >= ctx -> options -> limit))
    {
        if (! ctx -> stats . received_ok)
        {
            /* no packet received after *limit* packets sent -- time to give up */
            ctx -> done = 1;
        }
        return;
    }
    now = getTime(&(ctx -> retval));
    if (ctx -> retval)
        return;
    sprintf(now_buf,
            "%f",
            now);
    sprintf(seq_buf,
            "%ld",
            ctx -> seq);
    if ((! (lkn8_kn_time_t = KNSTR("kn_time_t"))) ||
        (! (lkn8_kn_payload = KNSTR("kn_payload"))) ||
        (! (lkn8_kn_id = KNSTR("kn_id"))) ||
        (! (lkn8_seq = KNSTR("seq"))) ||
        (! (lkn8_sid = KNSTR("sid"))) ||
        ! (lkn8_kn_expires = KNSTR("kn_expires")))
    {
        error_nowhack("KNSTR", &(ctx -> retval));
        return;
    }
    if (! (lkn8MyTime = kn_StringCreateWithCString(now_buf)))
    {
        error_nowhack("kn_StringCreateWithCString", &(ctx -> retval));
        return;
    }
    if (! (lkn8MySeq = kn_StringCreateWithCString(seq_buf)))
    {
        error_nowhack("kn_StringCreateWithCString", &(ctx -> retval));
        kn_Release(lkn8MyTime);
        return;
    }
    ctx -> seq = (ctx -> seq + 1L) % SEQ_FIELD;
    if ((evt = kn_EventCreateMutable(5)))
    {
        if (kn_EventSetValue(evt, lkn8_kn_time_t, lkn8MyTime) != kn_SUCCESS ||
            kn_EventSetValue(evt, lkn8_seq, lkn8MySeq) != kn_SUCCESS ||
            kn_EventSetValue(evt, lkn8_kn_payload, ctx -> payload) != kn_SUCCESS ||
            kn_EventSetValue(evt, lkn8_kn_expires, ctx -> expiry) != kn_SUCCESS ||
            kn_EventSetValue(evt, lkn8_sid, ctx -> sid) != kn_SUCCESS)
        {
            error_nowhack("kn_EventSetValue", &(ctx -> retval));
        }
    }
    kn_Release(lkn8MySeq);
    kn_Release(lkn8MyTime);
    if (! evt)
    {
        error_nowhack("kn_EventCreateMutable", &(ctx -> retval));
        return;
    }
    if (! ctx -> retval)
    {
        if (ctx -> options -> verbose > 2)
        {
            fprintf(stderr,
                    "%s <%p>: about to send event: ",
                    __FUNCTION__,
                    ctx);
            fput_knobj(evt, stderr);
            fprintf(stderr, "\n");
        }
        if (! (ctx -> waitingFor = kn_EventGetValue(evt, lkn8_kn_id)))
        {
            fprintf(stderr,
                    "%s: client library did not provide an event ID\n",
                    __FUNCTION__);
            exit(1);
        };
        kn_Retain(ctx -> waitingFor);
        ctx -> stats . transmitted ++;
        ctx -> timer =
            getTime(&(ctx -> retval)) +
            ctx -> options -> maxwait;
        if (kn_ServerPublishEventToTopic(
            ctx -> server,
            evt,
            ctx -> topic,
            onPingStatus,
            ctx,
            kn_FALSE))
        {
            double later;
            double rtt;

	    if (! whacked)
	      {
		fprintf(stderr,
			"Error: failed to publish to ");
		kn_StringWriteToStream(ctx -> topic, stderr);
		fprintf(stderr,
			": ");
		perror("kn_ServerPublishEventToTopic");
	      }
            later = getTime(&(ctx -> retval));
            rtt = later - now;
            if (later < ctx -> timer)
            {
                ctx -> timer = later;
                if (rtt < ctx -> options -> wait)
                {
                    ctx -> timer += ctx -> options -> wait - rtt;
                }
            }
        }
    }
    kn_Release(evt);
}

/* start sending events */
static void
reallyStart(PingContext *ctx)
{
    printf("PING ");
    kn_StringWriteToStream(ctx -> topic, stdout);
    printf(" from ??? : %ld byte payload\n",
           (long) ctx -> options -> size);
    ctx -> running = 1;
    ctx -> timer = getTime(&(ctx -> retval));
}

/* subscription success handler */
static void
onSubSuccess(kn_EventRef evt, void *void_ctx)
{
    PingContext *ctx = (PingContext *) void_ctx;

    if (ctx -> options -> verbose > 2)
    {
        fprintf(stderr,
                "%s <%p>: got successful status event: ",
                __FUNCTION__,
                ctx);
        fput_knobj(evt, stderr);
        fprintf(stderr, "\n");
    }
    reallyStart(ctx);
}

/* subscription error handler */
static void
onSubError(kn_EventRef evt, void *void_ctx)
{
    PingContext *ctx = (PingContext *) void_ctx;
    kn_StringRef lkn8_status = KNSTR("status");
    kn_StringRef lkn8_kn_payload = KNSTR("kn_payload");
    kn_StringRef lkn8MyStatus;
    kn_StringRef lkn8MyPayload;
    
    if ((! lkn8_status) ||
        ! lkn8_kn_payload)
    {
        error_nowhack("KNSTR", &(ctx -> retval));
        return;
    }
    if (ctx -> options -> verbose > 2)
    {
        fprintf(stderr,
                "%s <%p>: got error status event: ",
                __FUNCTION__,
                ctx);
        fput_knobj(evt, stderr);
        fprintf(stderr, "\n");
    }
    fprintf(stderr,
            "Error: Subscription to ");
    kn_StringWriteToStream(ctx -> topic, stderr);
    fprintf(stderr,
            " failed: ");
    if ((lkn8MyStatus = kn_EventGetValue(evt, lkn8_status)))
        kn_StringWriteToStream(lkn8MyStatus, stderr);
    else
        fprintf(stderr, "(null)");
    fprintf(stderr,
            "\n");
    if ((lkn8MyPayload = kn_EventGetValue(evt, lkn8_kn_payload)))
        kn_StringWriteToStream(lkn8MyPayload, stderr);
    else
        fprintf(stderr, "(null)");
    fprintf(stderr,
            "\n");
    ctx -> retval = 1;
}

/* subscription status handler */
static void
onSubStatus(kn_EventRef evt, void *void_ctx)
{
    statusDispatcher(evt, void_ctx, &onSubSuccess, &onSubError);
}

/* round a double to the nearest millionth (approximately) */
static double
uchop(double x)
{
    return floor(x * 1e6 + 0.5) / 1e3;
}

/* event handler */
static void
onPing(kn_EventRef evt, void *void_ctx)
{
    PingContext *ctx = (PingContext *) void_ctx;
    kn_StringRef lkn8_kn_time_t = KNSTR("kn_time_t");
    kn_StringRef lkn8_kn_payload = KNSTR("kn_payload");
    kn_StringRef lkn8_kn_id = KNSTR("kn_id");
    kn_StringRef lkn8_seq = KNSTR("seq");
    kn_StringRef lkn8_sid = KNSTR("sid");
    kn_StringRef lkn8MyTime = 0;
    kn_StringRef lkn8MyPayload = 0;
    kn_StringRef lkn8MySeq = 0;
    kn_StringRef lkn8MyID = 0;
    kn_StringRef lkn8MySID = 0;
    int have_rtt = 0;
    double rtt = 0.0;
    double now = 0.0;

    if (ctx -> options -> verbose > 2)
    {
        fprintf(stderr,
                "%s <%p>: got event: ",
                __FUNCTION__,
                ctx);
        fput_knobj(evt, stderr);
        fprintf(stderr, "\n");
    }
    if (! ctx -> running)
        return;
    if ((! lkn8_kn_time_t) ||
        (! lkn8_kn_payload) ||
        (! lkn8_kn_id) ||
        (! lkn8_sid) ||
        ! lkn8_seq)
    {
        error_nowhack("KNSTR", &(ctx -> retval));
        return;
    }
    if ((! (lkn8MySID = kn_EventGetValue(evt, lkn8_sid))) ||
        kn_StringCompare(ctx -> sid, lkn8MySID))
    {
        return;
    }
    lkn8MyPayload = kn_EventGetValue(evt, lkn8_kn_payload);
    if (! (lkn8MySeq = kn_EventGetValue(evt, lkn8_seq)))
    {
        fprintf(stderr,
                "Warning: sequence number lost\n");
    }
    else
    {
        size_t seq_len;
        char seq_buf[SEQ_BUFLEN];

        seq_len = kn_StringGetSize(lkn8MySeq);
        if (! seq_len)
        {
            fprintf(stderr,
                    "Warning: sequence number is empty\n");
        }
        else if (seq_len >= sizeof(seq_buf))
        {
            fprintf(stderr,
                    "Warning: sequence number is too long: %ld bytes\n",
                    (long) seq_len);
        }
        else
        {
            long seq;
            char *endptr;

            memcpy(seq_buf, kn_StringGetBytes(lkn8MySeq), seq_len);
            seq_buf[seq_len] = '\0';
            seq = strtol(seq_buf, &endptr, 0);
            if (*endptr)
            {
                fprintf(stderr,
                        "Warning: sequence number is not a number\n");
            }
            else
            {
                /* calculate the sequence field distance between this
                   ping and the last one we got back */
                long seq_diff =
                    ((seq - ctx -> last_seq + SEQ_FIELD) %
                     SEQ_FIELD + SEQ_FIELD / 2) %
                    SEQ_FIELD -
                    SEQ_FIELD / 2;

                if (seq_diff <= 0)
                {
                    fprintf(stderr,
                            "Warning: received duplicate event\n");
                }
                else
                {
                    ctx -> last_seq = seq;
                    ctx -> stats . received_ok ++;
                }
            }
        }
    }
    if (! (lkn8MyTime = kn_EventGetValue(evt, lkn8_kn_time_t)))
    {
        fprintf(stderr,
                "Warning: timestamp lost\n");
    }
    else
    {
        size_t then_len;
        char then_buf[TIME_BUFLEN];

        then_len = kn_StringGetSize(lkn8MyTime);
        if (! then_len)
        {
            fprintf(stderr,
                    "Warning: timestamp is empty\n");
        }
        else if (then_len >= sizeof(then_buf))
        {
            fprintf(stderr,
                    "Warning: timestamp is too long: %ld bytes\n",
                    (long) then_len);
        }
        else
        {
            double then;
            char *endptr;

            now = getTime(&(ctx -> retval));
            if (! ctx -> retval)
            {
                memcpy(then_buf, kn_StringGetBytes(lkn8MyTime), then_len);
                then_buf[then_len] = '\0';
                then = strtod(then_buf, &endptr);
                if (*endptr)
                {
                    fprintf(stderr,
                            "Warning: timestamp is not a number\n");
                }
                else
                {
                    have_rtt = 1;
                    rtt = now - then;
                }
            }
        }
    }
    if (have_rtt)
    {
        ctx -> stats . received ++;
        ctx -> stats . rt . sum += rtt;
        ctx -> stats . rt . squared_sum += rtt * rtt;
        if (ctx -> stats . received == 1)
            ctx -> stats . rt . min = ctx -> stats . rt . max = rtt;
        else if (rtt < ctx -> stats . rt . min)
            ctx -> stats . rt . min = rtt;
        else if (rtt > ctx -> stats . rt . max)
            ctx -> stats . rt . max = rtt;
        if (! ctx -> options -> quiet)
        {
            kn_StringRef lkn8_kn_routed_from = KNSTR("kn_routed_from");
            kn_StringRef lkn8MySource;

            if (! lkn8_kn_routed_from)
            {
                error_nowhack("KNSTR", &(ctx -> retval));
                return;
            }
            printf("%ld bytes from ",
                   (long) (lkn8MyPayload ?
                           kn_StringGetSize(lkn8MyPayload) :
                           0));
            if ((lkn8MySource = kn_EventGetValue(evt, lkn8_kn_routed_from)))
            {
                kn_StringWriteToStream(lkn8MySource, stdout);
            }
            else
            {
                printf("(null)");
            }
            printf(": seq=");
            if (lkn8MySeq)
            {
                kn_StringWriteToStream(lkn8MySeq, stdout);
            }
            else
            {
                printf("(null)");
            }
            printf(" time=%g ms\n",
                   uchop(rtt));
        }
        if (! lkn8MyPayload)
            fprintf(stderr, "Warning: payload lost\n");
        else if (kn_StringCompare(ctx -> payload, lkn8MyPayload))
            fprintf(stderr, "Warning: payload data corruption detected\n");
        if (! (lkn8MyID = kn_EventGetValue(evt, lkn8_kn_id)))
        {
            fprintf(stderr,
                    "Warning: event is missing kn_id header\n");
        }
        else if ((now < ctx -> timer) &&
                 ctx -> waitingFor &&
                 ! kn_StringCompare(ctx -> waitingFor, lkn8MyID))
        {
            ctx -> timer = now;
            if (rtt < ctx -> options -> wait)
            {
                ctx -> timer += ctx -> options -> wait - rtt;
            }
        }
        if (ctx -> options -> limit &&
            (ctx -> stats . received_ok >= ctx -> options -> limit))
        {
            ctx -> done = 1;
        }
    }
}

/* unsubscription success handler */
static void
onUnsubSuccess(kn_EventRef evt, void *void_ctx)
{
    PingContext *ctx = (PingContext *) void_ctx;

    if (ctx -> options -> verbose > 2)
    {
        fprintf(stderr,
                "%s <%p>: got successful status event: ",
                __FUNCTION__,
                ctx);
        fput_knobj(evt, stderr);
        fprintf(stderr, "\n");
    }
}

/* unsubscription error handler */
static void
onUnsubError(kn_EventRef evt, void *void_ctx)
{
    PingContext *ctx = (PingContext *) void_ctx;
    kn_StringRef lkn8_status = KNSTR("status");
    kn_StringRef lkn8_kn_payload = KNSTR("kn_payload");
    kn_StringRef lkn8MyStatus;
    kn_StringRef lkn8MyPayload;
    
    if ((! lkn8_status) ||
        ! lkn8_kn_payload)
    {
        error_nowhack("KNSTR", &(ctx -> retval));
        return;
    }
    if (ctx -> options -> verbose > 2)
    {
        fprintf(stderr,
                "%s <%p>: got error status event: ",
                __FUNCTION__,
                ctx);
        fput_knobj(evt, stderr);
        fprintf(stderr, "\n");
    }
    fprintf(stderr,
            "Error: Unsubscription from ");
    kn_StringWriteToStream(ctx -> topic, stderr);
    fprintf(stderr,
            " failed: ");
    if ((lkn8MyStatus = kn_EventGetValue(evt, lkn8_status)))
        kn_StringWriteToStream(lkn8MyStatus, stderr);
    else
        fprintf(stderr, "(null)");
    fprintf(stderr,
            "\n");
    if ((lkn8MyPayload = kn_EventGetValue(evt, lkn8_kn_payload)))
        kn_StringWriteToStream(lkn8MyPayload, stderr);
    else
        fprintf(stderr, "(null)");
    fprintf(stderr,
            "\n");
}

/* unsubscription status handler */
static void
onUnsubStatus(kn_EventRef evt, void *void_ctx)
{
    statusDispatcher(evt, void_ctx, &onUnsubSuccess, &onUnsubError);
}

/* file descriptor interest change notification handler */
static void
onInterest(kn_ServerRef server, int fd,
                       kn_ServerProcessCallback onReadable,
                       kn_ServerProcessCallback onWritable,
                       kn_ServerProcessCallback onException,
                       void *void_ctx)
{
    PingContext *ctx = (PingContext *) void_ctx;

    if (! ctx)
    {
        fprintf(stderr,
                "%s: server %p gave NULL userdata\n",
                __FUNCTION__,
                server);
        exit(1);
    }
    if (fd >= ctx -> nfds)
        ctx -> nfds = fd + 1;
    if (onReadable)
        FD_SET(fd, &(ctx -> readfds));
    else
        FD_CLR(fd, &(ctx -> readfds));
    if (onWritable)
        FD_SET(fd, &(ctx -> writefds));
    else
        FD_CLR(fd, &(ctx -> writefds));
    if (onException)
        FD_SET(fd, &(ctx -> exceptfds));
    else
        FD_CLR(fd, &(ctx -> exceptfds));
    ctx -> onReadable[fd] = onReadable;
    ctx -> onWritable[fd] = onWritable;
    ctx -> onException[fd] = onException;
}

/* describes the given object to the given stream; returns the number of
   bytes written (or EOF, in case of error) */
static int
fput_knobj(kn_ObjectRef obj, FILE *stream)
{
    size_t count = 0;
    kn_StringRef desc = kn_CopyDescription(obj);

    if (! desc)
        return EOF;
    count = kn_StringWriteToStream(desc, stream);
    if (count != kn_StringGetSize(desc))
        return EOF;
    kn_Release(desc);
    return count;
}

/** PingContext implementation **/

/* (optional) system entropy device */
#define ENTROPY "/dev/random"

/* base topic we choose a random subtopic of */
#define TOPIC_BASE "/what/ping"

/* big enough for TOPIC_BASE '/' random number '\0' */
#define TOPIC_BUFLEN (sizeof(TOPIC_BASE) + 1 + 42 + 1)

/* allocator and initializer */
static PingContext *
newPingContext(PingOptions *options, char *url, char *topic)
{
    PingContext *ctx = (PingContext *) malloc(sizeof(PingContext));
    char random_topic[TOPIC_BUFLEN]; /* used while autogenerating topic names */
    /* we try to collect a little randomness here */
    /* (when faking it, we put more random fakery in entropy[0] and less random in entropy[1]) */
    unsigned long entropy[2] = { 0UL, 0UL };
    char sid[42 + 1]; /* used to autogenerate a session ID string */

    if (! ctx)
        return ctx;

    /* user-supplied values */
    ctx -> options = options;

    /* set some default values */
    ctx -> retval = 0; /* the default return value is initially 0 (success) */
    ctx -> server = 0; /* server object */
    ctx -> route = 0; /* subscription object */
    ctx -> running = 0; /* not running, initially */
    ctx -> done = 0; /* not finished either */
    ctx -> timer = 0.0; /* when to send the next ping [only while running] */
    ctx -> seq = 0L; /* 0 is the first number in any sane sequence... */
    ctx -> last_seq = SEQ_FIELD - 1L; /* pretend to have received the last sequence number */

    /* statistics */
    ctx -> stats . transmitted = 0UL; /* number of pings sent */
    ctx -> stats . received = 0UL; /* total number of transmitted pings we get back */
    ctx -> stats . received_ok = 0UL; /* number of non-duplicate transmitted pings we get back */
    /* round-trip info for all transmitted pings we get back */
    ctx -> stats . rt . min = 0.0; /* minimum observed round-trip time */
    ctx -> stats . rt . max = 0.0; /* maximum observed round-trip time */
    ctx -> stats . rt . sum = 0.0; /* sum of observed round-trip times */
    ctx -> stats . rt . squared_sum = 0.0; /* sum of squares of observed round-trip times */

    /* file descriptor sets and handlers */
    ctx -> nfds = 0;
    FD_ZERO(&(ctx -> readfds));
    FD_ZERO(&(ctx -> writefds));
    FD_ZERO(&(ctx -> exceptfds));
    {
        int fd;

        for (fd = 0; fd < FD_SETSIZE; fd ++)
        {
            ctx -> onReadable[fd] = 0;
            ctx -> onWritable[fd] = 0;
            ctx -> onException[fd] = 0;
        }
    }

    /* dynamically-allocated strings */
    ctx -> url = 0;
    ctx -> topic = 0;
    ctx -> payload = 0;
    ctx -> expiry = 0;
    ctx -> waitingFor = 0;
    ctx -> sid = 0;

    /** initialization **/

    /* find some entropy, or fake it */
    if (ctx &&
        ! ctx -> retval)
    {
        struct timeval now_tv;
        FILE *random_file;
        pid_t pid;

        /* try to get the Real Thing */
        if ((random_file = fopen(ENTROPY, "rb")))
        {
            if (fread((void *) entropy,
                      sizeof(unsigned long),
                      2,
                      random_file) < 2 &&
                ctx -> options -> verbose)
            {
                perror(ENTROPY);
            };
            fclose(random_file);
        }
        else if (ctx -> options -> verbose)
        {
            perror(ENTROPY);
        }

        /* fallback fake entropy -- failure can be safely ignored here */
        gettimeofday(&now_tv, 0);
        pid = getpid();
        entropy[0] ^= (now_tv . tv_sec & 0xFF) ^ now_tv . tv_usec ^ pid;
        entropy[1] ^= pid ^ (now_tv . tv_sec >> 8);
    }

    /* choose a (not-very-random) session ID */
    sprintf(sid,
            "%lu",
            entropy[topic ? 0 : 1]);

    /* choose a topic, if necessary */
    if (! topic)
    {
        sprintf(random_topic,
                "%s/%lu",
                TOPIC_BASE,
                entropy[0]);
        topic = random_topic;
    }

    /* convert our perfectly good bytes into knstrs (ick!) */
    if (ctx &&
        (! ctx -> retval) &&
        ! ((ctx -> sid =
            kn_StringCreateWithCString(sid)) &&
           (ctx -> url =
            kn_StringCreateWithCString(url)) &&
           (ctx -> topic =
            kn_StringCreateWithCString(topic))))
    {
        error_nowhack("kn_StringCreateWithCString", &(ctx -> retval));
    }

    /* produce the payload pattern string */
    if (ctx &&
        ! ctx -> retval)
    {
        kn_StringRef lkn8MyHexBytes = kn_StringCreateWithCString(ctx -> options -> pattern);

        if (! lkn8MyHexBytes)
        {
            error_nowhack("kn_StringCreateWithCString", &(ctx -> retval));
        }
        else
        {
            kn_StringRef lkn8MyBytes = kn_StringCreateByDecodingURLHex(lkn8MyHexBytes);
            kn_Release(lkn8MyHexBytes);
            if (! lkn8MyBytes)
            {
                error_nowhack("kn_StringCreateByDecodingURLHex", &(ctx -> retval));
            }
            else
            {
                size_t len;

                if (! (len = kn_StringGetSize(lkn8MyBytes)))
                {
                    kn_Release(lkn8MyBytes);
                    if (! (lkn8MyBytes = kn_StringCreateWithBytesNoCopy("\0", 1, kn_FALSE)))
                    {
                        error_nowhack("kn_StringCreateWithBytesNoCopy", &(ctx -> retval));
                    }
                    else
                    {
                        len = 1;
                    }
                }
                if (len)
                {
                    kn_MutableStringRef lkn8MyTemplate = kn_StringCreateMutable(len + ctx -> options -> size);

                    if (! lkn8MyTemplate)
                    {
                        error_nowhack("kn_StringCreateMutable", &(ctx -> retval));
                    }
                    else
                    {
                        while (kn_StringGetSize(lkn8MyTemplate) < ctx -> options -> size)
                        {
                            if (kn_StringAppendString(lkn8MyTemplate, lkn8MyBytes) != kn_SUCCESS)
                            {
                                error_nowhack("kn_StringAppendString", &(ctx -> retval));
                                break;
                            }
                        }
                        if (! ctx -> retval)
                        {
                            const char *myTemplate = kn_StringGetBytes(lkn8MyTemplate);
                            ctx -> payload = kn_StringCreateWithBytes(myTemplate, ctx -> options -> size);
                            if (! ctx -> payload)
                            {
                                error_nowhack("kn_StringCreateWithBytes", &(ctx -> retval));
                            }
                        }
                        kn_Release(lkn8MyTemplate);
                    }
                }
                kn_Release(lkn8MyBytes);
            }
        }
    }

    /* generate our event expiry header */
    if (ctx &&
        ! ctx -> retval)
    {
        char expiry_buf[TIME_BUFLEN];

        sprintf(expiry_buf,
                "+%f",
                2.0 * ctx -> options -> maxwait);
        if (! (ctx -> expiry = kn_StringCreateWithCString(expiry_buf)))
        {
            error_nowhack("kn_StringCreateWithCString", &(ctx -> retval));
        }
    }

    /* allocate and initialize a knserv */
    if (ctx &&
        (! ctx -> retval) &&
        ! (ctx -> server = kn_ServerCreateWithURI(
            ctx -> url
            )))
    {
        deletePingContext(ctx);
        ctx = 0;
    }

    if (ctx &&
        (! ctx -> retval) &&
        (ctx -> options -> verbose > 2))
    {
        fputs("created server object: ", stderr);
        fput_knobj(ctx -> server, stderr);
        fputs("\n", stderr);
    }

    return ctx;
}

/* deallocator -- doesn't unsubscribe; do that before calling deletePingContext */
static void
deletePingContext(PingContext *ctx)
{
    if (ctx -> route)
        kn_Release(ctx -> route);
    if (ctx -> server)
        kn_Release(ctx -> server);
    if (ctx -> topic)
        kn_Release(ctx -> topic);
    if (ctx -> url)
        kn_Release(ctx -> url);
    if (ctx -> payload)
        kn_Release(ctx -> payload);
    if (ctx -> expiry)
        kn_Release(ctx -> expiry);
    if (ctx -> waitingFor)
        kn_Release(ctx -> waitingFor);
    if (ctx -> sid)
        kn_Release(ctx -> sid);

    free((void *) ctx);
}

/*** EXTERNAL ***/

/* print a string with special & non-printing characters escaped */
int
fputs_quoted(const char *str, FILE *stream)
{
    size_t i;
    int c;
    int last_octal = 0;

    if (fputc('"', stream) == EOF)
        return EOF;
    for (i = 0; (c = str[i]); i ++)
    {
        switch (c)
        {
        case '\\':
        case '\"':
        case '\'':
        case '\?':
            if (fputc('\\', stream) == EOF)
                return EOF;
            if (fputc(c, stream) == EOF)
                return EOF;
            break;
        case '\f':
            if (fputs("\\f", stream) == EOF)
                return EOF;
            break;
        case '\a':
            if (fputs("\\a", stream) == EOF)
                return EOF;
            break;
        case '\b':
            if (fputs("\\b", stream) == EOF)
                return EOF;
            break;
        case '\v':
            if (fputs("\\v", stream) == EOF)
                return EOF;
            break;
        case '\n':
            if (fputs("\\n", stream) == EOF)
                return EOF;
            break;
        case '\r':
            if (fputs("\\r", stream) == EOF)
                return EOF;
            break;
        case '\t':
            if (fputs("\\t", stream) == EOF)
                return EOF;
            break;
        default:
            if (isprint(c))
            {
                if (last_octal && isdigit(c) &&
                    (fputs("\"\"", stream) == EOF))
                    return EOF;
                if (fputc(c, stream) == EOF)
                    return EOF;
                last_octal = 0;
            }
            else
            {
                if (! fprintf(stream, "\\%o", c))
                    return EOF;
                last_octal = 1;
            }
        }
    }
    if (fputc('"', stream) == EOF)
        return EOF;
    return 0;
}

/** PingOptions implementation **/

/* allocator and initializer */
PingOptions *
newPingOptions(void)
{
    PingOptions *options = (PingOptions *) malloc(sizeof(PingOptions));

    if (! options)
        return options;

    /* set some default values */
    options -> limit = 0UL; /* no limit */
    options -> wait = 1.1; /* wait at least 1.1 seconds between publications */
    options -> pattern = "%00"; /* fill the payload with NULs */
    options -> verbose = 0U; /* non-verbose */
    options -> quiet = 0; /* don't suppress per-event reporting */
    options -> size = 32L; /* 32-byte payload, by default */
    options -> maxwait = 10.0; /* wait at most 10 seconds between publications */

    return options;
}

/* deallocator */
void
deletePingOptions(PingOptions *options)
{
    free((void *) options);
}

/* run pubsub_ping with the given options, returning a main()-style status code */
int
pubsub_ping(PingOptions *options, char *url, char *topic)
{
    PingContext *ctx = newPingContext(options, url, topic);
    void (*oldwhack)(int) = SIG_ERR;

    if (! ctx)
    {
        perror(url);
        return 1;
    }

    /* hook the server into our event loop */
    if (! ctx -> retval)
    {
        kn_ServerSetConnectionCallback(ctx -> server,
             onInterest,
             ctx);
    }

    /* establish a subscription */
    if (! ctx -> retval)
    {
        if (ctx -> options -> verbose)
        {
            fprintf(stderr, "Creating subscription... ");
            fflush(stderr);
        }
        if (! (ctx -> route = kn_RouteCreateFromTopicToFunctionViaServer(
            ctx -> topic,
            onPing,
            ctx -> server,
            NULL, /* <- headers go here */
            onSubStatus,
            ctx,
	    kn_FALSE)))
        {
            fprintf(stderr,
                    "Error: Subscription to ");
            kn_StringWriteToStream(ctx -> topic, stderr);
            fprintf(stderr,
                    " failed: ");
            perror("kn_RouteCreateFromTopicToFunctionViaServer");
            ctx -> retval = 1;
        }
        else if (ctx -> options -> verbose)
        {
            fprintf(stderr, "done.\n");
        }
    }

    /* we'll unsubscribe ourselves, thank you very much! */
    if (ctx -> route)
    {
        kn_RouteSetDeleteOnDealloc(
            ctx -> route,
            kn_FALSE
            );
    }

    /** the "meat" **/

    /* trap SIGINT */
    if (! ctx -> retval)
    {
        whacked = 0;
        if ((oldwhack = signal(SIGINT, whack)) == SIG_ERR)
        {
            perror("signal");
            ctx -> retval = 1;
        }
    }

    /* HACK HACK HACK */
    while ((! ctx -> retval) &&
           (! whacked) &&
           ! ctx -> done)
    {
        double now;
        struct timeval timeout_tv, *timeout_ptr = 0;
        double timeout = 0.0;
        int nready;
        fd_set readfds, writefds, exceptfds;

        now = getTime(&(ctx -> retval));
        if (ctx -> running)
        {
            if (ctx -> timer > now)
                timeout = ctx -> timer - now;
            timeout_tv . tv_sec = (long) floor(timeout);
            timeout_tv . tv_usec = (long) (1e6 * (timeout - floor(timeout)));
            timeout_ptr = &timeout_tv;
        }
        readfds = ctx -> readfds;
        writefds = ctx -> writefds;
        exceptfds = ctx -> exceptfds;
        if ((nready = select(ctx -> nfds,
                             &readfds,
                             &writefds,
                             &exceptfds,
                             timeout_ptr)) == -1)
        {
            error_nowhack("select", &(ctx -> retval));
            break;
        }
        if (nready)
        {
            int fd;

            for (fd = 0; fd < ctx -> nfds; fd ++)
            {
                if (FD_ISSET(fd, &readfds))
                {
                    (*ctx -> onReadable[fd])(ctx -> server, fd);
                }
                if (FD_ISSET(fd, &writefds))
                {
                    (*ctx -> onWritable[fd])(ctx -> server, fd);
                }
                if (FD_ISSET(fd, &exceptfds))
                {
                    (*ctx -> onException[fd])(ctx -> server, fd);
                }
            }
        }
        if (now >= ctx -> timer)
        {
            doPing(ctx);
        }
    }

    /* restore original SIGINT handler */
    if (oldwhack != SIG_ERR)
        signal(SIGINT, oldwhack);

    /* we're no longer running */
    if (ctx -> running)
    {
        ctx -> running = 0;
        printf("\n--- ");
        kn_StringWriteToStream(ctx -> topic, stdout);
        printf(" pubsub_ping statistics ---\n");
        printf("%lu events transmitted, ",
               ctx -> stats . transmitted);
        printf("%lu events received, ",
               ctx -> stats . received_ok);
        if (ctx -> stats . received > ctx -> stats . received_ok)
            printf("+%lu duplicates, ",
                   ctx -> stats . received - ctx -> stats . received_ok);
        printf("%g%% event loss\n",
               100.0 *
               (ctx -> stats . transmitted - ctx -> stats . received_ok) /
               ctx -> stats . transmitted);
        if (ctx -> stats . received)
        {
            double mean = ctx -> stats . rt . sum / ctx -> stats . received;
            double standard_deviation = sqrt(
                ctx -> stats . rt . squared_sum / ctx -> stats . received -
                mean * mean);
            printf("round-trip min/avg/max = %g/%g/%g ms, std. dev. = %g ms\n",
                   uchop(ctx -> stats . rt . min),
                   uchop(mean),
                   uchop(ctx -> stats . rt . max),
                   uchop(standard_deviation));
        }
    }

    /* if we got no events back, set the return value to 1 */
    if ((! ctx -> retval) &&
        ! ctx -> stats . received_ok)
    {
        ctx -> retval = 1;
    }

    /** cleanup **/

    if (ctx -> route)
    {
        if (ctx -> options -> verbose)
        {
            fprintf(stderr, "Removing subscription... ");
            fflush(stderr);
        }
        if (kn_RouteDelete(ctx -> route, onUnsubStatus, ctx, kn_FALSE) != kn_SUCCESS)
        {
            fprintf(stderr,
                    "Error: Unsubscription from ");
            kn_StringWriteToStream(ctx -> topic, stderr);
            fprintf(stderr,
                    " failed: ");
            perror("kn_RouteDelete");
            ctx -> retval = 1;
        }
        if (ctx -> options -> verbose)
            fprintf(stderr, "done.\n");
        kn_Release(ctx -> route);
        ctx -> route = 0;
    }

    /* copy the return value from our context before we delete it */
    {
        int retval = ctx -> retval;

        if (ctx)
            deletePingContext(ctx);
    
        return retval;
    }
}
