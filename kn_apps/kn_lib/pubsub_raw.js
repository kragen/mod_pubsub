/*! @file pubsub_raw.js PubSub JavaScript library <p>Select the appropriate PubSub JavaScript Library build, one of: <dl> <dt><tt>kn_raw</tt></dt> <dd>unlocalized, debug, uncompressed</dd> <dt><tt>knl_raw</tt></dt> <dd>localized, debug, uncompressed</dd> <dt><tt>knp_raw</tt></dt> <dd>unlocalized, non-debug, uncompressed</dd> <dt><tt>knl</tt></dt> <dd>localized, debug, compressed [for development]</dd> <dt><tt>knp</tt></dt> <dd>unlocalized, non-debug, compressed [for production]</dd> </dl> <pre> <b>self.kn_config_connector = "pubsubl"; // define before kn_config() runs</b> </pre>
 */

// FIXME: Refactor this library so it has an ECMA runtime
// and a separate browser piece so it can be used with
// things like Konfabulator.

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

// $Id: pubsub_raw.js,v 1.6 2003/04/28 23:17:58 ifindkarma Exp $

////////////////////////////////////////////////////////////////////////
// Notes on notation:
// 1. local variable and parameter names with four or more
//    characters prefixed by _l_ are compressed; at least one
//    reference to each such variable should appear on a line with no
//    other compressed variables
// 2. microserver-internal variables with names like _kn_xxx and kn._xxx
//    are compressed; at least one reference to each such variable
//    should appear on a line with no other compressed variables
// 3. "internal-but-visible" variables with names like kn__xxx are not
//    compressed
// 4. a //_ comment marks a point where a newline has been inserted
//    to guarantee that a particular compressed variable appears
//    alone on at least one line
// 5. a //# comment at the end of a line marks a line which will
//    be removed from the "production" version of the microserver
// 6. _kn_$("x") wraps a literal string "x" in the "kn" localization context;
//    at most one wrapper can appear on a particular line
// 7. _kn_$_("x", ...) wraps a literal formatting template "x" in the "kn"
//    localization context; at most one wrapper can appear on a
//    particular line

// Also, a few properties named kn.xxx_ are "internal-but-visible",
// generally because thay may be used in inter-window communication
// among different microserver releases [i.e. debug vs. production]

// /**/ is used for indentation before string constants. This
// particular one-line form of the C comment [with no * characters
// inside it] is removed by the compressor.

////////////////////////////////////////////////////////////////////
// BASIC JAVASCRIPT HELPERS
//

// used by _kn_object()
function _kn_object2(args)
{
    var _l_result = new Object();
    for (var i = 0; i < args.length; i += 2)
    {
        _l_result[args[i]] = args[i + 1];
    }
    return _l_result;
}

// internal version of kn__object()
function _kn_object() // args ...
{
    return _kn_object2(arguments);
}

////////////////////////////////////////////////////////////////////
// CHARACTER-CODE LOOKUP TABLE
//
// charcode table for ancient, broken JavaScript implementations
_kn_codes8 = new Array();
{
    var i;
    for (i = 1; i < 256; i ++)
    {
        var c = _kn_stringFromCharCode8(i);
        _kn_codes8[c] = i;
    }
}

////////////////////////////////////////////////////////////////////
// POOL OF MEMORY
//
// The pool is a place to put things we want to recall at some later
// time, when all other references to the original item have been
// lost. When the pool memorizes something, it returns an index
// number which can later be used to recall that item. When an object
// is no longer needed, the pool is told to forget it. This allows the
// pool to recycle indeces and avoid unlimited memory usage.
//
// Each microserver has its own pool, and typically uses the pool when
// constructing new Function objects or setTimeout() handlers, where
// code must be written inside strings. Since the constructed code is
// evaluated in a different context from that in which it is created,
// it can not refer to objects from the creating context directly. The
// solution is to memorize the object, and interpolate the resulting
// index into the string, along with a request to recall the object
// (and possibly forget it.)

// constructor for the kn._pool object (not very OO, really)
function _kn_Pool()
{
    this._l_pool = //_
        new Array();
    this._l_freelist = //_
        new Array();
    this._l_freecount = 0;
}

// implementation of kn._memorize()
function _kn_memorize(obj)
{
    var kn = this;
    var ptr = kn._pool._l_pool.length;
    if (kn._pool._l_freecount)
    {
        kn._pool._l_freecount = kn._pool._l_freecount - 1;
        ptr = kn._pool._l_freelist[kn._pool._l_freecount];
    }
    var p = _kn_object('obj', obj, 'ptr', ptr);
    if (_kn_debug() && //#
        kn._pool._l_pool[ptr]) //#
    { //#
        alert(_kn_$_("[KNJS:Pool] The pool has been corrupted: #%{0} \"%{1}\" was not completely forgotten.", //#
                     ptr, //#
                     obj)); //#
    } //#
    kn._pool._l_pool[ptr] = p;
    return ptr;
}

// implementation of kn._recall()
function _kn_recall(ptr, label)
{
    var kn = this;
    var p = kn._pool._l_pool[ptr];
    if (_kn_debug()) //#
    { //#
        if (! p) //#
            alert(_kn_$_("[KNJS:Pool] %{2}: #%{0} \"%{1}\" was recalled while unused", //#
                         ptr, p, label)); //#
        else if (p.ptr != ptr) //#
            alert(_kn_$_("[KNJS:Pool] %{3}: The pool has been corrupted. #%{0} \"%{1}\" is indexed as #%{2}", //#
                         ptr, p.obj, p.ptr, label)); //#
    } //#
    return p.obj;
}

// implementation of kn._forget()
function _kn_forget(ptr, label)
{
    var kn = this;
    var obj = kn._recall(ptr, label);
    kn._pool._l_pool[ptr] = void 0;
    kn._pool._l_freelist[kn._pool._l_freecount ++ ] = ptr;
    return obj;
}

////////////////////////////////////////////////////////////////////
// SCHEDULER
//

function _l_setInterval_set( //_
    _l_handle, //_
    _l_handler)
{
    kn.ownerWindow.setTimeout(
        "kn._recall(" + _l_handle + ",'_l_setInterval_set')._l_handlerFunction(" + _l_handle + ")",
        _l_handler._l_interval
        );
}

function _l_setInterval_handlerFunction( //_
    _l_handle)
{
    var _l_handler = //_
        kn._recall(_l_handle, '_l_setInterval_handlerFunction');
    _l_handler._l_set(_l_handle, _l_handler);
    return _l_handler._l_runCode();
}

// implementation of kn._setInterval()
function _kn_setInterval( //_
    _l_code, //_
    _l_interval)
{
    var kn = this;
    if (kn.ownerWindow.setInterval)
    {
        return kn.ownerWindow.setInterval(_l_code, _l_interval);
    }
    var _l_handler = //_
        _kn_object(
            '_l_runCode', //_
            new Function(_l_code),
            '_l_interval', //_
            _l_interval,
            '_l_set', //_
            _l_setInterval_set,
            '_l_handlerFunction', //_
            _l_setInterval_handlerFunction
            );
    var _l_handle = //_
        kn._memorize(_l_handler);
    _l_handler._l_set(_l_handle, _l_handler);
    return _l_handle;
}

function _l_clearInterval_handlerFunction( //_
    _l_handle)
{
    kn._forget(_l_handle, "_l_clearInterval_handlerFunction");
}

// implementation of kn._clearInterval()
function _kn_clearInterval( //_
    _l_handle)
{
    var kn = this;
    if (kn.ownerWindow.clearInterval)
    {
        return kn.ownerWindow.clearInterval(_l_handle);
    }
    kn._recall(_l_handle, '_kn_clearInterval')._l_handlerFunction =
        _l_clearInterval_handlerFunction;
    return void 0;
}

////////////////////////////////////////////////////////////////////
// LOCALIZED STRING TABLE
//

// indexed by kn_lang, context and string; the fallback kn_lang and
// context are ''
if (! self.kn_strings) self.kn_strings = _kn_object();

// a sample localization for kn_lang "en-us"
_kn_localizeDefault( //#
    'en-us', // language //#
    'PubSub JavaScript Library %{RCSID}', // source text //#
    '%{displayname}\'s Localized PubSub JavaScript Library %{RCSID}' // replacement //#
    ); //#

////////////////////////////////////////////////////////////////////
// CONSTANT HOLDER
//
// Most of these constants are related to Unicode support.

KN = _kn_object(

    // We map unrecognized UTF sequences to this value.
    'ucsNoChar', 0xFFFD,

    // This is the maximum permitted UCS character value
    'ucsMaxChar', 0x7FFFFFFF,

    // This is the maximum permitted UTF-16/UCS-2 wyde value
    'ucs2max', 0xFFFF,

    // This is the maximum permitted UTF-16 character value
    'utf16max', 0x10FFFF,

    // we use the first characters of the UTF-16 surrogate ranges
    'utf16firstLowHalf', 0xD800,
    'utf16firstHighHalf', 0xDC00,

    // all UTF-16 surrogate pairs are offset by this value after decoding
    'utf16offset', 0x10000,

    // mask and shift applied to UTF-16 surrogate halves to extract useful values
    'utf16mask', 0x3FF,
    'utf16shift', 10,

    // mask and shift for UTF-8 continuation bytes
    'utf8mask', 0x3F,
    'utf8shift', 6,

    // Kragen thinks a global variable is a nice solution to this problem;
    // Danny would prefer goto
    'hexDigits', '0123456789ABCDEFabcdef'
    
    );

////////////////////////////////////////////////////////////////////
// CONFIGURATION AND INITIALIZATION
//

// unescape parameters from do_method=whoami
if (self.kn_tunnelURI != null)
    kn_tunnelURI = _kn_utf8decode(kn_tunnelURI);
if (self.kn_tunnelID != null)
    kn_tunnelID = _kn_utf8decode(kn_tunnelID);
if (self.kn_tunnelMaxAge != null)
    kn_tunnelMaxAge = _kn_utf8decode(kn_tunnelMaxAge);
if (self.kn_lastTag_ != null)
    kn_lastTag_ = _kn_utf8decode(kn_lastTag_);
if (self.kn_hashCache != null)
    kn_hashCache = _kn_utf8decode(kn_hashCache);
if (self.kn_userid != null)
    kn_userid = _kn_utf8decode(kn_userid);
if (self.kn_displayname != null)
    kn_displayname = _kn_utf8decode(kn_displayname);
if (self.kn_server != null)
    kn_server = _kn_utf8decode(kn_server);
if (self.kn_blank != null)
    kn_blank = _kn_utf8decode(kn_blank);
if (! self.kn_lang)
    kn_lang = '';
if (! self.kn_response_flush)
    kn_response_flush = null;
// deprecated, but we'll still have to allow it for the forseeable future
if (self.kn_defaultHacks != null)
    kn_defaultHacks = _kn_utf8decode(kn_defaultHacks);
if (self.kn_defaultOptions != null)
    kn_defaultOptions = _kn_utf8decode(kn_defaultOptions);
if (self.kn_appFrameAttributes != null)
    _kn_appFrameAttributes = _kn_utf8decode(kn_appFrameAttributes);

if (self.kn_defaultHacks == null)
    kn_defaultHacks = null;

if (self.kn_defaultOptions == null)
    kn_defaultOptions = null;

// kn_maxHits is the number of events before a restart; tune it to
// control memory consumption in older browsers
if (self.kn_maxHits == null)
    kn_maxHits = 3000;

// the kn_appFrameAttributes string is inserted into the main
// application's <frame> tag; set it to 'noresize="noresize"
// scrolling="no"' or something similar if you are concerned about
// screen real-estate and can not afford to lose a few pixels for
// scrollbars.
if (self._kn_appFrameAttributes == null)
    _kn_appFrameAttributes = '';

// kn_retryInterval is the number of milliseconds the microserver will
// wait before retrying an unacknowledged PubSub Server request; each time
// the retry interval is reached, it is doubled for the next try, with
// kn_maxRetryInterval as an upper bound (set these before loading the
// microserver)
if (self.kn_retryInterval == null)
    kn_retryInterval = 4 * 1000;
if (self.kn_maxRetryInterval == null)
    kn_maxRetryInterval = 30 * 1000;

// kn_maxBatchSize is the maximum number of items the microserver will
// place in any single automatic batch request (set this before
// loading the microserver -- see kn.maxBatchSize for adjustment at
// runtime)
if (self.kn_maxBatchSize == null)
    kn_maxBatchSize = 10;

// internal global variables
_kn_tryingToInitTunnel=false;
_kn_aboutToRunTunnel=false;
_kn_heartbeat = 100; // milliseconds
kn_heartbeat_ = _kn_heartbeat; // exposed
// list of event sources indexed by window name
_kn_paidInformants = _kn_object();

// the global argument object
kn_argv = _kn_parseArgs();

// the global microserver object

function _kn_initMicroserver()
{
    if ((! self.kn__standAlone) &&
        parent && kn_isReady(parent) &&
        ((typeof parent.kn) != "unknown") && parent.kn)
    {
        kn = parent.kn;
        kn_server = parent.kn_server;
        kn_blank = parent.kn_blank;
    } else {
    

        kn = _kn_object(

            // CVS uses RCS for versioning
            'RCSID', "$Id: pubsub_raw.js,v 1.6 2003/04/28 23:17:58 ifindkarma Exp $", //#

            'ownerWindow', window,
            'leaderWindow', window,

            'tunnelURI', null,
            'tunnelID', null,
            'tunnelMaxAge', null,
            'lastTag_', null,

            'getHashCache', _kn_getHashCache,
            'setHashCache', _kn_setHashCache,

            'lastError', null,
            'tunnelRunning_', false,

            'tunnelFrame_', null,
            'dispatch_', _kn_object(),  // window-external 'router table' to other windows
            'documents', _kn_object(),

            // core pub/sub api
            'subscribe', _kn_subscribe,
            'unsubscribe', _kn_unsubscribe,
            'publish', _kn_publish,
            'publishForm', _kn_publishForm,

            'setHandler', _kn_setHandler,
            'clearHandler', _kn_clearHandler,

            // tunnel control
            'restartTunnel', kn__restartTunnel,

            'iw', kn_inspectInWindow, // for debugging
            'userid', "guest", // the default
            'displayname', _kn_$("Guest User") // the default

            );
            
          // call the handler to execute developer-defined code that 
          // modifies auth properties before they are appended to kn
          
          if (self.kn_customInit)
          {
            self.kn_customInit();
          }

        // implementation of kn_sendCallback
        kn._sendCallback = //_
            _kn_sendCallback;

        // implementation of kn_tunnelLoadCallback
        kn._tunnelLoadCallback = //_
            _kn_tunnelLoadCallback;

        // kn._doMethod(do_method, options, handler)
        // The kn._doMethod method is used to dispatch a request with some
        // arbitrary do_method.
        // - do_method is a method name to invoke on the router
        // - options is an optional object containing additional parameters;
        //   - do_method may be overridden by options.do_method
        //   - options.kn_status_from is ignored
        // - handler is an optional handler object with three optional methods:
        //   - onError(event), which will be called if there's an error
        //     creating the route and passed a status event describing the
        //     error.
        //   - onSuccess(event), which will be called if the route is created
        //     successfully and passed a status event describing the error.
        //   - onStatus(event), which supersedes both of the above if it is
        //     supplied.
        // No value is returned, and the generated request will have
        // an auto-generated kn_status_from header for status event
        // tracking purposes.
        kn._doMethod = _kn_doMethod;

        // kn.X(options, handler)
        // All currently known status-providing do_methods are exposed
        // using helper methods of kn. The helper methods are named
        // kn.X, where X is the uppercased version of the do_method in
        // question. These are equivalent to kn._doMethod("x",
        // options, handler), where x is the proper (lower-case)
        // version of the do_method in question.
        {
            // currently known do_methods which provide status events
            var p =
                ( "route add_route delete_route update_route add_journal" +
                  " notify add_topic clear_topic set_topic_property" +
                  " delete_topic" +
                  " add_notify delete_notify update_notify batch save_config" ) .
                split(" ");
            for (var i = 0; i < p.length; i ++)
            {
                kn[p[i].toUpperCase()] =
                    new Function('options', 'handler',
                                 'kn._doMethod(_kn_unescape("' +
                                 _kn_escape(p[i]) +
                                 '"), options, handler);'
                        );
            }
        }

        // in-memory queue support
        kn.sendQueue = _kn_sendQueue;

        // POOL OF MEMORY

        // the pool must be shared by different frames using the same
        // microserver instance
        kn._pool = new _kn_Pool();
        // register an object with the pool, and return its index
        kn._memorize = _kn_memorize;
        // dereference an index
        kn._recall = _kn_recall;
        // recycle an index, and return the object it referred to
        kn._forget = _kn_forget;

        // SCHEDULER

        // replacement for setInterval()
        kn._setInterval = _kn_setInterval;
        // replacement for clearInterval()
        kn._clearInterval = _kn_clearInterval;

        // STRING CONVENIENCE FUNCTIONS

        kn._ownerWindow = kn.ownerWindow;

        kn._seqNum = 0; // request sequence number for building kn_status_from
        kn.TFN_ = null; // short for kn.tunnelFrame_.name
        kn._hits = 0; // number of events on the current tunnel

        kn._isLeaderP = false;
        kn.isLoadedP_ = false;
        kn._isBrokenP = false;
        kn._isRestartingP = false;
        kn._leaderScannerTimer = null;

        kn._unreachable = //_
            _kn_object(); // list of unreachable windows

        kn._appFrame = null;
        kn._postFrame = null;

        kn._workQ = //_
            new Array(); // deferred work list
        kn._workQcursor = 0;
        kn._worker = null;
        kn._postFrameBusy = false;
        kn._processWorkQ = //_
            _kn_processWorkQ;

        kn._followers = //_
            _kn_object(); // all other known windows
        kn._dispatchFunction = //_
            _kn_object();  // window-local 'router table' to functions

        // window-local data pointers for functions; they are
        // automatically kn._forget()ed when no longer needed; the
        // pointed-to objects are passed as second arguments to handlers
        kn._dispatchPtr = //_
            _kn_object();

        kn._argv = kn_argv; // this is the kn_argv of the owner window
        // do not use directly; use _kn_debug() instead
        kn._debug = //_
            kn._argv.kn_debug;
        // do not use directly; use _kn_options() instead
        kn._options = //_
            kn._argv.kn_options || //_
            kn._argv.kn_opts || //_
            kn._argv.kn_hacks;
        kn_response_flush = kn._argv.kn_response_flush ?
            kn._argv.kn_response_flush :
            kn_response_flush;
        kn_lang = kn._argv.kn_lang ?
            kn._argv.kn_lang :
            kn_lang;

        kn._onPostSuccess = //_
            _kn_onPostSuccess;
        kn._onPostError = //_
            _kn_onPostError;
        kn._onTunnelStatus = //_
            _kn_onTunnelStatus;

        self.iw = kn.iw;

        // these two URIs are used outside of kn.js
        if (self.kn_server == null)
            kn_server = "/kn";
        if (kn._argv.kn_server) kn_server = kn._argv.kn_server; // the url says
        if (self.kn_blank == null)
            kn_blank = kn_server + "?do_method=blank";
        if (kn._argv.kn_blank) kn_blank = kn._argv.kn_blank; // the url says

        // make them absolute
        if (! _kn_hasProto(kn_server))
        {
            kn_server = location.protocol + '//' + location.host +
                kn_resolvePath(location.pathname, kn_server);
        }
        if (! _kn_hasProto(kn_blank))
        {
            kn_blank = location.protocol + '//' + location.host +
                kn_resolvePath(location.pathname, kn_blank);
        }

        if (self.kn_userid) kn.userid =  self.kn_userid; // the server says
        if (kn._argv.kn_userid) kn.userid = kn._argv.kn_userid; // the url says
        if (self.kn_displayname) kn.displayname =  self.kn_displayname; // the server says
        if (kn._argv.kn_displayname) kn.displayname = kn._argv.kn_displayname; // the url says

        var hashCache = "";
        if (self.kn_hashCache) hashCache = self.kn_hashCache; // the server says
        if (kn._argv.kn_hashCache) hashCache = kn._argv.kn_hashCache; // the url says
        kn.setHashCache(hashCache);

        kn.tunnelURI =
            "/who/" +
            _kn_escape(kn.userid) +
            "/s/"+Math.random().toString().substring(2, 10) +
            "/kn_journal";
        if (self.kn_tunnelURI) kn.tunnelURI =  self.kn_tunnelURI; // the server says
        if (kn._argv.kn_tunnelURI) kn.tunnelURI = kn._argv.kn_tunnelURI; // the url says
        kn.tunnelURI = _kn_absoluteTopicURI(kn.tunnelURI);

        kn.tunnelID = Math.random().toString().substring(2, 10);
        if (self.kn_tunnelID) kn.tunnelID = self.kn_tunnelID; // the server says
        if (kn._argv.kn_tunnelID) kn.tunnelID = kn._argv.kn_tunnelID; // the url says

        if (self.kn_tunnelMaxAge) kn.tunnelMaxAge = self.kn_tunnelMaxAge; // the server says
        if (kn._argv.kn_tunnelMaxAge) kn.tunnelMaxAge = kn._argv.kn_tunnelMaxAge; // the url says

        if (self.kn_lastTag_) kn.lastTag_ = self.kn_lastTag_; // the server says
        if (kn._argv.kn_lastTag_) kn.lastTag_ = kn._argv.kn_lastTag_; // the url says

        // automatic batching support
        kn.maxBatchSize = kn_maxBatchSize;
        if (kn._argv.kn_maxBatchSize) kn.maxBatchSize = parseInt(kn._argv.kn_maxBatchSize);

        // automatic retry support
        if (kn._argv.kn_retryInterval)
            kn_retryInterval = parseInt(kn._argv.kn_retryInterval);
        if (kn._argv.kn_maxRetryInterval)
            kn_maxRetryInterval = parseInt(kn._argv.kn_maxRetryInterval);

        if (self.kn__wrapApp)
        {
            // allow a custom override for appwrapping
            kn__wrapApp();
        }
        else
        {
            // only the master arranges for its own tunnel & post frames
            _kn_wrapApp();
        }
    }
    return kn;
}

