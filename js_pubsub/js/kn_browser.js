/*! @file kn_browser.js PubSub Browser Library
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;</b>
 * </pre>
 */

// Copyright 2003-2004 KnowNow, Inc.  All rights reserved.
 
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

// $Id: kn_browser.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $


// Define dependencies for bundled modules.

// FIXME: this list should be auto-generated!

kn_include_define("kn_im", ["kn_buddylist"]);
kn_include_define("kn_buddylist", ["kn_activepanels","kn_htmlsafe","kn_defaults"]);
kn_include_define("kn_chatlet", ["kn_activepanels","kn_htmlsafe","kn_defaults"]);
kn_include_define("kn_activepanels", ["kn_defaults"]);
kn_include_define("kn_htmlsafe", ["kn_defaults"]);
kn_include_define("plugins/kn_popup", ["kn_defaults"]);
kn_include_define("kn_screenpops", ["kn_lwws.Service"]);
kn_include_define("kn_lwws.Client", ["kn_lwws.ClientRequest"]);
kn_include_define("kn_screenprompt", ["kn_activepanels","kn_htmlsafe","kn_defaults","kn_lwws.Service"]);

// Initialize kn_include list with built-in module names.

self.kn_include_loaded = [
    "kn_browser",
    "kn_onload",
    "kn_popwindow",
    "kn_include"
];

if (typeof(kn_browser_includePath)=="undefined") kn_browser_includePath = "../../js/";

///////////////////////////////////////////////////////////////////////
// kn_onload
///////////////////////////////////////////////////////////////////////

kn_onloadHandlers = [];
kn_onloadWatchdog = null;

/*!
 * Registers a function to be executed on page load.
 *
 * @tparam function fn The function to be executed at page load.
 * @treturn void Nothing.
 */
function kn_onload_addHandler(fn)
{
  if (window.addEventListener)
  {
    window.addEventListener("load", fn, false);
    return true;
  } else if (window.attachEvent)
  {
    var r = window.attachEvent("onload", fn);
    return r;
  } else 
  {
    kn_onloadHandlers[kn_onloadHandlers.length] = fn;
    if (kn_onloadWatchdog == null)
    {
      window.onload = kn_onload_onLoad;
      kn_onload_startWatchdog();
    }
  }
}

function kn_onload_onLoadAlias(){}

function kn_onload_onLoad()
{
  clearInterval(kn_onloadWatchdog);
  for (var i=0;i<kn_onloadHandlers.length;i++)
  {
    kn_onload_onLoadAlias = kn_onloadHandlers[i];
    kn_onload_onLoadAlias();
  }
}

function kn_onload_startWatchdog()
{
  kn_onloadWatchdog = setInterval("kn_onload_checkOnload();",10);
}

function kn_onload_checkOnload()
{
  if (window.onload != kn_onload_onLoad)
  {
    kn_onload_addHandler(window.onload);
    window.onload = kn_onload_onLoad
  }
}

///////////////////////////////////////////////////////////////////////
// kn_popwindow
//////////////////////////////////////////////////////////////////////

function kn_popwindow(url,name,w,h,x,y,resize,scroll)
{
    var opts = new Array;
    opts[opts.length] = "screenX=" + (x||400);
    opts[opts.length] = "screenY=" + (y||400);
    opts[opts.length] = "left=" + (x||400);
    opts[opts.length] = "top=" + (y||400);
    opts[opts.length] = "width=" + (w||400);
    opts[opts.length] = "height=" + (h||400);
    opts[opts.length] = "resizeable=" + ((resize) ? "yes" : "no");
    opts[opts.length] = "scrollbars=" + ((scroll) ? "yes" : "no");

    var win = self.open(url,
                   name,
                   opts.join(","));
    
    //win.opener = self;
    return win;  
};


///////////////////////////////////////////////////////////////////////
// kn_include
///////////////////////////////////////////////////////////////////////

// aCanonicalScript = kn_include_canonicalName(aScript) - convert a
// script name (aScript) to canonical form (aCononicalScript);
// currently this converts Java-style '.'-notation to
// subdirectory-style '/'-notation.

function kn_include_canonicalName(aScript)
{
    if (aScript.indexOf("/") == -1)
    {
        // Allow Java-style "." notation for scripts in subdirectories.
        aScript = aScript.split(".").join("/");
    }

    return aScript;
}

