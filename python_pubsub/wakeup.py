#!/usr/bin/python

"""
    wakeup.py -- Allows asynchronous wakeup of a python_pubsub
    "asyncore" event loop (from mod_pubsub) by other threads, by
    converting non-network events into socket read events in the
    asyncore thread.

    This is needed if you want to run multithreaded clients with a
    nonblocking event loop that can respond to non-network events as
    they happen.

    wakeup.py would be much much simpler if Python included
    socketpair() -- instead, we have to fake it using a loopback
    TCP connection.

    $Id: wakeup.py,v 1.3 2003/03/12 02:35:26 ifindkarma Exp $
    Works fine on Debian GNU Linux 3.0 with Python 2.1.3.
    We include a sample multithreaded app as a test.

    Known Issues:

        *. Race condition on startup: Another program could open
           that port after we've established the listening socket
           but before we've established the connection.


    Contact Information:
        http://mod-pubsub.sf.net/
        mod-pubsub-developer@lists.sourceforge.net
"""

## Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

## @KNOWNOW_LICENSE_START@

## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:

## 1. Redistributions of source code must retain the above copyright
## notice, this list of conditions and the following disclaimer.

## 2. Redistributions in binary form must reproduce the above copyright
## notice, this list of conditions and the following disclaimer in
## the documentation and/or other materials provided with the
## distribution.

## 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
## be used to endorse or promote any product without prior written
## permission from KnowNow, Inc.

## THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
## WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
## MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
## IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
## DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
## GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
## IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
## OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
## ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## @KNOWNOW_LICENSE_END@

## $Id: wakeup.py,v 1.3 2003/03/12 02:35:26 ifindkarma Exp $



# Include standard system libraries:
import socket, errno, sys, thread

# Include local library:
import asyncore
"""
    Note that we are using the event-driven python_pubsub asyncore,
    not the polling "standard" asyncore. -- Ben and Adam, 2/8/2003
"""



class Wakeup:
    """
        Used to asynchronously wake up the event loop without busy-waiting.
        Classes:
            Server - Listens for a single connection on a local server socket.
            Client - Connects to the local Server, creating a Connection.
            Connection - Server-side socket registered with the event loop.
        To send a wakeup notification, write a byte on the Client socket.
        To consume a wakeup notification, Connection reads a byte.
    """

    def __init__(self):
        # Create a Wakeup.Server.
        self.server = Wakeup.Server()
        # When the Server is ready, create a Wakeup.Client.
        self.client = Wakeup.Client(self.server.addr)
        # When the Client is ready, a Wakeup.Connection will be created.
        # When the Wakeup.Connection is connected, self.ready will be 1.
        while self.server.expected_wakeups or not self.server.done:
            #print "asyncore.poll starting..."
            asyncore.poll(None)
            #print "asyncore.poll done"

    def send_wakeup(self):
        # Server, Client, and Connection all need to be ready before
        # we can send a wakeup.  Now, write a byte on the Client.
        self.client.send_wakeup()

    class Server:
        def __init__(self):
            # Dynamically allocate a listening port.
            self.addr = ("127.0.0.1", 0)
            self.done = 0 # Set to 1 once our connection is established.
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.socket.bind(self.addr)
            # Only uses one connection.
            self.socket.listen(1)
            self.addr = self.socket.getsockname()
            # Registers this Server w event loop.
            asyncore.socket_map[self] = 1
            # Wait for a wakeup to prove it is working.
            self.expected_wakeups = 1
            # Server is ready once we've created it.
        def readable(self):
            return 1
        def writable(self):
            return 0
        def fileno(self):
            return self.socket.fileno()
        def handle_read_event(self):
            #print "Wakeup.Server.handle_read_event"
            (conn, addr) = self.socket.accept()
            #print "Accepted: " + str(addr)
            self.connection = Wakeup.Connection(conn, self)
            # As soon as we've established the Connection,
            # close the Server socket.
            del asyncore.socket_map[self]
            self.socket.close()
            self.done = 1
            # Loopback is ready to use.

    class Client:
        def __init__(self, addr):
            self.addr = addr
            # Client is not connected yet.
            # That will happen asynchronously from inside the event loop.
            self._pending_wakeups_lock = thread.allocate_lock()
            self.pending_wakeups = 1
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.setblocking(0)
            self.initialized = 0
            try:
                self.socket.connect(self.addr)
                self.connected()
            except socket.error, val:
                # Non-blocking sockets may cause connect() to return
                # EINPROGRESS the first time we call it...
                if val[0] == errno.EINPROGRESS:
                    pass
                else:
                    self.close()
                    raise val
            # Registers this Client with the asyncore event loop.
            asyncore.socket_map[self] = 1
        def send_wakeup(self):
            #print "Acquiring lock..."
            self._pending_wakeups_lock.acquire()
            try:
                #print "Lock acquired."
                self.pending_wakeups += 1
            finally:
                self._pending_wakeups_lock.release()
                #print "Releasing lock."
            self.handle_write_event()
        def connected(self):
            # Client is ready to deliver wakeup notifications.
            #print "Wakeup.Client.connected"
            self.initialized = 1
        def readable(self):
            return not self.initialized
        def writable(self):
            s = 0
            self._pending_wakeups_lock.acquire()
            try:
                s = self.pending_wakeups > 0
            finally:
                self._pending_wakeups_lock.release()
            return s
        def fileno(self):
            return self.socket.fileno()
        def handle_read_event(self):
            #print "Wakeup.Client.handle_read_event"
            self.socket.connect(self.addr)
            self.connected()
        def handle_write_event(self):
            #print "Wakeup.Client.handle_write_event"
            self._pending_wakeups_lock.acquire()
            try:
                if self.pending_wakeups > 0:
                    x = self.socket.send('@')
                    self.pending_wakeups -= x
            finally:
                self._pending_wakeups_lock.release()

    class Connection:
        def __init__(self, sock, server):
            self.socket = sock
            self.server = server
            self.socket.setblocking(0)
            # Register this Connection with the asyncore event loop.
            asyncore.socket_map[self] = 1
        def fileno(self):
            return self.socket.fileno()
        def handle_read_event(self):
            #print "Wakeup.Connection.handle_read_event"
            try:
                data = self.socket.recv(1)
                if self.server.expected_wakeups > 0:
                    self.server.expected_wakeups -= 1
            except socket.error, val:
                del asyncore.socket_map[self]
                self.close()
                raise "%s: read error: %s" % (self, val)
        def writable(self):
            return 0
        def readable(self):
            return 1



# Simple multithreaded test of Wakeup.

import time, threading

def main(argv):

    w = Wakeup()

    def secondThread(w):
        while 1:
            time.sleep(5)
            print "Sending async wakeup from second thread."
            w.send_wakeup()
        return

    threading.Thread(target=secondThread, args=(w,)).start()

    while 1:
        asyncore.poll(None)
        print "Wakeup in first thread."

if __name__ == "__main__": main(sys.argv)

# End of wakeup.py
