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
 * $Id: evloop_soldevpoll.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

#include <sys/devpoll.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <math.h>
#include "util.h"
#include "evloop.h"
#include "evloop_common.h"

static unused char rcsid[] = "@(#) $Id: evloop_soldevpoll.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $";

/* We have to remember which events we're interested in on which
 * descriptors because evl_clear_callback has to know; it appears from
 * the man page that one must send a POLLREMOVE pollfd followed by a
 * new pollfd in order to reduce the set of events one is interested
 * in.
 *
 * It would seem that the select()-loop implementation would work as a
 * base for this, so I should factor it out.  */

struct evl_t {
    int fd; /* the fd for /dev/poll */
    evl_common_t *e;
};

evl_t *evl_init() 
{
    evl_t *rv = (evl_t*)malloc(sizeof(*rv));
    rv->fd = open("/dev/poll", O_RDWR);
    if (rv->fd < 0) 
    {
        free(rv);
        return 0;
    }
    rv->e = new_evl_common_t();
    if (!rv->e)
    {
        close(rv->fd);
        free(rv);
        return 0;
    }
    return rv;
}

void evl_cleanup(evl_t *evl)
{
    close(evl->fd);
    evl_common_free(evl->e);
    free(evl);
}

int evl_set_callback(evl_t *evl, int fd, evl_why why, evl_callback_t cb)
{
    struct pollfd pfd;
    int rv;

    if (!evl_common_set_callback(evl->e, fd, why, cb)) return 0;

    pfd.fd = fd;
    if (why == evl_read) 
    {
        pfd.events = POLLIN;
    } 
    else if (why == evl_write) 
    {
        pfd.events = POLLOUT;
    } 
    else if (why == evl_except)
    {
        return 0;  /* I don't know what evl_except means to poll() */
    }

    rv = write(evl->fd, &pfd, sizeof(pfd));
    if (rv < 0) 
    {
        return 0;
    } 
    else if (rv == sizeof(pfd)) 
    {
        return 1;
    } 
    else 
    {
        /* should never happen unless /dev/poll is broken */
        abort();
    }
}

int evl_clear_callback(evl_t *evl, int fd, evl_why why)
{
    struct pollfd pfds[2];
    int rv;

    if (!evl_common_clear_callback(evl->e, fd, why)) return 0;
    
    pfds[0].fd = fd;
    pfds[1].fd = fd;
    pfds[0].events = POLLREMOVE;
    pfds[1].events = 0;
    if (evl_common_is_interested(evl->e, fd, evl_write)) 
    {
        pfds[1].events |= POLLOUT;
    }
    if (evl_common_is_interested(evl->e, fd, evl_read))
    {
        pfds[1].events |= POLLIN;
    }
    rv = write(evl->fd, &pfds, sizeof(pfds));
    if (rv < 0) 
    {
        return 0;
    } 
    else if (rv == sizeof(pfds)) 
    {
        return 1;
    } 
    else
    {
        /* should never happen unless /dev/poll is broken */
        abort();
    }
}

#define EVENTS_PER_CALL 16
int evl_poll(evl_t *evl, double timeout)
{
    struct dvpoll pollparams;
    struct pollfd returned_events[EVENTS_PER_CALL];
    int rv;
    pollparams.dp_fds = returned_events;
    pollparams.dp_nfds = EVENTS_PER_CALL;
    if (timeout != -1.0) 
    {
        /* /dev/poll timeout is in milliseconds */
        pollparams.dp_timeout = ceil(timeout / 1000.0);
    } 
    else
    {
        pollparams.dp_timeout = -1;
    }
    rv = ioctl(evl->fd, DP_POLL, &pollparams);
    if (rv < 0) 
    {
        return 0;
    } 
    else if (rv == 0) 
    {
        return 2;
    } 
    else 
    {
        int i;
        for (i = 0; i < rv; i++)
            if (returned_events[i].revents & POLLIN)
                evl_common_invoke_handler(evl->e, returned_events[i].fd, 
                                          evl_read);
        for (i = 0; i < rv; i++) 
            if (returned_events[i].revents & POLLOUT)
                evl_common_invoke_handler(evl->e, returned_events[i].fd,
                                          evl_write);
        return 1;
    }
}
