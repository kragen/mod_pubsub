// PubSub JavaScript Event List Library.

// Demo that uses this library: mod_pubsub/kn_apps/eventlist/

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

// $Id: event_list.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $


// This is dead-simple but useful script that subscribes an HTML tag
// to a topic, making anything inside that tag a template for each
// event that arrives.  In this way, you can easily make a "live"
// list of events, formatted any way you wish.

// To seed your PubSub Server with some purchase orders, submit to your
// Web browser's URL:

//     http://mypubsubserver/kn/what/pos?do_method=notify&order_number=1&order_pcs=1000&order_item=Socks
//     http://mypubsubserver/kn/what/pos?do_method=notify&order_number=2&order_pcs=1001&order_item=Shoes

// Now, if you want to build a live list of purchase orders,
// you can do something like this:

// <div class="event_list" kn_from="/what/pos" do_max_age="infinity" kn_deletions="true">
//   <p>
//     <b>Purchase Order #{{order_number}}</b><br>
//     Item #: {{item_number}}<br>
//     Quantity: {{item_quantity}}<br>
//     Description: {{item_description}}<br>
//   </p>
// </div>

// When you load this page in IE, the script will take the innards of
// the DIV tag and make it a template, replacing the {{braced}} text
// with the value of the corresponding PubSub header.  It will repeat
// this process for each event received, creating a formatted list.



function PubSubEventListInit()
// Initializes elements for subscriptions.
{
    // Get all elements.
    var elms = document.all;
    // Loop thru.
    for (var i=0;i<elms.length;i++)
    {
        // Only elements of the event_list class get initialized.
        if (elms[i].className.indexOf("event_list") != -1)
        {
            elms[i].onMessage = doPubSubEventList;
            // Store template HTML.
            elms[i]._template = elms[i].innerHTML;
            elms[i]._evts = [];
            elms[i].innerHTML = "";
            kn.subscribe(
                elms[i].kn_from,
                elms[i],
                _pubsub_elm2event(elms[i])
            );
        }
    }
}

function _pubsub_elm2event(elm)
// Retrieves the specified request headers from the HTML element.
{
    var a = _pubsub_elm2event.lookup;
    var o = {};
    for (var i=0;i<a.length;i++)
    {
        if (elm[a[i]])
        {
            o[a[i]] = elm[a[i]]; 
        }
    }
    return o;
}
_pubsub_elm2event.lookup = ["do_max_n","do_max_age","kn_expires","kn_deletions"];

function doPubSubEventList(e)
// Processes the headers and places them into the correct cells.
{
    if (e.kn_deleted)
    {
        // If deleted, blank out this event's HTML entry.
        this._evts[assignIndex(this,e)].html  = "";
    } else
    {
        // Get template for this element.
        var html = this._template;
        for (var header in e)
        {
            // Replace template tokens with event header values.
            var regexp = new RegExp("{{" + header.toLowerCase() + "}}", "gi");
            html = html.replace(regexp,  kn_htmlEscape(e[header]));
        }
        // Create new HTML entry.
        this._evts[assignIndex(this,e)] = {id:e.kn_id,html:html};
    }
    
    // Now concatenate HTML string.
    var str = "";
    for (var i=0;i<this._evts.length;i++)
    {
        str += this._evts[i].html;
    }
    // And insert it into the element. Voila!
    this.innerHTML = str;
}

function assignIndex(obj,e)
// Looks up or assigns index of event object in this elements' array.
{
    for (var i=0;i<obj._evts.length;i++)
    {
        if (obj._evts[i].id == e.kn_id)
        {
            return i;
        }
    }
    return obj._evts.length;
}

// End of event_list.js