kn = _kn_initMicroserver(true);

// We handle our own JavaScript error logging with kn.lastError
onerror = _kn_lastErrorHandler;

////////////////////////////////////////////////////////////////////
// PUBLIC MICROSERVER INTERFACE
//


// implementation of kn.getHashCache; return serialization of
// hashCache
function _kn_getHashCache()
{
    var keys = new Array();
    for (var key in this._l_hashCache1)
    {
        keys[keys.length] = key;
    }
    for (var key in this._l_hashCache2)
    {
        keys[keys.length] = key;
    }
    return keys.join(" ");
}

// implementation of kn.setHashCache; deserialize hashCache from str
function _kn_setHashCache(str)
{
    this._l_hashCache1 = //_
        _kn_object();
    this._l_hashCache2 = //_
        _kn_object();
    this._l_hashCacheDate = //_
        new Date();
    var keys = str.split(" ");
    for (var index = 0; index < keys.length; index += 3)
    {
        this._l_hashCache1[
            keys[index] +
            " " +
            keys[index + 1] +
            " " +
            keys[index + 2]
            ] =
            true;
    }
}

// kn.subscribe takes four arguments:
// - kn_from, a URL
// - kn_to, a URL - typically a javascript: URL, closure, or Object with
//   onMessage
// - kn_options, an optional hash of additional parameters for the server
// - handler, an optional handler object with three optional methods:
//   - onError(event), which will be called if there's an error
//     creating the route and passed a status event describing the
//     error.
//   - onSuccess(event), which will be called if the route is created
//     successfully and passed a status event describing the success.
//   - onStatus(event), which supersedes both of the above if it is
//     supplied.
//
// kn_options is an Object containing optional keyword-value pairs to
// be passed to the server. do_max_age is one currently defined
// keyword. Other keywords are defined; see the server source code.

// implementation of kn.subscribe
function _kn_subscribe(kn_from, kn_to, kn_options, handler)
{
    if (! kn_options)
        kn_options = _kn_object();
    var ctx = _kn_object();
    ctx.kn_from = kn_from;
    ctx.kn_to = kn_to;
    ctx.kn_options = kn_options;
    ctx.handler = handler;
    ctx.routeID =
        kn_options.kn_id;
    if (! ctx.routeID)
        ctx.routeID = Math.random().toString().substring(2, 10);
    ctx.kn_from = _kn_absoluteTopicURI(ctx.kn_from);
    ctx.route_location =
        ctx.kn_options.kn_uri;
    if (! ctx.route_location)
        ctx.route_location =
            ctx.kn_from +
            "/kn_routes/" +
            kn_escape(ctx.routeID);

    ctx.e = _kn_object();
    var ptr = kn._memorize(ctx);
    kn._workQ[kn._workQ.length] = new Function(
        'var ctx = kn._recall(' + ptr + ', "_kn_subscribe(1)");' +
        'ctx.e.kn_status_from = _kn_setStatusHandler(ctx.handler);' +
        'if (_kn_realSubscribe(ctx.kn_from, ctx.kn_to, ctx.kn_options,' +
        /*                  */'ctx.routeID, ctx.route_location,' +
        /*                  */'ctx.e))' +
        '{' +
        /**/'kn._forget(' + ptr + ',  "_kn_subscribe(2)");' +
        /**/'return true;' +
        '}' +
        '_kn_clearHandler(ctx.e.kn_status_from);' +
        'return false;'
        );
    kn._processWorkQ(); //goose the scanner to try immediately
    return ctx.route_location;
}

// XXX: probably we should pass the kn_id in e.
// kn_id is the event ID of the route; kn_rid is the entire route location URI
function _kn_realSubscribe(kn_from, kn_to, kn_options, kn_id, kn_rid, e)
{
    // JavaScript String Case
    if ((typeof(kn_to) == "string") && _kn_matchPrefix("javascript:",kn_to))
    {
        var cmd = kn_to.substring(kn_to.indexOf(":")+1);
        kn_to = new Function(cmd);
    }

    // context object for handler, if any
    var obj = null;

    // Object Callback Case
    if ((typeof(kn_to) != "string") && (typeof(kn_to) != "function"))
    {
        obj = kn_to;
        kn_to =
            new Function('m', 'obj',
                         'obj.onMessage(m);'
                );
    }

    // Function Case: Default for the cases above, too
    if (typeof(kn_to) == "function")
    {
        //redirect route from /from-topic -> /tunnel-session-topic -> to-function(evt, obj)
        _kn_setHandler(kn_rid, kn_to, obj);
        kn_to = kn.tunnelURI;
    }

    // Default case: kn_to is a topic string
    if (typeof(kn_to) == "string")
    {
        if (! e.do_method)
            e.do_method = "route";
        e.kn_from = kn_from;
        e.kn_to = kn_to;
        e.kn_id = kn_id;
        e.kn_uri = kn_rid;
        // handle kn_options
        if (kn_options)
        {
            for (var p in kn_options)
            {
                e[p] = kn_options[p];
            }
        }

        _kn_setMessageDefaults(e) ;
        return kn__submitRequest(e) ;
    }
    return true;
}

// implementation of kn._doMethod
function _kn_doMethod(method, options, handler)
{
    // copy the request to avoid mangling user data
    var e = _kn_object("do_method", method);
    if (options)
    {
        for (var key in options)
        {
            e[key] = options[key];
        }
    }
    var ctx = _kn_object();
    ctx.e = e;
    ctx.handler = handler;
    var ptr = kn._memorize(ctx);
    kn._workQ[kn._workQ.length] = new Function(
        'var ctx = kn._recall(' + ptr + ', "_kn_doMethod(1)");' +
        'ctx.e.kn_status_from = _kn_setStatusHandler(ctx.handler);' +
        'if (kn__submitRequest(ctx.e))' +
        '{' +
        /**/'kn._forget(' + ptr + ', "_kn_doMethod(2)");' +
        /**/'return true;' +
        '}' +
        '_kn_clearHandler(ctx.e.kn_status_from);' +
        'return false;'
        );
    kn._processWorkQ(); //goose the scanner to try immediately
}

// kn.publishForm converts an HTML form into a mod_pubsub event, and
// publishes the resulting event using kn_publish. The function will
// not work for multiple selects, submit and buttons. The function
// does not support file objects; it will publish the path of the file
// but not the contents. Thanks to Poornima Srinivasan for the
// improvements for checkbox and radio button support.

// implementation of kn.publishForm
function _kn_publishForm(kn_to, theForm, handler)
{
    var e = _kn_object();
        
    for (var i = 0; i < theForm.elements.length; i++)
    {
        var _l_element = theForm.elements[i];
        var _l_type = //_
            _l_element.type.toLowerCase();
        if (_l_type == "checkbox" ||
            _l_type == "radio")
        {
            if (_l_element.checked)
            {
                e[_l_element.name] = _l_element.value;
            }
        }
        else
        {
            e[_l_element.name] = _l_element.value;
        }
    }
    return kn_publish(kn_to, e, handler);
}

function _kn_setMessageDefaults(e)
{
    if (e.userid == null)
        e.userid = kn.userid;

    if (e.displayname == null)
        e.displayname = kn.displayname;

    if (e.kn_id == null)
        e.kn_id = Math.random().toString().substring(2, 10);

    return e ;
}

// kn.publish takes three arguments:
// - the destination URL
// - the input message object
// - a handler object (optional; see kn._doMethod)

// implementation of kn.publish
function _kn_publish(kn_to, event, handler)
{
    // copy the event to avoid mangling user data
    var e = _kn_object();
    for (var key in event)
        e[key] = event[key];
    if (e.kn_to == null)
        e.kn_to = kn_to ;
    _kn_setMessageDefaults(e) ;

    // enqueue request
    kn.NOTIFY(e, handler);

    // For client return identification
    return e.kn_id;
}

// kn.unsubscribe takes two arguments: the kn_route_location URI and
// the optional status event handler object (see kn._doMethod)

// implementation of kn.unsubscribe
function _kn_unsubscribe(rid, handler)
{
    var _l_end_of_topic = rid.lastIndexOf("/kn_routes/");

    if (_l_end_of_topic == -1)
    {
        var e =
            _kn_object(
                "status",
                _kn_$("400 Bad Request"),
                "kn_payload",
                _kn_$("Client will not delete a route without the magic '/kn_routes/' substring.")
                );
        kn_doStatus(e, handler);
        return;
    }

    var eid = kn_unescape(rid.substring(_l_end_of_topic + 11));
    var _l_topic = //_
        rid.substring(0, _l_end_of_topic);
    var k = _l_topic.indexOf(kn_server);
    if (k != -1)
        _l_topic = _l_topic.substring(k + kn_server.length);

    var e = _kn_object(
        "kn_from", _l_topic,
        "kn_to", "",
        "kn_id", eid,
        "kn_expires", "+5"
        );
    _kn_setMessageDefaults(e) ;

    // enqueue request
    kn.ROUTE(e, handler);
}

// implementation of kn.sendQueue
function _kn_sendQueue(queue, options, handler)
{
    var e = _kn_object("kn_batch", new Array());
    for (var i = 0; i < queue.length; i ++)
    {
        e.kn_batch[i] = kn_encodeRequest(queue[i]);
    }
    if (options) for (var h in options) e[h] = options[h];
    return kn.BATCH(e, handler);
}


////////////////////////////////////////////////////////////////////
// PACKAGING, ROUTING, AND DISPATCH
//

// this internal function is used to wipe out an unwanted event source
function _kn_silenceWindow(theWindow)
{
    // we can not silence an undead source
    if (! kn_isReady(theWindow))
        return;
    // named windows can be removed from the rolls
    if (theWindow.name != null)
    {
        // remove it from the list of known event sources
        _kn_paidInformants[theWindow.name] = null;
        delete _kn_paidInformants[theWindow.name];
        // delete any redraw string state for this window
        kn.documents[theWindow.name] = null;
        delete kn.documents[theWindow.name];
    }
    // cancel loading
    theWindow.document.close();
    if (theWindow.stop) theWindow.stop();
    // replace with an empty page so control-N will not cause nasty duplicate posting
    if (((theWindow.location.pathname + theWindow.location.search) != kn_blank) &&
        (theWindow.location != kn_blank))
    {
        theWindow.location.replace(kn_blank);
    }
}

// implementation of kn._sendCallback
function _kn_sendCallback(theEvent, theWindow)
{
    if (_kn_debug("receipts") && theWindow) //#
    { //#
        var html = new Array(); //#
        html[html.length] = '<hr />'; //#
        html[html.length] = '<div class="dl"><dl>'; //#
        for (var i = 0; i < theEvent.elements.length; i ++) //#
        { //#
            var elt = theEvent.elements[i]; //#

            var name = (elt.name != null) ? elt.name : _kn_utf8decode(elt.nameU); //#
            html[html.length] = '<dt><var>'; //#
            html[html.length] = kn_htmlEscape(name); //#
            html[html.length] = '<' + '/var>:<' + '/dt>'; //#

            var value = (elt.value != null) ? elt.value : _kn_utf8decode(elt.valueU); //#
            html[html.length] = '<dd>'; //#
            html[html.length] = kn_htmlEscape(value); //#
            html[html.length] = '<' + '/dd>'; //#
        } //#
        html[html.length] = '<' + '/dl><' + '/div>'; //#
        theWindow.document.write(html.join("")); //#
    } //#

    // if we got an event, the server has managed to recover
    if (kn && kn._isBrokenP)
      {
        kn._isBrokenP = false;
      }

    // events from unexpected windows get thrown out
    // with extreme prejudice (i.e. we smash the source
    // windows)
    if (theWindow && ! _kn_paidInformants[theWindow.name])
    {
        if (_kn_debug("routes")) //#
        { //#
            alert(_kn_$_("[KNJS:Transport] The window \"%{0}\" has spoken out of turn, and will be removed.", //#
                         theWindow.name)); //#
        } //#
        _kn_silenceWindow(theWindow);
        return;
    }

    // convert the event from "form" style to a simple Object
    var e = _kn_object();

    for (var i = 0; i < theEvent.elements.length; i++)
    {
        var ei = theEvent.elements[i];
        var name = (ei.nameU == null) ? ei.name : _kn_utf8decode(ei.nameU);
        var _l_value = //_
            (ei.valueU == null) ? ei.value : _kn_utf8decode(ei.valueU);
        e[name] = _l_value;
    }

    // attempt to record information about this event for tunnel
    // recovery purposes
    if (kn)
    {
        if (e.kn_route_checkpoint != null)
        {
            kn.lastTag_ = e.kn_route_checkpoint;
        }
        else if (e.kn_event_hash && kn.tunnelMaxAge)
        {
            var key =
                _kn_escape(e.kn_route_location) +
                " " +
                _kn_escape(e.kn_id) +
                " " +
                _kn_escape(e.kn_event_hash);
            if (kn._l_hashCache1[key] || kn._l_hashCache2[key])
            {
                // duplicate...
                if (_kn_debug("duplicates"))
                {
                    alert(_kn_$_("[KNJS:Transport] A duplicate event was ignored: %{0}",
                                 key));
                }
                return;
            }
            kn._l_hashCache1[key] = true;
            now = new Date();
            if ((now - kn._l_hashCacheDate) >
                (parseFloat(kn.tunnelMaxAge) * 1000))
            {
                kn._l_hashCache2 = kn._l_hashCache1;
                kn._l_hashCache1 = _kn_object();
                kn._l_hashCacheDate = now;
            }
        }
    }

    // call the local event dispatcher
    kn__routeEvent(e);

    // if we got this over our tunnel frame, we may need to schedule a
    // tunnel restart
    if (kn && theWindow && (theWindow.name == kn.TFN_) &&
        ! kn._isRestartingP)
    {
        kn._hits ++;
        if ((! _kn_options("noswap")) &&
            (kn._hits > kn_maxHits))
        {
            _kn_restartTunnel();
        }
    }
}

