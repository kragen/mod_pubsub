// Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

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

// $Id: cookie_auth.js,v 1.4 2004/04/19 05:39:12 bsittler Exp $

// Client-side "authentication" stored in brower cookies.
// Note that this is not real security like Apache auth is.

// FIXME: This helper library needs to be changed to encode before
// storing its cookie, and decode after recovering it. The encoding
// chosen should preserve all special characters and the encoded form
// should be a valid cookie string (i.e. identifier characters only).
// Maybe it's time to use that JavaScript base64 encoder/decoder
// Scott found a while back.

// FIXME: This helper library sets a cookie that lasts a year.
// Make cookies have a rolling one-week expiration window.

// FIXME: Fix pubsub.cgi to not set userid at all when no http
// auth credentials are supplied, at which point this library
// can be included before do_method=lib.

// FIXME: This interacts very badly with browsers with cookies disabled.
// Instead, we need to set and test a trial cookie.  If that fails, the
// rest should be skipped.

// FIXME: Give people a way to modify/clear their auth information.

if (window.kn_userid == "anonymous") {
    var c = document.cookie;
    if (c.indexOf("kn_userid") == -1) {
        var s = prompt("What is your full name?","");
        if ((s != null) && (s != "")) {
            window.kn_displayname = s;
            window.kn_userid = s.toLowerCase().replace(/[\s\/]/g ,"");
        }
        var ex = new Date((new Date()).getTime() +  365 * 24 * 60 * 60 * 1000).toGMTString(); 
        document.cookie = "kn_userid="+escape(window.kn_userid) +";path=/;expires="+ex+";";
        document.cookie = "kn_displayname="+escape(window.kn_displayname) +";path=/;expires="+ex+"";
    } else {
        var iub = c.indexOf("kn_userid=") + 10;
        var iue = c.indexOf(";",iub);
        if (iue == -1) iue = c.length;
        kn_userid = unescape(c.substring(iub,iue));
        iub = c.indexOf("kn_displayname=") + 15;
        iue = c.indexOf(";",iub);
        if (iue == -1) iue = c.length;
        kn_displayname = unescape(c.substring(iub,iue));
    }
}

// End of cookie_auth.js
