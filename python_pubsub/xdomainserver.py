#!/usr/bin/python

"""
    xdomainserver.py -- a standalone (very) basic Python Web Server
    used to serve up the mod_pubsub weblications from a different port
    to the Python PubSub server to support cross domain testing.
    It was built by extending the Python SimpleHTTPServer.

    It filters any html files it opens, replacing (at the moment)
    'src="/kn' with 'src="http://PubSubServer:Port/kn" where the
    "http://PubSubServer:Port" is obtained by reading the prologue.js
    file used by the PubSub Server to setup cross domain scripting.

    Note that the usual means of specifying the port on which this
    server is to run (i.e. the command line) is ignored as currently
    this server *must* be run on port 80 to allow any weblications
    using frames to work.  Without port 80, it will fail to recognize
    that the blank frame is being opened in the same domain and try to
    open it via xdomainserver rather than the PubSub server.

    To run the cross domain test setup you need to :

    1. Setup mod_pubsub/kn_apps/kn_lib/prologue.js as required
    2. Start pubsub.py - which will read prologue.js to determine
    the port to run on (you still need the old command line params -
    it just ignores the port if prologue.js is present and is read
    successfully).
    3. Start xdomainserver.py - which will also read prologue.js to
    determine the server URL to use in its substitutions
    4. Goto http://localhost/ which will go straight to the Demo
    Weblications index.html page.
    5. Run apps/test as required.

    Contact Information:
       http://mod-pubsub.sf.net/
       mod-pubsub-developer@lists.sourceforge.net
"""

# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.
#
# @KNOWNOW_LICENSE_START@
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
#
# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# @KNOWNOW_LICENSE_END@
#
# $Id: xdomainserver.py,v 1.2 2003/04/30 23:18:47 ifindkarma Exp $

import SimpleHTTPServer
import types
import sys
import mmap
import os
from StringIO import StringIO
from serverutils import *

# define an alias (to save my poor weary fingers!)
BaseClass = SimpleHTTPServer.SimpleHTTPRequestHandler

# define the strings to search for
searchStrList = ['''src="/kn''']

# define the replacement strings
# NOTE there needs to a 1-1 correlation between the search strings and the replace strings
replaceStrList = ['''src="%s''']

# define the default mod_pubsub server location
defaultMpsServer = "http://localhost"

# define the overriding port to run this http server on
# this is necessary in order for the frame based samples to work
# TODO - find out why!
portOverride = "80"

class CrossDomainRequestHandler(BaseClass):

    def translate_path(self, path):
        """ do a bit of clean up on the path to allow
        the base class to work in more situations

        """
        # strip off any query params
        path = path.split("?")[0]
        return BaseClass.translate_path(self, path)

    def copyfile(self, source, outputfile):
        """If source is an html file perform the cross-domain server search/replace
        prior to letting the base class do its work

        """
        # assume source is untouched
        copySource = source

        # if source is an html file then memory map it to make search/replace easier
        if type(source) == types.FileType:
            if source.name.find(".html") != -1:
                mmapSource = mmap.mmap(source.fileno(), 0, access=mmap.ACCESS_READ)
                strSource = mmapSource.read(mmapSource.size())
                idx = 0
                for searchStr in searchStrList:
                    strSource = strSource.replace(searchStr, replaceStrList[idx])
                    idx = idx + 1
                copySource = StringIO(strSource)
                #print strSource
        # let the base class do the rest of the work
        return BaseClass.copyfile(self, copySource, outputfile)



def main(argv):
    # check assumption that  we are starting from the python_pubsub directory!
    appsDir = os.path.join("..", "kn_apps")
    if not os.access(appsDir, os.F_OK):
        "Apps directory: %s is not an accessible directory, exiting...\n" % appsDir

    # change to the kn_apps dir (as this basic HTTP server only references the
    # current directory and below)
    os.chdir(appsDir)

    # start out using the default mod_pubsub server address
    mpsServer = defaultMpsServer

    # read the prologue.js file to get the actual server address
    try:
        prologuePath = os.path.join(appsDir, 'kn_lib', 'prologue.js')
        mpsServer = readPubSubServerAddress (prologuePath)
    except IOError, (errno, strerror):
        sys.stderr.write(
            "Warning, problem accessing %s, (%s): %s\n" % (prologuePath, errno, strerror)
        )
        sys.stderr.flush()
        pass

    # set the PubSub Server name in the replacement list
    for i in range(0,len(replaceStrList)):
        replaceStrList[i] = replaceStrList[i] % mpsServer

    print "\nUsing mod_pubsub server: %s\n" % mpsServer

    # override the port definition to force running on port 80
    if len(argv) < 2:
        argv.append(portOverride);
    else:
        argv[1] = portOverride;
        print "Note - Forcing use of port 80 (to allow frames to work)\n"

    SimpleHTTPServer.test(CrossDomainRequestHandler)


if __name__ == "__main__": main(sys.argv)

# End of xdomainserver.py

