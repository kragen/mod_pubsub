/*! @file kn_chatlet.js PubSub Chatlet Component
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_chatlet");
 * &lt;/script&gt;</b>
 * </pre>
 */

// Copyright 2003-2004 KnowNow, Inc.  All rights reserved.
 
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

// $Id: kn_chatlet.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $

kn_include("kn_defaults");
kn_include("kn_htmlsafe");
kn_include("kn_activepanels");

var kn_chatlets = [];

/*!
 * Creates the HTML needed to render the Chatlet component.
 *
 * @tparam string id a unique identifier for this chatlet
 * @tparam int w the desired width of the chatlet, in pixels
 * @tparam int h the desired height of the chatlet, in pixels
 * @tparam string topic The topic that this chatlet will communicate on.
 * @treturn kn_chatlet_obj an object reference to the newly-created uninitialized Chatlet
 */
function kn_chatlet(id,w,h,topic)
{
  // if (!topic)
  // {
  // topic = kn_defaults_get("kn_chatlet","Topic","/what/chat");
  // }
                
  var chatlet_obj = new kn_chatlet_obj(id,w,h,topic);
                
  kn_chatlet_createTop(id,w);
  var ap = kn_activepanel(id + "-ap",w,h,true);
  kn_chatlet_createBottom(id,w);
  chatlet_obj.ap = ap;
  kn_chatlets[id] = chatlet_obj;  
}

function kn_chatlet_obj(id,w,h,topic)
{
  this.id = id
  this.uid = "kn_chatlet_" + Math.random().toString().split(".").join("");
  this.sub_default_topic = kn_defaults_get("kn_chatlet","Topic","/what/chat");
  this.topic = topic || this.sub_default_topic;
  this.sub_max_age = kn_defaults_get("kn_chatlet","MaxAge","+180");
  this.sub_max_n = kn_defaults_get("kn_chatlet","MaxN","10");
  this.chatlog = [];
  this.onMessage = kn_chatlet_onMessage;
  this.onSuccess = kn_chatlet_onSuccess;
  this.doUpdate = kn_chatlet_doUpdate;
  this._gotSuccess = false;
  return this;
}

function kn_chatlet_createTop(id,w)
{
  var str = "<table border='0' class=\"chatlet-outerborder\" id=\""+id+ "\-outerborder\"><tr><td valign=\"top\" class=\"chatlet-display\" id=\""+id+"-display\" width=\""+w+"\">";
  document.write(str);
}

function kn_chatlet_createBottom(id,w)
{

  var chatref = "kn_chatlets['" + id + "']";

  var html = [];
  html[html.length] = "</td></tr>";
  html[html.length] = "<tr><td valign=top class=\"chatlet-ui\" id=\""+ id +"-ui\">";
  html[html.length] = "<form action=\"#\" name=\"" + id + "_controls\" onsubmit=\"return false;\">";
  html[html.length] = "User: <input class=\"chatlet-username\" id=\""+id+"-username\" name=\"displayname\" value=\"*\" size=\""+Math.round(w/20)+"\"> ";
  html[html.length] = "<a href=\"#\" class=\"chat-button\" title=\"Bold\" ";
  html[html.length] = "onclick=\""+ chatref+".text_form.kn_payload.value = kn_chatlet_bold(" + chatref+".text_form.kn_payload.value); "+ chatref+".text_form.kn_payload.focus(); return false;\">B</a>&nbsp;";
  html[html.length] = "<a href=\"#\" class=\"chat-button\" title=\"Italicize\" ";
  html[html.length] = "onclick=\""+ chatref+".text_form.kn_payload.value = kn_chatlet_italics("+ chatref+".text_form.kn_payload.value); "+ chatref+".text_form.kn_payload.focus(); return false;\">I</a>&nbsp;";

  html[html.length] = "<a href=\"#\" class=\"chat-button\" title=\"Clear Screen\" onclick=\"kn_chatlet_clearChat('"+id+"'); return false;\">Clear</a>";

  html[html.length]= "</form>";
  html[html.length] = "<form action=\"#\" name=\"" + id + "_text\" onsubmit=\"kn_chatlet_sendChat('"+id+"'); return false;\">";
  html[html.length] = "<br class=\"chatlet-linebreak\" id=\""+id+"-linebreak\"> <input class=\"chatlet-payload\" id=\""+id+"-payload\" type=\"test\" size=\""+Math.round(w/20)+"\" name=\"kn_payload\">&nbsp;";
  html[html.length] = "<a href=\"#\" class=\"chat-button\" title=\"Send\" onclick=\"kn_chatlet_sendChat('"+id+"'); return false;\">Send</a>";
  html[html.length] = "</form></td></tr></table>";
  html = html.join("");
    
  document.write(html);
}

