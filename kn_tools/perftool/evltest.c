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
 * $Id: evltest.c,v 1.3 2004/04/19 05:39:15 bsittler Exp $
 **/

/* simple regression test for evloops.  Because I have bugs in evloop
 * implementations occasionally, and applications are not a fun place
 * to look for them.  
 *
 * This isn't a very good test suite.  It does test all of the public
 * entry points into the evloop implementation, it does use multiple
 * fds and verify that, at least under some circumstances, the right
 * handler gets called. 
 *
 * It doesn't test any of the following scenarios, many of which have
 * been troublesome in the past:
 *
 * - setting a callback from a callback (particularly when this
 *   results in reallocation)
 * - clearing a callback from a callback
 * - setting a callback on an fd larger than FD_SETSIZE
 */

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "evloop.h"
#include "util.h"

static unused char rcsid[] = "@(#) $Id: evltest.c,v 1.3 2004/04/19 05:39:15 bsittler Exp $";

static int goodint;
void *good_client_data;

static void inc_good(void *client_data, void *trash)
{
    assert(good_client_data == client_data);
    goodint++;
}

static void failcb(void *client_data, void *trash)
{
    abort();
}

int main()
{
    evl_t *evl = evl_init();
    int fd[2];
    int rv;
    char c;

    /* these tests shouldn't take ten seconds; if they do, we're in an
     * infinite loop and should fail */
    alarm(10);
    
    rv = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    if (rv < 0)
    {
        perror("socketpair");
        return 1;
    }

    /* this should return quickly */
    assert(evl_poll(evl, 0.0) == 2);
    assert(evl_poll(evl, 0.25) == 2);
    /* it's not readable */
    assert(evl_set_callback(evl, fd[0], evl_read, make_callback(failcb, 0)));
    assert(evl_poll(evl, 0.0) == 2);

    /* now make it readable, but make sure the other one isn't readable */
    goodint = 0;
    good_client_data = (void*)31;
    assert(evl_set_callback(evl, fd[0], evl_read, 
                            make_callback(inc_good, (void*)31)));
    assert(evl_set_callback(evl, fd[1], evl_read, make_callback(failcb, 0)));
    assert(write(fd[1], "x", 1) == 1);
    assert(evl_poll(evl, -1.0) == 1);
    assert(goodint == 1);

    /* both fds should be writable */
    goodint = 0;
    assert(evl_set_callback(evl, fd[0], evl_write,
                            make_callback(inc_good, (void*)31)));
    assert(evl_set_callback(evl, fd[1], evl_write,
                            make_callback(inc_good, (void*)31)));
    assert(evl_poll(evl, -1.0) == 1);
    assert(3 == goodint);

    /* removing a callback should make it no longer get called */
    goodint = 0;
    assert(evl_clear_callback(evl, fd[0], evl_write));
    assert(evl_poll(evl, -1.0) == 1);
    assert(2 == goodint);
    
    /* reading the data should make the fd no longer writable */
    goodint = 0;
    assert(read(fd[0], &c, 1) == 1);
    assert(evl_poll(evl, -1.0) == 1);
    assert(1 == goodint);

    evl_cleanup(evl);
    
    return 0;
}
