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

## $Id: sub.py,v 1.1 2003/04/29 07:31:51 ifindkarma Exp $

import sys
import pycurl
import string
import threading
import thread
import time


anurl = 'http://127.0.0.1/kn/routeTest/rcv49/kn_journal?kn_response_format=simple'

contents = ''

class throughputSubscriber(threading.Thread):
        def __init__(self, myID, numEvents, url, verbose):
                print "initializing thread" + str( myID )
                self.numEvents = numEvents
                self.myID = str(myID)
                self.url = url
                self.count = 0
                self.verbose = verbose
                self.curlhandle = pycurl.init()
                self.curlhandle.setopt( pycurl.URL, self.url);
                self.curlhandle.setopt( pycurl.WRITEFUNCTION, self.body_callback )
                threading.Thread.__init__(self)
                
        def run(self):
                try:
                        self.curlhandle.perform()
                except:
                        print "Calling Cleanup for thread " + self.myID
                        self.curlhandle.cleanup()       
                
        def body_callback(self, buf):
                numEvents = 0
                lastIndex = 0
                print "callback called for thread: " + str(self.myID)
                while string.find( buf, "kn_id", lastIndex) != -1:
                        if self.verbose: print "Found kn_id"
                        newIndex = string.find( buf, "kn_id", lastIndex)
                        lastIndex = newIndex + 1
                        self.count = self.count + 1
                        print "Total Events: " + str( self.count )
                        if self.count >= self.numEvents:
                                print "Thread" + self.myID + "found " + str( self.numEvents ) + " events."
                                return 0
                if self.verbose: print "done finding kn_id in current callback"
                if self.verbose: print "found " +  str(self.count) + " events"
                return len(buf)

        def performSubscribe(self):
                print 'Testing curl version', pycurl.version
                c = pycurl.init()
                c.setopt(pycurl.URL, url)
                c.setopt(pycurl.WRITEFUNCTION, body_callback)
                c.perform()
                c.cleanup()


if __name__ == "__main__":
        import sys
        import getopt
        try:
                opts, args = getopt.getopt(sys.argv[1:], 't:s:d:n:v')
        except getopt.error, msg:
                sys.stdout = sys.stderr
                print msg
                print "usage: sub.py [-t | -s | -d] [file] ..."
                print "-t: number of threads"
                print "-s: size of events"
                print "-d: delay between events"
                print "-n: number of events"
                sys.exit(2)
        numthreads = 1 
        eventsize = 32
        delay = 0 
        numevents = 0
        verbose = 0
        print opts      
        for o, a in opts:
                if o == '-d': delay = float(str(a))
                if o == '-t': numthreads = int(str(a))
                if o == '-s': size = int(str(a)) 
                if o == '-n': numevents = int(str(a))
                if o == '-v': verbose = 1

        if numevents == 0:
                print "Error... You must supply the number of events"
                print "via the -n option"
        print "Number of threads: " + str(numthreads)
        print "Event Size: " + str(eventsize)
        print "Delay: " + str( delay )

        threads = []
        for i in range(numthreads):
                thread = throughputSubscriber( i, numevents, anurl, verbose )
                threads.append(thread)
        for thread in threads:
                thread.start()
                #thread.join()

        print "Finished main thread."

