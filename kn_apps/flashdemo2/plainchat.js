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

// $Id: plainchat.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $

// Add some chat code now... we'll ignore how messy all of this is...
// Yeah... we will.... 

open_windows = new Array();
open_chats = new Array();
chat_sessions = new Array();

// Define a bunch of stuff that lets you change the presentation of chat lines.

var name_pre = " <b>";
var name_post = "</b>: ";
var msg_pre = "";
var msg_post = "</font>";
var line_end = "<br>";
var date_pre = "<font size='1'>(";
var date_post = ")";
var date_format = "now = new Date();now.getHours() + ':' + now.getMinutes() + ':' + now.getSeconds()";
var body_open = "<body marginheight=2 marginwidth=2 topmargin=2 leftmargin=2>";
var body_close = "</body>";

// Set the window name so we can refer back to it later.
window.name = "robot_commander";

function init_chat(){
    chat_topic = "/who/" + kn.userid + "/apps/robots/im_inbox";
    kn.subscribe(chat_topic, handleMessage);
}

function check_sessions(userid){

    for (i=0; i < chat_sessions.length; i++){
        if (chat_sessions[i] == userid){
            if (open_windows[i] != void 0 && open_windows[i].closed){
                return -1;
            } else {
                return i;
            }
        } else {
            // nothing!
        }
    }

    // Send back -1 if the user is not found. this is only executed if
    // the above doesn't return anything.

    chat_sessions[chat_sessions.length] = userid;
    return -1;
}

function handleMessage(event){

    if (check_sessions(event.userid) > -1){

        // Window is already open, send the messages there.
        display_message(event);

    } else {

        // Open a window and begin the chat love!
        open_chat_window(event);
        open_chats[check_sessions(event.userid)] = "";

        // This was an attempt to use KNDocument... but it failed.
        // setTimeout("open_chats[check_sessions(event.userid)] = new KNDocument(open_windows[check_sessions(event.userid)].frames[0]);",1);
        // setTimeout("open_chats[check_sessions(event.userid)].open();",2)
    }
}

function open_chat_window(event){
    open_windows[check_sessions(event.userid)] = window.open("chatwindow.html?" + event.userid + "," + escape(event.kn_payload),event.userid + "_window","width=300,height=200,location=no,status=yes,menubar=no,resizable=yes")
}

function display_message(event){

    if (event.kn_payload != ""){

        if (event.from == void 0){
            who = event.userid;
        } else {
            who = event.from;
        }

        // Construct the message.
        message = (date_pre + eval(date_format) + date_post + 
               name_pre + who + name_post + 
               msg_pre + event.kn_payload + msg_post +
               line_end);

        // Draw it.
        //open_chats[check_sessions(event.userid)].writeln(message);
        //open_windows[check_sessions(event.userid)].frames[0].document.write(open_chats[check_sessions(event.userid)].html);

        open_chats[check_sessions(event.userid)] = message + open_chats[check_sessions(event.userid)];

        win = open_windows[check_sessions(event.userid)];
        win.frames[0].document.open();
        win.frames[0].document.write("<html>" + body_open + open_chats[check_sessions(event.userid)] + body_close + "</html>");
        win.frames[0].document.close();
        
    } else {
        open_windows[check_sessions(event.userid)].focus();
    }
}

function publish_message(who,text){

    var pub_msg = new Object();
    pub_msg.kn_payload = text;
    pub_msg.topic = "/who/" + who + "/apps/robots/im_inbox"

    // Send it out to the recipient.
    kn.publish(pub_msg.topic, pub_msg);
    
    // Publish it locally.
    display_message({userid:who, kn_payload:text, from:kn.userid});
}

// End of plainchat.js
