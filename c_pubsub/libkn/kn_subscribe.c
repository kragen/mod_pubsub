/**
 * PubSub Client Library tools
 * subscribe to topic
 *
 * Wilfredo Sanchez
 **/
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
 **/

#include "kn_config.h"

RCSID("$Id: kn_subscribe.c,v 1.5 2004/04/19 05:39:08 bsittler Exp $");

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <kn/kn.h>

static int usage ()
{
  fprintf(stderr,
	  "usage: kn_subscribe [-fPS] [-H name value] [-p proxyHost proxyPort] uri topic [topic]\n"
	  "       -f: don't exit after receipt of first event; continue printing events\n"
	  "       -P: output event payload only (default)\n"
	  "       -S: output event in \"simple\" format\n"
	  "       -H: specify header name and value to add to the subscription event\n"
          "       -p: specify proxy setting as proxy host and proxy port\n"
	  );
  exit(EX_USAGE);
}

typedef enum {
  kOutputPayload    ,
  kOutputSimpleEvent,
} OutputType;

static void PrintObject(kn_ObjectRef anObject, FILE* aStream)
{
  kn_StringRef aDescription = kn_CopyDescription(anObject);
  kn_StringWriteToStream(aDescription, aStream);
  kn_Release(aDescription);
}

static void print_event (kn_EventRef anEvent, void* aUserData)
{
  OutputType anOutputType = *(OutputType*)aUserData;

  switch (anOutputType)
    {
    case kOutputPayload:
      {
	kn_StringRef aPayload = kn_EventGetValue(anEvent, kn_EVENT_PAYLOAD_HEADER_NAME);
	kn_StringWriteToStream(aPayload, stdout);
	kn_Release(aPayload);
	fflush(stdout);
      }
      break;

    case kOutputSimpleEvent:
      {
	kn_StringRef anEventString = kn_EventCreateStringByEncodingSimpleFormat(anEvent);
	kn_StringWriteToStream(anEventString, stdout);
	kn_Release(anEventString);
      }
      break;
    }
}

int main (int argc, char* const argv[])
{
  kn_MutableDictionaryRef anOptions = kn_DictionaryCreateMutable();

  extern char* optarg;
  extern int   optind;

  int opt;

  kn_StringRef proxyHost = NULL;
  int proxyPort = -1;

  kn_BOOL	useProxy = kn_FALSE;

  OutputType anOutputType = kOutputPayload;
  kn_BOOL    aDebugOpt    = kn_FALSE;
  kn_BOOL    aContinueOpt = kn_FALSE;

  while ((opt = getopt(argc, argv, "dfPSH:p:")) != -1)
    {
      switch (opt)
	{
	case 'd': aDebugOpt    = kn_TRUE; break;
	case 'f': aContinueOpt = kn_TRUE; break;

	case 'P': anOutputType = kOutputPayload    ; break;
	case 'S': anOutputType = kOutputSimpleEvent; break;

	case 'H':
	  if (optind >= argc)
	    {
	      fprintf(stderr, "%s: option requires two arguments -- %c\n", argv[0], (char)opt);
	      usage();
	    }
	  else
	    {
	      kn_StringRef aName  = kn_StringCreateWithCString(optarg      );
	      kn_StringRef aValue = kn_StringCreateWithCString(argv[optind]);

	      kn_DictionarySetValue(anOptions, aName, aValue);

	      kn_Release(aName );
	      kn_Release(aValue);

	      optind++;
	    }
	  break;

        case 'p':
          if (optind >= argc)
            {
              fprintf(stderr, "%s: option requires two arguments -- %c\n", argv[0], (char)opt);
              usage();
            }
          else
            {
              proxyHost = kn_StringCreateWithCString(optarg);
              proxyPort = atoi(argv[optind]);
              useProxy  = kn_TRUE;
              optind++;
            }
          break;

	default:
	  fprintf(stderr, "%s: unknown option %c\n", argv[0], (char)opt);
	  usage();
	}
    }
  argc -= optind;
  argv += optind;

  if (argc < 2 || argc > 3) usage();

  {
    kn_StringRef aURI    = kn_StringCreateWithCString(argv[0]);
    kn_StringRef aTopic  = kn_StringCreateWithCString(argv[1]);
    kn_ServerRef aServer = kn_ServerCreateWithURI(aURI);
    kn_RouteRef  aRoute  = NULL;

    if (aDebugOpt)
      fprintf(stderr, "Topic: %s\n", kn_StringGetBytes(aTopic));

    kn_Release(aURI);

    if (useProxy)
      {
        kn_ServerSetProxy(aServer, proxyHost, proxyPort, NULL, NULL);
        kn_Release(proxyHost);
      }

    if (aDebugOpt)
      { fprintf(stderr, "Server: "); PrintObject(aServer, stderr); fprintf(stderr, "\n"); }

    if (argc > 2)
      {
	kn_StringRef aDestination = kn_StringCreateWithCString(argv[2]);

	if (!(aRoute = kn_RouteCreateFromTopicToTopicViaServer(aTopic, aDestination, aServer, anOptions, NULL, &anOutputType, kn_TRUE)))
	  {
	    fprintf(stderr, "Failed to create route from %s to %s: %s\n",
		    kn_StringGetBytes(aTopic), kn_StringGetBytes(aDestination),
		    strerror(errno));
	    exit(EX_PROTOCOL);
	  }
      }
    else
      {
	if (!(aRoute = kn_RouteCreateFromTopicToFunctionViaServer(aTopic, print_event, aServer, anOptions, NULL, &anOutputType, kn_TRUE)))
	  {
	    fprintf(stderr, "Failed to create route from %s to local function: %s\n",
		    kn_StringGetBytes(aTopic),
		    strerror(errno));
	    exit(EX_PROTOCOL);
	  }
      }

    if (aDebugOpt)
      { fprintf(stderr, "Route: "); PrintObject(aRoute, stderr); fprintf(stderr, "\n"); }

    kn_Release(aTopic);
    kn_Release(aRoute);

    if (argc == 2)
      {
	if (aContinueOpt)
	  while (1)
	    {
	      if (kn_ServerProcessNextEvent(aServer) != kn_SUCCESS)
		{
		  fprintf(stderr, "Error reading event stream: %s\n", strerror(errno));
		  exit(1);
		}
	    }
	else
	  kn_ServerProcessNextEvent(aServer);
      }

    kn_Release(aServer);
  }

  exit(0);
}
