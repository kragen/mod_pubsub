// PubSub JavaScript Event Collector Library.

// Demo that uses this library: mod_pubsub/kn_apps/eventcollector/

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

// $Id: event_collector.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $


// To use event_collector.js, create an instance of PubSubEventCollection
// and pass it as the second argument to kn.subscribe. The object then
// maintains a "view" of the events in that topic.  It automatically
// removes deleted events and overwrites duplicates with any new
// header values.  The onEvent handler notifies your app of events;
// it is up to the application developer to decide how to interpret
// (and render) the results.

// There's also an "apply" method that basically takes a function and
// applies it to every event in the view.


function PubSubEventCollection()
// Constructor.
{
    this.events = [];
    if (typeof(this.events.item) == "undefined")
    {
        this.events.item = PubSubEventCollection.Item;
    }
    this.length = 0;
    this.deletions = false;
    return this;
}

// Default empty handler.
PubSubEventCollection.prototype.onEvent = function(){}

PubSubEventCollection.Item = function(index)
// Returns event at specified index in the collection.
// Replacement for item() method absent in most non-IE browsers.
{
    if (index < this.events.length - 1)
    {
        return this.events[index];
    }
    else
    {
        return null;
    }
}

PubSubEventCollection.prototype.onMessage = function(evt)
// Receives new events and places them into events[] collection.
// Identifies duplicates and removes deleted events if requested.
{
    var index = this.getIndex(evt);
    if (index < 0)
    {
        this.events[this.events.length] = evt;
    }
    else
    {
        this.events[index] = evt;
        if (this.deletions && evt.kn_deleted == "true")
        {
            this.remove(evt);
        }
    }
    this.length = this.events.length;
    // Fire onEvent handler.
    this.onEvent();
}

PubSubEventCollection.prototype.remove = function(evt)
// Removes an event from the events collection.
// Returns index of removed event.
{
    var index = this.getIndex(evt)
    if (index > -1)
    {
        var arry = [];
        for (var i=0;i<this.events.length;i++)
        {
            // Cannot rely on slice and concat, so we just copy
            // the values into a new array.
            if (!this.events[i].kn_deleted)
            {
                arry[arry.length] = this.events[i];
            }
        }
        this.events = arry;
        return index;
    }
    else
    {
        return -1;
    }
}

PubSubEventCollection.prototype.getIndex = function(evt)
// Returns the numeric index of this event in the collection.
{
    var id = evt.kn_id;
    for (var i=0; i<this.events.length;i++)
    {
        if (this.events[i].kn_id == id)
        {
            return i;
        }
    }
    return -1;
}

PubSubEventCollection.prototype.apply = function(fn)
// Applies function fn to all events in the collection.
// Returns array of return values.
{
    var arry = [];
    for (var i=0; i<this.events.length;i++)
    {
        arry[i] = fn(this.events[i]);
    }
    return arry;
}

// End of event_collector.js
