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
 * $Id: nc.c,v 1.4 2004/04/19 05:39:15 bsittler Exp $
 **/

/*
 * simple program to test event-loop functionality.
 *
 * You give it a hostname and a port on the command line.  It looks up the
 * hostname, connects to the port, and relays stdin and stdout.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include "util.h"
#include "evloop.h"
#include "dstring.h"
#include "pipefit.h"
#include "openconn.h"

static unused char rcsid[] = "@(#) $Id: nc.c,v 1.4 2004/04/19 05:39:15 bsittler Exp $";

static int open_pipes = 0;

/* So.  I have 45 minutes to make this work.  What does it need to do?
 * When data gets input on stdin, it needs to enqueue it to go out
 * over the socket.
 * When data gets input on the socket, it needs to enqueue it to go
 * out on stdout.
 * When an error happens on input or output on the socket, we should
 * close the socket and report an error; how we report the error
 * should depend on whether we have successfully read or written the
 * socket previously.
 * When a read from the socket returns EOF, we should close stdout and
 * remove the read callback for the socket.
 * When a read from stdin returns EOF, we should shutdown(s, SHUT_WR)
 * and remove the read callback for stdin.
 * If a read from the socket returns EOF and we have already shutdown
 * the outgoing half of the connection, we should close the socket.
 * Whenever we close the socket, either because of error or because
 * both halves have been shut down, we are "done", and we should also
 * remove the read and write callbacks for the socket.
 * When a callback gets called for the closed socket, we should simply
 * return without doing anything.
 * When an error happens on output on stdout or input on stdin, we
 * should probably close the socket and quit, too.

 * To rephrase along programmatic lines:
 * when stdin is readable:
 *     data -> enqueue it to send over the socket
 *     EOF -> shutdown the socket, and remove the read callback for stdin
 *     error -> close the socket and remove the callback for stdin
 * when the socket is readable:
 *     data -> enqueue it to send to stdout
 *     EOF -> close stdout and remove the read callback for the socket
 *     error -> close the socket
 * when writing data to stdout:
 *     error -> 
 */

static void copy_data(pipe_fitting_t *self, char *data, size_t len)
{
    pipe_fitting_t *other_end = (pipe_fitting_t *)
      (pipe_fitting_get_handlers(self)->client_data);
    int rv = pipe_fitting_write(other_end, data, len);
    if (rv < 0) {
        perror("writing data");
    }
}

static void finish_sending(pipe_fitting_t *self)
{
    pipe_fitting_t *victim = ((pipe_fitting_t*)
                              pipe_fitting_get_handlers(self)->client_data);
    pipe_fitting_finish_writing(victim);
}

static void handle_socket_close(pipe_fitting_t *self)
{
    fprintf(stderr, "Connection closed by foreign host.\n");
    finish_sending(self);
}

static void error_handler(pipe_fitting_t *self, 
                          evl_why what_operation,
                          int connected)
{
    char *msg;
    if (!connected) {
        msg = "connect";
    } else if (what_operation == evl_read) {
        msg = "read";
    } else {
        msg = "write";
    }
    perror(msg);
    pipe_fitting_destroy(self);
    pipe_fitting_destroy((pipe_fitting_t*)
                         pipe_fitting_get_handlers(self)->client_data);
}

static void open_handler(pipe_fitting_t *self)
{
    fprintf(stderr, "Connected.\n");
}

static void close_handler(pipe_fitting_t *self)
{
    open_pipes--;
}
void usage(char *argv0)
{
    fprintf(stderr, "%s: usage: %s host port\n", argv0, argv0);
    exit(1);
}

void setup_handlers(pipe_fitting_handlers *handlers, pipe_fitting_t *otherend)
{
    handlers->client_data = (void*)otherend;
    handlers->on_eof = finish_sending;
    handlers->on_error = error_handler;
    handlers->on_data = copy_data;
    handlers->on_close = close_handler;
}

int main(int argc, char **argv)
{
    evl_t *evl = evl_init();
    struct sockaddr_in sin;
    pipe_fitting_t *console, *socketpipe;
    int sock;
    int rv = 0;
    if (argc != 3) usage(argv[0]);
    if (!evl) {
        fprintf(stderr, "couldn't init event loop\n");
        return 1;
    }

    if (!make_sockaddr_in_byname(&sin, argv[1], atoi(argv[2]))) goto end;
    
    sock = open_connection(&sin);
    if ((sock < 0)) {
        rv = 1;
        goto end;
    }

    signal(SIGPIPE, SIG_IGN);

    console = new_pipe_fitting(evl, 0, 1);    /* stdin and stdout */
    socketpipe = new_pipe_fitting(evl, sock, sock);

    if (!(console && socketpipe)) {
        pipe_fitting_free(console);
        pipe_fitting_free(socketpipe);
    }
    
    setup_handlers(pipe_fitting_get_handlers(console), socketpipe);
    setup_handlers(pipe_fitting_get_handlers(socketpipe), console);
    pipe_fitting_get_handlers(socketpipe)->on_connect = open_handler;
    pipe_fitting_get_handlers(socketpipe)->on_eof = handle_socket_close;
 
    open_pipes = 2;
    while (open_pipes) {
        if (!evl_poll(evl, -1.0)) {
            perror("event loop");
            break;
        }
    }
end:
    evl_cleanup(evl);
    return rv;
}
