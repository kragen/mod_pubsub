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
 * $Id: pipefit.c,v 1.4 2004/04/19 05:39:15 bsittler Exp $
 **/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"
#include "pipefit.h"
#include "dstring.h"
#include "nonblock.h"

static unused char rcsid[] = 
    "@(#) $Id: pipefit.c,v 1.4 2004/04/19 05:39:15 bsittler Exp $";

static void nop(void *clientData, void *arg) 
{
}

struct pipe_fitting {
    evl_t *evl;
    dstring *outbuf;
    int infd;
    int outfd;
    pipe_fitting_handlers handlers;
    /* booleans: */
    unsigned int 
        connected: 1,  /* have we successfully read or written any data? */
        finished_writing: 1, /* is no more data to be written to us? */
        output_shutdown: 1,  /* have we shutdown or closed outfd? */
        read_eof: 1, /* have we read an EOF or error on infd? */
        closed: 1;  /* have we called the close callback? */
};

static void call_close_callback(pipe_fitting_t *p)
{
    if (!p->closed) 
    {
        p->closed = 1;
        p->handlers.on_close(p);
    }
}

void pipe_fitting_destroy(pipe_fitting_t *p)
{

    evl_clear_callback(p->evl, p->infd, evl_read);
    close(p->infd);
    p->read_eof = 1;
    
    evl_clear_callback(p->evl, p->outfd, evl_write);
    close(p->outfd);
    p->output_shutdown = 1;
    p->finished_writing = 1;
    
    call_close_callback(p);
}

static void shutdown_if_necessary(pipe_fitting_t *p)
{
    if (!p->output_shutdown && p->finished_writing && p->outbuf->len == 0) {
        int rv = shutdown(p->outfd, SHUT_WR);
        p->output_shutdown = 1;
        if (rv < 0) {
            if (errno == ENOTSOCK) {
                close(p->outfd);
            }
        }
        if (p->read_eof) {
            pipe_fitting_destroy(p);
        }
    }
}

/* called after errors have happened */
void pipe_fitting_error(pipe_fitting_t *p, evl_why why)
{
    if (!p->closed) p->handlers.on_error(p, why, p->connected);
    pipe_fitting_destroy(p);
}


/* XXX: at present this doesn't get called until some data makes its
 * way over the socket.  
 *
 * This is fixable in a couple of ways, the simplest of which is to
 * check the socket for writability until it's connected, and then use
 * getsockopt with SO_ERROR to see if the socket is connected or
 * broken.  If getsockopt gives us ENOTSOCK, we can figure we're
 * already connected.  This is reported to work at least on Solaris,
 * Linux, and BSD (UNIX Network Programming, Vol. 1, 2nd Ed., section
 * 15.4).
 * 
 * Another possibility would be to check the socket for writability
 * until it's connected, then use poll() or select() to see if the
 * same file descriptor is also readable; if so, we should invoke the
 * read callback, which will either read data or an EOF or get an
 * error; but if the socket is not readable, we know we have
 * successfully connected. 
 */
static void note_is_connected(pipe_fitting_t* p)
{
    if (!p->connected) {
        p->connected = 1;
        p->handlers.on_connect(p);
    }
}

static void pipe_fitting_read_callback(void *clientData, void *arg)
{
    pipe_fitting_t *p = (pipe_fitting_t*)clientData;
    dstring *inbuf = new_dstring();
    /* random magic number shows up in truss output  */
    int read_size = 3202;
    int rv;
    if (!inbuf) return;  /* hope someone frees some memory before 
                            calling us again */
    rv = dstring_read(p->infd, inbuf, read_size);
    if (rv < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS) {
            /* don't worry about it */
        } else if (errno == ENOMEM) {
            dstring_free(inbuf);
            return;  /* see comment by earlier return */
        } else {
            pipe_fitting_error(p, evl_read);
        }
    } else if (rv == 0) {
        note_is_connected(p);
        p->handlers.on_eof(p);
        evl_clear_callback(p->evl, p->infd, evl_read);
        p->read_eof = 1;
        if (p->output_shutdown) pipe_fitting_destroy(p);
    } else {
        note_is_connected(p);
        /*** Changes to see what's breaking... ***/
	/*
           fprintf(stderr, "rd<%d>: %ld bytes: [\x1b[7m", p->infd, (long)inbuf->len);
	   fwrite(inbuf->start, 1, inbuf->len, stderr); 
           fprintf(stderr, "\x1b[0m]\n");
	*/
        p->handlers.on_data(p, inbuf->start, inbuf->len);
    }
    dstring_free(inbuf);
}

