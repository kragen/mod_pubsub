/**
 * pocketnocd.c - v1.5
 * Author: Paul Sharpe
 *
 * Contains the main function for PocketNOC.
 * All libkn event posting is handled in this file.
 **/

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
 **/

#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/loadavg.h>
#include <sys/systeminfo.h>
#include <sys/types.h>
#include <sys/stat.h>

/* needed for CreateSysDictionary() */
#include "sysinfo.h"

#ifndef VERSION
#define VERSION "-dev-"
#endif

/*
 * PrintUsage(void)
 *
 * Prints copyright and usage info
 *
 */

static void PrintUsage(void)
{
  printf("PocketNOC version " VERSION "\nUsage: nocd -r router_uri [-u updates/sec]\n");
}


/**
 * Run in daemon mode.
 */

int daemon_init(void)
{
  pid_t pid;

  if((pid = fork()) < 0)
    return(-1);
  else if(pid != 0)
    exit(0);

  setsid();

  chdir("/");

  umask(0);

  return(0);
}

/*
 * main(int argc, char **argv)
 *
 * The main function that handles the posting of statistics to the
 * PubSub Server passed by the -r option.  Can also be told how often
 * to update stats w/ -u.
 *
 */

int main(int argc, char **argv)
{
  int options;           /* stores getopt() return status */
  extern char *optarg;   /* contains argument passed to an option */
  extern int optind;     /* stores the argv index of the next arg */

  long hostRes = 0;
  long count = 257;
  char hostname[257];
  char rateBuf[10];
  char expiresBuf[20];
  char *tmpTopic = NULL;
  char *routerUrl = NULL;  /* initial value of the router url */
  int updateRate = 5;      /* default value of the update rate */

  knserv_ref knServer;
  knevent_mref knEvent;
  knstr_ref uri, topic, rate, expires;

  /* Parse args and get options.  -r for pubsub server url
   * and -u for update rate.  Anything else will print a usage message
   */

  while ((options = getopt(argc, argv, "r:u:")) != EOF) {
        switch (options) {
        case 'r':
            routerUrl = optarg;  /* store arg in routerUrl */
            break;
        case 'u':
            updateRate = atoi(optarg); /* convert str to int and store */
            break;
        default:
            PrintUsage();
            exit(1);
            break;
        }
  }

      
  /* Check to make sure a pubsub server url was passed,
   * if not print usage message.
   */

  if(routerUrl == NULL) {
        PrintUsage();
        exit(1);
  }


  /* Get hostname from sysinfo and create the tmpTopic string. */

  if((hostRes = sysinfo(SI_HOSTNAME, hostname, count)) != -1) {
        tmpTopic = (char *) malloc(strlen(hostname) + strlen("/what/pocketnoc/"));
        sprintf(tmpTopic, "/what/pocketnoc/%s", hostname);
  } else {
        perror("Can't get hostname.");
        exit(1);
  }


  /* Create the uri, tmpTopic, rate, expires, and knserver using 
   * strings received from the cmd line options.
   */

  sprintf(rateBuf, "%i", updateRate);
  sprintf(expiresBuf, "%i", updateRate * 2);

  uri = knstr_create_with_cstr(routerUrl);
  topic = knstr_create_with_cstr(tmpTopic);
  rate = knstr_create_with_cstr(rateBuf);
  expires = knstr_create_with_cstr(expiresBuf);

  free(tmpTopic);

  knServer = knserv_create(uri, NULL, NULL);

  kn_release(uri);

  knEvent = knevent_mcreate((size_t) 1024);
  
  /* Main while loop, permanently execute until a sigkill
   * or crash occurs.  creates an event, gets sysInfo, 
   * populates event w/ sysInfo, then post.  
   */

  daemon_init();

  while(1) {

    CpuUsage(knEvent);
    MemoryUsage(knEvent);
    SwapUsage(knEvent);
    LoadAvg(knEvent);
    NumUsers(knEvent);

    knevent_set(knEvent, KNSTR("os"), KNSTR("solaris"));
    knevent_set(knEvent, KNSTR("rate"), rate);
    knevent_set(knEvent, KNSTR("kn_expires"), expires);

    kn_publish(knServer, knEvent, topic, NULL, NULL);

    /* Pause for amount of seconds passed as an argument 
     * (5 by default) and then iterate again.
     */
        
    sleep(updateRate);
  }


  /* Release all the other kn_StringRef vars. */

  kn_release(topic);
  kn_release(rate);
  kn_release(expires);
  kn_release(knEvent);
  kn_release(knServer);
}
