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
 * $Id: openconn.c,v 1.2 2003/04/25 02:37:44 bsittler Exp $
 **/

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "util.h"
#include "openconn.h"
#include "nonblock.h"

static unused char rcsid[] = "@(#) $Id: openconn.c,v 1.2 2003/04/25 02:37:44 bsittler Exp $";

char *my_hstrerror(int error) 
{
    /* these are not guaranteed to be distinct, and aren't with
     * glibc2.1, so we can't use a switch. */
    if (error == HOST_NOT_FOUND) {
        return "Host not found";
    } else if (error == TRY_AGAIN) {
        return "Try again";
    } else if (error == NO_DATA) {
        return "No data";
    } else if (error == NO_ADDRESS) {
        return "No address";
    } else {
        return "Unknown name lookup error";
    }
}

/* herror() exists in traditional resolvers, but not all resolvers; so
 * here we have a reimplementation of it. */
void my_herror(char *buf) 
{
    int error = h_errno;
    char *msg = my_hstrerror(error);
    if (buf) {
        fprintf(stderr, "%s: %s\n", buf, msg);
    } else {
        fprintf(stderr, "%s\n", msg);
    }
}

void make_sockaddr_in(struct sockaddr_in *output, char *ipaddr, int port)
{
    memset(output, '\0', sizeof(*output));
    memcpy(&output->sin_addr, ipaddr, sizeof(output->sin_addr));
    output->sin_family = AF_INET;
    output->sin_port = htons(port);
}

int make_sockaddr_in_byname(struct sockaddr_in *sin, char *hostname, int port)
{
    struct hostent *he = gethostbyname(hostname);
    if (!he) {
        my_herror(hostname);
        return 0;
    } 
    make_sockaddr_in(sin, he->h_addr_list[0], port);
    return 1;
}

int open_connection(struct sockaddr_in *sin)
{
    int s = socket(PF_INET, SOCK_STREAM, 0);
    int rv;

    if (s == -1) return s;

    if (!make_nonblocking(s)) {
        close(s);
        return -1;
    }
    rv = connect(s, (struct sockaddr*)sin, sizeof(*sin));
    if (rv == 0) return s;
    else if (rv < 0) {
        if ((errno == EINPROGRESS) || /* this is the one I've seen */
            (errno == EAGAIN) ||  /* don't think I've seen this one */
            (errno == EWOULDBLOCK) /* this one is usually EAGAIN */
            ) {
            return s;
        } else {
            close(s);
            return -1;
        }
    } else abort(); /* connect() screwed up */
}

