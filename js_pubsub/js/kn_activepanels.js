/*! @file kn_activepanels.js PubSub ActivePanels Component
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_activepanels");
 * &lt;/script&gt;</b>
 * </pre>
 */

// Copyright 2003 KnowNow, Inc.
 
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

// $Id: kn_activepanels.js,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

////////////////////////////////////////////////////////////////////////
// Documentation

// This component is used to display dynamically updated text in
// a cross-browser fashion.

// To use it, use kn_include() to import this library after the kn_browser library.

//    <script type="text/javascript" src="../../js/kn_browser.js"></script>
//    <script type="text/javascript">
//      kn_include("kn_activepanels");
//    </script>

// Then instantiate an ActivePanel with a call to kn_activepanel() constructor
// in the body of the HTML where you want it to appear:

//    <script>kn_activepanel("MyPanel",150,100,true)</script>

kn_include("kn_defaults");

kn_activepanels = [];
kn_activepanel.count = 0;

function kn_activepanel_getObj(id)
{
  var obj = (document.getElementById)? kn_activepanel_findElm(id + "-container"):kn_activepanel_findElm(id);
  
  return obj;
}

// kn_activepanel_init: connects AP objects to DOM elements 

function kn_activepanel_init(ap)
{   
  ap.obj = kn_activepanel_getObj(ap.id);
  
  if (!ap.obj)
  {
    alert("PubSub ActivePanel error: cannot find element \""+ ap.id +"\"");
    return;
  }
  if (ap.overflow == true)
  {
    if (document.layers)
    {
      kn_activepanel_enqueue( 
          function(){kn_activepanel_NSScroll(kn_activepanels[ap.id]);}
      );
    }
    else
    {
        kn_browser_setStyle(ap.obj, "overflow", "auto");
        kn_activepanel_initDone(ap);
    }
  }
  else
  {
    if (!document.layers)
    {
        kn_browser_setStyle(ap.obj, "overflow", "hidden");
    }
    kn_activepanel_initDone(ap);
  }
}

function kn_activepanel_onMouseOver(id)
{
  ap = kn_activepanels[id];
  ap._mouseIsOver = true;
}

function kn_activepanel_onMouseOut(id)
{
  ap = kn_activepanels[id];
  ap._mouseIsOver = false;
}

function kn_activepanel_scrollToBottom(id)
{
  ap = kn_activepanels[id];
  
  if (!document.layers)
  {
    obj = kn_activepanel_getObj(id);
    if (obj.scrollHeight > ap.h)
    {
      obj.scrollTop = obj.scrollHeight - ap.h;
    }
  } else
  {
    self.setTimeout("self.kn_activepanel_NSScrollToBottom(kn_activepanels['" + id + "']);",10);
  }
}

function kn_activepanel_initDone(ap)
{
  ap._ready = true;
  if (ap.str != "")
  {
    ap.update();
  }
  kn_activepanel_workDone();
}

function kn_activepanel_findElm(id,loc)
{
  if (!loc)
  {
      if (document.getElementById) return document.getElementById(id);
      else if (document.all) return document.all[id];
      else loc = document;
  }
  if (loc.layers && loc.layers[id]) return loc.layers[id];
  else
  {
      for (var i = 0; i < loc.layers.length; i++)
      {
          var x = kn_activepanel_findElm(id,loc.layers[i].document);
          if (x)
          {
              return x;
          }
      }
  }
}

// ActivePanel API

/*! @class kn_activepanel_obj
 * This object provides the PubSub ActivePanel interface.<p>This
 * constructor should never be called directly from application
 * code. Use kn_activepanel() instead to create an instance of the
 * ActivePanel component.
 * @ctor An object reference that provides the ActivePanel API.
 * @tparam string id a unique identifier for this ActivePanel
 * @tparam boolean overflow if set to true, scrollbars are applied to the content area
 */
