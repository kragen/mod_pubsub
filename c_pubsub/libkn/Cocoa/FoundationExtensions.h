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

#import <Foundation/NSObject.h>

@class NSString;
@class NSDictionary;

#include <kn/kn_String.h>
#include <kn/kn_Event.h>

/**
 * Convert NSString* <-> knStringRef
 **/
@interface NSString (kn_Extensions)

+ (NSString*) stringWithKn_String: (kn_StringRef) aString;
- (id       )   initWithKn_String: (kn_StringRef) aString;

- (kn_StringRef) kn_StringCopy;

@end

/**
 * Web codecs
 **/
@interface NSString (kn_Extensions_WebEncoding)

+ (NSString*) stringWithStringByEncodingURLHex          : (NSString*) aString;
- (id       )   initWithStringByEncodingURLHex          : (NSString*) aString;
+ (NSString*) stringWithStringByDecodingURLHex          : (NSString*) aString;
- (id       )   initWithStringByDecodingURLHex          : (NSString*) aString;
+ (NSString*) stringWithStringByEncodingQuotedPrintable : (NSString*) aString;
- (id       )   initWithStringByEncodingQuotedPrintable : (NSString*) aString;
+ (NSString*) stringWithStringByDecodingQuotedPrintable : (NSString*) aString;
- (id       )   initWithStringByDecodingQuotedPrintable : (NSString*) aString;
+ (NSString*) stringWithStringByEncodingBase64          : (NSString*) aString;
- (id       )   initWithStringByEncodingBase64          : (NSString*) aString;

@end

/**
 * Convert NSDictionary* <-> kn_DictionaryRef
 **/
@interface NSDictionary (kn_Extensions)

+ (NSDictionary*) dictionaryWithKn_Dictionary: (kn_DictionaryRef) aDictionary;
- (id           )       initWithKn_Dictionary: (kn_DictionaryRef) aDictionary;

- (kn_DictionaryRef) kn_DictionaryCopy;

@end
