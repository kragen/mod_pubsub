/*! @file kn_buddylist.js PubSub Buddy List Component
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_buddylist");
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

// $Id: kn_buddylist.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $

kn_include("kn_activepanels");
kn_include("kn_htmlsafe");
kn_include("kn_defaults");

kn_createContext("kn_buddylist");

kn_buddylists = new Object();

/*!
 * Creates the HTML needed to render the Buddy List component.
 *
 * @tparam string id a unique identifier for this Buddy List
 * @tparam int w the desired width of the Buddy List, in pixels
 * @tparam int h the desired height of the Buddy List, in pixels
 * @tparam string selectionsTopic If specified, list selections are published as events to this topic.
 * @treturn kn_buddylist_obj an object reference to the newly-created ActivePanel
 */
function kn_buddylist(id, w, h, selectionsTopic)
{
  document.write(kn_buddylist_createHeader(id));
  var ap = new kn_activepanel(id,w,h,true);
  document.write(kn_buddylist_createFooter(id));
  var obj = new kn_buddylist_obj(ap, selectionsTopic);
  obj.id = id;
  obj.win = self;
  kn_onload_addHandler(function () { obj.init(); });
  kn_buddylists[id] = obj;
  return obj;
}

function kn_buddylist_createHeader(id)
{
  var html = new Array();

  html[html.length] = "<form onsubmit=\"kn_buddylists['" + id + "'].changeStatus(this.elements.my_status.value); return false;\" action=\"javascript:void(0);//\" name=\"kn_buddylist_form_" + id + "\">";

  html[html.length] = "<input name=\"edit_button\" disabled=\"disabled\" type=\"button\" onclick=\"kn_buddylists['" + id + "'].editSelectedBuddy(); return false;\" value=\"" + kn_buddylist$("Edit") + "\">&nbsp;";

  html[html.length] = "<input name=\"rm_button\" disabled=\"disabled\" type=\"button\" onclick=\"kn_buddylists['" + id + "'].rmSelectedBuddy(); return false;\" value=\"" + kn_buddylist$(" - ") + "\">&nbsp;";

  html[html.length] = "<input name=\"add_button\" type=\"button\" onclick=\"kn_buddylists['" + id + "'].editBuddy(); return false;\" value=\" + \">&nbsp;";

  html[html.length] = $_("%{0} is %{1}",
                     kn_htmlsafe_visibleEscape(kn.userid),
                     "<input name=\"my_status\" type=\"text\" size=\"10\" />&nbsp;");

  html[html.length] = "<input type=\"submit\" value=\"" + kn_buddylist$("Change") + "\" />&nbsp;";

  html[html.length] = "</form>";
  return (html.join(""));
}

function kn_buddylist_createFooter(id)
{
  var html = new Array();
  return (html.join(""));
}

/*! @class kn_buddylist_obj
 * This object provides the PubSub Buddy List interface.<p>This constructor
 * should never be called directly from application code. Use
 * kn_buddylist() instead to create an instance of the ActivePanel
 * component.
 * @ctor An object reference that provides the PubSub Buddy List API.
 * @tparam kn_activepanel_obj ap an ActivePanel to serve as the rendering component
 * @tparam string selectionsTopic If specified, list selections are published as events to this topic.
 */
function kn_buddylist_obj(ap, selectionsTopic)
{
  this.ap = ap;
  /*!
   * A hash containing the collection of buddies for this list.
   * @type object
   */
  this.buddies = {};
  this.buddy_status = {};
  this.buddy_aka = {};
  /*!
   * Contains the userid of the currently selected buddy.
   * @type string
   */
  this.selectedID = null;

  this.rmSelectedBuddy = kn_buddylist_rmSelectedBuddy;
  this.onBuddy = kn_buddylist_onBuddy;
  this.onBuddyStatus = kn_buddylist_onBuddyStatus;
  this.drawBuddies = kn_buddylist_drawBuddies;
  this.selectBuddy = kn_buddylist_selectBuddy;
  this.deselectBuddy = kn_buddylist_deselectBuddy;
  this.init = kn_buddylist_init;
  this.editSelectedBuddy = kn_buddylist_editSelectedBuddy;
  this.editBuddy = kn_buddylist_editBuddy;
  this.getForm = kn_buddylist_getForm;
  
  // Topic map.
  this.conf_topic = "/who/" + kn.userid + "/apps/yew";
  this.buddy_topic = this.conf_topic + "/buddies";
  this.status_topic = this.conf_topic + "/status";
  this.buddy_status_topic = this.buddy_topic + "/status";
  this.buddy_selections_topic =  selectionsTopic;
}

