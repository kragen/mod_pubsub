/**
 * PubSub Client Library
 * String data type and routines
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

#ifndef _KN_KN_STRING_H_
#define _KN_KN_STRING_H_ "$Id: kn_String.h,v 1.2 2003/03/07 06:16:08 wsanchez Exp $"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "kn_base.h"
#include "kn_Error.h"

/*!
 * @header kn_String
 * kn_String implements a string object.
 *
 * The kn_String object encapsulates a byte string containing the
 * string data and keeps track of the size of the byte string, as byte
 * strings may contain nul and are therefore not assumed to be be
 * nul-terminated.  Strings may contain binary data or character (user
 * readable) string data.
 *
 * Character strings in kn_String objects are usually assumed by
 * convention to be UTF-8 encoded.  (7-bit ASCII strings need no
 * conversion, as ASCII is a subset of UTF-8.)  That is, event headers
 * and values are generally assumed by clients to be UTF-8 encoded.
 * This is not, however, a requirement.  Exceptions include binary
 * payload data (eg. an image payload), or text payload data when a
 * different content-encoding is specified.
 *
 * Though the byte strings as not specified to be nul-terminated (they
 * may, for example, contain embedded nul characters), the kn_String
 * object appends a nul character to all strings.  If you know that
 * the string data does not contain nul characters, you can pass the
 * value returned by kn_StringGetBytes along to a function (such as
 * printf) that expects a regular nul-terminated C string.  This
 * trailing nul is an interoperability convenience and is not
 * considered part of the string (eg. it is not calculated into the
 * string's size).
 *
 * Note, however, that strings which are created by direct reference
 * to a C string (eg. by calling kn_StringCreateWithBytesNoCopy())
 * will not attempt to write a nul byte after the end of the string.
 * If you want those strings to be nul-terminated, you must ensure
 * this yourself; however, this API does not require that you do so.
 * Authors of software libraries should never depend on a trailing nul
 * in strings they have not created themselves.
 */

/**
 * Provide opaque references to objects
 **/

/*!
 * @typedef    kn_StringRef
 * @abstract   String object reference
 * @discussion The kn_StringRef type refers to an immutable kn_String
 *             object.
 */
typedef const struct __kn_String * kn_StringRef;

/*!
 * @typedef    kn_MutableStringRef
 * @abstract   Mutable string object reference
 * @discussion The kn_StringRef type refers to a mutable kn_String
 *             object.
 */
typedef struct __kn_String * kn_MutableStringRef;

/**
 * Allocators
 **/

/*!
 * @function   kn_StringCreateMutable
 * @discussion kn_StringCreateMutable creates a mutable string.
 * @result Returns a reference to the newly allocated mutable
 *         kn_String.
 */
kn_MutableStringRef kn_StringCreateMutable ();

/*!
 * @function   kn_StringCreateMutableWithSize
 * @discussion kn_StringCreateMutableWithSize creates a mutable string.
 * @param aSize The recommended allocated size of the string.  Mutable
 *              strings vary in size as they are manipulated, and the
 *              allocated size for the internal buffer containing the
 *              represented byte string may be larger than the actual
 *              size of the byte string.  This routine allows the
 *              caller to recommend to the library that it allocate a
 *              byte string buffer at a given size based on the
 *              caller's expected usage of the object, thereby
 *              possibly avoiding later reallocation of the internal
 *              string buffer.
 * @result Returns a reference to the newly allocated mutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_MutableStringRef kn_StringCreateMutableWithSize (size_t aSize);

/*!
 * @function   kn_StringCreateWithCString
 * @discussion kn_StringCreateWithCString creates a string from a
 *             C string.
 * @param aCString The nul-terminated C string from which to initialize
 *                 the kn_String.
 * @result Returns a reference to the newly allocated immutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_StringRef kn_StringCreateWithCString (const char* aCString);

/*!
 * @defined    KNSTR()
 * @discussion KNSTR() makes a kn_String constant from a C string
 *             constant.  The returned reference is to an immutable
 *             kn_String which should not be released (unless it is
 *             first retained).
 */
#define KNSTR(aCString) _kn_StringMakeStringConstantWithCStringConstant(aCString "")

/* Do not call this function directly! Use the above macro. */
kn_StringRef _kn_StringMakeStringConstantWithCStringConstant (const char* aCString);

