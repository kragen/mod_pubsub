/**
 * PubSub Client Library
 * String encodings for web
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

#ifndef _KN_KN_STRING_WEBENCODING_H_
#define _KN_KN_STRING_WEBENCODING_H_ "$Id: kn_String_WebEncoding.h,v 1.2 2003/03/07 06:16:08 wsanchez Exp $"

#ifdef __cplusplus
extern "C" {
#endif

#include "kn_String.h"

/*!
 * @header kn_String_WebEncoding
 * kn_String_WebEncoding extends the kn_String API to add
 * encoding routines used by web applications.
 */

/**
 * Allocators
 **/

/*!
 * @function   kn_StringCreateByEncodingQuotedPrintable
 * @discussion kn_StringCreateByEncodingQuotedPrintable creates a new
 *             string by quoting the characters in the specified
 *             string using the quoted printable encoding.  All
 *             characters other than the unreserved character set as
 *             specified by the URI General Syntax (see RFC 2396,
 *             section 2.3) are encoded.
 * @param aString The string to encode.
 * @result Returns a reference to the newly allocated immutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_StringRef kn_StringCreateByEncodingQuotedPrintable (kn_StringRef aString);

/*!
 * @function   kn_StringCreateByDecodingQuotedPrintable
 * @discussion kn_StringCreateByDecodingQuotedPrintable creates a new
 *             string by unquoting the quoted printable encoded
 *             characters in the specified string.
 * @param aString The string to decode.
 * @result Returns a reference to the newly allocated immutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_StringRef kn_StringCreateByDecodingQuotedPrintable (kn_StringRef aString);

/*!
 * @function   kn_StringCreateByEncodingURLHex
 * @discussion kn_StringCreateByEncodingURLHex creates a new string by
 *             quoting the characters in the specified string using
 *             the URL hexadecimal encoding.  All characters other
 *             than the unreserved character set as specified by the
 *             URI General Syntax (see RFC 2396, section 2.3) are
 *             encoded.
 * @param aString The string to encode.
 * @result Returns a reference to the newly allocated immutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_StringRef kn_StringCreateByEncodingURLHex (kn_StringRef aString);

/*!
 * @function   kn_StringCreateByDecodingURLHex
 * @discussion kn_StringCreateByDecodingURLHex creates a new string by
 *             unquoting the URL hexadecimal encoded characters in the
 *             specified string.
 * @param aString The string to decode.
 * @result Returns a reference to the newly allocated immutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_StringRef kn_StringCreateByDecodingURLHex (kn_StringRef aString);

/*!
 * @function   kn_StringCreateByEncodingBase64
 * @discussion kn_StringCreateByEncodingBase64 creates a new string by
 *             encoding the characters in the specified string using
 *             the Base64 encoding.
 * @param aString The string to encode.
 * @result Returns a reference to the newly allocated immutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_StringRef kn_StringCreateByEncodingBase64 (kn_StringRef aString);

/**
 * Actions
 **/

/*!
 * @function   kn_StringAppendPostField
 * @discussion kn_StringAppendPostField appends an HTTP POST field with
 *             the specified name and value to a string.  The field name
 *             and value are URL hexadecimal encoded and are separated
 *             by an '=' character.  If aPostString has a non-zero length,
 *             a ';' character will precede the appended field data.
 * @param aPostString The target string.
 * @param aName       The name of the POST field to append to aPostString.
 * @param aValue      The value of the POST field to append to aPostString.
 * @result Returns kn_MEMORYFAIL if there was trouble appending to aString
 *         or kn_SUCCESS if successful.
 */
kn_Error kn_StringAppendPostField(kn_MutableStringRef aPostString, kn_StringRef aName, kn_StringRef aValue);

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_STRING_WEBENCODING_H_ */
