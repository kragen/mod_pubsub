
/**
 * Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
 *
 * @KNOWNOW_LICENSE_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the KnowNow, Inc., nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 * 
 *
 * $Id: PublisherThread.java,v 1.2 2004/04/19 05:39:10 bsittler Exp $
 *
 **/

//
// PublisherThread.java -
//
//	Implement a single thread of the publishing test.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

class PublisherThread
    implements Runnable
{
    // Instance variables
    String 		routerURI;
    String		topic;
    int    		eventSize;
    int    		eventCount;
    int    		delay;
    Log    		logger;
    PublisherReporter	reporter;
    int			reportIndex;
    
    // 
    // PublisherThread::PublisherThread -
    //
    //		Constructor.
    //

    PublisherThread(String            routerURI,
		    String            topic,
		    int               eventSize,
		    int               eventCount,
		    int	              delay,
		    Log	              logger,
		    PublisherReporter reporter,
		    int		      reportIndex)

    {
	this.routerURI   = routerURI;
	this.topic       = topic;
	this.eventSize   = eventSize;
	this.eventCount  = eventCount;
	this.delay       = delay;
	this.logger      = logger;
	this.reporter    = reporter;
	this.reportIndex = reportIndex;
    }

    //
    // Runnable::run -
    //
    //		Thread implementation.
    //

    public void run()
    {
	String threadName = Thread.currentThread().getName();
	if (logger != null)
	{
	    logger.Log("Thread " + threadName + " started");
	}

	boolean status =
	    Router.Publish(routerURI,
			   topic,
			   eventSize,
			   eventCount,
			   delay,
			   logger);

	if (logger != null)
	{
	    logger.Log("Thread " + threadName + " finished with status " + status);
	}

	if (reporter != null)
	{
	    reporter.ReportStatus(reportIndex, status);
	}
    }
}