/*!
 * Defines a PubSub Browser library script's dependencies. Call this
 * before including the library with kn_include().
 *
 * @tparam string aScript The name of the component library.
 * @tparam array aList A list of names of other libraries used by the component library.
 * @treturn void Nothing.
 */
function kn_include_define(aScript, aList)
{
    if (! self.kn_include_deps) kn_include_deps = new Object;

    for (var i = 0; i < aList.length; i++)
    {
        aList[i] = kn_include_canonicalName(aList[i]);
    }
    kn_include_deps[kn_include_canonicalName(aScript)] =
        aList;
}

/*!
 * Imports a PubSub Browser library script
 * This function can be used to import a script to be used with the JavaScript PubSub libraries package. When used to import
 * a JavaScript PubSub component or application, this function also imports all dependency libraries needed as 
 * defined by the kn_include_define() function.<p>
 * NOTE: calls to kn_include() should reside inside a SCRIPT tag outside of any other JavaScript. This is 
 * to ensure that kn_include() does not interfere with app logic.
 * kn_include() can also import other scripts outside the JavaScript PubSub directory. To do this, specify an 
 * absolute or relative URI to the script:<p>
 * <pre>kn_include("kn_chatlet"); // path relative js_pubsub/js directory
 * kn_include("/myscripts/custom_script"); // absolute path to script
 * kn_include("../../custom_script"); // path relative to page
 * kn_include("./custom_script"); // path relative (local) to page</pre><p>
 * NOTE: you do not need to include the ".js" extension when specifying a URI.
 * @tparam string aScript The name of the component library to include.
 * @treturn void Nothing.
 */
function kn_include(aScript)
{
  var s = "";
  var url;

  aScript = kn_include_canonicalName(aScript);

  if (! kn_include_isLoaded(aScript))
  {
      if (self.kn_include_deps)
      {
          var deps = self.kn_include_deps[aScript];
          if (deps)
          {
              for (var i=0;i<deps.length;i++)
              {
                  kn_include(deps[i]);
              }
          }
      }

      url = aScript + ".js";

      // FIXME: Looks like we need an actual URL and path resolver in here...      
      if (url.indexOf("/") != 0 &&
          url.indexOf("./") != 0 &&
          url.indexOf("../") != 0 &&
          ! kn_browser_hasProto(url))
      {
          url = kn_browser_includePath + url;
      }
      document.write('<script src="' + url + '"><' + '/script>');
      kn_include_loaded[kn_include_loaded.length] = aScript;
  }
}

/*!
 * Determines if a script library had been previously loaded.<p>
 * This will return true even if you have incorrectly specified the URL when including the
 * script with kn_include()
 *
 * @tparam string aScript The name of the component library to check.
 * @treturn boolean Returns true if the library has been previously loaded, false if not.
 */
function kn_include_isLoaded(aScript)
{
  aScript = kn_include_canonicalName(aScript);
  for (var i=0;i<self.kn_include_loaded.length;i++)
  {
    if (self.kn_include_loaded[i] == aScript)
    {
      return true;
    }
  }
  return false;
}

// include helper functions

/*!
 * Apply a CSS style to a DOM element ("aNode"). Works in the
 * Netscape6-style DOM and the MSIE-style DOM; elsewhere it has no
 * effect. The name of the stylesheet property in CSS syntax
 * ("aPropertyName") is automatically converted to the MSIE syntax
 * when necessary. The value ("aPropertyValue") should be in CSS
 * syntax.
 *
 * @tparam string aNode A DOM element.
 * @tparam string aPropertyName The name of the stylesheet property in CSS syntax.
 * @tparam string aPropertyValue The property value in CSS syntax.
 * @treturn public:void Nothing.
 */
function kn_browser_setStyle(aNode, aPropertyName, aPropertyValue)
{
    var anMSIEPropertyName = "";
    for (var i = 0; i < aPropertyName.length; i ++)
    {
        if (aPropertyName.charAt(i) == "-" && i < aPropertyName.length)
        {
            anMSIEPropertyName +=
                aPropertyName.charAt(++ i).toUpperCase();
        }
        else
        {
            anMSIEPropertyName +=
                aPropertyName.charAt(i);
        }
    }
    if (aNode.style)
    {
        if (aNode.style[anMSIEPropertyName] != null)
        {
            aNode.style[anMSIEPropertyName] =
                aPropertyValue;
        }
        else
        {
            aNode.style =
                aPropertyName +
                ': ' +
                aPropertyValue;
        }
    }
}

