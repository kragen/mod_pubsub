#!/usr/bin/python

# $Id: sensor_yb.py,v 1.1 2003/03/15 04:52:49 ifindkarma Exp $

import os
import pprint

from sensor import HTMLParser,WebScrapeSensor

YB_BASEURL='http://messages.yahoo.com'
YB_QUERYURL=YB_BASEURL+'/?action=q&board=%s'

class YBParser(HTMLParser):
    def reset(self):
        HTMLParser.reset(self)
        self.state='START'
        self.table=0
        self.inColumn=0
        self.row=0
        self.column=0
        self.rowData=[]
        self.rowUrls=[]

    def start_table(self, attrs):
        self.table+=1
        self.row=0
        if self.state=='START' and self.table==10:
            self.state='LOOKING FOR PREVIOUS URL'
        elif self.state=='LOOKING FOR PREVIOUS URL':
            self.state='LOOKING FOR HEADER'
        return

    def end_table(self):
        if self.state=='LOOKING FOR HEADER' or self.state=='LOOKING FOR MESSAGES':
            self.state='END'

    def start_tr(self, attrs):
        self.column=0
        return

    def end_tr(self):
        if self.state=='LOOKING FOR PREVIOUS URL':
            if self.row==0:
                self.rowData=["".join(x).strip() for x in self.rowData]
                if len(self.rowData)>0 and self.rowData[0].strip()[:8]=='Previous' and len(self.rowUrls[0])>0:
                    url=self.rowUrls[0][0]
                    self._previousUrl=YB_BASEURL+url
					
			
        elif self.state=='LOOKING FOR HEADER':
            self.rowData=["".join(x).strip() for x in self.rowData]
            if (len(self.rowData)>=8
                and self.rowData[0]=='#'
                and self.rowData[2]=='Subject'
                and self.rowData[3]=='Author'):
                self.state='LOOKING FOR MESSAGES'

        elif self.state=='LOOKING FOR MESSAGES' and len(self.rowData)>=8:
            self.rowData=["".join(x).strip() for x in self.rowData]

            #lots of cruft because strptime() not available on win2k
            dt=[x.strip() for x in self.rowData[7].split(' ')]
            dt[0]=dt[0].split('/')
            dt[1]=dt[1].split(':')
            if dt[2]=='pm':
                x=int(dt[1][0])
                if x>12:
                    dt[1][0]=str(x+12)

            v={ 'id':self.rowData[0],
                'subject':self.rowData[2],
                'author':self.rowData[3],
                'sentiment':self.rowData[4],
                'recs':self.rowData[5],
                'date':'%s/%s/%s'%(dt[0][0],dt[0][1],dt[0][2]),
                'time':'%s:%s:00'%(dt[1][0],dt[1][1]),
                'subjectUrl':''.join([YB_BASEURL,self.rowUrls[2][0]])}
            
            self._outputData.append(v)

        self.row+=1
        self.rowData=[]
        self.rowUrls=[]
        return

    def start_td(self, attrs):
        self.inColumn=1
        self.rowData.append([])
        self.rowUrls.append([])
        return

    def end_td(self):
        self.column+=1
        self.inColumn=0
        return

    def start_a(self, attrs):
        if not self.state in ['LOOKING FOR MESSAGES','LOOKING FOR PREVIOUS URL']:
            return

        href=None
        for k,v in attrs:
            if k=='href':
                href=v
                break

        if href:
            self.rowUrls[self.column].append(href)

    def end_a(self):
        return


    def handle_data(self,data):
        if self.inColumn and len(self.rowData)>self.column:
            self.rowData[self.column].append(data)


class YBSensor(WebScrapeSensor):
    #===========================================================================
    # Configuration of the WebScrapeSensor
    #===========================================================================
    rootTopic='/what/yahoostock/chat'
    subtopicBased=1
    maxPreviousPages=2
    knPayloadHeader='subject'
    expireString='+%d'%(60*60*24*3) #2 days
    htmlTimeout=60
    initialSubtopics=('ibm','exds','afci','a','orcl','cdts')
    maintainSingletonEvent=0

    parser = YBParser()

    #===========================================================================
    # Configuration of this sensor
    #===========================================================================
    offpeakLoopDelay = 120
    peakLoopDelay = 15
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
        return YB_QUERYURL%subtopic

    def fixEventAndGetKey(self,ev,subtopic,i):
        k = ev['id']
        ev['kn_id']='_'.join((subtopic,k))
        return (k,1)

    def fixupToSendList(self,list,subtopic=None):
        list.reverse()
        return

if __name__ == '__main__':
    YBSensor().run()


# End of sensor_yb.py