function kn_activepanel_obj(id,overflow)
{
  this._ready = false;
  this.id = id;
  this.str = "";
  this.template = kn_resolvePath(location.pathname,kn_browser_includePath + "html/blank.html");
  this.overflow = overflow||false;
  this._mouseIsOver = false;
  this.cleared = true;
  return this;
}

kn_activepanel_obj.prototype.scrollToBottom = function()
{
  kn_activepanel_scrollToBottom(this.id);
}

/*!
 * Indicates if the mouse pointer is currently over the ActivePanel or its scrollbars.<p>
 * This is useful for writing applications where rapid updates to the ActivePanel might 
 * cause it to become unusable.
 * @treturn public:boolean True if the pointer is over the ActivePanel, false if not.
 */
kn_activepanel_obj.prototype.isMouseOver = function()
{
  return this._mouseIsOver;
}

/*!
 * Appends specified string to internal ActivePanel buffer. Contents are not displayed until the update() method is called.
 * @treturn public:void Nothing.
 * @tparam string html a string to append to the content of this ActivePanel 
 */
kn_activepanel_obj.prototype.write = function (html)
{
  if (!html) return;
  this.str += html;
}

/*!
 * Clears the content of the ActivePanel
 * @treturn public:void Nothing.
 */
kn_activepanel_obj.prototype.clear = function()
{
  this.str = "";
  this.cleared = true;
}

/*!
 * Returns the current content of ActivePanel as a string.
 * @treturn public:string content of internal ActivePanel buffer
 */
kn_activepanel_obj.prototype.getHTML = function()
{
  return this.str;
}


/*!
 * Renders the content of the ActivePanel to the browser.
 * @treturn public:void Nothing.
 */
kn_activepanel_obj.prototype.update = function()
{

  if (!this._ready)
  {
    kn_activepanel_init(this);
    return;
  }
  
  obj = kn_activepanel_getObj(this.id);
  
  obj.innerHTML = this.str;
  
  if (obj.style)
  {
    // These are needed to reset style properties on the object, which is lost
    // during the doc.write().
    kn_browser_setStyle(obj, "overflow", (this.overflow) ? "auto" : "hidden");
    kn_browser_setStyle(obj, "height", this.h + "px");
    kn_browser_setStyle(obj, "width", this.w + "px");
    
    // These are needed to render IE5.5 Win2K correctly.
    obj.noWrap = true;
    obj.noWrap = false;
    setTimeout("kn_activePanel_FinishUpdate('" + this.id + "');", 100);
  }

  if (document.layers)
  {
      kn_activepanel_enqueue(
          new Function("kn_activepanel_nsUpdate(kn_activepanels['" + this.id + "']);")
      );
  }
}

function kn_activePanel_FinishUpdate(id)
{
  os=kn_activepanel_getObj(id);
  os.onmouseover = function () {kn_activepanel_onMouseOver(id)};
  os.onmouseout = function () {kn_activepanel_onMouseOut(id)};
}

function kn_activepanel_enqueue(fn)
{
  if (kn_activepanel.queue.length == 0)
  {
    window.setTimeout("_kn_activepanel_processQ();",1);
  }
  kn_activepanel.queue[kn_activepanel.queue.length] = fn;
}

function _kn_activepanel_processQ()
{
  var fn = kn_activepanel.queue[0];
  with (window) fn();
}

function kn_activepanel_workDone()
{
  /* FIXME: this should be a general kn_browser convenience
   * function, since anArray.shift() is non-portable. */

  var nq = new Array;
  for (var index = 1; index < kn_activepanel.queue.length; index ++)
  {
      nq[nq.length] = kn_activepanel.queue[index];
  }
  kn_activepanel.queue = nq;

  if (kn_activepanel.queue.length != 0)
  {
    window.setTimeout("_kn_activepanel_processQ();",1);
  }
}

// Update routines for Netscape 4.

kn_activepanel.queue = []; // For synchronous loading (for NN4).
kn_activepanel.loading = false;

