#!/usr/bin/python

"""
    subscribe.py -- Command-line subscribe.
    Arguments are server URL, topic, and do_max_n .
    Defaults are http://www.mod-pubsub.org:9000/kn ,
    /what/apps/blogchatter/pings , and 10.
    
    Example of usage:
        ./subscribe.py http://www.mod-pubsub.org:9000/kn /what/apps/blogchatter/pings 10

    $Id: subscribe.py,v 1.1 2003/07/19 06:41:01 ifindkarma Exp $
    
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



# Include standard system libraries:
import sys, time, string, os, asyncore

# Include local libraries:
sys.path = [ "../" ] + sys.path
import pubsublib, scheduler



def sub(url, topic, do_max_n):

    class SubscribeClient:

        def __init__(self, client, topic):
            self.client = client
            self.topic = topic

        def setRID(self, rid):
            self.rid = rid

        def onStatus(self, message):
            pass
        
        def onMessage(self, message):
            now = time.localtime(time.time())
            if message.has_key("blog_url"):
                blog_url = message["blog_url"]
                blog_name = message["blog_name"]
                print ("[" + time.asctime(now) + "] " +
                       blog_name + " -- " + blog_url)
                return
            if message.has_key("displayname"):
                displayname = message["displayname"]
                print (displayname + ": " + str(message["kn_payload"]) +
                       " [" + time.asctime(now) + "]")
            else:
                print ("[" + time.asctime(now) + "] " +
                       str(message["kn_payload"]))

    ua = pubsublib.HTTPUserAgent()
    client = pubsublib.SimpleClient(ua, url)
    subscribe_client = SubscribeClient(client, topic)
    subscribe_client.setRID(client.subscribe(topic, subscribe_client,
                            { "do_max_n" : do_max_n }, subscribe_client))


def main(argv):
    server_url = "http://www.mod-pubsub.org:9000/kn"
    if len(argv) > 1:
        server_url = argv[1]
    topic = "/what/apps/blogchatter/pings"
    if len(argv) > 2:
        topic = argv[2]
    do_max_n = "10"
    if len(argv) > 3:
        do_max_n = argv[3]
    sub(server_url, topic, do_max_n)
    while 1:
        asyncore.poll(scheduler.timeout())
        scheduler.run()

                                        
if __name__ == "__main__": main(sys.argv)

# End of subscribe.py
