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

## $Id: analyze.py,v 1.1 2003/04/29 07:31:51 ifindkarma Exp $

import sys
import time
import string

def processResultsFile( resFile ):
        f =open( resFile, "r" )
        theLines = f.readlines()
        # print "File " + resFile + " has " + str(len( theLines ))
        startTimes = []
        endTimes = []
        sumElapsedTimes = 0
        sumEventsReceived = 0
        elapsedTimes = []
        firstReceived = 0.0
        lastRecieved = 0.0
        throughput = 0.0
        totalElapsedTime = 0.0
        averageElapsed = 0.0
        
        for resultLine in theLines:
                theResult = string.split( resultLine, ",")
                startTimes.append( float(theResult[1]) )
                endTimes.append( float(theResult[2]) )
                elapsedTimes.append( float(theResult[3]) )
                sumElapsedTimes = sumElapsedTimes + float(theResult[3])
                sumEventsReceived = sumEventsReceived + int( theResult[0] )

        if ( min(elapsedTimes) < 0.1 ):
                print "Error!!! Error!!! Error!!!"
                print "Minimum elapsed time is less that 0.1 seconds"
                print "Operation probably didn't succeed.  The results"
                print "from this program are meaningless.  Exiting..."
                sys.exit(2)

        averageElapsed = float( sumElapsedTimes / len( theLines ))      
        firstRecieved = float(min( startTimes ))
        lastReceived = float(max( endTimes ))
        
        totalElapsedTime = lastReceived - firstReceived
        throughput = sumEventsReceived / totalElapsedTime

        print "File: " + resFile + "|" + \
              "Min Elapsed: " + str( min(elapsedTimes) ) + "|" + \
              "Max Elapsed: " + str( max(elapsedTimes) ) + "|" + \
              "Total Events: " + str( sumEventsReceived ) + "|" + \
              "Average Elapsed: " + str( averageElapsed ) + "|" + \
              "Average throughput: " + str( sumEventsReceived / averageElapsed ) 
        
        


if __name__ == "__main__":
        import sys
        import getopt
        try:
                opts, args = getopt.getopt(sys.argv[1:], 'f:')
        except getopt.error, msg:
                sys.stdout = sys.stderr
                print msg
                print "usage: analyze.py -f filename"
                print "filename: results file"
                sys.exit(2)

        resFile = None  
        print opts      
        for o, a in opts:
                if o == '-f': resFile = a

        if resFile == None:
                print "Error... You must specify a results file."
                sys.exit(2)
        
        processResultsFile( resFile )
        


