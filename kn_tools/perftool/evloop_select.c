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
 * $Id: evloop_select.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

/*
 * Implementation of a file-descriptor event loop for select().
 *
 * Depends on malloc and select, and assumes that an fd_set is a
 * vector of ints.
 *
 * An implementation using the fd_set type would work up to FD_SETSIZE
 * descriptors and then croak; this implementation works, albeit
 * slowly, with larger numbers.  However, the other implementation
 * would be simpler, would not require any dynamic allocation, and
 * might be more portable.
 */

#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include "evloop.h"
#include "evloop_common.h"
#include "dstring.h"

static unused char rcsid[] = "@(#) $Id: evloop_select.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $";


struct evl_t {
    evl_common_t *e;
    dstring *r, *w, *x;  /* returned fd_sets; here for efficiency */
};

/* this program assumes that fd_sets are arrays of 32-bit ints.  This
 * tests that dependency. */
static void test_fd_set_implementation()
{
    char *foo[8];
    int i;
    for (i = 0; i != 8; i++)
        foo[i] = '\0';
    assert(!get_bit((int*)foo, 0));
    assert(!FD_ISSET(0, (fd_set*)foo));
    assert(!get_bit((int*)foo, 8));
    assert(!FD_ISSET(8, (fd_set*)foo));
    assert(!get_bit((int*)foo, 32));
    assert(!FD_ISSET(32, (fd_set*)foo));

    set_bit((int*)foo, 0, 1);
    assert(get_bit((int*)foo, 0));
    assert(FD_ISSET(0, (fd_set*)foo));

    set_bit((int*)foo, 32, 1);
    assert(!get_bit((int*)foo, 8));
    assert(!FD_ISSET(8, (fd_set*)foo));
    assert(get_bit((int*)foo, 32));
    assert(FD_ISSET(32, (fd_set*)foo));

    assert(!get_bit((int*)foo, 31));
    assert(!FD_ISSET(31, (fd_set*)foo));
    set_bit((int*)foo, 31, 1);
    assert(get_bit((int*)foo, 31));
    assert(FD_ISSET(31, (fd_set*)foo));
}


struct evl_t *evl_init() {
    evl_t *evl = malloc(sizeof(evl_t));

    test_fd_set_implementation();
    
    if (!evl) return NULL;
    evl->e = new_evl_common_t();
    evl->r = new_dstring();
    evl->w = new_dstring();
    evl->x = new_dstring();
    if (!(evl->e && evl->r && evl->w && evl->x))
    {
        evl_cleanup(evl);
        return 0;
    }
    return evl;
}

void evl_cleanup(evl_t *evl)
{
    evl_common_free(evl->e);
    dstring_free(evl->r);
    dstring_free(evl->w);
    dstring_free(evl->x);
}

int evl_set_callback(evl_t *evl, int fd, evl_why why, evl_callback_t cb)
{
    return evl_common_set_callback(evl->e, fd, why, cb);
}

int evl_clear_callback(evl_t *evl, int fd, evl_why why) 
{
    return evl_common_clear_callback(evl->e, fd, why);
}

static void run_handlers(evl_t *evl, evl_why why) {
    int i, j;
    dstring *ds;
    
    switch(why)
    {
    case evl_read: ds = evl->r; break;
    case evl_write: ds = evl->w; break;
    case evl_except: ds = evl->x; break;
    default: abort();
    }

    for (i = 0; i < ds->len / sizeof(int);  i++)
        for (j = 0; j < 32; j++)
        {
            int n = i * 32 + j;
            if (get_bit((int*)ds->start, n)) 
                evl_common_invoke_handler(evl->e, n, why);
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

    if (!(evl_common_get_fdset(evl->e, evl->r, evl_read) &&
          evl_common_get_fdset(evl->e, evl->w, evl_write) &&
          evl_common_get_fdset(evl->e, evl->x, evl_except))) return 0;
    rv = select(evl_common_upper_bound(evl->e),
                (fd_set*)evl->r->start,
                (fd_set*)evl->w->start,
                (fd_set*)evl->x->start,
                timeoutp);
    if (rv < 0 && errno == EINTR) {
        return 1;  /* pretend we succeeded and didn't time out */
    } else if (rv < 0) { /* error in select */
        return 0;  
    } else if (rv == 0) return 2;  /* timeout */
    else {
        run_handlers(evl, evl_read);
        run_handlers(evl, evl_write);
        run_handlers(evl, evl_except);
        return 1;
    }
}
