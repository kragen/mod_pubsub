/**
 * PubSub Client Library tools
 * publish events
 *
 * Wilfredo Sanchez
 **/
/**
 * Copyright (c) 2001-2002 KnowNow, Inc.  All rights reserved.
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

#include "kn_config.h"

RCSID("$Id: kn_publish.c,v 1.1 2002/12/21 03:38:44 bsittler Exp $");

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
	  "usage: kn_publish [-PS] [-D name] [-H name value] [-p proxyHost proxyPort] uri topic file\n"
	  "       -P: input is payload data (default)\n"
	  "       -S: input is a pre-formatted event in \"simple\" format\n"
	  "       -D: delete header from the published event (useful with -S)\n"
	  "       -H: add header name and value the published event\n"
      "       -p: proxyHost and proxyPort \n"
	  );
  exit(EX_USAGE);
}

typedef enum {
  kInputPayload    ,
  kInputSimpleEvent,
} InputType;

static void PrintObject(kn_ObjectRef anObject, FILE* aStream)
{
  kn_StringRef aDescription = kn_CopyDescription(anObject);
  kn_StringWriteToStream(aDescription, aStream);
  kn_Release(aDescription);
}

int main (int argc, char* const argv[])
{
  kn_MutableDictionaryRef anOverridesDict = kn_DictionaryCreateMutable();
  kn_MutableDictionaryRef aRemoveDict     = kn_DictionaryCreateMutable();

  extern char* optarg;
  extern int   optind;

  char opt;
  kn_StringRef proxyHost = NULL;
  int   proxyPort = 0;
  kn_BOOL useProxy = kn_FALSE;

  InputType anInputType = kInputPayload;
  kn_BOOL   aDebugOpt   = kn_FALSE;

  while ((opt = (char)getopt(argc, argv, "dPSD:H:p:")) != -1)
    {
      switch (opt)
	{
	case 'd': aDebugOpt = kn_TRUE; break;

	case 'P': anInputType = kInputPayload    ; break;
	case 'S': anInputType = kInputSimpleEvent; break;

	case 'D':
	  {
	    kn_StringRef aName = kn_StringCreateWithCString(optarg);
	    kn_DictionaryAddValue(aRemoveDict, aName, aName);
	    kn_Release(aName);
	  }
	  break;

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

	      kn_DictionarySetValue(anOverridesDict, aName, aValue);

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
            useProxy = kn_TRUE;
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

  if (argc != 3) usage();

  {
    kn_EventRef         anEvent         = NULL;
    kn_StringRef        aURI            = kn_StringCreateWithCString(argv[0]);
    kn_StringRef        aTopic          = kn_StringCreateWithCString(argv[1]);
    kn_ServerRef        aServer         = kn_ServerCreateWithURI(aURI);
    char*               aFileName       = argv[2];
    int                 aFileDescriptor = STDIN_FILENO;
    kn_MutableStringRef anInput         = kn_StringCreateMutable();
    char                aBuffer[BUFSIZ];
    ssize_t             aRead;

    if (strcmp(aFileName, "-"))
      {
	if ((aFileDescriptor = open(aFileName, O_RDONLY, (mode_t)0)) == -1)
	  {
	    fprintf(stderr, "Cannot open file %s: %s\n", aFileName, strerror(errno));
	    exit(EX_NOINPUT);
	  }
      }
    else
      aFileName = "<stdin>";

	if (useProxy)
    {	
		kn_ServerSetProxy(aServer, proxyHost, proxyPort, NULL, NULL);
		kn_Release(proxyHost);
	}
		
    while ((aRead = read(aFileDescriptor, aBuffer, sizeof(aBuffer))) > 0)
      {
	kn_StringRef anAppend = kn_StringCreateWithBytesNoCopy(aBuffer, aRead, kn_FALSE);
	kn_StringAppendString(anInput, anAppend);
	kn_Release(anAppend);
      }

    if (aRead == -1)
      {
	fprintf(stderr, "Error reading file %s: %s\n", aFileName, strerror(errno));
	exit(EX_NOINPUT);
      }

    switch (anInputType)
      {
      case kInputPayload:
	{
	  kn_DictionarySetValue(anOverridesDict, kn_EVENT_PAYLOAD_HEADER_NAME, anInput);

	  anEvent = kn_EventCreateWithDictionary(anOverridesDict);
	}
	break;

      case kInputSimpleEvent:
	{
	  size_t anOverrideCount = kn_DictionaryGetCount(anOverridesDict);
	  size_t aRemoveCount    = kn_DictionaryGetCount(aRemoveDict    );

	  if (aDebugOpt)
	    {
	      fprintf(stderr, "Overrides: "); PrintObject(anOverridesDict, stderr); fprintf(stderr, "\n");
	      fprintf(stderr, "Removes: "  ); PrintObject(aRemoveDict    , stderr); fprintf(stderr, "\n");
	    }

	  if (anOverrideCount || aRemoveCount)
	    {
#ifdef __GNUC__
	      kn_StringRef aNames [(anOverrideCount>aRemoveCount)?anOverrideCount:aRemoveCount];
	      kn_StringRef aValues[anOverrideCount];
#else
	      kn_StringRef* aNames  = (kn_StringRef*)malloc(sizeof(kn_StringRef)*((anOverrideCount>aRemoveCount)?anOverrideCount:aRemoveCount));
	      kn_StringRef* aValues = (kn_StringRef*)malloc(sizeof(kn_StringRef)*anOverrideCount);
#endif
	      kn_EventRef  aReadEvent = kn_EventCreateWithSimpleEventFormat(anInput);

	      if (aReadEvent)
		{
		  kn_MutableEventRef anEditedEvent = kn_EventCreateMutableCopy(aReadEvent);

		  if (aDebugOpt)
		    { fprintf(stderr, "Input Event: "); PrintObject(aReadEvent, stderr); fprintf(stderr, "\n"); }

		  kn_Release(aReadEvent);

		  kn_DictionaryGetNamesAndValues(anOverridesDict, aNames, (kn_ObjectRef*)aValues);

		  while (anOverrideCount--)
		    kn_EventSetValue(anEditedEvent, aNames[anOverrideCount], aValues[anOverrideCount]);

		  kn_DictionaryGetNamesAndValues(aRemoveDict, aNames, NULL);

		  while (aRemoveCount--)
		    {
		      printf("Remove %s\n", kn_StringGetBytes(aNames[aRemoveCount]));
		      kn_EventRemoveValue(anEditedEvent, aNames[aRemoveCount]);
		    }

		  anEvent = anEditedEvent;

#ifndef __GNUC__
		  free(aNames );
		  free(aValues);
#endif
		}
	    }
	  else
	    anEvent = kn_EventCreateWithSimpleEventFormat(anInput);
	}
	break;

      } /* switch(anInputType) */

    if (!anEvent)
      {
	fprintf(stderr, "Unable to parse simple event format input: %s\n", strerror(errno));
	exit(EX_DATAERR);
      }

    if (aDebugOpt)
      {
	fprintf(stderr, "Topic: %s\n", kn_StringGetBytes(aTopic));
	fprintf(stderr, "Server: "); PrintObject(aServer, stderr); fprintf(stderr, "\n");
	fprintf(stderr, "Event: " ); PrintObject(anEvent, stderr); fprintf(stderr, "\n");
      }

    if (kn_ServerPublishEventToTopic(aServer, anEvent, aTopic, NULL, NULL, kn_TRUE))
      exit(EX_IOERR);

    kn_Release(aTopic );
    kn_Release(anEvent);
  }

  exit(0);
}
