#!/usr/bin/python

"""
    bridge.py -- PubSub Python Bridge for use with pubsub.py
    and mod_pubsub.  Originally written by Andrew Athan.

    We've often needed the capability to "bridge" between pubsub
    servers.  That is, to have two pubsub servers reflect the same
    topics and events on those topics.  Often, we want this to be the
    case for only a subset of the topic space of these two pubsub
    servers.  Often, we want to do transformation on the way.

    You could do this using routes between the pubsub servers -- use
    do_max_age with an inter-pubsub-server route.  Alternatively,
    bridge.py subscribes to various pubsub servers (as configured in
    the variables at the top) and attempts to synchronize topic
    hierarchies and events.

    FIXME: This should be generalized to bridge between URLs -- that
    is, it should work intra-pubsub-server as well as
    inter-pubsub-server.  For example, so that you can mimic the
    subtopic space of /what/nasdaq onto /what/realtime/app.

    FIXME: This code is neither thread safe nor completely correct.
    But it mostly works (TM).

    This app demonstrates well why the threaded PubSub Python Client
    Library libkn/libkn.py is hard to use (non-blocking IO is better).

    I hate semaphores and thread interlocks, and you'd need them if
    you tried to fix this code by batching event propagation rather
    than sending events piecemeal.  Therefore:

    FIXME: Modify bridge.py to use pubsublib.py instead of libkn.py .

    FIXME: Also you'd want a bridge like this to be adaptive; that is,
    to send events one at a time during times of low event arrival
    density, and to start batching things up if it can't keep up.
    
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
## WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
## OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
## DISCLAIMED.  IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
## OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
## BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
## LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
## USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
## DAMAGE.

## @KNOWNOW_LICENSE_END@

## $Id: bridge.py,v 1.2 2003/03/22 11:38:55 ifindkarma Exp $

from libkn import kn
import time
import types

# FIXME: Modify to work with pubsublib.py.
# import pubsublib

# FIXME: This should be generalized to allow bridging of
# topics intra-pubsub-server as well.

serverDicts = [{'host':'mypubsubserver.com',
                'port':80,
                'isTopicSource':1,
                'isEventSource':1},
               {'host':'localhost',
                'port':80,
                'isTopicSource':1,
                'isEventSource':0}]

#topics = ['/what/nasdaq','/what/edgar','/what/yahoostock']
topics = ['/foobar']

servers=[]

queueThemUpForThisManySeconds=1

class Server:
    eventQueue=[]
    def __init__(self,serverDict):
        self.serverDict=server
        self.ms=kn(host=server['host'],
              port=server['port'])
        self.ms.start()

    def addTopic(self,topic):
        print 'New topic %s on server %s'%(topic,self.serverDict['host'])
        if self.serverDict['isEventSource']:
            self.ms.subscribe(kn_from=topic,
                              cb=self.fromCallback,
                              options={'do_max_age':'infinity'})
        self.ms.subscribe(kn_from=''.join((topic,'/kn_subtopics')),
                          cb=self.subtopicCallback,
                          options={'do_max_age':'infinity'})
        if self.serverDict['isTopicSource']:
            for r in servers:
                if not r is self:
                    r.newTopic(topic)

    def fromCallback(self,ev):
        while 1:
            if queueThemUpForThisManySeconds>0:
                self.eventQueue.append(ev)
                break
            elif queueThemUpForThisManySeconds==0:
                count=self.serverDict.get('count')
                if count is None:
                    count=0
                self.serverDict['count']=count+1
                for r in servers:
                    if not r is self:
                        r.publishEvents(ev)
                break
            else:
                time.sleep(1)
            
    def distributeQueuedEvents(self):
        print 'Distributing %d events from server %s'%(len(self.eventQueue),self.serverDict)
        count=self.serverDict.get('count')
        if count is None:
            count=0
        self.serverDict['count']=count+len(self.eventQueue)
        for s in servers:
            if not s is self:
                self.publishEvents(self.eventQueue)

    def publishEvents(self,ev):
        if type(ev)==types.ListType:
            if len(ev):
                self.ms.publish(kn_to=self.fullTopicOfEvent(ev[0]),d=ev)
        else:
            self.ms.publish(kn_to=self.fullTopicOfEvent(ev),d=ev)
        
    def fullTopicOfEvent(self,ev):
        fullTopic=ev['kn_routed_from']
        end=fullTopic.rfind('/kn_subtopics')
        if end<0:
            end=len(fullTopic)
        return fullTopic[fullTopic.find('/kn')+3:end]

    def subtopicCallback(self,ev):
        newTopic=''.join((self.fullTopicOfEvent(ev),'/',ev.kn_payload))
        if newTopic.rfind('/kn_')<0:
            self.addTopic(newTopic)

    def newTopic(self,topic):
        self.ms.createTopic(topic)

if __name__ == '__main__':
    for server in serverDicts:
        s=Server(server)
        servers.append(s)

    for s in servers:
        for topic in topics:
            s.addTopic(topic)

    time.sleep(queueThemUpForThisManySeconds)
    queueThemUpForThisManySeconds=-1
    for s in servers:
        s.distributeQueuedEvents()
    queueThemUpForThisManySeconds=0

    while 1:
        time.sleep(10)
        print '---'
        for s in servers:
            print '%s -> %d'%(s.serverDict['host'],s.serverDict['count'])

# End of bridge.py
