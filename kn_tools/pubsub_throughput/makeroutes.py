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

## $Id: makeroutes.py,v 1.1 2003/04/29 07:31:51 ifindkarma Exp $

# This app makes routes from all the publisher topics to the journals of 
# the various subscribers.  The program divvies up the total number of 
# subscribers by across all the publishers.  So, 1000 subscribers with 
# 50 publishers will in essence cause this program to create 50 topics 
# where the publishers are expected to POST (PUBTOPICURI and publisher
# are used to create a topic for a specific publisher).  And, 20 
# subscribers will subscribe to each publisher.  We repeat: not ALL the 
# subscribers are subscribed to ALL the publishers!!!

# This app assumes that the publisher is used in URL sequencing mode.  
# This app is needed since the next step is to invoke the subscribing 
# program which program can only run 1000 threads, hence there have to be 
# numerous invocations of the program as we use X thousands of subscribers...

# Input
#
# config file - a configuration file that specifies number of publishers, 
#               subscribers, payload size, sequeunce start/end
# PUBTOPICURI - environment variable that specifies the topic URI used 
#               by the publisher to publish the events into the PubSub Server
# SUBTOPICURI - environment variable that specifies the topic URI to be 
#               used by all the subscribers

# Issues:
# We are sending the payload in the URL so the since of the payload is restricted.

import sys
import pycurl
import time
import threading
import string,re
import os

verbose = 0

# Callbacks for header and body data.

def body(buf):
        # Print body data.
        # sys.stdout.write(buf)
        pass

def header(buf):
        # Print header data.
        # sys.stdout.write(buf)
        pass


def makeroutes( curlhandle, sourceUrl,  urlbase, startNum, endNum ):
        
        print "Called makeroutes"
        print "sourceUrl: " + sourceUrl
        print "urlbase: " + urlbase
        print "startNum: " + str(startNum)
        print "endNum: " + str(endNum)
        
        i = startNum
        print "i: " + str(i) + " endNum: " + str(endNum)
        while i <= endNum:
                requestUrl = urlbase + "?do_method=route&kn_from=" + sourceUrl + \
                             "&kn_to=" + urlbase + str(i) + "/kn_journal&kn_expires=infinity" 
                if ( verbose ):
                        print "Request url:\n" + requestUrl
                curlhandle.setopt( pycurl.URL, requestUrl);
                curlhandle.perform()
                statuscode = curlhandle.getinfo( pycurl.HTTP_CODE )
                # Spit out success only if in debug mode.
                if( statuscode == 200 ):
                        if (verbose):
                                print "x"*30
                                print "Received 200 status code for route #" + str(i)
                                print "Incrementing counter"
                                print "x"*30
                        i = i + 1
                        if (i % 50 == 0):
                                print "Made " + str(i) + \
                                        " routes so far; sleeping for 2 seconds" + \
                                        " time= " + str(time.time())
                                sys.stdout.flush()
                                time.sleep(2)

                else:
                        print "x"*30
                        print "Error! Error! Error!"
                        print "Received status code " + str(statuscode)
                        print "Not incrementing counter that's at " + str(i)
                        print "x"*30
                        sys.exit(2)
        i  = i+1



def printusage( msg ):
        sys.stdout = sys.stderr
        print msg
        print "usage: makeroutes.py [-f configFileName ] -o subscribers-run.shell"
        print " -o to define the shell file that is created and used to run the subscribers"
        print " where configFilename (default runtest.ini) contains - "
        print "\t-startseq:      start number for sequence"
        print "\t-endseq:        end number for sequence"
        print "\t-numpublishers: number of publishers (default = 1)\n\t \
        Note: Number of publishers must be a multiple of the number of routes created"
        print "\t-verbose:       verbose output level\n"
        print "\t-h: help (this message)"
        print "\nEnvironment Variables\n"
        print "\tPUBTOPICURI - topic URI where publishers will publish events"
        print "\tSUBTOPICURI - topic URI where the subscirbers will create journals"
        sys.exit(2)

