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
 * $Id: evloop_common.h,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

/* stuff event loops have in common */
#ifndef __EVLOOP_COMMON_H
#define __EVLOOP_COMMON_H
#include "evloop.h"
#include "dstring.h"

typedef struct evl_common evl_common_t;

evl_common_t *new_evl_common_t();
void evl_common_free(evl_common_t *e);

/* invoke the handler for an event */
void evl_common_invoke_handler(evl_common_t *e, int fd, evl_why why);
/* get a copy of the specified fd_set; it will be written into dest.
 * Returns 0 on failure, nonzero on success. */
int evl_common_get_fdset(evl_common_t *e, dstring *dest, evl_why why);
/* determine whether or not any callback is registered for the
 * specified event on the specified fd */
int evl_common_is_interested(evl_common_t *e, int fd, evl_why why);
/* set the callback for an event */
int evl_common_set_callback(evl_common_t *e, int fd, evl_why why, 
                            evl_callback_t cb);
/* remove callbacks for an event and remove it from the interest set */
int evl_common_clear_callback(evl_common_t *e, int fd, evl_why why);
/* return a number larger than the largest fd being managed; *
 * guarantees that fd_sets obtained through evl_common_get_fdset won't
 * be any smaller than this number of bits, so you can use
 * evl_common_upper_bound's return value as the first argument to
 * select */
int evl_common_upper_bound(evl_common_t *e);

/* these functions don't really belong here, but they need to be
 * available in evloop_common.c, and every other thing that needs them
 * will include this .h file, so we might as well put them here. */
/* like FD_SET and FD_CLR: set the specified bit in a bit vector (such
 * as an fd_set) to the specified new value. */
void set_bit(void *data, int fd, int newval);
/* like FD_ISSET: return the specified bit from a bit vector */
int get_bit(void *data, int fd);

#endif