// this internal function checks for the presence of a particular
// word in the comma-separated parameter string; it returns
// true if the flag is present, and false otherwise; the special
// list element "all" causes it to return true for all flags.
function _kn_checkFlag(flag, parameters)
{
    if ((flag == null) &&
        ((typeof flag) == 'undefined'))
        return parameters ? true : false;
    if (parameters == null)
        return false;
    if ((typeof parameters == 'boolean') &&
        (parameters == true))
        return false;
    if ((','+parameters+',').indexOf(','+flag+',') != -1)
        return true;
    if ((','+parameters+',').indexOf(',all,') != -1)
        return true;
    return false;
}

// implementation of kn__options()
function _kn_options(flag)
{
    return self.kn ? (_kn_checkFlag(flag, kn._options) ||
                      _kn_checkFlag(flag, kn_defaultOptions || kn_defaultHacks)) : false;
}

// implementation of kn__debug()
function _kn_debug(flag)
{
    return (
        self.kn ? //#
        _kn_checkFlag(flag, kn._debug) : //#
        false);
}

// implementation of kn__submitRequest
function _kn_submitRequest(e)
{
    // this function supports the 'single' and 'noforward' options
    var _l_sent = false;

    // the 'single' option disables batching entirely
    if (_kn_options("single") &&
        ((! kn._batch_list) ||
         (! kn._batch_list.length)))
    {
        // hook ourselves into the routing table (manually!)
        var rid = e.kn_status_from;
        var obj = _kn_object();
        obj.handler = kn._dispatchFunction[rid];
        obj.data = kn._forget(kn._dispatchPtr[rid], "_kn_submitRequest");
        kn._dispatchPtr[rid] = kn._memorize(obj);
        kn._dispatchFunction[rid] =
            new Function('evt', 'obj',
                         'kn._onPostSuccess(evt);' +
                         'return obj.handler(evt, obj.data);'
                );

        // the 'noforward' option disables status forwarding
        if (! _kn_options("noforward"))
            e.kn_status_to = kn.tunnelURI;
        return _kn_submitFormInFrame(e, kn._postFrame);
    }

    if (! kn._batch_list)
        kn._batch_list = new Array();
    if ((kn._batch_list.length == 0) ||
        (kn._batch_list[0] == e))
    {
        kn._batch_list[0] = e;
        if (! kn._postFrameBusy)
        {
            var _l_batch = //_
                _kn_object("do_method", "batch");
            var _l_handler = //_
                _kn_object();
            _l_handler.onSuccess = kn._onPostSuccess;
            _l_handler.onError = kn._onPostError;
            _l_batch.kn_status_from =
                _kn_setStatusHandler(_l_handler);
            // the 'noforward' option disables status forwarding
            if (! _kn_options("noforward"))
                _l_batch.kn_status_to = kn.tunnelURI;
            _l_batch.kn_batch = kn._batch_list;

            // if we're here, the event that was passed to us was the
            // first one in a batch; we want to return false if the
            // batch needs to be retried, or true if we successfully
            // send it.

            _l_sent = _kn_submitFormInFrame(_l_batch, kn._postFrame);
            if (_l_sent)
            {
                kn._batch_list = null;
                delete kn._batch_list;
            }
            else
            {
                _kn_clearHandler(_l_batch.kn_status_from);
            }
        }
    }
    else if (kn._batch_list.length < kn.maxBatchSize)
    {
        kn._batch_list[kn._batch_list.length] = e;
        // since we've just batched up the request, we can return true.
        _l_sent = true;
    }
    return _l_sent;
}

// stolen from Object2Xml.js
// returns true when obj is an Array
function _kn_isArray(obj)
{
    if (!obj || (typeof obj.length == (null)))
        return false;

    if (obj.constructor == Array)
        return true;

    if (obj.constructor == String)
        return false;

    var _l_bFound = false;
    for (var i in obj)
    {
        if (typeof obj[i] == 'function')
            continue;

        if (isNaN(parseInt(i)))
        {
            return false;
        }
        _l_bFound = true;
    }
    return _l_bFound;
}

// send a form "e" to the KN server in frame "f",
// with results send to the window named "target"
// returns false until the request has been sent
function _kn_submitFormInFrame(e, f)
{
    if (f == kn._postFrame && kn._postFrameBusy) {
        return false; // signal the workQ to keep trying
    }

    if (f == kn._postFrame)
        kn._postFrameBusy=true;
        
    var _l_html;
    _l_html =
        '<html>\n'+
        '<head>\n'+
        '<title>' + _kn_$("Event Submission Form") + '<' + '/title>\n'+
        '<' + '/head>\n'+
        '<body onLoad="';

    // Netscape Navigator 4.x has a serious setTimeout bug, so we
    // act immediately there; other browsers (Netscape
    // 6/Mozilla, at least) need to do this from a
    // setTimeout to avoid occasional JavaScript errors
    if ((navigator.appVersion.charAt(0) == '4') &&
        (navigator.appVersion.indexOf("MSIE ") == -1))
    {
        _l_html +=
            'document.forms.eventForm.submit()';
    }
    else
    {
        _l_html +=
            'setTimeout(\'document.forms.eventForm.submit()\',1)';
    }

    _l_html +=
        '">\n'+
        '<form name="eventForm" method="post"'+
        ' action="' +
        kn_htmlEscape(kn_server) +
        '">' +
        (
            _kn_debug("posts") ? //#
            '<div class="dl"><dl>' : //#
            '') +
        '\n';
    for (var h in e)
    {
        // if this property exists and can be rendered as a string...

        // if this property is an array we split it into multiple fields with the same name
        if (_kn_debug("posts")) //#
        { //#
            _l_html += //#
                '<dt><var>' + //#
                kn_htmlEscape(h) + //#
                '<' + '/var>:<' + '/dt>' + //#
                '<dd>'; //#
        } //#
        if (_kn_isArray(e[h]))
        {
            for (var i = 0; i < e[h].length; i++)
            {
                if (typeof(e[h][i]) == "string")
                {
                    _l_html +=
                        '<input type="hidden" name="' + kn_htmlEscape(h) +
                        '" value="' + kn_htmlEscape(e[h][i]) + '" />';
                    if (_kn_debug("posts")) //#
                    { //#
                        if (i) //#
                            _l_html += //#
                                '<' + '/dd>\n' + //#
                                '<dd>'; //#
                        _l_html += //#
                            kn_htmlEscape(e[h][i]); //#
                    } //#
                }
                else
                {
                    if (i && _kn_debug("posts")) //#
                        _l_html += '<hr />'; //#
                    _l_html +=
                        '<input type="hidden" name="' + kn_htmlEscape(h) +
                        '" value="' + kn_encodeRequest(e[h][i]) + '" />';
                    if (_kn_debug("posts")) //#
                    { //#
                        _l_html += '<div class="dl"><dl>\n'; //#
                        for (var j in e[h][i]) //#
                        { //#
                            _l_html += //#
                                '<dt><var>' + kn_htmlEscape(j) + //#
                                '<' + '/var>:<' + '/dt>' + //#
                                '<dd>' + kn_htmlEscape(e[h][i][j].toString()) + //#
                                '<' + '/dd>\n'; //#
                        } //#
                        _l_html += '<' + '/dl><' + '/div>\n'; //#
                    } //#
                }
            }
        }
        else if ((e[h] != null) && (typeof(e[h].toString) == "function"))
        {
            _l_html +=
                '<input type="hidden" name="' + kn_htmlEscape(h) +
                '" value="' + kn_htmlEscape(e[h].toString()) + '" />';
            if (_kn_debug("posts")) //#
            { //#
                _l_html += kn_htmlEscape(e[h].toString()); //#
            } //#
        }
        if (_kn_debug("posts")) //#
        { //#
            _l_html += '<' + '/dd>\n'; //#
        } //#
    }
    if (_kn_debug("posts")) //#
        _l_html += '<' + '/dl><' + '/div>\n'; //#
    if (_kn_debug("posts")) {  // this is gratuitous UI for human debugging //#
        _l_html += '<input type="submit" value="' + _kn_$("go") + '" />\n'; //#
    } //#
    _l_html += '<' + '/form>\n';
    _l_html += '<' + '/body>\n';
    _l_html += '<' + '/html>\n';

    // generate the page.
    _kn_paidInformants[f.name] = true;
    if (! kn.documents[f.name])
    {
        kn.documents[f.name] = _kn_object();
        kn.documents[f.name].state = "ready";
    }
    kn.documents[f.name].html = _l_html;
    if (kn.documents[f.name].state != "loading")
    {
        kn.documents[f.name].state = "loading";
        if (f.stop) f.stop();
        f.location.replace(kn_blank);
    }

    if (f == kn._postFrame)
    {
        // Netscape Navigator 4.x has a serious setTimeout bug, so we use
        // a different window's timer list there; other browsers (Netscape
        // 6/Mozilla, at least) need to do this from the parent window to
        // avoid occasional JavaScript errors
        if ((navigator.appVersion.charAt(0) == '4') &&
            (navigator.appVersion.indexOf("MSIE ") == -1))
        {
            kn._postFrame.setTimeout('parent._kn_retry()',kn_retryInterval);
        }
        else
        {
            kn._ownerWindow.setTimeout('_kn_retry()',kn_retryInterval);
        }
    }
    return true; // signal to workQ to stop retrying
}

function _kn_retry()
{
    // un-hook from the leader election train
    kn._ownerWindow. //_
        _kn_postPending = false;
    // try to start the tunnel if it's not running already
    if ((!kn.isLoadedP_) ||
        // sometimes the post frame becomes unlinked (in kn_popup, for
        // example)
        ! kn_isReady(kn._postFrame) ||
        // this is a very odd and illegal state: F5 has reloaded
        // the frameset, including a tunnel frame from a past life
        // (which never loads, thus preventing the KN frameset's own
        // _kn_framesetOnLoad from ever being called
        (!kn.leaderWindow.kn) ||
        (!kn.leaderWindow.kn.isLoadedP_) ||
        (!kn.leaderWindow.kn.tunnelRunning_))
        // this is a normal case; we are the leader, but have not been
        // ratified in office yet
    {
        // postpone the work and initiate the leader election algorithm
        if (kn._ownerWindow == kn.leaderWindow)
        { 
            // tie onto the leader election train
            kn._ownerWindow. //_
                _kn_postPending = true;
            if (!_kn_tryingToInitTunnel)
                _kn_initTunnel();
        }
        else
        {
            // Netscape Navigator 4.x has a serious setTimeout bug, so we use
            // a different window's timer list there; other browsers (Netscape
            // 6/Mozilla, at least) need to do this from the parent window to
            // avoid occasional JavaScript errors
            if ((navigator.appVersion.charAt(0) == '4') &&
                (navigator.appVersion.indexOf("MSIE ") == -1) &&
                kn_isReady(kn._postFrame) &&
                kn._postFrame.parent == kn._ownerWindow)
            {
                kn._postFrame.setTimeout('parent._kn_retry()',kn_retryInterval);
            }
            else
            {
                kn._ownerWindow.setTimeout('_kn_retry()',kn_retryInterval);
            }
        }
        return;
    }
    if (kn._postFrameBusy)
    {
        kn_retryInterval = kn_retryInterval * 2;
        if (kn_retryInterval > kn_maxRetryInterval) kn_retryInterval = kn_maxRetryInterval;
        if (kn._postFrame.stop) kn._postFrame.stop();
        kn._postFrame.location.replace(kn_blank);
        // Netscape Navigator 4.x has a serious setTimeout bug, so we use
        // a different window's timer list there; other browsers (Netscape
        // 6/Mozilla, at least) need to do this from the parent window to
        // avoid occasional JavaScript errors
        if ((navigator.appVersion.charAt(0) == '4') &&
            (navigator.appVersion.indexOf("MSIE ") == -1))
        {
            kn._postFrame.setTimeout('parent._kn_retry()',kn_retryInterval);
        }
        else
        {
            kn._ownerWindow.setTimeout('_kn_retry()',kn_retryInterval);
        }
        if (_kn_debug()) //#
        { //#
            kn._ownerWindow.status = //#
                _kn_$_("retrying event publication @%{0}", //#
                       kn_retryInterval //#
                    ); //#
        } //#
    }
}

////////////////////////////////////////////////////////////////////
// LEADER ELECTION AND SYNCHRONIZATION
//

function _kn_initTunnel(force)
{
    _kn_tryingToInitTunnel = true;

    if (! kn.isLoadedP_ ||
        ! kn_isReady(kn.tunnelFrame_))
    {
        // spin cycle, waiting to resolve an F5 or control-N reload
        // race condition, or a temporarily inaccessible tunnel frame
        // (e.g. pop-up window proxy was closed for some reason)

        kn._ownerWindow.setTimeout("_kn_initTunnel(" +
                                   (force || "") +
                                   ")",_kn_heartbeat);
        return;
    }

    // We also announce our plans: proposed tunnel frame and dest domain
    // Knowing full well that a booted, running kn leader could stomp us
    // This cookie will expire at the end of this browser session
    var _l_cookieSupport = true;
    var _l_noOtherCookies = (document.cookie == "");
    document.cookie = _kn_escape(kn.TFN_) + "=opening;path=/";  // announce

    // Without cookie support, we still attempt to launch one tunnel, at least.
    if ((typeof navigator.cookieEnabled == 'boolean' &&
         navigator.cookieEnabled == false) ||
        (! document.cookie) ||
        (document.cookie.indexOf(_kn_escape(kn.TFN_)) == -1))
    {
        _l_cookieSupport=false;
        if ((navigator.appName.indexOf("Microsoft") != -1) &&
            (navigator.platform == "Win32") &&
            (kn._ownerWindow == top) &&
            ! _kn_options("quiet"))
            alert(
                _kn_$("[KNJS:IPC] This web application uses local cookies for communication. Without permission to set local cookies, Internet Explorer on Windows can only support one such web application at a time.")
                );
    }

    if (!kn._leaderScannerTimer)
    {
        // do not spawn an infinite number of parallel threads ;-)
        kn._leaderScannerTimer =
            kn._ownerWindow.kn._setInterval(
                _l_cookieSupport ?
                "_kn_leaderScanner()" :
                "_kn_checkTunnel()",
                _kn_heartbeat
                );
    }

    if (_kn_debug() && kn_isReady(kn.tunnelFrame_)) kn.tunnelFrame_.document.bgColor="blue"; //#

    if ((_l_noOtherCookies || force) && kn_isReady(kn.tunnelFrame_))
        _kn_launchTunnel();
    else  // gives the leader-election alg 5 beats to converge
        setTimeout("_kn_launchTunnel()",5*_kn_heartbeat);
}

// implementation of kn__restartTunnel()
function _kn_restartTunnel()
{
    kn._isRestartingP = true;
    if (kn.tunnelRunning_)
    {
        kn.tunnelRunning_ = false;
        if (self.kn_onTunnelStop) kn_onTunnelStop();
    }
    if (_kn_debug()) //#
    { //#
        kn._ownerWindow.status = //#
            _kn_$_("Restarting Tunnel: %{0}#%{1}", //#
                   kn.tunnelURI, //#
                   kn.lastTag_); //#
    } //#
    // What do we need to do to set this transaction up properly?
    _kn_initTunnel(true);
}

function _kn_launchTunnel()
{
    if (!kn.tunnelRunning_ && _kn_tryingToInitTunnel &&
        (kn._ownerWindow == kn.leaderWindow)) // sanity check; self believes it needs to be leader
    {
        var s = kn.tunnelURI +
            '/kn_status/' +
            Math.random().toString().substring(2, 10) +
            '_' +
            (kn._seqNum++);
        // like kn.setHandler(s, kn._onTunnelStatus, s), only we know
        // we're the leader
        kn._dispatchPtr[s] = kn._memorize(s);
        kn._dispatchFunction[s] =
            new Function('e', 's',
                         'kn._forget(kn._dispatchPtr[s], "_kn_launchTunnel");' +
                         'kn._dispatchFunction[s] = null;' +
                         'delete kn._dispatchFunction[s];' +
                         'kn._onTunnelStatus(e);'
                );

        _kn_paidInformants[kn.TFN_] = true;
        var _l_checkpoint = '';
        if (kn.lastTag_ != null)
        {
            _l_checkpoint =
                ';do_since_checkpoint=' +
                _kn_escape(kn.lastTag_);
        }
        else if (kn.tunnelMaxAge)
        {
            _l_checkpoint =
                ';do_max_age=' +
                _kn_escape(kn.tunnelMaxAge);
        }
        // bytes to send after pauses in the event stream
        var _l_flush = kn_response_flush;
        if (_l_flush == null)
        {
            // we assume most browsers need 4k to flush their buffers
            _l_flush = "4096";
        }
        if (kn.tunnelFrame_.stop) kn.tunnelFrame_.stop();
        kn.tunnelFrame_.location.replace(
            kn_server +
            '?kn_from=' + _kn_escape(kn.tunnelURI) +
            ';kn_id=' + _kn_escape(kn.tunnelID) +
            ';kn_response_flush=' + _kn_escape(_l_flush) +
            ';kn_status_from=' + _kn_escape(s) +
            _l_checkpoint
            );
        kn._tunnelStartTime = new Date();
    }
}

