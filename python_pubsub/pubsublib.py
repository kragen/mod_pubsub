#!/usr/bin/python

"""
    pubsublib.py -- PubSub Python Client Library
    for use with pubsub.py and the mod_pubsub distribution.
    Tested on Python 1.5 and above, on Debian and Red Hat Linux.

    This is a simple Python client library for use in event-driven
    programs.  It can publish and subscribe in (more or less) the
    same way the PubSub JavaScript Library does.

    Phil Harris' libkn Python client library is available in
    mod_pubsub/python_pubsub/libkn/libkn.py .

    Two ways in which pubsublib.py differs from libkn:
       1. The interface is URL based.
       2. All pubsublib.py operations are in a single thread and
          nonblocking, using asyncore.
          (libkn.py is multi-threaded and blocking.)

    $Id: pubsublib.py,v 1.13 2003/07/19 12:24:53 rloz Exp $

    Known Issues:
       1. Need to complete test suite.
       2. FIXME: asyncore is a global.
          Need to make it parameterized to allow multi-threaded use.
       3. Lots of other FIXME and TODO things left --
          see source code comments.

    Contact Information:
       http://mod-pubsub.sf.net/
       mod-pubsub-developer@lists.sourceforge.net
"""


# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

# @KNOWNOW_LICENSE_START@

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.

# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.

# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
# OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.

# @KNOWNOW_LICENSE_END@

# $Id: pubsublib.py,v 1.13 2003/07/19 12:24:53 rloz Exp $



import sys, time, random, types, quopri, string, cStringIO

import urllib, urlparse, cgi

import asyncore, asynchttp, scheduler
"""
    Note that we are using the event-driven python_pubsub asyncore,
    not the polling "standard" asyncore.

    TODO: Investigate using Twisted Internet, available from
    http://www.twistedmatrix.com/ , for the event core libraries.
"""



