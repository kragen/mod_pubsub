/**
 * PubSub Client Library
 * Dictionary data type and routines
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

#ifndef _KN_KN_DICTIONARY_H_
#define _KN_KN_DICTIONARY_H_ "$Id: kn_Dictionary.h,v 1.1 2002/12/21 03:38:44 bsittler Exp $"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#include "kn_Error.h"
#include "kn_Object.h"
#include "kn_String.h"

/*!
	@header kn_Dictionary
	kn_Dictionary implements a dictionary (associative array) object.

	The kn_Dictionary object is a container object for name/value
	pairs.  Names must be kn_String objects; values may be any
	kn_Object subtype and may not be NULL.  Names in any given
	dictionary must be unique; for a specified name, there is
	either zero or one object in the dictionary associated with
	that name.

	All names and values contained in the dictionary are retained
	by the dictionary while it keeps a reference; names and values
	are released when they are removed from the dictionary or when
	the dictionary is deallocated.
 */

/**
 * Provide opaque references to objects
 **/

/*!
 * @typedef    kn_DictionaryRef
 * @abstract   Dictionary object reference
 * @discussion The kn_DictionaryRef type refers to an immutable
 *             kn_Dictionary object.
 */
typedef const struct __kn_Dictionary * kn_DictionaryRef;

/*!
 * @typedef    kn_MutableDictionaryRef
 * @abstract   Mutable dictionary object reference
 * @discussion The kn_DictionaryRef type refers to a mutable
 *             kn_Dictionary object.
 */
typedef       struct __kn_Dictionary * kn_MutableDictionaryRef;

/**
 * Allocators
 **/

/*!
 * @function   kn_DictionaryCreateMutable
 * @discussion kn_DictionaryCreateMutable creates a mutable
 *             dictionary.
 * @result Returns a reference to the newly allocated mutable
 *         kn_Dictionary. On failure returns NULL and sets errno.
 */
kn_MutableDictionaryRef kn_DictionaryCreateMutable ();

/*!
 * @function   kn_DictionaryCreateMutableWithCapacity
 * @discussion kn_DictionaryCreateMutable creates a mutable
 *             dictionary.
 * @param aCapacity The recommended allocated capacity of the
 *                  dictionary.  Mutable dictionaries vary in size as
 *                  they are manipulated, and the allocated size for
 *                  the internal buffer containing the represented
 *                  names and values may be larger than the capacity
 *                  needed for the contained references.  This routine
 *                  allows the caller to recommend to the library that
 *                  it allocate enough space for aCapacity name/value
 *                  pairs based on the caller's expected usage of the
 *                  object, thereby possibly avoiding later
 *                  reallocation of the internal memory.
 * @result Returns a reference to the newly allocated mutable
 *         kn_Dictionary. On failure returns NULL and sets errno.
 */
kn_MutableDictionaryRef kn_DictionaryCreateMutableWithCapacity (size_t aCapacity);

/*!
 * @function   kn_DictionaryCreateCopy
 * @discussion kn_DictionaryCreateMutable creates a dictionary with
 *             the same contents as the specified dictionary.
 * @param aDictionary The dictionary to copy.
 * @result Returns a reference to the newly allocated immutable
 *         kn_Dictionary. On failure returns NULL and sets errno.
 */
kn_DictionaryRef kn_DictionaryCreateCopy (kn_DictionaryRef aDictionary);

/*!
 * @function   kn_DictionaryCreateMutableCopy
 * @discussion kn_DictionaryCreateMutableCopy creates a mutable
 *             dictionary with the same contents as the specified
 *             dictionary.
 * @param aDictionary The dictionary to copy.
 * @result Returns a reference to the newly allocated mutable
 *         kn_Dictionary. On failure returns NULL and sets errno.
 */
kn_MutableDictionaryRef kn_DictionaryCreateMutableCopy (kn_DictionaryRef aDictionary);