kn_activepanel_nsUpdate = function(ap)
{
  kn_activepanel.current = ap;
  var w = ap.w;
  w = w - kn_activepanel.sliderW;
  ap.obj.load(ap.template,w);
}

kn_activepanel_nsUpdateFinish = function()
{
  var s = kn_activepanel.current.str;
  kn_activepanel.current.obj.document.write(s);
  if (kn_activepanel.current.cleared)
  {
      kn_activepanel.current.cleared = false;
      kn_activepanel.current.obj.moveTo(0,0);
  }
  //alert(kn_activepanel.current.obj.document.height);
}

kn_activepanel_nsAdvanceQueue = function()
{
  if (kn_activepanel.current.overflow) kn_activepanel_resetKnob(kn_activepanel.current);
  kn_activepanel.current = null;
  kn_activepanel_workDone();
}

function kn_activepanel_resetKnob(ap)
{
  ap.obj.clip.right = ap.obj.document.width;
  ap.obj.clip.bottom = ap.obj.document.height;
        
  if (ap.obj.document.width > ap.obj.parentLayer.clip.right)
  {
    ap._scrollBarH.visibility = "show";
    ap._scrollBarH._knob.docdiff = ap.obj.document.width - ap.obj.parentLayer.clip.right;
    ap.hscrollable = true;
  } else {
    ap._scrollBarH.visibility = "hide";
                ap._scrollBarH._knob.moveTo(0,0);
    ap.hscrollable = false;
  }
        
  if (ap.obj.document.height > ap.obj.parentLayer.clip.bottom)
  {
    ap._scrollBarV.visibility = "show";
    ap._scrollBarV._knob.docdiff = ap.obj.document.height - ap.obj.parentLayer.clip.bottom;
                
    if (ap.hscrollable)
    {
      ap._scrollBarV._knob.docdiff = ap._scrollBarV._knob.docdiff + kn_activepanel.sliderW;;
    }
                
    ap.scrollable = true;

  } else {
    ap._scrollBarV.visibility = "hide";
                ap._scrollBarV._knob.moveTo(0,0);
    ap.scrollable = false;
  }
        
}

// Scrollbar implementation for NS4.

// kn_activepanel_NSElement: constructs a NS4 layer dynamically.

function kn_activepanel_NSElement(x,y,w,h,color,par)
{
  div = new Layer(w,par);
  par.document.layers[par.document.layers] = div;
  div.visibility = "inherit";
  div.clip.right = w;
  div.clip.bottom = h;
  div.document.bgColor = color;
  div.moveTo(x,y);
  return div;
}

// kn_activepanel_NSScroll: assembles a scrollbar widget.

