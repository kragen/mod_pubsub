#!/usr/bin/python

# $Id: sensor_inlumen.py,v 1.2 2003/04/29 06:44:18 ifindkarma Exp $

"""
This sensor scrapes pricing data off of the INLUMEN website.

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
  ...
}

Also, for compatibility/convenience in writing subscribers, if the
sensor notices sub-sub-topics such as /what/nasdaq/<symbol>/last,
/what/nasdaq/<symbol>/bid, etc. then it breaks up the event into
components and publishes separate events into each subtopic.  Even if
one sub-subtopic is noticed, it publishes to all possible ones (any of
/what/nasdaq/<symbol>/<quoteElements> ... see quoteElements in the
code below).  I encourage you to use the aggregate events/topics instead.

It works by making requests to:
http://product.inlumen.com/bin/quoteserver?ticker=<tickers>

with <tickers> replaced with a + separated list of tickers, batched 10 at a time.

The result are data sets that look like the one below, concatenated together in the same HTTP response.

[IBM]
AskPrice=N/A
AskSize=0
BidPrice=N/A
BidSize=0
ClosePrice=119.90
CloseTick=
CompanyName=International Business Machines Corp.
Country=
Dividend=0.14
DividendFrequency=
EarningsPerShare=4.50
ExDividendDate=11/7/01
HighPrice=114.90
ItemExchangeCode=NYSE
LastSalePrice=114.25
LastSaleSize=452900
LastSaleTime=1/18
LowPrice=112.81
NetChangePercent=-4.7%
NetChangePrice=-5.65
OpenPrice=114.25
OpenTick=
PERatio=25.39
SharesOutstanding=1722642
Symbol=IBM
TimeDate=1011387720
Volatility=1.23
Volume=18440400
YearHigh=126.39
YearLow=87.49
Yield=0.40
"""


import KNCONFIG
import copy, time, types, string, random

from pprint import pprint
from sensor import GenericParser,WebScrapeSensor

INLUMEN_MAXREQUESTS=10
INLUMEN_URL='http://product.inlumen.com/bin/quoteserver?ticker='

startingData="""
[%s]
AskPrice=119.95
AskSize=50
BidPrice=119.80
BidSize=100
ClosePrice=119.90
CloseTick=
CompanyName=Unknown
Country=
Dividend=0.14
DividendFrequency=
EarningsPerShare=4.50
ExDividendDate=11/7/01
HighPrice=114.90
ItemExchangeCode=NYSE
LastSalePrice=114.25
LastSaleSize=452900
LastSaleTime=1/18
LowPrice=112.81
NetChangePercent=-4.7
NetChangePrice=-5.65
OpenPrice=114.25
OpenTick=
PERatio=25.39
SharesOutstanding=1722642
Symbol=%s
TimeDate=1011387720
Volatility=1.23
Volume=18440400
YearHigh=126.39
YearLow=87.49
Yield=0.40
Simulated=1
"""

quoteElements={
    'AskPrice':                 'ask',
    'AskSize':                  'asksz',
    'BidPrice':                 'bid',
    'BidSize':                  'bidsz',
    'ClosePrice':               ['prev_close','close'],
    'CloseTick':                'closetk',
    'CompanyName':              'fullname',
    'Country':                  'country',
    'Dividend':                 'div',
    'DividendFrequency':        'divfreq',
    'EarningsPerShare':         'eps',
    'ExDividendDate':           'exdivdt',
    'HighPrice':                'high',
    'ItemExchangeCode':         'exchcode',
    'LastSalePrice':            'last',
    'LastSaleSize':             'lastsz',
    'LastSaleTime':             'lasttm',
    'LowPrice':                 'low',
    'NetChangePercent':         'changepct',
    'NetChangePrice':           'change',
    'OpenPrice':                'open',
    'OpenTick':                 'opentk',
    'PERatio':                  'pe',
    'SharesOutstanding':        'sharesout',
    'Symbol':                   'symbol',
    'TimeDate':                 'timedate',
    'Volatility':               'volatility',
    'Volume':                   ['vol','volume'],
    'YearHigh':                 'high52',
    'YearLow':                  'low52',
    'Yield':                    'yield'};

