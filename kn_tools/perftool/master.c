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
 * $Id: master.c,v 1.4 2003/05/06 04:42:16 bsittler Exp $
 **/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "util.h"
#include "master.h"
#include "evloop.h"
#include "connqueue.h"
#include "dstring.h"
#include "dstr_interp.h"
#include "openconn.h"
#include "evrpt.h"
#include "doubletime.h"
#include "parser.h"
#include "post_event.h"

static unused char rcsid[] = "@(#) $Id: master.c,v 1.4 2003/05/06 04:42:16 bsittler Exp $";

static dstring *tunnel_req(char *basetopic, int sessionid, int journalid,
                           int simple_format)
{
    return dstring_interp_s("GET /kn?kn_from=%s/%d/journals/%d/kn_journal&"
                            "do_method=route%s HTTP/1.0\r\n\r\n", 
                            basetopic, sessionid, journalid, 
                            (simple_format ? 
                             "&kn_response_format=simple" : 
                             ""));
}

static int tunnels = 0;


typedef struct tunnel_info {
    int tunnelid;
    parser_t *parser;
    int debug;  /* log input */
    int has_received_status_event;
    pipe_fitting_t *output; /* where to log events */
} tunnel_info_t;

void tunnel_info_free(tunnel_info_t *ti)
{
    parser_free(ti->parser);
    free(ti);
}

static void on_tunnel_close(pipe_fitting_t *self)
{
    tunnel_info_t *ti = (tunnel_info_t*)pipe_fitting_get_handlers(self)->client_data;
    tunnels--;
    fprintf(stderr, "Tunnel %d closed\n", ti->tunnelid);
    tunnel_info_free((tunnel_info_t*)pipe_fitting_get_handlers(self)->client_data);
    pipe_fitting_free(self);
    exit(1);
}

static void tunnel_error_handler(pipe_fitting_t *self, evl_why op, int connected)
{
    tunnel_info_t *ti = (tunnel_info_t*)pipe_fitting_get_handlers(self)->client_data;
    fprintf(stderr, "Error on tunnel %d: ", ti->tunnelid);
    perror("");
    exit(1);
}

static void tunnel_creation_failure(void *client_data)
{
    tunnel_info_t *ti = (tunnel_info_t*)client_data;
    fprintf(stderr, "Error opening tunnel %d: ", ti->tunnelid);
    perror("");
    tunnel_info_free(ti);
    exit(1);
}

static void tunnel_event_handler(void *client_data, int id, int bytes, 
                                 int conn)
{
    tunnel_info_t *ti = (tunnel_info_t*)client_data;
    if (!ti->has_received_status_event) 
    {
        ti->has_received_status_event = 1;
        tunnels++;
    }
    else
    {
        report_event(id, bytes, conn, ti->output);
    }
}

static void tunnel_data_handler(pipe_fitting_t *self, char *data, size_t len)
{
    tunnel_info_t *ti = (tunnel_info_t*)pipe_fitting_get_handlers(self)->client_data;
    /* kludgy and blocking */
    if (ti->debug) {
        char *separator = "<packet boundary>";
        write(2, separator, strlen(separator));
        write(2, data, len);
    }
    parse_data(ti->parser, ti->tunnelid, data, len);
}

tunnel_info_t *new_tunnel_info(int tunnelid, pipe_fitting_t *output, 
                               int debug, int simple_format)
{
    tunnel_info_t *rv = (tunnel_info_t*)malloc(sizeof(tunnel_info_t));
    if (!rv) return 0;
    rv->parser = new_simple_parser(tunnel_event_handler, (void*)rv, 
                                   simple_format);
    if (!rv->parser) {
        free(rv);
        return 0;
    }
    rv->tunnelid = tunnelid;
    rv->debug = debug;
    rv->output = output;
    rv->has_received_status_event = 0;
    return rv;
}


static void eof_handler(pipe_fitting_t *p)
{
    pipe_fitting_finish_writing(p);
}

static void route_creation_failure(void *client_data)
{
    perror("Creating route");
    exit(1);
}

static void route_error_handler(pipe_fitting_t *self, evl_why why, int connected)
{
    route_creation_failure(NULL);
}

static int remaining_routes;

static void route_on_close(pipe_fitting_t *self)
{
    remaining_routes--;
    pipe_fitting_free(self);
}

