#!/usr/bin/python

"""
    repeater.py -- Command-line "repeater" application that
    subscribes to a source topic on the "from" PubSub Server
    and publishes to a mirror topic on the "to" PubSub Server.

    To call:
        ./repeater.py from-server to-server topic [ ... ]
        
    Example of usage:
        ./repeater.py http://www.mod-pubsub.org:9000/kn http://127.0.0.1:8000/kn /what/apps/blogchatter/pings

    $Id: repeater.py,v 1.6 2003/10/09 21:55:57 bsittler Exp $
    
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
import sys, getopt, asyncore, urllib

# Include local libraries:
sys.path = [ "../" ] + sys.path
import pubsublib, scheduler



class StatusMonitor:
    def __init__(self, verbose, process):
        self.verbose = verbose
        self.process = process
    def onStatus(self, event):
        if self.verbose >= 3:
            sys.stderr.write(
                "status for " + self.process + ": " + event["status"] + "\n"
                )
            sys.stderr.flush()
        pass


class Repeater(StatusMonitor):
    def __init__(self, verbose, recursive, client1, client2, topic, options):
        StatusMonitor.__init__(self, verbose, "repeating " + topic + " from " + client1.getServerURL() + " to " + client2.getServerURL())
        self.verbose = verbose
        self.recursive = recursive
        self.client1 = client1
        self.client2 = client2
        self.topic = topic
        self.options = options
        if verbose:
            sys.stderr.write(
                "repeating %s from %s to %s\n"
                % (topic, client1.getServerURL(), client2.getServerURL())
                )
            sys.stderr.flush()
        client1.subscribe(topic,
                          self,
                          options,
                          StatusMonitor(self.verbose,
                                        "subscribing to " + topic + " on " + client1.getServerURL()))
        if recursive:
            client1.subscribe(topic + '/kn_subtopics',
                              self.onSubtopic,
                              { "do_max_age": "infinity" },
                              StatusMonitor(self.verbose,
                                            "monitoring subtopics of " + topic + " on " + client1.getServerURL()))
    def onSubtopic(self, subtopic):
        try:
            assert subtopic["kn_payload"].index("kn_") == 0
        except:
            Repeater(self.verbose,
                     self.recursive,
                     self.client1,
                     self.client2,
                     self.topic + "/" + subtopic["kn_payload"],
                     self.options),
    def onMessage(self, event):
        if self.verbose >= 2:
            msg = urllib.quote(event["kn_payload"])
            if len(msg) > 50:
                msg = msg[:23] + "..." + msg[-24:]
            sys.stderr.write(
                "message in " + self.topic + ": " + msg + "\n"
                )
            sys.stderr.flush()
        self.client2.publish(self.topic,
                             event,
                             StatusMonitor(self.verbose,
                                           "publishing to " + self.topic + " on " + self.client2.getServerURL()))

def main(argv):

    bridge = 0
    recursive = 0
    verbose = 0
    repeat_style = "do_max_n"
    repeat_value = "10"

    optlist, argv[1:] = getopt.getopt(argv[1:], "hbrv",
                                      ["help", "bridge", "recursive", "max-n=", "max-age=", "verbose"])
    for opt, val in optlist:
        if opt in ('-h', '--help'):
            print (
                "Usage: %s [ options ... ] from-server to-server topic [ topic ... ]\n"
                % (argv[0]) + "\n"
                "  -h, --help          print this message and exit\n"
                "  -b, --bridge        repeat from to-server to from-server also\n"
                "  -r, --recursive     recursively repeat non-\"kn_\" subtopics\n"
                "  -v, --verbose       write diagnostics to stderr (repeat to increase verbosity)\n"
                "  --max-n=N           try to replay the last N previous events on startup\n"
                "  --max-age=AGE       try to replay the last AGE seconds on startup\n"
                "  --max-age=\"infinity\"  try to replay all old events on startup\n"
                "\n"
                "Default is to try to replay the most recent 10 events from each topic on from-server\n"
                "into the corresponding topic on to-server.\n"
                )
            return
        elif opt in ('-b', '--bridge'):
            bridge = 1
        elif opt in ('-r', '--recursive'):
            recursive = 1
        elif opt in ('-v', '--verbose'):
            verbose += 1
        elif opt in ('--max-n'):
            repeat_style = "do_max_n"
            repeat_value = val
        elif opt in ('--max-age'):
            repeat_style = "do_max_age"
            repeat_value = val

    try:
        topic = argv[3]
    except:    
        sys.stderr.write(
            "Usage: %s [ --help ] [ options ... ] from-server to-server topic [ topic ... ]\n"
            % argv[0]
            )
        sys.stderr.flush()
        sys.exit(1)
        
    client1_url = argv[1]
    client2_url = argv[2]
    
    ua = pubsublib.HTTPUserAgent()
    client1 = pubsublib.SimpleClient(ua, client1_url)
    client2 = pubsublib.SimpleClient(ua, client2_url)
    for topic in argv[3:]:
        Repeater(verbose, recursive, client1, client2, topic, { repeat_style: repeat_value })
        if bridge:
            Repeater(verbose, recursive, client2, client1, topic, { repeat_style: repeat_value })

    while 1:
        asyncore.poll(scheduler.timeout())
        scheduler.run()


if __name__ == "__main__": main(sys.argv)

# End of repeater.py
