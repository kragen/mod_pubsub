/*! @file kn_im.js PubSub Instant Messenger Component
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_im");
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

// $Id: kn_im.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $

kn_include("kn_buddylist");

kn_ims = new Object();

/*!
 * Creates the HTML needed to render the Instant Messenger client.
 *
 * @tparam string id a unique identifier for this client
 * @tparam int w the desired width of the client's buddy list, in pixels
 * @tparam int h the desired height of the client's buddy list, in pixels
 * @tparam string selectionsTopic (optional) If specified, list selections are published as events to this topic.
 * @treturn kn_buddylist_obj an object reference to the newly-created ActivePanel
 */
function kn_im(id,w,h,selectionsTopic)
{
  if (!selectionsTopic)
  {
    selectionsTopic = "/who/" + kn.userid + "/apps/im/selections/" + kn_browser_uid();
  }
  var im = new kn_im_obj(id,w,h,selectionsTopic);
  document.write(kn_im_createUI(id));
  im.win = self;
  kn_ims[id] = im;
  return im;
}

function kn_im_obj(id,w,h,selectionsTopic)
{
  this.id = id;
  this.buddylist = new kn_buddylist(id,w,h,selectionsTopic);
  this.selections_topic = selectionsTopic;
  this.invites_topic = "/who/" + kn.userid + "/apps/im/invites";
  this.sessions_topic = "/who/" + kn.userid + "/apps/im/";
  this.session_windows = new Object();
  this.sessions = new Object();
  this.doSetup = kn_im_doSetup;
  this.onSelection = kn_im_onSelection;
  this.onInvite = kn_im_onInvite;
  this.doChatSession = kn_im_doChatSession;
  this.onSessionMsg = kn_im_onSessionMsg;
  this.doSessionWindow = kn_im_doSessionWindow;
  this.htmlPath = kn_defaults_get("kn_im","HtmlPath",kn_browser_includePath + "html/");
  return this;
}

function kn_im_createUI(id)
{
  var str = "<form name=\"kn_im_form_" + id + "\"><input name=\"chat_button\" disabled=\"disabled\" type=\"button\" value=\"Chat\" onclick=\"kn_ims[kn_unescape('" + kn_escape(id) + "')].doChatSession();\" /></form>"
  return str;
}

function kn_im_doSetup()
{
  // Listen for selections.
  kn.subscribe(this.selections_topic, new kn_browser_wrapper(this, "onSelection"), {do_max_n:1});
  // Listen for invites.
  kn.subscribe(this.invites_topic, new kn_browser_wrapper(this, "onInvite"));
}

function kn_im_onSelection(e)
{
  this.selectedID = e.kn_payload;
  if (this.selectedID)
  {
    this.win.document.forms["kn_im_form_" + this.id].elements.chat_button.disabled = false;
  } else
  {
    this.win.document.forms["kn_im_form_" + this.id].elements.chat_button.disabled = true;
  }
}

function kn_im_doSessionWindow(session,id)
{
  var who = id;
  if (this.buddylist.buddies[id] &&
      this.buddylist.buddies[id].who)
  {
    who = this.buddylist.buddies[id].who;
  }
  
  //var url = kn_resolvePath(location.pathname, this.htmlPath + "chat_session.html");
  var url = location.protocol + '//' + location.host + kn_resolvePath(location.pathname, this.htmlPath + "chat_session.html");
  url += "?kn_chatletTopic=" + kn_escape(session) + ";id=" + kn_escape(id);
  url += ";who=" + kn_escape(who);
  url += ";kn_browser_uid=" + kn_browser_uid();
        if (location.search != "")
        {
                url += ";" + location.search.substring(1);
        }
  if (kn__debug()) url += ";kn_debug";
  
  if (typeof(this.session_windows[session])=="undefined" || this.session_windows[session].closed)
  {
    this.session_windows[session] = kn_popwindow(url, "im_session_popup_" + 
      this.id + "_" + kn_browser_uid(),290,290,200,200,true,true);
  } else
  {
    this.session_windows[session].focus();
  }
}

function kn_im_doChatSession()
{
  if (!this.selectedID) return;
  
  var session;

  // See if we already have a session entry for this buddy.
  for (var i in this.sessions)
  {
    if (i.indexOf(this.selectedID) > -1)
    {
      session = i;
    }
  }
  
  if (!session)
  {
    // Generate the chat session topic.
    session = this.sessions_topic + this.selectedID + "/" + kn_browser_uid();
  }
 
  // Pop the session window.
  this.doSessionWindow(session, this.selectedID);
  
  // Publish an invitation from is supplied by userid (from the pubsub client library).
  var e = new Object();
  e.kn_payload = session;
  kn.publish("/who/" + this.selectedID + "/apps/im/invites",e);

  // Invite ourselves to the chat session. This allows the client
  // to repop session windows when the remote user sends a late message.
  this.onInvite(e);
}

function kn_im_onInvite(e)
{
  // Subscribe to session.
  if (!this.sessions[e.kn_payload])
  {
    this.sessions[e.kn_payload] = kn.subscribe(e.kn_payload, new kn_im_session(this,e.kn_payload), {do_max_n:1}); 
  }
}

function kn_im_session(im,topic)
{
  this.im = im;
  this.topic = topic;
}

kn_im_session.prototype.onMessage = function(e)
{
  this.im.onSessionMsg(e,this.topic);
}

function kn_im_onSessionMsg(e,session)
{
  this.doSessionWindow(session,e.userid);
}

function kn_im_init()
{
  for (var i in kn_ims)
  {
    kn_ims[i].doSetup();
  }
}

if (typeof(kn_onload_addHandler) == "function")
{
  kn_onload_addHandler(kn_im_init);
}

// End of kn_im.js
