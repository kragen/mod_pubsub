#!/usr/bin/python

"""
    serverutils.py -- a set of utilities shared by both the PubSub
    server and the CrossDomain server.

    Contact Information:
       http://mod-pubsub.sf.net/
       mod-pubsub-developer@lists.sourceforge.net
"""

# Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
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
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 
# 3. Neither the name of the KnowNow, Inc., nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @KNOWNOW_LICENSE_END@
# 
#
# $Id: serverutils.py,v 1.2 2004/04/19 05:39:15 bsittler Exp $


import sys, os, stat


'''
    attempt to read the address of the PubSub server from the supplied file
    returns the address string of the PubSub server read from the file
    or None if not found
    throws IOError if problem accessing the file
'''
def readPubSubServerAddress (sourceFileName):
    mpsServer = None
    prologueFile = open(sourceFileName, 'rb')
    size = os.fstat(prologueFile.fileno())[stat.ST_SIZE]
    buffer = prologueFile.read(size)
    idx = buffer.find('"http:')
    # if found the server name then proceed
    if idx != -1:
        endIdx = buffer.find('";', idx)
        mpsServer = buffer[idx + 1:endIdx]
    prologueFile.close()
    return mpsServer

def exitWithError (errStr):
    sys.stderr.write(errStr)
    sys.stderr.flush()
    sys.exit(2)
