#!/usr/bin/python

"""
    chat.py -- Command-line chat using the command-line-provided
    PubSub Server URL and topic.  Demonstrates pubsublib.py .
    Similar to mod_pubsub/kn_tools/chat.plx except that it
    doesn't use curses for bold and colors.

    Example of usage:
        ./chat.py http://127.0.0.1:8000/kn /what/chat

    $Id: chat.py,v 1.2 2004/04/19 05:39:15 bsittler Exp $
    
    Contact Information:
        http://mod-pubsub.sf.net/
        mod-pubsub-developer@lists.sourceforge.net
"""

## Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

## @KNOWNOW_LICENSE_START@
## 
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 
## 1. Redistributions of source code must retain the above copyright
## notice, this list of conditions and the following disclaimer.
## 
## 2. Redistributions in binary form must reproduce the above copyright
## notice, this list of conditions and the following disclaimer in the
## documentation and/or other materials provided with the distribution.
## 
## 3. Neither the name of the KnowNow, Inc., nor the names of its
## contributors may be used to endorse or promote products derived from
## this software without specific prior written permission.
## 
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
## A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
## OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
## 
## @KNOWNOW_LICENSE_END@
## 



# Include standard system libraries:
import sys, time, string, os, asyncore

# Include local libraries:
sys.path = [ "../" ] + sys.path
import pubsublib, scheduler



def chat(url, topic):

    class ChatClient:

        def __init__(self, client, topic):
            self.client = client
            self.topic = topic
            self.running = 1

        def setRID(self, rid):
            self.rid = rid

        def onMessage(self, message):
            now = time.localtime(time.time())
            displayname = "unknown"
            if message.has_key("displayname"):
                displayname = message["displayname"]
            print (displayname + ": " + str(message["kn_payload"]) +
                   " [" + time.asctime(now) + "]")

        def onStatus(self, message):
            # Status for the route request.
            # self.client.publish(self.topic, "Connected to " + self.topic + " ...")
            self.stdin_channel(self)

        def onInput(self, message):
            displayname = os.getenv("USER")
            if displayname is None:
                displayname = os.getenv("LOGNAME")
            if displayname is None:
                displayname = "Anonymous User"
            self.client.publish(self.topic,
                                { "displayname" : displayname, "kn_payload" : message })

        class stdin_channel (asyncore.file_dispatcher):
            def __init__(self, chat):
                asyncore.file_dispatcher.__init__(self, 0)
                self.buffer = ""
                self.chat = chat
            def handle_read(self):
                data = ""
                try:
                    data = self.recv(512)
                except:
                    pass
                if not data:
                    self.handle_close()
                self.buffer += data
                while 1:
                    data = ""
                    try:
                        data = self.recv(512)
                    except:
                        pass
                    if not data:
                        break
                    else:
                        self.buffer += data
                while 1:            
                    split_buffer = string.split(self.buffer, "\n", 1)
                    if len(split_buffer) > 1:
                        self.chat.onInput(split_buffer[0])
                        self.buffer = split_buffer[1]
                    else:
                        break
            def writable(self):
                return 0
            def log(self, *ignore):
                pass
            def handle_close(self):
                self.chat.running = 0

    ua = pubsublib.HTTPUserAgent()
    client = pubsublib.SimpleClient(ua, url)
    chat_client = ChatClient(client, topic)
    chat_client.setRID(client.subscribe(topic, chat_client,
                                        { "do_max_age" : "600" }, chat_client))
    return chat_client


def main(argv):
    server_url = argv[1]
    topic = "/what/chat"
    if len(argv) > 2:
        topic = argv[2]
    chat_client = chat(server_url, topic)
    while chat_client.running:
        asyncore.poll(scheduler.timeout())
        scheduler.run()

                                        
if __name__ == "__main__": main(sys.argv)

# End of chat.py
