#!/usr/bin/python

# $Id: sensor_nasdaq3.py,v 1.1 2003/03/15 04:52:49 ifindkarma Exp $

import KNCONFIG
import copy,time

from pprint import pprint
from sensor import HTMLParser,WebScrapeSensor

NASDAQ_MAXREQUESTS=10

translation={
    'last trade'	:'last',
    'change'		:'change',
    'prev cls'		:'prev_close',
    'volume'		:'volume',
    'div date'		:'divdate',
    "day's range"	:'dayrange',
    'bid'		:'bid',
    'ask'		:'ask',
    'open'		:'open',
    'avg vol'		:'avgvol',
    '52-week range'	:'range52',
    'earn/shr'		:'earnshr',
    'p/e'		:'pe',
    'mkt cap'		:'mktcap',
    'div/shr'		:'divshr',
    'yield'		:'yield'}

#These are the elements we will compare to decide if it's worth sending an event:
quoteElementsToCompare=['bid','ask','high','low','last','change','volume']
quoteElements=['symbol','fullname','bid','ask','prev_close',
               'high','low','last','change','volume']

class Nasdaq3Parser(HTMLParser):
    def reset(self):
        HTMLParser.reset(self)
        self.inBold=0
        self.boldData=[]
        self.itemData=[]

        self.itemName=None
        self.ev={}
        self.row=-1
        self.tables=-1
        self.state='FINDING QUOTE'


    def start_table(self, attrs):
        self.tables=self.tables+1
        self.row=0
        if self.tables==10:
            self.state='FOUND START OF QUOTE'

    def end_table(self):
        if self.state=='FOUND START OF QUOTE':
            self.state='IN QUOTE'
        elif self.state=='IN QUOTE':
            self._outputData=[self.ev]
            self.state='END'
            self.itemName=None
        pass

    def start_td(self, attrs):
        self.itemData=[]
        self.boldData=[]

    def start_br(self, attrs):
        if self.state=='IN QUOTE':
            self.itemName="".join(self.itemData).strip()
            self.itemData=[]
            
    def end_td(self):
        if self.itemName:
            key=translation.get(self.itemName.strip().lower())
            if key:
                self.itemData.extend(self.boldData)
                data="".join(self.itemData).strip()
                if data.find('/')>=0:
                    data="0"
                self.ev[key]=data

    def start_b(self, attrs):
        self.boldData=[]
        self.inBold=1

    def end_b(self):
        self.inBold=0
        if self.state=='FOUND START OF QUOTE':
            text="".join(self.boldData).strip()
            self.ev['fullname']=text

    def handle_data(self,data):
        if self.inBold:
            self.boldData.append(data)
        else:
            self.itemData.append(data)






class Nasdaq3Sensor(WebScrapeSensor):
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
    expireString='+%d'%(60*60*24)	#24 hour expiration for the events we generate
    htmlTimeout=60			#Timeout when requesting data
    initialSubtopics=['ibm']		#Don't pre-populate the subtopic space
    maintainSingletonEvent=0		#Don't maintain a 'singleton' event (See sensor.py)

    parser = Nasdaq3Parser()		#The parser instance to use

    lastEventCache={}			#A cache of events for difference comparison
    needsSubSubTopics={}		#Do we need to split events up into sub-sub-topics?


    #===========================================================================
    # Overrides
    #===========================================================================

    def loopDelay(self,time):
        if time>=930 and time<=1600:
            return 20
        else:
            return 120

    def purgeDelay(self,time):
        return 300

    def performSubscriptions(self,knr):
        for subtopic in self.subtopicEventDict.keys():
            self._performSubSubtopicSubscription(subtopic)
        pass

    def _performSubSubtopicSubscription(self,subtopic):
        while self.knr.subscribe(kn_from='/'.join((self.rootTopic,
                                                   subtopic,
                                                   'kn_subtopics')),
                                 cb=self.subSubtopicCallback,
                                 options={'do_max_age':'infinity'}) is None:
            pass
        

    def subSubtopicCallback(self,ev):
        if ev.kn_payload[:3]=='kn_':
            return

        symbol = ev.kn_routed_from
        symbol = symbol[symbol.find(self.rootTopic)+len(self.rootTopic)+1:
                            symbol.rfind('/')].lower().strip()

        if self.needsSubSubTopics.has_key(symbol):
            return
        
        self.needsSubSubTopics[symbol]=1


    def registerSubtopic(self,subtopic):
        self._performSubSubtopicSubscription(subtopic)

    def firstUrlForSubtopic(self,subtopic):
        return 'http://finance.yahoo.com/q?s=%s&d=t'%(subtopic)

    def fixEventAndGetKey(self,ev,subtopic,i):
        try:
            lastEv=self.lastEventCache[ev['kn_to']]
        except:
            lastEv={}

        temp=ev['last'].split('\xb7')
        ev['last']=temp[1]
        #ev['time']=temp[0]
        ev['symbol']=subtopic
        #ev['date']='********date'
        ev['volume']=''.join(ev['volume'].split(','))
        ev['avgvol']=''.join(ev['avgvol'].split(','))
        temp=ev['change'].split(' ')[0]
        if temp[0]=='+':
            ev['change']=temp[1:]
        else:
            ev['change']=temp
        temp=ev['dayrange']
        temp=temp.split('-')
        ev['high']=temp[1].strip()
        ev['low']=temp[0].strip()
        ev['kn_payload']=ev['last']

        sameAsOld=1
        for el in quoteElementsToCompare:
            if not lastEv.get(el)==ev[el]:
                sameAsOld=0
                lastEv[el]=ev[el]
                break

        
        if sameAsOld:
            return (None,0)
        
        self.lastEventCache[ev['kn_to']]=copy.copy(ev)
        
        t=time.time()
        ev['sensor_time']=t
        ev['kn_id']=str(t)
        return (None,1)

    #If we need to create piecemeal events...
    def fixupToSendList(self,eventList,subtopic=None):
        if not self.needsSubSubTopics.has_key(subtopic):
            return

        additionalEvents=[]
        for ev in eventList:
            for attr in quoteElements:
                oe={}
                oe['symbol']=ev['symbol']
                #oe['date']=ev['date']
                #oe['time']=ev['time']
                oe['sensor_time']=ev['sensor_time']
                oe['kn_payload']=ev[attr]
                oe['kn_to']='/'.join((self.rootTopic,
                                      subtopic,
                                      attr))
                oe['kn_expires']=ev['kn_expires']
                additionalEvents.append(oe)

                #Also generate sub-sub-topic events on
                #a couple of additional topics fo
                #compatibility with burdman's sensor
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
    Nasdaq3Sensor().run()


# End of sensor_nasdaq3.py