function _kn_leaderScanner()
{
    // gets called on a regular heartbeat; ensures a single tunnel at most
    // first, let's parse all the cookies
    var _l_cookie = document.cookie;
    var _l_allcookies= //_
        _kn_object();
    var _l_KNcookies = //_
        _kn_object();

    if (_l_cookie && _l_cookie.length)
    {
        var c=('' + document.cookie).split(" ").join("");
        _l_allcookies = c.split(";");

        if (_l_allcookies && _l_allcookies.length)
        {
            for (var i = 0; i < _l_allcookies.length; i++)
            {
                var _l_splitPos = //_
                    _l_allcookies[i].indexOf("=");
                var app = _l_allcookies[i].substring(0, _l_splitPos);
                var _l_mode = //_
                    _l_allcookies[i].substring(_l_splitPos+1);
                if (_kn_matchPrefix("kn_", app))
                {
                    _l_KNcookies[app] = _l_mode;
                }
            }
        }
    }

    // there is a shared cookie called kn_leader=kn_appname_sessionid
    if (kn._isLeaderP &&
        (_l_KNcookies["kn_leader"] != _kn_escape(kn.TFN_)))
    {
        // make sure that this pretender to the throne is not a victim
        // I've already scheduled for execution
        if (kn._followers[_kn_unescape(_l_KNcookies["kn_leader"])]) {
            // remind them who's boss; they may yet kill themselves
            document.cookie = "kn_leader=" + _kn_escape(kn.TFN_) + ";path=/";
            kn._isLeaderP = true;
            kn.tunnelRunning_ = true;
            return;
        }

        // this is an exceptional case: I, the leader, have been deposed.
        // this is either within a few milliseconds of opening the tunnel,
        // or else I have been locked in a modal loop, and others assumed I
        // was dead due to my inattention.

        // this code does NOT attempt to escrow onto any of its followers, if any
        // after all, this state requires the new leader to call kn__follow on me.

        // Of course, it broke Mozilla, so I killed it - Ben.
    }

    // _l_myMode has four modes: kn_appname_sessionid_tunnel={opening,closing,closed,oops}
    var _l_myMode = //_
        _l_KNcookies[_kn_escape(kn.TFN_)];

    // 0: Has a coup against me failed?

    if (_l_myMode == "oops")
    {
        _kn_restartTunnel();
        return;
    }

    // 1: Is there a death warrant for me?

    if ((_l_myMode == "closing") && !_kn_aboutToRunTunnel)
    {
        if (kn.tunnelRunning_) //#
        { //#
            if (_kn_debug()) //#
            { //#
                kn._ownerWindow.status = //#
                    _kn_$("KN: Shutting down concurrent tunnel. Recovery shunt UNIMPLEMENTED"); //#
            } //#
        } //#
        kn._isLeaderP = false;
        _kn_silenceWindow(kn.tunnelFrame_);
        if (_kn_debug() && kn_isReady(kn.tunnelFrame_)) kn.tunnelFrame_.document.bgColor="green"; //#
        _kn_tryingToInitTunnel=false;
        document.cookie = _kn_escape(kn.TFN_) + "=closed;path=/";
        // do what _kn_assumeLeadership() would have done
        if (kn._ownerWindow. //_
            _kn_postPending) //_
            _kn_retry();
        return; // short-circuit the rest
    }

    // 2. Do I need to kill anyone?

    if ((kn.tunnelRunning_ && kn._isLeaderP) || _kn_aboutToRunTunnel);
    {
        for (var _l_victim //_
                 in _l_KNcookies)
        {
            if ((_l_victim != _kn_escape(kn.TFN_)) &&
                ! kn._unreachable[_l_victim]) {
                if ((_l_KNcookies[_l_victim] == "opening"))
                {
                    // usually, kill, but if abouttoruntunnel, kill on sort order
                    if (!_kn_aboutToRunTunnel || (_l_victim < _kn_escape(kn.TFN_)))
                        document.cookie = _l_victim + "=closing;path=/";
                }
                else if (_l_KNcookies[_l_victim] == "closed")
                {
                    _kn_clearCookie(_l_victim); // acknowledge three-phase handshake
                    _l_victim = _kn_unescape(_l_victim);
                    kn._followers[_l_victim]= open(kn_blank,_l_victim);
                    _kn_finishCoup(_l_victim);
                }
            }
        }
    }
    _kn_checkTunnel();
}

function _kn_checkTunnel() {
    // 3. Do I need to restart my tunnel?
    
    /* check if tunnel frame became radioactive. A dead frame is a dead tunnel, no? */
    
    if (kn._isLeaderP && kn.tunnelRunning_ && !kn_isReady(kn.tunnelFrame_))
    {
        if (!_kn_tryingToInitTunnel)
        {
            kn_retryInterval = kn_retryInterval * 2;
            if (kn_retryInterval > kn_maxRetryInterval) kn_retryInterval = kn_maxRetryInterval;
            _kn_restartTunnel();
        }
        return;
    }
    
    if (kn._isLeaderP && kn.tunnelRunning_ && kn_isReady(kn.tunnelFrame_) && 
        (kn.tunnelFrame_.document.readyState == "complete")) {
        _kn_tunnelLoadCallback(kn.tunnelFrame_);
    }
    else
    {
        // sanity check; self believes it needs to be leader
        if (!kn.tunnelRunning_ && _kn_tryingToInitTunnel &&
            (kn._ownerWindow == kn.leaderWindow) &&
            (kn._tunnelStartTime != null))
        {
            var now = new Date();
            if ((now - kn._tunnelStartTime) > kn_retryInterval)
            {
                kn_retryInterval = kn_retryInterval * 2;
                if (kn_retryInterval > kn_maxRetryInterval) kn_retryInterval = kn_maxRetryInterval;
                kn__restartTunnel();
            }
        }
    }
}

function _kn_finishCoup(newFollower)
{
    var w = kn._followers[newFollower];

    if (kn_isReady(w))
    {
        var followerKN = null;
        if (typeof(w.parent.kn) != "unknown")
        {
            followerKN = w.parent.kn;
        }
        if (!followerKN)
        {
            // something went horribly wrong... the transporter
            // delivered our soldiers to the inside of a vacuum chamber
            kn._followers[newFollower] = null;
            w.close();
            var _l_cookieName = //_
                _kn_escape(newFollower);
            kn._unreachable[_l_cookieName] = true;
            document.cookie = _l_cookieName + "=oops;path=/";
            if (_kn_debug()) //#
            { //#
                // OOPS! dying horribly... //#
                alert(_kn_$("[KNJS:IPC] Inter-window communication failure, partitioning.")); //#
            } //#
        }
        else
        {
            // a *remote* invocation to follow, rather than directly
            // "writing" to a mess of the follower's private vars.
            followerKN.ownerWindow.kn__follow(kn._ownerWindow, kn.tunnelURI, kn.lastTag_);
        }
    }
    else
    {
        // keep knocking until the beachhead has been secured
        kn._ownerWindow.setTimeout("_kn_finishCoup(_kn_unescape('" +
                                  _kn_escape(newFollower) +
                                  "'))", _kn_heartbeat);
    }
}

function _kn_escrow(my_kn, hero_kn)
{
    //send over the crown jewels, AS A STRING!
    // e.g. current route table, current tunnel URL
    // and a list of followers, indexed window ID

    // these objects are required to fool the compressor
    var _l_my = //_
        _kn_object();
    _l_my.kn = my_kn;
    var _l_hero = //_
        _kn_object();
    _l_hero.kn = hero_kn;

    // forward the window dispatch table
    for (i in _l_my.kn.dispatch_)
    {
        if (_l_my.kn.dispatch_[i] != _l_hero.kn.ownerWindow)
        {   // do not rescue those who are not drowning.
            if (_kn_debug() && kn_isReady(_l_hero.kn.tunnelFrame_)) _l_hero.kn.tunnelFrame_.document.bgColor="orange"; //#
            // escrowing color mode iff remote routes exist *to a third party*
            _l_hero.kn.dispatch_[i] = _l_my.kn.dispatch_[i];
            if (_l_hero.kn.dispatch_[i].kn)
                _l_hero.kn.dispatch_[i].kn.leaderWindow = _l_hero.kn.ownerWindow;
        }
    }

    // forward any pending events from our delivery queues (as strings!)
    var ql = _l_my.kn.leaderWindow.kn__queue;
    if (ql)
    {
        for (var tfn in ql)
        {
            while (ql[tfn])
            {
                var s = new Array();
                s[0] = 'kn__routeEvent(kn__object(';
                var evt = ql[tfn].event;
                ql[tfn] = ql[tfn].next;
                var first = true;
                for (var hdr in evt)
                {
                    if (! first)
                    {
                        s[s.length] = ',';
                    }
                    first = false;
                    s[s.length] = 'kn_unescape("' + _kn_escape(hdr) + '"),';
                    s[s.length] = 'kn_unescape("' + _kn_escape(evt[hdr]) + '")';
                }
                s[s.length] = '))';
                _l_hero.kn.ownerWindow.setTimeout(s.join(''), 1);
            }
        }
    }

    // send over our tunnel, including a checkpoint if we have one
    if (! _l_hero.kn.tunnelRunning_ && _l_my.kn.tunnelURI) {
        _l_my.kn._isLeaderP = false; // make sure our scanners do not kill him in the process
        // this should pass the old tunnel/journal topic to the new regent.
        _l_hero.kn.tunnelURI = _l_my.kn.tunnelURI;
        _l_hero.kn.lastTag_ = _l_my.kn.lastTag_;
        // and fire him up!
        _l_hero.kn.leaderWindow = _l_hero.kn.ownerWindow; // initiate delusions of grandeur

        // Netscape Navigator 4.x has a serious setTimeout bug, so we
        // act immediately there; other browsers (Netscape
        // 6/Mozilla, at least) need to do this from a
        // setTimeout to avoid occasional JavaScript errors
        if ((navigator.appVersion.charAt(0) == '4') &&
            (navigator.appVersion.indexOf("MSIE ") == -1))
        {
            _l_hero.kn.ownerWindow.kn__restartTunnel();
        }
        else
        {
            _l_hero.kn.ownerWindow.setTimeout("kn__restartTunnel()",_kn_heartbeat);
        }
    }
}

function _kn_onPostError(ev)
{
    if (_kn_debug()) //#
        alert( //#
            _kn_$_("[KNJS:Transport] Request failed: %{0}\n\n%{1}", //#
                   ev.status, //#
                   ev.kn_payload //#
                ) //#
            ); //#
    _kn_onPostSuccess(ev);
}

function _kn_onPostSuccess(ev)
{
    // this function supports the 'noforward' option
    if (! kn__options("noforward")) _kn_silenceWindow(kn._postFrame);
    if (_kn_debug() && kn_isReady(kn._postFrame)) kn._postFrame.document.bgColor = "yeLLow"; //#
    kn._postFrameBusy = false;

    // Netscape Navigator 4.x has a serious setTimeout bug, so we
    // act immediately there; other browsers (Netscape
    // 6/Mozilla, at least) need to do this from a
    // setTimeout to avoid occasional JavaScript errors
    if ((navigator.appVersion.charAt(0) == '4') &&
        (navigator.appVersion.indexOf("MSIE ") == -1))
    {
        kn._ownerWindow.kn._processWorkQ();
    }
    else
    {
        kn._ownerWindow.setTimeout("kn._processWorkQ()", 1);
    }
}

function _kn_processWorkQ()
{
    // try to start the tunnel if it's not running already
    if ((!kn.isLoadedP_) ||
        // this is a very odd and illegal state: F5 has reloaded
        // the frameset, including a tunnel frame from a past life
        // (which never loads, thus preventing the KN frameset's own
        // _kn_framesetOnLoad from ever being called
        (!kn.leaderWindow.kn) ||
        (!kn.leaderWindow.kn.isLoadedP_) ||
        (!kn.leaderWindow.kn.tunnelRunning_))
        // this is a normal case; we are the leader, but have not been
        // ratified in office yet
    {
        // postpone the work and initiate the leader election algorithm
        if (kn._ownerWindow == kn.leaderWindow)
            if (!_kn_tryingToInitTunnel)
                _kn_initTunnel();
        return;
    }

    // if leader election is still in progress return
    if ((!kn.isLoadedP_) ||
        (!kn.tunnelRunning_ && (kn._ownerWindow == kn.leaderWindow)))
    {
        return;
    }

    // every publish and subscribe request is put on this workQ.
    // the workQ is processed once the tunnel is started
    //     (to clear pending subscriptions, mainly)
    // and every time the postFrame becomes unbusy.
    //     (to process the next publication, mainly)
    // Items stay on the workQ until they return true
    //     (the beatings will continue until morale improves)
    if ((kn._workQcursor < kn._workQ.length) &&
        kn._workQ[kn._workQcursor])
    {
        var _l_workItem = //_
            kn._workQ[kn._workQcursor];
        kn._workQ[kn._workQcursor] = null;

        if ((_l_workItem)())
        {
            kn._workQcursor++;
        }
        else
        {   // if work does not return true, try again
            //kn._workQ[kn._workQcursor] = _l_workItem;
            kn._workQ[kn._workQ.length] = _l_workItem;
            kn._workQcursor++;
        }
    }

    if (kn._workQcursor < kn._workQ.length)
    {   // if there's more work, make sure we're working!
        if (!kn._worker)
            kn._worker = kn._ownerWindow.kn._setInterval(
                "kn._processWorkQ()",
                _kn_heartbeat
                );
    }
    else if (kn._worker)
    {   // go back to sleep
        kn._ownerWindow.kn._clearInterval(kn._worker);
        kn._worker = null;
    }

    // collapse the workQ when it exceeds 50% empty slots; normally,
    // this will keep it quite short

    if ((2 * kn._workQcursor) >= kn._workQ.length)
    {
        var nq = new Array();
        for (var i = kn._workQcursor; i < kn._workQ.length; i ++)
        {
            if (kn._workQ[i]) nq[nq.length] = kn._workQ[i];
        }
        kn._workQ = nq;
        kn._workQcursor = 0;
    }

}

// implementation of kn._tunnelLoadCallback
function _kn_tunnelLoadCallback(theWindow)
{
    // we do not want to lock up during tunnel restarting
    if (theWindow != kn.tunnelFrame_)
    {
        return;
    }
    // this code is never called in IE when a connection is interrupted.
    // it is called manually when the leader scanner detects an interruption
    // (AT LEAST ON IE...)
    if (kn.tunnelRunning_)
    {
        kn.tunnelRunning_ = false;
        if (self.kn_onTunnelStop) kn_onTunnelStop();
    }
    if (_kn_debug() && kn_isReady(kn.tunnelFrame_)) kn.tunnelFrame_.document.bgColor="green"; //#
    // someday, this will intelligently consider restarting after some watchdog interval
    // i.e. if a proxy dropped it
    kn._ownerWindow.setTimeout("kn__restartTunnel()",5*_kn_heartbeat);
}

function _kn_onTunnelStatus(ev)
{
    if (self.kn_onTunnelStatus) kn_onTunnelStatus(ev);
    if (ev.status)
    {
        var _l_code = ev.status.charAt(0);
        if ((_l_code != '1') &&
            (_l_code != '2') &&
            (_l_code != '3') &&
            ! kn._isBrokenP)
        {
            kn._isBrokenP = true;
            if (! _kn_options("quiet"))
                alert(
                    _kn_$_("[KNJS:Transport] Can not connect: %{0}\nURL: %{1}#%{2}\n\n%{3}\n%{4}",
                           ev.status,
                           kn.tunnelURI,
                           kn.lastTag_,
                           ev.kn_payload,
                           _kn_$("When the server problem is resolved, reload this page.")
                        )
                    );
            return;
        }
    }

    // record reconnect info
    if (ev.kn_journal_reconnect_scheme == "kn_event_hash")
    {
        if (ev.kn_journal_reconnect_timeout)
        {
            kn.tunnelMaxAge = ev.kn_journal_reconnect_timeout;
        }
        else
        {
            kn.tunnelMaxAge = "60";
        }
    }

    // I won the leader sweepstakes!
    // to keep recovery-from-twins clean, pause at least 2 heartbeats before
    // actually committing this transaction. Every "leader" needs a chance
    // to verify our new claim to leadership

    _kn_aboutToRunTunnel=true;
    _kn_tryingToInitTunnel=false;
    kn._ownerWindow.setTimeout("_kn_assumeLeadership()", 3*_kn_heartbeat);
    _kn_leaderScanner(); // goose the scan cycle, regardless of interval
}

function _kn_assumeLeadership()
{
    if (_kn_debug() && kn_isReady(kn.tunnelFrame_)) kn.tunnelFrame_.document.bgColor="red"; //#
    if (kn._isRestartingP)
    {
        kn._isRestartingP = false;
    }
    kn._hits = 0;
    kn.tunnelRunning_ = true;
    document.cookie = "kn_leader=" + _kn_escape(kn.TFN_) + ";path=/";
    kn._isLeaderP = true;
    _kn_clearCookie();
    _kn_aboutToRunTunnel=false;
    // Netscape Navigator 4.x has a serious setTimeout bug, so we end
    // up retrying the request by blanking the post frame there; other
    // browsers use the regular _kn_retry() system.
    if ((navigator.appVersion.charAt(0) == '4') &&
        (navigator.appVersion.indexOf("MSIE ") == -1))
    {
        kn._postFrame.location.replace(kn_blank);
    }
    // restart any pending retry
    if (kn._ownerWindow. //_
        _kn_postPending) //_
        _kn_retry();
    kn._processWorkQ();
}

////////////////////////////////////////////////////////////////////
// WRAPPING THE CALLING PAGE IN A FRAMESET (AppWrapping)
//