static void pipe_fitting_write_callback(void *clientData, void *arg)
{
    pipe_fitting_t *p = (pipe_fitting_t*)clientData;
    int rv = dstring_write(p->outfd, p->outbuf);
    if (rv < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS) {
            /* don't worry about it */
        } else {
            pipe_fitting_error(p, evl_write);
        }
    } else {
      /*** Changes to see what's breaking... ***/
      /*
	fprintf(stderr, "wr<%d>: %ld bytes: [\x1b[1;7m", p->outfd, (long)rv);
	fwrite(p->outbuf->start, 1, rv, stderr); 
	fprintf(stderr, "\x1b[0m]\n");
      */
      /* XXX: potentially inefficient */
      memmove(p->outbuf->start, p->outbuf->start + rv, p->outbuf->len - rv);
      p->outbuf->len -= rv;
      note_is_connected(p);
    }
    if (!p->outbuf->len) {
        evl_clear_callback(p->evl, p->outfd, evl_write);
        shutdown_if_necessary(p);
    }
}

static pipe_fitting_handlers default_handlers = { 
    NULL, 
    (void(*)(pipe_fitting_t*))nop,
    (void(*)(pipe_fitting_t*))nop, 
    (void(*)(pipe_fitting_t*, evl_why, int))nop, 
    (void(*)(pipe_fitting_t*, char*, size_t))nop, 
    (void(*)(pipe_fitting_t*))nop 
};

/* external interfaces */

pipe_fitting_t *new_pipe_fitting(evl_t *evl, int infd, int outfd)
{
    pipe_fitting_t *rv = (pipe_fitting_t*)malloc(sizeof(*rv));
    /* making ttys nonblocking on Solaris is likely to cause you to be
     * spuriously logged out when this program exits or gets suspended. */
    if (!isatty(infd)) if (!make_nonblocking(infd)) return NULL;
    if (!isatty(outfd)) if (!make_nonblocking(outfd)) return NULL;
    if (!rv) return NULL;
    rv->outbuf = new_dstring();
    if (!rv->outbuf) {
        free(rv);
        return NULL;
    }
    if (!evl_set_callback(evl, infd, evl_read, 
                          make_callback(pipe_fitting_read_callback, 
                                        (void*)rv))) {
        dstring_free(rv->outbuf);
        free(rv);
        return NULL;
    }
    rv->infd = infd;
    rv->outfd = outfd;
    rv->connected = 0;
    rv->finished_writing = 0;
    rv->output_shutdown = 0;
    rv->read_eof = 0;
    rv->closed = 0;
    rv->handlers = default_handlers;
    rv->evl = evl;
    return rv;
}

void pipe_fitting_free(pipe_fitting_t *p)
{
    if (p) {
        pipe_fitting_destroy(p);
        dstring_free(p->outbuf);
        free(p);
    }
}

pipe_fitting_handlers *pipe_fitting_get_handlers(pipe_fitting_t *p)
{
    return &p->handlers;
}

int pipe_fitting_get_infd(pipe_fitting_t *p)
{
    return p->infd;
}

const pipe_fitting_handlers *pipe_fitting_get_default_handlers()
{
    return &default_handlers;
}

int pipe_fitting_write(pipe_fitting_t *p, char *data, size_t len)
{
    int rv;
    if (p->finished_writing) {
        errno = EBADF;  /* as if we were writing on a closed fd */
        return -1;
    }
    rv = dstring_append_cstr(p->outbuf, data, len);
    if (!rv) return -1;
    if (p->outbuf->len) {
        /* XXX: check for failure? */
        evl_set_callback(p->evl, p->outfd, evl_write, 
                         make_callback(pipe_fitting_write_callback, (void*)p));
    }
    return len;
}

int pipe_fitting_finish_writing(pipe_fitting_t *p)
{
    p->finished_writing = 1;
    shutdown_if_necessary(p);
    return 1;
}

