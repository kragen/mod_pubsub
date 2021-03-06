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
 * $Id: connqueue.h,v 1.4 2004/04/19 05:39:14 bsittler Exp $
 **/

/* 
 * Open lots of simultaneous connections.  
 *
 * We maintain a queue of connections to open as pipe_fitting_ts.
 * Each one has a string of data to write to the connection when it
 * opens.  The string of data is a dstring, and it will be
 * dstring_free()d after it gets written to the connection ---
 * possibly before enqueue_connection returns.  
 *
 * The user is responsible for calling pipe_fitting_free() on the
 * pipe_fitting from their on_close handler (or in some other way).
 */

#ifndef __CONNQUEUE_H
#define __CONNQUEUE_H

#include <sys/types.h>
#include <netinet/in.h>
#include "dstring.h"
#include "evloop.h"
#include "pipefit.h"

typedef struct connqueue connqueue_t;

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
connqueue_t *new_connqueue(evl_t *evl, int max_opening_conns, float conns_per_sec);
int connqueue_is_empty(connqueue_t *cq);
void connqueue_free(connqueue_t *cq);

/* Call this to cause a connection to be opened with the specified
 * data enqueued to go out over it ASAP; the connection will have a
 * pipe_fitting_t created for it with the handlers struct in the
 * handlers argument (modulo some transparent wrapping done by the
 * connection queue machinery).  'handlers' will be copied immediately
 * and no references to it will be retained, so you can reuse or
 * delete it as soon as this call returns.
 */
int enqueue_connection(connqueue_t *cq, struct sockaddr_in *sin, 
                       dstring *data,
                       pipe_fitting_handlers *handlers,
                       void (*on_failure)(void *client_data));

#endif