function _kn_wrapApp() {
    // Create a frameset iff our parent is NOT a KN window, or
    // if it is a KN window, but not itself a wrapper
    name = _kn_uniqueWindowName();
    kn.TFN_ = name + "_tunnel";

    // abort loading the current document and create a frameset instead
    var _l_framesetdoc =
        '<' + '/head>\n'+
        '<frameset name="'+name+'_esp" rows="'+
        (
            _kn_debug() ? //#
            (_kn_debug("posts") ? '70%,30%' : '90%,10%') : //#
            '100%,*') + '"'+
        (
            _kn_debug() ? '' : //#
            ' border="0"') +
        ' onLoad="_kn_framesetOnLoad();"\n'+
        ' onUnload="_kn_framesetOnUnload();">\n'+
        '   <frame name="'+name+'_app" src="'+kn_blank+'" '+
        _kn_appFrameAttributes +
        (
            _kn_debug() ? '' : //#
            ' frameborder="no"') + '>\n' +
        '   <frameset cols="50%,50%">\n'+
        /*   */'<frame name="'+name+'_tunnel" src="'+kn_blank+'" '+
        (
            _kn_debug() ? '' : //#
            'scrolling="no" noresize frameborder="no"') + '>\n' +
        /*   */'<frame name="'+name+'_post" src="'+kn_blank+'" '+
        (
            _kn_debug() ? '' : //#
            'scrolling="no" noresize frameborder="no"') + '>\n' +
        '   <' + '/frameset>\n'+
        '<' + '/frameset>\n'+
        '<noframes>\n'+
        '   <h1>' + _kn_$("PubSub JavaScript Library Error") + '<' + '/h1>\n'+
        '   <p>' + _kn_$("PubSub requires Frames, JavaScript, and local Cookies") + '<' + '/p>\n'+
        '<' + '/noframes>\n'+
        '<frameset><noframes><xmp><!-' + '-\0';
    document.write(_l_framesetdoc);


    // give the browser a chance to call the onload handler, then
    // force the issue
    if (! self.onload)
    {
        setTimeout('if(!kn.isLoadedP_)_kn_framesetOnLoad()', 1000);
    }
}

function _kn_framesetOnUnload()
{
    // if we own the 'kn' object, nuke all remote routes to local subscriptions
    if ((this == kn._ownerWindow) &&
        (this != kn.leaderWindow) &&
        kn_isReady(kn.leaderWindow) &&
        kn.leaderWindow.kn)
    {
        for (i in kn._dispatchFunction)
        {
            kn.leaderWindow.kn.dispatch_[i] = null;
            delete kn.leaderWindow.kn.dispatch_[i];
        }
    }

    var nav4 = false;

    // Netscape Navigator 4.x has a serious document.cookie bug,
    // causing the script to abort when cookies are manipulated in an
    // onunload handler.
    if ((navigator.appVersion.charAt(0) == '4') &&
        (navigator.appVersion.indexOf("MSIE ") == -1))
    {
        nav4 = true;
    }

    if (nav4 ||
        (('' + document.cookie + ';').split(" ").join("").indexOf("kn_leader=" +
                                                                  _kn_escape(kn.TFN_) +
                                                                  ";") != -1))
    {
        var _l_escrowed = false;

        // abandon leadership, if any
        if (! nav4)
        {
            // that is, if there is a kn_leader cookie of mine, nuke it.
            _kn_clearCookie("kn_leader");
            _kn_clearCookie();
        }
        kn.isLoadedP_ = false;
        kn._isLeaderP = false;

        // call upon any survivors to set up shop
        for (var i in kn.dispatch_)
        {
            // do not escrow onto someone in the same lifeboat
            if (kn_isReady(kn.dispatch_[i]) &&
                 (kn.dispatch_[i].kn != kn) &&
                (kn.dispatch_[i].top != kn._ownerWindow.top))
            {
                _kn_escrow(kn, kn.dispatch_[i].kn);
                _l_escrowed = true;
                break;
            }
        }

        if (! _l_escrowed)
        {
            // call upon any survivors to set up shop
            for (var i in kn.dispatch_)
            {
                if (kn_isReady(kn.dispatch_[i]) &&
                    (kn.dispatch_[i].kn != kn))
                {
                    _kn_escrow(kn, kn.dispatch_[i].kn);
                    _l_escrowed = true;
                    break;
                }
            }
        }
    }
    if (! nav4)
    {
        // stomp tunnel and post frames to prevent history-related
        // accidents later on
        _kn_silenceWindow(kn.tunnelFrame_);
        _kn_silenceWindow(kn._postFrame);
    }
}

function _kn_framesetOnLoad()
{
    // if writing the frameset failed, force a reload
    if (! frames.length)
    {
        location.reload(true);
        return;
    }
    
    if (_kn_debug()) //#
    { //#
        kn._ownerWindow.status = kn_formatString(_kn_$("PubSub JavaScript Library %{RCSID}"), kn); //#
    } //#
    kn.isLoadedP_ = false; // just to be sure
    for (var k = 0; k < frames.length; k++)
    {
        var t = typeof(frames[k].name);

        // sometimes a just-loaded frameset is unable to access its
        // children; we work around this condition by reloading the
        // whole frameset... it seems to work after a single reload
        if (t == "unknown")
        {
            if (_kn_debug()) //#
            { //#
                kn._ownerWindow.status = //#
                    _kn_$_("frame %{0} is not accessible! Rebuilding frameset...", //#
                           k //#
                        ); //#
            } //#
            kn._ownerWindow.location.reload(true);
            return;
        }

        // on control-N, it is possible resubmission-security rules can disable "poster" frames
        if (t != "string")
        {
            if (_kn_debug()) //#
            { //#
                kn._ownerWindow.status = //#
                    _kn_$_("frame %{0} is not accessible! Loading blank and recycling...", //#
                           k //#
                        ); //#
            } //#
            if (frames[k].stop) frames[k].stop();
            frames[k].location.replace(kn_blank);
            // stall, and retry
            kn._ownerWindow.setTimeout("_kn_framesetOnLoad()",5*_kn_heartbeat);
            return;
        }

        // due to Netscape bugs, we walk the frames list to find our app
        // and, as a convenience, set pointers to the three local frames
        if (frames[k].name == (name+"_app"))
        {
            kn._appFrame = frames[k];
        }
        if (frames[k].name == (name+"_tunnel"))
        {
            kn.tunnelFrame_ = frames[k];
        }
        if (frames[k].name == (name+"_post"))
        {
            kn._postFrame = frames[k];
        }
    }

    if (kn._appFrame && kn._appFrame.location && kn._appFrame.location.pathname) {
        // the calling app is reloaded into the appFrame only
        // after everything is set up properly.
        if (((kn._appFrame.location.pathname + kn._appFrame.location.search)
            == kn_blank) ||
            (kn._appFrame.location == kn_blank))
        {
            var _l_new_url = document.URL;
            if (kn._appFrame.stop) kn._appFrame.stop();
            kn._appFrame.location.replace(_l_new_url);
        }
        // NONE of our leader election or other library functions can be used
        // until after this point.
        kn.isLoadedP_ = true;
    }
    else
    {
        // something has not stabilized yet: the frames have loaded,
        // but the application has not. This was observed on Linux Netscape
        // and often leads to a completely broken application.
        location.reload(true);
    }
    
    // expose frame references
    // kn.postFrame_ = kn._postFrame;
    // kn.appFrame_ = kn._appFrame;
    
}

function kn__framesetOnUnload()
{
    return _kn_framesetOnUnload();
}

function kn__hookFrames(post,app)
{
    // hook in shim frame references
    kn._postFrame =  post;
    kn._appFrame = app;
}   

////////////////////////////////////////////////////////////////////
// INITIALIZATION, UTILITY, AND CONVENIENCE FUNCTIONS

// implementation of kn.setHandler; this is a low-level local-only
// equivalent to kn_subscribe; rid is a kn_route_location value and
// fxn is a handler to call for matching events; obj is a data object
// which will be passed to fxn as the second argument
function _kn_setHandler(rid, fxn, obj)
{
    kn._dispatchFunction[rid] = fxn;
    kn._dispatchPtr[rid] = kn._memorize(obj);

    // also tell the leader KN to route rid to our window IFF we are a
    // follower; assumes that no two followers can subscribe to the
    // *exact* same route id
    if (!kn._isLeaderP)
    {
        kn.leaderWindow.kn.dispatch_[rid] = kn._ownerWindow;
    }
}

// implementation of kn.clearHandler; this is a low-level
// local-only equivalent to kn_unsubscribe; rid is a kn_route_location
// value for which the handler is removed
function _kn_clearHandler(rid)
{
    kn._forget(kn._dispatchPtr[rid], "_kn_clearHandler");
    kn._dispatchFunction[rid] = null;
    delete kn._dispatchFunction[rid];
    if (!kn._isLeaderP)
    {
        kn.leaderWindow.kn.dispatch_[rid] = null;
        delete kn.leaderWindow.kn.dispatch_[rid];
    }
}

/*! default status handler for successful status events @tparam object e status event */
function kn_defaultOnSuccess(e)
{
    // do nothing
}

/*! default status handler for unsuccessful status events @tparam object e status event */
function kn_defaultOnError(e)
{
    if (_kn_debug()) //#
    { //#
        alert( //#
            kn_formatString( //#
                _kn_$("[KNJS:Dispatcher] Got an unhandled error status event: %{status}\n\n%{kn_payload}"), //#
                e //#
                ) //#
            ); //#
    } //#
}

/*! default status event handler; status handler object is passed as the (optional) second argument, or as "this" @tparam object e status event @tparam statusHandlers handler optional status handlers */
function kn_defaultOnStatus(e, handler)
{
    if (! handler) handler = this;
    var _l_code = e.status ? e.status.charAt(0) : null;
    if ((_l_code == "1") ||
        (_l_code == "2") ||
        (_l_code == "3"))
    {
        if (handler.onSuccess)
        {
            handler.onSuccess(e);
        }
        else
        {
            kn_defaultOnSuccess(e);
        }
    }
    else
    {
        if (handler.onError)
        {
            handler.onError(e);
        }
        else
        {
            kn_defaultOnError(e);
        }
    }
}

/*! invoke status handlers on optional handler object for status event e @tparam object e status event @tparam statusHandlers handler optional status handler object */
function kn_doStatus(e, handler)
{
    handler = handler ? handler : _kn_object();
    if (handler.onStatus)
    {
        handler.onStatus(e);
    }
    else
    {
        kn_defaultOnStatus(e, handler);
    }
}

function _l_setStatusHandler_oneshot(e, ctx)
{
    _kn_clearHandler(ctx.status_from);
    kn_doStatus(e, ctx.handler);
}

// register status handlers and return a new kn_route_location/kn_status_from
// string which can be used in later requests
function _kn_setStatusHandler(handler)
{
    var ctx = _kn_object(
        "handler", handler ? handler : _kn_object(),
        "status_from", (kn.tunnelURI +
                        '/kn_status/' +
                        Math.random().toString().substring(2, 10) +
                        '_' +
                        (kn._seqNum++))
        );
    _kn_setHandler(ctx.status_from,
                   _l_setStatusHandler_oneshot,
                   ctx);
    return ctx.status_from;
}

function _kn_matchPrefix(prefix, target)
{
    return target.length >= prefix.length &&
        target.substring(0, prefix.length) == prefix;
}

// "command-line arguments" can be passed in as urlencoded variables
// e.g.   ...foo?kn_debug=true;userVariable=value;...
// This is especially useful for passing in a default topic. In fact
// there is a special case of support for kn_topic as the following:
//        ...foo?topic/subtopic
// In particular, the lack of an equals sign is considered syntactic
// sugar: ...foo?kn_topic=topic/subtopic

function _kn_parseArgs()
{
    var _l_args = //_
        _kn_object();

    if (document.location.search || self.kn_queryString)
    {
        var _l_argString =
            self.kn_queryString ?
            kn_queryString :
            document.location.search.substring(1);
        var _l_pairs = null;
        if (_l_argString.indexOf(";") != -1)
            _l_pairs = _l_argString.split(";");
        else
            _l_pairs = _l_argString.split("&");

        for (var i=0; i < _l_pairs.length; i++)
        {
            var pos = _l_pairs[i].indexOf("=");
            if (pos != -1)
            {
                var _l_argname = //_
                    _l_pairs[i].substring(0, pos);
                var _l_argvalue = //_
                    _l_pairs[i].substring(pos+1);
// RFC1630: Within the query string, the plus sign is reserved as shorthand
// notation for a space.  Therefore, real plus signs must be encoded.
                _l_args[_kn_unescape(_l_argname)] = _kn_unescape(_l_argvalue);
            }
            else if (_l_pairs[i].length > 0) {
                // special cases for "parameters" with no "=" sign
                if (_kn_matchPrefix("kn_",_l_pairs[i]))
                    _l_args[_l_pairs[i]] = true;
                else
                    _l_args.kn_topic = _kn_unescape(_l_pairs[i]);
            }
        }
    }
    return _l_args;
}

/*! this resolves a relative path against an absolute base path (the absolute path should end with a slash!) @tparam string base absolute base path (must end with a slash!) @tparam string path relative path to resolve @treturn string absolute resolved version of relative path */
function kn_resolvePath(base,path)
{
    // if there is a leading slash, we return the path immediately
    if (path.charAt(0) == '/')
    {
        return path;
    }

    // otherwise, we concatenate the base and the relative path,
    // with an optional filename stripped from the base.
    var url = base;
    var end = url.lastIndexOf('/');
    if (end != -1)
    {
        url = url.substring(0, end) + '/';
    }
    url += path;

    // collapse repeated slashes
    var idx;
    while ((idx = url.indexOf('//')) != -1)
    {
        url =
            url.substring(0, idx) +
            url.substring(idx + 1);
    }

    // collapse foo/bar/../baz to foo/baz
    while ((end = url.indexOf('/../')) != -1 &&
           end > (idx = url.indexOf('/')))
    {
        url =
            url.substring(0, url.substring(0, end).lastIndexOf('/')) +
            url.substring(end + 3);
    }

    // temporarily cap the ends with slashes
    url = '/' + url + '/';

    // change any remaining /../ segments to /
    while ((idx = url.indexOf('/../')) != -1)
    {
        url =
            url.substring(0, idx) +
            url.substring(idx + 3);
    }

    // remove temporary end-caps
    url = url.substring(1, url.length - 1);

    return url;
}

// workaround for JavaScript implementations lacking
// String.fromCharCode; does not support multiple
// codes
function _kn_stringFromCharCode8(code)
{
    if (String.fromCharCode)
    {
        return String.fromCharCode(code);
    }

    if (code > 0 && code < 256)
    {
        return eval("'\\x" + code.toString(16) + "'");
    }

    // we replace the rest with 0xFF
    return '\xFF';
}

// workaround for JavaScript implementations lacking
// the charCodeAt method for String objects
function _kn_charCodeAt8(string, index)
{
    if (string.charCodeAt)
        return string.charCodeAt(index);
    // try to look it up...
    var _l_result = null;
    _l_result = _kn_codes8[string.charAt(index)];
    // if that fails, pretend it's the replacement character
    if (_l_result == null) _l_result = KN.ucsNoChar;
    return _l_result;
}