class Client:

    """
        Abstract client (specialize & include a transport to make it work).
    """

    # Magic substring which must be present for us to be able
    # to divine the source topic.
    _KN_ROUTES_ = "/kn_routes/"

    def __init__(self):
        self._C_dispatchTable = { }
        self._C_requestQueue = [ ]
        self._C_connected = 0
        self._C_connecting = 0
        self._C_random = random.Random()

        """
            The URL to get a persistent response needs to end in
            /kn_journal . The rest of its topic is free-form;
            our conventional default is
                %serverURL%/who/%username%/s/%random%/kn_journal

            FIXME: This should be configurable.
        """

        self._C_tunnelurl = "%s/who/anonymous/s/%s/kn_journal" % (
            self.getServerURL(),
            str(self._C_random.random()).replace(".","_"))

    def getTunnelURL(self):
        return self._C_tunnelurl

    def connect(self):
        """
            Connection state is used to statefully reconnect when the
            connection is lost.  Connection state includes a journal to
            connect to, and may include an (optional) checkpoint.

            When there is a connection to the server, its file descriptor
            needs to be registered with asyncore.  Applications using the
            client need to register with asyncore too, and cannot block
            for a long time.
        """
        if not self._C_connected:
            if not self._C_connecting:
                self._C_connecting = 1
                message = {
                    "do_method": "route",
                    "kn_from": self._C_tunnelurl,
                    "kn_to": "javascript:",
                    "kn_status_from": self._C_randURI(),
                    "kn_response_format": "simple"
                    }
                kn_status_from = message["kn_status_from"]
                # Calls self's onStatus with the server's response to the
                # route request.  This in turn sets self._C_connected in
                # its onStartTunnel method.
                wrapper = self._StatusHandlerWrapper(self, self, kn_status_from)
                self.addHandler(kn_status_from, wrapper)
                self.startTunnel(message)

    def disconnect(self):
        self._C_connecting = 0
        if self._C_connected:
            self.stopTunnel()

    def onMessage(self, message):
        self.dispatch(message)

    def onStatus(self, message):
        status = 500
        try:
            status = int(float(message["status"].split(" ")[0]))
        except ValueError, e:
            pass
        if status == 200:
            self.onStartTunnel()
        else: # FIXME: Needs error handling.
            self.onStopTunnel()

    def onStartTunnel(self):
        self._C_connected = 1
        self.onConnect()
        self._C_process()

    def onStopTunnel(self):
        self._C_connected = 0
        self.onDisconnect()
        if self._C_connecting:
            self.connect()

    def _C_randURI(self):
        return "javascript://tempuri.org/%f" % self._C_random.random()

    class _StatusHandlerWrapper:

        def __init__(self, client, inner, kn_status_from):
            self._SHW_client = client
            # Call its own onStatus() if inner is None.
            self._SHW_inner = inner or self
            self._SHW_kn_status_from = kn_status_from
            self._SHW_fired = 0
            scheduler.schedule_processing(self, time.time() + 300, "auto-timeout")

        def onStatus(self, message):
            pass

        def onMessage(self, message):
            # FIXME: We should cancel the timer if it's still running.
            self._SHW_fired = 1
            self._SHW_client.removeHandler(self._SHW_kn_status_from)
            self._SHW_inner.onStatus(message)

        def __call__(self):
            if not self._SHW_fired:
                self._SHW_client.cancel(self._SHW_kn_status_from)

    def enqueue(self, message, statushandler = None):
        """
            The request queue is an internal list data structure.

            Operations:
                 1. enqueue - Add request to the queue, start
                    connection (if needed).  For every request
                    submitted, forward a status event back over
                    connection.  enqueue returns kn_status_from,
                    which can subsequently be passed to cancel.
                 2. cancel - Pending queued item.

            Unacknowledged requests are automatically cancelled
            after 300 seconds.
        """

        # FIXME: This needs to handle retries.

        messageCopy = {
            "kn_status_from": self._C_randURI(),
            "kn_response_format": "simple",
            "kn_status_to": self._C_tunnelurl
            }

        for name in message.keys():
            messageCopy[name] = message[name]

        kn_status_from = messageCopy["kn_status_from"]

        wrapper = self._StatusHandlerWrapper(self, statushandler, kn_status_from)

        self.addHandler(kn_status_from, wrapper)

        self._C_requestQueue.append(messageCopy)
        self._C_process()

        return messageCopy["kn_status_from"]

    def _C_process(self):
        if not self._C_connected:
            if not self._C_connecting:
                self.connect()
            return
        while self._C_requestQueue != [ ]:
            message = self._C_requestQueue[0]
            self._C_requestQueue = self._C_requestQueue[ 1 : ]
            self.sendMessage(message)

    def cancel(self, kn_status_from):
        self.dispatch(
            {
            "kn_route_location": kn_status_from,
            "status": "500 Internal Server Error",
            "kn_payload": "Client Cancelled Request."
            }
            )

    def addHandler(self, kn_route_location, handler):
        """handler should have onMessage() defined"""
        self._C_dispatchTable[kn_route_location] = handler

    def removeHandler(self, kn_route_location):
        if self._C_dispatchTable.has_key(kn_route_location):
            del self._C_dispatchTable[kn_route_location]

    def dispatch(self, message):
        if message.has_key("kn_route_location"):
            kn_route_location = message["kn_route_location"]
            if self._C_dispatchTable.has_key(kn_route_location):
                self._C_dispatchTable[kn_route_location].onMessage(message)

    def publish(self, topic, message, statushandler = None):
        """
            publish - Enqueue "notify" request to submit to the server.

            kn_id = publish(topic, message, [status-handler])

            Inputs: a destination topic, a message, a status handler.
            Output: a message ID.

            What it does:
                1. If no message ID is provided, generate a random ID.
                2. Build a "notify" request using the destination topic,
                   supplied event and event ID.
                3. Enqueue request.

            Policy: Don't ignore the caller's request.  For example, if
            the publish is given a do_method="route", satisfy that request.
        """

        message = canonicalizeMessage(message)

        requestMessage = {
            "kn_to" : topic,
            "do_method" : "notify",
            "kn_id" : str(self._C_random.random()).replace(".","_")
            }

        for name in message.keys():
            requestMessage[name] = message[name]

        requestMessage["kn_to"] = canonicalizeTopic(
            self.getServerURL(), requestMessage["kn_to"])

        self.enqueue(requestMessage, statushandler)

        return requestMessage["kn_id"]

    def subscribe(self, topic, destination, options = None, statushandler = None):
        """
            subscribe - Add an entry to the dispatch table,
            then enqueue "route" request to submit to the server.

            kn_route_location =
                subscribe(topic, destination, [options], [status-handler])

            Inputs: a source topic, a destination, subscribe options,
                    a status handler.
            Output: a route URI.

            What it does:
                1. If no route ID is provided, generate a random route ID.
                2. If no route URI is provided, generate a route URI
                   using the route ID and source topic.
                3. If the destination is local, add a dispatch table
                   entry mapping the route URI to the local destination
                   and change the destination to the journal topic.
                4. Build a "route" request using the source topic, the
                   destination, and the options.  Options are
                   parameters to the route request.
                5. Enqueue request.

            Policy: Don't ignore the caller's request.  For example, if the
            publish is given a do_method="notify", satisfy that request.
        """

        message = canonicalizeMessage(options)

        requestMessage = {
            "kn_from" : topic,
            "kn_to" : destination,
            "do_method" : "route",
            "kn_id" : str(self._C_random.random()).replace(".","_")
            }

        for name in message.keys():
            requestMessage[name] = message[name]

        requestMessage["kn_from"] = canonicalizeTopic(
            self.getServerURL(), requestMessage["kn_from"])

        # requestMessage["kn_uri"] is the route URI.

        if not requestMessage.has_key("kn_uri"):
            requestMessage["kn_uri"] = (
                requestMessage["kn_from"] + self._KN_ROUTES_ +
                urllib.quote(requestMessage["kn_id"].encode("UTF-8")))

        if not is_scalar(requestMessage["kn_to"]):
            # If requestMessage["kn_to"] is not a string,
            # it's an object with an onMessage() callback.
            self.addHandler(requestMessage["kn_uri"], requestMessage["kn_to"])
            requestMessage["kn_to"] = self.getTunnelURL()

        self.enqueue(requestMessage, statushandler)

        return requestMessage["kn_uri"]


    def unsubscribe(self, kn_route_location, statushandler = None):
        """
            unsubscribe - Remove an entry from the dispatch table,
            then enqueue "route" request to submit to the server.

            unsubscribe(kn_route_location, [status-handler])

            Inputs: a route URI, a status handler.
            Output: none.

            What it does:
                1. Remove any corresponding dispatch table entries.
                2. Parse the route URI into source topic and route ID.
                   If parsing failed, skip steps 3 and 4.
                3. Build a "route" request using the source topic,
                   route ID, and empty string as the destination.
                4. Enqueue request.
        """

        parts = urlparse.urlparse(kn_route_location)

        end_of_topic = -1

        try:
            end_of_topic = string.rindex(parts[2], self._KN_ROUTES_)
        except ValueError:
            if statushandler:
                statushandler.onStatus(
                    {
                    "status" : "400 Bad Request",
                    "kn_payload" :
                    "Client will not delete a route without the magic '%s' substring."
                    % self._KN_ROUTES_
                    })
                return

        kn_id = unicode(
            urllib.unquote(parts[2][end_of_topic + len(self._KN_ROUTES_) : ]),
            "UTF-8", "replace")

        parts = parts[:2] + (parts[2][ : end_of_topic ],) + parts[3:]

        requestMessage = {
            "kn_from" : urlparse.urlunparse(parts),
            "kn_to" : "",
            "do_method" : "route",
            "kn_id" : kn_id,
            "kn_expires" : "+5"
            }

        requestMessage["kn_from"] = canonicalizeTopic(
            self.getServerURL(), requestMessage["kn_from"])

        self.enqueue(requestMessage, statushandler)

    def onConnect(self):
        """
            Inheriting class can specialize onConnect().
        """
        pass


    def onDisconnect(self):
        """
            Inheriting class can specialize onDisconnect().
        """
        pass



