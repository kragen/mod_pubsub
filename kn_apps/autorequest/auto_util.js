// Copyright 2000-2002 KnowNow, Inc.  All Rights Reserved.

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

// $Id: auto_util.js,v 1.1 2002/11/07 07:08:54 troutgirl Exp $

function addEvent(msg)
{
    events[msg.kn_id] = msg;
}

function getEvent(event_id)
{
    return events[msg.kn_id];
}

function removeEvent(offer_id)
{
    if (events[offer_id])
        delete events[offer_id];
}

function clearEvents()
{
    events = new Object();
}

function findOffer(id)
{
    var form = document.response;
    
    for (var i=0; i < form.offers.options.length; i++)
    {
        if (form.offers.options[i].value == id)
        {
            return form.offers.options[i];
        }
    }
    return null;
}

function appendOffer(msg)
{
    var form = document.response;
    form.body.value = msg.message;
    form.body.value += '\n';
    
    if (msg.body.offer.type == 'sell')
    {
        form.offers.options[form.offers.options.length] = 
            new Option(msg.userid + " $"+msg.body.offer.price,msg.kn_id,false,false);
    }
    else
    {
        form.offers.options[form.offers.options.length] = 
            new Option(msg.userid + " $"+msg.body.offer.max,msg.kn_id,false,false);
    }
    
}

function addOffer(msg)
{
    var form = document.response;
    var option = findOffer(msg.kn_id);
    
    // Update the visible text.

    if (option)
    {
        if (msg.body.offer.type == 'sell')
        {
            option.text = msg.userid + " $"+msg.body.offer.price;
        }
        else
        {
            option.text = msg.userid + " $"+msg.body.offer.max;
        }
        
        
        form.body.value = msg.message;
        form.body.value += '\n';
    }
    else
    {
        appendOffer(msg);
    }
}

function drawOffers()
{
    clearOffers();
    for (var i in events)
    {
        if (isGoodMatch(events[i]))
            appendOffer(events[i]);
    }
}

function showCurrentOffer()
{
    var form = document.response;
    var index = form.offers.selectedIndex;
    var msg;
    
    if (form.offers.length == 0)
        return;

    if (index < 0)
    {
        index = 0;
        form.offers.selectedIndex=0;
    }
    msg = events[form.offers[index].value];
    if (msg)
    {
        form.body.value = msg.message;
        form.body.value += '\n';
    }
}

function removeOffer(offer_id)
{
    // No need to remove something; we re-render the current event list.
    drawOffers();
}

function clearOffers()
{
    var form = document.response;
    form.body.value = "";
    form.offers.options.length=0;   
}

function doClearOffers()
{
    clearOffers();
    clearEvents();
}

function addTopic(href,topic)
{
    if (href.indexOf('?') < 0)
    {
        href += "?kn_topic="+topic; 
    }
    else
    {
        href += "&kn_topic="+topic; 
    }

    return href;
}

function postInvite(userID,topic,href)
{
    var p = new Object();
    p.from = kn.userid;
    p.topic = topic;
    p.app = href;

    kn_publishAsXml("/who/"+userID+"/invite", "invite", p );
}

function doBidderChat()
{
    var form = document.response;
    var msg;

    if (form.offers.selectedIndex < 0)
    {
        alert("Please select a bidder first.");
        return;
    }
    msg = getEvent(form.offers.options[form.offers.selectedIndex]);
    
    if (msg && msg.userid)
    {
        var topic = "/what/auto-request/"+kn.userid+"/buy_from/"+msg.userid;
        var href = "../chat2/?inviter="+escape(kn.userid)+"&invited="+escape(msg.userid);
        // Send invite.
        postInvite(msg.userid,topic,href);
        window.open(addTopic(href,topic),"auto_buy_chat");
    }
}

// End of auto_util.js
