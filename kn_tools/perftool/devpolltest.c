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
 * $Id: devpolltest.c,v 1.2 2004/04/19 05:39:14 bsittler Exp $
 **/

#include <sys/devpoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stropts.h>  /* for Solaris ioctl */
#include <stdio.h>
#include "util.h"

/* test behavior of Solaris /dev/poll */
/* the question is: how do you clear interest bits? */
/* the docs say:
 *
 *   If a pollfd array contains multiple pollfd entries with same fd
 *   field, the "events" field in each pollfd entry is OR'ed. A
 *   special POLLREMOVE event in the events field of the pollfd
 *   structure will remove the fd from the monitored set.
 *
 * They don't explain what happens when there are multiple pollfd
 * entries with the same fd field in different pollfd arrays.  My
 * hypothesis is that they behave as if they were in the same array
 * and are ORed together.  An alternative hypothesis is that a new
 * pollfd struct with the same fd field will replace the previous
 * pollfd struct with that fd field.
 *
 * This program demonstrates that the first hypothesis is correct.
 *
 * This means that registering or unregistering, say, write interest
 * on some fd requires you to know whether or not read interest is
 * currently registered on it.
 */

/* So the test is as follows:
 * - register read interest on an fd
 * - verify it's not reported as readable
 * - in another call, register write interest on it
 * - verify it's reported as writable but not readable
 * - make it readable by giving it a byte to read
 * - verify both writability and readability get reported
 * - unregister interest completely
 * - verify it's not reported as readable or writable
 */

static unused char *rcsid = "@(#) $Id: devpolltest.c,v 1.2 2004/04/19 05:39:14 bsittler Exp $";

void poll_expecting(int dpfd, struct dvpoll *dvp, int fds)
{
    int rv = ioctl(dpfd, DP_POLL, dvp);
    if (rv < 0)
    {
	perror("/dev/poll: ioctl");
	exit(1);
    }
    else if (rv != fds)
    {
        printf("Expected ioctl to return %d, but got %d\n", fds, rv);
    }
}

void write_dvp(int dpfd, int fd, int events)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    if (write(dpfd, &pfd, sizeof(pfd)) != sizeof(pfd))
    {
        perror("/dev/poll: write");
	exit(1);
    }
}

int main()
{
    int sv[2];
    int i = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    struct pollfd pfd;
    int dpfd;
    struct dvpoll dvp;

    if (i < 0)
    {
        perror("socketpair");
	return 1;
    }

    dpfd = open("/dev/poll", O_RDWR);
    if (dpfd < 0)
    {
        perror("/dev/poll");
        return 1;
    }

    write_dvp(dpfd, sv[0], POLLIN);

    dvp.dp_fds = &pfd;
    dvp.dp_nfds = 1;
    dvp.dp_timeout = 0;

    poll_expecting(dpfd, &dvp, 0);
 
    write_dvp(dpfd, sv[0], POLLOUT);

    poll_expecting(dpfd, &dvp, 1);
    if (pfd.fd != sv[0]) printf("funky fd %d (not %d)\n", pfd.fd, sv[0]);
    if (pfd.events != (POLLIN | POLLOUT)) printf("funky events %d (not %d)\n",
						 pfd.events, POLLIN | POLLOUT);
    if (pfd.revents != POLLOUT) printf("funky revents %d (not %d)\n",
				       pfd.revents, POLLOUT);
    
    /* after we write something to the other end of the socketpair,
     * our end should become readable */
    write(sv[1], "x", 1);
    poll_expecting(dpfd, &dvp, 1);
    if (pfd.revents != (POLLIN | POLLOUT)) 
      printf("funky revents %d (not %d)\n",
	     pfd.revents, POLLIN|POLLOUT);

    write_dvp(dpfd, sv[0], POLLREMOVE);

    poll_expecting(dpfd, &dvp, 0);

    return 0;
}