class HTTPUserAgent:
    """
        Manages a pool of HTTP client connections.
    """
    def __init__(self):
        pass
    def sendRequest(self, httprequest):
        # FIXME: At some point the request should have its own close()
        # method (at which point we could return it instead).
        return HTTPConnection(httprequest)
    # TODO: def setCredentials: # both regular and proxy
    # TODO: def getCredentials: # both regular and proxy
    # TODO: def setCookies:
    # TODO: def getCookies:
    # TODO: def closeKeptAliveConnection:
    # TODO: def setConnectionPoolSizeLimit:
    # TODO: def getConnectionPoolSizeLimit:
    # TODO: def manageCache:
    # TODO: def setProxyConfig:
    # TODO: def getProxyConfig:



class HTTPConnection(asynchttp.AsyncHTTPConnection):
    """
        The client side of an HTTP Connection.
    """
    def __init__(self, httprequest):
        self._HC_httprequest = httprequest
        url = httprequest.getURL()
        parts = urlparse.urlparse(url)
        if parts[0] == "http":
            defaultport = "80"
        # elif parts[0] == "https":
        #     defaultport = "443"
        else:
            raise "Unsupported URL scheme."
        hostport = urllib.splitport(parts[1])
        self._HC_url = parts[2]
        if parts[3]:
            self._HC_url += ";" + parts[3]
        if parts[4]:
            self._HC_url += "?" + parts[4]
        port = int(hostport[1] or defaultport)
        asynchttp.AsyncHTTPConnection.__init__(self, hostport[0], port)
        # Add to asyncore.dispatcher queue.
        self.connect()
    def intercept_body(self, data):
        self._HC_httprequest.onBodyData(data)
    def handle_response(self):
        self.close()
        self._HC_httprequest.onResponseCompleted()
    def handle_connect(self):
        # Call the super, then construct the request and push it.
        asynchttp.AsyncHTTPConnection.handle_connect(self)
        self.request(self._HC_httprequest.getMethod(),
                     self._HC_url,
                     self._HC_httprequest.getBody(),
                     self._HC_httprequest.getHeaders())
        self.getresponse()



