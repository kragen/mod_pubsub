#!/usr/bin/python

# $Id: nasdaq.py,v 1.1 2003/03/15 04:52:49 ifindkarma Exp $

import KNCONFIG
import time, random, urllib
from mx import DateTime

# Include local libraries:
sys.path = [ "../" ] + sys.path

from libkn import kn
from pprint import pprint
from threading import Thread

NASDAQ_MAXRQUESTS=10
NASDAQ_URL='http://quotes.nasdaq.com/quote.dll?page=custom&mode=stock'
NASDAQ_MINSWEEPTIME=5
NASDAQ_MINSWEEPTIME_NODELTA=300

KN_NASDAQ_TOPIC='/what/nasdaq2'
#expireString='+%d'%(NASDAQ_MINSWEEPTIME*5)
expireString='infinity'

quoteElements=['symbol','fullname','date','time','bid','ask','prev_close','high',
		      'low','last','unknown1','unknown2','change','volume']
numericQuoteElements=[4,5,7,8,9,9,9,9,9,12,13] #repeat the ones you want simulated most often
tickers={}
transmittedData={}
currentData={}
#didTransmit=0
gotDataException=0
sentSimulatedData=0
totalTransmitted=0
tickersByTens_last=None
tickersByTens=['']
tickersCount=0
gettingData=0
alwaysSimulate=1

knr=kn(host=KNCONFIG.PUBSUBSERVER_HOSTNAME,port=KNCONFIG.PUBSUBSERVER_PORT,traceOnly=KNCONFIG.PUBSUBSERVER_TRACEONLY)


#==========================================================================================
# Gets called for each new kn_subtopics event for the nasdaq topic
#==========================================================================================
def newTickerCallback(ev):
	"""for each new ticker add it to the dictionary, but also
	add it in groups of NASDAQ_MAXRQUESTS to tickersByTen"""

	global tickers,tickersByTens_last,tickersByTens,tickersCount
	global gettingData
	
	ticker=ev.kn_payload.lower()
	
	if ticker[:3]=='kn_':
		return

	if(ev.kn_payload!=ticker):
		knr.subscribe(kn_from=KN_NASDAQ_TOPIC+'/'+ticker,
			      kn_to=KN_NASDAQ_TOPIC+'/'+ev.kn_payload)
	
	if tickers.has_key(ticker):
		return

	tickers[ticker]=None

	if (tickersCount%NASDAQ_MAXRQUESTS)==0:
		if tickersCount and not gettingData:
			getTickerData(tickersByTens_last)
		tickersByTens_last=''

	tickersByTens_last+= ('&symbol='+ticker)
	tickersCount+=1
	tickersByTens[len(tickersByTens)-1]=tickersByTens_last

	print 'Registered ticker: '+ticker

#	if (tickersCount%10)==1:
#		tickersByTens.append(tickersByTens_last)


#==========================================================================================
# Gets 10 tickers at a time
#==========================================================================================
def getAllTickerData():
	global gettingData

	gettingData=1
	if not tickersCount:
		return

	for groupOfTen in tickersByTens:
		getTickerData(groupOfTen)
	gettingData=0
	#print 'finished getAllTickerData'


def getTickerData(groupOfTen,num=10):
	global tickers,tickersByTens_last,tickersByTens,tickersCount
	global currentData

	if num>10:
		raise

	st=urllib.urlopen(NASDAQ_URL+groupOfTen)
	for i in range(num):
		s=st.readline()
		if not s:break
		#s should be in the form
		#IBM|INTERNATIONAL BUSINESS MA|08/29/2001|16:00:00|0|0||104.95|105.90|103.82|104.13|16:00:00|-0.82|4840300||
		dat=s.split('|')
		if len(dat)<14:
			break

		dat=[s.split('*')[0] for s in dat]
		ev=[(quoteElements[i],
		     dat[i])
		    for i in range(min(len(dat),14))]
		ev.append(('kn_payload',dat[9]))
		ev.append(('kn_to',KN_NASDAQ_TOPIC+'/'+dat[0].lower()))
		ev.append(('kn_expires',expireString))
		ev.append(('kn_id','0'))
		currentData[dat[0]]=ev
	st.close()

#==========================================================================================
# Sends events for any changed values
#==========================================================================================
def sendAllChangedTickerData():
	global tickers,tickersByTens_last,tickersByTens,tickersCount
	global currentData,transmittedData,totalTransmitted
	#global didTransmit

	if not tickersCount:
		return

	dat=[]
	for ticker,current in currentData.items():
		#if transmittedData.has_key(ticker):
		#	if transmittedData[ticker]==current:
		#		continue

		if len(dat)>KNCONFIG.PUBSUBSERVER_MAXBATCHSIZE:
			knr.publish(d=dat)
			dat=[]

		current[17]=('kn_id','%s_%d'%(ticker,time.time()))
		dat.append(current)
		transmittedData[ticker]=current
		#didTransmit=1
		totalTransmitted+=1

	if len(dat)>0:
		knr.publish(d=dat)

	print 'finished sendAllChangedTickerData (totalTransmitted=%d)'%(totalTransmitted)

