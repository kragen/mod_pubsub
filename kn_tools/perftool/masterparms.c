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
 * $Id: masterparms.c,v 1.2 2003/05/06 04:42:16 bsittler Exp $
 **/

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "master.h"
#include "getopt.h"
#include "nan.h"

static unused char rcsid[] = "@(#) $Id: masterparms.c,v 1.2 2003/05/06 04:42:16 bsittler Exp $";

static void usage(char *argv0) {
    fprintf(stderr, ("Usage: %s [--master=hostname] [-m hostname]\n"
                     "          [--origin-topics=n] [-o n]\n"
                     "          [--journals=m] [-j m]\n"
                     "          [--tunnels=t] [-t t]\n"
                     "          [--topic=/topicname] [-T /topicname]\n"
                     "          [--run-for=nsec] [-r nsec]\n"
                     "          [--debug] [-d]\n"
                     "          [--payloadsize n] [-p n]\n"
                     "          [--simple-format] [-f]\n"
                     "          [--batchsize n] [-b n]\n"
		     "          [--max-opening-conns n] [-c n]\n"
		     "          [--conns-per-sec n] [-C n]\n"
                     "          request_per_sec servername port\n"
                     "or %s --slave or %s -s\n"),
            argv0, argv0, argv0);
    exit(1);
}

static struct option options[] = {
    {"master",         required_argument,  NULL,  'm'},
    {"origin-topics",  required_argument,  NULL,  'o'},
    {"journals",       required_argument,  NULL,  'j'},
    {"tunnels",        required_argument,  NULL,  't'},
    {"topic",          required_argument,  NULL,  'T'},
    {"run-for",        required_argument,  NULL,  'r'},
    {"slave",          no_argument,        NULL,  's'},
    {"debug",          no_argument,        NULL,  'd'},
    {"payloadsize",    required_argument,  NULL,  'p'},
    {"simple-format",  no_argument,        NULL,  'f'},
    {"batchsize",      required_argument,  NULL,  'b'},
    {"max-opening-conns", required_argument, NULL, 'c'},
    {"conns-per-sec",  required_argument,  NULL,  'C'},
    {0,                0,                  0,     0}
};


static int parse_int(char *num, int *where) {
    char *finish;
    int rv = strtol(num, &finish, 10);
    if (*finish != '\0') return 0;  /* parsing failed */
    if (finish == num) return 0;    /* empty string */
    *where = rv;
    return 1;
}

static int parse_float(char *num, float *where) {
    char *finish;
    float rv = strtod(num, &finish);
    if (*finish != '\0') return 0;
    if (finish == num) return 0;
    *where = rv;
    return 1;
}

int parse_masterparms(int argc, char **argv, struct masterparms *parms) 
{
    int opt;
    int option_index = 0;

    /* defaults */
    parms->origin_topics = 1;
    parms->journals_per_topic = 1;
    parms->tunnels_per_journal = 1;
    parms->basetopic = "/what/apps/perftool";
    parms->runfor = 0.0;
    parms->debug = 0;
    parms->payloadsize = 0;
    parms->simple_format = 0;
    parms->batchsize = 0;
    parms->max_opening_conns = 120;
    parms->conns_per_sec = return_nan();

    for (;;) {
        opt = getopt_long(argc, argv, "m:c:C:o:j:t:T:r:sdfp:b:", 
                          options, &option_index); 
        switch (opt)
        {
        case '?': usage(argv[0]); break;
        case 'm': add_slave(optarg); break;
        case 'c': 
            if (!parse_int(optarg, &parms->max_opening_conns)) 
                usage(argv[0]); 
            break;
        case 'C': 
            if (!parse_float(optarg, &parms->conns_per_sec)) 
                usage(argv[0]); 
            break;
        case 'o': 
            if (!parse_int(optarg, &parms->origin_topics)) 
                usage(argv[0]); 
            break;
        case 'j': 
            if (!parse_int(optarg, &parms->journals_per_topic)) 
                usage(argv[0]); 
            break;
        case 't': 
            if (!parse_int(optarg, &parms->tunnels_per_journal)) 
                usage(argv[0]); 
            break;
        case 'T': parms->basetopic = optarg; break;
        case 'r': 
            if (!parse_float(optarg, &parms->runfor)) 
                usage(argv[0]); 
            break;
        case 's': 
            if (optind != argc) usage(argv[0]);
            return 0; /* a slave */
        case 'd': parms->debug = 1; break;
        case 'f': parms->simple_format = 1; break;
        case 'p':
            if (!parse_int(optarg, &parms->payloadsize))
                usage(argv[0]);
            break;
        case 'b':
            if (!parse_int(optarg, &parms->batchsize))
                usage(argv[0]);
            break;
        case -1: 
            if (optind != argc - 3) usage(argv[0]);
            if (!parse_float(argv[optind], &parms->requests_per_sec)) 
                usage(argv[0]);
            parms->servername = argv[optind + 1];
            if (!parse_int(argv[optind + 2], &parms->port)) usage(argv[0]);
            return 1;  /* a master */
        default: fprintf(stderr, "got %d '%c' from getopt\n", opt, opt); break;
        }
    }
}
