/**
 * PubSub Client Library
 * Event data type and routines
 *
 * Wilfredo Sanchez
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

#include "kn_config.h"

RCSID("$Id: kn_Event.c,v 1.3 2003/03/19 05:36:47 ifindkarma Exp $");

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "kn_Object_private.h"
#include "kn_String_WebEncoding.h"
#include "kn_Event.h"

typedef struct __kn_Event {
  __KN_OBJECT_COMMON_FIELDS
  kn_MutableDictionaryRef event_dictionary;
} kn_Event;

/**
 * Allocators
 **/

static void kn_EventDealloc (kn_ObjectRef anObject)
{
  kn_Event* anEvent = (kn_Event*)anObject;

#ifdef KN_DEBUG_REFS
  printf("Dealloc event %p\n", anEvent);
#endif

  kn_Release(anEvent->event_dictionary);

  free(anEvent);
}

static kn_StringRef kn_EventDescribe (kn_ObjectRef anObject)
{
  kn_Event* anEvent = (kn_Event*)anObject;

  return kn_CopyDescription(anEvent->event_dictionary);
}

kn_MutableEventRef kn_EventCreateMutable ()
{
  return kn_EventCreateMutableWithCapacity(0);
}

kn_MutableEventRef kn_EventCreateMutableWithCapacity (size_t aCapacity)
{
  return kn_EventCreateMutableWithDictionary(kn_DictionaryCreateMutableWithCapacity(aCapacity));
}

kn_EventRef kn_EventCreateCopy (kn_EventRef anEvent)
{
  return kn_EventCreateMutableWithDictionary(kn_DictionaryCreateMutableCopy(anEvent->event_dictionary));
}

kn_MutableEventRef kn_EventCreateMutableCopy (kn_EventRef anEvent)
{
  return kn_EventCreateMutableWithDictionary(kn_DictionaryCreateMutableCopy(anEvent->event_dictionary));
}

kn_EventRef kn_EventCreateWithDictionary (kn_DictionaryRef aDictionary)
{
  return kn_EventCreateMutableWithDictionary(aDictionary);
}

kn_MutableEventRef kn_EventCreateMutableWithDictionary (kn_DictionaryRef aDictionary)
{
  kn_Event* anEvent = (kn_Event*)malloc(sizeof(kn_Event));

  if (anEvent)
    {
      kn_object_init(anEvent, kn_EventDealloc, kn_EventDescribe);

      if ((anEvent->event_dictionary = kn_DictionaryCreateMutableCopy(aDictionary)))
        {
          char         aCValue[1024] = ""; /* FIXME: Figure out the real size some time. */
          kn_StringRef aValue;

          /* Huk, huk, huk, blorp! */
          snprintf(aCValue, sizeof(aCValue), "libkn.event_%ld.%ld.%ld",
                   (long int)anEvent->id,
                   (long int)anEvent,
                   (time(NULL) ^ (getpid() << 16)));

          aValue = kn_StringCreateWithCString(aCValue);

          /* Use AddValue, not SetValue, so we don't override a user-specified id. */
          kn_DictionaryAddValue(anEvent->event_dictionary, kn_EVENT_ID_HEADER_NAME, aValue);

          kn_Release(aValue);
        }
      else
        {
          kn_Release(anEvent);
          anEvent = NULL;
        }
    }
  else
    errno = ENOMEM;

  return anEvent;
}

kn_EventRef kn_EventCreateWithSimpleEventFormat (kn_StringRef anEventString)
{
  kn_MutableEventRef anEvent;
  const char*        aBytes  = kn_StringGetBytes(anEventString);
  size_t             aSize   = kn_StringGetSize (anEventString);
  size_t             anIndex = 0;

  /* Sanity test: response should not be empty and should end with a newline. */
  if (!(aBytes[anIndex] && aBytes[aSize-1] == '\n')) { errno = EINVAL; return NULL; }

  /* Move up to the first newline. */
  while (aBytes[anIndex++] != '\n' && anIndex < aSize);

  /* Sanity test: verify the content-length. */
  {
    long int aCount;
#ifdef __GNUC__
    char aCountString[anIndex+1];
#else
    char* aCountString = (char*)malloc(sizeof(char)*(anIndex+1));

    if (!aCountString) return NULL;
#endif

    memmove(aCountString, aBytes, anIndex); aCountString[anIndex] = '\0';

    aCount = strtol(aCountString, NULL, 10);

#ifndef __GNUC__
    free(aCountString);
#endif

    if (aCount != aSize-anIndex-1) { errno = EINVAL; return NULL; }
  }

  /* Reset at beginning of actual content and toss the trailing newline. */
  aBytes += anIndex; aSize -= anIndex+1; anIndex = 0;

  /* Count the number of headers, and allocate an Event with that capacity. */
  {
    size_t aCount = 0;

    while (anIndex < aSize)
      {
        if (aBytes[anIndex++] == '\n') aCount++;
        if (aBytes[anIndex++] == '\n') break;
      }
    anIndex = 0;

    anEvent = kn_EventCreateMutableWithCapacity(aCount+1); /* +1 for payload. */
  }

  while (aBytes[0] != '\n' && aSize > 0) /* Stop at "\n\n" */
    {
      char*        aNext;
      kn_StringRef aName;
      kn_StringRef aValue;

      /* Find the end of the current name. */
      if (!(aNext = memchr(aBytes, ':', aSize)) || aNext[1] != ' ')
        {
          kn_Release(anEvent); anEvent = NULL; errno = EINVAL;
          break;
        }

      aName = kn_StringCreateWithBytesNoCopy(aBytes, aNext-aBytes, kn_FALSE);
      aSize -= (aNext-aBytes)+2; aBytes = aNext+2;

      /* Find the end of the current value. */
      if (!(aNext = memchr(aBytes, '\n', aSize)))
        {
          kn_Release(anEvent); anEvent = NULL; errno = EINVAL;
          kn_Release(aName);
          break;
        }

      aValue = kn_StringCreateWithBytesNoCopy(aBytes, aNext-aBytes, kn_FALSE);
      aSize -= (aNext-aBytes)+1; aBytes = aNext+1;

      /* Decode name and value */
      {
        kn_StringRef aDecodedName  = kn_StringCreateByDecodingQuotedPrintable(aName );
        kn_StringRef aDecodedValue = kn_StringCreateByDecodingQuotedPrintable(aValue);

        /* Add header to event. */
        kn_EventSetValue(anEvent, aDecodedName, aDecodedValue);

        kn_Release(aDecodedName );
        kn_Release(aDecodedValue);
      }

      kn_Release(aName );
      kn_Release(aValue);
    }

  /* Skip the newline. */
  aBytes++; aSize--;

  /* Everything else is the payload. */
  if (anEvent)
    {
      kn_StringRef aValue = kn_StringCreateWithBytes(aBytes, aSize);
      kn_EventSetValue(anEvent, kn_EVENT_PAYLOAD_HEADER_NAME, aValue);
      kn_Release(aValue);
    }

  return anEvent;
}

