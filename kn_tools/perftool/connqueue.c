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
 * $Id: connqueue.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

#include <stdlib.h>
#include <errno.h>
#include "util.h"
#include "connqueue.h"
#include "dstring.h"
#include "openconn.h"

static unused char rcsid[] = "@(#) $Id: connqueue.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $";

typedef struct conn {
    evl_t *evl;
    struct sockaddr_in sin;
    dstring *data;
    pipe_fitting_handlers handlers;
    struct conn *next;  /* linked list */
    void (*on_failure)(void *client_data);  /* when we can't launch the conn */
} conn;

struct connqueue {
    evl_t *evl;
    int opening_conns;
    int max_opening_conns;
    conn *next_queue_item;
    conn **queue_tail_pointer;
};

/* we have to wrap the on_connect and on_error callbacks so ours gets
 * called first */
typedef struct connect_wrapper_struct {
    connqueue_t *cq;
    void *user_client_data;
    void (*user_on_connect)(pipe_fitting_t *self);
    void (*user_on_error)(pipe_fitting_t *self, evl_why why, int connected);
} connect_wrapper_t;

/* run_queue calls launch_conn, which refers to connect_wrapper and
 * error_wrapper, which call unwrap, which calls run_queue, so one of
 * them needs a forward declaration. */
static void run_queue(connqueue_t *cq);

static pipe_fitting_handlers *unwrap(pipe_fitting_t *self)
{
    pipe_fitting_handlers *handlers = pipe_fitting_get_handlers(self);
    connect_wrapper_t *w = (connect_wrapper_t*)handlers->client_data;
    int old_errno = errno;  /* save user's errno */
    handlers->client_data = w->user_client_data;
    handlers->on_connect = w->user_on_connect;
    handlers->on_error = w->user_on_error;
    w->cq->opening_conns--;
    run_queue(w->cq);
    free(w);  /* the C equivalent of "delete this;" :) */
    errno = old_errno;
    return handlers;
}

/* undo the interposition and call the user's on_connect handler */
static void connect_wrapper(pipe_fitting_t *self)
{
    pipe_fitting_handlers *pfh = unwrap(self);
    pfh->on_connect(self);
}

static void error_wrapper(pipe_fitting_t *self, evl_why why, int connected)
{
    pipe_fitting_handlers *pfh = unwrap(self);
    pfh->on_error(self, why, connected);
}

static int launch_conn(connqueue_t *cq, conn *next)
{
    connect_wrapper_t *cw;
    pipe_fitting_t *pf;
    pipe_fitting_handlers *pfh;
    int s;
    /* open a connection */
    s = open_connection(&next->sin);
    if (s < 0) return 0;
    /* create a pipe fitting for the connection */
    pf = new_pipe_fitting(cq->evl, s, s);
    if (!pf) return 0;
    /* set the user's wrapped handlers on the connection */
    cw = (connect_wrapper_t*)malloc(sizeof(*cw));
    cw->cq = cq;
    cw->user_client_data = next->handlers.client_data;
    cw->user_on_connect = next->handlers.on_connect;
    cw->user_on_error = next->handlers.on_error;

    pfh = pipe_fitting_get_handlers(pf);
    *pfh = next->handlers;
    pfh->client_data = (void*)cw;
    pfh->on_connect = connect_wrapper;
    pfh->on_error = error_wrapper;

    if (!pipe_fitting_write(pf, next->data->start, next->data->len))
        pipe_fitting_destroy(pf);

    cq->opening_conns++;
    return 1;
}

static void run_queue(connqueue_t *cq) 
{
    conn *next;
    while (cq->opening_conns < cq->max_opening_conns && cq->next_queue_item) {
        next = cq->next_queue_item;
        if (!launch_conn(cq, next)) 
	    next->on_failure(next->handlers.client_data);
        cq->next_queue_item = next->next;
        if (!cq->next_queue_item) 
            cq->queue_tail_pointer = &cq->next_queue_item;
        dstring_free(next->data);
        free(next);
    }
}

/* 
 * We care about the following events:
 * - having a connection enqueued: open connections if possible
 * - opening a connection: increment opening_conns
 * - having a connection connect: decrement opening_conns; 
 *   hand the connection off to the user; open connections if possible
 */

connqueue_t *new_connqueue(evl_t *evl) 
{
    connqueue_t *rv = (connqueue_t*)malloc(sizeof(*rv));
    if (!rv) return NULL;
    rv->evl = evl;
    /* We have to limit the number of connections that are in the
     * "opening" state; the typical use of this code is to open a
     * bunch of connections to the same server at the same time, and
     * many systems have severe limits on their capacity to accept or
     * open many connections in a short time.  Typically if you try to
     * open more than 200 or so connections to the same server at the
     * same time, the later ones will time out.  So we limit the
     * number of connections that have had connect() called on them
     * but have not yet had any data pass over them. 
     *
     * Enforcing this limit is the entire raison d'etre for this
     * class. */
    rv->max_opening_conns = 120;
    rv->opening_conns = 0;
    /* this points to the next queue item to run */
    rv->next_queue_item = NULL;
    /* this points to the place to hang a new queue item */
    rv->queue_tail_pointer = &rv->next_queue_item;
    return rv;
}

int connqueue_is_empty(connqueue_t *cq)
{
    return !(cq->opening_conns || cq->next_queue_item);
}

/* It's not safe to call this while connections are still in the
 * not-yet-open state, because when they get to 'open', they will try
 * to decrement the opening connection count and run the queue.  
 *
 * So there are two cases where you'd want to call this.  One is when
 * you want to abort stuff and not do any further work on the
 * connection queue; this is the hard one.  The more normal one is
 * that you already know that all of your connections are out of the
 * connection queue, and probably already closed, and you just want to
 * get rid of the connqueue you allocated for the purpose of opening
 * all of them.  That one is pretty easy --- no nasty dangling
 * pointers show up.
 * 
 * We won't handle the first case for now.
 */
void connqueue_free(connqueue_t *cq) {
    if (!connqueue_is_empty(cq)) abort();
    free(cq);
}

int enqueue_connection(connqueue_t *cq, struct sockaddr_in *sin,
                       dstring *data,
                       pipe_fitting_handlers *handlers,
                       void (*on_failure)(void *client_data))
{
    conn *newconn = (conn*)malloc(sizeof(*newconn));
    if (!newconn) return 0;
    newconn->evl = cq->evl;
    newconn->sin = *sin;
    newconn->data = data;
    newconn->handlers = *handlers;
    newconn->on_failure = on_failure;
    newconn->next = NULL;
    *cq->queue_tail_pointer = newconn;
    cq->queue_tail_pointer = &newconn->next;
    run_queue(cq);
    return 1;
}
