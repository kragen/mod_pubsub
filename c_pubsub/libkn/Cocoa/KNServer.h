/**
 * PubSub Client Library
 * Server object
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

#ifndef _KN_KNSERVER_H_
#define _KN_KNSERVER_H_ "$Id: KNServer.h,v 1.3 2004/04/19 05:39:08 bsittler Exp $"

#include <kn/kn_Error.h>
#include <kn/kn_Server.h>

#import <Foundation/NSObject.h>

@class NSString;
@class KNEvent;

@interface KNServer : NSObject

{
@private
  kn_ServerRef  myServer;
}

/**
 * Factories
 **/

+ (KNServer*) serverWithKn_Server: (kn_ServerRef) aServer;

+ (KNServer*) serverWithURI: (NSString*) aURI;
+ (KNServer*) serverWithURI: (NSString*) aURI
		   username: (NSString*) aUsername
		   password: (NSString*) aPassword;

/**
 * Inits
 **/

- (id) initWithKn_Server: (kn_ServerRef) aServer;

- (id) initWithURI: (NSString*) aURI;
- (id) initWithURI: (NSString*) aURI
	  username: (NSString*) aUsername
	  password: (NSString*) aPassword;

/**
 * Accessors
 **/

- (NSString*) URI;

- (kn_ServerRef) kn_Server;

- (NSString*) description;

/**
 * Actions
 **/

- (kn_Error) waitForEventData;
- (void    ) waitForEventDataInBackgroundAndNotify;
- (kn_Error) processNextEvent;
- (size_t  ) processEvents;
- (void    ) processEventsInBackgroundAndNotify;

#define KNServerAvailableEventDataNotification @"KNServerAvailableEventData"
#define KNServerProcessedEventsNotification    @"KNServerProcessedEvents"
#define KNServerProcessedEventsCountKey        @"KNServerProcessedEventsCount"

/**
 * Actions
 **/

- (kn_Error) publishEvent: (KNEvent* ) anEvent
                  toTopic: (NSString*) aTopic
                  andWait: (BOOL     ) aWaitFlag;

@end

#endif /* _KN_KNSERVER_H_ */
