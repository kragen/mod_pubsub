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

// $Id: Group.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $

function Group(name)
{
    // methods
    this.attach = Group_attach;
    this.numOnline = Group_numOnline;
    this.numBuddies = Group_numBuddies;
    this.clear = Group_clear;
    this.hasBuddy = Group_hasBuddy;
    this.addBuddyRef = Group_addBuddyRef;
    this.findBuddyRef = Group_findBuddyRef;
    this.removeBuddy = Group_removeBuddy;
    this.removeBuddyByName = Group_removeBuddyByName;
    
    // properties
    this.name = name;
    this.isExpanded = true;
    this.buddyref = new Array();
}

function Group_clear()
{
    this.buddyref = new Array();
}

function Group_attach(obj)
{
    hook_props(this,obj);
    hook_array(this,'buddyref');
}

function Group_numBuddies()
{
    return this.buddyref.count();
}

function Group_numOnline()
{
    var count=0;

    for (var i=0; i < this.buddyref.length; i++)
    {
        if (!this.buddyref[i])
            continue;

        for (var j=0; j < myBuddies.buddy.length; j++)
        {
            if (!myBuddies.buddy[j])
                continue;
                
            if (myBuddies.buddy[j].person.userid.valueOf() == this.buddyref[i].valueOf())
            {
                buddy = myBuddies.buddy[j];
                if (buddy.status.code != 'offline')
                    count++;
                break;
            }
        }
    }

    return count;
}

function Group_hasBuddy(name)
{
    return (this.findBuddyRef(name) != null);
}

function Group_addBuddyRef(name)
{
    if (!this.hasBuddy(name))
    {
        // TODO: Add to an empty slot?
        this.buddyref[this.buddyref.length] = name;
    }
}

function Group_findBuddyRef(name)
{
    for (var i=0; i < this.buddyref.length; i++)
    {
        if (!this.buddyref[i])
            continue;
            
        if (this.buddyref[i].valueOf() == name)
        {
            return this.buddyref[i];
        }
    }   
    return null;
}


function Group_removeBuddy(buddy)
{
    this.removeBuddyByName(buddy.person.userid.valueOf());
}

function Group_removeBuddyByName(name)
{
    for (var i=0; i < this.buddyref.length; i++)
    {
        if (!this.buddyref[i])
            continue;
            
        if (this.buddyref[i].valueOf() == name)
        {
            // NOTE: This sets the slot value to 'undefined'. Length is unchanged.
            delete this.buddyref[i];
            break;  // hopefully there is only one...
        }
    }   
}

// End of Group.js
