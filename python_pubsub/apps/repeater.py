#!/usr/bin/python

"""
    repeater.py -- Command-line "repeater" application
    that subscribes to a source topic on one PubSub Server
    and publishes to a mirror on another PubSub Server.

    Example of usage:
        ./repeater.py

    $Id: repeater.py,v 1.1 2003/06/11 06:32:11 ifindkarma Exp $
    
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
import sys

# Include local libraries:
sys.path = [ "../" ] + sys.path
import pubsublib, scheduler, asyncore



class StatusMonitor:
    def onStatus(self, event):
        # print "STATUS: " + str(event["status"])
        pass


class Repeater(StatusMonitor):
    def __init__(self, client):
        self.client = client
    def onMessage(self, event):
        # print "Message: " + event["kn_payload"]
        self.client.publish("/what/apps/blogchatter/pings",
                            event,
                            StatusMonitor())


ua = pubsublib.HTTPUserAgent()
client1 = pubsublib.SimpleClient(ua, "http://www.mod-pubsub.org:9000/kn")
client2 = pubsublib.SimpleClient(ua, "http://127.0.0.1:8000/kn")
client1.subscribe("/what/apps/blogchatter/pings",
                  Repeater(client2),
                  { "do_max_n": "10" },
                  StatusMonitor())

while 1:
    asyncore.poll(scheduler.timeout())
    scheduler.run()


# End of repeater.py
