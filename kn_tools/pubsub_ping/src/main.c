/**
 * pubsub_ping - send events to a PubSub Server
 **/

/**
 * Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
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
 * $Id: main.c,v 1.3 2004/04/19 05:39:15 bsittler Exp $
 **/

#ifndef VERSION
#define VERSION \
"devel"
#endif

/* for printf(), getenv(), etc. */
#include <stdio.h>
#include <stdlib.h>

/* for my getopt() re-implementation */
#include "../my_getopt/getopt.h"

/* pubsub_ping interface */
#include "../include/pubsub_ping.h"

/* print version and copyright information */
static void
version(char *progname)
{
  printf("%s version %s\n",
           progname,
           VERSION);
  fflush(stdout);
}

/* print a help summary */
static void
help(char *progname, PingOptions *options)
{
    printf("Usage: %s [options] URL [TOPIC]\n"
           "Options:\n"
           "-c or\n"
           "-limit                  stop after sending (and receiving) 15 events\n"
           "-cNUM or\n"
           "-limit=NUM              stop after sending (and receiving) NUM events\n"
           "-h or -help             show this message and exit\n"
           "-i NUM or\n"
           "-wait=NUM               wait NUM seconds between sending each event\n"
           "-p STRING or\n"
           "-pattern=STRING         you may specify a \"pad\" STRING to fill the event payload;\n"
           "                        characters in this STRING should be converted to UTF-8 and\n"
           "                        then URL-encoded (ASCII alphanumeric characters do not need\n"
           "                        to be escaped)\n"
           "-P FILE or\n"
           "-payload=FILE           you may compose the complete event payload in a FILE; the FILE\n"
           "                        should be in UTF-8 (the special FILE \"-\" represents stdin;\n"
           "                        this option overrides the -size and -pattern options)\n"
           "-q or -quiet            quiet output -- nothing is displayed except the summary lines\n"
           "                        at startup time and when finished\n"
           "-s NUM or\n"
           "-size=NUM               specifies the payload size in bytes\n"
           "-v or -verbose          increase the level of verbosity by 1\n"
           "-vNUM or\n"
           "-verbose=NUM            set the level of verbosity to NUM\n"
           "-V or -version          print program version and exit\n"
           "-w NUM or\n"
           "-maxwait=NUM            wait at most NUM seconds for an event before publishing the\n"
           "                        next one\n"
           "\n"
           "This program repeatedly publishes events with payloads of a given *size*\n"
           "to a given *TOPIC* on a PubSub Server with a given *URL*, pausing\n"
           "at least *wait* seconds and no more than *maxwait* seconds between\n"
           "publications. Optionally, the event stream can be stopped after the\n"
           "number of events has reached a given *limit*.\n",
           progname
        );
    printf("\n"
           "Current Settings:\n");
    printf("-limit=%lu ",
           options -> limit);
    printf("-wait=%g ",
           options -> wait);
    printf("-maxwait=%g ",
           options -> maxwait);
    printf("-verbose=%u ",
           options -> verbose);
    if (options -> quiet)
        printf("-quiet ");
    printf("-size=%ld ",
           (long) options -> size);
    printf("-pattern=");
    fputs_quoted(options -> pattern, stdout);
    printf("\n");
    fflush(stdout);
}

/* print usage information to stderr */
static void
usage(char *progname)
{
    fprintf(stderr,
            "Summary: %s [-help] [-version] [options] URL [TOPIC]\n",
            progname);
    fflush(stderr);
}

/* FIXME: this should support the standard 'http_proxy' environment variable */

