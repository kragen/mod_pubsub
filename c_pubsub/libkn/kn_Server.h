/**
 * PubSub Client Library
 * PubSub Client Library
 *
 * Wilfredo Sanchez
 **/
/**
 * Copyright (c) 2001-2003 KnowNow, Inc.  All rights reserved.
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

#ifndef _KN_KN_SERVER_H_
#define _KN_KN_SERVER_H_ "$Id: kn_Server.h,v 1.3 2003/03/19 05:36:47 ifindkarma Exp $"

#ifdef __cplusplus
extern "C" {
#endif

#include "kn_Error.h"
#include "kn_String.h"
#include "kn_Event.h"

/*!
 * @header kn_Server
 * kn_Server implements a server object.
 * 
 * The kn_Server object represents a PubSub Server.  A server manages
 * shared events, which may be published from or received by any
 * client of the server.
 * 
 * A client may subscribe to a topic on a server by establishing a
 * route from that topic to a destination.  A destination may be
 * another topic, or a callback on the client.
 * 
 * A client may also publish an event to a topic on the server.
 * Events are sent to the topic, where a copy of the event may then be
 * passed along each route established on the topic to the appropriate
 * destination.
 * 
 * kn_Route is able to register "event handler" functions with a
 * server (using kn_RouteCreateFromTopicToFunctionViaServer()), which
 * creates a route from the server back to the client. The kn_Server
 * object can then be asked to process events which are received from
 * the server along any such routes.  As events are received, the
 * server will invoke the event handler to which the event is
 * destined.
 */

/**
 * Provide opaque references to objects
 **/

/*!
 * @typedef    kn_ServerRef
 * @abstract   Server object reference
 * @discussion The kn_ServerRef type refers to an immutable kn_Server
 *             object.
 */
typedef const struct __kn_Server * kn_ServerRef;

/*!
 * @typedef    kn_ServerProcessCallback
 * @abstract   Server connection processing callback
 * @discussion A kn_ServerProcessCallback is a function called by
 *             the receiver of a kn_ServerConnectionCallback when
 *             it notes activity of interest to a kn_Server object.
 *             It takes a reference to the server as an argument.
 */
typedef void (*kn_ServerProcessCallback)(kn_ServerRef aServer, int aFileDescriptor);

/*!
 * @typedef    kn_ServerConnectionCallback
 * @abstract   Server connection state callback
 * @discussion A kn_ServerConnectionCallback is a function called by
 *             a kn_Server whenever the server opens or closes
 *             (changes its interest in) certain file descriptors.
 *             It receives a reference to the server, the file
 *             descriptor affected, handlers for the receiver to call
 *             to notify the server of specific types of activity on the
 *             file descriptor, and a pointer to user-specified data.
 *             Note that modifying the file decriptor will corrupt the
 *             kn_Server's internal state. The receiver may not read to
 *             or write from the received file descriptor; it is meant
 *             for use by select(), poll(), or an equivalent routine.  
 */
typedef void (*kn_ServerConnectionCallback)(kn_ServerRef aServer, int aFileDescriptor,
                                            kn_ServerProcessCallback aReadCallback,
                                            kn_ServerProcessCallback aWriteCallback,
                                            kn_ServerProcessCallback anExceptionCallback,
                                            void* aUserData);

/**
 * Allocators
 **/

/*!
 * @function   kn_ServerCreateWithURI
 * @discussion kn_ServerCreateWithURI creates a server from a URI.
 * @param aURI The URI of the server.
 * @result Returns a reference to the newly allocated server.
 *         On failure returns NULL and sets errno.
 */
kn_ServerRef kn_ServerCreateWithURI (kn_StringRef aURI);

/*!
 * @function   kn_ServerCreateWithURIUserPassword
 * @discussion kn_ServerCreateWithURI creates a server from a URI,
 *             authenticating with the specified user name and password.
 * @param aURI      The URI of the server.
 * @param aUser     The name of the user account on the server to
 *                   authenticate as.
 * @param aPassword The password of aUser on the server.
 * @result Returns a reference to the newly allocated server.
 *         On failure returns NULL and sets errno.
 */
kn_ServerRef kn_ServerCreateWithURIUserPassword (kn_StringRef aURI, kn_StringRef aUser, kn_StringRef aPassword);

/**
 * Accessors
 **/

/*!
 * @function   kn_ServerGetURI
 * @discussion kn_ServerGetURI gets the URI of the specified server.
 * @param aServer The target server.
 * @result Returns a reference to the server URI.
 */
kn_StringRef kn_ServerGetURI (kn_ServerRef aServer);

/*!
 * @function   kn_ServerGetProxyHost
 * @discussion kn_ServerGetProxyHost gets the proxy server host. 
 * @param	   aServer The target server.
 * @result	   Returns a reference to the proxy host.
 */