function kn_activepanel_NSScroll(ap)
{
  var imgdir = kn_activepanel.images;
  var w = kn_activepanel.sliderW;// = 16;
  var h = kn_activepanel.sliderH;// = 14;
  var outer = ap.obj.parentLayer; // Get the outer layer.
  ap.obj.clip.right = outer.clip.right - w;
  var left = outer.clip.right - kn_activepanel.sliderW;
  var top = outer.clip.bottom - kn_activepanel.sliderW;
  var scrollBarV = kn_activepanel_NSElement(left,0,w,outer.clip.bottom,"blue",outer);
  scrollBarV.visibility = "hide";
  var button_up = kn_activepanel_NSElement(0,0,w,w,kn_activepanel.sliderKnobColor,scrollBarV);
  var button_dn = kn_activepanel_NSElement(0,scrollBarV.clip.bottom-(w*2),w,w,kn_activepanel.sliderKnobColor,scrollBarV);
  var scrollbar = kn_activepanel_NSElement(0,w,w,scrollBarV.clip.bottom-(w*3),kn_activepanel.sliderColor,scrollBarV);
  var corner = kn_activepanel_NSElement(0,scrollBarV.clip.bottom-w,w,w,kn_activepanel.sliderColor,scrollBarV);
  var knob = kn_activepanel_NSElement(0,0,w,h,kn_activepanel.sliderKnobColor,scrollbar);
  var scrollBarH = kn_activepanel_NSElement(0,top,outer.clip.right,w,"blue",outer);
  scrollBarH.visibility = "hide";
  var button_lf = kn_activepanel_NSElement(0,0,w,w,kn_activepanel.sliderKnobColor,scrollBarH);
  var button_rt = kn_activepanel_NSElement(scrollBarH.clip.right-(w*2),0,w,w,kn_activepanel.sliderKnobColor,scrollBarH);
   var hscrollbar = kn_activepanel_NSElement(w,0,scrollBarH.clip.right-(w*3),w,kn_activepanel.sliderColor,scrollBarH);
        var hcorner = kn_activepanel_NSElement(scrollBarH.clip.right-w,0,w,w,kn_activepanel.sliderColor,scrollBarH);
  var hknob = kn_activepanel_NSElement(0,0,h,w,kn_activepanel.sliderKnobColor,hscrollbar);
  corner.background.src = imgdir + "corner.gif";
  knob.background.src = imgdir + "knob.gif";
  button_up.background.src = imgdir + "bup.gif";
  button_dn.background.src = imgdir + "bdn.gif";
  hcorner.background.src = imgdir + "corner.gif";
  hknob.background.src = imgdir + "hknob.gif";
  button_lf.background.src = imgdir + "blf.gif";
  button_rt.background.src = imgdir + "brt.gif";
  outer.captureEvents(Event.MOUSEOVER | Event.MOUSEOUT);
  scrollBarV.captureEvents(Event.MOUSEMOVE);
  scrollBarH.captureEvents(Event.MOUSEMOVE);
  knob.captureEvents(Event.MOUSEDOWN | Event.MOUSEUP);
  button_up.captureEvents(Event.MOUSEDOWN | Event.MOUSEUP);
  button_dn.captureEvents(Event.MOUSEDOWN | Event.MOUSEUP);
  scrollbar.captureEvents(Event.MOUSEMOVE);
  hknob.captureEvents(Event.MOUSEDOWN | Event.MOUSEUP);
  button_rt.captureEvents(Event.MOUSEDOWN | Event.MOUSEUP);
  button_lf.captureEvents(Event.MOUSEDOWN | Event.MOUSEUP);
  hscrollbar.captureEvents(Event.MOUSEMOVE);
  outer.onmouseover = function () {kn_activepanel_onMouseOver(ap.id)};
  outer.onmouseout  = function () {kn_activepanel_onMouseOut(ap.id)};
  scrollBarV.onmousemove = kn_activepanel_NSDrag;
  scrollBarH.onmousemove = kn_activepanel_NSDragH;
  ap.doScrollDown = kn_activepanel_NSElement_scrollDown;
  ap.doScrollUp   = kn_activepanel_NSElement_scrollUp;
  ap.doScrollRight = kn_activepanel_NSElement_scrollRight;
  ap.doScrollLeft  = kn_activepanel_NSElement_scrollLeft;
  
  button_dn.onmousedown = function()
  {
    document.onmousemove = kn_activepanel_NSdisableSelect;
    this.background.src =  kn_activepanel.images + "bdn_active.gif";
    if (!this.knob.ap.scrollable) return;
    this.knob.ap.isScrolling = true;
    this.knob.ap.doScrollDown();
  }
        
  button_dn.onmouseup = function()
  {
    document.onmousemove = kn_activepanel_NSenableSelect;
    this.background.src =  kn_activepanel.images + "bdn.gif";
    this.knob.ap.isScrolling = false;
  }
  
  button_up.onmousedown  = function()
  {
    document.onmousemove = kn_activepanel_NSdisableSelect;
    this.background.src =  kn_activepanel.images + "bup_active.gif";
    if (!this.knob.ap.scrollable) return;
    this.knob.ap.isScrolling = true;
    this.knob.ap.doScrollUp();
  }

  button_up.onmouseup = function()
  {
    document.onmousemove = kn_activepanel_NSenableSelect;
    this.background.src =  kn_activepanel.images + "bup.gif";
    this.knob.ap.isScrolling = false;
  }
        
  button_rt.onmousedown  = function()
  {
    document.onmousemove = kn_activepanel_NSdisableSelect;
    this.background.src =  kn_activepanel.images + "brt_active.gif";
    if (!this.knob.ap.hscrollable) return;
    this.knob.ap.isScrolling = true;
    this.knob.ap.doScrollRight();
  }
        
  button_rt.onmouseup = function()
  {
    document.onmousemove = kn_activepanel_NSenableSelect;
    this.background.src =  kn_activepanel.images + "brt.gif";
    this.knob.ap.isScrolling = false;
  }
  
  button_lf.onmousedown  = function()
  {
    document.onmousemove = kn_activepanel_NSdisableSelect;
    this.background.src =  kn_activepanel.images + "blf_active.gif";
    if (!this.knob.ap.hscrollable) return;
    this.knob.ap.isScrolling = true;
    this.knob.ap.doScrollLeft();
  }
        
  button_lf.onmouseup = function()
  {
    document.onmousemove = kn_activepanel_NSenableSelect;
    this.background.src =  kn_activepanel.images + "blf.gif";
    this.knob.ap.isScrolling = false;
  }
        
  knob.constrain = true;
  knob.onmousedown = kn_activepanel_NSElement.holdLayer;
  knob.onmouseup = kn_activepanel_NSElement.releaseLayer;
        
  hknob.constrain = true;
  hknob.onmousedown = kn_activepanel_NSElement.holdLayer;
  hknob.onmouseup = kn_activepanel_NSElement.releaseLayer;
  
  knob.limitY = scrollbar.clip.bottom - h;
  hknob.limitX = hscrollbar.clip.right - h;
  
  // Backreferences.
  knob.ap = ap;
  button_up.knob = knob;
  button_dn.knob = knob;
  ap._scrollbar = scrollbar;
        
  hknob.ap = ap;
  button_rt.knob = hknob;
  button_lf.knob = hknob;
  ap._hscrollbar = hscrollbar;
        
  ap._scrollBarV = scrollBarV;
  ap._scrollBarV._knob = knob;
        
  ap._scrollBarH = scrollBarH;
  ap._scrollBarH._knob = hknob;
  
  ap._scrollBarV._knob.docdiff = ap.obj.document.height - ap.obj.parentLayer.clip.bottom;
  ap._scrollBarH._knob.docdiff = ap.obj.document.width - ap.obj.parentLayer.clip.right;
  
  kn_activepanel_initDone(ap);
}

