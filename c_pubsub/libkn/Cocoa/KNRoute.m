/**
 * PubSub Client Library
 * Route object
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

#import <Foundation/Foundation.h>
#import "FoundationExtensions.h"

#import "KNServer.h"
#import "KNEvent.h"
#import "KNRoute.h"

@implementation KNRoute : NSObject

/**
 * Factories
 **/

+ (KNRoute*) routeWithKn_Route: (kn_RouteRef) aRoute
{
  return [[(KNRoute*)[KNRoute alloc] initWithKn_Route: aRoute] autorelease];
}

+ (KNRoute*) routeWithTopic: (NSString*) aSourceTopic
                    toTopic: (NSString*) aDestinationTopic
                  viaServer: (KNServer*) aServer
                    andWait: (BOOL     ) aWaitFlag
{
  return [[(KNRoute*)[KNRoute alloc] initWithTopic: aSourceTopic
                                           toTopic: aDestinationTopic
                                         viaServer: aServer
                                           andWait: aWaitFlag] autorelease];
}

+ (KNRoute*) routeWithTopic: (NSString*    ) aSourceTopic
                    toTopic: (NSString*    ) aDestinationTopic
                  viaServer: (KNServer*    ) aServer
                withOptions: (NSDictionary*) anOptions
                   delegate: (id           ) aDelegate
                    andWait: (BOOL         ) aWaitFlag
{
  return [[(KNRoute*)[KNRoute alloc] initWithTopic: aSourceTopic
                                           toTopic: aDestinationTopic
                                         viaServer: aServer
                                       withOptions: anOptions
                                          delegate: aDelegate
                                           andWait: aWaitFlag] autorelease];
}

+ (KNRoute*) routeWithTopic: (NSString*      ) aSourceTopic
                   toObject: (id             ) aDestinationObject
                  viaServer: (KNServer*      ) aServer
                withOptions: (NSDictionary*  ) anOptions
                    andWait: (BOOL           ) aWaitFlag
{
  return [[(KNRoute*)[KNRoute alloc] initWithTopic: aSourceTopic
                                          toObject: aDestinationObject
                                         viaServer: aServer
                                       withOptions: anOptions
                                           andWait: aWaitFlag] autorelease];
}

/**
 * Inits
 **/

- (id) initWithKn_Route: (kn_RouteRef) aRoute
{
  if ((self = [super init]))
    {
      myRoute = kn_Retain(aRoute);
    }
  return self;
}

- (id) initWithTopic: (NSString*) aSourceTopic
             toTopic: (NSString*) aDestinationTopic
           viaServer: (KNServer*) aServer
             andWait: (BOOL     ) aWaitFlag
{
  return [self initWithTopic: aSourceTopic
                     toTopic: aDestinationTopic
                   viaServer: aServer
                 withOptions: nil
                     handler: NULL
                        data: NULL
                     andWait: aWaitFlag];
}

- (id) initWithTopic: (NSString*      ) aSourceTopic
             toTopic: (NSString*      ) aDestinationTopic
           viaServer: (KNServer*      ) aServer
         withOptions: (NSDictionary*  ) anOptions
             handler: (kn_EventHandler) aStatusHandler
                data: (void*          ) aData
             andWait: (BOOL           ) aWaitFlag
{
  if ((self = [super init]))
    {
      kn_StringRef     aSource       = [aSourceTopic      kn_StringCopy    ];
      kn_StringRef     aDestination  = [aDestinationTopic kn_StringCopy    ];
      kn_DictionaryRef anOptionsDict = [anOptions         kn_DictionaryCopy];

      if (!(myRoute =
              kn_RouteCreateFromTopicToTopicViaServer(aSource, aDestination,
                                                      [aServer kn_Server], anOptionsDict,
                                                      aStatusHandler, aData, (aWaitFlag)?kn_TRUE:kn_FALSE)))
        {
          [self release];
          self = NULL;
        }

      kn_Release(aSource      );
      kn_Release(aDestination );
      kn_Release(anOptionsDict);
    }
  return self;
}

- (id) initWithTopic: (NSString*      ) aSourceTopic
          toFunction: (kn_EventHandler) aDestinationFunction
           viaServer: (KNServer*      ) aServer
             andWait: (BOOL           ) aWaitFlag
{
  return [self initWithTopic: aSourceTopic
                  toFunction: aDestinationFunction
                   viaServer: aServer
                 withOptions: nil
                     handler: NULL
                        data: NULL
                     andWait: aWaitFlag];
}

