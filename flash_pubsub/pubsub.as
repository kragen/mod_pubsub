/*--------------------------------------------------------------------

PubSubFlash Library
Date: June 3 2003
Author: Jos Yule
$Id: pubsub.as,v 1.1 2003/07/20 07:41:30 ifindkarma Exp $
        
        
This work is licensed under the Creative Commons Attribution License. 

To view a copy of this license, visit 
http://creativecommons.org/licenses/by/1.0/ 
or send a letter to 
Creative Commons, 
559 Nathan Abbott Way, 
Stanford, California 94305, USA. 

---------------------------------------------------------------------*/

function pubSubSocket ( host, port){
        this.$host = host;
        this.$port = port;
        this.$sendQueue = new Array();
        this.$dataQueue = new Array();
        this.$try = 0;
}
pubSubSocket.prototype = new XMLSocket();

pubSubSocket.prototype.__resolve = function (name){
        trace("Error: Name '" + name + "' is not a function or prop of " + this);
}

pubSubSocket.prototype.$dataPath = "/kn";
// This is a hack, sort of. This obj can only be
// used by the Python PubSub server right now.

pubSubSocket.prototype.$connected = false;
// So if you do a send, and are not currently connected,
// the socket will attempt to reconnect and send your message.

pubSubSocket.prototype.$maxTries = 5;
// Number of milliseconds to keep trying to connect
// to the host. If this fails, all further sends fail.
// This may not be the behavior we want.

pubSubSocket.prototype.send = function ( content ) {
        if(typeof(content) == "object") {
                content = content.toString();
        }
        var req = this._makeMessage(content);
        this.$sendQueue.push(req);
        trace("\nsend: " + req);//this.$sendQueue);
        if(this.$connected == true) {
                this._send();
        } else {
                this.connect();
        }
}

pubSubSocket.prototype._send = function () {
        if(this.$connected != true) {
                this.connect();
        } else {
                var msg = new XML(this.$sendQueue.shift());
                trace("\n_send(): " + msg);
                super.send(msg);
                /*
                if(this.$sendQueue.length > 0){
                        this._send();
                }
                */
        }
}

pubSubSocket.prototype._makeMessage = function ( content ) {
        //var content = "kn_response_format=" + this.$responseFormat + ";" + content;
        var req = "POST " + this.$dataPath + " HTTP/1.0\r\n" +
        "Host: " + this.$host + "\r\n" +
        "Content-Length: " + content.length + "\r\n" +
        "\r\n" + content;
        return req;
}

pubSubSocket.prototype.onData = function ( src ) {
        //trace("\nonData:\n" + src);
        this.$dataQueue.push(src);
        trace("[" + src.split("\r\n") + "]");
                
}

pubSubSocket.prototype.connect = function (){
        trace("\nconnect()");
        clearInterval(this.$intervalId);
        if(this.$connected == false) {
                this.$connected = "working";
                super.connect(this.$host,this.$port);
        }
}

pubSubSocket.prototype.onConnect = function ( success ) {
        trace("\nonConnect: " + success);
        if(success == true) {
                this.$try = 0;
                this.$connected = true;
                if(this.$sendQueue.length > 0){
                        this._send();
                }
        } else {
                this.$connected = false;
                if(this.$try++ < this.$maxTries) {
                        this.$intervalID = setInterval(this,"connect",100);
                } else {
                        trace("Too many tries to connect. Stopping.");
                }
        }
}

pubsubSocket.prototype.close = function () {
        trace("\nclose()");
        this.$connected = false;
        super.close();
}

pubSubSocket.prototype.onClose = function () {
        trace("\nOnClose()");
        this.$connected = false;
        if(this.$sendQueue.length > 0) {
                this._send();
        }
}

//---------------------------
// pubSubData Object
//

function pubSubData ( response, method, payload, to, from){
        if(response) {
                this.kn_response_format = response;
        }
        if(method) {
                this.do_method = method;
        }
        if(payload) {
                this.kn_payload = payload;
        }
        if(to) {
                this.kn_to = to;
        }
        if(from) {
                this.kn_from = from;
        }
}

pubSubData.prototype.__resolve = function ( name ) {
        trace("Error with '" + name + "'. No function or property with that name.");
}

pubSubData.prototype.setToPath = function ( aPath ) {
        this.kn_to = aPath;
}

pubSubData.prototype.setFromPath = function ( aPath ) {
        this.kn_from = aPath;
}

pubSubData.prototype.setMethod = function ( aMethod ) {
        this.do_method = aMethod;
}

pubSubData.prototype.setResponse = function ( format ) {
        this.kn_response_format = format;
}

pubSubData.prototype.setPayload = function ( data ) {
        this.kn_payload = data;
}

pubSubdata.prototype.setProp = function ( name, value) {
        this[name] = value;
}

pubSubData.prototype.toString = function () {
        var output = "";
        var payload = "";
        for(i in this) {
                if(typeof(this[i]) == "string" && i == "kn_payload" ) {
                        payload += i + "=" + escape(this[i]) + "";
                } else if(typeof(this[i]) == "number" || typeof(this[i]) == "string") {
                        output += i + "=" + this[i] + ";";
                }
        }
        return output + payload;
}

//----------------------------
// TESTING AREA
//

pbs = new pubSubSocket ("localhost", 8008);
x = new pubSubData();
x.setFromPath("/who/jos/s/24/kn_journal");b
x.setToPath("/what/chat");
x.setMethod("route");
x.setProp("displayname", "Jos");
x.setPayload("Hey there smoothie, whats up? I \"LOVE\" you!");
//pbs.send(x);
trace(x)
stop()

// End of pubsub.as
