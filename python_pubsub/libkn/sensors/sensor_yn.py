#!/usr/bin/python

# $Id: sensor_yn.py,v 1.1 2003/03/15 04:52:49 ifindkarma Exp $

from sensor import HTMLParser,WebScrapeSensor

YN_BASEURL = 'http://biz.yahoo.com'
YN_QUERYURL = YN_BASEURL+'/n/%s/%s.html'


class YNParser(HTMLParser):
    months = { "Jan":1,
               "Feb":2,
               "Mar":3,
               "Apr":4,
               "May":5,
               "Jun":6,
               "Jul":7,
               "Aug":8,
               "Sep":9,
               "Oct":10,
               "Nov":11,
               "Dec":12}

    def reset(self):
        HTMLParser.reset(self)
        self.inBold=0
        self.boldData=[]
        self.itemData=[]
        self.urls=[]
        self.urlText=[]


    def start_ul(self, attrs):
        self.index=len(self._outputData)
        dateInfo = "".join(self.boldData).split(" ")
        try:
            self.date = "%d/%s/%s"%(self.months[dateInfo[1][:3]],dateInfo[2][:-1],dateInfo[3])
        except:
            raise
            self.date = "1/1/2001"
        self.urls=[]
        self.urlText=[]
        self.itemData=[]

    def end_ul(self):
        n=len(self._outputData)-self.index
        for i in range(self.index,len(self._outputData)):
            self._outputData[i]['index']=str(n)
            n=n-1
        self.urls=[]
        self.urlText=[]
        self.itemData=[]

    def start_li(self,attrs):
        if len(self.urls)>0:
            url=self.urls[0]
            if url[:5]!='http:':
                url=YN_BASEURL+url
            v={'title':self.urlText[0],
               'titleUrl':url,
               'date':self.date}
            self._outputData.append(v)
        self.urls=[]
        self.urlText=[]
        self.itemData=[]
		
    def end_li(self):
        pass

    def start_b(self, attrs):
        self.boldData=[]
        self.inBold=1

    def end_b(self):
        self.inBold=0

    def start_a(self, attrs):
        href=None
        for k,v in attrs:
            if k=='href':
                href=v
                break

        if href:
            self.urls.append(href)

        self.itemData=[]


    def end_a(self):
        if self.inBold:
            text="".join(self.boldData).strip()
        else:
            text="".join(self.itemData).strip()

        self.urlText.append(text)
        if text[:15]=='additional, old':
            self._previousUrl=YN_BASEURL+self.urls[0]

    def handle_data(self,data):
        if self.inBold:
            self.boldData.append(data)
        else:
            self.itemData.append(data)



class YNSensor(WebScrapeSensor):
    #===========================================================================
    # Configuration of the WebScrapeSensor
    #===========================================================================
    rootTopic='/what/yahoostock/news'
    subtopicBased=1
    maxPreviousPages=2
    knPayloadHeader='title'
    expireString='+%d'%(60*60*24*10) #10 days
    htmlTimeout=60
    initialSubtopics=('msft','ibm','exds','afci','a','orcl','cdts')
    maintainSingletonEvent=0

    parser = YNParser()

    #===========================================================================
    # Configuration of this sensor
    #===========================================================================
    offpeakLoopDelay = 300
    peakLoopDelay = 60
    peakStartTime = 800
    peakEndTime = 1700


    #===========================================================================
    # Overrides
    #===========================================================================
    def loopDelay(self,time):
        if time<self.peakStartTime or time>self.peakEndTime:
            return self.offpeakLoopDelay
        else:
            return self.peakLoopDelay

    def firstUrlForSubtopic(self,subtopic):
        return YN_QUERYURL%(subtopic[0],subtopic)

    def fixEventAndGetKey(self,ev,subtopic,i):
        try:
            k = ev['date'].split('/')
            k.extend(ev['index'])
            del ev['index']

            k = '_'.join(k)
            ev['kn_id']='_'.join((subtopic,k))
        except:
            k = "error key"
            print 'fixEventAndGetKey problem in Yahoo News sensor.'
            print ev
        return (k,1)
            

    def fixupToSendList(self,list,subtopic=None):
        list.reverse()
        return

if __name__ == '__main__':
    YNSensor().run()


# End of sensor_yn.py
