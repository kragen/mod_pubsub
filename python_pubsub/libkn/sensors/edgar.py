#!/usr/bin/python

# $Id: edgar.py,v 1.1 2003/03/15 04:52:49 ifindkarma Exp $

import KNCONFIG
import time, random, urllib, sys, traceback
from sgmllib import SGMLParser

# Include local libraries:
sys.path = [ "../" ] + sys.path

from libkn import kn
from pprint import pprint
from threading import Thread

EDGAR_BASEURL='http://www.sec.gov'
EDGAR_QUERYURL=EDGAR_BASEURL+'/cgi-bin/formlynx.pl.b?page=results&cik=%s'
EDGAR_SWEEPTIME=600

KN_EDGAR_TOPIC='/what/edgar'
expireString='+%d'%(60*60*24*1) #1 day

knr=kn(host=KNCONFIG.PUBSUBSERVER_HOSTNAME,port=KNCONFIG.PUBSUBSERVER_PORT,traceOnly=KNCONFIG.PUBSUBSERVER_TRACEONLY)
cikDict={}
cikCount=0


#==========================================================================================
# Gets called for each new kn_subtopics event
#==========================================================================================
def newCikCallback(ev):
	"""for each CIK add it to the dictionary"""
        global cikDict,cikCount

	cik=ev.kn_payload.strip()
	
	if cik[:3]=='kn_':
		return
	try:
		cik=str(int(cik))
	except:
		pass
	
	if cikDict.has_key(cik):
		return

	cikDict[cik]={}
        cikCount+=1

	publishEdgarDataForCik(cik)

	print 'Registered cik: '+cik

#==========================================================================================
# Edgar Parser
#==========================================================================================

class EdgarParser(SGMLParser):

	def reset(self):
		SGMLParser.reset(self)
		self.state='BEFORE HEADER'
		self.inColumn=0
		self.row=0
		self.column=0
		self.rowData=[]
		self.rowUrls=[]
		self.outputData=[]
		
#	def parse_starttag(self,i):
#		print 'Current state: '+self.state
#		return SGMLParser.parse_starttag(self,i)

	def handle_comment(self,text):
		if self.state=='BEFORE HEADER' and text.strip()=='END HEADER':
			self.state='AFTER HEADER'
		return

	def start_table(self, attrs):
		if self.state=='AFTER HEADER':
			self.state='IN TABLE'
			
		self.row=0
		return

	def end_table(self):
		if self.state=='IN TABLE':
			self.state='END'

	def start_tr(self, attrs):
		self.column=0
		return

	def end_tr(self):
		if self.state=='IN TABLE' and len(self.rowData)>=6:
			self.rowData=["".join(x) for x in self.rowData]

			if len(self.rowData[4])>=5:
				v={'type':self.rowData[3],
				   'date':self.rowData[4],
				   'size':self.rowData[5],
				   'typeUrl':''.join([EDGAR_BASEURL,'/',self.rowUrls[1]])}

				if len(self.rowUrls)>2:
					v['htmlUrl']=''.join([EDGAR_BASEURL,'/',self.rowUrls[2]])

				self.outputData.append(v)

		self.row+=1
		self.rowData=[]
		self.rowUrls=[]
		return

	def start_td(self, attrs):
		self.inColumn=1
		self.rowData.append([])
		return

	def end_td(self):
		self.column+=1
		self.inColumn=0
		return

	def start_a(self, attrs):
		href = [v for k, v in attrs if k=='href']  
		if href:
			self.rowUrls.append(v)

	def end_a(self):
		return


	def handle_data(self,data):
		if self.inColumn and len(self.rowData)>self.column:
			self.rowData[self.column].append(data)


#==========================================================================================

edgarParser=EdgarParser()
def publishEdgarDataForCik(cik):
	global edgarParser

	url=EDGAR_QUERYURL%(cik)
	kn_to=''.join([KN_EDGAR_TOPIC,'/',cik])

	documents=getEdgarDataFromUrl(url)

	toSend=[]
	out = edgarParser.outputData
	out.reverse()
	for v in out:
		v['cik']=cik
		try:
			dict=cikDict[cik]
		except:
			dict={}
			cikDict[cik]=dict
			
		#keys are (date,type,size)
		k=(v['date'],v['type'],v['size'])
		if not dict.has_key(k):
			v['kn_id']='.'.join(['_'.join(k[0].split('/')),
					     '_'.join(k[1].split('/')),
					     '_'.join(k[2].split(':'))])
			v['kn_expires']='infinity'
			v['kn_to']=kn_to
			v['kn_payload']=v['kn_id']
			
			dict[k]=v
			toSend.append(v)
			if len(toSend)>=KNCONFIG.PUBSUBSERVER_MAXBATCHSIZE:
				knr.publish(d=toSend)
				toSend=[]

	if len(toSend)>0:
		knr.publish(d=toSend)
		toSend=[]


def getEdgarDataFromUrl(url):
	print 'Getting results from URL '+url
	h=urllib.urlopen(url)
	h.fp.set_timeout(120)

	try:
		edgarParser.reset()
		s=h.read()
		edgarParser.feed(s)
	except Exception,e:
		print 'Exception %s raised in getEdgarDataFromUrl'%e
		traceback.print_tb(sys.exc_traceback)
		pass

	retVal=edgarParser.outputData
	edgarParser.close()
	h.close()

	return retVal



#---------------------------------------- START
	
try:
	knr.start()
	time.sleep(2)
	publishEdgarDataForCik('353524')
	publishEdgarDataForCik('1090872')
	publishEdgarDataForCik('777676')
	publishEdgarDataForCik('51143')
	while 1:
		m=knr.subscribe(kn_from=KN_EDGAR_TOPIC+'/kn_subtopics',cb=newCikCallback,options={'do_max_age':'infinity'})
		if m is None:
			print 'Retrying subscription'
		else:
			print m
			break

	while knr.connected():
		print 'cikCount=%d'%(cikCount)
		time.sleep(EDGAR_SWEEPTIME)
		for k in cikDict.keys():
			publishEdgarDataForCik(k)
		
finally:
	knr.stop()



# End of edgar.py
