/**
 * PubSub Client Library
 * Event object
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

#ifndef _KN_KNEVENT_H_
#define _KN_KNEVENT_H_ "$Id: KNEvent.h,v 1.2 2003/03/19 05:36:47 ifindkarma Exp $"

#include <kn/kn_Error.h>
#include <kn/kn_Event.h>

#import <Foundation/NSObject.h>

@class NSString;
@class NSDictionary;

#define KNEventIDHeaderName            [ NSString stringWithKn_String: kn_EVENT_ID_HEADER_NAME             ]
#define KNEventPayloadHeaderName       [ NSString stringWithKn_String: kn_EVENT_PAYLOAD_HEADER_NAME        ]
#define KNEventTimestampHeaderName     [ NSString stringWithKn_String: kn_EVENT_TIMESTAMP_HEADER_NAME      ]
#define KNEventExpiresHeaderName       [ NSString stringWithKn_String: kn_EVENT_EXPIRES_HEADER_NAME        ]
#define KNEventRouteLocationHeaderName [ NSString stringWithKn_String: kn_EVENT_ROUTE_LOCATION_HEADER_NAME ]
#define KNEventRouteIDHeaderName       [ NSString stringWithKn_String: kn_EVENT_ROUTE_ID_HEADER_NAME       ]
#define KNEventRoutedFromHeaderName    [ NSString stringWithKn_String: kn_EVENT_ROUTED_FROM_HEADER_NAME    ]
#define KNStatusEventStatusHeaderName  [ NSString stringWithKn_String: kn_STATUS_EVENT_STATUS_HEADER_NAME  ]

@interface KNEvent : NSObject

{
@protected
  kn_EventRef myEvent;
}

/**
 * Factories
 **/

+ (KNEvent*) eventWithKn_Event	 : (kn_EventRef	 ) anEvent;
+ (KNEvent*) eventWithDictionary : (NSDictionary*) aDictionary;

/**
 * Inits
 **/

- (id) initWithKn_Event	  : (kn_EventRef  ) anEvent;
- (id) initWithDictionary : (NSDictionary*) aDictionary;

- (id) copy;

/**
 * Accessors
 **/

- (size_t) count;

- (NSString*) valueForName: (NSString*) aName;

- (kn_EventRef) kn_Event;

- (NSString*) description;

@end

@interface KNMutableEvent : KNEvent

{
}

/**
 * Factories
 **/

+ (KNMutableEvent*) event;
+ (KNMutableEvent*) eventWithKn_Event       : (kn_EventRef       ) anEvent;
+ (KNMutableEvent*) eventWithKn_MutableEvent: (kn_MutableEventRef) anEvent;
+ (KNMutableEvent*) eventWithDictionary     : (NSDictionary*     ) aDictionary;

/**
 * Inits
 **/

- (id) init;
- (id) initWithKn_MutableEvent : (kn_MutableEventRef) anEvent;
- (id) initWithCapacity        : (size_t            ) aCapacity;

/**
 * Accessors
 **/

- (void) setValue: (NSString*) aValue
	  forName: (NSString*) aName;

- (void) removeValueForName: (NSString*) aName;

@end

#endif /* _KN_KNEVENT_H_ */
