/**
 * PubSub Client Library
 * URI string routines
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

#ifndef _KN_KN_STRING_URI_H_
#define _KN_KN_STRING_URI_H_ "$Id: kn_String_URI.h,v 1.4 2004/04/19 05:39:08 bsittler Exp $"

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "kn_String.h"

/*!
 * @header kn_String_URI
 * kn_String_URI extends the kn_String API to add URI parsing
 * routines.
 *       
 * URI strings passed into these routines are expected to be
 * properly formed URIs;  if malformed URIs are received, the
 * resulting behavior is undefined.
 */

/*!
 * @enum kn_Protocol
 * @discussion Protocols
 * @constant kn_PROTOCOL_INVALID  No protocol (error)
 * @constant kn_PROTOCOL_UNKNOWN  Unknown protocol
 * @constant kn_PROTOCOL_HTTP     HTTP
 * @constant kn_PROTOCOL_HTTPS    HTTP over SSL (HTTPS)
 */
typedef enum {
  kn_PROTOCOL_INVALID  = 0,
  kn_PROTOCOL_UNKNOWN,

  kn_PROTOCOL_HTTP     = 100,
  kn_PROTOCOL_HTTPS

} kn_Protocol;

/**
 * Accessors
 **/

/*!
 * @function   kn_StringGetProtocolFromName
 * @discussion kn_StringGetProtocolFromName converts a protocol
 *             name into a kn_Protocol.
 * @param aName The name of the protocol.
 * @result Returns a kn_Protocol.
 */
kn_Protocol kn_StringGetProtocolFromName (kn_StringRef aName);

/*!
 * @function   kn_StringCreateWithProtocolFromURI
 * @discussion kn_StringCreateWithProtocolFromURI creates a new
 *             string by extracting the protocol name from the
 *             specified URI.
 * @param aURI The source URI.
 * @result Returns a reference to a newly allocated immutable
 *         kn_String. On failure returns NULL.
 */
kn_StringRef kn_StringCreateWithProtocolFromURI (kn_StringRef aURI);

/*!
 * @function   kn_StringGetProtocolFromURI
 * @discussion kn_StringGetProtocolFromURI gets the protocol
 *             from the specified URI.
 * @param aURI The source URI.
 * @result Returns a kn_Protocol. On failure returns
 *         kn_PROTOCOL_UNKNOWN.
 */
kn_Protocol kn_StringGetProtocolFromURI (kn_StringRef aURI);

/*!
 * @function   kn_StringCreateWithUserNameFromURI
 * @discussion kn_StringCreateWithUserNameFromURI creates a new
 *             string by extracting the user name from the
 *             specified URI.
 * @param aURI The source URI.
 * @result Returns a reference to a newly allocated immutable
 *         kn_String. On failure returns NULL.
 */
kn_StringRef kn_StringCreateWithUserNameFromURI (kn_StringRef aURI);

/*!
 * @function   kn_StringCreateWithPasswordFromURI
 * @discussion kn_StringCreateWithPasswordFromURI creates a new
 *             string by extracting the password from the
 *             specified URI.
 * @param aURI The source URI.
 * @result Returns a reference to a newly allocated immutable
 *         kn_String. On failure returns NULL.
 */
kn_StringRef kn_StringCreateWithPasswordFromURI (kn_StringRef aURI);

/*!
 * @function   kn_StringCreateWithHostNameFromURI
 * @discussion kn_StringCreateWithHostNameFromURI creates a new
 *             string by extracting the host name from the
 *             specified URI.
 * @param aURI The source URI.
 * @result Returns a reference to a newly allocated immutable
 *         kn_String. On failure returns NULL.
 */
kn_StringRef kn_StringCreateWithHostNameFromURI (kn_StringRef aURI);

/*!
 * @function   kn_StringGetPortFromURI
 * @discussion kn_StringGetPortFromURI gets the port number from the
 *             specified URI.
 * @param aURI The source URI.
 * @result Returns a 16-bit unsigned integer port number.
 */
unsigned short int kn_StringGetPortFromURI (kn_StringRef aURI);

/*!
 * @function   kn_StringCreateWithPathFromURI
 * @discussion kn_StringCreateWithPathFromURI creates a new
 *             string by extracting the path from the
 *             specified URI.
 * @param aURI The source URI.
 * @result Returns a reference to a newly allocated immutable
 *         kn_String. On failure returns NULL.
 */
kn_StringRef kn_StringCreateWithPathFromURI (kn_StringRef aURI);

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_STRING_URI_H_ */