function kn_activepanel_NSScrollToBottom(ap)
{
  if (!ap.overflow) return;
  var y = ap.h - (2*kn_activepanel.sliderW) - kn_activepanel.sliderH;
  ap._scrollBarV._knob.moveTo(0, y);
  ap._scrollBarV._knob.visibility = "show";
  if (ap.obj.document.height > ap.obj.parentLayer.clip.bottom)
  {
    ap._scrollBarV._knob.docdiff = ap.obj.document.height - ap.obj.parentLayer.clip.bottom + kn_activepanel.sliderW;
    // ap.scrollable = true;
    ap.obj.moveTo(0,-ap._scrollBarV._knob.docdiff);
  }
}

function kn_activepanel_NSElement_scrollDown()
{
  var h = kn_activepanel.sliderH
  var k = this._scrollBarV._knob;
  var newy = k.top + 4;
  if (newy+h >= k.limitY) newy = k.limitY;
  k.moveTo(0,newy);
  k.ypercent = newy/k.limitY;
  this.obj.moveTo(this.obj.left,-(k.ypercent*k.docdiff) - h);
  if (this.isScrolling)
  {
    setTimeout("kn_activepanels['" + this.id + "'].doScrollDown();",50);
  }
}

function kn_activepanel_NSElement_scrollUp()
{
  var h = kn_activepanel.sliderH
  var k = this._scrollBarV._knob;
  var newy = k.top - 4;
  if (newy <= 0) newy = 0;
  k.moveTo(0,newy);
  k.ypercent = newy/k.limitY;
  this.obj.moveTo(this.obj.left,-(k.ypercent*k.docdiff));
  if (this.isScrolling)
  {
    setTimeout("kn_activepanels['" + this.id + "'].doScrollUp();",50);
  }
}

