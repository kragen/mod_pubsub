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

// $Id: Buddy.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $

/*
<presence>
 <status>
    <!-- online, offline, available, unavailable, invisible, active -->
    <active>false</active>
    <code>online</code>
    <message>YooHoo</message>
 </status>
 <person>
  <displayname>Strata</displayname>
  <userid>/knownow.com/who/strata</userid>
 </person>
 <contact>
  <level>primary</level>
  <type>cellphone</type>
  <phone>650-279-1136</phone>
 </contact>
 <ttl units='minutes'>75</ttl>
 <subscription>
    <routeid>xyz</routeid>
 </subscription>
 
</presence>
*/

function Buddy(userid,displayname)
{
    // properties
    this.status = new Object();
    this.status.code = 'offline';   // offline, online, active
    this.status.message = '';
    this.status.active = false;
    this.status.lastContactedOn = new Date(0);
        
    this.person = new Object();
    this.person.userid = "";
    this.person.displayname = "";
    
    this.contact = new Object();
    this.contact.level = "primary";
    this.contact.type = "cellphone";
    this.contact.phone = "";
    
    this.subscription = new Object();
    this.subscription.routeid = "";

    if (userid)
        this.person.userid = userid;

    if (displayname)
        this.person.displayname = displayname;

    // methods
    this.attach = Buddy_attach;
}

function Buddy_attach(obj)
{
    hook_props(this,obj);
    
    if (this.status)
    {
        // Boolean objects
        if ((this.status.active==null) || 
            (this.status.active.constructor != Boolean))
        {
            this.status.active = new Boolean(this.status.active.valueOf() == 'true');
        }
        
        // Date objects
        if (this.status.lastContactedOn)
        {
            if (this.status.lastContactedOn.constructor != Date)
            {
                this.status.lastContactedOn = new Date(this.status.lastContactedOn);
            }
        }
        else
        {
            this.status.lastContactedOn = new Date(0);
        }
    }
}

// End of Buddy.js