#These are the elements we will compare to decide if it's worth sending an event:
quoteElementsToCompare=['symbol','fullname','bid','ask','close',
                        'high','low','last','change','volume']

subsubTopicElementList=['last','bid','ask','vol','close','change']

quoteElementsToSimulate=[('f','bid',119.0),
                         ('f','ask',120.0),
                         ('f','last',119.50),
                         ('u',['vol','volume'],150234),
                         ('d','asksz',100),
                         ('d','bidsz',100),
                         ('f','change',10),
                         ('f','changepct',2)]

#Just for convenience...
lenQuoteElements=len(quoteElements)


#
#
class InlumenParser(GenericParser):
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
        lines=[l.strip() for l in lines]
        for l in lines:
            if not l:
                continue
            
            if l[0]=='[':
                ticker=l[1:-1].lower()
                ev={}
                self._outputData[ticker]=[ev]
                continue
            else:
                dat=l.split('=')
                dat=[d.strip() for d in dat]
                if len(dat)<2:
                    continue

                if dat[1]=='N/A':
                    dat[1]='0'
                if len(dat[1]) and dat[1][-1]=='%':
                    dat[1]=dat[1][:-1]

                trans=quoteElements.get(dat[0])
                if trans is None:
                    ev[dat[0]]=dat[1]
                else:
                    if type(trans) is types.ListType:
                        for k in trans:
                            ev[k]=dat[1]
                    else:
                        ev[trans]=dat[1]

    #Called by the framework to get the results of the parse
    def outputData(self):
        return self._outputData

