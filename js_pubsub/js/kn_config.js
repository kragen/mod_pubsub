/*! @file kn_config.js PubSub Browser configuration file
 * <pre>
 * <b>&lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;</b>
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * </pre>
 */

// Copyright 2003 KnowNow, Inc.
 
// @KNOWNOW_LICENSE_START@

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

// $Id: kn_config.js,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

if (! self.kn_browser_includePath)
{

// Uncomment and edit next line to point to location of
// JavaScript PubSub Libraries installation:

//kn_browser_includePath = "/js_pubsub/js/";

}

if (! self.kn_config_server)
{

// Cross-Domain Configuration:

// By default, the JavaScript PubSub Client Library assumes that all
// of the HTML pages that compose a PubSub-enabled application
// will be fetched through a PubSub Server's built-in web server
// component.  Sometimes, however, it is desirable
// to be able to serve the application from a different web
// server. For this to work correctly, there must be some cooperation
// between the pages of the PubSub-enabled application and
// a PubSub Server to sidestep the JavaScript security
// restrictions imposed by popular web browsers.

// The first step is to configure your PubSub Server for
// cross-domain operation. Please refer to your PubSub Server's
// configuration file for details (search for "Cross-Domain".)

// Second, the kn_config_server parameter must be set to the URL where
// your PubSub Server is found. To do so, uncomment the following
// line and edit it to reflect the "javascript_kn_server" setting from
// your PubSub Server's configuration file:

//kn_config_server = "http://knserver.mycompany.com/kn";

// Note that all PubSub-enabled applications using that
// PubSub Server must be accessed by URLs in the specified domain.

}

if (! self.kn_config_connector)
{

// Edit the following line to select the appropriate
// JavaScript PubSub Client Library build, one of:

// "kn_raw" - unlocalized, debug, uncompressed
// "knl_raw" - localized, debug, uncompressed
// "knp_raw" - unlocalized, non-debug, uncompressed
// "knl" - localized, debug, compressed [for development]
// "knp" - unlocalized, non-debug, compressed [for production]

self.kn_config_connector = "knl";

}

kn_config = self.kn_config || function()
{

// Frameless (iframe) transport (IFRAME-supporting browsers only).
self.kn_include_plugin("kn_frameless");

// Pop-up window transport (all supported browsers).
self.kn_include_plugin("kn_popup");

// Cookie auth is enabled by default. Comment out next line to disable it.
self.kn_include_auth("kn_cookies");

self.kn_include_connector(self.kn_config_server, self.kn_config_connector || "knl");

};

// End of kn_config.js
