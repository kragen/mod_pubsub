/*! @file kn_frameless.js JavaScript PubSub Library Frameless Transport
 * <pre>self.kn_include_plugin("kn_frameless"); // in self.kn_config()</pre>
 */

// Copyright 2002-2003 KnowNow, Inc.

// @KNOWNOW_LICENSE_START@

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

// @KNOWNOW_LICENSE_END@

// $Id: kn_frameless.js,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

function kn_frameless_createIFrame(frameID)
{
    if (!document.createElement)
    {
        return false;
    }
    
    if (navigator && 
        navigator.userAgent &&
        navigator.userAgent.indexOf("Gecko") > -1)
    {
      return kn_frameless_moz_createIFrame(frameID);
    }
    
    var IFrameDoc, IFrameObj, doc; 
    var iframeHTML = "\<iframe name=\""+ frameID +"\" id=\"" + frameID + "\" style=\"";
    if (kn__debug())
    {
        iframeHTML += "border:1px solid #000; width:150px; height:100px;";
    } else
    {
        iframeHTML += "border:none; width:0px; height:0px;";
    }
    iframeHTML += "\" src=\"" + kn_blank + "\" scrolling=\"no\"><" + "/iframe" + ">";
    document.body.innerHTML += iframeHTML;
    IFrameObj = document.getElementById(frameID);
    if (!IFrameObj.document)
    {
        IFrameObj.document = kn_frameless_findFrameDoc(IFrameObj);
    }
    return IFrameObj;
}

function kn_frameless_moz_createIFrame(frameID)
{
  var iframe = document.createElement("iframe");
  if (kn__debug())
  {
      iframe.setAttribute("style","border:1px solid #000; width:150px; height:100px;");
  } else
  {
      iframe.setAttribute("style","border:none; width:0px; height:0px;");
  }
  
  iframe.name = frameID;
  iframe.id = frameID;
  
  if (!iframe.document)
  {
        iframe.document = kn_frameless_findFrameDoc(iframe);
  }
  
  document.body.appendChild(iframe);
  iframe.setAttribute("src",kn_blank);
  
  return iframe;
}

function kn_frameless_findFrameDoc(obj)
{
    if (obj.contentDocument)
    {
        // For NS6
        return obj.contentDocument;   
    } else if (obj.contentWindow)
    {
        // For IE5.5 and IE6
        return obj.contentWindow.document;   
    } else if (obj.document)
    {
        // For IE5
         return obj.document;   
    } else
    {
        return false;
    }
}
function kn_frameless_wrapApp()
{
    name = kn__uniqueWindowName(); // need this for leader election
    kn.TFN_ = name + "_tunnel";
    
    self.kn_frameless_readyCount = 0;
        
    kn.documents[name + "_tunnel"] = kn__object(
                                                  "html",
                                                  "<body></body>",
                                                  "kn_onLoad",
                                                  self.kn_frameless_iframeReady
                                                );
    
    kn.documents[name + "_post"] = kn__object(
                                                  "html",
                                                  "<body></body>",
                                                  "kn_onLoad",
                                                  self.kn_frameless_iframeReady
                                                );
    
    if (document.body.tagName.toLowerCase() == "frameset" || 
        (kn_argv.kn_transport && kn_argv.kn_transport == "frameset"))
    {
        var set = top.document.createElement("frameset");
        set.setAttribute("rows","50%,50%"); // needed for Moz
        
        var tunnel = set.appendChild(document.createElement("frame"));
        tunnel.setAttribute("name",name + "_tunnel");
        
        var post = set.appendChild(document.createElement("frame"));
        post.setAttribute("name",name + "_post");
        
        document.documentElement.appendChild(set);
        
        tunnel.setAttribute("src",kn_blank);
        post.setAttribute("src",kn_blank);
        
        if (!tunnel.location && tunnel.contentWindow && document.all)
        {
            // need to point to the ACTUAL window object of the frames in IE.
            // also need to set the correct name on those window objects so
            // they'll be properly indexed in kn.documents[]
            
            tunnel.contentWindow.name = name + "_tunnel";
            kn.tunnelFrame_ = tunnel.contentWindow;
            
            post.contentWindow.name = name + "_post";
            kn.postFrame_ = post.contentWindow;
        } else
        {
            kn.tunnelFrame_ = tunnel;
            kn.postFrame_ = post;
        }
    } else
    {                                       
        kn.tunnelFrame_ = kn_frameless_createIFrame(name + "_tunnel");                                                                                      
        kn.postFrame_ = kn_frameless_createIFrame(name + "_post");
    }
    
    
    kn.appFrame_ = self;
    kn_frameless_addEvent(kn.appFrame_,"unload",kn__framesetOnUnload);
}

function kn_frameless_OnLoad()
{
    kn.isLoadedP_ = false; // just to be sure

                for (var i=0; i < self.frames.length; i++)
                {
                        if (kn_isReady(self.frames[i]))
                        {
                                if (self.frames[i].name == name + "_post")
                                {
                                        kn.postFrame_ = self.frames[i];
                                } else if (self.frames[i].name == name + "_tunnel")
                                {
                                        kn.tunnelFrame_ = self.frames[i];
                                }
                        }
                }
                
    //kn.tunnelFrame_ = self.frames[name+"_tunnel"];
    //kn.postFrame_ = self.frames[name+"_post"];
                
    if (kn_isReady(kn.tunnelFrame_) && kn_isReady(kn.postFrame_))
    {
        kn__hookFrames(kn.postFrame_,kn.appFrame_);
        kn.isLoadedP_ = true;
    }
    else
    {
        // something has not stabilized yet: the frames have loaded,
        // but the application has not. This was observed on Linux Netscape
        // and often leads to a completely broken application.
        location.reload(true);
    }
}

function kn_frameless_iframeReady()
{
    self.kn_frameless_readyCount++;
    if (self.kn_frameless_readyCount == 2)
    {
      delete kn.documents[kn.tunnelFrame_.name];
      delete kn.documents[kn.postFrame_.name];
      kn_frameless_OnLoad();
    }
}

function kn_frameless_addEvent(obj, evType, fn, useCapture)
{
  if (obj.addEventListener)
  {
    obj.addEventListener(evType, fn, useCapture);
    return true;
  } else if (obj.attachEvent)
  {
    var r = obj.attachEvent("on"+evType, fn);
    return r;
  } else 
  {
    return false;
  }
}

if (!self.kn__wrapApp && document.getElementById && document.createElement)
{
    if (kn_frameless_addEvent(window,"load",kn_frameless_wrapApp))
    {
        kn__wrapApp = function(){};
    }
}

// End of kn_frameless.js
