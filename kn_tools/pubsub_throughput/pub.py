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

## $Id: pub.py,v 1.1 2003/04/29 07:31:51 ifindkarma Exp $

import sys
import pycurl
import time
import threading
import string,re
import os


defaultUrl  = 'http://127.0.0.1/kn/routeTest'
threadTimes = []

class throughputPublisher(threading.Thread):

        def writeFunction(self, buf):
                #
                # Uncomment if you want the raw feedback from libcurl
                #
                # print buf
                # return len(buf)
                return(1)

        def __init__(self, myID, numEvents, size, delay, pubUrl, verbosity, outputFile=None ):
                print "initializing thread"
                urlbase = pubUrl + "?do_method=notify&kn_expires=%2b3600&kn_perfzzyyxx=1" + \
                        "&pubID=" + str(myID) + "&kn_blah=1&kn_payload="
                payload = "x" * size
                self.user = 'john:smith'
                self.numEvents = numEvents
                self.myID = str(myID)
                self.delay = delay
                self.verbosity = verbosity
                self.startTime = None
                self.endTime = None
                self.threadTimes = threadTimes
                self.outputFile = outputFile
                
                self.url = urlbase + payload
                print "URL is " + self.url

                self._lock = threading.Lock()

                self.curlhandle = pycurl.init()
                self.curlhandle.setopt( pycurl.URL, pubUrl);
                self.curlhandle.setopt( pycurl.USERPWD, self.user )

                #
                # Uncommenting this out crashes the program.  Wanted to uncomment
                # it so that we do not write to stdout the response.
                #
                # self.curlhandle.setopt( pycurl.WRITEFUNCTION, self.writeFunction )
                threading.Thread.__init__(self)


        def run(self):
                i=0;
                global threadTimes
                
                #
                # Initialize timer before the first event is sent
                #
                self.startTime = time.time()
                
                while i < self.numEvents:
                        #
                        # Get time in milliseconds
                        #
                        evtTime = "%f" % ( time.time() )
 
                        if verbosity > 1:
                                print "Thread:" + self.myID + \
                                        "|Sending message number: " \
                                        + str(i) + "|@time:" + \
                                        evtTime

                        currUrl = self.url + "&perf_time=" + evtTime \
                                + "&perf_eventnum=" + str(i)
                        if (verbosity > 2):
                                print "setting URL|" + str(currUrl)
                        self.curlhandle.setopt( pycurl.URL, currUrl )
                        self.curlhandle.perform()
                        statuscode = self.curlhandle.getinfo( pycurl.HTTP_CODE )
                        if( statuscode == 200 ):
                                if (verbosity > 0) and ( i %10 == 0 ):
                                        print "x"*30
                                        print "Thread: " + self.myID + "\nReceived 200 status code"
                                        print "Sent " + str(i) + " events"
                                        print "Incrementing counter"
                                        print "x"*30
                        else:
                                print "x"*30
                                print "Received status code " + str(statuscode)
                                print "Not incrementing counter"
                                print "x"*30
                                sys.exit(2)

                        i = i + 1
                        if ( self.delay > 0 ):
                                time.sleep( self.delay )
                

                self.endTime = time.time()
                print "Total Time: " + str( self.endTime - self.startTime )
                
                if verbosity > 0:
                        print "Calling cleanup for thread: " + self.myID
                self.curlhandle.cleanup()

                #
                # Update publish statistics
                #
                try:
                        self._lock.acquire()
                        #print "Updating publish statistics"
                        if self.outputFile != None:
                                f = open( self.outputFile, "a" )
                                f.write(str(self.numEvents) + "," + str(self.startTime) + ",")
                                f.write(str( self.endTime) + ',' + str(self.endTime - self.startTime ) + "\n" )
                                f.close()
                finally:
                        self._lock.release()


def printUsage( msg ):
        sys.stdout = sys.stderr
        print msg
        print "\n\nusage: pub.py [-f configFileName ]"
        print "where configFileName (default runtest.ini) contains:"
        print "\t-numpublishers: number of publisher threads"
        print "\t-eventsize: size of events"
        print "\t-delaybetweenpublish: delay between events"
        print "\t-eventsperpublisher: number of events"
        print "\t-v: verbosity"
        print "\t-c: use url sequencing for mulitple threads"
        print "\t-puboutputfile: output file for results"
        print "Environment Variable:"
        print "\tPUBTOPICURI - Root URI of the topic (and hence the PubSub Server) where the publishers should\
         publish events."
        sys.exit(2)


if __name__ == "__main__":
        import sys
        import getopt
        global threadTimes
        try:
                opts, args = getopt.getopt(sys.argv[1:], 'f:')
        except getopt.error, msg:
                printUsage( "Invalid command line options" )



        configFileName = "runtest.ini"
        # print opts    
        for o, a in opts:
            if o == "-f" : configFileName = str(a)

        numthreads = 1 
        size = 32
        delay = 0
        verbosity = 0
        pubUrl = None
        urlsequencing = 0
        outputFile = None

        if (os.environ.has_key('PUBTOPICURI')) :
                pubUrl = os.environ['PUBTOPICURI']
                print "PubSub Server is " + pubUrl
        else:
                print "Alert!!  PUBTOPICURI environment variable not set"
                print "I need to know which PubSub Server and topic to use."
                printUsage( "Need something like http://localhost:8000/kn/what/test.  Exiting...." )
                sys.exit(2)


        l = open(configFileName)
        for i in l.readlines() :
            o, a = string.split(i)
            print "Option is %s with argument %s" % (o, a)
            if o == '-delaybetweenpublish': delay = float(str(a))
            if o == '-numpublishers':       numthreads = int(str(a))
            if o == '-eventsize':           size = int(str(a)) 
            if o == '-eventsperpublisher':  numevents = int(str(a))
            if o == '-v':                   verbosity = int(str(a))
            if o == '-c':                   urlsequencing = 1

            # if o == '-puboutputfile':       outputFile = str(a)

        # generate the outputfile name based on configFileName
        outputFile = configFileName + "-pub-timing.out"

        print "Number of threads: " + str(numthreads)
        print "Event Size: " + str(size)
        print "Delay: " + str( delay )
        print "Number of events: " + str(numevents)
        print "Verbosity: " + str(verbosity)

        if urlsequencing != 0:
                print "Url Sequencing: Yes"
        else:
                print "Url Sequencing: No"
        
        if outputFile != None:
                print "Output File: " + outputFile
                f = open( outputFile, "w" )
                #f.write("Performance Test Publish Results:\n")
                f.close()
                
        threads = []
        for i in range(numthreads):
                reqUrl = None
                if pubUrl != None:
                        reqUrl = pubUrl
                else:
                        reqUrl = defaultUrl
                if urlsequencing != 0:
                        reqUrl = reqUrl + str( i )
                print "Publishing to: " + reqUrl
                thread = throughputPublisher( i,numevents, size, delay , reqUrl, verbosity, outputFile )
                threads.append(thread)

        for thread in threads:
                thread.start()