static int enqueue_route(connqueue_t *cq, struct sockaddr_in *sin, 
                          dstring *from, dstring *to)
{
    pipe_fitting_handlers handlers;
    dstring *req = 
        dstring_interp_s("GET /kn?kn_from=%k&do_method=route&kn_to=%k HTTP/1.0\r\n"
                         "\r\n",
                         from, to);
    int rv;
    if (!req) return 0;
    handlers = *pipe_fitting_get_default_handlers();
    handlers.on_close = route_on_close;
    handlers.on_eof = eof_handler;
    handlers.on_error = route_error_handler;
    rv = enqueue_connection(cq, sin, req, &handlers, route_creation_failure);
    if (!rv) dstring_free(req);
    return rv;
}


static void establish_routes(connqueue_t *cq, 
                             struct sockaddr_in *sin,
                             masterparms *parms, 
                             int sessionid)
{
    int i, j;
    dstring *from, *to;
    for (i = 1; i <= parms->origin_topics; i++) {
        from = dstring_interp_s("%s/%d/origin/%d", 
                             parms->basetopic, 
                             sessionid,
                             i);
        if (!from) abort();
        for (j = 1; j <= parms->journals_per_topic; j++) {
            to = dstring_interp_s("%s/%d/journals/%d/kn_journal",
                               parms->basetopic,
                               sessionid,
                               (i - 1) * parms->journals_per_topic + j);
            if (!to) abort();
            if (!enqueue_route(cq, sin, from, to)) abort();
            dstring_free(to);
        }
        dstring_free(from);
    }
}


/* 11. If you have a procedure with ten parameters, you probably missed some.
 * --- Alan Perlis, "Epigrams in Programming"
 */
static void establish_tunnel_connections(connqueue_t *cq, 
                                         struct sockaddr_in *sin,
                                         masterparms *parms, 
                                         int sessionid,
                                         pipe_fitting_t *output)
{
    int journalid, tunnelno;
    int total_journals = parms->journals_per_topic * parms->origin_topics;
    dstring *req;
    pipe_fitting_handlers handlers;
    static int conn = 0;

    handlers = *pipe_fitting_get_default_handlers();
    handlers.on_error = tunnel_error_handler;
    handlers.on_close = on_tunnel_close;
    handlers.on_data = tunnel_data_handler;
    handlers.on_eof = eof_handler;
    for (journalid = 1; journalid <= total_journals; journalid++) {
        for (tunnelno = 0; tunnelno < parms->tunnels_per_journal; tunnelno++) {
            req = tunnel_req(parms->basetopic, sessionid, journalid, 
                             parms->simple_format);
            conn++;
            /* only debug journal 500 */
            handlers.client_data = 
                new_tunnel_info(conn, output, 
                                parms->debug && journalid == 500,
                                parms->simple_format);
            if (!handlers.client_data || 
                !enqueue_connection(cq, sin, req, &handlers, 
                                    tunnel_creation_failure)) {
                fprintf(stderr, "ouch, couldn't enqueue conn");
                dstring_free(req);
            }
        }
    }
}

static void post_creation_failure(void *client_data)
{
    perror("Error posting");
    exit(1);
}

static void post_error_handler(pipe_fitting_t *self, evl_why why, int connected)
{
    post_creation_failure(NULL);
}

void enqueue_another_post(evl_t *evl, connqueue_t *cq, struct sockaddr_in *sin,
                          masterparms *parms, int sessionid, int serial,
                          void *client_data)
{
    /* real: */
    char *fmt = "%s/%d/origin/%d\0";
    /* fake: 
       char *fmt = "%s/%d/journals/%d/kn_journal\0"; */
    int fmtlen = strlen(fmt) + 1;
    dstring *topic = dstring_interp_cstr(fmt, fmtlen, parms->basetopic, 
                                         sessionid,
                                         /* real: */
                                          serial % parms->origin_topics + 1
                                         /* fake:
                                         serial % (parms->origin_topics *
                                         parms->journals_per_topic) + 1 */);
    dstring *message;
    pipe_fitting_handlers handlers;
    int kn_id = parms->batchsize ? serial * parms->batchsize : serial;

    handlers = *pipe_fitting_get_default_handlers();
    handlers.on_close = pipe_fitting_free;
    handlers.on_eof = eof_handler;
    handlers.on_error = post_error_handler;

    /* remember the original event */
    handlers.client_data = client_data;

    if (!topic) abort();
    message = post_event(topic->start, kn_id, parms->payloadsize, 
                         parms->batchsize);
    if (!message) abort();
    dstring_free(topic);

    if (!parms->batchsize)
    {
        event_was_just_sent(kn_id);
    }
    else
    {
        int i;
        for (i = kn_id; i < kn_id + parms->batchsize; i++)
        {
            event_was_just_sent(i);
        }
    }
    if (!enqueue_connection(cq, sin, message, &handlers, 
                            post_creation_failure)) 
        abort();
}

