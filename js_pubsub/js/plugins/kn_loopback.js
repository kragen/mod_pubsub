/*! @file kn_loopback.js JavaScript PubSub Library Loop-Back Transport
 * <pre>self.kn_include_plugin("kn_loopback"); // in self.kn_config()</pre>
 */

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

// $Id: kn_loopback.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $

// Loopback is refigured as an event transport API plug-in.
// Now multiple event transport plugins may be included, and the first
// one to load successfully will take effect.

// Subtopic constructor.

function kn_loopback_Topic(name, segment)
{
    this.name = name;
    this.segment = segment;
    this.events = new Array();
    this.event2slot = kn__object();
    this.subtopics = new Array();
    this.subtopic2slot = kn__object();
    this.toString = _kn_loopback_Topic_toString;
    this._routeEvent = _kn_loopback_Topic__routeEvent;
    this.notify = _kn_loopback_Topic_notify;
    this.subTopic = _kn_loopback_Topic_subTopic;
}

function _kn_loopback_Topic_toString(prefix)
{
    if (prefix == null)
    {
        prefix = '';
    }
    prefix = '    ' + prefix;
    var string = new Array();
    string[string.length] = this.segment + '/';
    for (var index = 0; index < this.subtopics.length; index ++)
    {
        topic = this.subtopics[index];
        string[string.length] = topic.toString(prefix);
    }
    for (var id in this.event2slot)
    {
        string[string.length] = id;
        var event = this.events[this.event2slot[id]];
        for (var p in event)
        {
            string[string.length] = '    ' + p + ': ' + kn_htmlEscape(event[p]);
        }
    }
    return string.join('\n' + prefix);
}

function _kn_loopback_Topic__routeEvent(event)
{
    var slot = this.subtopic2slot['kn_routes'];
    if (slot != null)
    {
        var routes = this.subtopics[slot];
        if (routes)
        {
            //var now = (new Date()).getTime() / 1000;
            for (var index = 0; index < routes.events.length; index ++)
            {
                var route = routes.events[index];
                if (route && route.kn_payload)
                {
                    var routedEvent = kn__object();
                    for (var p in event)
                    {
                        routedEvent[p] = event[p];
                    }
                    routedEvent.kn_route_id = route.kn_id;
                    routedEvent.kn_routed_from = this.name;
                    routedEvent.kn_route_location = route.kn_uri;
                    kn_loopback_notify(route.kn_payload, routedEvent, null);
                }
            }
        }
    }
}

function _kn_loopback_Topic_notify(event, status)
{
    if (event.kn_time_t == null)
    {
        event.kn_time_t = ((new Date()).getTime() / 1000).toString();
    }
    if (event.kn_id == null)
    {
        event.kn_id = Math.random().toString();
    }
    if (event.kn_uri == null)
    {
        event.kn_uri = kn_server + this.name + '/' + kn_escape(event.kn_id);
    }
    var slot = this.event2slot[event.kn_id];
    if (slot != null)
    {
        var oldEvent = this.events[slot];
        if (oldEvent && kn_loopback_match(event, oldEvent))
        {
            if (status)
            {
                status.status = '200 ' + kn_loopback$('Ignored');
                status.kn_payload = kn_loopback$('Your duplicate event was suppressed.');
            }
            return;
        }
        delete this.events[slot];
    }
    slot = this.events.length;
    this.event2slot[event.kn_id] = slot;
    this.events[slot] = event;
    if (status)
    {
        status.status = '200 ' + kn_loopback$('Notified');
        status.kn_payload = kn_loopback$('Your event will be forwarded to all subscribers.');
    }
    this._routeEvent(event);
}

function _kn_loopback_Topic_subTopic(segment)
{
    var slot = this.subtopic2slot[segment];
    if (slot != null)
    {
        return this.subtopics[slot];
    }
    slot = this.subtopics.length;
    var path = this.name.split('/');
    path[path.length] = segment;
    var topic = new kn_loopback_Topic(path.join('/'), segment);
    this.subtopic2slot[segment] = slot;
    this.subtopics[slot] = topic;
    return topic;
}

function kn_loopback_match(e1, e2)
{
    var union = kn__object();
    for (var p in e1)
    {
        union[p] = e1[p];
    }
    for (var p in e2)
    {
        union[p] = e2[p];
    }
    for (var p in union)
    {
        if ((p.substring(0, 'kn_route_'.length) == 'kn_route_') ||
            (p == 'kn_routed_from'))
        {
            continue;
        }
        if (e1[p] != e2[p])
        {
            return false;
        }
    }
    return true;
}

