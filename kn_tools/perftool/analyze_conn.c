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
 * $Id: analyze_conn.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

/*
 * Keep track of per-connection stuff.
 */
#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "dstring.h"
#include "analyze_conn.h"

static unused char rcsid[] = "@(#) $Id: analyze_conn.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $";

/* a hash table for keys that are uniformly and densely distributed
 * positive integers, like connection and event ids, so we don't need
 * to worry about making the hash function robust. */

typedef struct intNode
{
    int id;
    struct intNode *next;
    void *userData;
} intNode;

#define TABLESIZE 256

typedef struct intRangeHash
{
    intNode *table[TABLESIZE];
    int minId;
    int maxId;
} intRangeHash;

/* we keep a sorted list of event ranges each connection has received */
typedef struct eventRange
{
    struct eventRange *next;
    /* earliest event in this range */
    int min;
    /* last event in this range */
    int max;
} eventRange;

/* stored in the conn hash */
typedef struct connInfo 
{
    long nbytes;  /* number of bytes received */
    eventRange *events;
} connInfo;

static void initHash(intRangeHash *h)
{
    int i;
    for (i = 0; i < TABLESIZE; i++)
    {
        h->table[i] = 0;
    }
    h->minId = -1;
    h->maxId = -1;
}
 
static intNode *lookup(intRangeHash *h, int id, 
                       void (*newFunc)(void **userData))
{
    intNode **rv;
    
    if (id < h->minId || h->minId == -1) h->minId = id;
    if (id > h->maxId) h->maxId = id;

    rv = &h->table[(unsigned)id % TABLESIZE];
    while (*rv) 
    {
        if ((*rv)->id == id) return *rv;
        rv = &(*rv)->next;
    }
    /* create a new one and link it in */
    *rv = malloc(sizeof(**rv));
    if (!*rv) return 0;
    (*rv)->id = id;
    (*rv)->next = 0;
    newFunc(&(*rv)->userData);
    return *rv;
}

static void newConnInfo(void **userData)
{
    connInfo *cip = malloc(sizeof(*cip));
    *userData = cip;
    
    cip->nbytes = -1;
    cip->events = 0;
}

static intRangeHash connHash;

static connInfo *lookupConn(int conn)
{
    return (connInfo*)lookup(&connHash, conn, newConnInfo)->userData;
}

void connSetBytes(int conn, long nbytes)
{
    connInfo *c = lookupConn(conn);
    if (!c) abort();
    c->nbytes = nbytes;
}

long connGetBytes(int conn)
{
    connInfo *c = lookupConn(conn);
    if (!c) abort();
    return c->nbytes;
}


eventRange *newEventRange(int event)
{
    eventRange *rv = malloc(sizeof(*rv));
    if (!rv) return 0;
    rv->next = 0;
    rv->min = event;
    rv->max = event;
    return rv;
}

void coalesceRanges(eventRange *er)
{
    while (er->next)
    {
        if (er->next->min == er->max + 1) 
        {
            eventRange *toDrop = er->next;
            er->max = toDrop->max;
            er->next = toDrop->next;
            free(toDrop);
        } 
        else
        {
            er = er->next;
        }
    }
}

static void eventWasReceived(int event, double latency);

void connReceivedEvent(int conn, int event, double latency)
{
    connInfo *c;
    eventRange **er;

    eventWasReceived(event, latency);

    c = lookupConn(conn);
    if (!c) abort();
    er = &c->events;
    while (*er)
    {
        /* if we're before the range, we need to insert;
         * if we're in the range, we need to drop the event. */
        if (event < (*er)->min)
        {
            eventRange *oldp = *er;
            *er = newEventRange(event);
            if (!*er) abort();
            (*er)->next = oldp;
            coalesceRanges(lookupConn(conn)->events);
            return;
        } 
        else if (event <= (*er)->max)
        {
            return;
        }
        er = &(*er)->next;
    }
    *er = newEventRange(event);
    coalesceRanges(lookupConn(conn)->events);
}

int minConn()
{
    return connHash.minId;
}

int maxConn()
{
    return connHash.maxId;
}


void printReceivedEvents(int conn)
{
    connInfo *c = lookupConn(conn);
    eventRange *er;
    if (!c) abort();
    er = c->events;
    if (!er) printf("(none)");
    while (er)
    {
        if (er->min == er->max)
        {
            printf("%d", er->min);
        }
        else
        {
            printf("%d-%d", er->min, er->max);
        }
        er = er->next;
        if (er)
        {
            printf(", ");
        }
    }
}

    
    
static intRangeHash eventHash;

void initAnalyzeConn()
{
    initHash(&connHash);
    initHash(&eventHash);
}

typedef struct eventInfo
{
    int ntimes;
    double ntimes_latency;
} eventInfo;



static int expectedReceptions = -1;

static void newEvent(void **userData)
{
    eventInfo *eip = malloc(sizeof(*eip));
    *userData = eip;
    eip->ntimes = 0;
    eip->ntimes_latency = 0.0/0.0;
}

static eventInfo *lookupEvent(int event)
{
    return (eventInfo*)lookup(&eventHash, event, newEvent)->userData;
}

static void eventWasReceived(int event, double latency)
{
    eventInfo *p = lookupEvent(event);
    if (!p) abort();
    p->ntimes++;
    if (p->ntimes == expectedReceptions) p->ntimes_latency = latency;
    if (p->ntimes > expectedReceptions) 
    {
        fprintf(stderr, "Warning: received event %d %d times\n",
                event, p->ntimes);
    }
}

void expectToReceiveEachEventNTimes(int ntimes)
{
    expectedReceptions = ntimes;
}

int minEvent()
{
    return eventHash.minId;
}

int maxEvent()
{
    return eventHash.maxId;
}

double eventFinalLatency(int event)
{
    return lookupEvent(event)->ntimes_latency;
}
