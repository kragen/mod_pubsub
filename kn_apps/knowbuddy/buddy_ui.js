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

// $Id: buddy_ui.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $

function DocumentProxy(doc)
{
    this.open = DocumentProxy_open;
    this.write = DocumentProxy_write;
    this.close = DocumentProxy_close;
    this.flush = DocumentProxy_flush;
    
    this.doc = doc;
    this.isOpen = false;
    this.buffer = new String();
}

function DocumentProxy_flush()
{
    this.doc.write(this.buffer);
    this.buffer = "";
}

function DocumentProxy_close()
{
    if (this.isOpen)
    {
        this.flush();
    }
    this.doc.close();
    this.isOpen = false;
}

function DocumentProxy_open()
{
    this.isOpen = true;
    this.doc.open();
}

function DocumentProxy_write(t)
{
    this.buffer += t;
}

var helper_script= "var controller = parent.frames.control;" +
        "function doToggleGroup(groupname) {    controller.doToggleGroup(groupname);}  " +
        "function doSelectGroup(groupname,span){    controller.doSelectGroup(groupname,span);}" +
        "function doSelectUser(groupname,username,anchor){  controller.doSelectUser(groupname,username,anchor);}" +
        "function doSendChat(groupname){    controller.doSendChat(groupname);}" +
        "function doSendIM(username){   controller.doSendIM(username);}";

var css="<STYLE>\n"+
        "BODY               { font-family: Arial, sans-serif; font-size: 8pt}\n"+
        "TD                 { font-family: Arial, sans-serif; font-size: 8pt}\n"+
        ".Selected          { background-color: yellow}\n"+
        ".Buddy             { cursor: hand; }\n"+
        ".Buddy .Active     { color: black; font-weight: bold}\n"+
        ".Buddy .Offline    { color: slategray}\n"+
        ".Buddy .Online     { color: black}\n"+
        ".Buddy .Selected   { background-color: yellow}\n"+
        ".Buddy A:hover     { font-weight: bold}\n"+
        ".Buddy A           { }\n"+
        ".Group             { cursor: hand; font-weight: bold; color: black}\n"+
        ".Group .Selected   { background-color: yellow}\n"+
        ".Profile           {}\n"+
        ".Profile .displayname  { font-size: medium; font-weight: bold}\n"+
        ".Profile .userid       {}\n"+
        ".Profile .status       {}\n"+
        "</STYLE>\n";


function drawGroups(theDoc,buddyList)
{
    var doc;
    var group;
    var buddy;
        
    doc = new DocumentProxy(theDoc);
    
    doc.open();

    doc.write("<html>\n<head>\n");
    doc.write("<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n");
    doc.write("\n<script>\n"+helper_script+"\n</"+"script>\n");
    doc.write(css);
    
    doc.write("</head>\n");
    doc.write("<body topmargin='1' leftmargin='2'>");

    doc.write("<table border='0' width='100%'>");
    
    // all groups   
    for (var i=0; i < buddyList.group.length; i++)
    {
        if (!buddyList.group[i])
            continue;
        drawGroup(buddyList,doc,buddyList.group[i]);
    }
    doc.write("</table >");
    
    doc.write("</body></html>");
//  alert(doc.buffer);
    doc.close();
}

// TODO: Make safe for name= attributes.
function makeSafeName(name)
{
    return name;
}

function drawGroup(buddyList,doc,group)
{
    var buddy;
    var buddyref;
    var img;
    
    doc.write("<tr>");
    doc.write("<td colspan='2'>\n");
        
    if (group.isExpanded)
    {
        img = 'img/minus_aol.gif';
    }
    else
    {
        img = 'img/plus_aol.gif';
    }
    doc.write("<a href='javascript:doToggleGroup(\""+group.name+"\")'>");
    doc.write("<img name='img_"+group.name+"' border='0' src='"+img+"' >");
    doc.write("</a>");
    
    // The group header
    doc.write(" <span class='Group' ");
    doc.write(" onClick='doSelectGroup(\""+group.name+"\",this)'");
    doc.write(" onDblClick='doSendChat(\""+group.name+"\")'");
    doc.write(">");
    if (group.$selected)
    {
        doc.write("<span class='Selected'>");
    }
    doc.write("<a class='Group' href='javascript:void 0' onClick='doSelectGroup(\""+group.name+"\",this)'>");
    doc.write(group.name);
    doc.write("</a>");

    if (group.$selected)
    {
        doc.write("</span>");
    }
    doc.write(" ("+group.numOnline()+"/"+group.numBuddies()+")");
    doc.write("</span>");
    doc.write("<br />\n");

    doc.write("</td>\n");
    doc.write("</tr>\n");
    
    // The buddies
    if (group.isExpanded)
    {
        if ((bShowOfflineMembers && (group.numBuddies() > 0)) ||
            (group.numOnline() > 0))
        {
            doc.write("<tr>");
            doc.write("<td width='20pt'>&nbsp;</td>");
            doc.write("<td  width='90%'>\n");
    
            doc.write("<span id='"+group.name+"' class='Buddy'>\n");
        
            for (var i=0; i < group.buddyref.length; i++)
            {
                if (!group.buddyref[i])
                    continue;
                buddyref = group.buddyref[i];
    
                for (var j=0; j < buddyList.buddy.length; j++)
                {
                    if (!buddyList.buddy[j])
                        continue;
                        
                    if (buddyList.buddy[j].person.userid.valueOf() == buddyref.valueOf())
                    {
                        buddy = buddyList.buddy[j];
                        if (group == theOfflineGroup)
                            drawBuddy(doc,group,buddy,buddyref,true);
                        else
                            drawBuddy(doc,group,buddy,buddyref,bShowOfflineMembers);
                        break;
                    }
                }
            }
            doc.write("</span>\n");
        
            doc.write("</td>");
            doc.write("</tr>\n\n");
        }
    }
}