class HTTPRequest:
    """
        The client side of an outstanding HTTP Request.
    """
    def __init__(self, url, method = "GET", body = None, headers = {} ):
        self._HR_url = url
        self._HR_method = method
        self._HR_body = body
        self._HR_headers = headers
    def getURL(self):
        return self._HR_url
    def getMethod(self):
        return self._HR_method
    def getBody(self):
        return self._HR_body
    def getHeaders(self):
        return self._HR_headers
    def onBodyData(self, data):
        pass
    def onResponseCompleted(self):
        pass
    # TODO: def onStatusLine(self, responsecode, reason, version):
    # TODO: def onHeader(self, name, value):
    # TODO: def cancelRequest:
    # TODO: def onRequestSent:
    # TODO: def onHTTPRedirect:
    # TODO: def onCookieRequest:
    # TODO: def onCredentialRequest: # Both regular and proxy; make sure it's flexible enough to be used with certificates.
    # TODO: def onCertVerification:



class Transport:
    """
        Abstract client-side interface to a PubSub server.
    """
    def __init__(self, serverurl):
        self._T_serverurl = serverurl
    def getServerURL(self):
        return self._T_serverurl
    def sendMessage(self, message):
        pass
    def startTunnel(self, message):
        pass
    def stopTunnel(self):
        pass
    def onMessage(self, message):
        pass
    def onStopTunnel(self):
        pass
    def onHeartbeat(self):
        pass
    # TODO: def onStartTunnel



