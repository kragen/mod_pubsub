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

#ifndef _KN_KNROUTE_H_
#define _KN_KNROUTE_H_ "$Id: KNRoute.h,v 1.3 2004/04/19 05:39:08 bsittler Exp $"

#include <kn/kn_Error.h>
#include <kn/kn_Route.h>

#import <Foundation/NSObject.h>

@class NSString;
@class NSDictionary;
@class KNServer;

#define KNRouteOptionMaxAge    [ NSString stringWithKn_String: kn_ROUTE_OPTION_MAX_AGE   ]
#define KNRouteOptionMaxCount  [ NSString stringWithKn_String: kn_ROUTE_OPTION_MAX_COUNT ]
#define KNTopicSubtopicsSuffix [ NSString stringWithKn_String: kn_TOPIC_SUBTOPICS_SUFFIX ]
#define KNTopicRoutesSuffix    [ NSString stringWithKn_String: kn_TOPIC_ROUTES_SUFFIX    ]
#define KNTopicJournalSuffix   [ NSString stringWithKn_String: kn_TOPIC_JOURNAL_SUFFIX   ]

@interface KNRoute : NSObject

{
@private
  kn_RouteRef myRoute;
}

/**
 * Factories
 **/

+ (KNRoute*) routeWithKn_Route: (kn_RouteRef) aRoute;

+ (KNRoute*) routeWithTopic: (NSString*    ) aSourceTopic
                    toTopic: (NSString*    ) aDestinationTopic
                  viaServer: (KNServer*    ) aServer
                    andWait: (BOOL         ) aWaitFlag;

+ (KNRoute*) routeWithTopic: (NSString*    ) aSourceTopic
                    toTopic: (NSString*    ) aDestinationTopic
                  viaServer: (KNServer*    ) aServer
                withOptions: (NSDictionary*) anOptions
                   delegate: (id           ) aDelegate
                    andWait: (BOOL         ) aWaitFlag;

+ (KNRoute*) routeWithTopic: (NSString*    ) aSourceTopic
                   toObject: (id           ) aDestinationObject
                  viaServer: (KNServer*    ) aServer
                withOptions: (NSDictionary*) anOptions
                    andWait: (BOOL         ) aWaitFlag;

/**
 * Inits
 **/

- (id) initWithKn_Route: (kn_RouteRef) aRoute;

- (id) initWithTopic: (NSString*      ) aSourceTopic
             toTopic: (NSString*      ) aDestinationTopic
           viaServer: (KNServer*      ) aServer
             andWait: (BOOL           ) aWaitFlag;

- (id) initWithTopic: (NSString*      ) aSourceTopic
             toTopic: (NSString*      ) aDestinationTopic
           viaServer: (KNServer*      ) aServer
         withOptions: (NSDictionary*  ) anOptions
             handler: (kn_EventHandler) aStatusHandler
                data: (void*          ) aData
             andWait: (BOOL           ) aWaitFlag;

- (id) initWithTopic: (NSString*      ) aSourceTopic
             toTopic: (NSString*      ) aDestinationTopic
           viaServer: (KNServer*      ) aServer
         withOptions: (NSDictionary*  ) anOptions
            delegate: (id             ) aDelegate /* Not retained; implements handleStatusEvent: */
             andWait: (BOOL           ) aWaitFlag;

- (id) initWithTopic: (NSString*      ) aSourceTopic
          toFunction: (kn_EventHandler) aDestinationFunction
           viaServer: (KNServer*      ) aServer
             andWait: (BOOL           ) aWaitFlag;

- (id) initWithTopic: (NSString*      ) aSourceTopic
          toFunction: (kn_EventHandler) aDestinationFunction
           viaServer: (KNServer*      ) aServer
         withOptions: (NSDictionary*  ) anOptions
             handler: (kn_EventHandler) aStatusHandler
                data: (void*          ) aData
             andWait: (BOOL           ) aWaitFlag;

/*
 * aDestinationObject is not retained.
 * Callbacks are:
 * [aDestinationObject handleEvent      : (KNEvent*) anEvent     ]
 * [aDestinationObject handleStatusEvent: (KNEvent*) aStatusEvent]
 */
- (id) initWithTopic: (NSString*    ) aSourceTopic
            toObject: (id           ) aDestinationObject
           viaServer: (KNServer*    ) aServer
         withOptions: (NSDictionary*) anOptions
             andWait: (BOOL         ) aWaitFlag;

/**
 * Accessors
 **/

- (NSString*) ID;
- (KNServer*) server;
- (NSString*) topic;
- (NSString*) URI;
- (BOOL     ) deleteOnDealloc;

- (void) setDeleteOnDealloc: (BOOL) aFlag;

- (kn_RouteRef) kn_Route;

- (NSString*) description;

/**
 * Actions
 **/

- (kn_Error) deleteAndWait: (BOOL) aWaitFlag;

@end

#endif /* _KN_KNROUTE_H_ */
