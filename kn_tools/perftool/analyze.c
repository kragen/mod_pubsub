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
 * $Id: analyze.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

/*
 * Analyze perftool output.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "analyze_conn.h"
#include "linear.h"

static unused char rcsid[] = "@(#) $Id: analyze.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $";

char *findtag(char *input, char *tag)
{
    char *s = strstr(input, tag);
    if (!s) return 0;
    return s + strlen(tag);
}

double nan()
{
    return 0/0.0;
}

double float_tag(char *input, char *tag)
{
    char *val = findtag(input, tag);
    if (val) return atof(val);
    return nan();
}

int int_tag(char *input, char *tag)
{
    char *val = findtag(input, tag);
    if (val) return atol(val);
    return -1;
}

double latency(char *s)
{
    return float_tag(s, "latency: ");
}

double received(char *s)
{
    return float_tag(s, "received: ");
}

int conn(char *s)
{
    return int_tag(s, "conn: ");
}

long bytes(char *s)
{
    return int_tag(s, "bytes: ");
}

int event_id(char *s)
{
    return int_tag(s, "id: ");
}

double square(double d)  /* incongruous variable decl */
{
    return d * d;
}

/* try to read a line; if it's too long or there's a read error, print
 * an error message and exit.  If you want more sophisticated error
 * handling, it should be easy to add it here.  Returns 0 on eof, 1
 * otherwise.*/
int get_line(char *inbuf, ssize_t size)
{
    if (!fgets(inbuf, size, stdin))
    {
        if (feof(stdin)) /* EOF */
            return 0;
        perror("input read");
        exit(1);
    }
    if (!strchr(inbuf, '\n'))
    {
        fprintf(stderr, "Input line %s too long\n", inbuf);
        exit(1);
    }
    return 1;
}

#define INBUFSIZ 4096
int main(int argc, char **argv) 
{
    char inbuf[INBUFSIZ];
    int lineno = 0;
    double runTime, thisLat, thisTime;
    regression_t *timeVsLatency, *lineVsLatency, *idVsFinalLatency;
    int i;
    int tunnels_per_origin_topic;
    float requests_per_sec;
    int origin_topics, batchsize;
    
    if (argc > 1)
    {
        fprintf(stderr, "Usage: %s < infile\n", argv[0]);
        return 1;
    }
    
    timeVsLatency = new_regression();
    lineVsLatency = new_regression();
    idVsFinalLatency = new_regression();
    
    if (!(timeVsLatency && lineVsLatency && idVsFinalLatency))
    {
        perror("starting up");
        return 1;
    }

    initAnalyzeConn();

    if (!get_line(inbuf, INBUFSIZ))
    {
        fprintf(stderr, "Empty input\n");
        return 1;
    }
    tunnels_per_origin_topic = int_tag(inbuf, "tunnels-per-origin-topic: ");
    requests_per_sec = float_tag(inbuf, "requests-per-sec: ");
    origin_topics = int_tag(inbuf, "origin-topics: ");
    batchsize = int_tag(inbuf, "batchsize: ");

    if (isnan(requests_per_sec) || 
        (tunnels_per_origin_topic == -1) || (origin_topics == -1))
    {
        fprintf(stderr, "First line of input didn't have PubSub Server parameters:\n"
                "\ttunnels-per-origin-topic:\n"
                "\trequests-per-sec: and\n"
                "\torigin-topics:, but instead was\n"
                "%s", inbuf);
        if (conn(inbuf) != -1)
            fprintf(stderr, "Maybe this is from an old version of perftool?\n");
        return 1;
    }

    /* default batchsize for old data files from before batching */
    if (batchsize == -1) batchsize = 1;
    /* and no batching means one event each time */
    if (batchsize == 0) batchsize = 1;

    expectToReceiveEachEventNTimes(tunnels_per_origin_topic);
    for (;;)
    {
        if (!get_line(inbuf, INBUFSIZ)) break;

        lineno++;

        if (-1 != conn(inbuf)) 
        {
            connSetBytes(conn(inbuf), bytes(inbuf));

            thisLat = latency(inbuf);
            thisTime = received(inbuf);

            regression_enter_data(timeVsLatency, thisTime, thisLat);
            regression_enter_data(lineVsLatency, (double)lineno, thisLat);

            connReceivedEvent(conn(inbuf), event_id(inbuf), thisLat);
        }
    }

    if (isnan(regression_xmax(timeVsLatency)))
    {
        printf("No input events\n");
        return 1;
    }
    runTime = regression_xmax(timeVsLatency) - regression_xmin(timeVsLatency);
    printf("Run time was %f seconds.\n", runTime);
    printf("At %f postings of %d event%s per second,\n"
           "should have posted %f requests\n",
           requests_per_sec,
           batchsize,
           (batchsize == 1 ? "" : "s"),
           requests_per_sec * batchsize * runTime);
    printf("so we should expect to have received %f events per connection,\n",
           requests_per_sec * batchsize * runTime / origin_topics);
    printf("or %f events total.\n", 
           requests_per_sec * batchsize * runTime * tunnels_per_origin_topic);
    /* lineno-1 because there are n-1 time intervals between n events */
    printf("Analyzed %d events (%f per second).\n", lineno, 
           (lineno-1)/runTime);
    printf("Latency: min: %f; max: %f; average: %f\n",
           regression_ymin(timeVsLatency), regression_ymax(timeVsLatency), 
           regression_ymean(timeVsLatency));
    printf("Latency started at %f and grew by %f per event (R^2 = %f)\n",
           regression_predicted_y_at_xmin(lineVsLatency), 
           regression_slope(lineVsLatency),
           square(regression_r(lineVsLatency)));
    printf("or started at %f and grew by %f per second (R^2 = %f)\n",
           regression_predicted_y_at_xmin(timeVsLatency),
           regression_slope(timeVsLatency),
           square(regression_r(timeVsLatency)));

    for (i = minEvent(); i <= maxEvent(); i++)
    {
        if (isnan(eventFinalLatency(i)))
        {
            printf("Event %d wasn't received enough times\n", i);
        }
        else
        {
            regression_enter_data(idVsFinalLatency, (double)i, 
                                  eventFinalLatency(i));
        }
    }

    printf("Latency to all connections: min: %f; max: %f; average: %f\n",
           regression_ymin(idVsFinalLatency),
           regression_ymax(idVsFinalLatency),
           regression_ymean(idVsFinalLatency));
    printf("started at %f and grew by %f seconds per event posted\n"
           "(R^2 = %f)\n",
           regression_predicted_y_at_xmin(idVsFinalLatency),
           regression_slope(idVsFinalLatency),
           square(regression_r(idVsFinalLatency)));

    for (i = minConn(); i <= maxConn(); i++) 
    {
        if (connGetBytes(i) == -1)
        {
            printf("!!! Conn %d never got any events\n", i);
        }
        else
        {
            printf("Conn %d received %ld bytes and events ", i, connGetBytes(i));
            printReceivedEvents(i);
            printf(".\n");
        }
    }

    return 0;
}

