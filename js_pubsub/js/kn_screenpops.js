/*! @file kn_screenpops.js PubSub ScreenPops Component
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_screenpops");
 * &lt;/script&gt;</b>
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

// $Id: kn_screenpops.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $


////////////////////////////////////////////////////////////////////////
// Documentation

// This component is used to display text in a pop-up browser window
// in response to received events.

// 1. Application Configuration

// To use it, first include this script before the JavaScript
// PubSub Library:

//    <!-- PubSub ScreenPops Component -->
//    <script type="text/javascript" src="../../js/kn_screenpops.js"></script>

//    <!-- JavaScript PubSub Library -->
//    <script type="text/javascript" src="/kn?do_method=lib"></script>

// Then instantiate the PubSub ScreenPops Component:

//    <body onload="kn_screenpops()">

// By default, the PubSub ScreenPops Component monitors the
// /what/alerts topic, but this setting can be overridden for
// particular instances:

//    <body onload="kn_screenpops('/bigcorp.com/what/apps/screenpops');
//     kn_screenpops('/who/anonymous/apps/yew/buddies/status')">

// 2. Component Programming Interface

// The PubSub ScreenPops Component has configurable properties which
// may be set using either command-line parameters or "self"
// properties [the "self" properties must be UTF-8 encoded, and set
// before loading the JavaScript PubSub Library]:

//    kn_topic

//      or

//    kn_screenpopsTopic

// Default topic to monitor for alerts (defaults to kn_topic
// value if kn_screenpopsTopic is not present), eg:

//    /bigcorp.com/what/apps/screenpops

////////////////////////////////////////////////////////////////////////
// Implementation

kn_include("kn_lwws.Service");

// _kn_screenpops_init() - internal initialization function for the
// kn_screenpops component
function _kn_screenpops_init()
{
  if (self. //_
    _kn_screenpops_initDone)
  {
    return;
  }
  self. //_
    _kn_screenpops_initDone = true;
  self. //_
    kn_screenpops__uid = kn_browser_uid();
  self. //_
    _kn_screenpops_params =
    new Object();
  kn_createContext("kn_screenpops");
  var params = kn__object(
    "Topic",       kn_argv.kn_topic
    );
  var prefix = "kn_screenpops";
  for (var param in params)
  {
    var knparam = prefix + param;
    var value = null;
    if (params[param] != null)
    {
      value = params[param];
    }
    if (self[knparam] != null)
    {
      value = kn_utf8decode(self[knparam]);
    }
    if (kn_argv[knparam] != null)
    {
      value = kn_argv[knparam];
    }
    if (value != null)
    {
      self. //_
        _kn_screenpops_params[param] = value;
    }
  }
}

///////// kn_screenpops_obj

// kn_screenpops_obj class - PubSub ScreenPops Component

//// Constructors:

// new kn_screenpops_obj( [ aTopic ] ) - constructor for a
// PubSub ScreenPops Component instance; optional topic name (aTopic)
// determines which topic will be monitored for alerts once the
// instance is started with aScreenpops.start().

/*! @class kn_screenpops_obj
 * This object provides the PubSub ScreenPops interface.<p>This constructor
 * should never be called directly from application code. Use
 * kn_screenpops() instead to create an instance of the ScreenPops
 * component.
 * @ctor Screenpops object constructor.
 * @see kn_screenpops()
 * @tparam string aTopic The topic will be monitored for alerts.
 */
function kn_screenpops_obj(aTopic)
{
  self. //_
    _kn_screenpops_init();

  // Alerts topic.
  aTopic =
    // 1. Topic explicitly supplied as constructor argument.
    aTopic ? aTopic :
    // 2. Failing that, check for a URL parameter or "self" property.
    self. //_
    _kn_screenpops_params.Topic ?
    self. //_
    _kn_screenpops_params.Topic :
    // 3. By default, monitor the general alerts topic.
    "/what/alerts";

  // Private instance variables.

  // Remember the topic.
  this._kn_screenpops_topic = aTopic;

  // Find our files.
  this._kn_screenpops_htmlPath =
    kn_defaults_get("kn_screenpops","HtmlPath",kn_browser_includePath + "html/");

  // Window handle.
  this._alertWin = null;

  // Superclass constructor.
  this.Super = kn_lwws.Service;
  this.Super(self.kn, aTopic);
}

// Implements kn_lwws.Service.

kn_screenpops_obj.prototype = new kn_lwws_Service();

//// Instance methods and default instance method implementations:

// aScreenPops.onMessage(anEvent) - handle an event (anEvent) from the
// alerts topic.

// aScreenPops.onMessage()
kn_screenpops_obj.prototype.onMessage = function(anEvent)
{
  if (anEvent.kn_payload &&
    anEvent.kn_deleted != "true")
  {
    this.alert(
      anEvent.kn_payload);
  }
};


// aScreenPops.alert(aString) - displays a text message (aString) in a
// pop-up window.

// aScreenPops.alert()
kn_screenpops_obj.prototype.alert = function(aString)
{
  if (this._alertWin &&
    kn_isReady(this._alertWin) &&
    this._alertWin.kn_argv &&
    this._alertWin.kn_argv.kn_screenpops_pop == this._kn_screenpops_pop &&
    ! this._alertWin.kn_screenpops_closing)
  {
    // Rely on the existing pop-up window.
  }
  else
  {
    this._kn_screenpops_pop = kn_browser_uid();
    var anURL = (
      location.protocol + '//' + location.host + 
      kn_resolvePath(
        self.location.pathname,
        this._kn_screenpops_htmlPath + "kn_screenpops_popup.html") +
      '?kn_topic=' + kn_escape(this._kn_screenpops_topic) +
      ';kn_screenpops_pop=' + this._kn_screenpops_pop +
      ';kn_screenpops__uid=' + self.kn_screenpops__uid +
      (kn__debug() ? ';kn_debug' : ''));
    
    this._alertWin = self.open(anURL,
                   '_blank',
                   'width=250,height=200');
  }
}
;

//// Factories:

/*!
  * Create a PubSub ScreenPops Component and start processing requests.
  *
  * @tparam string aTopic topic on which this component instance receives notifications. (optional)
  * @tparam statusHandlers aStatusHandler subscription status handler (optional)
  * @treturn kn_screenpops_obj an object reference to the newly-created component.
  */
function kn_screenpops(aTopic, aStatusHandler)
{
  var aScreenpops = new kn_screenpops_obj(aTopic);
  var anOptions = new Object;
  anOptions.do_max_age = "infinity";
  anOptions.kn_deletions = "true";
  aScreenpops.start(anOptions, aStatusHandler);
  return aScreenpops;
}

// End of kn_screenpops.js
