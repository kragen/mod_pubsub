/**
 * PubSub Client Library
 * Route data type and routines
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

#ifndef _KN_KN_ROUTE_H_
#define _KN_KN_ROUTE_H_ "$Id: kn_Route.h,v 1.2 2003/03/07 06:16:08 wsanchez Exp $"

#ifdef __cplusplus
extern "C" {
#endif

#include "kn_String.h"
#include "kn_Dictionary.h"
#include "kn_Server.h"
#include "kn_Event.h"

/*!
 * @header kn_Route
 * kn_Route implements a route object.
 * 
 * The kn_Route object encapsulates a PubSub route. A route is an
 * attribute of a topic on the server, that causes events which are
 * published to that topic to be forwarded to a specific destination
 * topic.  The destination topic might be another topic on the server,
 * on another server ("route to a topic"), or a special "journal"
 * topic on the server which forwards events back to the client.
 * Events forwarded to journal topics are handled by this library via
 * event callback functions ("route to function").
 * 
 * Routes exist in the server's topic pool, not on the client; the
 * kn_Route object is only a local representation of a route.
 * Deallocation of a kn_Route, therefore, does not by default cause
 * the route to be deleted from the server.  The calling program must
 * call kn_RouteDelete() to delete a route on the server before
 * releasing the route if persistance of the route is not desired.
 * Note that routes to functions may be deleted by the server if the
 * client is no longer responding to the journal topic.  It is advised
 * that you delete routes which you intend to terminate, rather than
 * depending on the server to clean up for you.  You can request that
 * the library delete the route for you on deallocation by setting the
 * option with kn_RouteSetDeleteOnDealloc().
 */

/**
 * Provide opaque references to objects
 **/

/*!
 * @typedef    kn_RouteRef
 * @abstract   Route object reference
 * @discussion The kn_RouteRef type refers to an immutable kn_Route
 *             object.
 */
typedef const struct __kn_Route * kn_RouteRef;

/**
 * Allocators
 **/

/*!
 * @defined    kn_ROUTE_OPTION_MAX_AGE
 * @discussion kn_ROUTE_OPTION_MAX_AGE is the name of the route option
 *             specifying the maximum age of previously posted, unexpired
 *             events which should be forwarded along a new route.
 *             The value of this option should be specified in seconds or
 *             as "infinity".
 */
#define kn_ROUTE_OPTION_MAX_AGE KNSTR("do_max_age")

/*!
 * @defined    kn_ROUTE_OPTION_MAX_COUNT
 * @discussion kn_ROUTE_OPTION_MAX_COUNT is the name of the route option
 *             specifying the maximum number of previously posted,
 *             unexpired events which should be forwarded along a new
 *             route.  The most recent qualifying events are forwarded.
 *             The value of this option should be a number.
 */
#define kn_ROUTE_OPTION_MAX_COUNT KNSTR("do_max_n")

/*!
 * @defined    kn_TOPIC_SUBTOPICS_SUFFIX
 * @discussion kn_TOPIC_SUBTOPICS_SUFFIX, when appended to the end of
 *             a topic, will yield the name of the subtopics topic for
 *             the topic.
 */
#define kn_TOPIC_SUBTOPICS_SUFFIX KNSTR("/kn_subtopics")

/*!
 * @defined    kn_TOPIC_ROUTES_SUFFIX
 * @discussion kn_TOPIC_ROUTES_SUFFIX, when appended to the end of a
 *             topic, will yield the name of the routes topic for the
 *             topic.
 */
#define kn_TOPIC_ROUTES_SUFFIX KNSTR("/kn_routes")

/*!
 * @defined    kn_TOPIC_JOURNAL_SUFFIX
 * @discussion kn_TOPIC_JOURNAL_SUFFIX, when appended to the end of a
 *             topic, will yield the name of the journal topic for the
 *             topic.
 */
#define kn_TOPIC_JOURNAL_SUFFIX KNSTR("/kn_journal")

/*!
 * @function   kn_RouteCreateFromTopicToTopicViaServer
 * @discussion kn_RouteCreateFromTopicToTopicViaServer subscribes a topic
 *             to another topic (ie. creates a route to a topic) via the
 *             specified server.
 * @param aTopic         The topic to route from.
 * @param aDestination   The topic to subscribe to events from aTopic (route to).
 * @param aServer        The server to ask to create the route.
 * @param anOptions      Options to pass to the server (see PubSub Server API Reference Guide).
 * @param aStatusHandler A status handler for any associated status events
 *                       sent back by aServer.
 * @param aUserData      A pointer to user-defined data to be passed as the
 *                       second argument to aDestination and aStatusHandler.
 * @param aWaitFlag      If kn_FALSE, kn_RouteCreateFromTopicToTopicViaServer
 *                       will return immediately if it would otherwise block.
 *                       This means that the route request may not have yet
 *                       been received by the server, and the caller will need to
 *                       ask the kn_Server to process events in order to ensure
 *                       delivery of the request, which can be detected during
 *                       invokation of aStatusHandler.  If kn_TRUE,
 *                       kn_RouteCreateFromTopicToTopicViaServer will block
 *                       until a status event is received from the server.
 * @result Returns the newly allocated route. On failure returns NULL and
 *         sets errno.
 */
kn_RouteRef kn_RouteCreateFromTopicToTopicViaServer (kn_StringRef aTopic, kn_StringRef aDestination,
                                                     kn_ServerRef aServer, kn_DictionaryRef anOptions,
                                                     kn_EventHandler aStatusHandler, void* aUserData,
                                                     kn_BOOL aWaitFlag);

