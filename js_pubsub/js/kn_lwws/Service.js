/*! @file kn_lwws/Service.js Lightweight Web Service Provider API
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_lwws.Service");
 * &lt;/script&gt;</b>
 * </pre>
 */

// Copyright 2003-2003 KnowNow, Inc.

// @KNOWNOW_LICENSE_BEGIN@

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in
// the documentation and/or other materials provided with the
// distribution.
// 
// 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
// be used to endorse or promote any product without prior written
// permission from KnowNow, Inc.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// @KNOWNOW_LICENSE_END@

// $Id: Service.js,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

if (! self.kn_lwws) kn_lwws = new Object;

kn_createContext("kn_lwws_Service");

//////// Service

// Service class - Lightweight Web Service Provider

//// constructors

kn_lwws.Service = kn_lwws_Service;

// new Service(aServer, aTopic) - abstract constructor for lightweight
// web service Service objects. Requires a pubsub client library instance
// (aServer, i.e. kn,) a topic for the service (aTopic).

/*! @class kn_lwws_Service
 * Abstract constructor for lightweight web service Service objects. Requires a pubsub client library instance.
 * @ctor Constructor for lightweight web service Service objects.
 * @tparam object aServer a pubsub client library instance
 * @tparam string aTopic a topic for the service
 */
function kn_lwws_Service(aServer, aTopic)
{
    // private

    // pubsub client library instance
    this._server =
        aServer;

    // service topic
    this._topic =
        aTopic;

    // subscription id
    this._id = null;
}

//// abstract instance methods

// aService.onMessage(aRequest) - handle a request (aRequest).

//// instance methods

/*! 
 * send a reply (aReply) corresponding to a particular request (aRequest,)
 * optionally with a publication status handler (aStatusHandler).
 *
 * @tparam kn_lwws_ClientRequest aRequest A ClientRequest object
 * @tparam object aReply a reply to the request
 * @tparam function aStatusHandler an optional status handler
 * @treturn void Nothing.
 */
kn_lwws_Service.prototype.reply = function(aRequest, aReply, aStatusHandler)
{
    // copy the reply
    var myReplyCopy =
        new Object();
    for (var key in aReply)
    {
        myReplyCopy[key] =
            aReply[key];
    }

    var replyURI = aRequest["reply-uri"];

    if (replyURI != null)
    {
        // modify the copy to set the kn_route_location and kn_uri headers
        myReplyCopy.kn_route_location =
            replyURI;
        myReplyCopy.kn_uri =
            replyURI;
    }

    var replyTo = aRequest["reply-to"];

    if (replyTo != null)
    {
        // HACK: We shorten the reply-to URI iff it appears to be a topic
        // on the local server (this is a workaround for buggy old Perl
        // and C++ servers.) This hack only works when the requesting
        // client is using exactly the same server URI string we are
        // using.

        if (replyTo.indexOf(kn_server + "/") == 0)
        {
            replyTo = replyTo.substring(kn_server.length);
        }

        // send the reply
        this._server.publish(
            replyTo,
            myReplyCopy,
            aStatusHandler);
    }
    else
    {
        // HACK: spoof a status event if no replyTo topic was
        // specifiedSpo

        var anEvent = new Object;

        anEvent.status =
            kn_lwws_Service$("500 Internal Server Error");

        anEvent.kn_payload =
            kn_lwws_Service$("Internal Server Error: No 'reply-to' in kn_lwws.Service.reply");

        // invoke the user status handler
        kn_doStatus(anEvent, aStatusHandler);
    }
}

/*! 
 * start processing requests, optionally with subscription options (anOptions) and
 * optionally with a subscription status handler (aStatusHandler).
 *
 * @tparam object anOptions An object with optional subscription options
 * @tparam function aStatusHandler an optional status handler
 * @treturn void Nothing.
 */
kn_lwws_Service.prototype.start = function(anOptions, aStatusHandler)
{
    this._id =
    this._server.subscribe(this._topic,
      this,
      anOptions,
      aStatusHandler);
}

/*! 
 * stop processing further requests, optionally with an unsubscription status handler (aStatusHandler).
 *
 * @tparam function aStatusHandler an optional status handler
 * @treturn void Nothing.
 */
kn_lwws_Service.prototype.stop = function(aStatusHandler)
{
  this._server.unsubscribe(this._id, aStatusHandler);
  this._id = null;
}

// End of Service.js