function kn_loopback_notify(name, event, status)
{
    name = '' + name;
    if (name == kn_server)
    {
        name = '/';
    }
    else if (name.substring(0, kn_server.length + 1) == (kn_server + '/'))
    {
        name = name.substring(kn_server.length);
    }
    if (name == kn.tunnelURI)
    {
        if (status)
        {
            status.status = '200 ' + kn_loopback$('Notified');
            status.kn_payload = kn_loopback$('Your event was delivered to a simulated persistent connection.');
        }
        kn_loopback_enqueue(event);
        return;
    }
    var path = name.split('/');
    var topic = kn_loopback_root;
    for (var i = 0; i < path.length; i ++)
    {
        if (path[i])
        {
            topic = topic.subTopic(path[i]);
        }
    }
    topic.notify(event, status);
}

// Enqueue an event for local delivery; we must use setTimeout here to
// preserve the timing semantics of the transport built in to the
// JavaScript PubSub Library.

function kn_loopback_enqueue(event)
{
    kn_loopback_queue[kn_loopback_queue.length] = event;
    if (! kn_loopback_timer)
    {
        kn_loopback_timer = setTimeout("kn_loopback_dequeue()", 1);
    }
}

// Empty the local delivery queue.

function kn_loopback_dequeue()
{
    // save the old queue and start a fresh one; this must be done
    // before any events are delivered
    var queue = kn_loopback_queue;
    kn_loopback_queue = new Array();
    kn_loopback_timer = null;

    // deliver all events
    for (var index = 0; index < queue.length; index ++)
    {
        kn_loopback_send(queue[index]);
    }
}

// Submit (or possibly batch) a PubSub Server request
// returns false until the request has been sent (or at least batched).

function kn_loopback__submitRequest(e)
{
    var s = kn__object(
        'status',
        '500 ' + kn_loopback$('Internal Server Error'),
        'kn_payload',
        kn_loopback$('Your request could not be processed.')
        );
    if (e.kn_status_from)
    {
        s.kn_route_location = e.kn_status_from;
    }
    var status_to = kn.tunnelURI;
    if (e.kn_status_to != null)
    {
        status_to = e.kn_status_to;
    }
    var event = kn__object();
    for (var p in e)
    {
        if ((p.substring(0, 'kn_status_'.length) == 'kn_status_') ||
            (p.substring(0, 'kn_response_'.length) == 'kn_response_') ||
            (p == 'do_method') ||
            (p == 'kn_batch') ||
            (p == 'kn_from') ||
            (p == 'kn_to'))
        {
            continue;
        }
        event[p] = '' + e[p];
    }
    if (e.do_method == 'route')
    {
        var from = e.kn_from;
        if (from == null)
        {
            from = '/';
        }
        var path = from.split('/');
        path[path.length] = 'kn_routes';
        from = path.join('/');
        var to = e.kn_to;
        if (to == null)
        {
            to = '';
        }
        event.kn_payload = to;
        kn_loopback_notify(from, event, s);
    }
    else if (e.do_method == 'notify')
    {
        var to = e.kn_to;
        if (to == null)
        {
            to = '/';
        }
        kn_loopback_notify(to, event, s);
    }
    s.kn_time_t = ((new Date()).getTime() / 1000).toString();
    kn_loopback_notify(status_to, s, null);
    return true;
}

// Deliver an event to the dispatcher in the JavaScript PubSub Library.

function kn_loopback_send(e)
{
    var f = kn__object(
        'elements',
        new Array()
        );
    for (var p in e)
    {
        f.elements[f.elements.length] =
            kn__object(
                'name',
                p,
                'value',
                e[p]
                );
    }
    kn.ownerWindow.kn_sendCallback(f, null);
}

// Restart the tunnel connection.

function kn_loopback__restartTunnel()
{
}

// Mock up a local pubsub server.

function kn_loopback__wrapApp()
{
    kn_createContext("kn_loopback");
    kn_loopback_root = new kn_loopback_Topic("", "");
    kn_loopback_queue = new Array();
    kn_loopback_timer = null;
    kn.ownerWindow.kn__submitRequest = kn_loopback__submitRequest;
    kn.ownerWindow.kn__restartTunnel = kn_loopback__restartTunnel;
    kn.isLoadedP_ = true;
    kn.tunnelRunning_ = true;
    kn.tunnelURI = "javascript:";
    kn.lastTag_ = null;
}

// Only run if no other custom transport has been loaded.

if (! self.kn__wrapApp)
{
    // hook ourselves into the pubsub client library
    kn__wrapApp = kn_loopback__wrapApp;
}

// End of kn_loopback.js