def sendSimulatedData():
	global tickers,tickersByTens_last,tickersByTens,tickersCount
	global currentData,transmittedData,totalTransmitted
	#global didTransmit

	keys=tickers.keys()
	dat=[]
	for ticker in keys:
		if len(dat)>KNCONFIG.PUBSUBSERVER_MAXBATCHSIZE:
			knr.publish(d=dat)
			dat=[]

		if random.randrange(50)>25:
			try:
				current=currentData[ticker]
			except:
				current=[('symbol',ticker),
					 ('fullname','FULLNAME'),
					 ('date','1/1/2001'),
					 ('time','10:00:00'),
					 ('bid','40'),
					 ('ask','40'),
					 ('prev_close','39'),
					 ('high','41'),
					 ('low','39'),
					 ('last','40'),
					 ('unknown1',''),
					 ('unknown2',''),
					 ('change','1'),
					 ('volume','10000'),
					 ('kn_payload','40'),
					 ('kn_to',KN_NASDAQ_TOPIC+'/'+ticker.lower()),
					 ('kn_expires',expireString),
					 ('kn_id','0')]
				currentData[ticker]=current
					 
			x=numericQuoteElements[random.randrange(len(numericQuoteElements))]
			y=current[x][1].strip()
			try:
				if(len(y)<0):
					continue
				y=float(y)
				if(x==13):
					y=int(y)
					y+=random.randrange(int(float(y)/10))
				else:
					y+=(y/30)-random.random()*(y/15)
					if y!=13 and y<0:y=-y
					if y!=13 and y==0:y=random.randrange(40)
			except:
				y=None

			if not y is None:
				current[x]=(current[x][0],str(y))
				if x==9:
					current[14]=('kn_payload',str(y))
				current[17]=('kn_id','%s_%d'%(ticker,time.time()))
				dat.append(current)
				transmittedData[ticker]=current
				totalTransmitted+=1

			if len(dat)>KNCONFIG.PUBSUBSERVER_MAXBATCHSIZE:
				knr.publish(d=dat)
				dat=[]

	if len(dat)>0:
		knr.publish(d=dat)
	print 'finished sendSimulatedData'
		


	

#==========================================================================================
# Threads
#==========================================================================================
def thread_getDataFunction():
	#global didTransmit
	global tickersCount,gotDataException
	print 'thead_getDataFunction started'

	lastTickersCount=0
	while 1:
		lastTime=time.time()
		sleeptime=0
		while 1:
			try:
				getAllTickerData()
				gotDataException=0
				break
			except:
				gotDataException+=1
				sleeptime+=5
				if sleeptime>20:
					sleeptime=20
				time.sleep(sleeptime)
				print 'Exception opening URL in getData, retrying...'

		while (time.time()-lastTime)<NASDAQ_MINSWEEPTIME_NODELTA:
			x=max(min(NASDAQ_MINSWEEPTIME_NODELTA-time.time()+lastTime,NASDAQ_MINSWEEPTIME),1)
			print 'getData sleeping for %d'%x
			time.sleep(x)

			#if didTransmit:
			#	didTransmit=0
			#	break
			
			if tickersCount!=lastTickersCount:
				break

			x=(time.time()-lastTime)/NASDAQ_MINSWEEPTIME
			if x==3:
				print 'getData long sleep'
			
			
			
		lastTickersCount=tickersCount

def thread_sendDataFunction():
	global gotDataException,sentSimulatedData
	print 'thead_sendDataFunction started'
	while 1:
		lastTime=time.time()
		lt=time.localtime()

		# NASDAQ open?
		x=lt[3]*100+lt[4]

		if DateTime.today().day_of_week>=5 or x<930 or x>1600 or gotDataException>3 or alwaysSimulate:
			sleepTime=NASDAQ_MINSWEEPTIME
			sendSimulatedData()
		else:
			sleepTime=NASDAQ_MINSWEEPTIME/2
			sendAllChangedTickerData()
			
		if (time.time()-lastTime)<sleepTime:
			time.sleep(max(sleepTime-time.time()+lastTime,1))
	

#---------------------------------------- START
	
try:
	knr.start()
	time.sleep(2)
	while 1:
		m=knr.subscribe(kn_from=KN_NASDAQ_TOPIC+'/kn_subtopics',cb=newTickerCallback,options={'do_max_age':'infinity'})
		if m is None:
			print 'Retrying subscription'
		else:
			print m
			break

	#knr.publish(d=[('kn_to','/what/foo'),('kn_payload','hello')])
	getDataThread=None
	sendDataThread=None
	
	lastCount=0
	x=0
	while knr.connected():
		if not getDataThread or not getDataThread.isAlive():
			print '(Re)starting getDataThread...'
			getDataThread=Thread(target=thread_getDataFunction)
			getDataThread.setDaemon(1)
			getDataThread.start()

		if not sendDataThread or not sendDataThread.isAlive():
			print '(Re)starting sendDataThread...'
			sendDataThread=Thread(target=thread_sendDataFunction)
			sendDataThread.setDaemon(1)
			sendDataThread.start()

		x+=1
		if x>5:
			print 'Tickers=%d  NumEvents=%d'%(tickersCount,totalTransmitted-lastCount)
			lastCount=totalTransmitted
			x=0
		time.sleep(10)
		
finally:
	knr.stop()


# End of nasdaq.py
