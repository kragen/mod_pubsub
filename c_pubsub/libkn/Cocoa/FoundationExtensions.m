/**
 * PubSub Client Library
 * Foundation extensions
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

#include <kn/kn_String_WebEncoding.h>

#import "FoundationExtensions.h"

@implementation NSString (kn_Extensions)

+ (NSString*) stringWithKn_String: (kn_StringRef) aString
{
  return [[(NSString*)[NSString alloc] initWithKn_String: aString] autorelease];
}

- (id) initWithKn_String: (kn_StringRef) aString
{
  if (aString)
    {
      NSData* aData = [NSData dataWithBytes: kn_StringGetBytes(aString)
                                     length: kn_StringGetSize (aString)];
    
      return [[NSString alloc] initWithData: aData
                                  encoding: NSUTF8StringEncoding];
    }

  return nil;
}

- (kn_StringRef) kn_StringCopy
{
  NSData* aData = [self dataUsingEncoding: NSUTF8StringEncoding];

  return kn_StringCreateWithBytes([aData bytes], [aData length]);
}

@end

@implementation NSString (kn_Extensions_WebEncoding)

#define initWithTransform(aString, aTransform)					\
  kn_StringRef aKNString = [aString kn_StringCopy];				\
                                                                                \
  if (aKNString)								\
    {										\
      kn_StringRef aNewString = aTransform(aKNString);				\
                                                                                \
      if (aNewString)								\
        {									\
          self = [self initWithKn_String: aNewString];				\
                                                                                \
          kn_Release(aNewString);						\
        }									\
      else									\
        { [self release]; self = NULL; }					\
                                                                                \
      kn_Release(aKNString);							\
    }										\
  else										\
    { [self release]; self = NULL; }						\
                                                                                \
  return self

#define makeBridgeWithTransform(aTransform)							\
+ (NSString*) stringWithStringBy##aTransform: (NSString*) aString				\
{												\
  return [[(NSString*)[NSString alloc] initWithStringBy##aTransform: aString] autorelease];	\
}												\
- (id) initWithStringBy##aTransform: (NSString*) aString					\
{												\
  initWithTransform(aString, kn_StringCreateBy##aTransform);					\
}

makeBridgeWithTransform(EncodingURLHex         )
makeBridgeWithTransform(DecodingURLHex         )
makeBridgeWithTransform(EncodingQuotedPrintable)
makeBridgeWithTransform(DecodingQuotedPrintable)
makeBridgeWithTransform(EncodingBase64         )

@end

@implementation NSDictionary (kn_Extensions)

+ (NSDictionary*) dictionaryWithKn_Dictionary: (kn_DictionaryRef) aDictionary
{
  return [[(NSDictionary*)[NSDictionary alloc] initWithKn_Dictionary: aDictionary] autorelease];
}

- (id) initWithKn_Dictionary: (kn_DictionaryRef) aDictionary
{
  NSMutableDictionary* aResult = nil;

  if (aDictionary)
    {
      size_t               aCount  = kn_DictionaryGetCount(aDictionary);
      kn_StringRef*        aNames  = (kn_StringRef*)malloc(aCount*sizeof(kn_StringRef));
      kn_ObjectRef*        aValues = (kn_ObjectRef*)malloc(aCount*sizeof(kn_StringRef));

      aResult = [[NSMutableDictionary alloc] initWithCapacity: aCount];
    
      if (!aNames || !aValues || !aResult ||
          kn_DictionaryGetNamesAndValues(aDictionary, aNames, aValues) != kn_SUCCESS)
        {
          free(aNames );
          free(aValues);
    
          [aResult release];
    
          return nil;
        }
    
      while (aCount--)
        [aResult setObject: [NSString stringWithKn_String: aValues[aCount]]
                    forKey: [NSString stringWithKn_String: aNames [aCount]]];
    
      free(aNames );
      free(aValues);
    }
  return aResult;
}

- (kn_DictionaryRef) kn_DictionaryCopy
{
  kn_MutableDictionaryRef aDictionary =
    kn_DictionaryCreateMutableWithCapacity([self count]);

  NSEnumerator* aNameEnumerator = [[self allKeys] objectEnumerator];
  NSString*     aName;

  while ((aName = [aNameEnumerator nextObject]))
    {
      kn_StringRef aCName  = [aName                      kn_StringCopy];
      kn_StringRef aCValue = [[self objectForKey: aName] kn_StringCopy];

      kn_DictionarySetValue(aDictionary, aCName, aCValue);

      kn_Release(aCName );
      kn_Release(aCValue);
    }

  return aDictionary;
}

@end
