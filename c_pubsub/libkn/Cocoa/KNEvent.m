/**
 * PubSub Client Library
 * Event object
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

@implementation KNEvent

/**
 * Factories
 **/

+ (KNEvent*) eventWithKn_Event: (kn_EventRef) anEvent
{
  return [[(KNEvent*)[KNEvent alloc] initWithKn_Event: anEvent] autorelease];
}

+ (KNEvent*) eventWithDictionary: (NSDictionary*) aDictionary
{
  return [[(KNEvent*)[KNEvent alloc] initWithDictionary: aDictionary] autorelease];
}

/**
 * Inits
 **/

- (id) initWithKn_Event: (kn_EventRef) anEvent
{
  if ((self = [super init]))
    {
      myEvent = kn_Retain(anEvent);
    }
  return self;
}

- (id) initWithDictionary: (NSDictionary*) aDictionary
{
  if ((self = [super init]))
    {
      kn_DictionaryRef aCDictionary = [aDictionary kn_DictionaryCopy];

      if (!(myEvent = kn_EventCreateWithDictionary(aCDictionary)))
        {
          [self release];
          self = NULL;
        }

      kn_Release(aCDictionary);
    }
  return self;
}

- (void) dealloc
{
  kn_Release(myEvent);

  [super dealloc];
}

- (id) copy
{
  kn_EventRef anEvent = kn_EventCreateCopy(myEvent);
  KNEvent*    aCopy   = [KNEvent eventWithKn_Event: anEvent];

  kn_Release(anEvent);

  return aCopy;
}

/**
 * Accessors
 **/

- (size_t) count { return kn_EventGetCount(myEvent); }

- (NSString*) valueForName: (NSString*) aName
{
  kn_StringRef aCName = [aName kn_StringCopy];
  NSString*    aValue = [NSString stringWithKn_String: kn_EventGetValue(myEvent, aCName)];

  kn_Release(aCName);

  return aValue;
}

- (kn_EventRef) kn_Event { return myEvent; }

- (NSString*) description
{
  kn_StringRef aCDescription = kn_CopyDescription(myEvent);
  NSString*    aDescription  = (aCDescription) ? [NSString stringWithKn_String: aCDescription] : [super description];

  kn_Release(aCDescription);

  return aDescription;
}

@end

@implementation KNMutableEvent : KNEvent

/**
 * Factories
 **/

+ (KNMutableEvent*) event
{
  return [[(KNMutableEvent*)[KNMutableEvent alloc] init] autorelease];
}

+ (KNMutableEvent*) eventWithKn_Event: (kn_EventRef) anEvent
{
  return [[(KNMutableEvent*)[KNMutableEvent alloc] initWithKn_Event: anEvent] autorelease];
}

+ (KNMutableEvent*) eventWithKn_MutableEvent: (kn_MutableEventRef) anEvent
{
  return [[(KNMutableEvent*)[KNMutableEvent alloc] initWithKn_MutableEvent: anEvent] autorelease];
}

+ (KNMutableEvent*) eventWithDictionary: (NSDictionary*) aDictionary
{
  return [[(KNMutableEvent*)[KNMutableEvent alloc] initWithDictionary: aDictionary] autorelease];
}

/**
 * Inits
 **/

- (id) initWithKn_Event: (kn_EventRef) anEvent
{
  kn_MutableEventRef aMutableEvent = kn_EventCreateMutableCopy(anEvent);

  KNMutableEvent* aNewEvent = [self initWithKn_MutableEvent: aMutableEvent];

  kn_Release(aMutableEvent);

  return aNewEvent;
}

- (id) initWithKn_MutableEvent: (kn_MutableEventRef) anEvent
{
  if ((self = [super init]))
    {
      myEvent = (kn_EventRef)kn_Retain(anEvent);
    }
  return self;
}

- (id) init
{
  return [self initWithCapacity: 0];
}

- (id) initWithDictionary: (NSDictionary*) aDictionary
{
  if ((self = [super init]))
    {
      kn_DictionaryRef aCDictionary = [aDictionary kn_DictionaryCopy];

      if (!(myEvent = (kn_EventRef)kn_EventCreateMutableWithDictionary(aCDictionary)))
        {
          [self release];
          self = NULL;
        }

      kn_Release(aCDictionary);
    }
  return self;
}

- (id) initWithCapacity: (size_t) aCapacity
{
  if ((self = [super init]))
    {
      if (!(myEvent = (kn_EventRef)kn_EventCreateMutableWithCapacity(aCapacity)))
        {
          [self release];
          self = NULL;
        }
    }
  return self;
}

- (id) copy
{
  kn_MutableEventRef anEvent = kn_EventCreateMutableCopy(myEvent);
  KNMutableEvent*    aCopy   = [KNMutableEvent eventWithKn_MutableEvent: anEvent];

  kn_Release(anEvent);

  return aCopy;
}

/**
 * Accessors
 **/

- (void) setValue: (NSString*) aValue
	  forName: (NSString*) aName
{
  kn_StringRef aCName  = [aName  kn_StringCopy];
  kn_StringRef aCValue = [aValue kn_StringCopy];

  kn_EventSetValue((kn_MutableEventRef)myEvent, aCName, aCValue);

  kn_Release(aCName );
  kn_Release(aCValue);
}

- (void) removeValueForName: (NSString*) aName
{
  kn_StringRef aCName = [aName kn_StringCopy];

  kn_EventRemoveValue((kn_MutableEventRef)myEvent, aCName);

  kn_Release(aCName);
}

@end
