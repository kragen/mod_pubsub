#!/usr/bin/python

"""
    libkn.py -- PubSub Python Client Library
    for use with pubsub.py & mod_pubsub

    This is a simple Python client library for use in event-driven
    programs.  It supports multi-threaded, blocking publish-and-subscribe
    semantics.  (By contrast, python_pubsub/pubsublib.py supports
    single-threaded, non-blocking publish-and-subscribe semantics.)

    Original version supplied by Phil Harris
    (phil dot harris at zope dot co dot uk).

    This version contains significant modifications by Andrew Athan.

    To do:
    * Cleanup.
    * Write test cases.

    $Id: libkn.py,v 1.2 2003/04/29 06:44:17 ifindkarma Exp $
    
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

## $Id: libkn.py,v 1.2 2003/04/29 06:44:17 ifindkarma Exp $


# Modified for this distribution:
import urllib3, quopri_s, timeoutsocket

# Part of the standard distribution:
import string, re, time, random, threading
import mimetools, types, sys, traceback, copy, select
from pprint import pprint

timeoutsocket.setDefaultSocketTimeout(15)
urllib=urllib3

workers=0
lock=threading.Lock()

try:
        import cStringIO
        StringIO=cStringIO
except:
        import StringIO



class CircularArray:
    def __init__(self,size):
        self.data=[None]*size
        self.size=size
        self.start=-1
        self.end=0
        self.count=0
        self.lost=0

    def push(self,d):
        if self.start==self.end and self.count:
            ret=self.data[self.start]
            self.lost=self.lost+1
        else:
            self.count=self.count+1
            ret=None
            if self.start<0:
                self.start=0
        
        self.data[self.end]=d
        self.end=self.end+1
        if self.end>=self.size:
            self.end=0
            
        return ret

    def pull(self,count=1):
        #print self.start,self.end,self.count,count
        if self.count:
            if count==1:
                self.count=self.count-1
                d=self.data[self.start]
                self.data[self.start]=None
                self.start=self.start+1
                if self.start>=self.size:
                    self.start=0
                return d
            else:
                if count>self.count:
                    count=self.count
                if self.end>self.start:
                    d=self.data[self.start:self.start+count]
                    self.count=self.count-count
                    self.start=self.start+count
                    return d
                else:
                    end=self.start+count
                    if end>self.size:
                        end=self.size
                    d=self.data[self.start:end]
                    left=count-end+self.start
                    if left:
                        d.extend(self.data[0:left])
                        self.start=left
                    else:
                        self.start=end
                    self.count=self.count-count
                    return d
        return None
                

    def pop(self):
        if not self.start==self.end and self.start>=0:
            self.count=self.count-1
            self.end=self.end-1
            if self.end<0:
                self.end=self.size-1
            d=self.data[self.end]
            self.data[self.end]=None
            return d
        return None


def encodeHeaders(dictOrTupleList):
        return urllib.urlencode(dictOrTupleList)

class pyMessage(mimetools.Message):
    def isheader(self, line):
        """Determine whether a given line is a legal header.

        This method should return the header name, suitably canonicalized.
        You may override this method in order to use Message parsing
        on tagged data in RFC822-like formats with special header formats.
        """
        i = line.find(':')
        if i > 0:
            return line[:i]
        else:
            return None

    def __getitem__(self, name):
        """Get a specific header, as from a dictionary."""
        return self.dict[name]

    def __setitem__(self, name, value):
        """Set the value of a header.

        Note: This is not a perfect inversion of __getitem__, because
        any changed headers get stuck at the end of the raw-headers list
        rather than where the altered header was.
        """
        del self[name] # Won't fail if it doesn't exist
        self.dict[name] = value
        text = name + ": " + value
        lines = text.split("\n")
        for line in lines:
            self.headers.append(line + "\n")

    def __delitem__(self, name):
        """Delete all occurrences of a specific header, if it is present."""
        name = name
        if not self.dict.has_key(name):
            return
        del self.dict[name]
        name = name + ':'
        n = len(name)
        list = []
        hit = 0
        for i in range(len(self.headers)):
            line = self.headers[i]
            if line[:n] == name:
                hit = 1
            elif not line[:1].isspace():
                hit = 0
            if hit:
                list.append(i)
        list.reverse()
        for i in list:
            del self.headers[i]
        

class knEvent:
        def __str__(self):
                return str(self.rawEvent)
        
        def __init__(self,e=None):
                self.rawEvent={}
                if not e is None:
                        for k,v in e.items():
                                self.rawEvent[k]=urllib.unquote(v)

        def __getattr__(self,key):
                return self.rawEvent[key]

        def __getitem__(self,key):
                return self.rawEvent[key]

        def __setitem__(self,key,value):
                self.rawEvent[key]=value

        def get(self,key):
                return self.rawEvent.get(key)

        def items(self):
                return self.rawEvent.items()

knEventType=type(knEvent({}))

class kn(threading.Thread):
        
        def __init__(self,host='localhost',port=8000,user=None,password=None,traceOnly=0):
                self.recvArray=CircularArray(100)
                self.topics={}
                self.routes={}
                self.die=0
                self.isStarted=0
                self._returnNow=0
                threading.Thread.__init__(self)
                self.connected=self.isAlive
                self.host=host
                self.port=port
                self.user=user
                self.password=password
                self.traceOnly=traceOnly
                self.lock=threading.Lock()
                self.kn_journal="/what/apps/py_kn/%s%s/kn_journal" % (time.time(), random.random())
                self.run_exception_callback=None
                self.run_journal_callback=None
                self.outboundThread=None
                self.subchan=None
                self.subchanlock=threading.RLock()
                if self.user:
                        #aa1.3:
                        #kn_response_format cruft due to a bug in 1.3
                        #if self.password:
                        #       self.kn_url_base='http://%s:%s@%s:%s/kn/?kn_response_format=simple' % (self.user,self.password,self.host,self.port)
                        #else:
                        #       self.kn_url_base='http://%s@%s:%s/kn/?kn_response_format=simple' % (self.user,self.host,self.port)
                #else:
                        #self.kn_url_base='http://%s:%s/kn/?kn_response_format=simple' % (self.host,self.port)
                        if self.password:
                                if self.port==80:
                                        self.kn_url_base='http://%s:%s@%s/kn/' % (self.user,self.password,self.host)
                                else:
                                        self.kn_url_base='http://%s:%s@%s:%s/kn/' % (self.user,self.password,self.host,self.port)
                        else:
                                if self.port==80:
                                        self.kn_url_base='http://%s@%s/kn/' % (self.user,self.host)
                                else:
                                        self.kn_url_base='http://%s@%s:%s/kn/' % (self.user,self.host,self.port)
                else:
                        if self.port==80:
                                self.kn_url_base='http://%s/kn/' % (self.host)
                        else:
                                self.kn_url_base='http://%s:%s/kn/' % (self.host,self.port)
                #self.kn_url_base_no_slash=self.kn_url_base[:-1]

        def start(self,onException=None,onJournal=None,outboundThread=0):
                self.lock.acquire()
                if self.isStarted:
                        self.lock.release()
                        return
                self.setDaemon(1)
                
                if onException==None:
                        self.run_exception_callback=self.default_run_exception_callback
                else:
                        self.run_exception_callback=onException
                        
                if onJournal==None:
                        self.run_journal_callback=self.default_run_journal_callback
                else:
                        self.run_journal_callback=onJournal

                if outboundThread:
                        self.outboundThread=threading.Thread(target=self.outboundThreadTarget)
                        self.outboundThread.cv=threading.Condition()
                        self.outboundThread.toSend=[]
                        self.outboundThread.working=0
                        self.outboundThread.setDaemon(1)
                        self.outboundThread.start()
                        
                threading.Thread.start(self)
                while(self._returnNow==0):
                        pass

                self.isStarted=1
                self.lock.release()

        def unQuote(self,s):
                fi=StringIO.StringIO(s)
                s=quopri_s.decode(fi)
                fi.close()
                return s

        def parseHeaders(self,headers='',strip_first=1):
                if strip_first: headers=string.join(string.split(headers,'\n')[1:],'\n')
                f=StringIO.StringIO(headers)
                m=pyMessage(f)
                return m

        #Man, this pubsub client library needs a rewrite
        def outboundThreadTarget(self):
                global workers,lock
                ot=self.outboundThread
                while not self.die:
                        ot.cv.acquire()
                        sendQueue=ot.toSend
                        if not len(sendQueue):
                                #print 'Waiting %s:%d %s'%(self.host,self.port,threading.currentThread())
                                ot.cv.wait()
                                #print 'Released %s:%d %s'%(self.host,self.port,threading.currentThread())
                        ot.toSend=[]
                        ot.cv.release()

                        if sendQueue:
                                ot.working=1
                                #lock.acquire()
                                #workers=workers+1
                                #lock.release()
                                #print '--------------------------->%d'%workers
                                #print '--------------------------->%d'%len(sendQueue)
                                x=0
                                while x<len(sendQueue):
                                        q=sendQueue[x]
                                        try:
                                                if q[0]=='req':
                                                        self.finishReqeust(q[1],q[2],q[3])
                                                        #h=urllib.urlopen(q[1],q[2])
                                                        #headers=h.read()
                                                        #m=self.parseHeaders(headers)
                                                        #h.close()
                                                        #q[3][0]=m
                                                elif q[0]=='subscribe':
                                                        self.finishRequest(self.kn_url_base,q[1],q[2])
                                        except:
                                                #timeouts, everything, is ignored
                                                pass
                                        x=x+1
                                #lock.acquire()
                                #workers=workers-1
                                #lock.release()
                                ot.working=0
                                #print '--------------------------->%d COMPLETE'%len(sendQueue)
                                

        def sendRequest(self, url='',options='',ret=None):
                if self.traceOnly:
                        if self.traceOnly<2:
                                print 'sendRequest(url=%s,options=%s)'%(url,options)
                        return {'status':'200=20Notified',
                                'kn_id':'45461f8a-9e64-11d5-bd98-080020b290df'}

                try:
                        if options:
                                #aa1.3: This cruft is necessary because of a bug in 1.3
                                if type(options)==types.DictionaryType:
                                        options=copy.copy(options)
                                        options['kn_response_format']='simple'
                                elif type(options)==types.ListType:
                                        options=copy.copy(options)
                                        options.append(('kn_response_format','simple'))
                                params=encodeHeaders(options)
                        else:
                                params=None

                        if self.outboundThread:
                                self.outboundThread.cv.acquire()
                                if ret is None:
                                        ret=[None]
                                self.outboundThread.toSend.append(('req',url,params,ret))
                                self.outboundThread.cv.notify()
                                self.outboundThread.cv.release()
                        else:
                                self.subchanlock.acquire()
                                #h=urllib.urlopen(url,params)
                                ret=self.finishRequest(url,params)
                                self.subchanlock.release()
                                return ret

                        if not self.outboundThread:
                                headers=h.read()
                                
                                m=self.parseHeaders(headers)
                                h.close()
                                return m
                        else:
                                return ret
                except KeyboardInterrupt,e:
                        raise
                except timeoutsocket.Timeout,e:
                        print 'Timeout'
                        return
                except Exception,e:
                        print 'Exception %s raised in sendRequest'%e
                        traceback.print_exc()
                        print '---'
                        traceback.print_stack()
                        return

        def batchSubscribe(self,subs):
                #headers=[('do_method','batch')]
                #for sub in subs:
                #        kn_from=sub[0]
                #        kn_to=sub[1]
                #        if not kn_to:
                #                kn_to=self.kn_journal
                #        options=sub[2]
                #        cb=sub[3]
                #        ret=sub[4]
                #        req=[('do_method','route'),
                #             ('kn_from',kn_from),
                #             ('kn_to',kn_to)]
                #        if type(options)==types.DictionaryType:
                #                for k,v in options.items():
                #                        req.append((k,v))
                #        elif type(options)==types.ListType:
                #                req=req+options
                #        headers.append(('kn_batch',urllib.urlencode(req)))
                for sub in subs:
                        kn_from=sub[0]
                        kn_to=sub[1]
                        if not kn_to:
                                kn_to=self.kn_journal
                        options=sub[2]
                        cb=sub[3]
                        ret=sub[4]
                        self.subscribe(kn_from,kn_to,options,cb,ret)

        def subscribe(self,kn_from='',kn_to=None,options=None,cb=None,ret=None):
                #self.batchSubscribe([(kn_from,kn_to,options,cb,ret)])
                #return
                if not kn_from:
                        raise 'Blank topics are not allowed.'
                if not cb and not kn_to:
                        raise 'You must have a callback.'
                if not kn_to:
                        kn_to=self.kn_journal

                opt=[('do_method','route'),
                     ('kn_response_format','simple'),
                     ('kn_to',kn_to),
                     ('kn_from',kn_from)]
                if type(options)==types.DictionaryType:
                        for k,v in options.items():
                                opt.append((k,v))
                elif type(options)==types.ListType:
                        opt=opt+options

                #must register the topic before making the request, in order to avoid
                #race condition.  Otherwise, the receiving thread might get the
                #pubsub server's response before we return from this call
                dict={'cb':cb}
                #self.topics[''.join((self.kn_url_base_no_slash,kn_from))]=dict
                self.topics[kn_from]=dict

                opt=encodeHeaders(opt)
                if self.outboundThread:
                        self.outboundThread.cv.acquire()
                        if ret is None:
                                ret=[None]
                        self.outboundThread.toSend.append(('subscribe',opt,ret))
                        self.outboundThread.cv.notify()
                        self.outboundThread.cv.release()
                        return
                        
                
                self.subchanlock.acquire()
                ret=self.finishRequest(self.kn_url_base,opt,ret)
                self.subchanlock.release()
                return ret


        def finishRequest(self,loc,opt,ret=None):
                while 1:
                        if self.subchan is None:
                                url=urllib.urlopen(loc,opt)
                                self.subchan=url.h()
                                break
                                        
                        else:
                                try:
                                        if loc.startswith('http://'):
                                                loc=loc[len(self.kn_url_base)-4:]
                                        self.subchan.putrequest('POST', loc)
                                        self.subchan.putheader('Content-type', 'application/x-www-form-urlencoded')
                                        self.subchan.putheader('Content-length', '%d' % len(opt))
                                        self.subchan.endheaders()
                                        self.subchan.send(opt)
                                        errcode,errmsg,headers=self.subchan.getreply()
                                        break
                                except Exception,e:
                                        print 'Oh crap %s %s'%(type(e),e.__class__)
                                        self.subchan=None
                                        #print 'Sleeping...'
                                        #time.sleep(5)
                                        #print 'Done...'
                                        #traceback.print_exception()
                                        continue

                fp=self.subchan.getfile()
                headers=fp.read()
                m=self.parseHeaders(headers)
                fp.close()
                if ret:
                        ret[0]=(m['status'],m['kn_id'])
                else:
                        return (m['status'],m['kn_id'])

        def unsubscribe(self,kn_from='',rid=''):
                if not kn_from:
                        raise 'Blank topics are not allowed'
                if not rid:
                        raise 'You must supply a route id'
                m=self.sendRequest(url=self.kn_url_base,options=[
                                                ('do_method','route'),
                                                ('kn_to',''),
                                                ('kn_from',kn_from),
                                                ('kn_id',rid)])
                #I'm in a hurry
                if self.outboundThread:
                        return m
                
                if m!=None:
                        return m['status']
                else:
                        return None
                
        def publish(self,kn_to=None,d=None):
                # we should be checking for maximum URL length of 2048

                if (type(d)==types.ListType and
                    (type(d[0])==types.ListType or
                     type(d[0])==types.DictionaryType or
                     type(d[0])==knEventType)):
                        if len(d)>1:
                                #We should send separate requests past some len(d)
                                if self.traceOnly==1:
                                        print 'publish multiple'
                                url=self.kn_url_base
                                headers=[('do_method','batch')]
                                new_kn_to=('kn_to',kn_to)
                                for b in d:
                                        event=[('do_method','notify')]
                                        if type(b)==types.ListType:
                                                event+=b
                                        else:
                                                event+=b.items()
                                        if not kn_to is None:
                                                found=0
                                                for e in range(len(event)):
                                                        if event[e][0]=='kn_to':
                                                                event[e]=new_kn_to
                                                                found=1
                                                                break
                                                if not found:
                                                        event.append(new_kn_to)
                                                        
                                        headers.append(('kn_batch',urllib.urlencode(event)))
                        elif len(d)==1:
                                return self.publish(kn_to,d[0])
                        else:
                                return
                else:
                        #Bugs in here when type(d)=knEventType
                        if self.traceOnly==1:
                                print 'publish single'
                        if kn_to is None:
                                url=self.kn_url_base
                                headers=d
                        else:
                                #This crap is necessary because of the 1.3 bug
                                #url='%s&kn_to=%s'%(self.kn_url_base,kn_to)
                                url=self.kn_url_base
                                if not type(d)==types.ListType:
                                        headers=d.items()
                                else:
                                        headers=copy.copy(d)
                                found=0
                                for e in range(len(headers)):
                                        if headers[e][0]=='kn_to':
                                                headers[e]=new_kn_to
                                                found=1
                                                break
                                if not found:
                                        headers.append(('kn_to',kn_to))

                m=self.sendRequest(url=url,options=headers)
                #I'm in a hurry
                if self.outboundThread:
                        return m
                
                if m is None:
                        return None
                else:
                        try:
                                return m[1]
                                #return m['kn_id']
                        except:
                                print "Hmmmm .... m['kn_id'] exception"
                                print "m = "
                                print m
                        return None

        def createTopic(self,topic):
                if type(topic)==types.ListType:
                        headers=[('do_method','batch')]
                        for t in topic:
                                req=[('do_method','route'),
                                     ('kn_from',t),
                                     ('kn_to','/bogus'),
                                     ('kn_expires','+0')]
                                headers.append(('kn_batch',urllib.urlencode(req)))
                        return self.sendRequest(self.kn_url_base,headers)
                else:
                        return self.sendRequest(self.kn_url_base,
                                                {'do_method':'route',
                                                 'kn_from':topic})

        def checkSubscription(self,t):
                #The demultiplexing should actually be done from
                #kn_route_location or kn_uri parameter (can set kn_uri when
                #creating route)
                #
                #Also, semantics of subscription/routes are "wrong" here
                #Setting up 2 routes means you get 2x the events
                if t.has_key('kn_routed_from'):
                        topic=t['kn_routed_from']
                        if topic.startswith('http://'):
                                i=topic.index('/',9)
                                if i>0:
                                        topic=topic[i+3:]
                        cb=self.topics.get(topic)
                        if cb is None:
                                return
                        cb=cb['cb']
                        if cb is None:
                                return
                        cb(knEvent(e=t))
                        return
                        for topic,cb in self.topics.items():
                                if re.match('.*?'+topic+'$',t['kn_routed_from']):
                                        if cb['cb'] != None:
                                                print len(self.topics)
                                                cb['cb'](knEvent(e=t))  
        
        def run(self):
                while not self.die:
                        try:
                                if self.traceOnly:
                                        self._returnNow=1
                                        time.sleep(10)
                                        continue
        
                                #aa1.3: simple message format hack
                                #aa It is possible to lose events while the connection is re-established
                                #aa what is the right interlock to minimize this, I wonder?
                                #aa Perhaps no publishes should be allowed during those times?
                                h=urllib.urlopen(self.kn_url_base,
                                                 encodeHeaders([('kn_response_format','simple'),
                                                                ('do_method','route'),
                                                                ('kn_from',self.kn_journal)]))
                                h.fp.set_timeout(None)
                                self._returnNow=1

                                #I wonder if this never returns, if the server never sends anything
                                #If so, then even if self.die, then we won't come back
                                #FIXME
                                s=h.readline()

                                try:
                                        if self.run_journal_callback:
                                                self.run_journal_callback(self)
                                except Exception,e:
                                        print e
                                        print 'Exception in run_journal_callback!'

                                t=''
                                while s and not self.die:
                                        if re.match('          [ ]*?[0-9]+$',s):
                                                t=string.strip(s)
                                                t=int(t)
                                                #the response (apart from the first line) is in a format that resembles mimetools format
                                                # we can use the Python mimetools library to parse the headers.
                                                s=h.read(t)
                                                m=self.parseHeaders(headers=s,strip_first=0)
                                                t={}
                                                for k,v in m.items():
                                                        t[k]=self.unQuote(v)
                                                #Since the kn_payload is sent as the body of the response we need to get 
                                                #that in a different way. 
                                                #Luckily the mimetools message contains a copy of the file pointer passed to it, 
                                                # it's current file position is at the body content, yippee!
                                                m.rewindbody()
                                                t['kn_payload']=self.unQuote(m.fp.read())
                                                read,write,error=select.select([h],[],[],0)
                                                if len(read):
                                                        t=self.recvArray.push(t)
                                                while not t is None:
                                                        self.checkSubscription(t)
                                                        t=self.recvArray.pull()
                                        s=h.readline()
                                h.close()
                        except Exception,e:
                                try:
                                        if self.run_exception_callback!=None:
                                                self.run_exception_callback(self,e)
                                except:
                                        print 'Exception in run_exception_callback!'
                                        traceback.print_exc()
                                        print '---'
                                        traceback.print_stack()

                        if not self.die:
                                time.sleep(2)
                self._returnNow=1


        def default_run_journal_callback(self,knr):
                pass

        def default_run_exception_callback(self,knr,e):
                print 'Exception %s raised in run()'%e
                traceback.print_exc()
                print '---'
                traceback.print_stack()
                print '===================================================================='

        def stop(self):
                self.die=1
                #FIXME Once we get readline problem above fixed
                #then isStarted should be set to 0 after run returns
                if self.isStarted:
                        self.lock.acquire()
                        if self.outboundThread:
                                self.outboundThread.cv.acquire()
                                self.outboundThread.cv.notifyAll()
                                self.outboundThread.cv.release()
                        self.isStarted=0
                        self.lock.release()
                
# End of libkn.py