/* argument parser and dispatcher */
int
main(int argc, char * argv[])
{
    /* the program name */
    char *progname = argv[0];
    /* during argument parsing, opt contains the return value from getopt() */
    int opt;
    /* the default return value is initially 0 (success) */
    int retval = 0;
    /* options for this pubsub_ping run */
    PingOptions *options;
    /* server URL */
    char *url;
    /* topic URL */
    char *topic = 0;
    /* payload buffer */
    char *payload = 0;

    /* short options string */
    char *shortopts = "c::hi:p:P:qs:v::Vw:";
    /* long options list */
    struct option longopts[] =
    {
        /* name,        has_arg,           flag, val */ /* longind */
        { "limit",      optional_argument, 0,    'c' }, /*       0 */
        { "help",       no_argument,       0,    'h' }, /*       1 */
        { "wait",       required_argument, 0,    'i' }, /*       2 */
        { "pattern",    required_argument, 0,    'p' }, /*       3 */
        { "payload",    required_argument, 0,    'P' }, /*       4 */
        { "quiet",      no_argument,       0,    'q' }, /*       5 */
        { "size",       required_argument, 0,    's' }, /*       6 */
        { "verbose",    optional_argument, 0,    'v' }, /*       7 */
        { "version",    no_argument,       0,    'V' }, /*       8 */
        { "maxwait",    required_argument, 0,    'w' }, /*       9 */
        /* end-of-list marker */
        { 0, 0, 0, 0 }
    };
    /* long option list index */
    int longind = 0;

    /* allocate and initialize options for this pubsub_ping run */
    if (! (options = newPingOptions()))
    {
        perror("newPingOptions");
        return 1;
    }

    /* 
     * print a warning when the POSIXLY_CORRECT environment variable will
     * interfere with argument placement
     */
    if (getenv("POSIXLY_CORRECT"))
    {
        fprintf(stderr,
                "%s: "
                "Warning: implicit argument reordering disallowed by "
                "POSIXLY_CORRECT\n",
                progname);
	fflush(stderr);
    }

    /* parse all options from the command line */
    while ((opt =
            getopt_long_only(argc, argv, shortopts, longopts, &longind)) != -1)
        switch (opt)
        {
        case 0: /* a long option without an equivalent short option */
            switch (longind)
            {
            default: /* something unexpected has happened */
                fprintf(stderr,
                        "%s: "
                        "getopt_long_only unexpectedly returned %d for `--%s'\n",
                        progname,
                        opt,
                        longopts[longind].name);
		fflush(stderr);
                return 1;
            }
            break;
        case 'c': /* -limit[=NUM] */
        {
            if (optarg)
            {
                /* we use this while trying to parse a numeric argument */
                char ignored;
                if (sscanf(optarg,
                           "%lu%c",
                           &options -> limit,
                           &ignored) != 1)
                {
                    fprintf(stderr,
                            "%s: "
                            "`%s' is not a valid number of events to transmit\n",
                            progname,
                            optarg);
		    fflush(stderr);
                    usage(progname);
                    return 2;
                }
            }
            else
                options -> limit = 15;
            break;
        }
        case 'h': /* -help */
            help(progname, options);
            return 0;
        case 'i': /* -wait=NUM */
        {
            /* we use this while trying to parse a numeric argument */
            char ignored;
            if (sscanf(optarg,
                       "%lf%c",
                       &options -> wait,
                       &ignored) != 1)
            {
                fprintf(stderr,
                        "%s: "
                        "minimum inter-event interval `%s' is not a number\n",
                        progname,
                        optarg);
		fflush(stderr);
                usage(progname);
                return 2;
            }
            break;
        }
        case 'p': /* -pattern=STRING */
            options -> pattern = optarg;
            break;
        case 'P': /* -payload=FILE */
        {
            /* open FILE */
            /* read it and URL-escape it into a buffer of adequate
               size, keeping a length count for the original
               (non-escaped) file */
            /* close FILE */
            fprintf(stderr,
                    payload ? "" : "-payload: NOT IMPLEMENTED\n");
	    fflush(stderr);
            exit(1);
        }
        break;
        case 'q': /* -quiet */
            options -> quiet = 1;
            break;
        case 's': /* -size=NUM */
        {
            /* we use this while trying to parse a numeric argument */
            char ignored;
            long scan_sz;
            if (sscanf(optarg,
                       "%ld%c",
                       &scan_sz,
                       &ignored) != 1)
            {
                fprintf(stderr,
                        "%s: "
                        "payload size `%s' is not a number\n",
                        progname,
                        optarg);
		fflush(stderr);
                usage(progname);
                return 2;
            }
            options -> size = scan_sz;
            if (options -> size < 0)
            {
                fprintf(stderr,
                        "%s: "
                        "payload size `%s' is negative\n",
                        progname,
                        optarg);
		fflush(stderr);
                usage(progname);
                return 2;
            }
            break;
        }
        case 'v': /* -verbose[=NUM] */
            if (optarg)
            {
                /* we use this while trying to parse a numeric argument */
                char ignored;
                if (sscanf(optarg,
                           "%u%c",
                           &options -> verbose,
                           &ignored) != 1)
                {
                    fprintf(stderr,
                            "%s: "
                            "verbosity level `%s' is not a number\n",
                            progname,
                            optarg);
		    fflush(stderr);
                    usage(progname);
                    return 2;
                }
            }
            else
                options -> verbose ++;
            break;
        case 'V': /* -version */
            version(progname);
            return 0;
        case 'w': /* -maxwait=NUM */
        {
            /* we use this while trying to parse a numeric argument */
            char ignored;
            if (sscanf(optarg,
                       "%lf%c",
                       &options -> maxwait,
                       &ignored) != 1)
            {
                fprintf(stderr,
                        "%s: "
                        "maximum inter-event interval `%s' is not a number\n",
                        progname,
                        optarg);
		fflush(stderr);
                usage(progname);
                return 2;
            }
            break;
        }
        case '?': /* getopt_long_only noticed an error */
            usage(progname);
            return 2;
        default: /* something unexpected has happened */
            fprintf(stderr,
                    "%s: "
                    "getopt_long_only returned an unexpected value (%d)\n",
                    progname,
                    opt);
	    fflush(stderr);
            return 1;
        }

    /* shift past option arguments */
    argc -= optind;
    argv += optind;

    if (argc < 1)
    {
        fprintf(stderr,
                "%s: missing required URL argument\n",
                progname
            );
	fflush(stderr);
        usage(progname);
        return 2;
    }

    url = *argv;

    /* shift past server URL */
    argv ++, argc --;

    if (argc > 0)
    {
        topic = *argv;

        /* shift past topic URL */
        argv ++, argc --;
    }

    if (argc)
    {
        fprintf(stderr,
                "%s: too many arguments\n",
                progname
            );
	fflush(stderr);
        usage(progname);
        return 2;
    }

    if (options -> verbose)
    {
        fprintf(stderr,
                "%s: verbosity level is %u, URL is \"%s\", TOPIC is \"%s\"\n",
                progname,
                options -> verbose,
                url,
                topic ? topic : "(null)"
            );
	fflush(stderr);
    }

    if (options -> verbose > 3)
    {
        fprintf(stderr,
                "\topterr: %d\n\toptind: %d\n\toptopt: %d (%c)\n\toptarg: %s\n",
                opterr,
                optind,
                optopt, optopt,
                optarg ? optarg : "(null)");
	fflush(stderr);
    }

    if (! retval)
    {
        retval = pubsub_ping(options, url, topic);
    }

    if (options -> verbose > 3)
    {
        fprintf(stderr,
                "%s: normal return, exit code is %d\n",
                progname,
                retval);
		fflush(stderr);
    }

    deletePingOptions(options);

    return retval;
}
