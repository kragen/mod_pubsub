// tunnelmonitor.js - JavaScript PubSub Library Tunnel Monitor

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

// $Id: tunnelmonitor.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $

////////////////////////////////////////////////////////////////////////
// Documentation

// The JavaScript Tunnel Monitor allows an application using the
// JavaScript PubSub Library to detect a failed network connection
// or a stopped PubSub Server. The document covers application
// configuration, and the monitor's programming interface.

// 1. Application Configuration

// To use the monitor, include this script before the JavaScript
// PubSub Library.

//    <!-- JavaScript PubSub Library Tunnel Monitor -->
//    <script type="text/javascript" src="tunnelmonitor.js"></script>

//    <!-- The JavaScript PubSub Library -->
//    <script type="text/javascript" src="/kn?do_method=lib"></script>

// 2. Programming Interface

// Create an tunnelMonitor handler object with onConnect(true) and
// onDisconnect(false) handlers and register it using kn_tunnelMonitor:

//     kn_tunnelMonitor(aTunnelMonitorHandler);

// The tunnelMonitor also attempts to register itself as a
// kn_heartbeatCallback() handler, but this only works in the
// microserver owner window; to make this work, include this script
// before the microserver, but after any transport plug-ins. The first
// time kn_heartbeatCallback() is invoked, the tunnelMonitor will also
// hook itself into kn_sendCallback().

////////////////////////////////////////////////////////////////////////
// Implementation

// _kn_tunnelMonitorInit - internal; initialize tunnelMonitor defaults
function _kn_tunnelMonitorInit()
{
    // if initialization is done, don't re-do it
    if (self. //_
        _kn_tunnelMonitorInitDone) return;

    // initialize settings from kn_argv or self properties

// NOTE: set self properties in UTF-8 before including this script, as
// they will be decoded during script initialization; don't use UTF-8
// encoding when changing them after the script has been included

    var tunnelMonitorSettings = new Array(

// kn_tunnelMonitorInterval - sampling interval (in seconds)
        "kn_tunnelMonitorInterval", "10",

// kn_tunnelMonitorTopic - topic to subscribe to
        "kn_tunnelMonitorTopic", "/kn_statistics",

// kn_tunnelMonitorHeader - header name to check for interval overrides
        "kn_tunnelMonitorHeader", "interval",

// kn_tunnelMonitorMaxMissedBeats - maximum number of missed heartbeats
// before we declare the connection closed.
        "kn_tunnelMonitorMaxMissedBeats", "4");

    for (var i = 0; i < tunnelMonitorSettings.length; i += 2)
    {
        var setting =
            tunnelMonitorSettings[i];
        var value =
            tunnelMonitorSettings[i + 1];
        if (self[setting] != null)
            value =
                "" + self[setting];
        if (self.kn_utf8decode)
            setting =
                self.kn_utf8decode(setting);
        if (self.kn_utf8decode)
            value =
                self.kn_utf8decode(value);
        if (self.kn_argv && self.kn_argv[setting])
            value =
                self.kn_argv[setting];
        self[setting] = value;
    }

    self.kn_tunnelMonitorInterval =
        parseInt(self.kn_tunnelMonitorInterval);

    self.kn_tunnelMonitorMaxMissedBeats =
        parseInt(self.kn_tunnelMonitorMaxMissedBeats);

    // initialize some internal settings

    // _kn_tunnelMonitorMissedBeats - internal number of missed heartbeats
    // since the last heartbeat was received
    self. //_
        _kn_tunnelMonitorMissedBeats = 0;

    // _kn_tunnelMonitorHandler - internal reference to tunnelMonitor handler object
    self. //_
        _kn_tunnelMonitorHandler = null;

    // _kn_tunnelMonitorTimer - internal setTimeout handler
    self. //_
        _kn_tunnelMonitorTimer = null;

    // _kn_tunnelMonitorRoute - internal route id
    self. //_
        _kn_tunnelMonitorRoute = null;

    // _kn_tunnelMonitorHappy - internal connection status
    self. //_
        _kn_tunnelMonitorHappy = null;

    // initialization complete
    self. //_
        _kn_tunnelMonitorInitDone = true;
}

// kn_tunnelMonitor(aTunnelMonitorHandler, aStatusHandler) - start monitoring
// the connection to the event router, and call
// aTunnelMonitorHandler.onConnect(true) when the connection starts or
// restarts, and aTunnelMonitorHandler.onDisconnect(false) when the
// connection stops. A status-handler object (aStatusHandler) may be
// supplied for the tunnelMonitor subscription. Calling kn_tunnelMonitor() will
// replace any previously-registered tunnelMonitor handler (without
// invoking it!) and resets the internal connection state to "unknown"
// without invoking the new handler.

// NOTE: setting kn.leaderWindow.kn__tunnelMonitorHeartbeatDate changes the
// behavior of kn_tunnelMonitor(...) in ways documented earlier in this
// file.