function kn_buddylist_init()
{
  var finisher = new kn_browser_wrapper(this, "drawBuddies");
  finisher.onStatus = finisher.onMessage;

  // Get default pop-up.
  
  this.htmlPath = kn_defaults_get("kn_buddylist","HtmlPath",kn_browser_includePath + "html/");
  
  // Subs.
        
  kn_subscribe(this.buddy_topic, new kn_browser_wrapper(this, "onBuddy"), {do_max_age: "infinity"});
  kn_subscribe(this.status_topic, new kn_browser_wrapper(this, "onBuddyStatus"), {do_max_age: "infinity"}, finisher);
  kn_subscribe(this.buddy_status_topic, new kn_browser_wrapper(this, "onBuddyStatus"), {do_max_age: "infinity"});

  return this;
}

/*!
 * Changes and broadcasts the user's status
 * @tparam string status The user status as a string.
 * @treturn public:void Nothing.
 */
kn_buddylist_obj.prototype.changeStatus = function(status)
{
  kn.publish("/who/" + kn.userid + "/apps/yew/status",
      {
        kn_id: kn.userid,
        userid: kn.userid,
        displayname: kn.displayname,
        kn_payload: status,
        kn_expires: "infinity"
      });
}

/*!
 * Adds a buddy with given userid and displayname to buddy list in a particular named category
 * @tparam string cat The Buddy category as a string.
 * @tparam string userid The user ID as a string.
 * @tparam string displayname The user displayname as a string.
 * @tparam statusHandlers aStatusHandlers An optional object to handle status responses from the server.
 * @treturn public:void Nothing.
 */
kn_buddylist_obj.prototype.addBuddy = function (cat, userid, displayname, aStatusHandlers)
{
    if (!aStatusHandlers) aStatusHandlers = {};
    var e = new Object();
    e.kn_id = userid;
    e.kn_payload = kn_subscribe('/who/' + userid + '/apps/yew/status',
                                this.buddy_status_topic,
                                {do_max_age: "infinity"});
    e.who = displayname;
    e.where = cat;
    e.rid = e.kn_payload; // This is a hack to avoid using old-style buddies.
    e.kn_expires = "infinity";
    kn.publish(this.buddy_topic, e, aStatusHandlers);
}

/*!
 * Removes buddy with given userid from buddy list
 * @tparam string userid The user ID as a string.
 * @tparam statusHandlers aStatusHandlers An optional object to handle status responses from the server.
 * @treturn public:void Nothing.
 */
kn_buddylist_obj.prototype.rmBuddy = function (userid, aStatusHandlers)
{
    if (!aStatusHandlers) aStatusHandlers = new Object();   
    if (this.buddies[userid])
    {
        if (!confirm($_("Are you sure you want to remove %{0}?",
                                    this.buddies[userid].who)))
        {
            return;
        }
        
        if (this.buddies[userid]['rid'] == this.buddies[userid].kn_payload)
        {
            kn.unsubscribe(this.buddies[userid].kn_payload);
        }
        
        var ev = new Object();
        ev.kn_id = userid;
        ev.kn_payload = "";
        kn.publish(this.buddy_topic, ev, new Object(), aStatusHandlers);

    }
}

function kn_buddylist_onBuddy(e)
{
  if (! e.kn_id) return;
    if (e.kn_payload == '' || e.kn_deleted == "true")
    {
        if (this.buddies[e.kn_id] != null)
        {
          if (e.kn_id == this.selectedID)
          {
            this.deselectBuddy(e.kn_id,true);
          }
          delete this.buddies[e.kn_id];
        }
    }
    else
    {    
        this.buddies[e.kn_id] = e;
    }
    this.drawBuddies();
}

function kn_buddylist_getForm()
{
  return this.win.document.forms["kn_buddylist_form_" + this.id];
}