- (id) initWithTopic: (NSString*      ) aSourceTopic
          toFunction: (kn_EventHandler) aDestinationFunction
           viaServer: (KNServer*      ) aServer
         withOptions: (NSDictionary*  ) anOptions
             handler: (kn_EventHandler) aStatusHandler
                data: (void*          ) aData
             andWait: (BOOL           ) aWaitFlag
{
  if ((self = [super init]))
    {
      kn_StringRef     aSource       = [aSourceTopic kn_StringCopy    ];
      kn_DictionaryRef anOptionsDict = [anOptions    kn_DictionaryCopy];

      if (!(myRoute =
              kn_RouteCreateFromTopicToFunctionViaServer(aSource, aDestinationFunction,
                                                         [aServer kn_Server], anOptionsDict,
                                                         aStatusHandler, aData, (aWaitFlag)?kn_TRUE:kn_FALSE)))
        {
          [self release];
          self = NULL;
        }

      kn_Release(aSource      );
      kn_Release(anOptionsDict);
    }
  return self;
}

static void event_dispatch (kn_EventRef aCEvent, void* anObject)
{
  id       aTarget = (id)anObject;
  KNEvent* anEvent = [KNEvent eventWithKn_Event: aCEvent];

  if ([aTarget respondsToSelector: @selector(handleEvent:)])
    [aTarget performSelector:@selector(handleEvent:) withObject:anEvent];
}

static void status_event_dispatch (kn_EventRef aCEvent, void* anObject)
{
  id       aTarget = (id)anObject;
  KNEvent* anEvent = [KNEvent eventWithKn_Event: aCEvent];

  if ([aTarget respondsToSelector: @selector(handleStatusEvent:)])
    [aTarget performSelector:@selector(handleStatusEvent:) withObject:anEvent];
}

- (id) initWithTopic: (NSString*    ) aSourceTopic
             toTopic: (NSString*    ) aDestinationTopic
           viaServer: (KNServer*    ) aServer
         withOptions: (NSDictionary*) anOptions
            delegate: (id           ) aDelegate
             andWait: (BOOL         ) aWaitFlag
{
  return [self initWithTopic: aSourceTopic
                     toTopic: aDestinationTopic
                   viaServer: aServer
                 withOptions: anOptions
                     handler: status_event_dispatch
                        data: aDelegate
                     andWait: aWaitFlag];
}

- (id) initWithTopic: (NSString*    ) aSourceTopic
            toObject: (id           ) aDestinationObject
           viaServer: (KNServer*    ) aServer
         withOptions: (NSDictionary*) anOptions
             andWait: (BOOL         ) aWaitFlag
{
  return [self initWithTopic: aSourceTopic
                  toFunction: event_dispatch
                   viaServer: aServer
                 withOptions: anOptions
                     handler: status_event_dispatch
                        data: aDestinationObject
                     andWait: aWaitFlag];
}

- (void) dealloc
{
  kn_Release(myRoute);

  [super dealloc];
}

/**
 * Accessors
 **/

- (NSString*) ID     { return [NSString stringWithKn_String: kn_RouteGetID    (myRoute)]; }
- (KNServer*) server { return [KNServer serverWithKn_Server: kn_RouteGetServer(myRoute)]; }
- (NSString*) topic  { return [NSString stringWithKn_String: kn_RouteGetTopic (myRoute)]; }
- (NSString*) URI    { return [NSString stringWithKn_String: kn_RouteGetURI   (myRoute)]; }

- (BOOL) deleteOnDealloc { return kn_RouteGetDeleteOnDealloc(myRoute); }

- (void) setDeleteOnDealloc: (BOOL) aFlag { kn_RouteSetDeleteOnDealloc(myRoute, aFlag); }

- (kn_RouteRef) kn_Route { return myRoute; }

- (NSString*) description
{
  kn_StringRef aCDescription = kn_CopyDescription(myRoute);
  NSString*    aDescription  = (aCDescription) ? [NSString stringWithKn_String: aCDescription] : [super description];

  kn_Release(aCDescription);

  return aDescription;
}

/**
 * Actions
 **/

- (kn_Error) deleteAndWait: (BOOL) aWaitFlag
{
  return kn_RouteDelete(myRoute, NULL, NULL, aWaitFlag);
}

@end
