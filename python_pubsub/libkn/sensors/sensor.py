#!/usr/bin/python

# $Id: sensor.py,v 1.2 2003/04/29 06:44:17 ifindkarma Exp $

import KNCONFIG
import threading, time, urllib, sys, traceback, inspect, pprint, types, os, copy
from sgmllib import SGMLParser

# Include local libraries:
sys.path = [ "../" ] + sys.path

from libkn import kn

gknr=kn(host=KNCONFIG.PUBSUBSERVER_HOSTNAME,port=KNCONFIG.PUBSUBSERVER_PORT,traceOnly=KNCONFIG.PUBSUBSERVER_TRACEONLY)
import os

#There is still way too much reliance on side effects &c in all this code
#has to be cleaned up!!!

class GenericParser:
    def reset(self):
        pass

    def feed(self,s):
        pass

    def close(self):
        pass

    def outputData(self):
        return None

    def previousUrl(self):
        return None


class HTMLParser(SGMLParser,GenericParser):
    entitydefs = \
               {'lt': '<',
                'gt': '>',
                'amp': '&',
                'quot': '"',
                'apos': '\'',
                'nbsp': ' '}

    def reset(self):
        SGMLParser.reset(self)
        self._outputData=[]
        self._previousUrl=None
        return

    def outputData(self):
        return self._outputData

    def previousUrl(self):
        return self._previousUrl

    def handle_comment(self,text):
        return