function kn_buddylist_onBuddyStatus(e)
{
  if (! e.kn_id) return;
  if (e.kn_id == kn.userid)
  {
    var form = this.getForm();
    if (form)
    {
        var myOldStatus = form.elements.my_status.value;
        var myNewStatus =
            ((e.kn_deleted == "true") ?
             "" :
             (e.kn_payload || ""));
        if (myOldStatus != myNewStatus)
        {
            form.elements.my_status.value = myNewStatus;
        }
    }
  }
  var oldDeleted = ! this.buddy_status[e.kn_id];
  var newDeleted = (e.kn_deleted == "true") || ! e.kn_payload;
  var oldStatus = (this.buddy_status[e.kn_id] && this.buddy_status[e.kn_id].kn_payload) || "";
  var newStatus = e.kn_payload || "";
  var oldAKA = this.buddy_aka[e.kn_id] || e.kn_id;
  var newAKA = e.displayname || e.kn_id;
  this.buddy_status[e.kn_id] = e;
  this.buddy_aka[e.kn_id] = newAKA;
  if (newDeleted) delete this.buddy_status[e.kn_id];
  if (oldStatus != newStatus || oldAKA != newAKA || oldDeleted != newDeleted)
  {
    this.drawBuddies();
  }
}

// NOTE: drawBuddies must be ready to accept (and ignore) an event argument.
function kn_buddylist_drawBuddies()
{

    // update my status field
    {
        var e = this.buddy_status[kn.userid] || new Object;
        var form = this.getForm();
        if (form)
        {
            var myOldStatus = form.elements.my_status.value;
            var myNewStatus =
                ((e.kn_deleted == "true") ?
                 "" :
                 (e.kn_payload || ""));
            if (myOldStatus != myNewStatus)
            {
                form.elements.my_status.value = myNewStatus;
            }
        }
    }

  var str = "";
  var groups = new Object;
  for (var i in this.buddies)
  {
    var where = this.buddies[i].where ||
        kn_buddylist$("default");
    if (! groups[where])
    {
      groups[where] = new Array;
    }
    groups[where][groups[where].length] = i;
  }
  var groupList = new Array;
  for (var groupName in groups)
  {
    groupList[groupList.length] = groupName;
    groups[groupName].sort();
  }
  groupList.sort();
  var html = new Array;
  for (var groupIndex = 0; groupIndex < groupList.length; groupIndex ++)
  {
    var groupName = groupList[groupIndex];
    var group = groups[groupName];
    html[html.length] = "<span class=\"buddylist-category\">" +
      kn_htmlsafe_visibleEscape(groupName) +
      "</span><br />";
    for (var buddyIndex = 0; buddyIndex < group.length; buddyIndex ++)
    {
      var buddyID = group[buddyIndex];
      var buddy = this.buddies[buddyID];

      html[html.length] = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
      var linkHTML =
          "<a";
      if (this.selectedID != buddyID)
          linkHTML +=
              " href=\"#\" target=\"_self\" onclick=\"kn_buddylists['" +
              this.id +
              "'].selectBuddy(kn_unescape('" +
              kn_htmlEscape(kn_escape(buddyID)) +
              "'));return false;\"";

      linkHTML += ">";
      
      var who = buddy.who || buddyID;
      
      linkHTML += kn_htmlsafe_visibleEscape(who);
          
      linkHTML += "</a>";
      if (this.selectedID == buddyID)
      {
        linkHTML = "<span class=\"buddylist-selected\">" + linkHTML + "</span>";
      } else
                        {
                                linkHTML = "<span class=\"buddylist-normal\">" + linkHTML + "</span>";
                        }
      html[html.length] = linkHTML;

      var aka = this.buddy_aka[buddyID];
      if (aka && aka != who)
      {
        html[html.length] = " a.k.a. " + kn_htmlsafe_htmlSanitize(aka);
      }
      
      if (this.buddy_status[buddyID] && this.buddy_status[buddyID].kn_payload)
      {
        html[html.length] = ":&nbsp;";
        html[html.length] = kn_htmlEscape(this.buddy_status[buddyID].kn_payload);
      }
      html[html.length] = "<br />";
    }
  }
  if (this.ap.clear)
      this.ap.clear();
  if (this.ap.write)
      this.ap.write(html.join(""));
  if (this.ap.update)
      this.ap.update();
}