class SimpleTransport(Transport):
    """
        The client-side interface to a PubSub server using
        simple response format.
    """
    def __init__(self, useragent, serverurl):
        Transport.__init__(self, serverurl)
        self._ST_useragent = useragent
        self._ST_tunnel = None
    def sendMessage(self, message):
        # FIXME: We leak sockets!
        # This is a completely one-shot messaging model.  Fires and forgets.
        # This should be replaced with a retry mechanism and the ability
        # from a caller to cancel a particular message in progress.
        # And yet, this needs to be general enough that any transport
        # could provide it.
        self._ST_useragent.sendRequest(SimpleRequest(self, message))
    def startTunnel(self, message):
        if self._ST_tunnel is None:
            self._ST_tunnel = (
                self._ST_useragent.sendRequest(SimpleTunnel(self, message)))
    def stopTunnel(self):
        if not self._ST_tunnel is None:
            self._ST_tunnel.close()
            self._ST_tunnel = None



class SimpleClient(Client, SimpleTransport):
    """
        Client implementation using the SimpleTransport.
    """
    def __init__(self, *args, **kw):
        # NOTE: Order of super constructors *is* important here!
        SimpleTransport.__init__(self, *args, **kw)
        Client.__init__(self)



class SimpleParser:
    """
        Parser of messages from a PubSub server in simple response
        format:

        1. A framing format: (repeat until connection closes)

        Arbitrary amounts of ignored whitespace separating frames.*
        Each frame is prefixed by a decimal bytecount.
        Arbitrary amounts of ignored whitespace.
        A newline.
        The counted number of bytes, which form the frame.

        * whitespace = vertical_whitespace + horizontal_whitespace
          vertical_whitespace = { \\f, \\v, \\n, \\r }
          horizonal_whitespace = { \\0, \\t, ' ' }

        2. An event format used inside the frames:

        Zero or more named headers.  Each header consists of a
        name, colon followed by horizontal_whitespace, a value,
        and then a newline.

        Header name and value, for the named headers, may contain
        quoted bytes written quoted-printable style as an equal sign
        followed by 2 hexadecimal digits.  Specifically, any colons
        in the header name, any leading horizonal_whitespace in the
        value, and any vertical_whitespace or equal signs in the
        header or value, must be quoted this way.

        After the named headers, there's a newline.

        The rest of the bytes in the frame are the message payload,
        conventionally treated as if it were a header named
        kn_payload.  This payload value is not quoted in any way.

        Once all quoting has been removed, header names and
        values and payload value are assumed to be in UTF-8.
    """

    def __init__(self):
        self._SP_residue = ""
        self._SP_have_count = 0
        self._SP_count = 0

    def onMessage(self, message):
        """To be provided by classes that derive from SimpleParser."""
        pass

    def onHeartbeat(self):
        """To be provided by classes that derive from SimpleParser."""
        pass

    def handleData(self, data):
        # Called by SimpleRequest.
        self.onHeartbeat()
        self._SP_residue += data
        while self.parseResidue():
            pass

    def parseResidue(self):
        # Parse out simple formatted data from residue string.
        # Return true if we successfully consume some of residue,
        # so caller can call us again to try to consume more of residue.
        consumed = 0
        try:
            if not self._SP_have_count:
                pos = string.index(self._SP_residue, "\n")
                firstline = string.replace(self._SP_residue[ : pos ], "\0", " ")
                self._SP_residue = self._SP_residue[ pos + 1 : ]
                consumed = 1
                self._SP_count = int(firstline)
                self._SP_have_count = 1
            else:
                if len(self._SP_residue) >= self._SP_count:
                    self._SP_have_count = 0
                    frame = self._SP_residue[ 0 : self._SP_count ]
                    self._SP_residue = self._SP_residue[ self._SP_count : ]
                    consumed = 1
                    self.parseFrame(frame)
                # Else there's not enough residue yet, so return.
        except ValueError, x:
            pass  # Either residue has no new lines, or no count line yet.
        return consumed

    def parseFrame(self, frame):
        # Parse a passed-in frame for name-value pairs.
        # Same as from_rfc_822_format in cgi-bin/PubSub/EventFormat.pm .
        message = { }
        while len(frame):
            pos = string.index(frame, "\n")
            header = None
            value = None
            header = frame[ : pos ]
            frame = frame[ pos + 1 : ]
            if not pos:
                # The rest of the frame is the "kn_payload" value.
                name = "kn_payload"
                value = frame
                frame = ""
            else:
                # Now we've parsed out a header line.  Split into name and value.
                sep = string.index(header, ":")
                nameEscaped = header[ : sep ]
                valueEscaped = string.lstrip(header[ sep + 1 : ])
                # Decode them.
                nameEscapedStream = cStringIO.StringIO(nameEscaped)
                valueEscapedStream = cStringIO.StringIO(valueEscaped)
                nameStream = cStringIO.StringIO()
                valueStream = cStringIO.StringIO()
                quopri.decode(nameEscapedStream, nameStream)
                quopri.decode(valueEscapedStream, valueStream)
                nameStream.seek(0)
                valueStream.seek(0)
                name = nameStream.read()
                value = valueStream.read()
            # Decode UTF-8.
            nameU = unicode(name, "UTF-8", "replace")
            valueU = unicode(value, "UTF-8", "replace")
            # Add this name-value pair to the message.
            if message.has_key(nameU):
                valueU = message[nameU] + ", " + valueU
            message[nameU] = valueU
            continue
        self.onMessage(message)



