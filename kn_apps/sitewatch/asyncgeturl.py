#!/usr/bin/python

"""
	asyncgeturl.py -- A sensor for whether a site is up or not.

        Version 1.0 -- February 8, 2003.  Initial implementation.
        Works fine on Debian GNU Linux 3.0 with Python 2.1.3.

        Known Issues: None.

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

## $Id: asyncgeturl.py,v 1.1 2003/02/09 05:50:33 ifindkarma Exp $


import urllib, urlparse, asynchttp

import asyncore
"""
    Note that we are using the event-driven python_pubsub asyncore,
    not the polling "standard" asyncore. -- Ben and Adam, 2/8/2003
"""



class AsyncGetURL(asynchttp.AsyncHTTPConnection):
    def __init__(self, url, onResponse = None):
        # print "AsyncGetURL.__init__" + str((url, onResponse))
        self.onResponse = onResponse
        parts = urlparse.urlparse(url)
        hostport = urllib.splitport(parts[1])
        request = parts[2]
        if parts[3]:
            request += ";" + parts[3]
        if parts[4]:
            request += "?" + parts[4]
        # print '%-10s : %s : %s : %s' % (self.server_url, parts, hostport, request)
        asynchttp.AsyncHTTPConnection.__init__(self, hostport[0], int(hostport[1] or "80"))
        self._url = request
    def handle_response(self):
        self.close()
        if not self.onResponse is None:
            self.onResponse(self)
    def handle_connect(self):
        # print "AsyncGetURL.handle_connect"
        asynchttp.AsyncHTTPConnection.handle_connect(self)
        self.putrequest("GET", self._url)
        self.endheaders()
        self.getresponse()
                                                                                

def main(argv):
    # Do you Yahoo!?
    http = AsyncGetURL("http://www.yahoo.com/")
    http.connect()

if __name__ == "__main__": main(sys.argv)

# End of asyncgeturl.py