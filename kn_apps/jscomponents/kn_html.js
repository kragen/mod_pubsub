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
 * $Id: kn_html.js,v 1.3 2004/04/19 05:39:12 bsittler Exp $
 */

function kd_init(frame){
	// frame is of type Window/Frame object
	/*
	
	Begins processing document
	
	Return value: a collection of the subscribed elements
	
	Usage:	Call with the document onload event.
	"frame" is an optional reference to the window or frame
	with the document you want to rip through. If left blank,
	"frame" defaults to the current window
	
	*/
	
	
	if (kd_watchdog) {
		clearInterval(kd_watchdog);
		kd_watchdog=null;
	}
	
	/*constants for KN attributes
	
	Possible attributes:
	
	kn_topic
	value: a subscription path string.
	action: auto-subscribe this element to this topic and set up the element for management
		of children.
	
	kn_onmessage
	value: a string reference to a custom event handler invoked when the element receives an event.
	action: attach custom handler as actual method, to be invoked when an event is received.
	
	kn_property
	value: any KN event property
	action: insert the value of this KN event property into this element when one is received.
	
	The values of all kn_ attributes are preserved as properties of the element.
	*/
	
	kd_attr_property = "kn_property";
	kd_attr_onmessage = "kn_onmessage";
	kd_attr_topic = "kn_topic";
	
	// get frame and document reference
	
	if (frame) kd_doc = frame.document;
	else kd_doc = document;
	
	// kn_doc is a *global* reference for the document being chewed.
	
	
	// rip through document
	var elms,kd_subs;
	elms = kd_getElementsByTagName("*",kd_doc); // get all elements in this document.

	kd_subs = kd_findSubscribers(elms);
	return kd_subs;
	
	
}

function kd_getElementsByTagName(tagName,obj){
	// tagName is of type String
	// obj is of type Node
	
	/*
	
	<HATE> IE5 Win doesn't support the wildcard "*" in getElementsByTagName,
	so we need this damn workaround. </HATE>
	
	*/

	obj = obj||document; // no obj? use document
	var elms;
	if (obj.all) { //IE
		elms = (tagName=="*") ? obj.all : obj.all.tags(tagName);
	} else { // NS6
		elms = obj.getElementsByTagName(tagName);
	}
	return elms;
}

function kd_hasAttribute(elm,name){
	// elm is of type Node
	/*
	stupid crossbrowser workaround for 
	stupid, stupid, stupid non-standard IE5
	*/
	
	if (!elm.getAttribute) return false;
	var att = elm.getAttribute(name);
	if (att!=null && att!=""){
		return true;
	}
	else return false;
}

function kd_findSubscribers(elms){

	/*
	
	looks for elements with kn_ attributes and
	processes them accordingly.
	
	Return value: a collection of all auto-subscribed elements.
	
	*/
	
	
	var subs = [];
	if (!elms || elms.length==0) return null; // duh
	var l = elms.length;
	for (var i = 0; i < l ; i++){ // start searchin'
		var elm = elms[i];
			if (elm.tagName && elm.tagName.toLowerCase()=="form" && kd_hasAttribute(elm,"action")){ // is this a form element 
				//kd_checkForm(elm);
			}
			if (kd_hasAttribute(elm,kd_attr_topic)){ // should the element be subscribed?
				kd_enable(elm);
				subs[subs.length] = elm;
			}
			if (kd_hasAttribute(elm,kd_attr_onmessage)){ // does it have a custom handler?
				kd_processHandler(elm);
			}
			if (kd_hasAttribute(elm,kd_attr_property)){ // should it display a property?
				kd_processValue(elm);
			}
	}
	return subs; // return the collection
}