/*!
 * @function   kn_DictionaryCreateWithVAList
 * @discussion kn_DictionaryCreateWithVAList creates an immutable
 *             dictionary with the specified names and values.
 * @param aName  The first name to insert into the dictionary.
 * @param aValue The value corresponding to aName to insert into the
 *               dictionary.
 * @param anArgs The remaining names and values to insert into the
 *               dictionary, in the form of a NULL-terminated va_list
 *               of names and values.
 * @result Returns a reference to the newly allocated immutable
 *         kn_Dictionary. On failure returns NULL and sets errno.
 */
kn_DictionaryRef kn_DictionaryCreateWithVAList (kn_StringRef aName, kn_ObjectRef aValue, va_list anArgs);

/*!
 * @function   kn_DictionaryCreateWithNamesAndValues
 * @discussion kn_DictionaryCreateWithNamesAndValues creates an
 *             immutable dictionary with the specified names and values.
 * @param aName  The first name to insert into the dictionary.
 * @param aValue The value corresponding to aName to insert into the
 *               dictionary.
 * @param ...    The remaining names and values to insert into the
 *               dictionary, in the form of a NULL-terminated list of
 *               names and values.
 * @result Returns a reference to the newly allocated immutable
 *         kn_Dictionary. On failure returns NULL and sets errno.
 */
kn_DictionaryRef kn_DictionaryCreateWithNamesAndValues (kn_StringRef aName, kn_ObjectRef aValue, ...);

/*!
 * @function   kn_DictionaryCreateMutableWithVAList
 * @discussion kn_DictionaryCreateMutableWithVAList creates a mutable
 *             dictionary with the specified names and values.
 * @param aName  The first name to insert into the dictionary.
 * @param aValue The value corresponding to aName to insert into the
 *               dictionary.
 * @param anArgs The remaining names and values to insert into the
 *               dictionary, in the form of a NULL-terminated va_list
 *               of names and values.
 * @result Returns a reference to the newly allocated mutable
 *         kn_Dictionary. On failure returns NULL and sets errno.
 */
kn_MutableDictionaryRef kn_DictionaryCreateMutableWithVAList (kn_StringRef aName, kn_ObjectRef aValue, va_list anArgs);

/*!
 * @function   kn_DictionaryCreateMutableWithNamesAndValues
 * @discussion kn_DictionaryCreateMutableWithNamesAndValues creates a
 *             mutable dictionary with the specified names
 *             and values.
 * @param aName  The first name to insert into the dictionary.
 * @param aValue The value corresponding to aName to insert into the
 *               dictionary.
 * @param ...    The remaining names and values to insert into the
 *               dictionary, in the form of a NULL-terminated list of
 *               names and values.
 * @result Returns a reference to the newly allocated mutable
 *         kn_Dictionary. On failure returns NULL and sets errno.
 */
kn_MutableDictionaryRef kn_DictionaryCreateMutableWithNamesAndValues (kn_StringRef aName, kn_ObjectRef aValue, ...);

/**
 * Accessors
 **/

/*!
 * @function   kn_DictionaryGetCount
 * @discussion kn_DictionaryGetCount accesses the number of
 *             name/value pairs in a dictionary.
 * @param aDictionary The target dictionary.
 * @result Returns the number of name/value pairs in aDictionary.
 */
size_t kn_DictionaryGetCount (kn_DictionaryRef aDictionary);

/*!
 * @function   kn_DictionaryGetValue
 * @discussion kn_DictionaryGetValue accesses the object value
 *             associated with the specified name in the specified
 *             dictionary.
 * @param aDictionary The target dictionary.
 * @param aName       The name associated with the desired value.
 * @result Returns a reference to the desired kn_Object or NULL if
 *         there is no object associated with aName.
 */
kn_ObjectRef kn_DictionaryGetValue (kn_DictionaryRef aDictionary, kn_StringRef aName);