def is_scalar(value):
    return (type(value) == types.UnicodeType or
            type(value) == types.StringType or
            type(value) == types.IntType or
            type(value) == types.LongType or
            type(value) == types.FloatType)


def stringify(value):
    """
    Unicode-safe wrapper for str().
    """
    if type(value) == types.UnicodeType:
        return value
    return str(value)

def canonicalizeTopic(base, topic):
    """
        Creates an absolute URI for a topic relative to a base server URI.
    """
    if urlparse.urlparse(topic)[0]:
        # Complete topic with protocol.
        return topic
    elif topic[0] == "/":
        # FIXME: in this case, uri is not really a relative URI. if
        # it were, /what/chat relative to server
        # http://10.10.10.30/kn/cgi-bin/kn.cgi would resolve to
        # http://10.10.10.30/what/chat, not (as it does)
        # http://10.10.10.30/kn/cgi-bin/kn.cgi/what/chat
        return base + topic
    else:
        return base + "/" + topic


def canonicalizeMessage(message):
    """
        We expect a message to be a map.  This function converts
        a message string or sequence of name-value pairs to a map,
        if needed.
    """
    if message is None:
        message = { }
    # print str(message)
    if is_scalar(message):
        message = { "kn_payload" : stringify(message) }
    canonicalMessage = { }
    try:
        # message is a sequence of (name, value) tuples.
        for name, value in message:
            if canonicalMessage.has_key(name):
                values = canonicalMessage[name]
                if is_scalar(values):
                    # single "values"
                    values = ( stringify(values) )
                if is_scalar(value):
                    # single "value"
                    value = ( stringify(value) )
                values += value
                value = values
            canonicalMessage[name] = value
    except ValueError:
        # message is a map.
        canonicalMessage = message
    # print str(canonicalMessage)
    return canonicalMessage



def encodeFormUTF_8(form):
    """
        Return a form (that is, Unicode form-data) encoded as
        x-www-form-urlencoded UTF-8.
    """
    headers = []
    canonicalForm = None;
    try:
        # sequence of (name, value) tuples.
        for nameU, values in form:
            break
        canonicalForm = form
    except ValueError:
        # map
        canonicalForm = []
        for nameU in form.keys():
            canonicalForm.append((nameU, form[nameU]))
    for nameU, values in canonicalForm:
        if is_scalar(values):
            # single value
            values = [ stringify(values) ]
        nameQ = urllib.quote(nameU.encode("UTF-8"))
        for valueU in values:
            header = [ nameQ, urllib.quote(valueU.encode("UTF-8")) ]
            headers.append("=".join(header))
    return "&".join(headers)



