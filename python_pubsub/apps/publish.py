#!/usr/bin/python

"""
    publish.py -- Command-line publish.
    Arguments are server URL, topic, payload size, and expires.
    Defaults are http://127.0.0.1:8000/kn , /what/chat , 1024, and +15.
    
    Example of usage:
        ./publish.py http://127.0.0.1:8000/kn /what/test "1024*1024" +15

    $Id: publish.py,v 1.1 2003/05/31 02:35:36 ifindkarma Exp $
    
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

# Include local pubsub library:
sys.path = [ "../" ] + sys.path
import pubsublib, scheduler, asyncore


class publish_payload:
    def __init__(self,
                 server_url = "http://127.0.0.1:8000/kn",
                 topic = "/what/chat",
                 payload_size = "1024",
                 expires = "+15"):
        ua = pubsublib.HTTPUserAgent()
        self.client = pubsublib.SimpleClient(ua, server_url)
        print ("\nPublishing message of size " + str(eval(payload_size)) +
               " and expires " + expires + ".\n" +
               "Note that this creates two HTTPConnections: one for the tunnel" +
               " and one for the post.\n")
        self.client.publish(topic,
                            { "kn_payload" : "X" * eval(payload_size),
                              "kn_expires" : expires },
                            self)
                            # When there's timeout:
                            # , 30 + int(eval(payload_size)/1024))
        self.running = 1
    def onStatus(self, event):
        print ("\n\nMessage published.  Status is " + event['status'] + ".\n" +
               event['kn_payload'] + "\n\n")
        self.client.disconnect()
        asyncore.poll()
        self.running = 0


def main(argv):
    server_url = argv[1]
    if len(argv) > 2:
        topic = argv[2]
    if len(argv) > 3:
        payload_size = argv[3]
    if len(argv) > 4:
        expires = argv[4]
    publisher = publish_payload(server_url, topic, payload_size, expires)
    while publisher.running:
        asyncore.poll(scheduler.timeout())
        scheduler.run()
                        

                                        
if __name__ == "__main__": main(sys.argv)

# End of publish.py
