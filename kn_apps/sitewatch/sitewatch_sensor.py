#!/usr/bin/python

"""
    sitewatch_sensor.py -- A sensor for whether a site is up or not.

    Example of usage:
        ./sitewatch_sensor.py http://192.168.0.2/kn

    Now works with the helper libraries in python_pubsub.
    Works fine on Debian GNU Linux 3.0 with Python 2.1.3.

    $ Id: $
    
    Known Issues:

        Works fine; "TO DO": remove the asyncgeturl.py import and make
        this sensor work with mod_pubsub/python_pubsub/pubsublib.py

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
import os, sys, time, socket, inspect, errno, urllib, urlparse

# Include local libraries:
sys.path = [ "../../python_pubsub" ] + sys.path

# In this directory:
import asyncgeturl

#Included with python_pubsub distribution:
import asyncore, asynchttp, scheduler, wakeup
"""
    Note that we are using the event-driven python_pubsub asyncore,
    not the polling "standard" asyncore. -- Ben and Adam, 2/8/2003
"""



class Heartbeat:
    def __init__(self, server_url, topic, name):
        self.speed = 0
        self.server_url = server_url
        self.topic = topic
        self.name = name
        self.checked = "Never"
    def __call__(self):
        # print "Running Heartbeat."
        request = self.server_url + self.topic + '?do_method=notify&kn_id=sitewatch_heartbeat&speed=%d&statement=%s%%0aChecked:+%s'%(self.speed,urllib.quote(self.name),urllib.quote(self.checked))
        http = asyncgeturl.AsyncGetURL(request)
        # post.set_debuglevel(2)
        http.connect()
        # Run every second.
        now = time.time() + 1
        if self.speed > 0:
            self.speed -= 1
        # schedule_processing is similar to set_timeout
        scheduler.schedule_processing(self, now, "heartbeat")
    def set_speed(self, speed):
        self.speed = speed
        self.checked = time.ctime()



class Monitor:
    def __init__(self, hbeat, url):
        self.hbeat = hbeat
        self.url = url
    def __call__(self, http = None):
        # print "Running Monitor" + str((http,))
        if http is not None:
            if int(http.response.status) == 200:
                self.hbeat.set_speed(100)
            scheduler.schedule_processing(self, time.time() + 10, "monitor")
        else:
            http = asyncgeturl.AsyncGetURL(self.url, self)
            http.connect()


def main(argv):

    print "Starting SiteWatch Sensor."
 
    w = wakeup.Wakeup()

    if len(argv) < 2:
        print "Usage: ./sitewatch_sensor.py server_url"
        return -1
    else:
        server_url = argv[1]

    """Note: replace these sample sites with sites you want to watch."""
    
    hbeat = Heartbeat(server_url, "/what/sitewatch/1", "sales.knownow.com")
    Monitor(hbeat, "http://sales.knownow.com/kn?do_method=noop")()
    hbeat()

    hbeat = Heartbeat(server_url, "/what/sitewatch/2", "showcase.knownow.com")
    Monitor(hbeat, "http://showcase.knownow.com:8000/kn?do_method=noop")()
    hbeat()

    hbeat = Heartbeat(server_url, "/what/sitewatch/3", "knrouter.knownow.com")
    Monitor(hbeat, "http://knrouter.developer.knownow.com/kn?do_method=noop")()
    hbeat()

    hbeat = Heartbeat(server_url, "/what/sitewatch/4", "www.knownow.com")
    Monitor(hbeat, "http://www.knownow.com/")()
    hbeat()

    hbeat = Heartbeat(server_url, "/what/sitewatch/5", "developer.knownow.com")
    Monitor(hbeat, "http://developer.knownow.com/")()
    hbeat()

    hbeat = Heartbeat(server_url, "/what/sitewatch/6", "cvs.developer.knownow.com")
    Monitor(hbeat, "http://cvs.developer.knownow.com/index.cgi/")()
    hbeat()

    while 1:
        asyncore.poll(scheduler.timeout())
        # print "\n\n asyncore.poll \n"
        scheduler.run()

if __name__ == "__main__": main(sys.argv)

# End of sitewatch_sensor.py