/*!
 * @function   kn_RouteCreateFromTopicToFunctionViaServer
 * @discussion kn_RouteCreateFromTopicToFunctionViaServer subscribes a
 *             function to a topic (ie. creates a route to a function) via
 *             the specified server.
 * @param aTopic         The topic to route from.
 * @param aDestination   The function to call on receipt of event events from
 *                       aTopic (route to).
 * @param aServer        The server to send the route request to.
 * @param anOptions      Options to pass to the server (see event notification
 *                       server documentation).
 * @param aStatusHandler A status handler for any associated status events
 *                       sent back by aServer.
 * @param aUserData      A pointer to user-defined data to be passed as the
 *                       second argument to aDestination and aStatusHandler.
 * @param aWaitFlag      If kn_FALSE, kn_RouteCreateFromTopicToFunctionViaServer
 *                       will return immediately if it would otherwise block.
 *                       This means that the route request may not have yet
 *                       been received by the server, and the caller will need
 *                       ask the kn_Server to process events in order to ensure
 *                       delivery of the request, which can be detected during
 *                       invokation of aStatusHandler.  If kn_TRUE,
 *                       kn_RouteCreateFromTopicToFunctionViaServer will block
 *                       until a status event is received from the server.
 * @result Returns the newly allocated route. On failure returns NULL and
 *         sets errno.
 */
kn_RouteRef kn_RouteCreateFromTopicToFunctionViaServer (kn_StringRef aTopic, kn_EventHandler aDestination,
                                                        kn_ServerRef aServer, kn_DictionaryRef anOptions,
                                                        kn_EventHandler aStatusHandler, void* aUserData,
                                                        kn_BOOL aWaitFlag);

/**
 * Accessors
 **/

/*!
 * @function   kn_RouteGetID
 * @discussion kn_RouteGetID accesses a route's ID.
 * @param aRoute The target route.
 * @result Returns the route's ID.
 */
kn_StringRef kn_RouteGetID (kn_RouteRef aRoute);

/*!
 * @function   kn_RouteGetServer
 * @discussion kn_RouteGetServer accesses the server which is handling
 *             a route.
 * @param aRoute The target route.
 * @result Returns the route's server.
 */
kn_ServerRef kn_RouteGetServer (kn_RouteRef aRoute);

/*!
 * @function   kn_RouteGetTopic
 * @discussion kn_RouteGetTopic accesses a route's topic.
 * @param aRoute The target route.
 * @result Returns the route's topic.
 */
kn_StringRef kn_RouteGetTopic (kn_RouteRef aRoute);

/*!
 * @function   kn_RouteGetURI
 * @discussion kn_RouteGetURI accesses a route's location.
 * @param aRoute The target route.
 * @result Returns the route's location.
 */
kn_StringRef kn_RouteGetURI (kn_RouteRef aRoute);

/*!
 * @function   kn_RouteGetDeleteOnDealloc
 * @discussion kn_RouteGetDeleteOnDealloc queries a kn_Route as to whether
 *             it will automatically delete the route on the server as
 *             part of its deallocation.
 * @param aRoute The target route.
 * @result Returns kn_TRUE if the route will be deleted on dealloc,
 *         kn_FALSE otherwise.
 */
kn_BOOL kn_RouteGetDeleteOnDealloc (kn_RouteRef aRoute);

/*!
 * @function   kn_RouteSetDeleteOnDealloc
 * @discussion kn_RouteSetDeleteOnDealloc tells the route object whether to
 *             delete the route on the server as part of its deallocation.
 * @param aRoute The target route.
 * @param aFlag  If kn_TRUE, the route is deleted when the kn_Route object
 *               is deallocated.  Otherwise, the route is not automatically
 *               deleted.  Note that automatic deletion is non-blocking
 *               and that delivery of the delete request to the server
 *               is not guaranteed, as additional server processing may be
 *               necessary and there is no way for you to verify that the
 *               server received and acknowledged the request.  If you need
 *               to ensure that the delete request is sent, you will need to
 *               call kn_RouteDelete, which allows you to register for a
 *               status event callback.
 * @result Returns no value.
 */
void kn_RouteSetDeleteOnDealloc (kn_RouteRef aRoute, kn_BOOL aFlag);

/**
 * Actions
 **/

/*!
 * @function   kn_RouteDelete
 * @discussion kn_RouteDelete deletes a route.
 * @param aRoute         The target route.
 * @param aStatusHandler A status handler for any associated status events
 *                       sent back by aServer.
 * @param aUserData      A pointer to user-defined data to be passed as the
 *                       second argument to aDestination and aStatusHandler.
 * @param aWaitFlag      If kn_FALSE, kn_RouteDelete will return immediately
 *                       if it would otherwise block.
 *                       This means that the delete request may not have yet
 *                       been received by the server, and the caller will need
 *                       ask the kn_Server to process events in order to ensure
 *                       delivery of the request, which can be detected during
 *                       invokation of aStatusHandler.  If kn_TRUE,
 *                       kn_RouteDelete will block until a status event is
 *                       received from the server.
 * @result Returns an error status.
 */
kn_Error kn_RouteDelete (kn_RouteRef aRoute, kn_EventHandler aStatusHandler, void* aUserData,
                         kn_BOOL aWaitFlag);

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_EVENT_H_ */
