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

// $Id: BuddyList.js,v 1.1 2002/12/07 10:08:19 ifindkarma Exp $

function BuddyList()
{
    // methods
    this.attach = BuddyList_attach;

    this.numOnline = BuddyList_numOnline;
    this.clear = BuddyList_clear;
    
    // group management
    this.numGroups = BuddyList_numGroups;
    this.findGroup = BuddyList_findGroup;
    this.addGroup = BuddyList_addGroup;
    this.removeGroup = BuddyList_removeGroup;
    this.removeGroupByName = BuddyList_removeGroupByName;
    
    // buddy management
    this.numBuddies = BuddyList_numBuddies;
    this.findBuddy = BuddyList_findBuddy;
    this.addBuddy = BuddyList_addBuddy;
    this.removeBuddy = BuddyList_removeBuddy;
    this.removeBuddyByName = BuddyList_removeBuddyByName;
    this.isBuddyReferenced = BuddyList_isBuddyReferenced
    // properties

    this.buddy = new Array();
    this.group = new Array();
    this.isExpanded = true;
}


/*
* This adopts all the properties of the specified object.
*/
function BuddyList_attach(obj)
{
    // copy properties
    hook_props(this,obj);
    
    // hook non-array property
    hook_array(this,'buddy');
    hook_array(this,'group');
    
    // hook each buddy
    hook_array_elements(this,'buddy',Buddy);
    hook_array_elements(this,'group',Group);
}

function BuddyList_findGroup(name)
{
    for (var i =0; i < this.group.length; i++)
    {
        if (!this.group[i])
            continue;
            
        if (this.group[i].name == name)
        {
            return this.group[i];
        }
    }
    return null;
}

function BuddyList_addGroup(group)
{
    if (this.findGroup(group.name))
    {
        return;
    }
    // TODO: Add to empty slot?
    this.group[this.group.length] = group;
}

function BuddyList_removeGroupByName(name)
{
    for (var i =0; i < this.group.length; i++)
    {
        if (!this.group[i])
            continue;
            
        if (this.group[i].name == name)
        {
            delete this.group[i];
        }
    }
}

function BuddyList_removeGroup(group)
{
    this.removeGroupByName(group.name);
}

function BuddyList_findBuddy(name)
{
    for (var i =0; i < this.buddy.length; i++)
    {
        if (!this.buddy[i])
            continue;
    
        if (this.buddy[i].person.userid.valueOf() == name)
        {
            return this.buddy[i];
        }
    }
    return null;
}

function BuddyList_addBuddy(buddy)
{
    // don't allow duplicates
    if (this.findBuddy(buddy.person.userid))
    {
        return;
    }
    this.buddy[this.buddy.length] = buddy;
}

function BuddyList_removeBuddyByName(name)
{
    var buddy;
    buddy = this.findBuddy(name);
    if (buddy)
        this.removeBuddy(buddy);
}

function BuddyList_removeBuddy(buddy)
{
    var group;
    
    for (var i =0; i < this.buddy.length; i++)
    {
        if (!this.buddy[i])
            continue;
        
        // todo: should this be '==='?
        if (this.buddy[i] == buddy)
        {
            // remove from all groups
            for (var j=0; j < this.group.length; j++)
            {
                if (!this.group[j])
                    continue;

                group = this.group[j];
                group.removeBuddy(this.buddy[i]);
            }

            // NOTE: This sets the slot value to 'undefined'. Length is unchanged.
            delete this.buddy[i];
        }
    }
}

function BuddyList_isBuddyReferenced(buddy)
{
    var group;
    var found=false;
    for (var i = 0; i < this.group.length; i++)
    {
        group = this.group[i];
        if (!group)    continue;
        if (group.hasBuddy(selection.buddy.person.userid.valueOf()))
        {
            found=true;
            break;
        }
    }
    return found;
}


function BuddyList_clear()
{
    this.buddy = new Array();
    this.group = new Array();
}

function BuddyList_numBuddies()
{
    return this.buddy.count();
}

function BuddyList_numGroups()
{
    return this.group.count();
}

function BuddyList_numOnline()
{
    var count=0;
    for (var i =0; i < this.buddy.length; i++)
    {
        if (!this.buddy[i])
            continue;
        
        if (this.buddy[i].status.code.valueOf() != 'offline')
        {
            count++;
        }
    }
    return count;
}

// End of BuddyList.js
