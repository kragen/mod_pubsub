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

// $Id: document.js,v 1.1 2002/11/07 07:08:09 troutgirl Exp $

// document.js uses kn_blank and other features from do_method=lib.

//  The problems:

//  1. History list pollution caused by Document.open()
//  2. Document.write() caching
//  3. Broken Unicode support in Netscape

//  The solution:

//  d = new KNDocument(window,onLoad) // constructor (renames window and sets onload)
//  d.open() // clear the cached document image
//  d.write(string) // append to the cached document image
//  d.writeln(string) // same, but appends '\n' also
//  d.close() // replace the frame and draw the cached document image

//  Some useful properties are available:

//  d.window // the managed window handle

//  We provide a function to test the state of the KNDocument,
//  analogous to the IE5 document.readyState property:

//  if (d.isReady()) d.window.document.bgColor="gray"

//  In addition, these two synonyms are provided:

//  d.clear() // synonym for d.open()
//  d.flush() // synonym for d.close()

// There are five kinds of onloads that you can specify:

// 1. "top"		      KNDocument(window,"top")
// top will obviously return the scrollbars to their top most position.

// 2. "bottom"		      KNDocument(window,"bottom")
// bottom will make sure the scroll bars are always at the absolute bottom

// 3. "preserve-top-relative" KNDocument(window,"preserve-top-relatve")
// preserve-top will grab the browsers scroll positions before the rewrite
// and re-instate it after the write. there is a preserve-bottom which is
// bottom relative instead of top relative in the works... but it does not
// work yet. use this if you're appending to the top of a document.

// 4. "preserve-bottom-relative" KNDocument(window,"preserve-bottom-relatve")
// use this if you're appending to the end of the document.

// 5. Custom		      KNDocument(window,yourfunctionHere)
// you can now also use this onload capability to pass a function handler 
// to kndocument. if the document itself contains a native event handler 
// it will be bumped down one leel of priority. That is, the kndocument's
// onload will be fired first, and the native even handler will go next

// 6. none		      KNDocument(window)
// if you dont specify an onload there will be none. duh.


function KNDocument(w,onLoad)
{
    if (!w.name || w.name.substring(0,5) != '_knd_')
    {
        w.name = '_knd_' + Math.random().toString().substring(2) +
	  window.name;
    }
    this.window = w;
    this._win = w.name;

    // Set the onload to whatever is passed OR a native function if specified
    this.onLoad = onLoad;
    var this_doc = this;
    if (this.onLoad == "preserve-top-relative"){
	    this.onLoad = function(){
		set_document_positions(w,0,this_doc.ypos + (this_doc.ymax - this_doc.lastYmax));
	    }
    } else if (this.onLoad == "preserve-bottom-relative"){
	    this.onLoad = function(){
		if (this_doc.lastYpos == parseInt(this_doc.lastYmax - this_doc.height) || this_doc.lastYpos == -1){
			set_document_positions(w,0,10000000000);
		} else {
			set_document_positions(w,0,this_doc.ymax - (this_doc.lastYmax-this_doc.ypos) - (this_doc.ymax - this_doc.lastYmax));
		}
		// Works but doesnt keep window at bottom set_document_positions(w,0,this_doc.ymax - (this_doc.lastYmax-this_doc.ypos) - (this_doc.ymax - this_doc.lastYmax));
		// Leaving this in so we can play with it later
	    }
    } else if (this.onLoad == "top"){
	    this.onLoad = function(){
		set_document_positions(w,0,0);
	    }
    } else if (this.onLoad == "bottom"){
	    this.onLoad = function(){
	    	this.onLoad = set_document_positions(w,0,this_doc.ymax);
	    }
    }
    this._base = window.location.href;
    this.open = this.clear = function()
    {
        this.html = '<base href="' + this._base + '" />\n';
    };
    this.write = function(s)
    {
        this.html += '' + s;
    };
    this.writeln = function(s)
    {
        this.html += '' + s + '\n';
    };
    this.close = this.flush = function()
    {
        var i;
        var d;
        if (! kn.documents[w.name])
        {
            kn.documents[w.name] = new Object();
            kn.documents[w.name].state = "ready";
        }
	// Let's set the html in kn.documents and also set an onload
        kn.documents[w.name].html = this.html;
	kn.documents[w.name].kn_onLoad = this.onLoad;

        if (kn.documents[w.name].state != "loading")
        {
            grab_document_positions(w,this);

 	        // This cannot be a native function of the KNDocument object
			// due to some weird JavaScript context stuff.  Believe me.

            if (kn__hacks('fastdoc'))
            {
                // This is an ugly and broken hack, but it's quite fast too;
                // to enable it, give kn_hacks=fastdoc in the URL
                // query string. (e.g. .../Foo.esp/?kn_document=fastdoc)

                w.document.open();
                w.document.write('<html><head>' + this.html);
                w.document.close();
                if (this.onLoad)
                {
                    this.onLoad();
                }
            }
            else
            {
                // This is slower but more correct, and it's what we
                // do by default.
                kn.documents[w.name].state = "loading";
                w.location.replace(kn_blank);
            }
        }
    };
    this.isReady = function()
    {
        return (kn.documents[w.name].state == "ready");
    }
    this.open();
    return this;
}

function grab_document_positions(w,obj){
	if (w.document.body){
		obj.lastYmax = obj.ymax;
		obj.lastXmax = obj.xmax;
		obj.lastYpos = (obj.ypos != "undefined") ? obj.ypos : -1;
		obj.lastXpos = obj.xpos;

                obj.ymax = w.document.body.scrollHeight;
                obj.ypos = w.document.body.scrollTop;
                obj.xmax = w.document.body.scrollWidth;
		obj.xpos = w.document.body.scrollLeft;

		obj.height = w.document.body.clientHeight;
		obj.width = w.document.body.clientWidth;
	}
}

function set_document_positions(w,x,y){
	if (w.document.body){
		w.document.body.scrollLeft = x;
		w.document.body.scrollTop = y;
	}
}

// End of document.js
