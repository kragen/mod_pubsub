/*! @file kn_lwws/ClientRequest.js PubSub Lightweight Web Service Client Request API
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_lwws.ClientRequest");
 * &lt;/script&gt;</b>
 * </pre>
 */

// Copyright 2002-2003 KnowNow, Inc.

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

// $Id: ClientRequest.js,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

if (! self.kn_lwws) kn_lwws = new Object;

//////// ClientRequest

// ClientRequest class - interface for client request objects returned
// by aClient.request().
kn_lwws.ClientRequest = kn_lwws_ClientRequest;

/*! @interface kn_lwws_ClientRequest
 * Interface for lightweight web service Client objects returned by kn_lwws_Client.request()
 * @ctor Use use the kn_lwws_Client.request() factory.
 */
function kn_lwws_ClientRequest()
{
    return void(null);
}

//// abstract instance methods

// aClientRequest.abort() - abort this client request.

/*! 
 * abort this client request.
 */
kn_lwws_ClientRequest.prototype.abort = function()
{
}

// End of ClientRequest.js