function kn_tunnelMonitor(aTunnelMonitorHandler, aStatusHandler)
{
    // first, initialize tunnelMonitor defaults
    self. //_
        _kn_tunnelMonitorInit();
    var anInterval = self.kn_tunnelMonitorInterval;
    self. //_
        _kn_tunnelMonitorMissedBeats = 0;
    self. //_
        _kn_tunnelMonitorHappy = null;
    self. //_
        _kn_tunnelMonitorHandler = aTunnelMonitorHandler;
    if (self. //_
        _kn_tunnelMonitorTimer)
        clearTimeout(self. //_
                     _kn_tunnelMonitorTimer);
    if (self. //_
        _kn_tunnelMonitorRoute)
        self.kn.unsubscribe(self. //_
                            _kn_tunnelMonitorRoute);
    self. //_
        _kn_tunnelMonitorRoute = null;
    if (self.kn &&
        self.kn_isReady &&
        self.kn_isReady(self.kn.leaderWindow) &&
        typeof self.kn.leaderWindow.kn__tunnelMonitorHeartbeatDate == "object")
    {
        anInterval =
            (self.kn.leaderWindow.kn__tunnelMonitorHeartbeatDate.getTime() -
             (new Date()).getTime()) /
            1000;
    }
    else if (self.kn)
    {
        self. //_
            _kn_tunnelMonitorRoute =
            self.kn.subscribe(self.kn_tunnelMonitorTopic,
                              self. //_
                              _kn_tunnelMonitorOnEvent,
                              ({ do_max_n: 1 }),
                              aStatusHandler);
    }
    if (anInterval < 0) anInterval = 0;
    self. //_
        _kn_tunnelMonitorTimer = setTimeout("self." + //_
                                       "_kn_tunnelMonitorOnTimer()",
                                       parseInt(anInterval * 1000));
}

// _kn_tunnelMonitorOnEvent(anEvent) - internal handler for heartbeat
// events
function _kn_tunnelMonitorOnEvent(anEvent)
{
    // first, initialize tunnelMonitor defaults
    self. //_
        _kn_tunnelMonitorInit();
    self.kn_tunnelMonitorReset(anEvent[self.kn_tunnelMonitorHeader]);
}

// kn_tunnelMonitorReset( [ anInterval ] ) - pet the tunnelMonitor, and optionally
// specify the positive interval in seconds for the next timer period
// (anInterval)
function kn_tunnelMonitorReset(anInterval)
{
    // first, initialize tunnelMonitor defaults
    self. //_
        _kn_tunnelMonitorInit();
    self. //_
        _kn_tunnelMonitorMissedBeats = 0;
    if (self. //_
        _kn_tunnelMonitorHappy != true)
    {
        self. //_
            _kn_tunnelMonitorHappy = true;
        self. //_
            _kn_tunnelMonitorHandler.onConnect(self. //_
                                          _kn_tunnelMonitorHappy);
    }
    if (typeof anInterval == "string")
        anInterval = parseFloat(anInterval);
    if (! anInterval || ! anInterval > 0.0)
        anInterval = self.kn_tunnelMonitorInterval;
    if (self. //_
        _kn_tunnelMonitorTimer)
        clearTimeout(self. //_
                     _kn_tunnelMonitorTimer);
    self. //_
        _kn_tunnelMonitorTimer = setTimeout("self." + //_
                                       "_kn_tunnelMonitorOnTimer()",
                                       parseInt(anInterval * 1000));
}

// _kn_tunnelMonitorOnTimer() - internal handler for timer expiration
// events
function _kn_tunnelMonitorOnTimer()
{
    var connectionSeen = false;
    // first, initialize tunnelMonitor defaults
    self. //_
        _kn_tunnelMonitorInit();
    var anInterval = self.kn_tunnelMonitorInterval;
    self. //_
        _kn_tunnelMonitorTimer = null;
    if (self.kn &&
        self.kn_isReady &&
        self.kn_isReady(self.kn.leaderWindow) &&
        typeof self.kn.leaderWindow.kn__tunnelMonitorHeartbeatDate == "object")
    {
        anInterval =
            (self.kn.leaderWindow.kn__tunnelMonitorHeartbeatDate.getTime() -
             (new Date()).getTime()) /
            1000;
        if (self. //_
            _kn_tunnelMonitorRoute)
            self.kn.unsubscribe(self. //_
                                _kn_tunnelMonitorRoute);
        self. //_
            _kn_tunnelMonitorRoute = null;
        if (anInterval < 0)
        {
            anInterval = 0;
            self. //_
                _kn_tunnelMonitorMissedBeats =
            self. //_
                _kn_tunnelMonitorMissedBeats + 1;
        }
        else
        {
            self. //_
                _kn_tunnelMonitorMissedBeats = 0;
        }
        anInterval += self.kn_tunnelMonitorInterval;
        connectionSeen = true;
    }
    else
    {
        self. //_
            _kn_tunnelMonitorMissedBeats =
            self. //_
            _kn_tunnelMonitorMissedBeats + 1;
    }
    self. //_
        _kn_tunnelMonitorTimer = setTimeout("self." + //_
                                       "_kn_tunnelMonitorOnTimer()",
                                       parseInt(self.kn_tunnelMonitorInterval * 1000));
    if (self. //_
        _kn_tunnelMonitorMissedBeats >= self. //_
        kn_tunnelMonitorMaxMissedBeats)
    {
        if (self. //_
            _kn_tunnelMonitorHappy != false)
        {
            self. //_
                _kn_tunnelMonitorHappy = false;
            self. //_
                _kn_tunnelMonitorHandler.onDisconnect(self. //_
                                                 _kn_tunnelMonitorHappy);
        }
    }
    else if (connectionSeen)
    {
        if (self. //_
            _kn_tunnelMonitorHappy != true)
        {
            self. //_
                _kn_tunnelMonitorHappy = true;
            self. //_
                _kn_tunnelMonitorHandler.onConnect(self. //_
                                              _kn_tunnelMonitorHappy);
        }
    }
}

