// Copyright 2000-2002 KnowNow, Inc.  All Rights Reserved.
function _41(args){var _135 = new Object();for(var i = 0; i < args.length; i += 2){_135[args[i]] = args[i + 1];}return _135;}function _40(){return _41(arguments);}function _7(){var _135 = new Array();for(var i = 0; i < arguments.length; i ++){_135[_135.length] = arguments[i];}return _135;}_15=_7();{var i;for(i = 1; i < 256; i ++){var c = _63(i);_15[c] = i;}}function _3(){this._133 =
_7();this._105 =
_7();this._104 = 0;}function _38(obj){var kn = this;var ptr = kn._173._133.length;if(kn._173._104){kn._173._104 = kn._173._104 - 1;ptr=kn._173._105[kn._173._104];}var p = _40('obj', obj, 'ptr', ptr);if(_16() &&
kn._173._133[ptr]){alert(_2("[KNJS:Pool] The pool has been corrupted: #%{0} \"%{1}\" was not completely forgotten.",ptr,obj));}kn._173._133[ptr] = p;return ptr;}function _51(ptr, label){var kn = this;var p = kn._173._133[ptr];if(_16()){if (! p)alert(_2("[KNJS:Pool] %{2}: #%{0} \"%{1}\" was recalled while unused",ptr, p, label));else if (p.ptr != ptr)alert(_2("[KNJS:Pool] %{3}: The pool has been corrupted. #%{0} \"%{1}\" is indexed as #%{2}",ptr, p.obj, p.ptr, label));}return p.obj;}function _21(ptr, label){var kn = this;var obj = kn._177(ptr, label);kn._173._133[ptr] = void 0;kn._173._105[kn._173._104 ++ ] = ptr;return obj;}function _141(_106,_107){kn.ownerWindow.setTimeout("kn._177(" + _106 + ",'_141')._108(" + _106 + ")",_107._116
);}function _140(_106){var _107 =
kn._177(_106, '_140');_107._139(_106, _107);return _107._136();}function _58(_88,_116){var kn = this;if(kn.ownerWindow.setInterval){return kn.ownerWindow.setInterval(_88, _116);}var _107 =
_40('_136',new Function(_88),'_116',_116,'_139',_141,'_108',_140
);var _106 =
kn._168(_107);_107._139(_106, _107);return _106;}function _87(_106){kn._161(_106, "_87");}function _14(_106){var kn = this;if(kn.ownerWindow.clearInterval){return kn.ownerWindow.clearInterval(_106);}kn._177(_106, '_14')._108 =
_87;return void 0;}if (! self.kn_strings) self.kn_strings = _40();_36('en-us','PubSub JavaScript Library %{RCSID}','%{displayname}\'s Localized PubSub JavaScript Library %{RCSID}');KN=_40('ucsNoChar', 0xFFFD,'ucsMaxChar', 0x7FFFFFFF,'ucs2max', 0xFFFF,'utf16max', 0x10FFFF,'utf16firstLowHalf', 0xD800,'utf16firstHighHalf', 0xDC00,'utf16offset', 0x10000,'utf16mask', 0x3FF,'utf16shift', 10,'utf8mask', 0x3F,'utf8shift', 6,'hexDigits', '0123456789ABCDEFabcdef');if(self.kn_tunnelURI != null)kn_tunnelURI=_74(kn_tunnelURI);if(self.kn_tunnelID != null)kn_tunnelID=_74(kn_tunnelID);if(self.kn_tunnelMaxAge != null)kn_tunnelMaxAge=_74(kn_tunnelMaxAge);if(self.kn_lastTag_ != null)kn_lastTag_=_74(kn_lastTag_);if(self.kn_hashCache != null)kn_hashCache=_74(kn_hashCache);if(self.kn_userid != null)kn_userid=_74(kn_userid);if(self.kn_displayname != null)kn_displayname=_74(kn_displayname);if(self.kn_server != null)kn_server=_74(kn_server);if(self.kn_blank != null)kn_blank=_74(kn_blank);if(! self.kn_lang)kn_lang='';if(! self.kn_response_flush)kn_response_flush=null;if(self.kn_defaultHacks != null)kn_defaultHacks=_74(kn_defaultHacks);if(self.kn_appFrameAttributes != null)_6=_74(kn_appFrameAttributes);if(self.kn_defaultHacks == null)kn_defaultHacks=null;if(self.kn_maxHits == null)kn_maxHits=3000;if(self._6 == null)_6='';kn_retryInterval=4000;_68=false;_4=false;_27=100;_45=_40();kn_argv=_46();kn=_28();function _28(){if ((! self.kn__standAlone) &&
parent && kn_isReady(parent) &&
((typeof parent.kn)!= "unknown") && parent.kn){kn=parent.kn;kn_server=parent.kn_server;kn_blank=parent.kn_blank;}else{kn=_40('RCSID', "$Id: pubsub.js,v 1.1 2002/11/07 07:08:08 troutgirl Exp $",'ownerWindow', window,'leaderWindow', window,'tunnelURI', null,'tunnelID', null,'tunnelMaxAge', null,'lastTag_', null,'getHashCache', _24,'setHashCache', _57,'lastError', null,'tunnelRunning_', false,'tunnelFrame_', null,'dispatch_', _40(),'documents', _40(),'subscribe', _67,'unsubscribe', _72,'publish', _48,'publishForm', _49,'setHandler', _56,'clearHandler', _13,'restartTunnel', kn__restartTunnel,'iw', kn_inspectInWindow,'userid', "guest",'displayname', _1("Guest User"));kn._178 =
_54;kn._182 =
_69;kn._159=_17;{var p =
("route add_route delete_route update_route add_journal notify add_topic clear_topic set_topic_property add_notify delete_notify update_notify batch" ) .
split(" ");for(var i = 0; i < p.length; i ++){kn[p[i].toUpperCase()] =
new Function('options', 'handler','kn._159(_70("' +
_18(p[i])+'"), options, handler);');}}kn.sendQueue=_55;kn._173=new _3();kn._168=_38;kn._177=_51;kn._161=_21;kn._181=_58;kn._155=_14;kn._172=kn.ownerWindow;kn._179=0;kn.TFN_=null;kn._163=0;kn._165=false;kn.isLoadedP_=false;kn._164=false;kn._166=false;kn._167=null;kn._184 =
_40();kn._152=null;kn._174=null;kn._185 =
_7();kn._186=0;kn._187=null;kn._175=false;kn._176 =
_47;kn._160 =
_40();kn._157 =
_40();kn._158 =
_40();kn._153=kn_argv;kn._156 =
kn._153.kn_debug;kn._162 =
kn._153.kn_hacks;kn_response_flush=kn._153.kn_response_flush ?
kn._153.kn_response_flush :
kn_response_flush;kn_lang=kn._153.kn_lang ?
kn._153.kn_lang :
kn_lang;kn._170 =
_43;kn._169 =
_42;kn._171 =
_44;self.iw = kn.iw;if(self.kn_server == null)kn_server="/kn";if(kn._153.kn_server) kn_server = kn._153.kn_server;if(self.kn_blank == null)kn_blank=kn_server+ "?do_method=blank";if(kn._153.kn_blank) kn_blank = kn._153.kn_blank;if(! _26(kn_server)){kn_server=location.protocol+ '//' + location.host +
kn_resolvePath(location.pathname, kn_server);}if (! _26(kn_blank)){kn_blank=location.protocol+ '//' + location.host +
kn_resolvePath(location.pathname, kn_blank);}if(self.kn_userid) kn.userid =  self.kn_userid;if(kn._153.kn_userid) kn.userid = kn._153.kn_userid;if(self.kn_displayname) kn.displayname =  self.kn_displayname;if(kn._153.kn_displayname) kn.displayname = kn._153.kn_displayname;var hashCache= "";if(self.kn_hashCache) hashCache = self.kn_hashCache;if(kn._153.kn_hashCache) hashCache = kn._153.kn_hashCache;kn.setHashCache(hashCache);kn.tunnelURI="/who/" +
_18(kn.userid)+"/s/"+Math.random().toString().substring(2, 10) +"/kn_journal";if(self.kn_tunnelURI) kn.tunnelURI =  self.kn_tunnelURI;if(kn._153.kn_tunnelURI) kn.tunnelURI = kn._153.kn_tunnelURI;kn.tunnelURI=_5(kn.tunnelURI);kn.tunnelID=Math.random().toString().substring(2, 10);if(self.kn_tunnelID) kn.tunnelID = self.kn_tunnelID;if(kn._153.kn_tunnelID) kn.tunnelID = kn._153.kn_tunnelID;if(self.kn_tunnelMaxAge) kn.tunnelMaxAge = self.kn_tunnelMaxAge;if(kn._153.kn_tunnelMaxAge) kn.tunnelMaxAge = kn._153.kn_tunnelMaxAge;if(self.kn_lastTag_) kn.lastTag_ = self.kn_lastTag_;if(kn._153.kn_lastTag_) kn.lastTag_ = kn._153.kn_lastTag_;if(self.kn__wrapApp){kn__wrapApp();}else{_75();}}return kn;}onerror=_32;function _24(){var keys = _7();for(var key in this._109){keys[keys.length] = key;}for(var key in this._110){keys[keys.length] = key;}return keys.join(" ");}function _57(str){this._109 =
_40();this._110 =
_40();this._111 =
new Date();var keys = str.split(" ");for(var index = 0; index < keys.length; index += 3){this._109[
keys[index] +" " +
keys[index + 1] +" " +
keys[index + 2]
] =
true;}}function _67(kn_from, kn_to, kn_options, handler){if (! kn_options)kn_options=_40();var ctx = _40();ctx.kn_from = kn_from;ctx.kn_to = kn_to;ctx.kn_options = kn_options;ctx.handler = handler;ctx.routeID =
kn_options.kn_id;if(! ctx.routeID)ctx.routeID = Math.random().toString().substring(2, 10);ctx.kn_from = _5(ctx.kn_from);ctx.route_location =
ctx.kn_options.kn_uri;if(! ctx.route_location)ctx.route_location =
ctx.kn_from+"/kn_routes/" +
kn_escape(ctx.routeID);ctx.e = _40();var ptr = kn._168(ctx);kn._185[kn._185.length] = new Function('var ctx = kn._177(' + ptr + ', "_67(1)");ctx.e.kn_status_from = _60(ctx.handler);if (_50(ctx.kn_from, ctx.kn_to, ctx.kn_options,ctx.routeID, ctx.route_location,ctx.e)){kn._161(' + ptr + ',  "_67(2)");return true;}_13(ctx.e.kn_status_from);return false;');kn._176();return ctx.route_location;}function _50(kn_from, kn_to, kn_options, kn_id, kn_rid, e){if ((typeof(kn_to)== "string") && _37("javascript:",kn_to)){var cmd = kn_to.substring(kn_to.indexOf(":")+1);kn_to=new Function(cmd);}var obj = null;if((typeof(kn_to)!= "string") && (typeof(kn_to) != "function")){obj=kn_to;kn_to =
new Function('m', 'obj','obj.onMessage(m);');}if(typeof(kn_to)== "function"){_56(kn_rid, kn_to, obj);kn_to=kn.tunnelURI;}if(typeof(kn_to)== "string"){if (! e.do_method)e.do_method= "route";e.kn_from = kn_from;e.kn_to = kn_to;e.kn_id = kn_id;e.kn_uri = kn_rid;if(kn_options){for(var p in kn_options){e[p] = kn_options[p];}}_59(e);return kn__submitRequest(e) ;}return true;}function _17(method, options, handler){var e = _40("do_method", method);if(options){for(var key in options){e[key] = options[key];}}var ctx = _40();ctx.e = e;ctx.handler = handler;var ptr = kn._168(ctx);kn._185[kn._185.length] = new Function('var ctx = kn._177(' + ptr + ', "_17(1)");ctx.e.kn_status_from = _60(ctx.handler);if (kn__submitRequest(ctx.e)){kn._161(' + ptr + ', "_17(2)");return true;}_13(ctx.e.kn_status_from);return false;');kn._176();}function _49(kn_to, theForm, handler){var e = _40();for(var i = 0; i < theForm.elements.length; i++){var _95 = theForm.elements[i];var _146 =
_95.type.toLowerCase();if(_146== "checkbox" ||
_146== "radio"){if(_95.checked){e[_95.name] = _95.value;}}else{e[_95.name] = _95.value;}}return kn_publish(kn_to, e, handler);}function _59(e){if(e.userid == null)e.userid = kn.userid;if(e.displayname == null)e.displayname = kn.displayname;if(e.kn_id == null)e.kn_id = Math.random().toString().substring(2, 10);return e ;}function _48(kn_to, event, handler){var e = _40();for(var key in event)e[key] = event[key];if(e.kn_to == null)e.kn_to = kn_to;_59(e);kn.NOTIFY(e, handler);return e.kn_id;}function _72(rid, handler){var _96 = rid.lastIndexOf("/kn_routes/");if(_96 == -1){var e =
_40("status",_1("400 Bad Request"),"kn_payload",_1("Client will not delete a route without the magic '/kn_routes/' substring."));kn_doStatus(e, handler);return;}var eid = kn_unescape(rid.substring(_96 + 11));var _144 =
rid.substring(0, _96);var k = _144.indexOf(kn_server);if(k != -1)_144=_144.substring(k + kn_server.length);var e = _40("kn_from", _144,"kn_to", "","kn_id", eid,"kn_expires", "+5");_59(e);kn.ROUTE(e, handler);}function _55(queue, options, handler){var e = _40("kn_batch", _7());for(var i = 0; i < queue.length; i ++){e.kn_batch[i] = kn_encodeRequest(queue[i]);}if(options) for (var h in options) e[h] = options[h];return kn.BATCH(e, handler);}function _61(theWindow){if (! kn_isReady(theWindow))return;if(theWindow.name != null){_45[theWindow.name] = null;delete _45[theWindow.name];kn.documents[theWindow.name] = null;delete kn.documents[theWindow.name];}theWindow.document.close();if(theWindow.stop) theWindow.stop();if(((theWindow.location.pathname + theWindow.location.search) != kn_blank) &&
(theWindow.location != kn_blank)){theWindow.location.replace(kn_blank);}}function _54(theEvent, theWindow){if(_16("receipts") && theWindow){var html = new Array();html[html.length] = '<hr />';html[html.length] = '<div class="dl"><dl>';for(var i = 0; i < theEvent.elements.length; i ++){var elt = theEvent.elements[i];var name = (elt.name != null) ? elt.name : _74(elt.nameU);html[html.length] = '<dt><var>';html[html.length] = kn_htmlEscape(name);html[html.length] = '<' + '/var>:<' + '/dt>';var value = (elt.value != null) ? elt.value : _74(elt.valueU);html[html.length] = '<dd>';html[html.length] = kn_htmlEscape(value);html[html.length] = '<' + '/dd>';}html[html.length] = '<' + '/dl><' + '/div>';theWindow.document.write(html.join(""));}if(kn && kn._164){kn._164=false;}if(theWindow && ! _45[theWindow.name]){if(_16("routes")){alert(_2("[KNJS:Transport] The window \"%{0}\" has spoken out of turn, and will be removed.",theWindow.name));}_61(theWindow);return;}var e = _40();for(var i = 0; i < theEvent.elements.length; i++){var ei = theEvent.elements[i];var name = (ei.nameU == null) ? ei.name : _74(ei.nameU);var _147 =
(ei.valueU == null) ? ei.value : _74(ei.valueU);e[name] = _147;}if(kn){if(e.kn_route_checkpoint != null){kn.lastTag_=e.kn_route_checkpoint;}else if (e.kn_event_hash && kn.tunnelMaxAge){var key =
_18(e.kn_route_location)+" " +
_18(e.kn_id)+" " +
_18(e.kn_event_hash);if(kn._109[key] || kn._110[key]){if(_16("duplicates")){alert(_2("[KNJS:Transport] A duplicate event was ignored: %{0}",key));}return;}kn._109[key] = true;now=new Date();if((now - kn._111) >
(parseFloat(kn.tunnelMaxAge) * 1000)){kn._110=kn._109;kn._109=_40();kn._111=now;}}}kn__routeEvent(e);if(kn && theWindow && (theWindow.name == kn.TFN_) &&
! kn._166){kn._163 ++;if((! _25("noswap")) &&
(kn._163 > kn_maxHits)){_52();}}}function _10(flag, parameters){if ((flag == null) &&
((typeof flag)== 'undefined'))return parameters ? true : false;if(parameters == null)return false;if((typeof parameters== 'boolean') &&
(parameters == true))return false;if((','+parameters+',').indexOf(','+flag+',') != -1)return true;if((','+parameters+',').indexOf(',all,') != -1)return true;return false;}function _25(flag){return self.kn ? (_10(flag, kn._162) ||
_10(flag, kn_defaultHacks)) : false;}function _16(flag){return(self.kn ?
_10(flag, kn._156) :
false);}function _66(e){var _137 = false;if(_25("single") &&
((! kn._154) ||
(! kn._154.length))){var rid = e.kn_status_from;var obj = _40();obj.handler = kn._157[rid];obj.data = kn._161(kn._158[rid], "_66");kn._158[rid] = kn._168(obj);kn._157[rid] =
new Function('evt', 'obj','kn._170(evt);return obj.handler(evt, obj.data);');if(! _25("noforward"))e.kn_status_to = kn.tunnelURI;return _65(e, kn._174);}if (! kn._154)kn._154=_7();if((kn._154.length == 0) ||
(kn._154[0] == e)){kn._154[0] = e;if(kn_isReady(kn._174) && ! kn._175){var _84 =
_40("do_method", "batch");var _107 =
_40();_107.onSuccess = kn._170;_107.onError = kn._169;_84.kn_status_from =
_60(_107);if(! _25("noforward"))_84.kn_status_to = kn.tunnelURI;_84.kn_batch = kn._154;_137=_65(_84, kn._174);if(_137){kn._154=null;delete kn._154;}else{_13(_84.kn_status_from);}}}else{kn._154[kn._154.length] = e;_137=true;}return _137;}function _30(obj){if (!obj || (typeof obj.length == (null)))return false;if(obj.constructor == Array)return true;if(obj.constructor == String)return false;var _83 = false;for(var i in obj){if(typeof obj[i] == 'function')continue;if(isNaN(parseInt(i))){return false;}_83=true;}return _83;}function _65(e, f){if (!kn_isReady(f) ||
((f == kn._174) && kn._175)){return false;}if(f == kn._174)kn._175=true;var _113;_113='<html>\n'+'<head>\n'+'<title>' + _1("Event Submission Form") + '<' + '/title>\n'+'<' + '/head>\n'+'<body onLoad="';if((navigator.appVersion.charAt(0)== '4') &&
(navigator.appVersion.indexOf("MSIE ") == -1)){_113+='document.forms.eventForm.submit()';}else{_113+='setTimeout(\'document.forms.eventForm.submit()\',1)';}_113+='">\n'+'<form name="eventForm" method="post"'+' action="' +
kn_htmlEscape(kn_server)+'">' +(_16("posts") ?'<div class="dl"><dl>' :'') +'\n';for(var h in e){if(_16("posts")){_113+='<dt><var>' +
kn_htmlEscape(h)+'<' + '/var>:<' + '/dt><dd>';}if(_30(e[h])){for(var i = 0; i < e[h].length; i++){if(typeof(e[h][i])== "string"){_113+='<input type="hidden" name="' + kn_htmlEscape(h) +'" value="' + kn_htmlEscape(e[h][i]) + '" />';if(_16("posts")){if(i)_113+='<' + '/dd>\n<dd>';_113 +=
kn_htmlEscape(e[h][i]);}}else{if(i && _16("posts"))_113+= '<hr />';_113+='<input type="hidden" name="' + kn_htmlEscape(h) +'" value="' + kn_encodeRequest(e[h][i]) + '" />';if(_16("posts")){_113+= '<div class="dl"><dl>\n';for(var j in e[h][i]){_113+='<dt><var>' + kn_htmlEscape(j) +'<' + '/var>:<' + '/dt><dd>' + kn_htmlEscape(e[h][i][j].toString()) +'<' + '/dd>\n';}_113+= '<' + '/dl><' + '/div>\n';}}}}else if ((e[h] != null) && (typeof(e[h].toString)== "function")){_113+='<input type="hidden" name="' + kn_htmlEscape(h) +'" value="' + kn_htmlEscape(e[h].toString()) + '" />';if(_16("posts")){_113 += kn_htmlEscape(e[h].toString());}}if(_16("posts")){_113+= '<' + '/dd>\n';}}if(_16("posts"))_113+= '<' + '/dl><' + '/div>\n';if(_16("posts")){_113+= '<input type="submit" value="' + _1("go") + '" />\n';}_113+= '<' + '/form>\n';_113+= '<' + '/body>\n';_113+= '<' + '/html>\n';_45[f.name] = true;if(! kn.documents[f.name]){kn.documents[f.name] = _40();kn.documents[f.name].state= "ready";}kn.documents[f.name].html = _113;if(kn.documents[f.name].state!= "loading"){kn.documents[f.name].state= "loading";if(f.stop) f.stop();f.location.replace(kn_blank);}f.setTimeout('parent._53()',kn_retryInterval);return true;}function _53(){if(kn._175){kn_retryInterval=kn_retryInterval * 2;if(kn._174.stop) kn._174.stop();kn._174.location.replace(kn_blank);kn._174.setTimeout('parent._53()',kn_retryInterval);if(_16()){kn._172.status =
_2("retrying event publication @%{0}",kn_retryInterval
);}}}function _29(force){_68=true;if(!kn.isLoadedP_){kn._172.setTimeout("_29()",_27);return;}var _93 = true;var _130 = (document.cookie== "");document.cookie = _18(kn.TFN_)+ "=opening;path=/";if((typeof navigator.cookieEnabled== 'boolean' &&
navigator.cookieEnabled == false) ||
(! document.cookie) ||
(document.cookie.indexOf(_18(kn.TFN_)) == -1)){_93=false;if((navigator.appName.indexOf("Microsoft") != -1) &&
(navigator.platform== "Win32") &&
(kn._172 == top) &&
! _25("quiet"))alert(_1("[KNJS:IPC] This web application uses local cookies for communication. Without permission to set local cookies, Internet Explorer on Windows can only support one such web application at a time."));}if (!kn._167){kn._167 =
kn._172.kn._181(_93?"_34()" :"_11()",_27
);}if(_16()) kn.tunnelFrame_.document.bgColor="blue";if(_130 || force)_33();else
setTimeout("_33()",5*_27);}function _52(){kn._166=true;if(kn.tunnelRunning_){kn.tunnelRunning_=false;if(self.kn_onTunnelStop) kn_onTunnelStop();}if(_16()){kn._172.status =
_2("Restarting Tunnel: %{0}#%{1}",kn.tunnelURI,kn.lastTag_);}_29(true);}function _33(){if (!kn.tunnelRunning_ && _68 &&
(kn._172 == kn.leaderWindow)){var s = kn.tunnelURI+'/kn_status/' +
Math.random().toString().substring(2, 10)+'_' +
(kn._179++);kn._158[s] = kn._168(s);kn._157[s] =
new Function('e', 's','kn._161(kn._158[s], "_33");kn._157[s] = null;delete kn._157[s];kn._171(e);');_45[kn.TFN_] = true;var _86= '';if(kn.lastTag_ != null){_86=';do_since_checkpoint=' +
_18(kn.lastTag_);}else if (kn.tunnelMaxAge){_86=';do_max_age=' +
_18(kn.tunnelMaxAge);}var _102 = kn_response_flush;if(_102 == null){_102="4096";}if(kn.tunnelFrame_.stop) kn.tunnelFrame_.stop();kn.tunnelFrame_.location.replace(kn_server+'?kn_from=' + _18(kn.tunnelURI) +';kn_id=' + _18(kn.tunnelID) +';kn_response_flush=' + _18(_102) +';kn_status_from=' + _18(s) +
_86
);kn._183=new Date();}}function _34(){var _91 = document.cookie;var _78=
_40();var _77 =
_40();if(_91 && _91.length){var c=('' + document.cookie).split(" ").join("");_78=c.split(";");if(_78 && _78.length){for(var i = 0; i < _78.length; i++){var _143 =
_78[i].indexOf("=");var app = _78[i].substring(0, _143);var _125 =
_78[i].substring(_143+1);if(_37("kn_", app)){_77[app] = _125;}}}}if(kn._165 &&
(_77["kn_leader"] != _18(kn.TFN_))){if(kn._160[_70(_77["kn_leader"])]){document.cookie= "kn_leader=" + _18(kn.TFN_) + ";path=/";kn._165=true;kn.tunnelRunning_=true;return;}}var _127 =
_77[_18(kn.TFN_)];if(_127== "oops"){_52();return;}if ((_127== "closing") && !_4){if(kn.tunnelRunning_){if(_16()){kn._172.status =
_1("KN: Shutting down concurrent tunnel. Recovery shunt UNIMPLEMENTED");}}kn._165=false;_61(kn.tunnelFrame_);if(_16()) kn.tunnelFrame_.document.bgColor="green";_68=false;document.cookie = _18(kn.TFN_)+ "=closed;path=/";return;}if ((kn.tunnelRunning_ && kn._165) || _4);{for(var _148
in _77){if ((_148 != _18(kn.TFN_)) &&
! kn._184[_148]) {if((_77[_148] == "opening")){if (!_4 || (_148 < _18(kn.TFN_)))document.cookie = _148+ "=closing;path=/";}else if (_77[_148] == "closed"){_12(_148);_148=_70(_148);kn._160[_148]= open(kn_blank,_148);_20(_148);}}}}_11();}function _11(){if(kn._165 && kn.tunnelRunning_ &&
(kn.tunnelFrame_.document.readyState== "complete")){_69(kn.tunnelFrame_);}else{if (!kn.tunnelRunning_ && _68 &&
(kn._172 == kn.leaderWindow) &&
(kn._183 != null)){var now = new Date();if((now - kn._183) > kn_retryInterval){kn_retryInterval=kn_retryInterval * 2;kn__restartTunnel();}}}}function _20(newFollower){var w = kn._160[newFollower];if(kn_isReady(w)){var followerKN = null;if(typeof(w.parent.kn)!= "unknown"){followerKN=w.parent.kn;}if (!followerKN){kn._160[newFollower] = null;w.close();var _92 =
_18(newFollower);kn._184[_92] = true;document.cookie = _92+ "=oops;path=/";if(_16()){alert(_1("[KNJS:IPC] Inter-window communication failure, partitioning."));}}else{followerKN.ownerWindow.kn__follow(kn._172, kn.tunnelURI, kn.lastTag_);}}else{kn._172.setTimeout("_20(_70('" +
_18(newFollower)+"'))", _27);}}function _19(my_kn, hero_kn){var _126 =
_40();_126.kn = my_kn;var _112 =
_40();_112.kn = hero_kn;for(i in _126.kn.dispatch_){if(_126.kn.dispatch_[i] != _112.kn.ownerWindow){if(_16()) _112.kn.tunnelFrame_.document.bgColor="orange";_112.kn.dispatch_[i] = _126.kn.dispatch_[i];if(_112.kn.dispatch_[i].kn)_112.kn.dispatch_[i].kn.leaderWindow = _112.kn.ownerWindow;}}var ql = _126.kn.leaderWindow.kn__queue;if(ql){for(var tfn in ql){while(ql[tfn]){var s = _7('kn__routeEvent(kn__object(');var evt = ql[tfn].event;ql[tfn] = ql[tfn].next;var first = true;for(var hdr in evt){if (! first){s[s.length] = ',';}first=false;s[s.length] = 'kn_unescape("' + _18(hdr) + '"),';s[s.length] = 'kn_unescape("' + _18(evt[hdr]) + '")';}s[s.length] = '))';_112.kn.ownerWindow.setTimeout(s.join(''), 1);}}}if (! _112.kn.tunnelRunning_ && _126.kn.tunnelURI){_126.kn._165 = false;_112.kn.tunnelURI = _126.kn.tunnelURI;_112.kn.lastTag_ = _126.kn.lastTag_;_112.kn.leaderWindow = _112.kn.ownerWindow;if((navigator.appVersion.charAt(0)== '4') &&
(navigator.appVersion.indexOf("MSIE ") == -1)){_112.kn.ownerWindow.kn__restartTunnel();}else{_112.kn.ownerWindow.setTimeout("kn__restartTunnel()",_27);}}}function _42(ev){if(_16())alert(_2("[KNJS:Transport] Request failed: %{0}\n\n%{1}",ev.status,ev.kn_payload));_43(ev);}function _43(ev){if (! kn__hacks("noforward")) _61(kn._174);if(_16()) kn._174.document.bgColor= "yeLLow";kn._175=false;if((navigator.appVersion.charAt(0)== '4') &&
(navigator.appVersion.indexOf("MSIE ") == -1)){kn._172.kn._176();}else{kn._172.setTimeout("kn._176()", 1);}}function _47(){if ((!kn.isLoadedP_) ||
(!kn.leaderWindow.kn) ||
(!kn.leaderWindow.kn.isLoadedP_) ||
(!kn.leaderWindow.kn.tunnelRunning_)){if(kn._172 == kn.leaderWindow)if(!_68)_29();return;}if ((!kn.isLoadedP_) ||
(!kn.tunnelRunning_ && (kn._172 == kn.leaderWindow))){return;}if ((kn._186 < kn._185.length) &&
kn._185[kn._186]){var _150 =
kn._185[kn._186];kn._185[kn._186] = null;if((_150)()){kn._186++;}else{kn._185[kn._185.length] = _150;kn._186++;}}if(kn._186 < kn._185.length){if (!kn._187)kn._187=kn._172.kn._181("kn._176()",_27
);}else if (kn._187){kn._172.kn._155(kn._187);kn._187=null;}}function _69(theWindow){if(theWindow != kn.tunnelFrame_){return;}if(kn.tunnelRunning_){kn.tunnelRunning_=false;if(self.kn_onTunnelStop) kn_onTunnelStop();}if(_16()) kn.tunnelFrame_.document.bgColor="pink";kn._172.setTimeout("kn__restartTunnel()",5*_27);}function _44(ev){if(self.kn_onTunnelStatus) kn_onTunnelStatus(ev);if(ev.status){var _88 = ev.status.charAt(0);if((_88!= '1') &&
(_88!= '2') &&
(_88!= '3') &&
! kn._164){kn._164=true;if(! _25("quiet"))alert(_2("[KNJS:Transport] Can not connect: %{0}\nURL: %{1}#%{2}\n\n%{3}\n%{4}",ev.status,kn.tunnelURI,kn.lastTag_,ev.kn_payload,_1("When the server problem is resolved, reload this page.")));return;}}if(ev.kn_journal_reconnect_scheme== "kn_event_hash"){if(ev.kn_journal_reconnect_timeout){kn.tunnelMaxAge=ev.kn_journal_reconnect_timeout;}else{kn.tunnelMaxAge="60";}}_4=true;_68=false;kn._172.setTimeout("_8()", 3*_27);_34();}function _8(){if(_16()) kn.tunnelFrame_.document.bgColor="red";if(kn._166){kn._166=false;}kn._163=0;kn.tunnelRunning_=true;document.cookie= "kn_leader=" + _18(kn.TFN_) + ";path=/";kn._165=true;_12();_4=false;kn._176();}function _75(){name=_71();kn.TFN_=name+ "_tunnel";var _103='<' + '/head>\n'+'<frameset name="'+name+'_esp" rows="'+(_16() ?
(_16("posts") ? '70%,30%' : '90%,10%') :'100%,*') + '"'+(_16()? '' :' border="0"') +' onLoad="_22();"\n'+' onUnload="_23();">\n'+'   <frame name="'+name+'_app" src="'+kn_blank+'" '+
_6 +(_16()? '' :' frameborder="no"') + '>\n   <frameset cols="50%,50%">\n'+'<frame name="'+name+'_tunnel" src="'+kn_blank+'" '+(_16()? '' :'scrolling="no" noresize frameborder="no"') + '>\n<frame name="'+name+'_post" src="'+kn_blank+'" '+(_16()? '' :'scrolling="no" noresize frameborder="no"') + '>\n   <' + '/frameset>\n'+'<' + '/frameset>\n'+'<noframes>\n'+'   <h1>' + _1("PubSub JavaScript Library Error") + '<' + '/h1>\n'+'   <p>' + _1("PubSub requires Frames, JavaScript, and local Cookies") + '<' + '/p>\n'+'<' + '/noframes>\n'+'<frameset><noframes><xmp><!-' + '-\0';document.write(_103);if(! frames.length){location.reload(true);return;}if (! self.onload){setTimeout('if(!kn.isLoadedP_)_22()', 1000);}}function _23(){if ((this == kn._172) &&
(this != kn.leaderWindow) &&
kn_isReady(kn.leaderWindow) &&
kn.leaderWindow.kn){for(i in kn._157){kn.leaderWindow.kn.dispatch_[i] = null;delete kn.leaderWindow.kn.dispatch_[i];}}var nav4 = false;if((navigator.appVersion.charAt(0)== '4') &&
(navigator.appVersion.indexOf("MSIE ") == -1)){nav4=true;}if(nav4 ||
(('' + document.cookie + ';').split(" ").join("").indexOf("kn_leader=" +
_18(kn.TFN_)+";") != -1)){if (! nav4){_12("kn_leader");_12();}kn.isLoadedP_=false;kn._165=false;for(var i in kn.dispatch_){if(kn_isReady(kn.dispatch_[i]) &&
(kn.dispatch_[i].kn != kn) &&
(kn.dispatch_[i].top != kn._172.top)){_19(kn, kn.dispatch_[i].kn);break;}}}if (! nav4){_61(kn.tunnelFrame_);_61(kn._174);}}function _22(){if(_16()){kn._172.status = kn_formatString(_1("PubSub JavaScript Library %{RCSID}"), kn);}kn.isLoadedP_=false;for(var k = 0; k < frames.length; k++){var t = typeof(frames[k].name);if(t== "unknown"){if(_16()){kn._172.status =
_2("frame %{0} is not accessible! Rebuilding frameset...",k
);}kn._172.location.reload(true);return;}if(t!= "string"){if(_16()){kn._172.status =
_2("frame %{0} is not accessible! Loading blank and recycling...",k
);}if(frames[k].stop) frames[k].stop();frames[k].location.replace(kn_blank);kn._172.setTimeout("_22()",5*_27);return;}if(frames[k].name == (name+"_app")){kn._152=frames[k];}if(frames[k].name == (name+"_tunnel")){kn.tunnelFrame_=frames[k];}if(frames[k].name == (name+"_post")){kn._174=frames[k];}}if(kn._152 && kn._152.location && kn._152.location.pathname) {if(((kn._152.location.pathname + kn._152.location.search)== kn_blank) ||
(kn._152.location == kn_blank)){var _129 = document.URL;if(kn._152.stop) kn._152.stop();kn._152.location.replace(_129);}kn.isLoadedP_=true;}else{location.reload(true);}}function _56(rid, fxn, obj){kn._157[rid] = fxn;kn._158[rid] = kn._168(obj);if(!kn._165){kn.leaderWindow.kn.dispatch_[rid] = kn._172;}}function _13(rid){kn._161(kn._158[rid], "_13");kn._157[rid] = null;delete kn._157[rid];if(!kn._165){kn.leaderWindow.kn.dispatch_[rid] = null;delete kn.leaderWindow.kn.dispatch_[rid];}}function kn_defaultOnSuccess(e){}function kn_defaultOnError(e){if(_16()){alert(kn_formatString(_1("[KNJS:Dispatcher] Got an unhandled error status event: %{status}\n\n%{kn_payload}"),e));}}function kn_defaultOnStatus(e, handler){if (! handler) handler = this;var _88 = e.status ? e.status.charAt(0) : null;if((_88== "1") ||
(_88== "2") ||
(_88== "3")){if(handler.onSuccess){handler.onSuccess(e);}else{kn_defaultOnSuccess(e);}}else{if(handler.onError){handler.onError(e);}else{kn_defaultOnError(e);}}}function kn_doStatus(e, handler){handler=handler ? handler : _40();if(handler.onStatus){handler.onStatus(e);}else{kn_defaultOnStatus(e, handler);}}function _142(e, ctx){_13(ctx.status_from);kn_doStatus(e, ctx.handler);}function _60(handler){var ctx = _40("handler", handler ? handler : _40(),"status_from", (kn.tunnelURI +'/kn_status/' +
Math.random().toString().substring(2, 10)+'_' +
(kn._179++)));_56(ctx.status_from,_142,ctx);return ctx.status_from;}function _37(prefix, target){return target.length >= prefix.length &&
target.substring(0, prefix.length) == prefix;}function _46(){var _81 =
_40();if(document.location.search || self.kn_queryString){var _79 =
self.kn_queryString ?
kn_queryString :
document.location.search.substring(1);var _132 = null;if(_79.indexOf(";") != -1)_132=_79.split(";");else
_132=_79.split("&");for(var i=0; i < _132.length; i++){var pos = _132[i].indexOf("=");if(pos != -1){var _80 =
_132[i].substring(0, pos);var _82 =
_132[i].substring(pos+1);_81[_70(_80)] = _70(_82);}else if (_132[i].length > 0){if(_37("kn_",_132[i]))_81[_132[i]] = true;else
_81.kn_topic = _70(_132[i]);}}}return _81;}function kn_resolvePath(base,path){if(path.charAt(0)== '/'){return path;}var url = base;var end = url.lastIndexOf('/');if(end != -1){url=url.substring(0, end)+ '/';}url += path;var idx;while((idx = url.indexOf('//')) != -1){url =
url.substring(0, idx) +
url.substring(idx + 1);}while ((end = url.indexOf('/../')) != -1 &&
end > (idx = url.indexOf('/'))){url =
url.substring(0, url.substring(0, end).lastIndexOf('/')) +
url.substring(end + 3);}url='/' + url + '/';while((idx = url.indexOf('/../')) != -1){url =
url.substring(0, idx) +
url.substring(idx + 3);}url=url.substring(1, url.length - 1);return url;}function _63(code){if(String.fromCharCode){return String.fromCharCode(code);}if(code > 0 && code < 256){return eval("'\\x" + code.toString(16) + "'");}return'\xFF';}function _9(string, index){if(string.charCodeAt)return string.charCodeAt(index);var _135 = null;_135=_15[string.charAt(index)];if(_135 == null) _135 = KN.ucsNoChar;return _135;}function _26(uri){var c = uri.indexOf(":");if(c < 1) return false;for(var i = 0; i < c; i ++){var cc = uri.charAt(i);if(!(cc>= "0" && cc <= "9"||
cc>= "A" && cc <= "Z"||
cc>= "a" && cc <= "z")){return false;}}return true;}function _5(uri){var _138 = location.protocol+ '//' + location.host + kn_server;if(_26(kn_server)){_138=kn_server;}if(_26(uri)){return uri;}else if (uri.charAt(0)== '/'){return _138 + uri;}else{return _138+ "/" + uri;}}function _71(){var _128 = null;if(document && document.location &&
document.location.pathname &&
(document.location.pathname.length > 0)){_128=document.location.pathname;while(_128.charAt(_128.length - 1)== '/'){_128=_128.substring(0, _128.length - 1);}var idx = _128.lastIndexOf('/');if(idx != -1){_128=_128.substring(idx + 1);}for(var i = 0; i < _128.length; i ++){var cc = _128.charAt(i);if(!(cc>= "0" && cc <= "9"||
cc>= "A" && cc <= "Z"||
cc>= "a" && cc <= "z")){_128 =
_128.substring(0, i)+'_' +
_128.substring(i + 1);}}}if (! _128){_128=Math.random().toString().substring(2,10);}_128="kn_" +
_128+"_" +
Math.random().toString().substring(2, 7);return _128;}function kn_isReady(w){return ((w != null) &&
((typeof w.name)!= "unknown") &&
(w.closed != null) &&
((typeof w.closed)== "boolean") &&
(!w.closed));}function _32(message, url, line){if(this.kn){kn.lastError=_40();kn.lastError.message=message;kn.lastError.url=url;kn.lastError.line=line;kn.lastError.date=new Date();}else{return false;}return !_16();}function kn_encodeRequest(e){var _134 =
_7();for(var key in e){if(e[key] != null){if(_30(e[key])){for(var i = 0; i < e[key].length; i ++){var val = e[key][i];if(typeof(val)!= "string"){val=kn_encodeRequest(val);}_134[_134.length] = _18(key)+'=' + _18(val);}}else if (typeof(e[key].toString)== "function"){_134[_134.length] = _18(key)+'=' + _18(e[key]);}}}return _134.join(';');}function _114(c, tc){if ((c < 32) || (c > 126) || (tc== '\'')) return '&' + '#' + c + ';';else if (tc== '&') return '&' + "amp;";else if (tc== '\"') return '&' + "quot;";else if (tc== '<') return '&' + "lt;";else if (tc== '>') return '&' + "gt;";else return tc;}function kn_htmlEscape(t){return _73(t, _114);}function _31(s){return KN.hexDigits.indexOf(s) != -1;}function _70(value){value='' + value;if(_25("unescape"))return unescape(value);var _94= '';for(var i = 0; i < value.length; i ++){var ch = value.charAt(i);if(ch== '+') ch = ' ';var c0;var c1;if(((i + 2) < value.length) &&
(ch== '%') &&
_31((c0 = value.charAt(i+1))) &&
_31((c1 = value.charAt(i+2)))){var c = parseInt(c0 + c1, 16);ch=_63(c);i += 2;}_94 += ch;}return'' + _74(_94);}function kn_unescape(value){return _70(value);}function _62(){return _64(arguments);}function _64(codes){var _135 = _7();for(var i = 0; i < codes.length; i++){var _88 =
parseInt(codes[i]);if((_88 >= 0) && (_88 <= KN.ucs2max)){_135[_135.length] = _63(_88);}else if ((_88 > KN.ucs2max) && (_88 <= KN.utf16max)){_88 -= KN.utf16offset;_135[_135.length] =
_63(KN.utf16firstLowHalf |
(_88 >> KN.utf16shift)) +
_63(KN.utf16firstHighHalf |
(_88 & KN.utf16mask));}else{_135[_135.length] = _63(KN.ucsNoChar);}}return _135.join("");}function kn_stringFromCharCode(){return _64(arguments);}function _74(string){string='' + string;var s= '';var _88 = 0;var _124 = 0;var _100 = 0;for(var i = 0; i < string.length; i ++){var c = _9(string, i);var ch = string.charAt(i);if((c >= 0) && (c < 0x80)){if(_100)s += _62(KN.ucsNoChar);_100=0;s += _62(c);}else if ((c & 0xC0) == 0x80){if(_100){_88 = (_88 << KN.utf8shift) | (c & KN.utf8mask);_100=_100 - 1;if(! _100){if(_88 < _124)_88=KN.ucsNoChar;s += _62(_88);}}else
s += _62(KN.ucsNoChar);}else if ((c & 0xE0) == 0xC0){if(_100)s += _62(KN.ucsNoChar);_124=0x80;_100=1;_88=c & (KN.utf8mask >> _100);}else if ((c & 0xF0) == 0xE0){if(_100)s += _62(KN.ucsNoChar);_124=0x800;_100=2;_88=c & (KN.utf8mask >> _100);}else if ((c & 0xF8) == 0xF0){if(_100)s += _62(KN.ucsNoChar);_124=0x10000;_100=3;_88=c & (KN.utf8mask >> _100);}else if ((c & 0xFC) == 0xF8){if(_100)s += _62(KN.ucsNoChar);_124=0x200000;_100=4;_88=c & (KN.utf8mask >> _100);}else if ((c & 0xFE) == 0xFC){if(_100)s += _62(KN.ucsNoChar);_124=0x4000000;_100=5;_88=c & (KN.utf8mask >> _100);}else{if(_100)s += _62(KN.ucsNoChar);_100=0;s += _62(KN.ucsNoChar);}}if(_100)s += _62(KN.ucsNoChar);return'' + s;}function kn_utf8decode(string){return _74(string);}function kn_charCodesFromString(string){var ptr = kn._168(_7());var _90 =
new Function('_88','var _89 = kn._177(' + ptr +', "kn_charCodesFromString(1)");_89[_89.length] = _88;');_73(string, _90);return kn._161(ptr, "kn_charCodesFromString(2)");}function _73(string, converter){var s = _7();string='' + string;for(var i = 0; i < string.length; i ++){var _88 =
_9(string, i);var ch = string.charAt(i);if(! ((_88 >= 0) && (_88 <= KN.ucsMaxChar))){_88=KN.ucsNoChar;ch=_63(_88);}if (((_88 & ~KN.utf16mask) == KN.utf16firstLowHalf) &&
((i + 1) < string.length)){var _85 =
_9(string, i + 1);var cch = string.charAt(i + 1);if((_85 & ~KN.utf16mask) == KN.utf16firstHighHalf){i ++;_88=_88 & KN.utf16mask;_85=_85 & KN.utf16mask;_88 = ((_88 << KN.utf16shift) | _85) + KN.utf16offset;ch += cch;}}s[s.length] = converter ? converter(_88, ch) : ch;}return s.join("");}function _98(n){return('%' +
KN.hexDigits.charAt(n >> 4) +
KN.hexDigits.charAt(n & 15));}function _97(_88, ch){if ((_88 < 0x20) ||
(ch== '+') ||
(ch== '/') ||
(_88 == 0x7F)){return _98(_88);}else if (_88 < 0x80){return escape(ch);}else if (_88 < 0x800){return(_98(0xC0 | ((_88 >> 6) & 0x1f)) +
_98(0x80 | (_88 & 0x3f)));}else if (_88 < 0x10000){return(_98(0xE0 | ((_88 >> 12) & 0xf)) +
_98(0x80 | ((_88 >> 6) & 0x3f)) +
_98(0x80 | (_88 & 0x3f)));}else if (_88 < 0x200000){return(_98(0xF0 | ((_88 >> 18) & 0x7)) +
_98(0x80 | ((_88 >> 12) & 0x3f)) +
_98(0x80 | ((_88 >> 6) & 0x3f)) +
_98(0x80 | (_88 & 0x3f)));}else if (_88 < 0x4000000){return(_98(0xF8 | ((_88 >> 24) & 0x3)) +
_98(0x80 | ((_88 >> 18) & 0x3f)) +
_98(0x80 | ((_88 >> 12) & 0x3f)) +
_98(0x80 | ((_88 >> 6) & 0x3f)) +
_98(0x80 | (_88 & 0x3f)));}else{return(_98(0xFC | ((_88 >> 30) & 0x1)) +
_98(0x80 | ((_88 >> 24) & 0x3f)) +
_98(0x80 | ((_88 >> 18) & 0x3f)) +
_98(0x80 | ((_88 >> 12) & 0x3f)) +
_98(0x80 | ((_88 >> 6) & 0x3f)) +
_98(0x80 | (_88 & 0x3f)));}}function _18(value){value='' + value;if(_25("escape"))return escape(value);var _135 =
_73(value, _97);return _135;}function kn_escape(value){return _18(value);}function _12(s){if (!s) s = _18(kn.TFN_);var _99 = new Date((new Date()).getTime(0));document.cookie = s+ "=THIS_IS_AN_EX_COOKIE;expires=" +
_99.toGMTString()+ ";path=/";}function kn_formatString(format, args){var _135= '';var _131 = true;while(1){var pos = format.indexOf(_131? '%' : '}');if(pos == -1) return _135 + (_131 ? format : args[format]);var _123 = format.substring(0, pos);format=format.substring(pos + 1);_135 += (_131 ? _123 : args[_123]);if(_131){var _101 = format.substring(0,1);format=format.substring(1);if(_101== '{')_131=false;else
_135 += _101;}else{_131=true;}}return _135;}function kn_localizeDefault(l, c, s, ls){if (! kn_strings[l])kn_strings[l] = _40();if(! kn_strings[l][c])kn_strings[l][c] = _40();if(! kn_strings[l][c][s])kn_strings[l][c][s] = (ls != null) ? ls : s;}function _36(l, s, ls){kn_localizeDefault(l, "kn", s, ls);}function kn_localize(l, c, s, ls){if (! kn_strings[l])kn_strings[l] = _40();if(! kn_strings[l][c])kn_strings[l][c] = _40();kn_strings[l][c][s] = (ls != null) ? ls : s;}function _35(l, s, ls){kn_localize(l, "kn", s, ls);}function $$(c, s){if (! kn_lang)return s;var ls = s;var q = 0;var _119 = kn_lang.split(',');for(var _115 =
0; _115 < 2; _115 ++)for(var i = 0; i < _119.length; i ++){var _117 =
_119[i].toLowerCase();var lq = 1;while(_117.substring(0, 1)== ' ') _117 = _117.substring(1);var _118 =
_117.split(';q=');_117=_118[0];if(_118.length > 1) lq = new Number(_118[1]);if(lq > q){if(_117== '*'){ls=s;q=lq;continue;}for(var _121 in kn_strings){var _122 =
_121.toLowerCase();if((_117 == _122) ||
_115 &&
(_122.substring(0, _117.length + 1) == (_117+ "-"))){if(kn_strings[_121][c] &&
kn_strings[_121][c][s]){ls=kn_strings[_121][c][s];q=lq;}}}}}return ls;}function _1(s){return $$("kn", s);}function $$_(c, s){var _81 = new Array(arguments.length - 2);for(var i = 0; i < _81.length; i ++){_81[i] = arguments[i + 2];}return kn_formatString($$(c, s), _81)}function _2(s){var _81 = new Array(arguments.length - 1);for(var i = 0; i < _81.length; i ++){_81[i] = arguments[i + 1];}return kn_formatString($$("kn", s), _81)}function kn_createContext(c){this[c+ "$"] =
new Function('s','return $$(_70("' + _18(c) + '"), s);');this[c+ "$_"] =
new Function('s','var _81 =new Array(arguments.length - 1);for (var i = 0; i < _81.length; i ++){_81[i] = arguments[i + 1];}return kn_formatString($$(_70("' + _18(c) + '"), s), _81);');}kn_createContext("");function kn_debug(flags){if ((flags == null) && (typeof(flags)== 'undefined'))kn._156=true;else
kn._156=flags;if(kn._172.document.all){kn._172.document.all[kn._172.name+ "_esp"].rows =
(_16() ? (_16("posts") ? '70%,30%' : '90%,10%') : '100%,*');}}function kn_hacks(flags){if ((flags == null) && (typeof(flags)== 'undefined'))kn._162=true;else
kn._162=flags;}function kn_inspectAsText (target, depth, prefix){if(prefix == null) prefix= "";if(depth == null) depth = 1;if(target == null) return"null";var y = target+ "{";for(var x in target){y+="\n " + prefix + x + " = " + "(" + typeof(target[x]) + ")" +
(((depth > 1) && ((typeof target[x])== "object"))? kn_inspectAsText (target[x], depth - 1, prefix+ " "): ("" + target[x]).split("\n").join("\n " + prefix));}y+= "\n" + prefix + "}";return y;}function kn_inspectAsHTML (target, depth){if(depth == null) depth = 1;if(target == null) return"null";var y= "\n<dl>";for(var x in target){y+="\n<dt><b>" +
kn_htmlEscape(x)+"<" + "/b>: <i>" +
kn_htmlEscape(typeof(target[x]))+"<" + "/i>\n<dd>" +
(((depth > 1) && ((typeof target[x])== "object"))? kn_inspectAsHTML (target[x], depth - 1): ("<pre>" + kn_htmlEscape(target[x]) + "<" + "/pre>"));}y+= "\n<" + "/dl>";return y;}function kn_inspectInWindow(target, depth){var f = self.open(kn_blank,"_blank","height=200,width=400,scrollbars=yes,resizable=yes");f.document.open("text/html");f.document.write(kn_inspectAsHTML(target, depth));f.document.close();}function kn__submitRequest(e){return _66(e);}function kn__restartTunnel(){return _52();}function kn__object(){return _41(arguments);}function kn__consumeEvents(){while(kn.leaderWindow.kn__queue &&
kn.leaderWindow.kn__queue[kn.TFN_]){var q = kn.leaderWindow.kn__queue[kn.TFN_];kn.leaderWindow.kn__queue[kn.TFN_] = q.next;var evt = _40();for(var header in q.event){evt[header] = q.event[header];}kn__routeEvent(evt);}}function kn__hacks(flag){return _25(flag);}function kn__debug(flag){return _16(flag);}function kn__routeEvent(e){var rid = e.kn_route_location;if(_16("events")){alert(_1("[KNJS:Transport] Got an event:\n") +'kn_id: ' + e.kn_id + '\nkn_route_location: ' + e.kn_route_location + '\nstatus: ' + e.status + '\n\n' + e.kn_payload);}if(rid){var rw = kn.dispatch_[rid];if((('' + kn._157[rid]) != '') &&
(kn._157[rid] != null)){(kn._157[rid])(e, kn._177(kn._158[rid], "kn__routeEvent"));}else if ((rw != null) && (rw != kn._152) && kn_isReady(rw)){if (! kn.leaderWindow.kn__queue){kn.leaderWindow.kn__queue = _40();}var o = _40('event', e, 'next', null);var tail = kn.leaderWindow.kn__queue[rw.kn.TFN_];if(! tail){kn.leaderWindow.kn__queue[rw.kn.TFN_] = o;if((navigator.appVersion.charAt(0)== '4') &&
(navigator.appVersion.indexOf("MSIE ") == -1)){rw.kn__consumeEvents();}else{rw.setTimeout('kn__consumeEvents()',1);}}else{while(tail.next) tail = tail.next;tail.next = o;}}else{if(kn.ownerWindow.kn_defaultOnMessage){kn.ownerWindow.kn_defaultOnMessage(e);}else{if(_16("routes")){alert(_2("[KNJS:Dispatcher] Removing stale subscription:\nkn_route_location: %{0}",e.kn_route_location));}{var _96 = e.kn_route_location.lastIndexOf("/kn_routes/");if(_96 != -1){kn_unsubscribe(e.kn_route_location);}}}}}else{if(_25("single") && _25("noforward")){if(_16())alert(_1("[KNJS:Dispatcher] Got an unlabeled event, assuming post frame success."));kn._170(e);}else{if(kn.ownerWindow.kn_defaultOnMessage){kn.ownerWindow.kn_defaultOnMessage(e);}else{if(_16())alert(_1("[KNJS:Dispatcher] Got an unlabeled event, discarding it."));}}}}function kn__follow(_149,_145,_120){if(kn._167){kn._172.kn._155(kn._167);kn._167=null;}if(kn._165){if(_16()) kn.tunnelFrame_.document.bgColor="seagreen";_19(kn, _149.kn);}if(_16() && !_145){alert(_2("[KNJS:IPC] New leader %{0} had no tunnel; using my %{1}#%{2}",_149.name,kn.tunnelURI,kn.lastTag_));}if(_145){kn.tunnelURI=_145;kn.lastTag_=_120;}kn.leaderWindow=_149;kn._176();}function kn_subscribe(kn_from, kn_to, kn_options, handler){return kn.subscribe(kn_from, kn_to, kn_options, handler);}function kn_publishForm(kn_to, theForm, handler){return kn.publishForm(kn_to, theForm, handler);}function kn_publish(kn_to, event, handler){return kn.publish(kn_to, event, handler);}function kn_unsubscribe(rid, handler){return kn.unsubscribe(rid, handler);}function kn_sendCallback(theEvent, theWindow){return kn._178(theEvent, theWindow);}function kn_redrawCallback(w){if(w['document'] && w.document['write']){if(kn.documents[w.name]){kn.documents[w.name].state= "drawing";var str = kn.documents[w.name].html;if(kn.documents[w.name].kn_onLoad){str+="<" + "script type=\"text/javascript\">\n<" + "!-" + "-\n _39 = self.onload;onload=new Function('parent.kn.documents[self.name].kn_onLoad();if (_39) _39();');// -" + "->\n<" + "/script>";}w.document.write(str);kn.documents[w.name].state= "ready";}else{w.document.write('<body bgcolor="white"><' + '/body>');}}}function kn_tunnelLoadCallback(theWindow){return kn._182(theWindow);}
