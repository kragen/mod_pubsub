/**
 * PubSub Client Library
 * Route object
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

#ifndef _KN_KNROUTE_H_
#define _KN_KNROUTE_H_ "$Id: KNRoute.h,v 1.2 2003/03/19 05:36:47 ifindkarma Exp $"

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