/*!
 * Test a URI (aURI) to determine whether it starts with a valid protocol prefix.
 * @tparam string aURI URI to test.
 * @treturn boolean Returns true iff the URI (aURI) begins with a
 * valid protocol prefix.
 */
function kn_browser_hasProto(aURI)
{
    var c = aURI.indexOf(":");
    if (c < 1) return false;
    for (var i = 0; i < c; i ++)
    {
        var cc = aURI.charAt(i);
        if (! (
            cc >= "0" && cc <= "9"
                     ||
            cc >= "A" && cc <= "Z"
            ||
            cc >= "a" && cc <= "z"
            ))
        {
            return false;
        }
    }
    return true;
}

/*!
 * @class kn_browser_wrapper
 * A kn_browser_wrapper wrapper object forwards its onMessage(anEvent)
 * method to the method name you specify (aMethodName) on the callback
 * object you specify (anObject).<p> Use this to dispatch several
 * different subscriptions to separate methods of the same object.
 *
 * @ctor kn_browser_wrapper object constructor.
 *
 * @tparam object anObject The name of the callback object.
 * @tparam string aMethodName The name of the callback method.
 */
function kn_browser_wrapper(anObject, aMethodName)
{
  this.obj = anObject;
  this.methodName = aMethodName;
}

/*!
 * This callback forwards the event (anEvent) as an argument to the object and method
 * specified in the constructor.
 * @tparam object anEvent The event to be forwarded.
 * @treturn object Returns the forwarding method's return value.
 */
kn_browser_wrapper.prototype.onMessage = function (anEvent)
{
    // FIXME: Duz not werk with wierdo characters in methodName. Use identifiers only.
    return eval("this.obj."+this.methodName+"(anEvent)");
}

/*!
 * Returns a pseudo-random string.
 * @treturn string Returns a pseudo-random string containing digits
 * and underscores.
 */
function kn_browser_uid()
{
  return Math.random().toString().split(".").join("_");
}

kn_browser_plugins = [];

/*!
 * Activates a plugin from the configuration script
 *
 * @tparam string name The name of the plugin library to include.
 * @treturn void Nothing.
 */
function kn_include_plugin(name)
{
  if (document.layers)
  {
    kn_browser_plugins[kn_browser_plugins.length] = name;
    return;
  }
  kn_include("plugins/" + name);
}

kn_browser_auth = [];

/*!
 * Activates an authorization plugin from the configuration script.
 *
 * @tparam string name The name of the authorization library to include.
 * @treturn void Nothing.
 */
function kn_include_auth(name)
{
  kn_browser_auth[kn_browser_auth.length] = name;
}

/*!
 * Imports a JavaScript PubSub Library.
 *
 * @tparam string uri The URI of the PubSub Server.
 * @tparam string name The name of the PubSub Client Library to include.
 * @treturn void Nothing.
 */
function kn_include_connector(uri,name)
{ 
  kn_browser_uri = uri||"/kn";
  kn_browser_connector = name||"kn";
  if (document.layers) return; // Return in NS4.
  
  document.write('<script src="' + kn_browser_uri + '?do_method=whoami"><' + '/script>');
  for (var i=0;i<kn_browser_auth.length;i++)
  {
    kn_include("auth/" + kn_browser_auth[i]);
  }
  kn_include("connectors/" + kn_browser_connector);
}

function kn_browser_warning_0_0_1_8()
{
    alert("WARNING: Application must be upgraded, please\n" +
          "notify application maintainer of the following:\n" +
          "\n" +
          "Application pages must now include kn_config.js before\n" +
          "including kn_browser.js, and kn_config.js must define\n" +
          "the kn_config() function.");
}

if (! self.kn_config)
{
    self.kn_config = function()
        {

            if (! self.kn_browser_silent)
            {
                self.setTimeout("kn_browser_warning_0_0_1_8()", 1);
            }

            kn_include("kn_config");

        };
}

self.kn_config();

// In NS4, we have to write these immediately.
if (document.layers && (self.kn_browser_uri != null))
{
  for (var i=0;i<kn_browser_plugins.length;i++)
  {
    kn_include("plugins/" + kn_browser_plugins[i]);
  }
  document.write('<script src="' + kn_browser_uri + '?do_method=whoami"><' + '/script>');
  for (var i=0;i<kn_browser_auth.length;i++)
  {
    kn_include("auth/" + kn_browser_auth[i]);
  }
  kn_include("connectors/" + kn_browser_connector);
}

// End of kn_browser.js
