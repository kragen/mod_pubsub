// sampleService.js - Sample Lightweight Web Service Provider

// Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

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
// 
// @KNOWNOW_LICENSE_END@

// $Id: sampleService.js,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

kn_include("kn_lwws.Service");

//////// sampleService

// sampleService class - a lightweight web service provider.
// Implements the kn_lwws.Service interface.

function sampleService(aServer, aTopic)
{
    this.Super = kn_lwws.Service;
    this.Super(aServer, aTopic);
}

sampleService.prototype = new kn_lwws.Service();
sampleService.prototype.onMessage = sampleService_onMessage;

function sampleService_onMessage(aRequest)
{
  var myReply = new Object();
  myReply.kn_payload =
    $_("The request payload contained %{0} characters.\nServer Host: %{1}\nServer Platform: %{2}.",
       aRequest.kn_payload.length.toString(),
       this._server.tunnelURI,
       self.navigator.userAgent);
  this.reply(aRequest, myReply);
}

// End of sampleService.js