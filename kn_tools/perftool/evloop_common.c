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
 * $Id: evloop_common.c,v 1.2 2003/04/25 02:37:44 bsittler Exp $
 **/

/* 
 * Most event loops need to maintain some of the same data:
 * - a set of fds they care about for each of (r, w, x)
 * - a callback for each registered fd
 * In order to avoid duplicating this stuff between implementations of evloop,
 * I've moved the common stuff in here.
 */

#include <assert.h>
#include <string.h>
#include "evloop_common.h"
#include "util.h"
#include "dstring.h"

static unused char rcsid[] = "@(#) $Id: evloop_common.c,v 1.2 2003/04/25 02:37:44 bsittler Exp $";

struct select_data 
{
    dstring *fds;
    dstring *callbacks;
};


struct evl_common 
{
    int nfds;
    struct select_data r, w, x;
};

/* XXX: perhaps these guys should be in an array!? */
static struct select_data *rwx(evl_common_t *e, evl_why why)
{
    switch(why)
    {
    case evl_read: return &e->r;
    case evl_write: return &e->w;
    case evl_except: return &e->x;
    default: abort();
    }
}


/* initializes one of r, w, and x; leaves all the stuff in it in a
   valid (allocated or NULL) state. */
static int evl_init_select_data(struct select_data *d)
{
    d->fds = new_dstring();
    d->callbacks = new_dstring();
    return d->fds && d->callbacks;
}

/* clean up one of r, w, and x */
static void evl_cleanup_select_data(struct select_data *d)
{
    dstring_free(d->fds);
    dstring_free(d->callbacks);
}


void evl_common_free(evl_common_t *e) 
{
    evl_cleanup_select_data(&e->r);
    evl_cleanup_select_data(&e->w);
    evl_cleanup_select_data(&e->x);
    free(e);
}

evl_common_t *new_evl_common_t()
{
    evl_common_t *e = malloc(sizeof(*e));
    int r, w, x;
    if (!e) return 0;
    memset(e, '\x9f', sizeof(*e));
    e->nfds = 0;
    /* we can't do these in the conditional, because
       evl_cleanup_select_data needs all the members to be initialized
       to either NULL or something it can safely free. */
    r = evl_init_select_data(&e->r);
    w = evl_init_select_data(&e->w);
    x = evl_init_select_data(&e->x);
    if (!(r && w && x)) {
        evl_common_free(e);
        return 0;
    }
    return e;
}

void set_bit(void *data, int fd, int newval) 
{
    int *ip = (int*)data;
    /* the bottom five bits index inside an int; the others index 
       into the array */
    int mask = 1 << (fd & 0x1f);
    int index = fd >> 5;
    if (newval) ip[index] |= mask;
    else ip[index] &= ~mask;
}

int get_bit(void *data, int fd) 
{
    int *ip = (int*)data;
    int mask = 1 << (fd & 0x1f);
    int index = fd >> 5;
    return ip[index] & mask;
}

/* make the buffers in one of r, w, x big enough. */
static int make_buffers_big_enough_specifically(struct select_data *d, int fd,
                                                int newfdsize, int newcbsize)
{
    if (!dstring_vector_ready(d->fds, newfdsize)) return 0;
    if (!dstring_vector_ready(d->callbacks, newcbsize)) return 0;
    return 1;
}

static int make_buffers_big_enough(evl_common_t *e, int fd)
{
    int newsize = (fd / 32 + 1) * 4;
    int newcbsize = sizeof(evl_callback_t) * (fd + 1);
    int r, w, x;
    r = make_buffers_big_enough_specifically(&e->r, fd, newsize, newcbsize);
    w = make_buffers_big_enough_specifically(&e->w, fd, newsize, newcbsize);
    x = make_buffers_big_enough_specifically(&e->x, fd, newsize, newcbsize);
    return r && w && x;
}

static void set_callback(struct select_data *d, int fd, evl_callback_t cb) 
{
    ((evl_callback_t *)d->callbacks->start)[fd] = cb;
    set_bit(d->fds->start, fd, 1);
}

int evl_common_set_callback(evl_common_t *e, int fd, evl_why why, 
                            evl_callback_t cb) 
{
    assert(cb.f);
    if (!make_buffers_big_enough(e, fd)) return 0;
    set_callback(rwx(e, why), fd, cb);
    if (fd >= e->nfds) e->nfds = fd + 1;
    return 1;
}

static int clear_callback(struct select_data *d, int fd)
{
    if (fd >= d->callbacks->len/sizeof(evl_callback_t)) return 0;
    if (!get_bit(d->fds->start, fd)) return 0;
    set_bit(d->fds->start, fd, 0);
    /* XXX: we could compute a new nfds here, but won't bother */
    return 1;
}

int evl_common_clear_callback(evl_common_t *e, int fd, evl_why why) 
{
    return clear_callback(rwx(e, why), fd);
}

int evl_common_is_interested(evl_common_t *e, int fd, evl_why why)
{
    dstring *d = rwx(e, why)->fds;
    return (fd < d->len * 8) && get_bit((int*)d->start, fd);
}

void evl_common_invoke_handler(evl_common_t *e, int fd, evl_why why)
{
    evl_callback_t *cb;
    if (evl_common_is_interested(e, fd, why)) 
    {
        cb = (evl_callback_t*)rwx(e, why)->callbacks->start + fd;
        evl_callback_invoke(*cb, 0);
    }
}

int evl_common_get_fdset(evl_common_t *e, dstring *dest, evl_why why)
{
    return dstring_copy(dest, rwx(e, why)->fds);
}

int evl_common_upper_bound(evl_common_t *e)
{
    return e->nfds;
}