#There should probably have been an abstract superclass & two subclasses to handle
#the "parser based" and "general case" sensors, but
class WebScrapeSensor:
    #===========================================================================
    # Variables you should override
    #   Leaving these undefined so it'll raise if you forget to set any
    #===========================================================================
    _debugFileName=None
    #rootTopic='/what/error'
    #subtopicBased=1
    #maxPreviousPages=2
    #knPayloadHeader='kn_payload'
    #expireString='infinity'
    #htmlTimeout=60
    #initialSubtopics=()
    #
    #parser = HTMLParser()
    maintainSingletonEvent=0
    foobar=1
    

    #===========================================================================
    # Methods NOT likely to be overriden
    #===========================================================================
    def __init__(self):
        """Instance initializer
        rootTopic is the topic that will be watched for new subtopics

        PROBABLY NO NEED TO OVERRIDE THIS
        """
        
        self.subtopicEventDict={} #Dictionary of dictionary of events
        self.subtopicSingletonDict={} #A dictionary of single events
        self.subtopicCount=0
        self.subtopicBatchList=None
        #self.lock=threading.RLock()

    def run(self):
        """Call this method to start the sensor
        """
        try:
            self._run()
            print 'Sensor %s will die (because _run() returned, client library is no longer connected?).'
        except Exception,e:
            print 'Sensor %s will die (because of an exception).'%self.__class__.__name__
            traceback.print_exc()
            print '---'
            traceback.print_stack()
            raise

    def _performSubtopicSubscription(self,knr):
        while self.subtopicBased:
            m=self.knr.subscribe(kn_from=''.join((self.rootTopic,
                                                  '/kn_subtopics')),
                                 cb=self._subtopicCallback,
                                 options={'do_max_age':'infinity'})
            if m is None:
                print ('%s:%s')%(self.__class__.__name__,'Retrying subscription to subtopics')
            else:
                print ('%s:%s')%(self.__class__.__name__,'Subtopic subscription successful')
                break
        self.performSubscriptions(knr)
        
    def _run(self):
        global gknr
        #This is all screwed up because we actually want different handlers
        #for different sensors
        self.knr=gknr
        self.knr.start(onJournal=self._performSubtopicSubscription)
        time.sleep(2)

        if self.initialSubtopics!=None:
            for x in self.initialSubtopics:
                self._subtopicCallback(ev=None,subtopic=x)

        #This should be enhanced so that new subtopic registrations
        #immediately fire off publishing of the new data
        #..................................................
        lastPurgeTime=time.time()
        while self.knr.connected():
            lastTime=time.time()
            #self.lock.acquire()
            if self.subtopicBatchList!=None:
                for b in self.subtopicBatchList:
                    #self.lock.acquire()
                    self._publishForBatch(b)
                    #self.lock.release()
            elif self.subtopicBased:
                for k in self.subtopicEventDict.keys():
                    #self.lock.acquire()
                    self._publishForSubtopic(k)
                    #self.lock.release()
            else:
                #self.lock.acquire()
                self._publishForSubtopic(None)
                #self.lock.release()
            #self.lock.release()

            lt=time.localtime()
            x=lt[3]*100+lt[4]
            sweeptime=self.loopDelay(x)

            if time.time()-lastPurgeTime>self.purgeDelay(x):
                self._purgeSubtopicEventDict()
                lastPurgeTime=lastTime

            if time.time()-lastTime<sweeptime:
                time.sleep(max(sweeptime-time.time()+lastTime,1))
            else:
                print ('%s:%s')%(self.__class__.__name__,'Sweeptime not long enough')


    def _publishForSubtopic(self,subtopic=None):
        """Actually attempts to publish information associated with the specified subtopic.
        Gets a list of raw events extracted from the page.  Allows the subclass to fix each of those
        and also adds a bunch of other useful headers.  Then, for each unique event adds it to a list
        in preparation to send the events.  Fixes up the list, and then sends the events in
        batches.
        """

        previousUrl=self.firstUrlForSubtopic(subtopic)
        if subtopic==None:
            kn_to=self.rootTopic
        else:
            kn_to=''.join((self.rootTopic,'/',subtopic))
        toSend=[]

        if subtopic==None:
            sub='XX'
        else:
            sub=subtopic

        try:
            eventDict=self.subtopicEventDict[sub]
        except:
            eventDict={}
            self.subtopicEventDict[sub]=eventDict

        for x in range(self.maxPreviousPages):
            if previousUrl is None:
                break
            
            dataDict,previousUrl = self.beforeGetDataFromUrl(previousUrl,subtopic)
            if dataDict is None:
                data,previousUrl = self._getDataFromUrl(previousUrl)
            data,previousUrl = self.afterGetDataFromUrl(data,previousUrl,subtopic)
            
            if data==None:
                break

            numDuplicate=0
            num=len(data)
            for i in range(num):
                v=data[i]
                v['kn_expires']=self.expireString
                v['kn_to']=kn_to
                (k,sendit)=self.fixEventAndGetKey(v,subtopic,i)
                if not self.knPayloadHeader=='kn_payload':
                    temp=v.get(self.knPayloadHeader)
                    if temp:
                        v['kn_payload']=temp
                        
                if k!=None:
                    if not eventDict.has_key(k):
                        eventDict[k]=v
                        if sendit:
                            toSend.append(v)
                    else:
                        #print Already found event with key %s'%k
                        numDuplicate += 1
                        if numDuplicate>1:
                            previousUrl = None
                            break
                elif sendit:
                    toSend.append(v)

        self.fixupToSendList(toSend,subtopic)

        if self.maintainSingletonEvent:
            try:
                ev=self.subtopicSingletonDict[sub]
            except:
                ev=None
            ev=self.updateSingletonForSubtopic(toSend,ev,subtopic)
            if ev!=None:
                ev['kn_expires']='infinity'
                ev['kn_id']='singleton'
                ev['kn_to']=kn_to
                self.subtopicSingletonDict[sub]=ev
                self.knr.publish(d=ev)
        
        print '%s: Sending %d new events for subtopic %s'%(self.__class__.__name__,len(toSend),subtopic)
        x=0
        while x<len(toSend):
            self.knr.publish(d=toSend[x:x+KNCONFIG.PUBSUBSERVER_MAXBATCHSIZE])
            x+=KNCONFIG.PUBSUBSERVER_MAXBATCHSIZE

        self.afterPublishForSubtopic(subtopic)


    def _getDataFromUrl(self,url):
        """
        """

        #print 'Getting data from URL %s'%url

        if self._debugFileName:
            try:
                h=open(self._debugFileName)
            except:
                h=None
        else:
            h=None
            
        s=None
        retVal=None
        previousUrl=None
        
        try:
            if h is None:
                h=urllib.urlopen(url)
                h.fp.set_timeout(self.htmlTimeout)
            s=h.read()
        except Exception,e:
            print '%s: Exception %s raised in _getDataFromUrl'%(self.__class__.__name__,e)
            traceback.print_exc()
        else:
            try:
                self.parser.reset()
                self.parser.feed(s)
                retVal=self.parser.outputData()
                previousUrl=self.parser.previousUrl()
                self.parser.close()
                h.close()
            except TypeError,e:
                raise
            except SyntaxError,e:
                raise
            except KeyboardInterrupt,e:
                raise
            except Exception,e:
                print '%s: Exception %s raised in _getDataFromUrl'%(self.__class__.__name__,e)
                self._writeDiagnosticFile(s,e)
                traceback.print_exc()

        return retVal,previousUrl
            


    def _subtopicCallback(self,ev,subtopic=None):
        """kn_subtopics callback
        """

        if subtopic==None:
            subtopic = ev.kn_payload.lower().strip()

        #might want to add code which routes data from
        #canonical subtopic names to non-canonical
        #subtopic names.  E.g., if a client subscribes
        #to a topic such as IbM vs ibm
        #..................................................
        #if subtopic!=ev.kn_payload:
        #    self.knr.subscribe(kn_from=<canonical>
        #                  kn_to=<url>+ev.kn_payload)

        if subtopic[:3]=='kn_':
            return

        #self.lock.acquire()
        if self.subtopicEventDict.has_key(subtopic):
            #self.lock.release()
            return

        self.subtopicEventDict[subtopic]={}
        self.subtopicCount += 1

        self.registerSubtopic(subtopic)

        print '%s: Registered new subtopic: %s'%(self.__class__.__name__,subtopic)
        #self.lock.release()

    def _purgeSubtopicEventDict(self):
        for x in self.subtopicEventDict.keys():
            dict = self.subtopicEventDict[x]
            for y in dict.keys():
                self.possiblyPurgeEntry(x,y,dict)


    def _writeDiagnosticFile(self,s,exc=None):
        if exc:
            #Save exception info
            etype,value,tb = sys.exc_info()
            
        x=0
        fd=-1
        while fd<0 and x<1024:
            try:
                if x==1023:
                    filename="%s_final"%self.__class__.__name__
                    fd=os.open(filename,os.O_WRONLY|os.O_CREAT|os.O_TRUNC,0644)
                else:
                    filename="%s_%d"%(self.__class__.__name__,x)
                    fd=os.open(filename,os.O_WRONLY|os.O_CREAT|os.O_EXCL,0644)
            except OSError,e:
                if e.errno!=17:
                    break
            except:
                break
            x=x+1

        if fd<0:
            print '%s: Could not write diagnostic file'%self.__class__.__name__
            return

        print '%s: Writing diagnostic file %s'%(self.__class__.__name__,filename)
        f=os.fdopen(fd,"w")
        if exc:
            traceback.print_exception(etype,value,tb,file=f)

        f.write('============================================Stack:\n')
        traceback.print_stack(file=f)
        if not s is None:
            f.write('============================================Data:\n')
            f.write(s)
        f.close()


    def _publishForBatch(self,batch):
        """Presumably, the subclass has entered a set of batch information into
        self.subtopicBatchList.  Here, the subclass is expected to get and then
        publish the data for each such batch list entry

        Actually attempts to publish information associated
        with the specified subtopic.  Gets a list of raw events
        extracted from the page.  Allows the subclass to fix each of
        those and also adds a bunch of other useful headers.  Then,
        for each unique event adds it to a list in preparation to send
        the events.  Fixes up the list, and then sends the events in
        batches.  """

        previousUrl=self.urlForBatch(batch)
        toSend=[]

        for x in range(self.maxPreviousPages):
            if previousUrl is None:
                break

            dataDict,previousUrl = self.beforeGetDataFromUrl(previousUrl,batch)
            if dataDict is None:
                dataDict,previousUrl = self._getDataFromUrl(previousUrl)
            dataDict,previousUrl = self.afterGetDataFromUrl(dataDict,previousUrl,batch)
            if dataDict==None:
                break
            
            numDuplicate=0
            for subtopic in dataDict.keys():
                data=dataDict[subtopic]
                num=len(data)
                toSendThisSubtopic=[]

                kn_to=''.join((self.rootTopic,'/',subtopic))

                try:
                    eventDict=self.subtopicEventDict[subtopic]
                except:
                    eventDict={}
                    self.subtopicEventDict[subtopic]=eventDict

                for i in range(num):
                    v=data[i]
                    v['kn_expires']=self.expireString
                    v['kn_to']=kn_to
                    (k,sendit)=self.fixEventAndGetKey(v,subtopic,i)
                    if not self.knPayloadHeader=='kn_payload':
                        temp=v.get(self.knPayloadHeader)
                        if temp:
                            v['kn_payload']=temp

                    if k!=None:
                        if not eventDict.has_key(k):
                            eventDict[k]=v
                            if sendit:
                                toSendThisSubtopic.append(v)
                        else:
                            #print Already found event with key %s'%k
                            numDuplicate += 1
                            if numDuplicate>1:
                                previousUrl = None
                                break
                    elif sendit:
                        toSendThisSubtopic.append(v)

                self.fixupToSendList(toSendThisSubtopic,subtopic)
                toSend.extend(toSendThisSubtopic)
                
                if self.maintainSingletonEvent:
                    try:
                        ev=self.subtopicSingletonDict[sub]
                    except:
                        ev=None
                    ev=self.updateSingletonForSubtopic(toSendThisSubtopic,ev,subtopic)
                    if ev!=None:
                        ev['kn_expires']='infinity'
                        ev['kn_id']='singleton'
                        ev['kn_to']=kn_to
                        self.subtopicSingletonDict[sub]=ev
                        self.knr.publish(d=ev)

        self.fixupToSendList(toSend)

        print '%s: Sending %d new events for batch %s'%(self.__class__.__name__,len(toSend),batch)
        x=0
        while x<len(toSend):
            self.knr.publish(d=toSend[x:x+KNCONFIG.PUBSUBSERVER_MAXBATCHSIZE])
            x+=KNCONFIG.PUBSUBSERVER_MAXBATCHSIZE

        self.afterPublishForBatch(batch)

    #===========================================================================
    # Methods you _may_ want to override
    #===========================================================================
    def performSubscriptions(self,knr):
        """Called every time it's necessary to re-register any
        subscriptions you may have with the client library"""
        pass
        
    def registerSubtopic(self,subtopic):
        pass

    def possiblyPurgeEntry(self,subtopic,key,subtopicEventDict):
        """You may want to override this if you want to purge events from the
        subtopic dictionary every now and then"""
        pass

    def beforeGetDataFromUrl(self,previousUrl,subtopicOrBatch):
        return None,previousUrl

    def afterGetDataFromUrl(self,data,previousUrl,subtopicOrBatch):
        return data,previousUrl

    def afterPublishForBatch(self,batch):
        pass

    def afterPublishForSubtopic(self,subtopic=None):
        pass

    def updateSingletonForSubtopic(self,toSend,last,subtopic):
        try:
            last=toSend[-1]
        except:
            pass
        return last
    
    #===========================================================================
    # Methods likely to be overriden
    #===========================================================================

    def loopDelay(self,time):
        """Provides the number of seconds to wait before attempting
        to extract new stuff.

        time = integer representing current 24hr time (e.g., 900 for 9AM, 1330 for 1:30pm)
        """
        
        return 60

    def purgeDelay(self,time):
        """Provides the number of seconds to wait before attempting
        to purge the subtopic dictionary

        time = integer representing current 24hr time (e.g., 900 for 9AM, 1330 for 1:30pm)
        """
        
        return 300

    def firstUrlForSubtopic(self,subtopic):
        """Provides the URL for the first page of data for a given subtopic
        """
        
        raise "No firstUrlForSubtopic implementation!"

    def urlForBatch(self,batch):
        """Provides the URL for the first page for a given batch
        """
        
        raise "No urlForBatch implementation!"

    def fixEventAndGetKey(self,ev,subtopic,index):
        """Add/remove/fix any headers you want in the event here, and also return
        the key for this id in this sensor's dictionary.  The kn_id should probably
        be set in this method, and may be different from the id returned.  The returned id
        is used for keying into a local dictionary and can therefore be a tuple.  It is used
        to recognize duplicate events that don't need to be re-sent
        """
        
        return ev['kn_id']

    def fixupToSendList(self,list,subtopic=None):
        """The events in list are about to be sent.  They were added on to the list
        in the order that they came out of the HTML parser.  Often, you'll want to reverse
        this list before sending it out.  E.g., because news stories are listed in reverse
        time order on the web page, but you want the events to go out oldest first
        """

        return


