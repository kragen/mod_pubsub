#!/usr/bin/python

# $Id: sensor_nasdaq2.py,v 1.1 2003/03/15 04:52:49 ifindkarma Exp $

"""
This sensor scrapes pricing data off of the NASDAQ website.

It watches the topic space /what/nasdaq for subtopics of the form
/what/nasdaq/<ticker>

It publishes events to /what/nasdaq/<ticker> of the following
form--not all attributes are represented here:
{
  'symbol':'IBM',
  'fullname':'INTERNATIONAL BUSINESS MA',
  'date':'08/29/2001'
  'time':'16:00:00'
  'bid':'0'
  'ask':'0'
  'unknown1':''
  'prev_close':'104.95'
  'high':'105.90'
  'low':'103.82'
  'last':'104.13'
  'unknown_time1':'16:00:00'
  'change':'-0.82'
  'volume':'4840300'
  'unknown2':''
  'sensor_time':'1234123'
  'kn_payload'='104.13'
}

Also, for compatibility/convenience in writing subscribers, if the
sensor notices sub-sub-topics such as /what/nasdaq/<symbol>/last,
/what/nasdaq/<symbol>/bid, etc. then it breaks up the event into
components and publishes separate events into each subtopic.  Even if
one sub-subtopic is noticed, it publishes to all possible ones (any of
/what/nasdaq/<symbol>/<quoteElements> ... see quoteElements in the
code below).  I encourage you to use the aggregate events/topics instead.

It works by making requests to:
http://quotes.nasdaq.com/quote.dll?page=custom&mode=stock&symbol=ibm&symbol=msft

The nasdaq website returns a line for each ticker in the URL query string.  Here is an example:
IBM|INTERNATIONAL BUSINESS MA|08/29/2001|16:00:00|0|0||104.95|105.90|103.82|104.13|16:00:00|-0.82|4840300||
"""


import KNCONFIG
import copy,time

from pprint import pprint
from sensor import GenericParser,WebScrapeSensor

NASDAQ_MAXREQUESTS=10
NASDAQ_URL='http://quotes.nasdaq.com/quote.dll?page=custom&mode=stock'

#These are the elements of that line, in order:
quoteElements=['symbol','fullname','date','time','bid','ask','unknown1','prev_close',
               'high','low','last','unknown_time1','change','volume','unknown2']

#These are the elements we will compare to decide if it's worth sending an event:
quoteElementsToCompare=['symbol','fullname','bid','ask','unknown1','prev_close',
                        'high','low','last','change','volume','unknown2']

#Just for convenience...
lenQuoteElements=len(quoteElements)


#
#
class NasdaqParser(GenericParser):
    """
    This class is passed to the sensor framework as the parser of
    the data returned by the server.
    """

    #Called between each invocation
    #Reset the data structures
    def reset(self):
        self._outputData={}
        pass

    #Called when new data is available
    #We assume here that we're getting full lines of data, even though we might not
    def feed(self,urldata):
        lines=urldata.split('\n')
        for s in lines:
            dat=s.split('|')
            if len(dat)<lenQuoteElements:
                break

            dat=[s.split('*')[0] for s in dat]
            ev={}
            for i in range(min(len(dat),lenQuoteElements)):
                ev[quoteElements[i]]=dat[i]
            
            self._outputData[dat[0].lower()]=[ev]

    #Called by the framework to get the results of the parse
    def outputData(self):
        return self._outputData

