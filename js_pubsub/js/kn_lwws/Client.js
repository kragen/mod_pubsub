/*! @file kn_lwws/Client.js PubSub Lightweight Web Service Client API
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_lwws.Client");
 * &lt;/script&gt;</b>
 * </pre>
 */

// Copyright 2002-2004 KnowNow, Inc.  All rights reserved.

// @KNOWNOW_LICENSE_START@
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the KnowNow, Inc., nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// @KNOWNOW_LICENSE_END@
// 

// $Id: Client.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $

if (! self.kn_lwws) kn_lwws = new Object;

kn_include("kn_lwws.ClientRequest");

//////// Client

// Client class - Lightweight Web Service Client

//// constructors

kn_lwws.Client = kn_lwws_Client;

// new kn_lwws_Client(aServer, aTopic) - constructor for lightweight web
// service Client objects. Requires a pubsub client library instance (aServer,
// i.e. kn) and a topic for the service (aTopic).

/*! @class kn_lwws_Client
 * Constructor for lightweight web service Client objects. Requires a pubsub client library instance
 * @ctor Constructor for lightweight web service Client objects.
 * @tparam object aServer a pubsub client library instance
 * @tparam string aTopic a topic for the service
 */
function kn_lwws_Client(aServer, aTopic)
{
    // private instance variables

    // pubsub client library instance
    this._server =
        aServer;

    // service topic
    this._topic =
        aTopic;

    // Hack: set up a subscription and toss it, just to get the journal started.
    var rid = kn.subscribe("/kn_journal",
                           (function (event) {} ),
                           ({}),
                           ({ onError: (function (event) {}) }));
    kn.unsubscribe(rid, ({ onError: (function (event) {}) }));
}

//// instance methods

// aClientRequest = aClient.request(aRequest, aHandler,
// aStatusHandler)

/*! 
 * invoke the service with a request event
 * (aRequest) and install a reply handler (aHandler) and an optional
 * status handler for the publication to the service topic
 * (aStatusHandler). Returns a client request object (aClientRequest)
 * which can be told to abort(). The reply handler (aHandler) must
 * provide an onMessage(anEvent) callback to handle reply events.
 *
 * @tparam event aRequest a request event
 * @tparam function aHandler a reply handler to be installed
 * @tparam function aStatusHandler on optional status handler
 * @treturn kn_lwws_ClientRequest a ClientRequest object.
 */
kn_lwws_Client.prototype.request = function(aRequest, aHandler, aStatusHandler)
{
  var my_RequestHandler =
        new kn_lwws_Client__Request(
            this._server,
            aHandler,
            aStatusHandler
            );

    // copy the request
    var myRequestCopy =
        new Object();
    for (var key in aRequest)
    {
        myRequestCopy[key] =
            aRequest[key];
    }

    // modify the copy to set the reply-to and reply-uri headers
    myRequestCopy["reply-to"] =
        this._server.tunnelURI;
    myRequestCopy["reply-uri"] =
        my_RequestHandler.getURI();

    // send the request
    this._server.publish(
        this._topic,
        myRequestCopy,
        my_RequestHandler);

    // return the client request object
    return my_RequestHandler;
}

// _RequestHandler class - a private implementation of the
// ClientRequest interface, the status handler interface, and the
// event handler interface.

//// constructors

kn_lwws.Client__Request = kn_lwws_Client__Request;

// new kn_lwws__RequestHandler(aHandler, aStatusHandler) - constructor for
// Lightweight Web Service Client request objects.  Requires a
// pubsub client library instance (aServer, i.e. kn), a reply event handler
// (aHandler) and a publication status handler (aStatusHandler).
function kn_lwws_Client__Request(aServer, aHandler, aStatusHandler)
{
    // private instance variables

    // pubsub client library instance
    this._server =
        aServer;

    // reply handler
    this._handler =
        aHandler;

    // status handler for the publication to the service topic
    this._statusHandler =
        aStatusHandler;

    // forge a URI for the reply
    this._uri =
        this._server.tunnelURI + "/_RequestHandler/" + Math.random();

    // trap reply events
    this._server.setHandler(this._uri,
                            kn_lwws_Client__Request._onMessageFunction,
                            this);
}

kn_lwws_Client__Request.prototype = new kn_lwws_ClientRequest();

//// instance methods

// a_RequestHandler.abort()
kn_lwws_Client__Request.prototype.abort = kn_lwws_Client__Request_abort;

// a_RequestHandler.onMessage(aReply)
kn_lwws_Client__Request.prototype.onMessage = kn_lwws_Client__Request_onMessage;

// a_RequestHandler.onSuccess(anEvent)
kn_lwws_Client__Request.prototype.onSuccess = kn_lwws_Client__Request_onSuccess;

// a_RequestHandler.onError(anEvent)
kn_lwws_Client__Request.prototype.onError = kn_lwws_Client__Request_onError;

// aURI = a_RequestHandler.getURI()
kn_lwws_Client__Request.prototype.getURI = kn_lwws_Client__Request_getURI;

//// default instance method implementations

// a_RequestHandler.abort() - abort this client request.
function kn_lwws_Client__Request_abort()
{
    this._server.clearHandler(this._uri);
}

// a_RequestHandler.onMessage(aReply) - handle a reply event (aReply).
function kn_lwws_Client__Request_onMessage(aReply)
{
    // clean up
    this.abort();

    // invoke the user reply handler
    this._handler.onMessage(aReply);
}

// a_RequestHandler.onSuccess(anEvent) - handle a publication success
// status event.
function kn_lwws_Client__Request_onSuccess(anEvent)
{
    // invoke the user status handler
    kn_doStatus(anEvent, this._statusHandler);
}

// a_RequestHandler.onError(anEvent) - handle a publication error
// status event.
function kn_lwws_Client__Request_onError(anEvent)
{
    // clean up
    this.abort();

    // invoke the user status handler
    kn_doStatus(anEvent, this._statusHandler);
}

// aURI = a_RequestHandler.getURI() - find the identity of this client
// request.
function kn_lwws_Client__Request_getURI()
{
    return this._uri;
}

//// private class methods

// Client__Request__onMessageFunction(aReply, a_RequestHandler)
kn_lwws_Client__Request._onMessageFunction = kn_lwws_Client__Request__onMessageFunction;

//// default private class method implementations

// Client__Request__onMessageFunction(aReply,
// a_RequestHandler) - a tiny functional-to-OO bridge.
function kn_lwws_Client__Request__onMessageFunction(aReply, a_RequestHandler)
{
    a_RequestHandler.onMessage(aReply);
}

// End of Client.js
