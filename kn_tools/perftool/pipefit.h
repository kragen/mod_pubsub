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
 * $Id: pipefit.h,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

#ifndef __PIPEFIT_H
#define __PIPEFIT_H

#include "evloop.h"

typedef struct pipe_fitting pipe_fitting_t;

/* A pipe fitting is one end of a pipe.
 * 
 * This API largely uses explicit continuation-passing style, which
 * makes code using it somewhat less readable.
 *
 * infd and outfd are normally the same, but don't have to be; the
 * pipe fitting reads from infd when data is available, and calls one
 * of on_eof, on_error, and on_data, depending on whether it just got
 * an end-of-file condition, an error, or some data.
 *
 * The pipe fitting also calls on_error when writing to its outfd
 * yields an error; the what_operation member of the
 * pipe_fitting_error_arg struct is used to tell the error handler
 * whether it was a read or a write error.
 *
 * When you pipe_fitting_finish_writing, then when any buffered data
 * has been written, the pipe fitting will try to shutdown() its outfd
 * with SHUT_WR.  If that fails with ENOTSOCK, it will close() its
 * outfd.
 *
 * A pipe fitting gets "closed" when any of the following things happen:
 *
 * - it gets a read or write error (including EAGAIN, EWOULDBLOCK, and EINTR)
 * - it close()s its outfd, as described above
 * - it has both read an EOF from its infd and pipe_fitting_finish_writing()ed.
 * */

pipe_fitting_t *new_pipe_fitting(evl_t *evl, 
                                 int infd, int outfd);
void pipe_fitting_free(pipe_fitting_t *p);

/* Each pipe fitting has a handler object associated with it which
 * handles events generated by the pipe fitting.  
 */

typedef struct pipe_fitting_handlers {
    void *client_data;
    /* called when the pipe fitting is connected.  This will happen
     * immediately for file descriptors attached to pipes, files, and
     * ttys; but for sockets, it may take a while. FIXME: right now,
     * this won't fire immediately if no data is buffered to be sent
     * over the connection and no data is sent from the other end. */
    void (*on_connect)(pipe_fitting_t *self);
    /* called when the pipe fitting reads an EOF */
    void (*on_eof)(pipe_fitting_t *self);
    /* called when the pipe fitting gets an error on read or write */
    /* the 'connected' parameter specifies whether the pipe fitting
     * has successfully written or read data; it's useful to be able
     * to distinguish errors on the first read or write to a network
     * connection from errors on later reads or writes, as the former
     * generally indicate a problem connecting, while the latter
     * generally indicate that the connection was closed after being
     * opened */
    void (*on_error)(pipe_fitting_t *self, 
                     evl_why operation,  /* was error on read or write? */
                     int connected);
    /* called when the pipe fitting receives data */
    void (*on_data)(pipe_fitting_t *self, char *data, size_t len);
    /* called when both the inbound and outbound parts of the pipe
     * fitting are closed.  None of the pipe_fitting code will ever
     * touch the pipe fitting after this point, so it is safe to
     * pipe_fitting_free() the pipe fitting from the on_close
     * callback. */
    void (*on_close)(pipe_fitting_t *self);
} pipe_fitting_handlers;

/* Returns a pointer to the pipe_fitting_handlers struct for a pipe
 * fitting; you can fill this struct with your handlers to change the
 * behavior of the pipe fitting.  */
pipe_fitting_handlers *pipe_fitting_get_handlers(pipe_fitting_t *p);
/* Returns a pointer to the default pipe_fitting_handlers struct, *
 * which is handy if you're constructing pipe_fitting_handlers
 * yourself. */
const pipe_fitting_handlers *pipe_fitting_get_default_handlers();

/* returns the input file descriptor for debug-logging purposes */
int pipe_fitting_get_infd(pipe_fitting_t *p);

/* like write(2), returns number of bytes written on success or -1 on
 * failure; running out of buffer space is considered failure. */
int pipe_fitting_write(pipe_fitting_t *p, char *data, size_t len);

/* call this to indicate that you are not going to write any more on
 * the pipe; relays this to the other end if possible and returns 1 on
 * success. */
int pipe_fitting_finish_writing(pipe_fitting_t *p);

/* call this if you want to invalidate the pipe fitting but aren't
 * responsible for freeing it */
void pipe_fitting_destroy(pipe_fitting_t *p);

#endif