class InlumenSensor(WebScrapeSensor):
    """
    This is the actual sensor.
    """
    
    #===========================================================================
    # Configuration of the WebScrapeSensor
    #===========================================================================
    rootTopic='/what/nasdaq'            #Our topic space starts here
    subtopicBased=1                     #Yes, please watch for new subtopics for me
    maxPreviousPages=1                  #Not really used by this sensor, set to safe value
    knPayloadHeader='last'              #Duplicate the 'last' attribute as 'kn_payload' for me
    expireString='+%d'%(60*60*3)        #3 hour expiration for the events we generate
    htmlTimeout=60                      #Timeout when requesting data
    initialSubtopics=['ibm','aapl']     #Don't pre-populate the subtopic space
    maintainSingletonEvent=0            #Don't maintain a 'singleton' event (See sensor.py)

    parser = InlumenParser()            #The parser instance to use

    lastEventCache={}                   #A cache of events for difference comparison
    needsSubSubTopics={}                #Do we need to split events up into sub-sub-topics?

    lastNoDataTime=0
    pendingSymbols={}
    lastAttemptTime=0

    #===========================================================================
    # Overrides
    #===========================================================================

    #Poll this often
    def loopDelay(self,t):
        #We need a more detailed concept of time, elsewhere
        return 5

    #Purge events this often
    def purgeDelay(self,t):
        return 100

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
        #string = a string we can append to INLUMEN_URL to make our request

        self._performSubSubtopicSubscription(subtopic)

        if self.subtopicBatchList:
            batchList=self.subtopicBatchList[-1]
        else:
            self.subtopicBatchList=[]
            batchList=None

        if not batchList or batchList[0]>=INLUMEN_MAXREQUESTS:
            batchList=[0,[],'']
            self.subtopicBatchList.append(batchList)

        self.pendingSymbols[subtopic]=1
        batchList[0]+=1;
        batchList[1].append(subtopic)
        batchList[2]+='+'+subtopic
        pass

    #Called by the framework: Returns the url for a given batch
    def urlForBatch(self,batch):
        return INLUMEN_URL+batch[2]
        pass

    #Called by the framework to 'fix up' the events
    #returned by the parser.  We take the opportunity to
    #delete events when the data for a symbol is unchanged
    #Since we do our own uniquing, we prevent the framework
    #from doing it by returning a None key
    def fixEventAndGetKey(self,ev,subtopic,i):
        if ev is None or len(ev)==0:
            return (None,0)

        lastEv=self.lastEventCache.get(ev['kn_to'])
        if not lastEv is None:
            sameAsOld=1
            for el in quoteElementsToCompare:
                if not lastEv.get(el)==ev[el]:
                    sameAsOld=0
                    lastEv[el]=ev[el]

            if sameAsOld and not self.simulate(ev):
                return (None,0)

        ev['symbol']=ev['symbol'].lower()
        if ev.get('Simulated') is None:
            try:
                del self.pendingSymbols[ev['symbol']]
            except:
                pass

        try:
            timedate=string.atoi(ev['timedate'])
            t=time.localtime(timedate)
            ev['time']='%02d:%02d:%02d'%(t[3],t[4],t[5])
            ev['date']='%02d/%02d/%d'%(t[1],t[2],t[0])
        except:
            pass
        
        self.lastEventCache[ev['kn_to']]=copy.copy(ev)
        
        t=time.time()
        ev['sensor_time']=t
        ev['kn_id']=str(t)
        return (None,1)

    def simulate(self,ev):
        t=self.time[3]*100+self.time[4]
        if not (self.time[6]>4 or t<950 or t>1620):
            return 0

        l=len(quoteElementsToSimulate)
        if not l:
            return 0

        l=(l*5)/2
        ret=0
        for k in quoteElementsToSimulate:
            elname=k[1]
            if type(elname) is types.ListType:
                elname=elname[0]

            if random.randrange(l)==0:
                try:
                    value=string.atof(ev[elname])
                except:
                    continue

                if value==0:
                    value=k[2]

                if k[0]=='u':
                    dir=1
                else:
                    dir=random.randrange(2)
                    if dir==0:
                        dir=-1

                value+=value*dir*(0.005*(random.randrange(5)+1))
                
                if k[0]=='f':
                    value='%.2f'%value
                else:
                    value='%d'%value
                        
                if not elname is k[1]:
                    for elname in k[1]:
                        ev[elname]=value
                else:
                    ev[elname]=value
                ret=1

        if ret:
            ev['timedate']='%d'%time.time()
        return ret

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
            for attr in subsubTopicElementList:
                oe={}
                oe['symbol']=ev['symbol']
                oe['time']=ev['time']
                oe['date']=ev['date']
                oe['timedate']=ev['timedate']
                oe['sensor_time']=ev['sensor_time']
                oe['kn_payload']=ev[attr]
                oe['kn_to']='/'.join((self.rootTopic,
                                      subtopic,
                                      attr))
                oe['kn_expires']=ev['kn_expires']
                additionalEvents.append(oe)

        eventList.extend(additionalEvents)


    #If we cannot connect to the server, we will generate a bunch of events
    # for the batch based on a dummy event at the beginning of this file
    #If we cannot connect but we have connected before, then that's ok
    # because the events will be taken from the lastEventCache
    def beforeGetDataFromUrl(self,previousUrl,subtopicOrBatch):

        self.time=time.localtime(time.time())
        
        t=self.time[3]*100+self.time[4]
        if self.time[6]>4 or t<940 or t>1620:
            interval=60
        elif (t>=940 and t<=1000):
            interval=20
        else:
            interval=10

        offhours=(self.time[6]>4 or t<950 or t>1620)
        timeSinceLastNoData=time.time()-self.lastNoDataTime

        if offhours and not len(self.pendingSymbols):
            return self.simulateGetData(subtopicOrBatch),previousUrl
        elif (timeSinceLastNoData<interval):
            print 'Not re-connecting for another %d seconds (No data on last attempt).'%(interval-timeSinceLastNoData)
            return self.simulateGetData(subtopicOrBatch),previousUrl
        else:
            return None,previousUrl
        
    def afterGetDataFromUrl(self,data,previousUrl,subtopicOrBatch):
        if not data is None:
            return data,previousUrl

        self.lastNoDataTime=time.time()
        return self.simulateGetData(subtopicOrBatch),previousUrl

    def simulateGetData(self,batch):
        outputData={}
        for symbol in batch[1]:
            lastEv=self.lastEventCache.get('/'.join((self.rootTopic,symbol)))
            if lastEv is None:
                s=startingData%(symbol,symbol)
                self.parser.reset()
                self.parser.feed(s)
                data=self.parser.outputData()
                lastEv=data[symbol][0]

            outputData[symbol]=[lastEv]

        return outputData
            

if __name__ == '__main__':
    InlumenSensor().run()


# End of sensor_inlumen.py