function kd_enable(elm){

	/*
	
	1) Auto-suscribes the element to it's topic
	2) Attaches event handlers
	3) Finds all live children and tokens, and creates a collection to reference them
	4) attaches a few private methods to the element for KN use
	
	
	For SUBSCRIBERS only. Children of a subscriber need only listen
	for events from the parent subscriber.
	
	*/

	if (!elm) return; // duh
	
	// preserve topic
	elm.kn_topic = elm.getAttribute(kd_attr_topic);
	
	knEv = {}
	if (kd_hasAttribute(elm,"kn_do_max_age")) knEv.do_max_age = elm.getAttribute("kn_do_max_age");
	if (kd_hasAttribute(elm,"kn_do_max_n")) knEv.do_max_n = elm.getAttribute("kn_do_max_n");
	//if (kd_hasAttribute(elm,"kn_id")) knEv.id = elm.getAttribute("kn_id"); // maybe later
	
	// attach default handler
	elm.onMessage = kd_defaultMethod;
	// attach intentionally blank custom handler, can be overwritten with kn_onevent
	elm.customHandler = function (){}; 
	
	//do it have live children or tokens?
	
	elm.liveChildren = kd_getLiveChildren(elm);
	elm._hasLiveChildren = kd_hasLiveChildren;
	
	// now, auto-subscribe the element to the topic
	// the default onevent becomes the callback method
	
	kn_subscribe(elm.kn_topic,elm,knEv);

}

function kd_checkForm(elm){
	/* looks at a form element, determines if it is supposed to create a kn event onSubmit. */
	 
	//make sure the action is a kn: action
	var action = elm.action;
	if (!action || action.toLowerCase().indexOf("kn:")!=0) {
		return;
	}
	 
	// get the action path
	action = action.substring(3);
	elm.kn_publish_path = action;
	elm.action="javascript:void(0);";
	
	// if there's on onSubmit, preserve it
	
	if (elm.onsubmit) elm._onsubmit = elm.onsubmit;
	
	// attach our own onsubmit handler
	elm.onsubmit = kd_handleSubmit;
}

function kd_handleSubmit(){

	// if there was an original submit handler, execute it
	var subResult = null;
	if (this._onsubmit) subResult = this._onsubmit();

	// create the new KN event from form data
	//var ev = kd_createKNEventFromForm(this);
	
	//publish it
	//kn_publish(this.kn_publish_path,ev);
	
	var r = kn_publishForm(this.kn_publish_path,this);
	
	// the following checks to see if the result of the user's onSubmit handler returned a value
	// if yes, the value is returned; if not, we assume the result is true.
	// This is useful if the user's  handler was meant to stop the form submit process
	
	//if (subResult==null) return true;
	//else return false;
	return false;
}

function kd_normalizeTokenText(text){
	//remove ending tag symbols
	text = text.substring(0,text.length-2);
	//debug.log(text);
	
	// trim whitespace
	while (text.charAt(0)==" "){
		text = text.substring(1);
	}
	while (text.charAt(text.length-1)==" "){
		text = text.substring(0,text.length-1);
	}
	return text;

}


function kd_getLiveChildren(elm){

	/*
	
	Finds and returns the collection of all live children and tokens within a subscriber element
	The collection is attached to the element as the .liveChildren property.
	
	*/

	var liveChildren = [];
	if (!elm.childNodes || elm.childNodes.length==0) return liveChildren; // duh
	for (var i=0; i<elm.childNodes.length; i++){
		var child = elm.childNodes[i];
		if (child.nodeType && child.nodeType == 1) {
			if (!kd_hasAttribute(child,kd_attr_topic)){ // skip if this child this has a subscription of it's own
				if (kd_hasAttribute(child,kd_attr_property)|| kd_hasAttribute(child,kd_attr_onmessage)){
					// this is a live child
					liveChildren[liveChildren.length] = child;
				}
				// search the child's children
				liveChildren = liveChildren.concat(kd_getLiveChildren(child));
			}
		}
		if (child.nodeType && child.nodeType == 3){
			// this is a text node.  Search for the token tag
			//return;
			var flag = true;
			while (flag==true){
				var st = child.nodeValue.indexOf("{{");
				// if a token is found, we split the node into 3 separate text nodes
				// we're only going to manage the middle one: the token itself
				if (st!=-1) {
					var tokenNode = child.splitText(st);
					tokenNode.nodeValue = tokenNode.nodeValue.substring(2); // shave off first $$$
					var endNode = tokenNode.splitText(tokenNode.nodeValue.indexOf("}}")+2);
					var span = kd_doc.createElement("span");
					
					// insert the <span> in place of the token node
					if (tokenNode.replaceNode) tokenNode.replaceNode(span); //IE5
					else (elm.replaceChild(span,tokenNode)); //NS6
					
					var d = kd_normalizeTokenText(tokenNode.data)
					span.setAttribute(kd_attr_property,d);
					
					
					liveChildren[liveChildren.length] = span;
				
					/*
					we may need to loop again, because this process may create new
					text nodes that will be skipped by the original loop
					*/
					
					if (endNode.nodeValue.indexOf("{{")!=-1){
						child = endNode; // recurse and process the end node
					} else {
						flag = false; // else end the loop
					}
				} else {
					flag = false;
				}
			}
		}
	}
	return liveChildren;
}