class Nasdaq2Sensor(WebScrapeSensor):
    """
    This is the actual sensor.
    """
    
    #===========================================================================
    # Configuration of the WebScrapeSensor
    #===========================================================================
    rootTopic='/what/nasdaq'		#Our topic space starts here
    subtopicBased=1			#Yes, please watch for new subtopics for me
    maxPreviousPages=1			#Not really used by this sensor, set to safe value
    knPayloadHeader='last'		#Duplicate the 'last' attribute as 'kn_payload' for me
    expireString='+%d'%(60*60*3)	#3 hour expiration for the events we generate
    htmlTimeout=60			#Timeout when requesting data
    initialSubtopics=None		#Don't pre-populate the subtopic space
    maintainSingletonEvent=0		#Don't maintain a 'singleton' event (See sensor.py)

    parser = NasdaqParser()		#The parser instance to use

    lastEventCache={}			#A cache of events for difference comparison
    needsSubSubTopics={}		#Do we need to split events up into sub-sub-topics?


    #===========================================================================
    # Overrides
    #===========================================================================

    #Poll this often
    def loopDelay(self,time):
        return 10

    #Purge events this often
    def purgeDelay(self,time):
        return 300

    #Callback for PubSub Server re-connection:  Re subscribe for sub-subtopics /what/nasdaq/<symbol>/...
    def performSubscriptions(self,knr):
        for subtopic in self.subtopicEventDict.keys():
            self._performSubSubtopicSubscription(subtopic)
        pass

    #For each sub-subtopic watch kn_subtopic, which sends events enumerating
    #the topic space underneath
    def _performSubSubtopicSubscription(self,subtopic):
        while self.knr.subscribe(kn_from='/'.join((self.rootTopic,
                                                   subtopic,
                                                   'kn_subtopics')),
                                 cb=self.subSubtopicCallback,
                                 options={'do_max_age':'infinity'}) is None:
            pass
        

    #So, if we get a callback here and it's not for something
    #the begins with "kn_" then we remember that we have to split
    #up that symbol's events into parts
    def subSubtopicCallback(self,ev):
        #Ignore internal topic names
        if ev.kn_payload[:3]=='kn_':
            return

        #As per our subscription, any events delivered
        #to this callback, they will have been routed through:
        #http://aid.knownow.com/kn/what/nasdaq/<symbol>/kn_subtopics
        #We extract <symbol>
        symbol = ev.kn_routed_from
        symbol = symbol[symbol.find(self.rootTopic)+len(self.rootTopic)+1:
                            symbol.rfind('/')].lower().strip()

        if self.needsSubSubTopics.has_key(symbol):
            return
        
        self.needsSubSubTopics[symbol]=1


    #The sensor framework lets us know each time there's a new subtopic of rootTopic
    #Remember, each subtopic is just a symbol (e.g., for /what/nasdaq/ibm
    #this callback has subtopic='ibm')
    def registerSubtopic(self,subtopic):
        #First, we make sure to register for events that tell us about
        #the sub-sub-topic space (see above)
        #
        #We are batch oriented, so we populate the framework's
        #subtopicBatchList
        #
        #Each element of the batch list can be an arbitrary object
        #We use a list of the form [count,[<symbol>,<symbol>,...],string]
        #count =  of symbols in this batch
        #[<symbol>,...] = a list of the symbols in the batch (not used anywhere else)
        #string = a string we can append to NASDAQ_URL to make our request

        self._performSubSubtopicSubscription(subtopic)

        if self.subtopicBatchList:
            batchList=self.subtopicBatchList[-1]
        else:
            self.subtopicBatchList=[]
            batchList=None

        if not batchList or batchList[0]>=NASDAQ_MAXREQUESTS:
            batchList=[0,[],'']
            self.subtopicBatchList.append(batchList)

        batchList[0]+=1;
        batchList[1].append(subtopic)
        batchList[2]+='&symbol='+subtopic
        pass

    #Called by the framework: Returns the url for a given batch
    def urlForBatch(self,batch):
        return NASDAQ_URL+batch[2]
        pass

    #Called by the framework to 'fix up' the events
    #returned by the parser.  We take the opportunity to
    #delete events when the data for a symbol is unchanged
    #Since we do our own uniquing, we prevent the framework
    #from doing it by returning a None key
    def fixEventAndGetKey(self,ev,subtopic,i):
        try:
            lastEv=self.lastEventCache[ev['kn_to']]
        except:
            lastEv={}

        sameAsOld=1
        for el in quoteElementsToCompare:
            if not lastEv.get(el)==ev[el]:
                sameAsOld=0
                lastEv[el]=ev[el]

        if sameAsOld:
            return (None,0)
        
        self.lastEventCache[ev['kn_to']]=copy.copy(ev)
        
        t=time.time()
        ev['sensor_time']=t
        ev['kn_id']=str(t)
        return (None,1)

    #Called by the framework to 'fix up' the final event
    #list just before publishing.  We populate the
    #list with sub-sub-topic events as appropriate
    #(Yes, we're not smart about not sending a 'last'
    # sub-sub-topic event if 'last' wasn't the piece
    # of the event that changed, etc.)
    def fixupToSendList(self,eventList,subtopic=None):
        if not self.needsSubSubTopics.has_key(subtopic):
            return

        additionalEvents=[]
        for ev in eventList:
            #Create a new event where kn_payload has the
            #element, for each quoteElement
            for attr in quoteElements:
                oe={}
                oe['symbol']=ev['symbol']
                oe['date']=ev['date']
                oe['time']=ev['time']
                oe['sensor_time']=ev['sensor_time']
                oe['kn_payload']=ev[attr]
                oe['kn_to']='/'.join((self.rootTopic,
                                      subtopic,
                                      attr))
                oe['kn_expires']=ev['kn_expires']
                additionalEvents.append(oe)

                #Also generate sub-sub-topic events on
                #a couple of additional topics fo
                #compatibility with older nasdaq sensor naming
                #convention
                if attr=='prev_close':
                    oe['kn_to']='/'.join((self.rootTopic,
                                          subtopic,
                                          'close'))
                    additionalEvents.append(oe)
                if attr=='volume':
                    oe['kn_to']='/'.join((self.rootTopic,
                                          subtopic,
                                          'vol'))
                    additionalEvents.append(oe)

        eventList.extend(additionalEvents)
                    

if __name__ == '__main__':
    Nasdaq2Sensor().run()


# End of sensor_nasdaq2.py
