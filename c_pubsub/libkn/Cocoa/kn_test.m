/**
 * PubSub Client Library
 * libkn (Cocoa) test suite
 *
 * WARNING: Don't use this as a source of good examples as to how to
 * program the library; it's job is only to test it.  In particular,
 * one should note that we very often return from a function without
 * having released all of the objects we have hanging about.  This is
 * rather poor practice, as it leaks memory, but we're going to exit
 * the program when that happens, and this is all messy enough as is.
 * -Fred
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

#include <sys/cdefs.h>
#ifdef __RCSID
__RCSID("$Id: kn_test.m,v 1.2 2003/03/19 05:36:47 ifindkarma Exp $");
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <Foundation/Foundation.h>
#include "FoundationExtensions.h"
#include <KnowNow/KnowNow.h>

#define HELLO_WORLD_PART1            "Hello, World!\n"
#define HELLO_WORLD_PART2            "1!2@3#4$5%6^7&8*9(0)\n"
#define HELLO_WORLD_PART3            "Wilfredo S\xc3\xa1nchez\n"
#define HELLO_WORLD                  HELLO_WORLD_PART1 HELLO_WORLD_PART2 HELLO_WORLD_PART3
#define HELLO_WORLD_QUOTED_PRINTABLE "Hello=2C=20World!=0A1!2=403=234=245=256=5E7=268*9(0)=0AWilfredo=20S=C3=A1nchez=0A"
#define HELLO_WORLD_URLHEX           "Hello%2C%20World!%0A1!2%403%234%245%256%5E7%268*9(0)%0AWilfredo%20S%C3%A1nchez%0A"
#define HELLO_WORLD_BASE64           "SGVsbG8sIFdvcmxkIQoxITJAMyM0JDUlNl43JjgqOSgwKQpXaWxmcmVkbyBTw6FuY2hlego="

#ifndef PUBSUB_URI
#define PUBSUB_URI       "http://localhost/mod_pubsub/cgi-bin/pubsub.cgi"
#endif
#define PUBSUB_USER      @"kncgi"
#define PUBSUB_PASSWORD  @"kncgi"
#define PUBSUB_SUB_TOPIC @"/what/chat"
#define PUBSUB_PUB_TOPIC @"/test/kn_test.m/routed_topic/"

int string ()
{
  NSAutoreleasePool* aPool = [[NSAutoreleasePool alloc] init];

  kn_StringRef aCString = kn_StringCreateWithCString(HELLO_WORLD);

  printf("Testing NSString extensions:\n");

  printf("NSString/init..."); fflush(stdout);
  {
    NSString* aString = [NSString stringWithKn_String: aCString];

    if (strcmp([aString UTF8String], HELLO_WORLD))
      {
        printf("\n\tFAIL: String compare (\"%s\" != \"", [aString UTF8String]);
        kn_StringWriteToStream(aCString, stdout);
        printf("\")\n");
        return 1;
      }
  }
  printf("OK\n");

  kn_Release(aCString);
  printf("\n");
  [aPool release];
  return 0;
}

int string_webencode ()
{
  NSAutoreleasePool* aPool = [[NSAutoreleasePool alloc] init];

  NSString* aString       = [NSString stringWithUTF8String: HELLO_WORLD];
  NSString* aNutherString;

  printf("Testing NSString extensions (encodings):\n");

  printf("NSString/encode.qp..."); fflush(stdout);
  aNutherString = [NSString stringWithStringByEncodingQuotedPrintable: aString];
  if (strcmp([aNutherString UTF8String], HELLO_WORLD_QUOTED_PRINTABLE))
    {
      printf("\n\tFAIL: \"%s\" != \"%s\"\n", [aNutherString UTF8String], HELLO_WORLD_QUOTED_PRINTABLE);
      return 1;
    }
  printf("OK\n");
  printf("NSString/decode.qp..."); fflush(stdout);
  aNutherString = [NSString stringWithStringByDecodingQuotedPrintable: aNutherString];
  if (strcmp([aNutherString UTF8String], HELLO_WORLD))
    {
      printf("\n\tFAIL: \"%s\" != \"%s\"\n", [aNutherString UTF8String], HELLO_WORLD);
      return 1;
    }
  printf("OK\n");

  printf("NSString/encode.url..."); fflush(stdout);
  aNutherString = [NSString stringWithStringByEncodingURLHex: aString];
  if (strcmp([aNutherString UTF8String], HELLO_WORLD_URLHEX))
    {
      printf("\n\tFAIL: \"%s\" != \"%s\"\n", [aNutherString UTF8String], HELLO_WORLD_URLHEX);
      return 1;
    }
  printf("OK\n");
  printf("NSString/decode.url..."); fflush(stdout);
  aNutherString = [NSString stringWithStringByDecodingURLHex: aNutherString];
  if (strcmp([aNutherString UTF8String], HELLO_WORLD))
    {
      printf("\n\tFAIL: \"%s\" != \"%s\"\n", [aNutherString UTF8String], HELLO_WORLD);
      return 1;
    }
  printf("OK\n");

  printf("NSString/encode.base64..."); fflush(stdout);
  aNutherString = [NSString stringWithStringByEncodingBase64: aString];
  if (strcmp([aNutherString UTF8String], HELLO_WORLD_BASE64))
    {
      printf("\n\tFAIL: \"%s\" != \"%s\"\n", [aNutherString UTF8String], HELLO_WORLD_BASE64);
      return 1;
    }
  printf("OK\n");

  printf("\n");
  [aPool release];
  return 0;
}

int dictionary ()
{
  NSAutoreleasePool* aPool = [[NSAutoreleasePool alloc] init];

  kn_StringRef aName1  = kn_StringCreateWithCString("Name #1");
  kn_StringRef aName2  = kn_StringCreateWithCString("Name the 2nd");
  kn_StringRef aName3  = kn_StringCreateWithCString("3rd name");
  kn_StringRef aName4  = kn_StringCreateWithCString("Name 4");
  kn_StringRef aValue1 = kn_StringCreateWithCString("This is value #1");
  kn_StringRef aValue2 = kn_StringCreateWithCString("This is value the 2nd");
  kn_StringRef aValue3 = kn_StringCreateWithCString("This is the 3rd value");
  kn_StringRef aValue4 = kn_StringCreateWithCString("This is value 4");

  kn_DictionaryRef aCDictionary = kn_DictionaryCreateMutableWithNamesAndValues(aName1, aValue1,
                                                                               aName2, aValue2,
                                                                               aName3, aValue3,
                                                                               aName4, aValue4,
                                                                               NULL  , NULL   );

  printf("Testing NSDictionary extensions:\n");

  printf("NSDictionary/init..."); fflush(stdout);
  {
    NSDictionary* aDictionary = [NSDictionary dictionaryWithKn_Dictionary: aCDictionary];
    NSString* aValue = [aDictionary objectForKey: [NSString stringWithKn_String: aName1]];
    if (strcmp([aValue UTF8String], kn_StringGetBytes(aValue1)))
      {
        printf("\n\tFAIL: Value #1 mismatch (\"%s\" != \"%s\")\n", [aValue UTF8String], kn_StringGetBytes(aValue1));
        return 1;
      }
    aValue = [aDictionary objectForKey: [NSString stringWithKn_String: aName3]];
    if (strcmp([aValue UTF8String], kn_StringGetBytes(aValue3)))
      {
        printf("\n\tFAIL: Value #3 mismatch (\"%s\" != \"%s\")\n", [aValue UTF8String], kn_StringGetBytes(aValue3));
        return 1;
      }
    printf("OK\n");
  }

  kn_Release(aCDictionary);
  printf("\n");
  [aPool release];
  return 0;
}

int server ()
{
  NSAutoreleasePool* aPool = [[NSAutoreleasePool alloc] init];
  KNServer* aServer;

  printf("Testing KNServer:\n");

  printf("KNServer/init..."); fflush(stdout);
  aServer = [KNServer serverWithURI: [NSString stringWithUTF8String: PUBSUB_URI]
                           username: PUBSUB_USER
                           password: PUBSUB_PASSWORD];
  printf("OK\n");

  printf("\n");
  [aPool release];
  return 0;
}

int event ()
{
  NSAutoreleasePool* aPool = [[NSAutoreleasePool alloc] init];

  KNMutableEvent* anEvent;

  NSString* aName1  = [NSString stringWithUTF8String: "Name #1"               ];
  NSString* aName2  = [NSString stringWithUTF8String: "Name the 2nd"          ];
  NSString* aName3  = [NSString stringWithUTF8String: "3rd name"              ];
  NSString* aName4  = [NSString stringWithUTF8String: "Name 4"                ];
  NSString* aValue1 = [NSString stringWithUTF8String: "This is value #1"      ];
  NSString* aValue2 = [NSString stringWithUTF8String: "This is value the 2nd" ];
  NSString* aValue3 = [NSString stringWithUTF8String: "This is the 3rd value" ];
  NSString* aValue4 = [NSString stringWithUTF8String: "This is value 4"       ];

  NSDictionary* anEventDictionary;

  printf("Testing KNEvent:\n");

  printf("KNEvent/init.dictionary..."); fflush(stdout);
  anEventDictionary =
    [NSDictionary dictionaryWithObjectsAndKeys: aValue1, aName1,
                                                aValue2, aName2,
                                                aValue3, aName3,
                                                aValue4, aName4,
                                                nil    , nil   ];
  anEvent = [KNMutableEvent eventWithDictionary: anEventDictionary];
  if ([aValue1 compare: [anEvent valueForName: aName1]])
    {
      printf("\n\tFAIL: Value mismatch.\n");
      NSLog(@"%@ != %@\n", aValue1, [anEvent valueForName: aName1]);
      return 1;
    }
  if ([aValue3 compare: [anEvent valueForName: aName3]])
    {
      printf("\n\tFAIL: Value mismatch.\n");
      NSLog(@"%@ != %@\n", aValue3, [anEvent valueForName: aName3]);
      return 1;
    }
  printf("OK\n");

  printf("KNEvent/init.mutable..."); fflush(stdout);
  anEvent = [KNMutableEvent event];
  [anEvent setValue:aValue1 forName:aName1];
  [anEvent setValue:aValue2 forName:aName2];
  [anEvent setValue:aValue3 forName:aName3];
  [anEvent setValue:aValue4 forName:aName4];
  if ([aValue2 compare: [anEvent valueForName: aName2]])
    {
      printf("\n\tFAIL: Value mismatch.\n");
      NSLog(@"%@ != %@\n", aValue2, [anEvent valueForName: aName2]);
      return 1;
    }
  if ([aValue4 compare: [anEvent valueForName: aName4]])
    {
      printf("\n\tFAIL: Value mismatch.\n");
      NSLog(@"%@ != %@\n", aValue3, [anEvent valueForName: aName3]);
      return 1;
    }
  printf("OK\n");

  printf("\n");
  [aPool release];
  return 0;
}

@interface EventHandler : NSObject
{
@public
  NSString*       name;
  NSMutableArray* events;
  NSMutableArray* statusEvents;
}
- (id) initWithName: (NSString*) aName;
@end
@implementation EventHandler
- (id) initWithName: (NSString*) aName
{
  if ((self = [super init]))
    {
      name         = [aName retain];
      events       = [[NSMutableArray alloc] init];
      statusEvents = [[NSMutableArray alloc] init];
    }
  return self;
}
- (void) dealloc
{
  [name         release];
  [events       release];
  [statusEvents release];
  [super dealloc];
}
- (void) handleEvent: (KNEvent*) anEvent
{
  //NSLog(@"%@ got event %@ (%d)", name, anEvent, [anEvent retainCount]);
  [events addObject: anEvent];
}
- (void) handleStatusEvent : (KNEvent*) anEvent
{
  //NSLog(@"%@ got status event %@ (%d)", name, anEvent, [anEvent retainCount]);
  [statusEvents addObject: anEvent];
}
@end

int message ()
{
  NSAutoreleasePool* aPool = [[NSAutoreleasePool alloc] init];

  KNServer*       aServer;
  KNRoute*        aPubToSubRoute;
  EventHandler*   aPubToSubHandler = nil;
  KNRoute*        aSubToObjectRoute;
  EventHandler*   aSubToObjectHandler = nil;
  NSString*       aPubTopic = [NSString stringWithFormat: @"%@%@",
                                PUBSUB_PUB_TOPIC,
                                [[KNMutableEvent event] valueForName: KNEventIDHeaderName]];

  printf("Testing publish and subscribe:\n");

  aServer = [KNServer serverWithURI: [NSString stringWithUTF8String: PUBSUB_URI]
                           username: PUBSUB_USER
                           password: PUBSUB_PASSWORD];

  printf("KNRoute/create.totopic..."); fflush(stdout);
  aPubToSubHandler = [[EventHandler alloc] initWithName: @"Route pub->sub"];
  aPubToSubRoute = [KNRoute routeWithTopic: aPubTopic
                                   toTopic: PUBSUB_SUB_TOPIC
                                 viaServer: aServer
                               withOptions: nil
                                  delegate: aPubToSubHandler
                                   andWait: YES];
  if (!aPubToSubRoute)
    {
      printf("\n\tFAIL: -[KNRoute routeWithTopic:toTopic:viaServer:withOptions:delegate:] returned NULL.\n");
      printf("WARNING: Skipping pub/sub tests.\n");
      return 2;
    }
  if ([aPubToSubHandler->statusEvents count] != 1)
    {
      printf("\n\tFAIL: No status event for route->topic.\n");
      NSLog(@"Status events: %@\n", aPubToSubHandler->statusEvents);
      return 1;
    }
  printf("OK\n");

  printf("KNRoute/create.toobject..."); fflush(stdout);
  aSubToObjectHandler = [[EventHandler alloc] initWithName: @"Route sub->obj"];
  aSubToObjectRoute = [KNRoute routeWithTopic: PUBSUB_SUB_TOPIC
                                     toObject: aSubToObjectHandler
                                    viaServer: aServer
                                  withOptions: nil
                                      andWait: YES];
  if (!aSubToObjectRoute)
    {
      printf("\n\tFAIL: -[KNRoute routeWithTopic:toObject:viaServer:withOptions:] returned NULL.\n");
      return 1;
    }
  if ([aSubToObjectHandler->statusEvents count] != 1)
    {
      printf("\n\tFAIL: No status event for route->object.\n");
      NSLog(@"Status events: %@\n", aSubToObjectHandler->statusEvents);
      return 1;
    }
  printf("OK\n");

  printf("KNEvent/send..."); fflush(stdout);
  {
    kn_Error anError;
    KNMutableEvent* anEvent = [KNMutableEvent event];
    [anEvent setValue: @"kn_test.m" forName: @"displayname"];
    [anEvent setValue: [NSString stringWithUTF8String: HELLO_WORLD] forName: KNEventPayloadHeaderName];
    if ((anError = [aServer publishEvent: anEvent toTopic: aPubTopic andWait: YES]))
      {
        printf("\n\tFAIL: -[KNServer publishEvent:toTopic:] returned error %d.\n", anError);
        return 1;
      }
  }
  printf("OK\n");

  printf("KNEvent/get..."); fflush(stdout);
  {
    KNEvent* anEvent;
    kn_Error anError = [aServer processNextEvent];
    if (anError)
      {
        printf("\n\tFAIL: -[KNServer processNextEvent] returned error %d.\n", anError);
        return 1;
      }
    if ([aSubToObjectHandler->events count] != 1)
      {
        printf("\n\tFAIL: Single sent event not received.\n");
        NSLog(@"Events: %@\n", aSubToObjectHandler->events);
        return 1;
      }
    anEvent = [aSubToObjectHandler->events objectAtIndex: 0];
    if ([(NSString*)[NSString stringWithUTF8String: HELLO_WORLD] compare: [anEvent valueForName: KNEventPayloadHeaderName]])
      {
        printf("\n\tFAIL: Event payload does not match sent payload:\n");
        NSLog(@"%@ != %@\n", [NSString stringWithUTF8String: HELLO_WORLD],
                             [anEvent valueForName: KNEventPayloadHeaderName]);
        return 1;
      }
  }
  printf("OK\n");

  printf("KNRoute/delete..."); fflush(stdout);
  {
    kn_Error anError;
    if ((anError = [aPubToSubRoute deleteAndWait: YES]))
      {
        printf("\n\tFAIL: -[KNRoute delete] returned error %d.\n", anError);
        return 1;
      }
    {
      KNMutableEvent* anEvent = [KNMutableEvent event];
      [anEvent setValue: @"kn_test.m" forName: @"displayname"];
      [anEvent setValue: [NSString stringWithUTF8String: HELLO_WORLD] forName: KNEventPayloadHeaderName];
      if ((anError = [aServer publishEvent: anEvent toTopic: aPubTopic andWait: YES]))
        {
          printf("\n\tFAIL: -[KNServer publishEvent:toTopic:] returned error %d.\n", anError);
          return 1;
        }
    }
    if ([aServer processEvents] != 0)
      printf("\n\tWARNING: kn_ServerProcessEvents found unexpected events.\n");
    sleep(5);
    if ([aServer processEvents] != 0)
      printf("\n\tWARNING: kn_ServerProcessEvents found unexpected events.\n");
    if ([aSubToObjectHandler->events count] != 1)
      {
        printf("\n\tFAIL: Too many events for send (%d != 1).\n", [aSubToObjectHandler->events count]);
        NSLog(@"Events: %@\n", aSubToObjectHandler->events);
        return 1;
      }
    if ((anError = [aSubToObjectRoute deleteAndWait: YES]))
      {
        printf("\n\tFAIL: -[KNRoute delete] returned error %d.\n", anError);
        return 1;
      }
  }
  printf("OK\n");

  [aPubToSubHandler    release];
  [aSubToObjectHandler release];

  printf("\n");
  [aPool release];
  return 0;
}

void fail ()
{
  printf("Test suite failed.\n");
  exit(1);
}

int main ()
{
  srandom (time(NULL) ^ (getpid() << 16));

  printf("Running Cocoa test suite:\n\n");

  if (string          () == 1) fail();
  if (string_webencode() == 1) fail();
  if (dictionary      () == 1) fail();
  if (server          () == 1) fail();
  if (event           () == 1) fail();
  if (message         () == 1) fail();

  printf("All tests passed.\n");

  exit(0);
}