/*!
 * @function   kn_StringCreateMutableWithCString
 * @discussion kn_StringCreateMutableWithCString creates a mutable
 *             string from a C string.
 * @param aCString The nul-terminated C string from which to initialize
 *                 the kn_String.
 * @result Returns a reference to the newly allocated mutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_MutableStringRef kn_StringCreateMutableWithCString (const char* aCString);

/*!
 * @function   kn_StringCreateWithBytes
 * @discussion kn_StringCreateWithBytes creates a string from a byte
 *             string.
 * @param aBytes The byte string from which to initialize the string.
 * @param aSize  The size in bytes of aBytes.
 * @result Returns a reference to the newly allocated immutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_StringRef kn_StringCreateWithBytes (const char* aBytes, size_t aSize);

/*!
 * @function   kn_StringCreateWithBytesNoCopy
 * @discussion kn_StringCreateWithBytesNoCopy creates a string from a
 *             byte string by holding a reference to the byte string,
 *             rather than copying the string into a new buffer.
 *             This function is provided to avoid copying of memory
 *             when the caller knows it to be unnecessary.
 *             The byte string must not be modified or deallocated
 *             before the resulting kn_String is deallocated.
 *             Note that there will be no trailing nul character
 *             immediately following the string, as provided with
 *             most kn_Strings.
 * @param aBytes             The byte string from which to initialize
 *                           the kn_String.
 * @param aSize              The size in bytes of aBytes.
 * @param aFreeOnDeallocFlag If kn_TRUE, free() will be called on aBytes
 *                           when the new kn_String is deallocated.
 *                           Otherwise, the caller is responsible for
 *                           freeing the memory associated with aBytes.
 * @result Returns a reference to the newly allocated immutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_StringRef kn_StringCreateWithBytesNoCopy (const char* aBytes, size_t aSize, kn_BOOL aFreeOnDeallocFlag);

/*!
 * @function   kn_StringCreateMutableWithBytes
 * @discussion kn_StringCreateMutableWithBytes creates a mutable
 *             string from a byte string.
 * @param aBytes The byte string from which to initialize the string.
 * @param aSize  The size in bytes of aBytes.
 * @result Returns a reference to the newly allocated mutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_MutableStringRef kn_StringCreateMutableWithBytes (const char* aBytes, size_t aSize);

/*!
 * @function   kn_StringCreateCopy
 * @discussion kn_StringCreateCopy creates a string with the same
 *             contents as the specified string.
 * @param aString The string to copy.
 * @result Returns a reference to the newly allocated immutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_StringRef kn_StringCreateCopy (kn_StringRef aString);

/*!
 * @function   kn_StringCreateMutableCopy
 * @discussion kn_StringCreateMutableCopy creates a mutable string
 *             with the same contents the specified string.
 * @param aString The string to copy.
 * @result Returns a reference to the newly allocated mutable
 *         kn_String. On failure returns NULL and sets errno.
 */
kn_MutableStringRef kn_StringCreateMutableCopy (kn_StringRef aString);

/**
 * Accessors
 **/

/*!
 * @function   kn_StringGetBytes
 * @discussion kn_StringGetBytes accesses the bytes represented by the
 *             specified string.
 * @param aString The target string.
 * @result Returns a pointer to the byte string represented by
 *         aString.
 */
const char* kn_StringGetBytes (kn_StringRef aString);

/*!
 * @function   kn_StringGetSize
 * @discussion kn_StringGetSize accesses the size in bytes of the
 *             bytes represented by the specified string.
 * @param aString The target string.
 * @result Returns the size in bytes of the byte string represented by
 *         aString.
 */
size_t kn_StringGetSize (kn_StringRef aString);

/**
 * Actions
 **/

/*!
 * @function   kn_StringCompare
 * @discussion kn_StringCompare compares two strings.
 * @param aString1 The first string to compare.
 * @param aString2 The second string to compare.
 * @result Returns zero if the two strings are identical; less than
 *         zero if aString1 sorts before aString2; and greater than
 *         zero if aString1 sorts after aString2.  Two zero-length
 *         strings are considered equal.
 */
int kn_StringCompare (kn_StringRef aString1, kn_StringRef aString2);

/*!
 * @function   kn_StringCompareCaseInsensitive
 * @discussion kn_StringCompareCaseInsensitive compares two ASCII
 *             strings in a case-insensitive manner.  The compared
 *             strings may not contain embedded NULL characters.
 * @param aString1 The first string to compare.
 * @param aString2 The second string to compare.
 * @result Returns zero if the two strings are identical; less than
 *         zero if aString1 sorts before aString2; and greater than
 *         zero if aString1 sorts after aString2.  Two zero-length
 *         strings are considered equal.
 */
int kn_StringCompareCaseInsensitive (kn_StringRef aString1, kn_StringRef aString2);

/*!
 * @function   kn_StringAppendString
 * @discussion kn_StringAppendString appends one string to the end of
 *             another.
 * @param aString  The string to modify.
 * @param anAppend The string to append to the end of aString.
 * @result Returns kn_MEMORYFAIL if unable to allocate memory,
 *         kn_SUCCESS otherwise.
 */
kn_Error kn_StringAppendString (kn_MutableStringRef aString, kn_StringRef anAppend);

/*!
 * @function   kn_StringAppendCString
 * @discussion kn_StringAppendString appends a nul-termniated C
 *             string to the end of a string.
 * @param aString  The string to modify.
 * @param anAppend The C string to append to the end of aString.
 * @result Returns kn_MEMORYFAIL if unable to allocate memory,
 *         kn_SUCCESS otherwise.
 */
kn_Error kn_StringAppendCString (kn_MutableStringRef aString, const char* anAppend);

/*!
 * @function   kn_StringWriteToStream
 * @discussion kn_StringWriteToStream writes the bytes in the
 *             specified string to the specified file stream.
 * @param aString The string to write.
 * @param aStream The stream to write to.
 * @result Returns the number of bytes written.
 */
size_t kn_StringWriteToStream (kn_StringRef aString, FILE* aStream);

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_STRING_H_ */