function kn_chatlet_init()
{
  if (!kn.isLoadedP_)
  {
    self.setTimeout("kn_chatlet_init();",50);
    return;
  }
    
  for (var i in kn_chatlets)
  {
    var chatlet = kn_chatlets[i];
        
    chatlet.ctrl_form = document.forms[chatlet.id + "_controls"];
    chatlet.text_form = document.forms[chatlet.id + "_text"];
    chatlet.ctrl_form.elements["displayname"].value = kn_htmlEscape(kn_displayname);
        
    chatlet._rid = kn.subscribe(
        chatlet.topic,
        chatlet,
        ( { do_max_age: chatlet.sub_max_age,
            do_max_n: chatlet.sub_max_n
        } ),
        chatlet);
    }
}

if (typeof(kn_onload_addHandler) == "function")
{
  kn_onload_addHandler(kn_chatlet_init);
}

var kn_chatlet_eventIDCache = { };
var kn_chatlet_typeMap = { };

kn_chatlet_onMessage = function(e)
{
  var class_type;
  var displayname = null, payload = null, uid=null;
  var id = null;
  var time_t = null;
  var html = [];

  uid = e.uid;
  displayname = e.displayname;
  payload = e.kn_payload;
  id = e.kn_id;
  time_t = e.kn_time_t; // this field is not present @ local echo

  if (e.deleted == 'true')
  {
    return;
  }

  if (uid == this.uid)
  {
    class_name = 'chatlet-local-user';
  }
  else if (displayname == "SYSTEM")
  {
    class_name = 'chatlet-system-msg';
  }
  else
  {
    class_name = 'chatlet-remote-user';
  }
   
   html[html.length] = "<span class=\"" + class_name + "\">";
   html[html.length] = kn_htmlsafe_htmlSanitize(displayname);
   html[html.length] = "</span>:&nbsp;" + kn_htmlsafe_htmlSanitize(payload,true,true);
   html[html.length] = "<br>";
   html = html.join("");
   
   this.chatlog[this.chatlog.length] = html;
   if (this._gotSuccess)
   {
      this.doUpdate();
   }
}

function kn_chatlet_onSuccess()
{
  this._gotSuccess = true;
  this.doUpdate();
}

function kn_chatlet_doUpdate()
{
   if (!this.ap.isMouseOver()) {
      html = this.chatlog.join("");
      this.ap.clear();
      this.ap.write(html);
      this.ap.update();
      this.ap.scrollToBottom();
   } else {
      self.setTimeout("kn_chatlets['" + this.id + "'].doUpdate();",1000);
   }
}

function kn_chatlet_sendChat(id)
{
  var chatlet = kn_chatlets[id];
  var e = new Object();
  e.displayname = chatlet.ctrl_form.elements["displayname"].value;
  e.kn_payload = chatlet.text_form.elements["kn_payload"].value;
  e.uid = chatlet.uid;
  e['content-type'] = 'text/html';
  e.kn_id = kn.publish(chatlet.topic, e, new Object());
  chatlet.text_form.kn_payload.value = ''; // Clear payload.
  chatlet.text_form.kn_payload.focus();
  return false;
}

function kn_chatlet_clearChat(id)
{
  var chatlet = kn_chatlets[id];
  chatlet.chatlog = []; // wax message log
  chatlet.doUpdate();
  return false;
}

function kn_chatlet_systemMessage(msg,id)
{
  var chatlet = kn_chatlets[id];
  var e = {};
  e.displayname = "SYSTEM";
  e.kn_payload = msg;
  chatlet.onMessage(e);
}

function kn_chatlet_error_handler(e,id)
{
  alert("Error: "  + e.status + "<br>" + e.kn_payload);
}

// We must provide our own versions of bold() and italics() instead of
// using the intrinsic String methods becuase those methods are
// incredibly buggy in Netscape 6/Mozilla...

/*!
 * Encloses a string in HTML boldface tags
 *
 * @tparam string aString The string to be boldfaced.
 * @treturn string The boldfaced string.
 */
function kn_chatlet_bold(aString) { return '<b>' + aString + '</b>'; }

/*!
 * Encloses a string in HTML italics tags
 *
 * @tparam string aString The string to be italicized.
 * @treturn string The italicized string.
 */
function kn_chatlet_italics(aString) { return '<i>' + aString + '</i>'; }

// End of kn_chatlet.js