void dump_parms(struct masterparms *parms, pipe_fitting_t *pf)
{
    dstring *ds = dstring_interp_s("host: %s port: %d"
                                   " tunnels-per-origin-topic: %d"
                                   " origin-topics: %d requests-per-sec: %f"
                                   " batchsize: %d"
                                   " max-opening-conns: %d"
                                   " conns-per-sec: %f\n",
                                   parms->servername,
                                   parms->port,
                                   parms->tunnels_per_journal * parms->journals_per_topic,
                                   parms->origin_topics,
                                   parms->requests_per_sec, 
                                   parms->batchsize,
                                   parms->max_opening_conns,
                                   parms->conns_per_sec);
    if (!ds) abort();
    if (!pipe_fitting_write(pf, ds->start, ds->len)) abort();
    dstring_free(ds);
}

void run_master(struct masterparms *parms) {
    evl_t *evl;
    connqueue_t *cq;
    pipe_fitting_t *output;
    struct sockaddr_in sin;
    int sessionid = time(NULL);
    int post_serial = 0;
    double now, next_post_time, start_time, end_time;
    double posting_interval = 1.0 / parms->requests_per_sec;
    int total_tunnels = (parms->journals_per_topic * parms->origin_topics 
                         * parms->tunnels_per_journal);

    fprintf(stderr, "Running master.\n");
    signal(SIGPIPE, SIG_IGN);

    if (!make_sockaddr_in_byname(&sin, parms->servername, parms->port)) return;

    evl = evl_init();
    if (!evl) {
        perror("creating event loop");
        return;
    }

    output = new_pipe_fitting(evl, 0, 1);
    if (!output) {
        perror("creating output pipe fitting");
        evl_cleanup(evl);
        return;
    }

    dump_parms(parms, output);

    cq = new_connqueue(evl, parms->max_opening_conns, parms->conns_per_sec);
    if (!cq) {
        perror("creating connection queue");
        pipe_fitting_free(output);
        evl_cleanup(evl);
        return;
    }

    /* establish a number of tunnel connections to the PubSub Server */
    establish_tunnel_connections(cq, &sin, parms, sessionid, output);
    /* run the event loop until all the tunnels are open */
    /* wait for status-events to settle down */
    while (tunnels < total_tunnels) 
    {
        evl_poll(evl, -1.0);
    }
    fprintf(stderr, "Tunnels open; establishing routes...\n");
    /* set up a number of routes on the PubSub Server */
    remaining_routes = parms->journals_per_topic * parms->origin_topics;
    establish_routes(cq, &sin, parms, sessionid);
    while (remaining_routes) 
    {
        evl_poll(evl, -1.0);
    }
    fprintf(stderr, "routes created; posting events...\n");
    /* set up stuff to make events get posted to the PubSub Server */
    start_time = gettimeofday_double(); 
    end_time = start_time + parms->runfor;
    next_post_time = start_time; /* i.e. now */
    /* run the event loop */
    while (tunnels || !connqueue_is_empty(cq)) {
        double interval = next_post_time - gettimeofday_double();
        if (interval < 0.0) interval = 0.0;
        if (!evl_poll(evl, interval)) {
            perror("event loop");
            break;
        }
        now = gettimeofday_double();
        if (now > next_post_time) {
            enqueue_another_post(evl, cq, &sin, parms, sessionid, 
                                 post_serial++, 0);
            next_post_time = now + posting_interval;
        }
        /* FIXME: bad hack to make it exit */
        if (parms->runfor != 0.0 && now > end_time) exit(0);
    }
    /* clean up and exit */
    connqueue_free(cq);
    pipe_fitting_free(output);
    evl_cleanup(evl);
}

