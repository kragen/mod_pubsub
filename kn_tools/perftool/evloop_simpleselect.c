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
 * $Id: evloop_simpleselect.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

/*
 * Implementation of a file-descriptor event loop for select().
 *
 * Uses the standard fd_set macros, but relies on an fd_set being an
 * array of 32-bit longs.  Will crash when the number of file
 * descriptors becomes too large.  */

#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include "evloop.h"
#include "dstring.h"

static unused char rcsid[] = "@(#) $Id: evloop_simpleselect.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $";

/* invariant: if callbacks is long enough to hold an entry for fd N, so
 *            is fds, but not conversely.  This means you should lengthen
 *            fds first. */
struct select_data {
    fd_set fds;
    evl_callback_t callbacks[FD_SETSIZE];
    fd_set returned_fds;
};

struct evl_t {
    int nfds;  /* first argument to select() */
    struct select_data r;
    struct select_data w;
    struct select_data x;
};

/* initializes one of r, w, and x; leaves all the stuff in it in a
   valid (allocated or NULL) state. */
static void evl_init_select_data(struct select_data *d)
{
    FD_ZERO(&d->fds);
    FD_ZERO(&d->returned_fds);
    memset(d->callbacks, '\0', sizeof(d->callbacks));
}

int evl_cleanup(evl_t *evl) {
    free(evl);
    return 1;
}

struct evl_t *evl_init() {
    evl_t *evl = malloc(sizeof(evl_t));
    if (!evl) return NULL;
    evl->nfds = 0;
    evl_init_select_data(&evl->r);
    evl_init_select_data(&evl->w);
    evl_init_select_data(&evl->x);
    return evl;
}

static void set_callback(struct select_data *d, int fd, evl_callback_t cb) {
    d->callbacks[fd] = cb;
    FD_SET(fd, &d->fds);
}

int evl_set_callback(evl_t *evl, int fd, evl_why why, evl_callback_t cb) {
    assert(cb.f);
    switch (why) {
    case evl_read: set_callback(&evl->r, fd, cb); break;
    case evl_write: set_callback(&evl->w, fd, cb); break;
    case evl_except: set_callback(&evl->x, fd, cb); break;
    default: assert("bad" == "why in set_callback"); return 0;
    }
    if (fd >= evl->nfds) evl->nfds = fd + 1;
    return 1;
}

static void clear_callback(struct select_data *d, int fd) {
    FD_CLR(fd, &d->fds);
    /* XXX: we could compute a new nfds here, but won't bother */
}

int evl_clear_callback(evl_t *evl, int fd, evl_why why) {
    switch (why) {
    case evl_read: clear_callback(&evl->r, fd); return 1;
    case evl_write: clear_callback(&evl->w, fd); return 1;
    case evl_except: clear_callback(&evl->x, fd); return 1;
    default: assert("bad" == "why in clear_callback"); return 0;
    }
}

/* This function has a somewhat tricky life; we need to make sure we
 * don't try to call any callbacks that were removed between the time
 * we called select() and the time we get around to calling them,
 * because jumping to address 0 usually crashes programs.  */
static void run_handlers(evl_t *evl, struct select_data *d) {
    int i, j;
    evl_cb_arg cba;
    cba.evl = evl;
    for (i = 0;  i < evl->nfds/32 + 1;  i++)
        if (((long*)&d->returned_fds)[i])
            for (j = 0; j < 32; j++) {
                int n = i * 32 + j;
                if (FD_ISSET(n, &d->returned_fds) &&
                    FD_ISSET(n, &d->fds)) {
                    evl_callback_invoke(d->callbacks[n], (void*)&cba);
                }
            }
}

int evl_poll(evl_t *evl, double timeout) {
    int rv;
    struct timeval timeout_tv;
    struct timeval *timeoutp = NULL;
    if (timeout != -1.0) {
        timeout_tv.tv_sec = timeout;
        timeout_tv.tv_usec = (timeout - timeout_tv.tv_sec) * 1000000 + 0.5;
        timeoutp = &timeout_tv;
    }

    evl->r.returned_fds = evl->r.fds;
    evl->w.returned_fds = evl->w.fds;
    evl->x.returned_fds = evl->x.fds;
    rv = select(evl->nfds, 
                &evl->r.returned_fds,
                &evl->w.returned_fds,
                &evl->x.returned_fds, 
                timeoutp);
    if (rv < 0 && errno == EINTR) {
        return 1;  /* pretend we succeeded and didn't time out */
    } else if (rv < 0) { /* error in select */
        return 0;  
    } else if (rv == 0) return 2;  /* timeout */
    else {
        run_handlers(evl, &evl->r);
        run_handlers(evl, &evl->w);
        run_handlers(evl, &evl->x);
        return 1;
    }
}
