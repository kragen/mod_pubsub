/**
 * PubSub Client Library
 * Event data type and routines
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

#ifndef _KN_KN_EVENT_H_
#define _KN_KN_EVENT_H_ "$Id: kn_Event.h,v 1.2 2003/03/07 06:16:08 wsanchez Exp $"

#ifdef __cplusplus
extern "C" {
#endif

#include "kn_Error.h"
#include "kn_String.h"
#include "kn_Dictionary.h"

/*!
 * @header kn_Event
 * kn_Event implements an event object.
 * 
 * The kn_Event object encapsulates a PubSub event.  The kn_Event
 * object defines the interaction between the PubSub client and the
 * PubSub Server.  The typical PubSub application is event driven: it
 * subscribes to and responds to events received from PubSub servers,
 * and it sends events back to the server to notify others of activity
 * on the client.
 * 
 * An event is comprised of an (unsorted) array of headers.  Each
 * header, in turn, consists of a name and a value.  Both the name and
 * the value in a header are strings.  The kn_Event object associates
 * the names and values, which are represented as kn_String objects.
 * 
 * As a container object, a kn_Event will retain the kn_String
 * references it maintains until they are removed from the event or
 * the event is deallocated, at which point they are released.
 */

/**
 * Provide opaque references to objects
 **/

/*!
 * @typedef    kn_EventRef
 * @abstract   Event object reference
 * @discussion The kn_EventRef type refers to an immutable kn_Event
 *             object.
 */
typedef const struct __kn_Event * kn_EventRef;

/*!
 * @typedef    kn_MutableEventRef
 * @abstract   Mutable event object reference
 * @discussion The kn_EventRef type refers to a mutable kn_Event
 *             object.
 */
typedef struct __kn_Event * kn_MutableEventRef;

/*!
 * @typedef    kn_EventHandler
 * @abstract   Event handler
 * @discussion A kn_EventHandler is a function which acts on an event.
 *             It takes the event to handle as its first argument, and
 *             a pointer to user-defined state data as its second
 *             argument.
 */
typedef void (*kn_EventHandler)(kn_EventRef, void*);

/**
 * Allocators
 **/

/*!
 * @function   kn_EventCreateMutable
 * @discussion kn_EventCreateMutable creates a mutable event.
 * @result Returns a reference to the newly allocated mutable
 *         kn_Event. On failure, returns NULL and sets errno.
 */
kn_MutableEventRef kn_EventCreateMutable ();

/*!
 * @function   kn_EventCreateMutable
 * @discussion kn_EventCreateMutable creates a mutable event.
 * @param aCapacity The recommended allocated capacity of the event.
 *                  Mutable events vary in size as they are
 *                  manipulated, and the allocated size for the
 *                  internal buffer containing the represented event
 *                  headers (names and values) may be larger than the
 *                  capacity needed for the contained references.
 *                  This routine allows the caller to recommend to the
 *                  library that it allocate enough space for
 *                  aCapacity headers pairs based on the caller's
 *                  expected usage of the object, thereby possibly
 *                  avoiding later reallocation of the internal
 *                  memory.
 * @result Returns a reference to the newly allocated mutable
 *         kn_Event.
 */
kn_MutableEventRef kn_EventCreateMutableWithCapacity (size_t aCapacity);

/*!
 * @function   kn_EventCreateCopy
 * @discussion kn_EventCreateMutable creates an immutable event with
 *             the same contents as the specified event.
 * @param aEvent The event to copy.
 * @result Returns a reference to the newly allocated immutable
 *         kn_Event.
 */
kn_EventRef kn_EventCreateCopy (kn_EventRef anEvent);

/*!
 * @function   kn_EventCreateMutableCopy
 * @discussion kn_EventCreateMutableCopy creates a mutable event with
 *             the same contents as the specified event.
 * @param aEvent The event to copy.
 * @result Returns a reference to the newly allocated mutable
 *         kn_Event.
 */
kn_MutableEventRef kn_EventCreateMutableCopy (kn_EventRef anEvent);

/*!
 * @function   kn_EventCreateWithDictionary
 * @discussion kn_EventCreateWithDictionary creates an immutable event
 *             with the same the names and values as the specified
 *             dictionary.
 * @param aDictionary A dictionary to containing header names and values.
 *                    Names and values are copied by reference.
 * @result Returns a reference to the newly allocated immutable
 *         kn_Event.
 */
kn_EventRef kn_EventCreateWithDictionary (kn_DictionaryRef aDictionary);

/*!
 * @function   kn_EventCreateMutableWithDictionary
 * @discussion kn_EventCreateMutableWithDictionary creates a mutable
 *             event with the same the names and values as the
 *             specified dictionary.
 * @param aDictionary The dictionary to get header names and values
 *                    from.  Names are values are copies by reference.
 * @result Returns a reference to the newly allocated mutable
 *         kn_Event.
 */
kn_MutableEventRef kn_EventCreateMutableWithDictionary (kn_DictionaryRef aDictionary);

/*!
 * @function   kn_EventCreateWithSimpleEventFormat
 * @discussion kn_EventCreateWithSimpleEventFormat creates an
 *             immutable event by decoding a string containing
 *             an event encoded in the simple event format.
 * @param anEventString A string containing event data in simple format.
 * @result Returns a reference to the newly allocated immutable
 *         kn_Event.
 */
kn_EventRef kn_EventCreateWithSimpleEventFormat (kn_StringRef anEventString);

/**
 * Accessors
 **/