class SimpleRequest(HTTPRequest, SimpleParser):
    """
        The client-side of an HTTP POST Request to a PubSub server.
    """
    def __init__(self, transport, message):
        url = transport.getServerURL()
        body = encodeFormUTF_8(message)
        headers = { }
        headers["Content-type"] = "application/x-www-form-urlencoded"
        HTTPRequest.__init__(self, url, "POST", body, headers)
        SimpleParser.__init__(self)
        self._SR_transport = transport
    def getTransport(self):
        return self._SR_transport
    def onMessage(self, message):
        # Forward message to SimpleTransport instance.
        self._SR_transport.onMessage(message)
    def onBodyData(self, data):
        # Call SimpleParser's handleData method.
        self.handleData(data)



class SimpleTunnel(SimpleRequest):
    """
        The connection with a PubSub server.  A simple subclass of
        SimpleRequest that forwards its heartbeat to its transport.
    """
    def onHeartbeat(self):
        # Calls onHeartbeat of its SimpleTransport instance.
        self.getTransport().onHeartbeat()
    def onResponseCompleted(self):
        self.getTransport().onStopTunnel()



def test(url):

    # ./pubsublib.py http://127.0.0.1:8000/kn

    class _TestSimpleClient(SimpleClient):
        def onMessage(self, message):
            # print "TSC onMessage " + str(message)
            SimpleClient.onMessage(self, message)
        def onHeartbeat(self):
            print "TSC onHeartbeat"
            SimpleClient.onHeartbeat(self)
        def onConnect(self):
            print "TSC onConnect"
            SimpleClient.onConnect(self)
        def onDisconnect(self):
            print "TSC onDisconnect"
            SimpleClient.onDisconnect(self)

    class _TestSubscription:
        def __init__(self, client):
            self.client = client
        def setRID(self, rid):
            self.rid = rid
        def onMessage(self, message):
            print "TS onMessage " + str(message)
            # Use either one to stop receiving messages after the first...
            # self.client.unsubscribe(self.rid)
            # self.client.disconnect()
        def onStatus(self, message):
            print "TS onStatus " + str(message)
            self.client.publish("/what/test", "Hello Ben")

    ua = HTTPUserAgent()
    client = _TestSimpleClient(ua, url)
    sub = _TestSubscription(client)
    sub.setRID(client.subscribe("/what/test", sub, {}, sub))

    # FIXME: These are the tests needed to complete the test suite.
    # These five tests are from the JavaScript PubSub Library in
    #    mod_pubsub/kn_apps/test/

    # define_test("post and receive an event", function(),
    #    { var topic = test_topic + "/post_and_receive"
    #      left_frame_esp("post_recv.html?kn_debug=1;kn_topic=" + topic) })

    # define_test("post and get success", function()
    #    { var topic = test_topic + "/post_and_get_success"
    #      left_frame_esp("post_get_succ.html?kn_debug=1;kn_topic=" + topic) })

    # define_test("unsubscribe", function()
    #    { var topic = test_topic + "/unsubscribe"
    #      left_frame_esp("unsubscribe.html?kn_debug=1;kn_topic=" + topic) })

    # These two tests depend on our server's current brain-dead way of
    # restricting topic names.  If we fix that, we'll have to find a better
    # way to get these guys to fail.

    # define_test("subscribe and get failure", function()
    #    { var topic = test_topic + "/subscribe_and_get_failure/..."
    #      left_frame_esp("subs_get_fail.html?kn_debug=1;kn_topic=" + topic) })

    # define_test("post and get failure", function()
    #    { var topic = test_topic + "/post_and_get_failure/..."
    #      left_frame_esp("post_get_fail.html?kn_debug=1;kn_topic=" + topic) })



def main(argv):
    url = argv[1]
    test(url)
    while 1:
        asyncore.poll(scheduler.timeout())
        # print "\n\n asyncore.poll \n"
        scheduler.run()


if __name__ == "__main__": main(sys.argv)

# End of pubsublib.py