if __name__ == "__main__":
        import sys
        import getopt
        try:
                opts, args = getopt.getopt(sys.argv[1:], 'f:o:')
        except getopt.error, msg:
                sys.stdout = sys.stderr
                printusage( msg )

        startNum = 0
        endNum = 0
        urlBase = None
        sourceUrl = None
        numPub = 1
        configFileName = "runtest.ini"
        verboseRun = ""

        # Pick up the URI of the topic (and hence the PubSub Server)
        # that should be used by the publisher.

        if (os.environ.has_key('PUBTOPICURI')) :
                pubUrlBase = os.environ['PUBTOPICURI']
                print "Topic URI (and hence PubSub Server) is " + pubUrlBase
        else:
                print "Alert!!  PUBTOPICURI environment variable not set."
                print "Specify which PubSub Server and topic to use."
                print "Need something like http://localhost:8000/kn/what/test."
                print "Exiting...."
                sys.exit(2)

        # Pick up the URI of the topic (and hence the PubSub Server)
        # that should be used by the subscribers.

        if (os.environ.has_key('SUBTOPICURI')) :
                subscriberURLBase = os.environ['SUBTOPICURI']
                print "Subscription Topic URI is " + subscriberURLBase
        else:
                print "Alert!!  SUBTOPICURI environment variable not set."
                print "Specify which PubSub Server and topic to use as root node for journals."
                print "Need something like http://localhost:8000/kn/what/subscribers."
                print "Exiting...."
                sys.exit(2)

        subShellFileName = ""
        # Print opts.
        for o, a in opts:
                if o == '-f': configFileName = a
                if o == '-o': subShellFileName = a

        if (subShellFileName == "" ):
                subShellFileName = "sub.bash"
        subshOutf = open(subShellFileName, "w");

        # Read the config file.
        l = open(configFileName)
        for i in l.readlines() :
            o, a = string.split(i)
            print "Option is %s with argument %s" % (o, a)
            if o == '-startseq':            startNum = int(str(a))
            if o == '-endseq':              endNum   = int(str(a))
            if o == '-numpublishers':       numPub = int(str(a))
            if o == '-eventsperpublisher':  numEvents = int(str(a))
            if o == '-v':                   
                     verboseRun = "-v " + (str(a))
                     verbose = int(str(a))

        if( startNum < 0 or endNum < 0 or subscriberURLBase == None or pubUrlBase == None ):
                printusage( "Invalid command line arguments" )
        if numPub < 1:
                printusage( "Number of publishers cannot be less than 1")
        if (( endNum - startNum ) + 1)  % numPub != 0:
                printusage( "Number of publishers must be a multiple of number of routes")
                
        print "Start Number: " + str(startNum)
        print "End Number: " + str(endNum)
        print "Base url: " + subscriberURLBase 
        print "Source url: " + pubUrlBase
        print "Number of Publishers: " + str( numPub )

        # UNDERSTAND THIS:
        # The number of subscribers doesn't mean that each subscriber will 
        # subscribe to each publisher!!!! The set of subscribers will be divvied 
        # up by the number of publishers.  So, 1000 subscriber with 5 publishers 
        # means that the 5 publisher will print into their own topic (when 
        # using URL sequencing on the pubs).  Hence, each topic will have 200 
        # subscribers.

        i = 0
        tmpStartNum = startNum
        tmpEndNum = 0
        pubUrl = pubUrlBase
        incrementPerPublisher = ((endNum - startNum) + 1)/numPub
        if (incrementPerPublisher > 1000 ):
                print "Error! Error! Error!"
                print "THe number of subscribers is divvied up against the "
                print " number of publishers.  But, there can only be upto "
                print "1000 subscribers per publisher in this test suite."
                print "Parameters passed to this program set number of subscribers"
                print "per publisher to " + str(incrementPerPublisher)
        
        curlhandle = pycurl.init()
        curlhandle.setopt( pycurl.WRITEFUNCTION, body )
        curlhandle.setopt( pycurl.HEADERFUNCTION, header )
        
        while i < numPub:
                if( numPub != 1 ):
                        tmpStartNum = incrementPerPublisher * i
                        tmpEndNum = tmpStartNum + (incrementPerPublisher - 1)
                else:
                        tmpEndNum = endNum

                #  Beware: this is tightly coupled to pub.py

                pubUrl = pubUrlBase + str(i)
                print "--------------------------------------------------"
                print "Iteration for makeroutes"
                print "i: " + str(i)
                print "pubUrl: " + pubUrl
                print "subscriberURLBase: " + subscriberURLBase
                print "tmpStartNum: " + str(tmpStartNum)
                print "tmpEndNum: " + str(tmpEndNum)
                makeroutes( curlhandle, pubUrl, subscriberURLBase, 
                            tmpStartNum, tmpEndNum )

                # Define how the subscribers will be run.  The subscriberURLBase 
                # is used in conjunction with the sequence number to derive 
                # the kn_journal to be used.  Each throughput_sub program can run 
                # up to 1000 threads where each connects to its own kn_journal.

                subshOutf.write ( "throughput_sub -t " + 
                                  str(((tmpEndNum - tmpStartNum) + 1)) + \
                                  " " + verboseRun + \
                                  " -n " + str(numEvents) + \
                                  " -o " + configFileName + "-sub" + str(i) + \
                                       "-timing.out" + \
                                  " -seq -startseq " + str(tmpStartNum) + \
                                  " -endseq " + str(tmpEndNum) + \
                                  " -urlbase " + subscriberURLBase + \
                                  " > " + configFileName + "-sub" + str(i) \
                                        + ".log 2>&1 &\n" )
                
                # Allow the previous process to start up, connect, and
                # open the connections.
                # subshOutf.write ( "sleep 5")
                i = i + 1

        print "Calling cleanup"
        curlhandle.cleanup()
        subshOutf.close()

        print "Run the publisher w/ this command:"
        print "python pub.py -f " + configFileName + " > pub." + configFileName + ".log"
        print "Successful makeroutes.py run"
        sys.exit(0)
