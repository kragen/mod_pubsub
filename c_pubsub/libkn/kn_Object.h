/**
 * PubSub Client Library
 * Base object type and routines
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

#ifndef _KN_KN_OBJECT_H_
#define _KN_KN_OBJECT_H_ "$Id: kn_Object.h,v 1.4 2004/04/19 05:39:08 bsittler Exp $"

#ifdef __cplusplus
extern "C" {
#endif

#include "kn_base.h"
#include "kn_String.h"

/*!
 * @header kn_Object
 * kn_Object implements the base object type.
 * 
 * kn_Object provides the base functionality for all kn_* object
 * types.  All kn_* types are kn_Objects; therefore, any kn_*
 * reference may be cast into a kn_ObjectRef.
 * 
 * Every kn_Object contains a reference count and may be retained or
 * released.  When an object with a retain count of one is released,
 * it is (eventually) deallocated by the library.  (In general, it is
 * deallocated immediately, but this is not guaranteed by the
 * library.)
 * 
 * All object functions which create new objects
 * (eg. kn_StringCreateWithCString) return an object with a retained
 * reference.  It is the responsibility of the caller to release that
 * reference when it is done with the object.  Having released the
 * object, it should no longer be used, as it may no longer be a valid
 * reference.
 * 
 * Any caller of a function which returns an unretained object (for
 * example "get" functions like kn_EventGetValue) must retain that
 * object if it is to keep a reference to the object beyond its scope.
 * That is, the object will not be deallocated before the calling
 * function returns, but if a reference is kept for future use, it
 * must be retained.
 * 
 * Having retained an object, you must also ensure that it is later
 * released when it is no longer referenced so that it can be
 * deallocated.  As an example, a container like kn_Dictionary retains
 * all names and values as they are inserted into the dictionary
 * object and releases them as they are removed.
 */

/**
 * Provide opaque references to objects
 **/

/*!
 * @typedef    kn_ObjectRef
 * @abstract   Object reference
 * @discussion The kn_ObjectRef type refers to an object.
 */
typedef const void * kn_ObjectRef;

/**
 * Accessors
 **/

/*!
 * @function   kn_GetRetainCount
 * @discussion kn_GetRetainCount returns the retain count of an
 *             object.
 * @param anObject The target object.
 * @result Returns the number of retained references to anObject.
 */
unsigned int kn_GetRetainCount (kn_ObjectRef anObject);

/*!
 * @function   kn_CopyDescription
 * @discussion kn_CopyDescription copies a description of an object
 *             into a string.
 * @param anObject The target object.
 * @result Returns a newly allocated string.
 */
kn_StringRef kn_CopyDescription (kn_ObjectRef anObject);

/**
 * Actions
 **/

/*!
 * @function   kn_Retain
 * @discussion kn_Retain increments the retain count of the specified
 *             object by one.
 * @param anObject The target object.
 * @result Returns anObject.
 */
kn_ObjectRef kn_Retain (kn_ObjectRef anObject);

/*!
 * @function   kn_Release
 * @discussion kn_Release decrements the retain count of the specified
 *             object by one.  If the retain count reaches zero, it
 *             may be immediately deallocated.
 * @param anObject The target object.
 */
void  kn_Release (kn_ObjectRef  anObject);
void _kn_Release (kn_ObjectRef* anObject);

/**
 * Aid debugging of use-after-release errors: sets anObject to NULL
 * after performing kn_Release().
 * This doesn't work if you ever pass anything other than an lvalue to
 * kn_Release().
 **/
#ifdef KN_DEBUG_REFS
#define kn_Release(anObject) _kn_Release((kn_ObjectRef*)&(anObject))
#endif

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_OBJECT_H_ */