// _kn_tunnelMonitorHeartbeat( [ anInterval ] ) - update
// kn.leaderWindow.kn__tunnelMonitorHeartbeatDate to be anInterval seconds in the
// future (or kn_tunnelMonitorInterval seconds in the future, if anInterval
// is not specified)
function _kn_tunnelMonitorHeartbeat(anInterval)
{
    // first, initialize tunnelMonitor defaults
    self. //_
        _kn_tunnelMonitorInit();
    // second, calculate anInterval (if not already done)
    if (typeof anInterval == "string")
        anInterval = parseFloat(anInterval);
    if (! anInterval || ! anInterval > 0.0)
        anInterval = self.kn_tunnelMonitorInterval;
    // third, update self.kn__tunnelMonitorHeartbeatDate
    self.kn__tunnelMonitorHeartbeatDate == 0;
    var heartbeatDate =
        new Date(
            (new Date()).getTime() +
            parseInt(anInterval * 1000)
            );
    if (! self.kn__tunnelMonitorHeartbeatDate ||
        ((self.kn__tunnelMonitorHeartbeatDate.getTime()) <
         heartbeatDate.getTime()))
    {
        self.kn__tunnelMonitorHeartbeatDate =
            heartbeatDate;
    }
}

// _kn_tunnelMonitorSendCallback(theEvent, theWindow) - piggy-backed
// callback for event delivery, as defined in the Event Notification
// Service document; refer to kn_sendCallback for documentation
function _kn_tunnelMonitorSendCallback(theEvent, theWindow)
{
    // first, initialize tunnelMonitor defaults
    self. //_
        _kn_tunnelMonitorInit();
    // second, simulate a heartbeat
    self. //_
        _kn_tunnelMonitorHeartbeat();
    // finally, invoke the original kn_sendCallback
    return self. //_
        _kn_tunnelMonitorSendCallbackOrig(theEvent, theWindow);
}

// _kn_tunnelMonitorHeartbeatCallback( anInterval [ , theWindow ] ) -
// kn_heartbeatCallback() handler (updates heartbeat); the first
// argument (theWindow) is ignored and the second one is optional (see
// _kn_tunnelMonitorHeartbeat()); this also runs any pre-existing
// piggy-backed kn_heartbeatCallback() with the same two arguments
function _kn_tunnelMonitorHeartbeatCallback(anInterval, theWindow)
{
    // first, initialize tunnelMonitor defaults
    self. //_
        _kn_tunnelMonitorInit();
    // second, update the heartbeat date
    self. //_
        _kn_tunnelMonitorHeartbeat(anInterval);
    // third, hook in our piggy-backed kn_sendCallback() handler if
    // this is the first time we've run
    if (! self. //_
        _kn_tunnelMonitorSendCallbackOrig)
    {
        self. //_
            _kn_tunnelMonitorSendCallbackOrig = self.kn_sendCallback;
        self.kn_sendCallback = _kn_tunnelMonitorSendCallback;
    }
    // finally, run any piggy-backed kn_heartbeatCallback() handler
    if (self. //_
        _kn_tunnelMonitorHeartbeatCallbackOrig)
    {
        return self. //_
            _kn_tunnelMonitorHeartbeatCallbackOrig(anInterval, theWindow);
    }
}

// _kn_tunnelMonitorRegister() - piggy-back onto kn_heartbeatCallback();
// only works in the owner window
function _kn_tunnelMonitorRegister()
{
    // first, save any existing kn_heartbeatCallback
    self. //_
        _kn_tunnelMonitorHeartbeatCallbackOrig = self.kn_heartbeatCallback;
    // second, register our kn_heartbeatCallback
    self.kn_heartbeatCallback = self. //_
        _kn_tunnelMonitorHeartbeatCallback;
}

_kn_tunnelMonitorRegister();

// $Log: tunnelmonitor.js,v $
// Revision 1.2  2004/04/19 05:39:12  bsittler
// Propagate updated license text and associated copyright date to all
// affected files.
//
// Also replaced a few KNOWNOW_LICENSE_BEGINs with the correct
// KNOWNOW_LICENSE_START.
//
// Revision 1.1  2003/03/22 08:31:46  ifindkarma
// Added tunnelmonitor library and sample usage.
//
