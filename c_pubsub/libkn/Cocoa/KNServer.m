/**
 * PubSub Client Library
 * Server object
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

#import <Foundation/Foundation.h>
#import "FoundationExtensions.h"

#import "KNEvent.h"
#import "KNServer.h"

@implementation KNServer : NSObject

/**
 * Factories
 **/

+ (KNServer*) serverWithKn_Server: (kn_ServerRef) aServer
{
  return [[[KNServer alloc] initWithKn_Server: aServer] autorelease];
}

+ (KNServer*) serverWithURI: (NSString*) aURI
{
  return [[[KNServer alloc] initWithURI: aURI] autorelease];
}

+ (KNServer*) serverWithURI: (NSString*) aURI
		   username: (NSString*) aUsername
		   password: (NSString*) aPassword
{
  return [[[KNServer alloc] initWithURI: aURI
			       username: aUsername
			       password: aPassword] autorelease];
}

/**
 * Inits
 **/

- (id) initWithKn_Server: (kn_ServerRef) aServer
{
  if ((self = [super init]))
    {
      myServer = kn_Retain(aServer);
    }
  return self;
}

- (id) initWithURI: (NSString*) aURI
{
  if ((self = [super init]))
    {
      kn_StringRef aCURI = [aURI kn_StringCopy];

      if (!(myServer = kn_ServerCreateWithURI(aCURI)))
        {
          [self release];
          self = NULL;
        }

      kn_Release(aCURI);
    }
  return self;
}

- (id) initWithURI: (NSString*) aURI
	  username: (NSString*) aUsername
	  password: (NSString*) aPassword
{
  if ((self = [super init]))
    {
      kn_StringRef aCURI      = [aURI      kn_StringCopy];
      kn_StringRef aCUsername = [aUsername kn_StringCopy];
      kn_StringRef aCPassword = [aPassword kn_StringCopy];

      if (!(myServer = kn_ServerCreateWithURIUserPassword(aCURI, aCUsername, aCPassword)))
        {
          [self release];
          self = NULL;
        }

      kn_Release(aCURI     );
      kn_Release(aCUsername);
      kn_Release(aCPassword);
    }
  return self;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver:self name:nil object:nil];

  kn_Release(myServer);

  [super dealloc];
}

/**
 * Accessors
 **/

- (NSString*) URI { return [NSString stringWithKn_String: kn_ServerGetURI (myServer)]; }

- (kn_ServerRef) kn_Server { return myServer; }

- (NSString*) description
{
  kn_StringRef aCDescription = kn_CopyDescription(myServer);
  NSString*    aDescription  = (aCDescription) ? [NSString stringWithKn_String: aCDescription] : [super description];

  kn_Release(aCDescription);

  return aDescription;
}

/**
 * Actions
 **/

- (kn_Error) waitForEventData { return kn_ServerWaitForEventData(myServer); }
- (kn_Error) processNextEvent { return kn_ServerProcessNextEvent(myServer); }
- (size_t  ) processEvents    { return kn_ServerProcessEvents   (myServer); }

- (void) __KNServer__notifyAvailableData: (id)aJunk
{
  NSAutoreleasePool* aPool = [[NSAutoreleasePool alloc] init];

  [self waitForEventData];

  [[NSNotificationCenter defaultCenter] postNotificationName: KNServerAvailableEventDataNotification
                                                      object: self];

  [aPool release];
}

- (void) waitForEventDataInBackgroundAndNotify
{
  [NSThread detachNewThreadSelector: @selector(__KNServer__notifyAvailableData:)
                           toTarget: self
                         withObject: nil];
}

- (void) __KNServer__processEventsAndNotify: (NSNotification*) aNotification
{
  size_t aCount;

  [[NSNotificationCenter defaultCenter] removeObserver: self
                                                  name: KNServerAvailableEventDataNotification
                                                object: self];

  aCount = [self processEvents];

  if (aCount > 0)
    {
      NSDictionary* anInfo = [NSDictionary dictionaryWithObject: [NSNumber numberWithLong: (long)aCount]
                                                         forKey: KNServerProcessedEventsCountKey];

  
      [[NSNotificationCenter defaultCenter] postNotificationName: KNServerProcessedEventsNotification
                                                          object: self
                                                        userInfo: anInfo];
    }
  else
    [self processEventsInBackgroundAndNotify];
}

- (void) processEventsInBackgroundAndNotify
{
  [[NSNotificationCenter defaultCenter] addObserver: self
                                          selector: @selector(__KNServer__processEventsAndNotify:)
                                              name: KNServerAvailableEventDataNotification
                                            object: self];

  [self waitForEventDataInBackgroundAndNotify];
}

- (kn_Error) publishEvent: (KNEvent* ) anEvent
                  toTopic: (NSString*) aTopic
                  andWait: (BOOL     ) aWaitFlag;
{
  kn_StringRef aCTopic = [aTopic kn_StringCopy];

  kn_Error anError = kn_ServerPublishEventToTopic(myServer, [anEvent kn_Event], aCTopic, NULL, NULL, (aWaitFlag)?kn_TRUE:kn_FALSE);

  kn_Release(aCTopic);

  return anError;
}

@end