function encodeToJs(text)
{
    if (!text)
        return "";
    return text.replace(/[']/g,"\'");
}

function drawBuddy(doc,group,buddy,buddyref,bShowOfflineMembers)
{
    var style;

    if (!bShowOfflineMembers && 
        (buddy.status.code == 'offline'))
    {
        return;
    }
    if (buddy.status.code == 'invisible')
        return;
    
    // todo: use a status2style lookup table.
    if (buddy.status.active.valueOf())
    {
        style = 'Active';   
    }
    else
    {
        if (buddy.status.code == 'offline')
            style = 'Offline';  
        else
            style = 'Online';   
    }
    
    doc.write("<a class='"+style+"' href='javascript:void 0'");
    doc.write(" title='"+encodeToJs(buddy.status.message)+"'");
    doc.write(" onClick='doSelectUser(\""+group.name+"\",\""+buddy.person.userid+"\",this)' ");
    doc.write(" onDblClick='doSendIM(\""+buddy.person.userid+"\")'");
    doc.write(">");
    if (buddyref.$selected)
    {
        doc.write("<span class='Selected'>");
    }
    doc.write(buddy.person.displayname);
    if (buddyref.$selected)
    {
        doc.write("</span>");
    }
    doc.write("</a>");
    doc.write("<br />\n");
}

function drawProfile(theDoc,buddy)
{
    var doc;

    doc = new DocumentProxy(theDoc);
    
    doc.open();
    doc.write("<html><head>");
    doc.write("<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n");
    doc.write(css);
    doc.write("</head>\n");
    doc.write("<body topmargin='1' leftmargin='2' >");
    doc.write("<basefont face='arial' size='2'>\n");

    // get current object
    if (buddy)
    {
        drawBuddyProfile(doc,buddy);
    }

    doc.write("</body>");
    doc.write("</html>\n"); 
    doc.close();
}


var html_buddyProfile = "" +
"<table class='Profile'>\n" +
"<tr>\n" +
"<td>" +
    "<span class='displayname'>" +
    "&buddy.person.displayname;" +
    "</span> " +
    "<span class='userid'>(" +
    "&buddy.person.userid;" +
    ")</span>" +
"</td>\n" +
"</tr>\n" +
"<tr>\n" +
"<td>" +
    "Status: " +
    "<span class='status'>" +
    "(&buddy.status.code;) &buddy.status.message;" +
    "</span> " +
"</td>\n" +
"</tr>\n" +

"<tr>\n" +
"<td>" +
    "<span class='contact'>" +
        "Phone: " +
        "<span class='phone'>" +
        "&buddy.contact.phone;" +
        "</span> " +
    "</span> " +
"</td>\n" +
"</tr>\n" +

"<tr>\n" +
"<td>" +
    "<span class='contact'>" +
        "Last updated: " +
        "<span class='last_time'>" +
        "&buddy.status.lastContactedOn;" +
        "</span> " +
    "</span> " +
"</td>\n" +
"</tr>\n" +

"</table>\n";

// TODO: Make this code cleaner.
function drawBuddyProfile(doc,buddy)
{
    var html;
    var entities=new Object();
    
    if (!buddy)
        return;
    entities["buddy.person.displayname"] = buddy.person.displayname;
    entities["buddy.person.userid"] = buddy.person.userid;
    entities["buddy.status.code"] = buddy.status.code;
    entities["buddy.status.message"] = buddy.status.message;
    entities["buddy.contact.phone"] = buddy.contact.phone;
    
    if (!buddy.status.lastContactedOn || 
        buddy.status.lastContactedOn.getTime() == 0)
        entities["buddy.status.lastContactedOn"] = "Unknown";
    else
        entities["buddy.status.lastContactedOn"] = buddy.status.lastContactedOn.toString();
        
    html = replaceEntities(html_buddyProfile,entities);
    doc.write(html);
}
  
// End of buddy_ui.js