function kn_buddylist_selectBuddy(id)
{
  if (!this.buddies[id] || this.selectedID == id) return;
  this.selectedID = id;
  
  this.win.document.forms["kn_buddylist_form_" + this.id].elements.edit_button.disabled = false;
  this.win.document.forms["kn_buddylist_form_" + this.id].elements.rm_button.disabled = false;
  
  this.drawBuddies();
  
  if (this.buddy_selections_topic)
  {
    var buddy = this.buddies[id];
    var e = new Object();
    e.kn_id = "1";
    e.who = buddy.who;
    e.where = buddy.where;
    e.kn_expires = "infinity";
    e.kn_payload = id;
    kn.publish(this.buddy_selections_topic,e);
  }
}

function kn_buddylist_deselectBuddy(id,noDraw)
{
  if (!this.buddies[id] || this.selectedID != id) return;
  this.selectedID = null;
  this.win.document.forms["kn_buddylist_form_" + this.id].elements.edit_button.disabled = true;
  this.win.document.forms["kn_buddylist_form_" + this.id].elements.rm_button.disabled = true;
  if (!noDraw) this.drawBuddies();
  
  if (this.buddy_selections_topic)
  {
    var buddy = this.buddies[id];
    var e = new Object();
    e.kn_id = "1";
    e.kn_expires = "+30";
    e.kn_payload = "";
    kn.publish(this.buddy_selections_topic,e);
  }
}

kn_buddy_popups = {};

function kn_buddylist_editSelectedBuddy()
{
  
  if (! this.selectedID)
  {
    alert(kn_buddylist$("Please select a buddy to edit."));
    return;
  }
  this.editBuddy(this.selectedID);
}

function kn_buddylist_rmSelectedBuddy()
{
  if (! this.selectedID)
  {
    alert(kn_buddylist$("Please select a buddy to remove."));
    return;
  }
  this.rmBuddy(this.selectedID);
}
  
function kn_buddylist_editBuddy(id)
{

  //var url = kn_resolvePath(location.pathname, this.htmlPath + "buddylist_edit.html");
  
  var url = location.protocol + '//' + location.host + kn_resolvePath(location.pathname, this.htmlPath + "buddylist_edit.html");
  
  if (id)
  {
    
    if (typeof(kn_buddy_popups[id]) != "undefined" && kn_isReady(kn_buddy_popups[id]))
    {
      kn_buddy_popups[id].focus();
    } else
    {
      url += "?id=" + kn_escape(id) + ";kn_browser_uid=" + kn_browser_uid();
                        if (location.search!="") url += ";" + location.search.substring(1);
      if (kn__debug()) url += ";kn_debug";
      kn_buddy_popups[id] =
        kn_popwindow(url, "buddy_edit_popup_" + this.id + "_" + id,200,250,200,200);
    }
  }
  else
  {
        url+= "?kn_browser_uid=" + kn_browser_uid()
                                if (location.search!="") url += ";" + location.search.substring(1);
        if (kn__debug()) url += ";kn_debug";
        kn_popwindow(url, "buddy_add_popup_" + this.id + "_" + Math.random().toString().split(".").join("_"),200,250,200,200);
  }
  kn_buddylist_setOnUnload();
}

function kn_buddylist_setOnUnload()
{
    if (self.kn_buddylist_onunload) return;
    self.kn_buddylist_user_onunload = self.onunload;
    var recursive_onunload = false;
    self.kn_buddylist_onunload = self.onunload = function ()
    {
        var retval = void null;
        if (recursive_onunload) return;
        recursive_onunload = true;
        
        for (var i in kn_buddy_popups)
        {
          var win = kn_buddy_popups[i];
          if (win && ! win.closed)
          {
              win.close();
          }
        }
        
        if (self.kn_buddylist_user_onunload &&
            typeof(self.kn_buddylist_user_onunload) == "function")
        {
            if (self.onunload != self.kn_buddylist_user_onunload)
            {
                retval = self.kn_buddylist_user_onunload();
            }
        }
        
        recursive_onunload = false;
        return retval;
    };
}

// End of kn_buddylist.js