kn_StringRef kn_ServerGetProxyHost (kn_ServerRef aServer);

/*!
 * @function   kn_ServerGetProxyPort
 * @discussion kn_ServerGetProxyPort gets the proxy server port. 
 * @param	   aServer The target server.
 * @result	   Returns the proxy port as an unsigned short int.
 */
unsigned short int	kn_ServerGetProxyPort (kn_ServerRef aServer);


/*!
 * @function   kn_ServerSetConnectionCallback
 * @discussion kn_ServerSetConnectionCallback sets a function for the
 *             server to invoke whenever it changes its interest in a file
 *             descriptor on which it might be waiting for or sending data
 *             This is useful for clients which have multiple event sources
 *             (eg. user interface events in addition to PubSub events) which
 *             need to be simulataneously monitored asyncronously.
 *             If your program is syncronous, or only needs to respond
 *             to PubSub events, the kn_ServerProcessNextEvent or
 *             kn_ServerProcessEvents routines may prove simpler to
 *             use.
 * @param aServer   The target server.
 * @param aCallback The callback to invoke.
 * @param aUserData A pointer to user-defined data.
 * @result Returns no value.
 */
void kn_ServerSetConnectionCallback (kn_ServerRef aServer, kn_ServerConnectionCallback aCallback, void* aUserData);

/*!
 * @function   kn_ServerSetProxy
 * @discussion kn_ServerSetProxy sets proxy server, proxy host and 
 *             proxy user name/password (if any) to be used to connect 
 *             to the server
 * @param	   aServer  	The target server.
 * @param	   aProxyHost	The proxy server host.
 * @param      aProxyPort   The proxy server port.
 * @param	   aProxyUser   The name of user on proxy to authenticate as.
 * @param      aProxyPasswd The password of aProxyUser on proxy srever.
 * @result     Returns no value.
 */
void kn_ServerSetProxy (kn_ServerRef aServer, 
					  kn_StringRef aProxyHost, 
					  unsigned short int aProxyPort, 
					  kn_StringRef aProxyUser, 
					  kn_StringRef aProxyPasswd);

/**
 * Actions
 **/

/*!
 * @function   kn_ServerWaitForEventData
 * @discussion kn_ServerWaitForEventData blocks until additional event
 *             data is available from the server.  Note there may or
 *             may not be a complete event available for reading.
 * @param aServer The target server.
 * @result Returns kn_SUCCESS when data is available, or an error code.
 */
kn_Error kn_ServerWaitForEventData (kn_ServerRef aServer);

/*!
 * @function   kn_ServerProcessNextEvent
 * @discussion kn_ServerProcessNextEvent processes the next event
 *             received from the server.  If no event has been
 *             received, kn_ServerProcessNextEvent blocks until the
 *             next event is received.
 * @param aServer The target server.
 * @result Returns kn_SUCCESS if it processed an event, or kn_FAIL
 *         and sets errno if an error occurred.
 */
kn_Error kn_ServerProcessNextEvent (kn_ServerRef aServer);

/*!
 * @function   kn_ServerProcessEvents
 * @discussion kn_ServerProcessEvents processes all events that
 *             have been received from or sent to the server (and have
 *             not yet been processed).  kn_ServerProcessEvents sets
 *             errno and returns immediately if it would otherwise block
 *             (ie. there are no available events left to process;
 *             errno is set to EAGAIN) or if there is an error.
 * @param aServer The target server.
 * @result Returns the number of events processed.
 */
size_t kn_ServerProcessEvents (kn_ServerRef aServer);

/*!
 * @function   kn_ServerPublishEventToTopic
 * @discussion kn_ServerPublishEventToTopic publishes an event to the
 *             specified topic via the specified server.
 * @param aServer        The server to publish anEvent to.
 * @param anEvent        The event to publish.
 * @param aTopic         The topic to publish anEvent to.
 * @param aStatusHandler A status handler for any associated status events
 *                       sent back by aServer.
 * @param aUserData      A pointer to user-defined data to be passed as the
 *                       second argument to aStatusHandler.
 * @param aWaitFlag      If kn_FALSE, kn_ServerPublishEventToTopic will return
 *                       immediately if it would otherwise block. This means
 *                       that the event may not have yet been delivered to the
 *                       server, and the caller will need to ask the kn_Server to
 *                       process events in order to ensure delivery of the
 *                       event, which can be detected during invokation of
 *                       aStatusHandler.  If kn_TRUE, kn_ServerPublishEventToTopic
 *                       will block until a status event is received from the
 *                       server.
 * @result Returns a kn_Error.
 */
kn_Error kn_ServerPublishEventToTopic (kn_ServerRef aServer, kn_EventRef anEvent, kn_StringRef aTopic,
                                       kn_EventHandler aStatusHandler, void* aUserData,
                                       kn_BOOL aWaitFlag);

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_SERVER_H_ */
