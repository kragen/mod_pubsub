/*! @file kn_cookies.js Cookie Authentication Component
 * <pre>self.kn_include_auth("kn_cookies"); // in self.kn_config()</pre>
 */

// Copyright 2000-2003 KnowNow, Inc.

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

// $Id: kn_cookies.js,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

////////////////////////////////////////////////////////////////////////
// Client-side "authentication" stored in brower cookies.
// Note that this is not real security like Apache auth is.

// FIXME: This helper library sets a cookie that lasts a year.
// Make cookies have a rolling one-week expiration window.

// FIXME: Fix pubsub.cgi to not set userid at all when no http
// auth credentials are supplied, at which point this library
// can be included before do_method=lib.

// FIXME: This interacts very badly with browsers with cookies disabled.

// FIXME: Give people a way to modify/clear their auth information.

function kn_cookies_auth()
{

  // Set a cookie and see if we can access it.
  var ex = new Date((new Date()).getTime() +  365 * 24 * 60 * 60 * 1000).toGMTString(); 
  document.cookie = "kn_cookieTest=foo;path=/;expires="+ex+";";
  
  // Exit if cookies are inaccessible.
  if ((typeof navigator.cookieEnabled == 'boolean' &&
         navigator.cookieEnabled == false) ||
        (!document.cookie) ||
        (document.cookie.indexOf("kn_cookieTest") == -1))
  {
    return;
  }

  if (window.kn_userid == "anonymous") {
      var c = document.cookie;
      if (c.indexOf("kn_userid") == -1) {
          
          // Can't use kn_defaults_get here yet, so...
          var auth_page = (typeof(self.kn_cookiesAuthPage)!="undefined")?self.kn_cookiesAuthPage :  (kn_browser_includePath + "html/cookie_login.html");

          if (self.stop) self.stop();
          self.setTimeout("self.location = '" + auth_page + "'",1);
          
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
        
        // Check to see if we've overridden them from the url.
        var q = location.search;
        if (q || q!="")
        {
                if (q.indexOf("kn_userid=") != -1)
                {
                        var iub = q.indexOf("kn_userid=") + 10;
        var iue = q.indexOf(";",iub);
        if (iue == -1) iue = q.length;
        kn_userid = unescape(q.substring(iub,iue));
                }
                if (q.indexOf("kn_displayname=") != -1)
                {
            iub = q.indexOf("kn_displayname=") + 15;
            iue = q.indexOf(";",iub);
            if (iue == -1) iue = q.length;
            kn_displayname = unescape(q.substring(iub,iue));
                }
        }
        
        
  if (self.kn_cookies_previousCustomInit)
  {
    self.kn_cookies_previousCustomInit();
    return;
  }
}

if (self.kn_customInit)
{
  self.kn_cookies_previousCustomInit = self.kn_customInit;
}

self.kn_customInit = kn_cookies_auth;

// End of cookie_auth.js
