/*! @file kn_popup.js JavaScript PubSub Library Pop-Up Window Transport
 * <pre>self.kn_include_plugin("kn_popup"); // in self.kn_config()</pre>
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

// $Id: kn_popup.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $

kn_include("kn_defaults");
function kn_popup_repop()
{
    if (!self.kn_repop_ok) return;
    self.kn_repop_ok = false;
    var winopts;
    self.kn_popup_sid = Math.random().toString().substr(2,8);
    if (kn__debug())
    {
      winopts = ",width=400,height=200,resizable=yes;toolbars=yes";
    } else
    {
      winopts = ",width="+ kn_popup_win_w + ",height=" +
                kn_popup_win_h + ",resizable=no,scrollbars=no";
    }
    kn_popup_proxyWin = self.open(kn_popup_url + "?kn_popup_sid=" + self.kn_popup_sid,
                   name + "_stub",
                   ("screenX=400" +
                    ",screenY=400" +
                    ",left=400" +
                    ",top=400" +
                    ",scrollbars=no" +
                    winopts));
    
    //kn_popup_proxyWin.opener = self;
    
    if (!kn__debug() && kn_isReady(kn_popup_proxyWin))
    {
        kn__try(new Function('kn_popup_proxyWin.blur();'));
    }
    
    // Netscape Navigator 4.x has a serious setTimeout bug, so we
    // focus immediately there; other browsers (Netscape
    // 6/Mozilla, at least) need to run the focus() from a
    // setTimeout to avoid overriding it with the window.open().

    if (!kn__debug())
    {
      if (navigator.appVersion.charAt(0) == '4')
      {
          self.focus();
      }
      else
      {
          self.setTimeout('focus()',1);
      }
    }
    var user_onunload = self.onunload;
    var recursive_onunload = false;
    self.kn_onunload = self.onunload = function ()
    {
        kn__framesetOnUnload();
        var retval = void null;
        if (recursive_onunload) return;
        recursive_onunload = true;
        self.kn_repop_ok = false;
        if (kn_popup_proxyWin && ! kn_popup_proxyWin.closed)
        {
            kn_popup_proxyWin.close();
        }
        if (user_onunload &&
            typeof(user_onunload) == "function")
        {
            if (self.onunload != user_onunload)
            {
                retval = user_onunload();
            }
        }
        recursive_onunload = false;
        return retval;
    };
    self.kn_repop_ok = true;
};

function kn_popup__wrapApp()
{
  
    self.kn_repop_ok = true;
  kn_popup_proxyWin = null;
  kn_popup_url = kn_defaults_get("kn_popup","PopupUrl","popup_proxy.html");
  kn_popup_user_message = kn_defaults_get("kn_popup","UserMessage","popup_usermessage.html");
  
  if (typeof(kn_browser_includePath)!="undefined")
  {
    kn_popup_url = kn_resolvePath(location.pathname,kn_browser_includePath + "html/" + kn_popup_url);
    kn_popup_userMessage = kn_resolvePath(location.pathname,kn_browser_includePath + "html/" + kn_popup_user_message);
  }
  
  kn_popup_win_w = kn_defaults_get("kn_popup","WinWidth",100);
  kn_popup_win_h = kn_defaults_get("kn_popup","WinHeight",100);

  name = kn__uniqueWindowName(); // need this for leader election
  kn_popup_repop();
}

function kn_popup_onLoad()
{
  kn.isLoadedP_ = false; // just to be sure
  kn.tunnelFrame_ = kn_popup_proxyWin.frames[name + "_tunnel"];
  kn.postFrame_ = kn_popup_proxyWin.frames[name + "_post"];
  kn.appFrame_ = self;
  kn.TFN_ = kn.tunnelFrame_.name;

  kn__hookFrames(kn.postFrame_,kn.appFrame_);
  kn.isLoadedP_ = true;
}

if (! self.kn__wrapApp)
{
    // hook ourselves into the pubsub client library
    kn__wrapApp = kn_popup__wrapApp;
}

// End of kn_popup.js