class SensorContainer:
    modules=[]
    def __init__(self):
        try:
            dirlist=os.environ['SENSORPATH']
        except:
            dirlist='.'

        if not dirlist:
            dirlist='.'
            
        dirlist=dirlist.split(os.pathsep)

        oldpath=sys.path
        newpath=copy.copy(oldpath)
        newpath[1:1]=['']
        for d in dirlist:
            newpath[1]=d
            sys.path=newpath
            for filename in os.listdir(d):
                if (filename[-3:]=='.py' and
                    filename[:6]=='sensor' and
                    not filename=='sensor.py'):
                    basename = filename[:len(filename)-3]
                    self.modules.append(__import__(basename))
        sys.path=oldpath

    #This code should be cleaned up using __bases__, which I just found out about
    def start(self):
        allclasses=[WebScrapeSensor]
        for m in self.modules:
            print 'Module: %s'%m
            classes=inspect.getmembers(m,inspect.isclass)
            for c in classes:
                allclasses.append(c[1])

        parentclass=sys.modules['sensor'].WebScrapeSensor
        classList = [c for c in allclasses if parentclass in c.__bases__]

        classesAndThreads={}
        classNames=[]
        index=0
        for c in classList:
            name=c.__name__
            print 'Starting thread for: %s'%name
            t=threading.Thread(target=c().run)
            t.setName("Thread_%d_%s"%(index,name))
            classesAndThreads[name]=(c,t)
            index=index+1

            classNames.append(name)
            t.start()


        print 'Watchdog loop started'
        while 1:
            time.sleep(10)
            for name in classNames:
                c,t=classesAndThreads[name]
                if not t.isAlive():
                    print "Thread %s reported dead.  Restarting."%t.getName()
                    t=threading.Thread(target=c().run)
                    t.setName("Thread_%d_%s"%(index,name))
                    classesAndThreads[name]=(c,t)
                    index=index+1

                    t.start()

                    
if __name__ == '__main__':
    SensorContainer().start()


# End of sensor.py