function kn_activepanel_NSElement_scrollRight()
{
  var w = kn_activepanel.sliderH
  var k = this._scrollBarH._knob;
  var newx = k.left + 4;
  if (newx+w >= k.limitX) newx = k.limitX;
  k.moveTo(newx,0);
  k.xpercent = newx/k.limitX;
  this.obj.moveTo(Math.floor(-(k.xpercent*k.docdiff)) - w, this.obj.top);
  if (this.isScrolling)
  {
    setTimeout("kn_activepanels['" + this.id + "'].doScrollRight();",50);
  }
}

function kn_activepanel_NSElement_scrollLeft()
{
  var w = kn_activepanel.sliderH
  var k = this._scrollBarH._knob;
  var newx = k.left - 4;
  if (newx <= 0) newx = 0;
  k.moveTo(newx,0);
  k.xpercent = newx/k.limitX;
  this.obj.moveTo(Math.ceil(-(k.xpercent*k.docdiff)),this.obj.top);
  if (this.isScrolling)
  {
    setTimeout("kn_activepanels['" + this.id + "'].doScrollLeft();",50);
  }
}

kn_activepanel_NSElement.my = 0;
kn_activepanel_NSElement.mx = 0;
kn_activepanel_NSElement.co = null;

function kn_activepanel_NSDrag(e)
{
  var nsElm = kn_activepanel_NSElement;
  if (nsElm.co == null) return false;
  nsElm.my = e.pageY  - nsElm.co.doy;
  if (nsElm.my >= nsElm.co.limitY) nsElm.my = nsElm.co.limitY;
  if (nsElm.my <= 0) nsElm.my = 0;
  nsElm.co.moveTo(0,nsElm.my);
  nsElm.co.ypercent = nsElm.my/nsElm.co.limitY;
  nsElm.co.ap.obj.moveTo(nsElm.co.ap.obj.left,-(nsElm.co.ypercent*nsElm.co.docdiff));
  if (document._onmousemove) document._onmousemove(e);
}

function kn_activepanel_NSDragH(e)
{
  var nsElm = kn_activepanel_NSElement;
  if (nsElm.co == null) return false;
  nsElm.mx = e.pageX  - nsElm.co.dox;
  if (nsElm.mx >= nsElm.co.limitX) nsElm.mx = nsElm.co.limitX;
  if (nsElm.mx <= 0) nsElm.mx = 0;
  nsElm.co.moveTo(nsElm.mx,0);
  nsElm.co.xpercent = nsElm.mx/nsElm.co.limitX;
  nsElm.co.ap.obj.moveTo(Math.ceil(-(nsElm.co.xpercent*nsElm.co.docdiff)),nsElm.co.ap.obj.top);
  if (document._onmousemove) document._onmousemove(e);
}

kn_activepanel_NSElement.holdLayer = function(e)
{
  if (e.target != this.document)
  {
    routeEvent(e);
    return;
  }
  document.onmousemove = kn_activepanel_NSdisableSelect;
  kn_activepanel_NSElement.co = this;
  this.doy = e.layerY + ((this.parentLayer!=window) ? this.parentLayer.pageY : 0);
  this.dox = e.layerX + ((this.parentLayer!=window) ? this.parentLayer.pageX : 0);
}

kn_activepanel_NSElement.releaseLayer = function(e)
{
  document.onmousemove = kn_activepanel_NSenableSelect;
  kn_activepanel_NSElement.co = null;
}

if (document.layers)
{
  document.captureEvents(Event.MOUSEMOVE);
  if (document.onmousemove)
  {
    document._onmousemove = document.onmousemove;
  }
}