function _kn_hasProto(uri)
{
    var c = uri.indexOf(":");
    if (c < 1) return false;
    for (var i = 0; i < c; i ++)
    {
        var cc = uri.charAt(i);
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

function _kn_absoluteTopicURI(uri)
{
    var _l_server_uri = location.protocol + '//' + location.host + kn_server;
    if (_kn_hasProto(kn_server))
    {
        // we already had a complete server URI with a protocol
        _l_server_uri = kn_server;
    }
    if (_kn_hasProto(uri))
    {
        // complete URI with protocol
        return uri;
    }
    else if (uri.charAt(0) == '/')
    {
        // FIXME: in this case, uri is not really a relative URI. if
        // it were, /what/chat relative to server
        // http://10.10.10.30/kn/cgi-bin/kn.cgi would resolve to
        // http://10.10.10.30/what/chat, not (as it does)
        // http://10.10.10.30/kn/cgi-bin/kn.cgi/what/chat
        return _l_server_uri + uri;
    }
    else
    {
        return _l_server_uri + "/" + uri;
    }
}

// This returns a unique window name.
function _kn_uniqueWindowName()
{
    var _l_new_name = null;

    // Clean up the location text & use it with a random number.
    if (document && document.location &&
        document.location.pathname &&
       (document.location.pathname.length > 0))
    {
        _l_new_name = document.location.pathname;

        // trim trailing slashes
        while (_l_new_name.charAt(_l_new_name.length - 1) == '/')
        {
            _l_new_name = _l_new_name.substring(0, _l_new_name.length - 1);
        }

        // trim everything except the last component
        var idx = _l_new_name.lastIndexOf('/');
        if (idx != -1)
        {
            _l_new_name = _l_new_name.substring(idx + 1);
        }

        // replace non-alphanumeric characters with underscore
        for (var i = 0; i < _l_new_name.length; i ++)
        {
            var cc = _l_new_name.charAt(i);
            if (! (
                cc >= "0" && cc <= "9"
                ||
                cc >= "A" && cc <= "Z"
                ||
                cc >= "a" && cc <= "z"
                ))
            {
                _l_new_name =
                    _l_new_name.substring(0, i) +
                    '_' +
                    _l_new_name.substring(i + 1);
            }
        }
    }
    if (! _l_new_name)
    {
        _l_new_name = Math.random().toString().substring(2,10);
    }
    _l_new_name =
        "kn_" +
        _l_new_name +
        "_" +
        Math.random().toString().substring(2, 7);
    return _l_new_name;
}

function kn__uniqueWindowName()
{
    return _kn_uniqueWindowName();
}

// anException = kn__try(aClosure [ , aClosureArg [, aClosureArg ... ]
// ] ) - invoke a closure (aClosure) - - the first argument - - with
// the specified arguments - - the remaining arguments. If an
// exception is raised during execution of the closure, kn__try()
// attempts to catch and return the exception object (anException); in
// browsers with missing or incomplete try/catch support, this is
// similar to eval('aClosure(aClosureArg, aClosureArg, ...)')

/*! invoke a closure (aClosure) with the specified arguments. If an exception is raised during execution of the closure, kn__try() attempts to catch and return the exception object (anException); in browsers with missing or incomplete try/catch support, this is similar to eval('aClosure(aClosureArg, aClosureArg, ...)') @tparam function aClosure closure to execute @treturn object exception generated, or undefined */
function kn__try(aClosure /* [ , aClosureArg [ , aClosureArg ... ] ] */)
{
    var aClosureArgs = new Array;
    var aClosureArgStrings = new Array;
    for (var anIndex = 1; anIndex < arguments.length; anIndex ++)
    {
        var aClosureArgsIndex = aClosureArgs.length;
        aClosureArgStrings[aClosureArgStrings.length] =
            "aClosureArgs[" + aClosureArgsIndex + "]";
        aClosureArgs[aClosureArgsIndex] = arguments[anIndex];
    }
    var anException;
    var aClosureString = "aClosure(" + aClosureArgStrings.join(", ") + ");";
    if (typeof Error != 'undefined' ||
        (navigator.appVersion.indexOf('MSIE ') != -1 &&
         navigator.platform == 'MacPPC'))
    {
        eval('try { ' + aClosureString + ' } catch (x) { anException = x; }');
    }
    else
    {
        eval(aClosureString);
    }            
}

/*! This tests a potentially lethal radioactive Window handle @tparam Window w window handle to test @treturn boolean true if w is safe to access */
function kn_isReady(w)
{
    var ctx = new Object;
    ctx.wNameType = "unknown";
    var testClosure =
        new Function('ctx', 'w',
                     'if (w!=null)' +
                     '{' +
                     /**/'ctx.wNameType=typeof(w["name"]);' +
                     '}'
            );
    kn__try(testClosure, ctx, w);
    return ((w != null) &&
            (ctx.wNameType != "unknown") &&
            (w.closed != null) &&
            ((typeof w.closed) == "boolean") &&
            (!w.closed));
}

// Our default error handler attempts to be as subtle as possible
function _kn_lastErrorHandler(message, url, line)
{
    if (this.kn)
    {
        kn.lastError = _kn_object();
        kn.lastError.message=message;
        kn.lastError.url=url;
        kn.lastError.line=line;
        kn.lastError.date=new Date();
    }
    else
    {
        return false;
    }
    return !_kn_debug(); // if debugging is off, return true silently
}

// encoded = kn_encodeRequest(e)
// given an event e, kn_encodeRequest returns the corresponding
// application/x-www-form-urlencoded string

/*! given an event e, kn_encodeRequest returns the corresponding application/x-www-form-urlencoded string @tparam object e request to encode @treturn string encoded version of request */
function kn_encodeRequest(e)
{
    var _l_request = //_
        new Array();
    for (var key in e)
    {
        if (e[key] != null)
        {
            // flatten arrays
            if (_kn_isArray(e[key]))
            {
                for (var i = 0; i < e[key].length; i ++)
                {
                    var val = e[key][i];
                    if (typeof(val) != "string")
                    {
                        // for nested batches
                        val = kn_encodeRequest(val);
                    }
                    _l_request[_l_request.length] = _kn_escape(key) +
                        '=' + _kn_escape(val);
                }
            }
            else if (typeof(e[key].toString) == "function")
            {
                _l_request[_l_request.length] = _kn_escape(key) +
                    '=' + _kn_escape(e[key]);
            }
        }
    }
    return _l_request.join(';');
}

function _l_htmlEscape_converter(c, tc)
{
    if ((c < 32) || (c > 126) || (tc == '\'')) return '&' + '#' + c + ';';
    else if (tc == '&') return '&' + "amp;";
    else if (tc == '\"') return '&' + "quot;";
    else if (tc == '<') return '&' + "lt;";
    else if (tc == '>') return '&' + "gt;";
    else return tc;
}

/*! replace unsafe characters with character entities. makes any string safe for doc.write() as an HTML attribute or textarea. does UTF-16 decoding. @tparam string t free-form string @treturn string safely encoded HTML version of the input */
function kn_htmlEscape(t)
{
    return _kn_utf16decode(t, _l_htmlEscape_converter);
}

function _kn_isxdigit(s)
{
    return KN.hexDigits.indexOf(s) != -1;
}

// implementation of kn_unescape()
function _kn_unescape(value)
{
    // stringify the input
    value = '' + value;
    if (_kn_options("unescape"))
        return unescape(value);
    // decode URL-encoding
    var _l_decoded = '';
    for (var i = 0; i < value.length; i ++)
    {
        var ch = value.charAt(i);

        // decode +-encoded spaces
        if (ch == '+') ch = ' ';

        // decode %-hex-hex-encoded bytes
        var c0;
        var c1;
        if (((i + 2) < value.length) &&
            (ch == '%') &&
            _kn_isxdigit((c0 = value.charAt(i+1))) &&
            _kn_isxdigit((c1 = value.charAt(i+2))))
        {
            var c = parseInt(c0 + c1, 16);
            ch = _kn_stringFromCharCode8(c);
            i += 2;
        }
        _l_decoded += ch;
    }
    // recode from UTF-8 to UTF-16
    return '' + _kn_utf8decode(_l_decoded);
}

/*! convert a string from URL-encoded UTF-8 to UTF-16 @tparam string value UTF-8-encoded, application/x-www-form-urlencoded string @treturn string decoded UTF-16 string */
function kn_unescape(value)
{
    return _kn_unescape(value);
}

// internal version of kn_stringFromCharCode
function _kn_stringFromCharCode() // codes ...
{
    return _kn_stringFromCharCodes(arguments);
}

// implementation of kn_stringFromCharCodes
function _kn_stringFromCharCodes(codes)
{
    var _l_result = new Array();
    for (var i = 0; i < codes.length; i++)
    {
        var _l_code = //_
            parseInt(codes[i]);
        // pass UCS-2 unmodified [except for reserved surrogate codepoints]
        if ((_l_code >= 0) && (_l_code <= KN.ucs2max))
        {
            if (_l_code >= KN.utf16firstLowHalf &&
                _l_code <= (KN.utf16firstHighHalf | KN.utf16mask))
            {
                // reserved surrogate codepoint
                _l_code = KN.ucsNoChar;
            }
            _l_result[_l_result.length] = _kn_stringFromCharCode8(_l_code);
        }
        // encode other UTF-16 characters using surrogate pairs
        else if ((_l_code > KN.ucs2max) && (_l_code <= KN.utf16max))
        {
            _l_code -= KN.utf16offset;
            _l_result[_l_result.length] =
                _kn_stringFromCharCode8(KN.utf16firstLowHalf |
                                        (_l_code >> KN.utf16shift)) +
                _kn_stringFromCharCode8(KN.utf16firstHighHalf |
                                        (_l_code & KN.utf16mask));
        }
        // anything else is turned into the replacement character
        else
        {
            _l_result[_l_result.length] = _kn_stringFromCharCode8(KN.ucsNoChar);
        }
    }
    return _l_result.join("");
}

// this function is an implementation of String.fromCharCode for
// UTF-16 JavaScript implementations; the resulting string may be
// longer than the argument list due to UTF-16 surrogate pair encoding
function kn_stringFromCharCode() // codes ...
{
    return _kn_stringFromCharCodes(arguments);
}

// implementation of kn_utf8decode
function _kn_utf8decode(string)
{
    // stringify the input
    string = '' + string;
    // overall result
    var s = '';
    // residual character value from earlier bytes of a multibyte sequence
    var _l_code = 0;
    // minimum allowed character value for a multibyte sequence
    var _l_mincode = 0;
    // number of continuation bytes expected
    var _l_expected_bytes = 0;
    for (var i = 0; i < string.length; i ++)
    {
        var c = _kn_charCodeAt8(string, i);
        var ch = string.charAt(i);

        // binary pattern 0xxx xxxx is unchanged
        if ((c >= 0) && (c < 0x80))
        {
            if (_l_expected_bytes)
                s += _kn_stringFromCharCode(KN.ucsNoChar);
            _l_expected_bytes = 0;
            s += _kn_stringFromCharCode(c);
        }
        // binary pattern 10xx xxxx continues a UTF-8 sequence
        else if ((c & 0xC0) == 0x80)
        {
            if (_l_expected_bytes)
            {
                _l_code = (_l_code << KN.utf8shift) | (c & KN.utf8mask);
                _l_expected_bytes = _l_expected_bytes - 1;
                if (! _l_expected_bytes)
                {
                    // overlong sequences get squashed here
                    if (_l_code < _l_mincode)
                        _l_code = KN.ucsNoChar;
                    s += _kn_stringFromCharCode(_l_code);
                }
            }
            else
                s += _kn_stringFromCharCode(KN.ucsNoChar);
        }
        // binary pattern 110x xxxx introduces a two-byte UTF-8 sequence
        else if ((c & 0xE0) == 0xC0)
        {
            if (_l_expected_bytes)
                s += _kn_stringFromCharCode(KN.ucsNoChar);
            _l_mincode = 0x80;
            _l_expected_bytes = 1;
            _l_code = c & (KN.utf8mask >> _l_expected_bytes);
        }
        // binary pattern 1110 xxxx introduces a three-byte UTF-8 sequence
        else if ((c & 0xF0) == 0xE0)
        {
            if (_l_expected_bytes)
                s += _kn_stringFromCharCode(KN.ucsNoChar);
            _l_mincode = 0x800;
            _l_expected_bytes = 2;
            _l_code = c & (KN.utf8mask >> _l_expected_bytes);
        }
        // binary pattern 1111 0xxx introduces a four-byte UTF-8 sequence
        else if ((c & 0xF8) == 0xF0)
        {
            if (_l_expected_bytes)
                s += _kn_stringFromCharCode(KN.ucsNoChar);
            _l_mincode = 0x10000;
            _l_expected_bytes = 3;
            _l_code = c & (KN.utf8mask >> _l_expected_bytes);
        }
        // binary pattern 1111 10xx introduces a five-byte UTF-8 sequence
        else if ((c & 0xFC) == 0xF8)
        {
            if (_l_expected_bytes)
                s += _kn_stringFromCharCode(KN.ucsNoChar);
            _l_mincode = 0x200000;
            _l_expected_bytes = 4;
            _l_code = c & (KN.utf8mask >> _l_expected_bytes);
        }
        // binary pattern 1111 110x introduces a six-byte UTF-8 sequence
        else if ((c & 0xFE) == 0xFC)
        {
            if (_l_expected_bytes)
                s += _kn_stringFromCharCode(KN.ucsNoChar);
            _l_mincode = 0x4000000;
            _l_expected_bytes = 5;
            _l_code = c & (KN.utf8mask >> _l_expected_bytes);
        }
        // something illegal this way comes...
        else
        {
            if (_l_expected_bytes)
                s += _kn_stringFromCharCode(KN.ucsNoChar);
            _l_expected_bytes = 0;
            s += _kn_stringFromCharCode(KN.ucsNoChar);
        }
    }
    if (_l_expected_bytes)
        s += _kn_stringFromCharCode(KN.ucsNoChar);
    return '' + s;
}

// decode a UTF-8 string into UTF-16
function kn_utf8decode(string)
{
    return _kn_utf8decode(string);
}

// Given a UTF-16 string, this function returns an array containing
// the corresponding UCS character codes.
function kn_charCodesFromString(string)
{
    var ptr = kn._memorize(new Array());
    var _l_converter = //_
        new Function('_l_code', //_
                     'var _l_codes = ' + //_
                     /**/'kn._recall(' + ptr +', "kn_charCodesFromString(1)");' +
                     '_l_codes[_l_codes.length] = _l_code;'
            );
    _kn_utf16decode(string, _l_converter);
    return kn._forget(ptr, "kn_charCodesFromString(2)");
}

// this function decodes a UTF-16 string and calls an optional conversion
// routine for each UCS character; this converter is passed a UCS value and
// the corresponding substring for each character, and returns a string
// which will be appended to the overall result
function _kn_utf16decode(string, converter)
{
    var s = new Array();
    string = '' + string;
    for (var i = 0; i < string.length; i ++)
    {
        var _l_code = //_
            _kn_charCodeAt8(string, i);
        var ch = string.charAt(i);
        if (! ((_l_code >= 0) && (_l_code <= KN.ucsMaxChar)))
        {
            _l_code = KN.ucsNoChar;
            ch = _kn_stringFromCharCode8(_l_code);
        }
        // decode surrogate pairs between U+D800 and U+DFFF
        // we do not bother to handle unpaired surrogates; app can do that
        if (((_l_code & ~KN.utf16mask) == KN.utf16firstLowHalf) &&
            ((i + 1) < string.length))
        {
            var _l_ccode = //_
                _kn_charCodeAt8(string, i + 1);
            var cch = string.charAt(i + 1);
            if ((_l_ccode & ~KN.utf16mask) == KN.utf16firstHighHalf)
            {
                i ++;
                _l_code = _l_code & KN.utf16mask;
                _l_ccode = _l_ccode & KN.utf16mask;
                _l_code = ((_l_code << KN.utf16shift) | _l_ccode) + KN.utf16offset;
                ch += cch;
            }
        }
        // by default, the converter produces UTF-16
        s[s.length] = converter ? converter(_l_code, ch) : ch;
    }
    return s.join("");
}

function _l_escape_quote(n)
{
    return ('%' +
            KN.hexDigits.charAt(n >> 4) +
            KN.hexDigits.charAt(n & 15));
}

function _l_escape_converter( //_
    _l_code, ch)
{
    if ((_l_code < 0x20) ||
        (ch == '+') ||
        (ch == '/') ||
        (_l_code == 0x7F))
    {
        return _l_escape_quote(_l_code);
    }
    else if (_l_code < 0x80)
    {
        return escape(ch);
    }
    else if (_l_code < 0x800)
    {
        return (_l_escape_quote(0xC0 | ((_l_code >> 6) & 0x1f)) +
                _l_escape_quote(0x80 | (_l_code & 0x3f)));
    }
    else if (_l_code < 0x10000)
    {
        return (_l_escape_quote(0xE0 | ((_l_code >> 12) & 0xf)) +
                _l_escape_quote(0x80 | ((_l_code >> 6) & 0x3f)) +
                _l_escape_quote(0x80 | (_l_code & 0x3f)));
    }
    else if (_l_code < 0x200000)
    {
        return (_l_escape_quote(0xF0 | ((_l_code >> 18) & 0x7)) +
                _l_escape_quote(0x80 | ((_l_code >> 12) & 0x3f)) +
                _l_escape_quote(0x80 | ((_l_code >> 6) & 0x3f)) +
                _l_escape_quote(0x80 | (_l_code & 0x3f)));
    }
    else if (_l_code < 0x4000000)
    {
        return (_l_escape_quote(0xF8 | ((_l_code >> 24) & 0x3)) +
                _l_escape_quote(0x80 | ((_l_code >> 18) & 0x3f)) +
                _l_escape_quote(0x80 | ((_l_code >> 12) & 0x3f)) +
                _l_escape_quote(0x80 | ((_l_code >> 6) & 0x3f)) +
                _l_escape_quote(0x80 | (_l_code & 0x3f)));
    }
    else
    {
        return (_l_escape_quote(0xFC | ((_l_code >> 30) & 0x1)) +
                _l_escape_quote(0x80 | ((_l_code >> 24) & 0x3f)) +
                _l_escape_quote(0x80 | ((_l_code >> 18) & 0x3f)) +
                _l_escape_quote(0x80 | ((_l_code >> 12) & 0x3f)) +
                _l_escape_quote(0x80 | ((_l_code >> 6) & 0x3f)) +
                _l_escape_quote(0x80 | (_l_code & 0x3f)));
    }
}

// implementation of kn_escape()
function _kn_escape(value)
{
    value = '' + value;
    if (_kn_options("escape"))
        return escape(value);
    var _l_result = //_
        _kn_utf16decode(value, _l_escape_converter);
    return _l_result;
}

/*! convert a string from UTF-16 to URL-encoded UTF-8 @treturn string UTF-8-encoded, application/x-www-form-urlencoded string @tparam string value decoded UTF-16 string */
// convert a string from UTF-16 to URL-encoded UTF-8
function kn_escape(value)
{
    return _kn_escape(value);
}

function _kn_clearCookie(s)
{
    if (!s) s = _kn_escape(kn.TFN_);
    // clean up after ourselves
    // forcing the cookie to be erased, even if the browser session persists
    var _l_expDate = new Date((new Date()).getTime(0));
    document.cookie = s + "=THIS_IS_AN_EX_COOKIE;expires=" +
        _l_expDate.toGMTString() + ";path=/";
}

// interpolate bits of an object into a string in place of %{names};
// e.g. kn_formatString("%{name} is %{emotion}",
//                      {name: 'Ben', emotion: 'jubilant'})
// returns "Ben is jubilant". Use '%%' to include a literal '%'.
// names cannot include the '}' character.
function kn_formatString(format, args)
{
    var _l_result = '';
    var _l_normal = true;
    while (1)
    {
        var pos = format.indexOf(_l_normal ? '%' : '}');
        if (pos == -1) return _l_result + (_l_normal ? format : args[format]);
        var _l_match = format.substring(0, pos);
        format = format.substring(pos + 1);
        _l_result += (_l_normal ? _l_match : args[_l_match]);
        if (_l_normal)
        {
            var _l_first = format.substring(0,1);
            format = format.substring(1);
            if (_l_first == '{')
                _l_normal = false;
            else
                _l_result += _l_first;
        }
        else
        {
            _l_normal = true;
        }
    }
    // This is to help foolish JavaScript interpreters which can not
    // understand that this code is unreachable...
    return _l_result;
}

// localize string s to ls in context c for language l, provided it has not already
// been localized; a null ls is equivalent to s
function kn_localizeDefault(l, c, s, ls)
{
    if (! kn_strings[l])
        kn_strings[l] = _kn_object();
    if (! kn_strings[l][c])
        kn_strings[l][c] = _kn_object();
    if (! kn_strings[l][c][s])
        kn_strings[l][c][s] = (ls != null) ? ls : s;
}

// special kn_localizeDefault for the "kn" context
function _kn_localizeDefault(l, s, ls)
{
    kn_localizeDefault(l, "kn", s, ls);
}

// localize string s to ls in context c for language l; a null ls is equivalent to s
function kn_localize(l, c, s, ls)
{
    if (! kn_strings[l])
        kn_strings[l] = _kn_object();
    if (! kn_strings[l][c])
        kn_strings[l][c] = _kn_object();
    kn_strings[l][c][s] = (ls != null) ? ls : s;
}

// special kn_localize for the "kn" context
function _kn_localize(l, s, ls)
{
    kn_localize(l, "kn", s, ls);
}

// provide a localized version of string s in context c
function $$(c, s)
{
    if (! kn_lang)
        return s;
    var ls = s; // localized strings
    var q = 0;
    var _l_langs_split = kn_lang.split(',');
    for (var _l_inexact = //_
             0; _l_inexact < 2; _l_inexact ++)
        for (var i = 0; i < _l_langs_split.length; i ++)
        {
            var _l_lang = //_
                _l_langs_split[i].toLowerCase();
            var lq = 1; // qvalue for this language range
            while (_l_lang.substring(0, 1) == ' ') _l_lang = _l_lang.substring(1);
            var _l_lang_split = //_
                _l_lang.split(';q=');
            _l_lang = _l_lang_split[0];
            if (_l_lang_split.length > 1) lq = new Number(_l_lang_split[1]);
            if (lq > q)
            {
                if (_l_lang == '*')
                {
                    ls = s;
                    q = lq;
                    continue;
                }

                for (var _l_llang in kn_strings)
                {
                    var _l_llang_lc = //_
                        _l_llang.toLowerCase();
                    if ((_l_lang == _l_llang_lc) ||
                        _l_inexact &&
                        (_l_llang_lc.substring(0, _l_lang.length + 1) == (_l_lang + "-")))
                    {
                        if (kn_strings[_l_llang][c] &&
                            kn_strings[_l_llang][c][s])
                        {
                            ls = kn_strings[_l_llang][c][s];
                            q = lq;
                        }
                    }
                }
            }
        }
    return ls;
}

// provide a localized version of string s in the "kn" context
function _kn_$(s)
{
    return $$("kn", s);
}

// provide a localized kn_formatString for string s in context c;
// equivalent to kn_formatString($$(c, s), [ args ... ]), so args
// must be referred to using numeric offsets in s. The first arg
// is %{0}, the second %{1}, etc.
function $$_(c, s) //, args ...
{
    var _l_args = new Array(arguments.length - 2);
    for (var i = 0; i < _l_args.length; i ++)
    {
        _l_args[i] = arguments[i + 2];
    }
    return kn_formatString($$(c, s), _l_args)
}

// a version of $$_ for the "kn" context
function _kn_$_(s) //, args ...
{
    var _l_args = new Array(arguments.length - 1);
    for (var i = 0; i < _l_args.length; i ++)
    {
        _l_args[i] = arguments[i + 1];
    }
    return kn_formatString($$("kn", s), _l_args)
}

// create helpers for a localization context
function kn_createContext(c)
{
    this[c + "$"] =
        new Function('s',
                     'return $$(_kn_unescape("' + _kn_escape(c) + '"), s);'
            );
    this[c + "$_"] =
        new Function('s', //, args ...
                     'var _l_args =' + //_
                     'new Array(arguments.length - 1);' +
                     'for (var i = 0; i < _l_args.length; i ++)' +
                     '{' +
                     /**/'_l_args[i] = arguments[i + 1];' +
                     '}' +
                     'return kn_formatString($$(_kn_unescape("' + _kn_escape(c) + '"), s), _l_args);'
            );
}

// create helpers for the empty context ($ and $_)
kn_createContext("");

// CONVENIENCE FUNCTIONS FOR DEBUGGING

// this changes the value of the kn_debug=... parameter to the
// specified value; an omitted value or true enables general
// debugging; an empty string or false disables all debugging; any
// other string enables general debugging and is treated as a
// comma-separated list of specific debugging options to enable (the
// special debugging option "all" enables all debugging);
// additionally, debug() hides or unhides the tunnel and post frames
// to reflect the value of general debugging.
function kn_debug(flags)
{
    if ((flags == null) && (typeof(flags) == 'undefined')) //#
        kn._debug = true; //#
    else //#
        kn._debug = flags; //#
    if (kn_isReady(kn._ownerWindow) && //#
        kn._ownerWindow.document && //#
        kn._ownerWindow.document.all) //#
    { //#
        var esp = kn._ownerWindow.document.all[kn._ownerWindow.name + "_esp"]; //#
        if (esp) esp.rows = //#
            (_kn_debug() ? (_kn_debug("posts") ? '70%,30%' : '90%,10%') : '100%,*'); //#
    } //#
}

// like kn_debug(), but for general options instead of debugging options
function kn_options(flags)
{
    if ((flags == null) && (typeof(flags) == 'undefined'))
        kn._options = true;
    else
        kn._options = flags;
}

// abbreviation for kn_options(flags)
function kn_opts(flags)
{
    return kn_options(flags);
}

// deprecated, but we'll still have to allow it for the forseeable future
function kn_hacks(flags)
{
    return kn_options(flags);
}

function kn_inspectAsText (target, depth, prefix)
{
    if (prefix == null) prefix = "";
    if (depth == null) depth = 1;
    if (target == null) return "null";
    var y = target + "{";
    for (var x in target)
    {
    y +=
        "\n " + prefix + x + " = " + "(" + typeof(target[x]) + ")" +
        (((depth > 1) && ((typeof target[x]) == "object"))
         ? kn_inspectAsText (target[x], depth - 1, prefix + " ")
         : ("" + target[x]).split("\n").join("\n " + prefix));
    }

    y += "\n" + prefix + "}";
    return y;
}

function kn_inspectAsHTML (target, depth)
{
    if (depth == null) depth = 1;
    if (target == null) return "null";
    var y = "\n<dl>";
    for (var x in target)
    {
        y +=
            "\n<dt><b>" +
            kn_htmlEscape(x) +
            "<" + "/b>: <i>" +
            kn_htmlEscape(typeof(target[x])) +
            "<" + "/i>\n<dd>" +
            (((depth > 1) && ((typeof target[x]) == "object"))
             ? kn_inspectAsHTML (target[x], depth - 1)
             : ("<pre>" + kn_htmlEscape(target[x]) + "<" + "/pre>"));
    }
    y += "\n<" + "/dl>";
    return y;
}

function kn_inspectInWindow(target, depth)
{
    var f = self.open(kn_blank,"_blank","height=200,width=400,scrollbars=yes,resizable=yes");
    f.document.open("text/html");
    f.document.write(kn_inspectAsHTML(target, depth));
    f.document.close();
}

////////////////////////////////////////////////////////////////////
// INTER-WINDOW API
//
// These are used for communication across window and frame
// boundaries, where different microserver versions may be present.

/*! submit (or possibly batch) a KN server request returns false until the request has been sent (or at least batched) @tparam object e request to submit */
function kn__submitRequest(e)
{
    return _kn_submitRequest(e);
}

/*! restart the tunnel connection */
function kn__restartTunnel()
{
    return _kn_restartTunnel();
}

/*! create an object from a list of names and values; substitute for JavaScript 1.2 object literals; since this is used to pass events across window boundaries [during escrow] it has an "internal-but-visible" name. @treturn object an object with properties corresponding to the names and values in the argument list */
function kn__object() // ...
{
    return _kn_object2(arguments);
}

/*! handle dispatches from the leader; all parts of the inter-window API need to be uncompressed [and preferably backwards-compatible, in the future] for the sake of interoperability */
function kn__consumeEvents()
{
    while (kn.leaderWindow.kn__queue &&
           kn.leaderWindow.kn__queue[kn.TFN_])
    {
        // dequeue
        var q = kn.leaderWindow.kn__queue[kn.TFN_];
        kn.leaderWindow.kn__queue[kn.TFN_] = q.next;

        // shallow-copy the event to avoid radioactivity
        var evt = _kn_object();

        for (var header in q.event)
        {
            evt[header] = q.event[header];
        }

        // dispatch it
        kn__routeEvent(evt);
    }
}

/*! this internal function checks for the presence of a particular word in the comma-separated kn_options=... value; it returns true if the flag is present, and false otherwise; the special flag kn_options=all causes _kn_options to return true for all flags. If no flag is specified, _kn_options checks for the presence of the kn_options parameter. @tparam string flag word to scan the kn_options list for @treturn boolean true if the named flag is set */
function kn__options(flag)
{
    return _kn_options(flag);
}

/*! deprecated, but we'll still have to allow it for the forseeable future @see kn__options() @tparam string flag word to scan the kn_options list for @treturn boolean true if the named flag is set */
function kn__hacks(flag)
{
    return kn__options(flag);
}

/*! this internal function checks for the presence of a particular word in the comma-separated kn_debug=... value; it returns true if the flag is present, and false otherwise; the special flag kn_debug=all causes _kn_debug to return true for all flags. If no flag is specified, _kn_debug checks for the presence of the kn_debug parameter. @tparam string flag word to scan the kn_debug list for @treturn boolean true if the named flag is set */
function kn__debug(flag)
{
    return _kn_debug(flag);
}

/*! this is a low-level local-only equivalent to kn_publish(); it uses the event's kn_route_location header for dispatching @tparam object e event to route */
function kn__routeEvent(e)
{
    // this function supports the 'single' and 'noforward' options
    var rid = e.kn_route_location;
    if (_kn_debug("events")) //#
    { //#
        alert(_kn_$("[KNJS:Transport] Got an event:\n") + //#
              'kn_id: ' + e.kn_id + '\n' + //#
              'kn_route_location: ' + e.kn_route_location + '\n' + //#
              'status: ' + e.status + '\n' + //#
              '\n' + e.kn_payload); //#
    } //#

    if (rid)
    {
        // if this an off-"host" post to another window..
        var rw = kn.dispatch_[rid];
        // First, is this a local function invocation?
        if ((('' + kn._dispatchFunction[rid]) != '') &&
            (kn._dispatchFunction[rid] != null))
        {
            (kn._dispatchFunction[rid])(e, kn._recall(kn._dispatchPtr[rid], "kn__routeEvent"));
        }
        else if ((rw != null) && (rw != kn._appFrame) && kn_isReady(rw))
        {
            // else, forward the event to the remote router

            // enqueue
            if (! kn.leaderWindow.kn__queue)
            {
                kn.leaderWindow.kn__queue = _kn_object();
            }
            var o = _kn_object('event', e, 'next', null);
            var tail = kn.leaderWindow.kn__queue[rw.kn.TFN_];
            if (! tail)
            {
                kn.leaderWindow.kn__queue[rw.kn.TFN_] = o;

                // a new queue needs a consumer

                // Netscape Navigator 4.x has a serious setTimeout bug, so we
                // act immediately there; other browsers (Netscape
                // 6/Mozilla, at least) need to do this from a
                // setTimeout to avoid occasional JavaScript errors
                if ((navigator.appVersion.charAt(0) == '4') &&
                    (navigator.appVersion.indexOf("MSIE ") == -1))
                {
                    rw.kn__consumeEvents();
                }
                else
                {
                    rw.setTimeout('kn__consumeEvents()',1);
                }
            }
            else
            {
                while (tail.next) tail = tail.next;
                tail.next = o;
            }
        }
        else
        {
            if (kn.ownerWindow.kn_defaultOnMessage)
            {
                kn.ownerWindow.kn_defaultOnMessage(e);
            }
            else
            {

                if (_kn_debug("routes")) //#
                { //#
                    alert( //#
                        _kn_$_("[KNJS:Dispatcher] Removing stale subscription:\nkn_route_location: %{0}", //#
                               e.kn_route_location //#
                            ) //#
                        ); //#
                } //#

                // this is an orphan event, and should be unsubscribed from
                {
                    // ... but only if it looks like a valid route URI
                    var _l_end_of_topic = e.kn_route_location.lastIndexOf("/kn_routes/");

                    if (_l_end_of_topic != -1)
                    {
                        kn_unsubscribe(e.kn_route_location);
                    }
                }
            }
        }
    }
    else
    {
        if (_kn_options("single") && _kn_options("noforward"))
        {
            if (_kn_debug() && ! _kn_options("quiet")) //#
                alert( //#
                    _kn_$("[KNJS:Dispatcher] Got an unlabeled event, assuming post frame success.") //#
                    ); //#
            kn._onPostSuccess(e);
        }
        else
        {
            if (kn.ownerWindow.kn_defaultOnMessage)
            {
                kn.ownerWindow.kn_defaultOnMessage(e);
            }
            else
            {
                if (_kn_debug()) //#
                    alert( //#
                        _kn_$("[KNJS:Dispatcher] Got an unlabeled event, discarding it.") //#
                        ); //#
            }
        }
    }
}

/*! called in new follower windows by the leader window @tparam Window _l_window new leader window @tparam string _l_tunnelURI leader's journal path @tparam string _l_lastTag leader's curent kn_route_checkpoint */
function kn__follow(
    _l_window, //_
    _l_tunnelURI, //_
    _l_lastTag)
{
    if (kn._leaderScannerTimer)
    {
        kn._ownerWindow.kn._clearInterval(kn._leaderScannerTimer);
        kn._leaderScannerTimer = null;
    }
    if (kn._isLeaderP)
    {
        if (_kn_debug() && kn_isReady(kn.tunnelFrame_)) kn.tunnelFrame_.document.bgColor="seagreen"; //#
        // a special case to send in-progress route table entries to the new leader. Very rare.
        _kn_escrow(kn, _l_window.kn);
    }
    //kn.tunnelFrame_.document.close(); // dicey; will this be too enthusiastic to kill itself?
    if (_kn_debug() && !_l_tunnelURI) //#
    { //#
        alert( //#
            _kn_$_("[KNJS:IPC] New leader %{0} had no tunnel; using my %{1}#%{2}", //#
                   _l_window.name, //#
                   kn.tunnelURI, //#
                   kn.lastTag_ //#
                ) //#
            ); //#
    } //#
    if (_l_tunnelURI)
    {
        kn.tunnelURI = _l_tunnelURI;
        kn.lastTag_ = _l_lastTag;
    }
    kn.leaderWindow = _l_window;
    kn._processWorkQ();
}

////////////////////////////////////////////////////////////////////
// FUNCTION WRAPPERS FOR PUB/SUB METHODS
//
// these exist to appease old-school javascript programmers who do
// not like method syntax

/*! @see kn.subsribe */
function kn_subscribe(kn_from, kn_to, kn_options, handler)
{
    return kn.subscribe(kn_from, kn_to, kn_options, handler);
}

/*! @see kn.publishForm */
function kn_publishForm(kn_to, theForm, handler)
{
    return kn.publishForm(kn_to, theForm, handler);
}

/*! @see kn.publish */
function kn_publish(kn_to, event, handler)
{
    return kn.publish(kn_to, event, handler);
}

/*! @see kn.unsubsribe */
function kn_unsubscribe(rid, handler)
{
    return kn.unsubscribe(rid, handler);
}

////////////////////////////////////////////////////////////////////
// EVENT NOTIFICATION SERVICE CALLBACKS
//
// These callbacks are invoked by pages returned by mod_pubsub

/*! callback for event delivery, as defined in the PubSub Protocol document; accepts events in a "form" style, and converts them to simple Objects @tparam object theEvent event to deliver, in "form" style @tparam Window theWindow originating frame (optional) */
function kn_sendCallback(theEvent, theWindow)
{
    return kn._sendCallback(theEvent, theWindow);
}

/*! called from kn_blank, and used for kn.documents and KNDocument @tparam Window w the originating frame */
function kn_redrawCallback(w)
{
    if (w['document'] && w.document['write'])
    {
        if (kn.documents[w.name])
        {
            kn.documents[w.name].state = "drawing";
            var str = kn.documents[w.name].html;

            if (kn.documents[w.name].kn_onLoad){
                str +=
                    "<" + "script type=\"text/javascript\">\n<" + "!-" + "-\n" +

                    // the string on the next line has a leading space
                    // to fool the compressor
                    " _kn_native_onload = self.onload;" +

                    "onload=new Function('" +

                    // make sure the documents cache entry still
                    // exists and has a kn_onLoad method before
                    // invoking the kn_onLoad.

                    "if (parent != null &&" +
                    " typeof(parent.name) != \"unknown\" &&" +
                    " parent.closed != null &&" +
                    " typeof(parent.closed) == \"boolean\" &&" +
                    " ! parent.closed &&" +
                    " parent.kn &&" +
                    " parent.kn.documents &&" +
                    " parent.kn.documents[self.name] &&" +
                    " parent.kn.documents[self.name].kn_onLoad)" +
                    "{" +
                    /**/"parent.kn.documents[self.name].kn_onLoad();" +
                    "}" +
                    // there is some weird timing issue to be worked out here
                    "if (_kn_native_onload) _kn_native_onload();" +
                    "');" +
                    "// -" + "->\n<" + "/script>";
            }
            w.document.write(str);
            kn.documents[w.name].state = "ready";
        }
        else
        {
            w.document.write('<body bgcolor="white"><' + '/body>');
        }
    }
}

/*! triggered by a tunnel frame's onLoad */
function kn_tunnelLoadCallback(theWindow)
{
    return kn._tunnelLoadCallback(theWindow);
}

//kn = _kn_initMicroserver(true);

////////////////////////////////////////////////////////////////////
// HISTORY
//
// These are brief historical notes on some of the software which lead
// to the current PubSub JavaScript Library.

//////// KNSERV
// Who: Peyman Oreizy, Rohit Khare and Adam Rifkin (the three founders)
// What: KnowNow's first event router
// When: early 2000 - June 2000
// Where: Peyman's basement, and at KnowNow in Seattle
// ------ The KnowNow Seattle office was on the fourth floor of a
//        converted early-20th century warehouse building across the
//        street from the SAFECO Field baseball stadium, home of the
//        Seattle Mariners. The building was seriously damaged during
//        the 2001 Seattle earthquake, and has since been
//        condemned. Even before the earthquake, one could see
//        sunlight through the cracks in the brick walls.
//
// In the beginning [early 2000] was "knserv", a standalone Perl web
// server/event router with topic-based routing and a
// tightly-integrated JavaScript client. Both the server and the
// client were nicely written and in the OO style.
//
// In the beginning events had only numeric IDs and simple string
// payloads. Topic names were case-insensitive strings, and pub/sub was
// accomplished through the "kn_subscribe" and "kn_unsubscribe"
// functions which are still with us today. There were no inter-topic
// routes, only topic -> client routes. Clients were
// username/devicename pairs. knserv was a pure router, with no event
// persistence. The routing tables were forgotten each time knserv was
// restarted. There was a default "anonymous" user, and support for
// named users with their own individual collections of devices.
//
// The knserv username/devicename system made for some great demos,
// with mailboxes, pagers, cellphones, and web browsers each receiving
// appropriate versions of events.
//
// knserv kept growing. Eventually it added partial support for XML
// event payloads, email notifications [including abbreviations for
// tiny skytel and nextel pagers,] and some interesting sensors (email
// and RSS).
//
// knserv's HTTP server had special URI and method support for event
// routing and other stuff:
//
// GET on /STAT returned a human-readable summary of routing status
// for the client making the request
//
// GET on /RESET reset the router
//
// GET on /KN/topicname set up a persistent subscription to
// topicname, and closed any previous connection from that client.
//
// POST on /KN?topic=topicname published an event onto topicname, and
// returned the 204 No Content HTTP response. The protocol included
// "add", "update" and "remove" operations which were meaningful mostly
// on the client side.
//
// knserv had no useful reflection, and the only first-class client
// was the JavaScript-enabled web browser.

//////// STREAMINGSCRIPT CLIENT LIBRARY
// Who: [ see KNSERV ]
// What: knserv JavaScript window manager and event notification
//       library
// When: [ see KNSERV ]
// Where: [ see KNSERV ]
//
// The knserv JavaScript client (based on "StreamingScript") was
// fairly tightly integrated with knserv. It only worked in Internet
// Explorer, and only looked good in the Windows version.
//
// The browser's concurrent connection limit was overcome by sharing a
// multiplexed persistent connection based on a shared identifier and
// IP address. To avoid problems with stalled connections, newly
// connecting clients with known IP/identifier combinations caused the
// server to close previous connections and reroute events over the
// new persistent connection. To avoid problems with lazy rendering
// (and hence lazy event delivery,) knserv sent 4k of spaces after
// each event.
//
// The persistent connection sent event data inside HTML form fields
// paired with SCRIPT tags which extracted the form data and handed it
// to the client library. Each delivery included a list of topics,
// and the client library would dispatch the same event to handlers for
// each topic.
//
// The event delivery format suffered from severe quoting problems,
// and some special characters were replaced with spaces on the
// server side.
//
// The Control Panel, Chat, Email, Pager and Post-It demos were
// originally developed using the knserv client library.
//
// The client library included a window manager to place newly opened
// applications in a grid-cell pattern, which made for slick demos
// indeed.

//////// MARS, KN, RUTH AND THE KNOWNOW EVENT ROUTER, VERSION 1.0
// Who: Rohit Khare, Adam Rifkin, Kragen Sitaker, Robert Thau, Ben
//      Sittler, and Chris Olds
// What: modular event router
// When: summer 2000 - April 2001
// Where: KnowNow in Seattle and KnowNow in Menlo Park
//
// Rohit Khare realized that knserv had some serious shortcomings, and
// finally convinced the programmers to throw out knserv and start on
// a new event router. The new event router was to be less tightly
// coupled with the web browser than knserv, and also would be a CGI
// script, so that it could be dropped into different existing web
// servers, with Apache as the initial target. The server would be
// able to send notifications to servers at other URLs, allowing truly
// Internet-scale event notification. Clients could run their own
// local event routers. Perhaps most importantly, the client and
// server would be decoupled enough so that implementations of each
// could evolve and multiply separately.
//
// The new Message Routing Server (MaRS) was a pure event router, with
// only minimal support for client session management. For the first
// time, events and routes were persistent, allowing clients to
// receive earlier events and allowing polling clients and usable
// intermittent connections. URI routes (including topic-to-topic
// routes) were introduced, and the conflicting needs for event
// updates (in the presence of routing loops) and reliable sequence
// resumption lead to the eventual separation of regular
// duplicate-suppressing topics and kn_journal queue topics. Content
// filtering (based on regular expressions) was made a part of the
// event router, as was content transformation (using URI-accessible
// transformers outside the router.) Clients were addressed using
// their randomly-chosen queue topics, meaning distributed proxies and
// DHCP could no longer disrupt the session-management scheme by
// changing client IP addresses.
//
// MaRS was implemented as a CGI script called mars.cgi, which was
// later renamed kn.cgi when MaRS and MeRCury were folded together to
// create "kn", the first complete event router package since knserv.
//
// Performance problems with the CGI model lead to the use of Apache
// mod_perl, and to RUTH (Robert's Ugly thttpd Hack [or perhaps a
// tribute to Babe Ruth]). mod_perl decreased latency by caching the
// CGI script, and RUTH improved router performance by consolidating
// all persistent connection handling in a single monolothic server
// based on thttpd.
//
// The router's internal event format was originally based on
// RFC 822, with header names and values and an arbitrary-length text
// payload. The original plan was to eventually send events directly
// as HTTP requests. Eventually, though, the event format deviated
// enough to be nearly unrecognizable. Header names and values became
// arbitrary-length and were allowed to contain arbitrary data, and
// the payload was allowed to contain arbitrary data. To accomodate
// these extensions, quoted-printable escaping was added for header
// names and values, and case-insensitivity was dropped. The
// extensions eventually allowed all clients to switch to using UTF-8
// data on the wire.
//
// Initially only JavaScript clients were supported, using an HTML
// form element + JavaScript event delivery format similar to that
// used by knserv. Jeff Barr wrote the first non-JavaScript client (a
// Visual BASIC KnowNow Chat program) using this JavaScript format. It
// soon became clear, though, that upgrades in the JavaScript (for
// example, switching from HTML form elements to JavaScript object
// literals) would require painful updates to the parser heuristics in
// non-JavaScript clients. For this reason a new "simple" format was
// developed for non-JavaScript clients. The simple format was labeled
// text/plain, and sent events using the router's internal event
// format, prefixed by a byte count.
//
// MaRS was eventually released as version 1.0 of the KnowNow Event
// Router. The event notification network protocols developed using
// MaRS are still used in more-or-less compatible form in KnowNow's
// current products.

//////// EARLY MERCURY
// Who: Mike Dierken and Kragen Sitaker
// What: incomplete JavaScript event notification library for MaRS
// When: summer 2000
// Where: KnowNow in Seattle
//
// The new MaRS event router was incompatible with knserv, so Mike
// Dierken wrote a new JavaScript client library for it. This first
// Message Routing Client (MeRC or MeRCury) supported local event
// dispatch based on topic name, and used a popup Master Control
// Program (MCP, a Tron reference) window to hold the persistent
// connection. No real window management was done, so opening more
// than two concurrent applications caused connection deadlock.
//
// MeRCury used two frames at the bottom of the MCP window for
// communication with the router.
//
// The publish/subscribe API developed for MeRCury (which included
// both OO methods and function versions of publish and subscribe) is
// still implemented in more-or-less compatible form in the current
// KnowNow JavaScript Microserver.
//
// The Control Panel, Chat, Pager and Post-It demos were ported from
// the knserv client libraries to the new MeRCury libraries. The old
// Email demo fell by the wayside.

//////// LATER MERCURY, ESP AND THE KNOWNOW JAVASCRIPT MICROSERVER
// Who: Rohit Khare, Mike Dierken, Kragen Sitaker, Ben Sittler, Danny
//      Quist, Brian Boyer and others
// What: JavaScript event notification library for MaRS, Nomad and
//       clens
// When: fall 2000 - present
// Where: KnowNow in Seattle, KnowNow in Menlo Park and KnowNow in
//        Mountain View
// ------ On August 28th, 2000, Rohit Khare was involved in an
//        automobile accident which broke his foot, restricting him to
//        his room for several days. He used this time well, writing
//        the first usable version of MeRCury.
//
// Rohit Khare was disgusted by the failure of the original MeRCury to
// handle concurrent connections and by the ever-present MCP
// window. Ben Sittler's research had demonstrated that inter-window
// communication was possible using local cookies and that a page
// could be transparently wrapped inside an frameset by an external
// script file. Using these techniques, Rohit Khare recreated on the
// client side the connection sharing system knserv had implemented
// inside the server.
//
// The heavily modified version of MeRCury which resulted from this
// exercise was largely transparent both to the user and to the
// application developer. The MCP window was gone, replaced by a pair
// of frames at the bottom of the dynamic frameset around the
// application. Originally visible, these frames were later hidden
// except while debugging the library or application.
//
// Newly opened applications communicated their window names and queue
// topic names using shared local cookies; an already-established
// window with a persistent connection (the "leader" window)
// established JavaScript references to them using window.open(). When
// the leader received events for one of the other open applications
// ("follower" windows,) it would use setTimeout to deliver a
// serialized version of the event to the follower. When the leader
// window was closed, it would attempt to designate one of the
// followers as a new leader. Since the leader window didn't work
// completely reliably (the user could close it at any time,) the
// followers also implemented a leader election algorithm, so that one
// of them would eventually become the new leader.
//
// For a time two separate versions of the library were maintained in
// parallel. The regular version looked much like this file, while the
// compressed version had no extra whitespace, comments, or debugging
// code. However, the error-prone process of synchronizing them soon
// proved tiresome, so Ben Sittler wrote kn_compress.sh, a GNU Bourne
// Again shell script which performed heuristic identifier and
// whitespace compression, automatically generating a compressed
// version from the regular version. The limited nature of the shell
// script led to several peculiar naming conventions which persist
// to this day (_kn_xxx, kn._xxx, _l_xxx, //_, etc.) The compressor
// could really benefit from a rewrite using Flex and Bison, but so
// far nobody has had the time or inclination to do this.
//
// The result has since been called (at various times) MeRCury, the
// KnowNow Event Server Page (ESP) Library and the KnowNow
// JavaScript Microserver. Minor details have changed, but the overall
// design and the meat of the code remain the same. Initally only
// Internet Explorer was supported, but support for Netscape 4.7 was
// added early on. Later experimental Mozilla and Netscape 6 support
// was added. At various times the library has worked with the 16-bit
// versions of Netscape 4, but this is currently broken due to the
// large file size of kn.js (even the compressed version is too big
// for the 16-bit Netscape script size limitations.)
//
// Applications using the KnowNow JavaScript Microserver were at one
// time called Event Server Pages (ESPs). Most of the old knserv
// demos eventually became ESPs and many new ESPs were written from
// scratch.

//////// NOMAD (A.K.A. THE KNOWNOW EVENT ROUTER, VERSION 1.1+)
// Who: Steve Dossick, Adam Zell and Todd Suzanski
// What: multithreaded MaRS-compatible "enterprise" event router
// When: early 2001 - present
// Where: KnowNow in Menlo Park and KnowNow in Mountain View
// ------ In late 2000 KnowNow entered into negotiations with the
//        Kleiner Perkins Caufield and Byers (KPCB) venture
//        capitalists.  These negotiations lead eventually to the
//        first large round of funding (in early 2001) and resulted in
//        KnowNow moving from Seattle to California (first to
//        temporary offices on the KPCB campus in Menlo Park and
//        later to a larger office in downtown Mountain View.)
//
// In an attempt to get enterprise customers and play into the markets
// KnowNow's new investors saw, a new high-performance "enterprise"
// event router was written as module for the open-source AOLServer
// web server.
//
// The new KnowNow Event Router was named Nomad. Nomad attempted to
// maintain backwards-compatibility with MaRS at the protocol level,
// although some semantic changes broke a large numer of older ESPs.
//
// Nomad has pioneered LAN-based clustered event router configurations
// and some new types of event router introspection. Recent versions
// seem about as capable as the last MaRS release, and much faster.

//////// OTHER MICROSERVERS
// Who: Jeff Barr, Chuck Saxon, Mythili Gopalakrishnan, Kragen Sitaker
//      and Fred Sanchez
// What: other clients of MaRS and Nomad
// When: late 2000 - present
// Where: Washington, KnowNow in Menlo Park and KnowNow in Mountain View
//
// Jeff Barr's Visual BASIC Chat application was for some time the
// only non-JavaScript client of the KnowNow Event Router. The simple
// event delivery format, though, gave rise to a new crop of
// microservers in C, C++, Java and Perl. Some features now found
// in the KnowNow JavaScript Microserver can be traced to these.

//////// CLENS
// Who: Kragen Sitaker
// What: singlethreaded experimental MaRS-compatible event router
// When: spring 2001
// Where: KnowNow in Menlo Park
//
// The Clean Event Notification System (clens) was a high-performance
// single-threaded event router developed in Python for research
// purposes. Out of that research came an improved restart algorithm
// now used for queue topics, and an improved status reporting
// technique which has yet to be applied elsewhere.
//
// clens was developed using rapid prototyping techniques, and was the
// first stand-alone router to run most of the MaRS-compatible
// applications without modification.

////////////////////////////////////////////////////////////////////
// INCOMPLETE CHANGE LOG
//
// This file has outlived at least two separate CVS repositories, and
// is descended from several separate files. They were merged together
// when MaRS and MeRCury became "kn".

//
// $Log: pubsub_raw.js,v $
// Revision 1.6  2003/04/28 23:17:58  ifindkarma
// Fixed rejection of invalid surrogate characters in Unicode.
//
// Revision 1.5  2003/04/19 00:20:35  ifindkarma
// Fixed long-lived connections in IE6+.
//
// Revision 1.4  2003/04/19 00:19:58  ifindkarma
// Fixed long-lived connections in IE6+.
//
// Revision 1.14  2002/08/16 03:31:34  bsittler
// Allowed kn_retryInterval and kn_maxRetryInterval to be overridden by
// the user at microserver startup.
//
// Added a new integer property kn_maxBatchSize to limit the size of
// automatic batches. The default value is 10 (requests).
//
// All three parameters can also be initialized using command-line
// arguments.
//
// Request retries now work even after tunnel restarts, even in Netscape
// 4 (they hook into the leader-election algorithm.)
//
// Revision 1.13  2002/08/10 05:57:18  bsittler
// added support for the undocumented do_method=delete_topic
//
// Revision 1.12  2002/08/08 19:04:33  bsittler
// Retries now work in MSIE and Netscape 7 even after extended connection
// outages. Netscape 4 still seems to have some setTimeout issues here,
// though.
//
// Added kn_options/kn_opts stuff (thanks, Scott!) to replace the old kn_hacks flags.
//
// Removed _kn_array(), since it wasn't actually necessary.
//
// Added Scott's fix for the checkTunnel exception in MSIE.
//
// Revision 1.11  2002/03/26 19:25:44  bsittler
// Fixed a nasty kn_resolvePath bug which broke the microserver after a while.
//
// Revision 1.10  2002/03/26 17:34:10  bsittler
// Now kn_server and kn_blank work properly when relative URLs are given.
//
// Revision 1.9  2002/03/26 04:37:55  bsittler
// Fixed a type caused by copy-n-paste. Now kn_blank works properly when
// set through kn_argv, and doesn't overwrite kn_server.
//
// Revision 1.8  2002/03/22 22:45:51  bsittler
// Fixed some incompletely compressed property of the kn object to be
// completely compressed (made them start with _l_ instead of _, since
// they are sometimes used as "this" properties instead of as a "kn"
// properties.)
//
// Revision 1.7  2002/03/18 07:12:15  bsittler
// changed this -> self in initializtion
// changed window -> self
// new self/argument config vars:
//         kn_tunnelURI -> kn.tunnelURI [URI for tunnel route source topic]
//         kn_tunnelID -> kn.tunnelID [kn_id for tunnel route]
//         kn_tunnelMaxAge -> kn.tunnelMaxAge [do_max_age value for reconnect]
//         kn_lastTag_ -> kn.lastTag_ [do_since_checkpoint value for reconnect]
//         kn_hashCache -> kn.getHashCache()/kn.setHashCache() [hashCache manipulation]
//         kn_server
//         kn_blank
// kn.tunnelID
// kn.tunnelMaxAge
// str = kn.getHashCache
// kn.setHashCache(str)
// kn.restartTunnel()
// eliminated the barely-used kn._serverURI
// cleaned up status handler dispatch handles in deferred workQ items
// added support for the kn_event_hash/do_max_age tunnel reconnect scheme
//         kn_debug flag "duplicates" alerts when duplicate hashes are suppressed
//         hashCache API allows the event hash cache to be preloaded, serialized, and deserialized
// new user-defined callbacks:
//         kn_onTunnelStop()
//         kn_onTunnelStatus(statusEvent)
//         kn_defaultOnMessage(event) [overrides default "unsubscribe"/"ignore" behavior]
//
// Revision 1.6  2002/01/04 04:49:33  bsittler
// Fixed some subtle scoping bugs.
//
// Now improperly formed status events don't cause JavaScript errors.
//
// Revision 1.5  2001/12/28 21:44:39  bsittler
// Certain Mac IE 5.1 multi-window scenarios break inter-window
// communication in a fairly subtle way: the leader is able to open a
// frame in the follower's frameset, but attempts to access the
// follower's properties (e.g. the 'kn' object) lead to "Permission
// denied" errors. The workaround is to check the type of the "kn"
// object, and set up a partition between the old and new windows if the
// type is "undefined". This appears to actually work, at least for small
// numbers of windows.
//
// Revision 1.4  2001/12/27 23:37:41  bsittler
// Up-to-date with perforce.
//
// Revision 1.3  2001/12/06 22:40:52  bsittler
// Fixed some nav4 bugs. Now succession and multiple windows seem to work.
//
// Revision 1.2  2001/11/13 02:22:56  bsittler
// Reorganized, fixed some bugs. Gave up on Netscape 3 support until the
// router guys are willing to give it a shot with another response format.
//
//

////////////////////////////////////////////////////////////////////
// POSTSCRIPT
//
// In November 2002, KnowNow authorized an Apache-based license for
// releasing a toolkit containing a lot of the early work, which we've
// put together as mod_pubsub.
//
// Specifically, mod_pubsub calls the code you're reading the "PubSub
// JavaScript Library".  You are free to modify and use it as stated by
// the license; enjoy!

