/**
 * PubSub Client Library
 * Route data type and routines
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

RCSID("$Id: kn_Route.c,v 1.2 2003/03/07 06:16:08 wsanchez Exp $");

#include <stdlib.h>
#include <errno.h>

#include "kn_Object_private.h"
#include "kn_String_URI.h"
#include "kn_Server_private.h"
#include "kn_Route.h"

typedef struct __kn_Route {
  __KN_OBJECT_COMMON_FIELDS
  kn_StringRef route_id;
  kn_ServerRef server;
  kn_StringRef topic;
  kn_StringRef location;
  kn_BOOL      delete_on_dealloc;
} kn_Route;

/**
 * Allocators
 **/

static void kn_RouteDealloc (kn_ObjectRef anObject)
{
  kn_Route* aRoute = (kn_Route*)anObject;

#ifdef KN_DEBUG_REFS
  printf("Dealloc route %p\n", aRoute);
#endif

  if (aRoute->delete_on_dealloc) kn_RouteDelete(aRoute, NULL, NULL, kn_FALSE);

  kn_Release(aRoute->server  );
  kn_Release(aRoute->topic   );
  kn_Release(aRoute->location);

  free(aRoute);
}

static kn_StringRef kn_RouteDescribe (kn_ObjectRef anObject)
{
  kn_Route*           aRoute       = (kn_Route*)anObject;
  kn_MutableStringRef aDescription = kn_StringCreateMutable();
  char                aStart[1024] = "";

  sprintf(aStart, "<kn_Route: %p; server = <kn_Server %p>; id = ", aRoute, aRoute->server);

  kn_StringAppendCString(aDescription, aStart             );
  kn_StringAppendString (aDescription, aRoute->route_id   );
  kn_StringAppendCString(aDescription, "; topic = \""     );
  kn_StringAppendString (aDescription, aRoute->topic      );
  kn_StringAppendCString(aDescription, "\"; location = \"");
  kn_StringAppendString (aDescription, aRoute->location   );
  kn_StringAppendCString(aDescription, "\">"              );

  return aDescription;
}

static kn_EventRef kn_EventCreateRouteEvent (kn_ServerRef aServer, kn_StringRef aTopic, kn_DictionaryRef anOptions)
{
  kn_MutableEventRef anEvent = (anOptions) ? kn_EventCreateMutableWithDictionary(anOptions) : kn_EventCreateMutable();

  if (anEvent)
    {
      kn_MutableStringRef aRouteLocation = kn_StringCreateMutableWithSize(0);
    
      if (aRouteLocation)
        {
          if (kn_StringGetProtocolFromURI(aTopic) == kn_PROTOCOL_INVALID)
            kn_StringAppendString(aRouteLocation, kn_ServerGetURI(aServer));

          kn_StringAppendString (aRouteLocation, aTopic                                            );
          kn_StringAppendString (aRouteLocation, kn_TOPIC_ROUTES_SUFFIX                            );
          kn_StringAppendCString(aRouteLocation, "/"                                               );
          kn_StringAppendString (aRouteLocation, kn_EventGetValue(anEvent, kn_EVENT_ID_HEADER_NAME));
        
          kn_EventSetValue(anEvent, KNSTR("do_method"), KNSTR("route"));
          kn_EventSetValue(anEvent, KNSTR("kn_from"  ), aTopic        );
          kn_EventSetValue(anEvent, KNSTR("kn_uri"   ), aRouteLocation);
        }
      else
        {
          kn_Release(anEvent); anEvent = NULL;
        }

      kn_Release(aRouteLocation);
    }

  return anEvent;
}

static kn_RouteRef kn_RouteCreateFromTopicToTopicViaServerWithEvent (kn_StringRef aTopic, kn_StringRef aDestination,
                                                                     kn_ServerRef aServer, kn_DictionaryRef anOptions,
                                                                     kn_EventHandler aStatusHandler, void* aUserData,
                                                                     kn_EventRef aRouteEvent, kn_BOOL aWaitFlag)
{
  kn_Route* aRoute = (kn_Route*)malloc(sizeof(kn_Route));

  if (!aRoute) { errno = ENOMEM; return NULL; }

  /*
   * Hack alert: We want the kn_to value to be aDestination and the kn_from value to be a Topic.
   * The kn_to is set up via the POST encoding while publishing to a topic.  So we pass the
   * destination as the topic to kn_ServerPublishEventToTopic and set the topic to route from
   * in the event to publish.
   * It's wierd, but it works, and even makes sense if you think about it sideways.
   */
  if (!kn_ServerPublishEventToTopic(aServer, aRouteEvent, aDestination, aStatusHandler, aUserData, aWaitFlag))
    {
      kn_object_init(aRoute, kn_RouteDealloc, kn_RouteDescribe);

      aRoute->route_id          = kn_Retain(kn_EventGetValue(aRouteEvent, kn_EVENT_ID_HEADER_NAME));
      aRoute->server            = kn_Retain(aServer                                               );
      aRoute->topic             = kn_Retain(aTopic                                                );
      aRoute->location          = kn_Retain(kn_EventGetValue(aRouteEvent, KNSTR("kn_uri"))        );
      aRoute->delete_on_dealloc = kn_FALSE;
    }
  else
    {
      free(aRoute);
      aRoute = NULL;
    }

  return aRoute;
}