/*!
 * @function   kn_DictionaryGetNamesAndValues
 * @discussion kn_DictionaryGetNamesAndValues accesses the names and
 *             values in the specified dictionary.
 * @param aDictionary The target dictionary.
 * @param aNames      An array of strings to fill in with the name
 *                    strings in aDictionary.  aNames must be at
 *                    least kn_DictionaryGetCount(aDictionary) in
 *                    size, or NULL if aNames is not to be filled in.
 * @param aValues     An array of strings to fill in with the object
 *                    values in aDictionary.  aValues must be at
 *                    least kn_DictionaryGetCount(aDictionary) in
 *                    size, or NULL if aValues is not to be filled in.
 * @result Returns kn_SUCCESS.
 */
kn_Error kn_DictionaryGetNamesAndValues (kn_DictionaryRef aDictionary, kn_StringRef* aNames, kn_ObjectRef* aValues);

/*!
 * @function   kn_DictionaryAddValue
 * @discussion kn_DictionaryAddValue adds the object value
 *             associated with the specified name in the specified
 *             (mutable) dictionary if the name does not already
 *             exist.
 * @param aDictionary The target dictionary.
 * @param aName       The name to associate with the specified value.
 *                    If aName already exists in aDictionary, this function
 *                    does nothing.  Otherwise, a new name and value pair
 *                    is added to aDictionary.
 * @param aValue      The value corresponding to aName.
 * @result Returns kn_SUCCESS if the name and value was added, kn_EXISTS
 *         if aName already exists in aDictionary, or kn_MEMORYFAIL if
 *         a memory allocation failure occurred.
 */
kn_Error kn_DictionaryAddValue (kn_MutableDictionaryRef aDictionary, kn_StringRef aName, kn_ObjectRef aValue);

/*!
 * @function   kn_DictionaryReplaceValue
 * @discussion kn_DictionaryReplaceValue replaces the object value
 *             associated with the specified name in the specified
 *             (mutable) dictionary if the name exists in the
 *             dictionary.
 * @param aDictionary The target dictionary.
 * @param aName       The name to associate with the specified value.
 *                    If aName does not exist in aDictionary, this function
 *                    does nothing.  Otherwise, the value in aDictionary
 *                    associated with aName is replaced with aValue.
 * @param aValue      The value corresponding to aName.
 * @result Returns kn_SUCCESS if the value was replaced, kn_NOENTRY
 *         if aName does not exist in aDictionary, or kn_MEMORYFAIL if
 *         a memory allocation failure occurred.
 */
kn_Error kn_DictionaryReplaceValue (kn_MutableDictionaryRef aDictionary, kn_StringRef aName, kn_ObjectRef aValue);

/*!
 * @function   kn_DictionarySetValue
 * @discussion kn_DictionarySetValue sets the object value
 *             associated with the specified name in the specified
 *             (mutable) dictionary.
 * @param aDictionary The target dictionary.
 * @param aName       The name to associate with the aValue.
 *                    If aName already exists in aDictionary, its value
 *                    is replaced with aValue.  Otherwise, a new name and
 *                    value pair is added to aDictionary.
 * @param aValue      The value corresponding to aName.
 * @result Returns kn_SUCCESS if the value was set or kn_MEMORYFAIL if
 *         a memory allocation failure occurred.
 */
kn_Error kn_DictionarySetValue (kn_MutableDictionaryRef aDictionary, kn_StringRef aName, kn_ObjectRef aValue);

/*!
 * @function   kn_DictionaryRemoveValue
 * @discussion kn_DictionaryRemoveValue removes the specified name and
 *             corresponding value in the specified (mutable)
 *             dictionary.
 * @param aDictionary The target dictionary.
 * @param aName       The name of the entry to remove.
 * @result Returns an error status.
 */
kn_Error kn_DictionaryRemoveValue (kn_MutableDictionaryRef aDictionary, kn_StringRef aName);

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_DICTIONARY_H_ */
