/*
 * Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
 *
 * @KNOWNOW_LICENSE_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the KnowNow, Inc., nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 * 
 *
 * $Id: kn_activepanels.js,v 1.3 2004/04/19 05:39:12 bsittler Exp $
 */

//KN ActivePanels

function findLayer(id,loc){
	if (!loc){
		if (document.getElementById) return document.getElementById(id);
		else if (document.all) return document.all[id];
		else loc = document;
	}
	if (loc.layers && loc.layers[id]) return loc.layers[id];
	else {
		for (var i = 0; i < loc.layers.length; i++){
			var x = findLayer(id,loc.layers[i].document);
			if (x) return x;
		}
	}
}

function ActivePanel(id,template){
	this.obj = findLayer(id);
	this.id = id;
	if (!this.obj) alert("KN ActivePanel error: cannot find DIV \""+id+"\"");
	if (document.all) this.dom = this.obj;
	this.str = "";
	this.template = kn_resolvePath(location.pathname,(template||"blank.html"));
}

ActivePanel.prototype.write = function(html){
	if (!html) return;
	this.str += html;
}

ActivePanel.prototype.clear = function(html){
	this.str = "";
}

ActivePanel.prototype.getHTML = function(){
	return this.obj.innerHTML||this.str;
}

ActivePanel.prototype.update = function(){
	this.obj.innerHTML = this.str;
	if (document.layers) {
		if (ActivePanel.loading){
			ActivePanel.queue[ActivePanel.queue.length] = this;
		} else {
			ActivePanel._nsUpdate(this);
		}
	}
}

ActivePanel._nsUpdate = function(af){
	this.loading = true;
	this.current = af;
	af.obj.load(af.template,af.obj.parentLayer.clip.right);
}

ActivePanel.queue = []; // for synchronous loading (for NN4)
ActivePanel.loading = false;

ActivePanel._nsUpdateFinish = function(){
	this.current.obj.document.write(this.current.str);
}

ActivePanel._nsAdvanceQueue = function(){
	this.current = null;
	if (this.queue.length > 0){
		var nextAF = this.queue.shift();
		this._nsUpdate(nextAF);
	}
	this.loading = false;

}

ActivePanel.create = function(id,w,h,col){
	col = col||"#ffffff";
  w = w||100;
  h = h||100;
	var style = "<style type='text/css'>#"+id+"-container {position:relative; background-color:"+col+"; layer-background-color:"+col+"; width:"+w+"px; height:"+h+"px; clip:rect(0px "+w+"px "+h+"px 0px); overflow:hidden;}#"+id+" {position:absolute;}</style>";

	var div = '<table border="0" cellspacing="0" cellpadding="0"><tr><td width="'+w+'" height="'+h+'" valign="top" align="left"><div id="'+id+'-container"><div id="'+id+'"></div></div>\n</td></tr></table>';

	document.write(style);
	document.write(div);
}

// End of kn_activepanels.js