/**
 * Accessors
 **/

size_t kn_EventGetCount (kn_EventRef anEvent)
{
  return kn_DictionaryGetCount(anEvent->event_dictionary);
}

kn_StringRef kn_EventGetValue (kn_EventRef anEvent, kn_StringRef aName)
{
  return kn_DictionaryGetValue(anEvent->event_dictionary, aName);
}

kn_Error kn_EventGetNamesAndValues (kn_EventRef anEvent, kn_StringRef* aNames, kn_StringRef* aValues)
{
  return kn_DictionaryGetNamesAndValues(anEvent->event_dictionary, aNames, (kn_ObjectRef*)aValues);
}

kn_Error kn_EventSetValue (kn_MutableEventRef anEvent, kn_StringRef aName, kn_StringRef aValue)
{
  return kn_DictionarySetValue(anEvent->event_dictionary, aName, aValue);
}

kn_Error kn_EventRemoveValue (kn_MutableEventRef anEvent, kn_StringRef aName)
{
  return kn_DictionaryRemoveValue(anEvent->event_dictionary, aName);
}

kn_StringRef kn_EventCreateStringByEncodingSimpleFormat (kn_EventRef anEvent)
{
  kn_MutableStringRef anEventString = kn_StringCreateMutable();
  size_t              aCount        = kn_DictionaryGetCount(anEvent->event_dictionary);
  size_t              aSize         = 0;
#ifndef __GNUC__
  kn_StringRef*       aNames        = (kn_StringRef*)malloc(sizeof(kn_StringRef)*aCount);
  kn_StringRef*       aValues       = (kn_StringRef*)malloc(sizeof(kn_StringRef)*aCount);
#else
  kn_StringRef        aNames [aCount];
  kn_StringRef        aValues[aCount];
#endif
  size_t              anIndex;

  kn_DictionaryGetNamesAndValues(anEvent->event_dictionary, aNames, (kn_ObjectRef*)aValues);

  for (anIndex = 0; anIndex < aCount; anIndex++)
    {
      if (kn_StringCompare(aNames[anIndex], kn_EVENT_PAYLOAD_HEADER_NAME))
	{
	  aSize += kn_StringGetSize(aNames [anIndex]) +
	           2                                  + /* ": " */
	           kn_StringGetSize(aValues[anIndex]) +
	           1                                  ; /* "\n" */
	}
      else
	aSize += kn_StringGetSize(aValues[anIndex]);
    }

  {
#ifdef __GNUC__
    char  aContentCLength[(aSize/10)+1];
#else
    char* aContentCLength = (char*)malloc(sizeof(char)*((aSize/10)+1));
#endif
    kn_StringRef aContentLength;

    aSize += 1; /* "\n" after headers */

    sprintf(aContentCLength, "%ld\n", (long int)aSize);

    aContentLength = kn_StringCreateWithCString(aContentCLength);

    aSize += kn_StringGetSize(aContentLength) + 2; /* "\n" before and after event data */

    kn_StringAppendString(anEventString, aContentLength);

    kn_Release(aContentLength);

#ifndef __GNUC__
    free(aContentCLength);
#endif
  }

  {
    kn_StringRef aPayload = NULL;

    for (anIndex = 0; anIndex < aCount; anIndex++)
      {
	if (kn_StringCompare(aNames[anIndex], kn_EVENT_PAYLOAD_HEADER_NAME))
	  {
	    kn_StringAppendString (anEventString, aNames [anIndex]);
	    kn_StringAppendCString(anEventString, ": "            );
	    kn_StringAppendString (anEventString, aValues[anIndex]);
	    kn_StringAppendCString(anEventString, "\n"            );
	  }
	else
	  aPayload = aValues[anIndex];
      }

    kn_StringAppendCString(anEventString, "\n"    );
    kn_StringAppendString (anEventString, aPayload);
    kn_StringAppendCString(anEventString, "\n"    );

    kn_Release(aPayload);
  }

#ifndef __GNUC__
  free(aNames );
  free(aValues);
#endif

  return anEventString;
}