/*!
 * @function   kn_EventGetCount
 * @discussion kn_EventGetCount accesses the number of
 *             headers in an event.
 * @param anEvent The target event.
 * @result Returns the number of headers in anEvent.
 */
size_t kn_EventGetCount (kn_EventRef anEvent);

/*!
 * @defined    kn_EVENT_ID_HEADER_NAME
 * @discussion kn_EVENT_ID_HEADER_NAME is the name of the header
 *             which contains the PubSub ID in an event.
 */
#define kn_EVENT_ID_HEADER_NAME KNSTR("kn_id")

/*!
 * @defined    kn_EVENT_PAYLOAD_HEADER_NAME
 * @discussion kn_EVENT_PAYLOAD_HEADER_NAME is the name of the
 *             header which contains the payload data in an event.
 */
#define kn_EVENT_PAYLOAD_HEADER_NAME KNSTR("kn_payload")

/*!
 * @defined    kn_EVENT_TIMESTAMP_HEADER_NAME
 * @discussion kn_EVENT_TIMESTAMP_HEADER_NAME is the name of the
 *             header which contains the event timestamp in an event.
 */
#define kn_EVENT_TIMESTAMP_HEADER_NAME KNSTR("kn_time_t")

/*!
 * @defined    kn_EVENT_EXPIRES_HEADER_NAME
 * @discussion kn_EVENT_EXPIRES_HEADER_NAME is the name of the
 *             header which contains the event expiration time in an
 *             event.
 */
#define kn_EVENT_EXPIRES_HEADER_NAME KNSTR("kn_expires")

/*!
 * @defined    kn_EVENT_ROUTE_LOCATION_HEADER_NAME
 * @discussion kn_EVENT_ROUTE_LOCATION_HEADER_NAME is the name of the
 *             header which contains the URI of the route through
 *             which an event was last forwarded.
 */
#define kn_EVENT_ROUTE_LOCATION_HEADER_NAME KNSTR("kn_route_location")

/*!
 * @defined    kn_EVENT_ROUTE_ID_HEADER_NAME
 * @discussion kn_EVENT_ROUTE_ID_HEADER_NAME is the name of the
 *             header which contains the ID of the route through
 *             which an event was last forwarded.
 */
#define kn_EVENT_ROUTE_ID_HEADER_NAME KNSTR("kn_route_id")

/*!
 * @defined    kn_EVENT_ROUTED_FROM_HEADER_NAME
 * @discussion kn_EVENT_ROUTED_FROM_HEADER_NAME is the name of the
 *             header which contains the topic from which an event
 *             was last forwarded.
 */
#define kn_EVENT_ROUTED_FROM_HEADER_NAME KNSTR("kn_routed_from")

/*!
 * @defined    kn_STATUS_EVENT_STATUS_HEADER_NAME
 * @discussion kn_STATUS_EVENT_STATUS_HEADER_NAME is the name of the
 *             header which contains the status code in a status event.
 */
#define kn_STATUS_EVENT_STATUS_HEADER_NAME KNSTR("status")

/*!
 * @function   kn_EventGetValue
 * @discussion kn_EventGetValue accesses the string value of the
 *             header with the specified name in the specified event.
 * @param aEvent The target event.
 * @param aName  The name of the header to get the value from.
 * @result Returns a reference to the kn_String in the specified
 *         header or NULL if there is no header named aName in
 *         anEvent.
 */
kn_StringRef kn_EventGetValue (kn_EventRef anEvent, kn_StringRef aName);

/*!
 * @function   kn_EventGetNamesAndValues
 * @discussion kn_EventGetNamesAndValues accesses the names and
 *             values in the specified event.
 * @param anEvent The target event.
 * @param aNames  An array of strings to be filled in with the name
 *                strings in anEvent.  aNames must be at
 *                least kn_EventGetCount(anEvent) in
 *                size, or NULL if aNames is not to be filled in.
 * @param aValues An array of strings to be filled in with the string
 *                values in anEvent.  aValues must be at
 *                least kn_EventGetCount(anEvent) in
 *                size, or NULL if aValues is not to be filled in.
 * @result Returns kn_SUCCESS.
 */
kn_Error kn_EventGetNamesAndValues (kn_EventRef anEvent, kn_StringRef* aNames, kn_StringRef* aValues);

/*!
 * @function   kn_EventSetValue
 * @discussion kn_EventSetValue sets the string value of the header
 *             with the specified name in the specified (mutable)
 *             event.
 * @param aEvent The target event.
 * @param aName  The name of the header to set the value of.
 * @param aValue The value to set the header name aName to.
 * @result Returns an error status.
 */
kn_Error kn_EventSetValue (kn_MutableEventRef anEvent, kn_StringRef aName, kn_StringRef aValue);

/*!
 * @function   kn_EventRemoveValue
 * @discussion kn_EventRemoveValue removes the header with the specified
 *             name in the specified (mutable) event.
 * @param aEvent The target event.
 * @param aName  The name of the header to remove.
 * @result Returns an error status.
 */
kn_Error kn_EventRemoveValue (kn_MutableEventRef anEvent, kn_StringRef aName);

/*!
 * @function   kn_EventCreateStringByEncodingSimpleFormat
 * @discussion kn_EventCreateStringByEncodingSimpleFormat
 *             encodes an event in the simple event format.
 * @param aEvent The target event.
 * @result Returns an newly allocated immutable string
 *         containing the encoded event.
 */
kn_StringRef kn_EventCreateStringByEncodingSimpleFormat (kn_EventRef anEvent);

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_EVENT_H_ */