function kd_hasLiveChildren(){

	/*
	returns true if an element has live children, false if not.
	*/

	if (!this.liveChildren || this.liveChildren.length==0) return false;
	else return true;
}

function kd_processHandler(elm){

	/*
	If an element has a kn_onevent attribute, this method creates the physical
	method that calls the custom handler
	*/
	
	// get the JS code in the attribute, preserve it
	var value = elm.getAttribute(kd_attr_onmessage);
	elm.kn_onmessage = value;
	
	/* since we have no idea if the custom handler will use the event
	object or not, we'll append it as an extra argument */
	
	if (value.indexOf(")")!=-1){
		var a = value.split(")");
		for (var i = 0; i < a.length; i++){
			var lastChar = a[i].charAt(a[i].length-1);
			if (lastChar!="" && lastChar!=";" && lastChar!=" "){
				if (lastChar!="(") a[i]+=",kn_event";
				else a[i]+="kn_event";
			}
		}
		value = a.join(")");
	}
	// create the anonymous wrapper
	var fn = new Function("kn_event",value);
	
	/* now attach the handler. */
	
	if (!elm.onMessage) elm.onMessage = fn
	else elm.customHandler = fn;
	
}

function kd_processValue(elm,value){

	/*
	if the element contains a kn_prop attribute, this method
	attaches the attribute value as a property and the 
	internal method to easily update the content of the element.
	*/	

	value = value||elm.getAttribute(kd_attr_property);
	elm.kn_prop = value;
	elm._updateValue = kd_updateValue;
}

function kd_defaultMethod(event){

	
	/*
	this is the default handler method attached to all subscribers.  It guides the
	flow of the event from the subscriber element to it's live children and token tags
	*/

	// we handle the subscriber element first!
	
	// attach the source element to the event
	
	event.sourceElement = this;
	event.currentElement = this;
	
	//klodge = this;
	
	// first, execute custom handler, even if it's empty
	
	if (this.customHandler) this.customHandler(event);
	
	/*
	we can optionally stop the flow of the event with an added 
	Boolean event property ".stop"
	un-comment the next line if we want this
	*/
	
	//if (event.stop) return;
	
	// if the subscriber itself has a kn_prop atrribute, update it.
	if (this.kn_prop) this._updateValue(event[this.kn_prop]);
	
	//now update live children

	if (this._hasLiveChildren()){
		var l = this.liveChildren.length;
		for (var i = 0; i < l ; i ++){
			var child = this.liveChildren[i];
			
			// call live child event handler
			event.currentElement = child;
			if (child.onMessage) child.onMessage(event);
			
			// if the live child has a kn_prop, update it
			if (event[child.kn_prop]) child._updateValue(event[child.kn_prop]);
		}
	}
	
	return true;
}

function kd_updateValue(value){
	
	/*
	Dirt simple way for an element to update it's own content.
	*/
	
	this.innerHTML = value;
}

function kd_setwatchdog()
{
	// we've been replaced by user code, so run kd_init() ourselves
	if (onload != kd_init) {
		clearInterval(kd_watchdog);
		kd_watchdog=null;
		setTimeout("kd_init(window)",500);
	}
}


onload = kd_init(window);
var kd_watchdog = setInterval("kd_setwatchdog()",10);

// End of kn_html.js
