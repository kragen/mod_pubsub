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

// $Id: client_presence.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $

function RouteWatcher(topic)
{
    this.topic = topic;
    this.routes = [];
    this.journalWatchlist = [];
    this.watch = RouteWatcher_watch;
    this.onMessage = RouteWatcher_onMessage;
    this.onSuccess = RouteWatcher_ignoreStatus;
    this.onError = RouteWatcher_ignoreStatus;
    this.onRouteStatus = function(){};
    this.autodelete = false;
    this.stomp_timer = null;
}

function RouteWatcher_watch(topic)
{
    if (topic) this.topic = topic;
    this.r_rid = kn.subscribe(
                                this.topic + "/kn_routes",
                                this,
                                {
                                    kn_expires:"infinity",
                                    do_max_age:"infinity",
                                    kn_deletions:"true"
                                },
                                this
                            );
}

function RouteWatcher_onMessage(route)
{
    if (route.kn_deleted == "true" || route.kn_payload == "")
    {
    
        if (this.stomp_timer)
        {
            clearInterval(this.stomp_timer);
            this.stomp_timer = null;
        }
        
        var rt = get_event_by_id(route.kn_id,this.routes);
        if (rt) this.onRouteStatus(rt,"deleted");
        
        this.routes = remove_event(route,this.routes);
        
    } else
    {
        this.routes[this.routes.length] = route;
        //now we find the matching journal
        var journalSuffixPattern = /\/kn_journal$/;
        if (route.kn_payload.match(journalSuffixPattern))
        {
            
            var journalLoc = route.kn_payload.replace(journalSuffixPattern,"/kn_subtopics");
            
            var jwatcher = {};
            jwatcher.parent = this;
            jwatcher.routeURI = route.kn_payload;
            jwatcher.routeID = route.kn_id;
            jwatcher.hasJournal = false;
            
            jwatcher.onMessage = JournalWatcher_onMessage;
            jwatcher.onSuccess = JournalWatcher_onSuccess;
            jwatcher.onError = RouteWatcher_ignoreStatus
            
            jwatcher.j_rid = kn.subscribe(
                                        journalLoc,
                                        jwatcher,
                                        {
                                            kn_expires:"infinity",
                                            do_max_age:"infinity",
                                            kn_deletions:"true"
                                        },
                                        jwatcher
            
                                    );

        } else
        {
            // this is a non-journal route, so we don't have to watch it.
        }
        
        this.onRouteStatus(route,"new");
    }
    
}

function RouteWatcher_ignoreStatus()
{

}

function JournalWatcher_onMessage(subtopic)
{
    var journalSuffix = "kn_journal";

    // check to make sure this is a journal topic

    if ((subtopic.kn_id == journalSuffix) ||
        (subtopic.kn_payload == journalSuffix))
    {
        if (subtopic.kn_deleted == "true" || !subtopic.kn_payload)
	    {
            
            var rt = get_event_by_id(this.routeID,this.parent.routes);
            if (rt) this.parent.onRouteStatus(rt,"journal deleted");
            
            if (this.parent.autodelete && !this.parent.stomp_timer)
            {
                // kill the associated route
                var time = (Math.random()*this.parent.routes.length)*1000;
                
                this.parent.stomp_timer = 
                    setTimeout(
                        "RouteWatcher_stomp('" + 
                        this.parent.topic + "','" +
                        this.routeURI + "','" +
                        this.routeID + "');", time);
   
            }
            
            kn.unsubscribe(this.j_rid);
            
	    } else
	    {
            
            // we have a route and a journal, so let's go find the tunnel
            
            var rt = get_event_by_id(this.routeID,this.parent.routes);
            if (rt) this.parent.onRouteStatus(rt,"journal found");
            
            this.hasJournal = true;
            
            var tWatcher = {};
            tWatcher.parent = this;
            
            tWatcher.onMessage = TunnelWatcher_onMessage;
            tWatcher.onSuccess = TunnelWatcher_onSuccess;
            tWatcher.onError = RouteWatcher_ignoreStatus;
            
            tWatcher.hasTunnel = false;
            this.t_rid = kn.subscribe(
                                        this.routeURI + "/kn_routes",
                                        tWatcher,
                                        {
                                            kn_expires:"infinity",
                                            do_max_age:"infinity",
                                            kn_deletions:"true"
                                        },
                                        tWatcher
                                     );
            
	    }
    }
}

function JournalWatcher_onSuccess()
{
    if (!this.hasJournal)
    {
    
        var rt = get_event_by_id(this.routeID,this.parent.routes);
        if (rt) this.parent.onRouteStatus(rt,"no journal");
        
        if (this.parent.autodelete && !this.parent.stomp_timer)
        {
        
            var time = (Math.random()*this.parent.routes.length)*1000;
            
            this.parent.stomp_timer = 
                setTimeout(
                    "RouteWatcher_stomp(" + 
                    this.parent.topic + "," +
                    this.routeURI + "," +
                    this.routeID + ");", time);

         }
         
         kn.unsubscribe(this.j_rid);       
    }
}

var tunnelsVisible = false;

function TunnelWatcher_onMessage(tunnel)
{
    
    if (!tunnelsVisible)
    {
        tunnelsVisible = true;
    }
    if (tunnel.kn_deleted)
    {
        this.hasTunnel = false;
        
        // tunnel deceased.
        
        var rt = get_event_by_id(this.parent.routeID,this.parent.parent.routes);
        if (rt) this.parent.parent.onRouteStatus(rt,"tunnel closed");
        
    } else
    {
        var rt = get_event_by_id(this.parent.routeID,this.parent.parent.routes);
        if (rt) this.parent.parent.onRouteStatus(rt,"tunnel open");
            
        this.hasTunnel = true;
    }
}

function TunnelWatcher_onSuccess(status)
{
    if (!this.hasTunnel)
    {
        // could be in the "journaling" (queuing) state.
        
        var rt = get_event_by_id(this.parent.routeID,this.parent.parent.routes);
        if (rt) this.parent.parent.onRouteStatus(rt,"no tunnel");
        
    }
}

function RouteWatcher_stomp(kn_from, kn_to, kn_id)
{
    kn.DELETE_ROUTE(
                    {
                        kn_from:kn_from,
                        kn_to:kn_to,
                        kn_id:kn_id
                    },
                    {
                        onError:RouteWatcher_ignoreStatus
                    }
            );
}

// array helper functions

function remove_event(evt,arry)
{
    var arry2 = [];
    for (var i=0;i<arry.length;i++)
    {
        if (arry[i].kn_id != evt.kn_id)
        {
            arry2[arry2.length] = arry[i];
        }
    }
    return arry2;
}

function remove_event_by_id(id,arry)
{
    var arry2 = [];
    for (var i=0;i<arry.length;i++)
    {
        if (arry[i].kn_id != id)
        {
            arry2[arry2.length] = arry[i];
        }
    }
    return arry2;
}

function get_event_by_id(id,arry)
{
    if (arry.length == 0) return;
    for (var i=0;i<arry.length;i++)
    {
        if (arry[i].kn_id == id)
        {
            return arry[i];
        }
    }
    return false;
}

// End of client_presence.js
