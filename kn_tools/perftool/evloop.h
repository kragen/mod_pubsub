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
 * $Id: evloop.h,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

/*
 * File descriptor event loop interface.
 *
 * This can be implemented by different event loop modules --- a
 * select() loop for portability, a Solaris /dev/poll version for
 * speed on Solaris, a Linux /dev/poll version for speed on Linux,
 * a Win32 completion-ports version for speed on Win32, etc.
 *
 * This interface supports multiple event loops in the same process,
 * which makes it much harder to implement correctly.
 */

#ifndef __EVLOOP_H
#define __EVLOOP_H

#include <sys/time.h>

#ifndef __GNUC__
#ifndef inline
#define inline
#endif
#endif

typedef struct {
    void (*f)(void *clientData, void *arg);
    void *clientData;
} evl_callback_t;

static inline evl_callback_t make_callback(void (*f)(void*, void*), 
                                         void *clientData) {
    evl_callback_t tmp;
    tmp.f = f;
    tmp.clientData = clientData;
    return tmp;
}

static inline void evl_callback_invoke(evl_callback_t cb, void *arg)
{
    cb.f(cb.clientData, arg);
}

typedef enum {
    evl_read=0xceded, evl_write, evl_except
} evl_why;

typedef struct evl_t evl_t;

/* All functions return 0 on failure or nonzero on success. 
 * There is presently no decent error reporting. */

/* call this before anything else; it returns NULL on failure or a
   pointer to a newly-allocated event-loop object on success. */

evl_t *evl_init();

/* Registers a callback to be called when a particular condition is
 * true on a particular file descriptor --- data or EOF is waiting to
 * be read (evl_read), buffer space is available for writing
 * (evl_write), or an exceptional condition is pending (evl_except).
 * Removes any previous callback for the same condition.  If it
 * returns failure, there's no guarantee the old callback has been
 * preserved. */
int evl_set_callback(evl_t *evl, int fd, evl_why why, evl_callback_t cb);

/* Call this to remove a callback registered with evl_set_callback.
 * It is guaranteed that the callback will never be called again after
 * it is cleared with evl_clear_callback, so it is safe, for example,
 * to deallocate data the callback referred to. */
int evl_clear_callback(evl_t *evl, int fd, evl_why why);

/* Call this to check the status of all file descriptors and call all
 * appropriate callbacks.  If some of the conditions we're interested
 * in happen before the specified timeout is up, it returns 1; if it
 * times out, it returns 2; and if it failed, it returns 0, as usual.
 * The 'timeout' value specifies how many seconds to wait; -1.0 is
 * 'infinite'. */
int evl_poll(evl_t *evl, double timeout);

/* call this to make memleak detectors happy */
void evl_cleanup(evl_t *evl); 

#endif