kn_RouteRef kn_RouteCreateFromTopicToTopicViaServer (kn_StringRef aTopic, kn_StringRef aDestination,
                                                     kn_ServerRef aServer, kn_DictionaryRef anOptions,
                                                     kn_EventHandler aStatusHandler, void* aUserData,
                                                     kn_BOOL aWaitFlag)
{
  kn_RouteRef aRoute  = NULL;
  kn_EventRef anEvent = kn_EventCreateRouteEvent(aServer, aTopic, anOptions);

  if (anEvent)
    {
      aRoute = kn_RouteCreateFromTopicToTopicViaServerWithEvent(aTopic, aDestination,
                                                                aServer, anOptions,
                                                                aStatusHandler, aUserData,
                                                                anEvent, aWaitFlag);

      kn_Release(anEvent);
    }

  return aRoute;
}

kn_RouteRef kn_RouteCreateFromTopicToFunctionViaServer (kn_StringRef aTopic, kn_EventHandler aDestination,
                                                        kn_ServerRef aServer, kn_DictionaryRef anOptions,
                                                        kn_EventHandler aStatusHandler, void* aUserData,
                                                        kn_BOOL aWaitFlag)
{
  kn_RouteRef  aRoute          = NULL;
  kn_StringRef aDestinationURI = kn_ServerGetCallbackURI(aServer);

  if (aDestinationURI)
    {
      kn_EventRef anEvent = kn_EventCreateRouteEvent(aServer, aTopic, anOptions);

      if (!anEvent) return NULL;

      if (!kn_ServerRegisterCallback(aServer, kn_EventGetValue(anEvent, KNSTR("kn_uri")), aDestination, aUserData))
        aRoute = kn_RouteCreateFromTopicToTopicViaServerWithEvent(aTopic, aDestinationURI,
                                                                  aServer, anOptions,
                                                                  aStatusHandler, aUserData,
                                                                  anEvent, aWaitFlag);

      kn_Release(anEvent);
    }

  return aRoute;
}

/**
 * Accessors
 **/

kn_StringRef kn_RouteGetID              (kn_RouteRef aRoute) { return aRoute->route_id         ; }
kn_ServerRef kn_RouteGetServer          (kn_RouteRef aRoute) { return aRoute->server           ; }
kn_StringRef kn_RouteGetTopic           (kn_RouteRef aRoute) { return aRoute->topic            ; }
kn_StringRef kn_RouteGetURI             (kn_RouteRef aRoute) { return aRoute->location         ; }
kn_BOOL      kn_RouteGetDeleteOnDealloc (kn_RouteRef aRoute) { return aRoute->delete_on_dealloc; }

void kn_RouteSetDeleteOnDealloc (kn_RouteRef aRoute, kn_BOOL aFlag) { ((kn_Route*)aRoute)->delete_on_dealloc = aFlag; }

/**
 * Actions
 **/

kn_Error kn_RouteDelete (kn_RouteRef aRoute, kn_EventHandler aStatusHandler, void* aUserData, kn_BOOL aWaitFlag)
{
  /* This is a hack; there should be a server-side delete method. */
  kn_DictionaryRef anOptions = kn_DictionaryCreateWithNamesAndValues(kn_EVENT_ID_HEADER_NAME, aRoute->route_id,
                                                                     KNSTR("kn_expires")    , KNSTR("+300")   ,
                                                                     NULL);

  kn_RouteRef aDeadRoute = kn_RouteCreateFromTopicToTopicViaServer (aRoute->topic, KNSTR(""),
                                                                    aRoute->server, anOptions,
                                                                    aStatusHandler, aUserData,
                                                                    aWaitFlag);

  kn_Error anError = (aDeadRoute) ? kn_SUCCESS : kn_FAIL;

  kn_Release(anOptions );
  kn_Release(aDeadRoute);

  return anError;
}