function kn_activepanel_NSdisableSelect(e)
{
  // Disables highlighting of text when knob is dragged.
  routeEvent(e); // pass event along...
  return false;  // but do nothing else here
}

function kn_activepanel_NSenableSelect(e)
{
  // Re-enables text selecting after knob drag.
  routeEvent(e); // just pass the event
}

function kn_activepanel_setup()
{
  kn_activepanel.sliderW = kn_defaults_get("kn_activepanel","SliderW",16);
  kn_activepanel.sliderH = kn_defaults_get("kn_activepanel","SliderH",14);
  kn_activepanel.images = kn_defaults_get("kn_activepanel","ImagePath",kn_browser_includePath + "images/");
  kn_activepanel.sliderColor = kn_defaults_get("kn_activepanel","SliderColor","#eaeaea");
  kn_activepanel.sliderKnobColor = kn_defaults_get("kn_activepanel","SliderKnobColor","#b0b0b0");
}

/*!
 * Creates the HTML needed to render the ActivePanel component.
 *
 * @tparam string id a unique identifier for this ActivePanel
 * @tparam int w the desired width of the ActivePanel, in pixels
 * @tparam int h the desired height of the ActivePanel, in pixels
 * @tparam boolean scroll if set to true, scrollbars are applied to the content area
 * @treturn kn_activepanel_obj an object reference to the newly-created ActivePanel
 */
function kn_activepanel(id,w,h,scroll)
{ 
  w = w||100;
  h = h||100;

  var html = new Array;

  // Style.
  html[html.length] =
      "<style type='text/css'>" +

      // Style for relatively-positioned container DIV or ILAYER
      "#" + id + "-container" +
      "{" +
      /**/"position: relative;" +
      /**/"width: " + w + "px;" +
      /**/"height: " + h + "px;" +
      /**/"clip: rect(0px " + w + "px " + h + "px 0px); " +
      "}" +

      // Style for absolutely=positioned content DIV or LAYER
      "#" + id +
      "{" +
      /**/"position: relative;" +
      /**/"width: " + w + "px;" +
      /**/"height: " + h + "px;" +
      "}" +

      "<" + "/style>";

  document.write(html.join(""));

  // NOTE: the stylesheet and the elements affected by it must be
  // written separately to keep NS4 happy.

  html = new Array;

  html[html.length] =
      '<table' +
      ' border="0"' +
      ' id="' + id + '-activepanel"' +
      ' class="activepanel"' +
      ' cellspacing="0"' +
      ' cellpadding="0"' +
      '>' +
      '<tr>' +
      '<td width="' + w + '"' +
      ' height="' + h + '"' +
      ' valign="top"' +
      ' align="left"' +
      '>';

  if (document.layers)
  {
      html[html.length] =
          '<ilayer' +
          ' name="' + id + '-container"' +
          ' width="' + w + '"' +
          ' height="' + h + '"' +
          ' clip="0,0,' + w + ',' + h + '"' +
          '>' +
          '<layer' +
          ' name="' + id + '"' +
          ' width="' + w + '"' +
          ' height="' + h + '"' +
          '>' +
          '<' + '/layer>' +
          '<' + '/ilayer>';
  }
  else
  {
      html[html.length] =
          '<div' +
          ' id="' + id + '-container"' +
          '>' +
          '<div' +
          ' id="' + id + '"' +
          '>' +
          '<' + '/div>' +
          '<' + '/div>';
  }

  html[html.length] =
      '<' + '/td>' +
      '<' + '/tr>' +
      '<' + '/table>';

  document.write(html.join(""));

  var ap = new kn_activepanel_obj(id,scroll);
  ap.w = w;
  ap.h = h;
  kn_activepanels[id] = ap;
  return ap;
}

if (typeof(kn_onload_addHandler) == "function")
{
  kn_onload_addHandler(kn_activepanel_setup);
}

// End of kn_activepanels.js
