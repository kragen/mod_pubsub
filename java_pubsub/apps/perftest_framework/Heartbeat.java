
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
 * $Id: Heartbeat.java,v 1.2 2004/04/19 05:39:10 bsittler Exp $
 *
 **/

//
// Heartbeat.java -
//
//	Implement the Heartbeat thread which tells the Manager
//	that the Agent running the thread is alive and well.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;

public class Heartbeat
    implements Runnable
{
    // Instance variables
    private Agent	MyAgent;
    private String	MyHost;

    //
    // Heartbeat::Heartbeat -
    //
    //	Constructor.
    //

    public Heartbeat(Agent MyAgent, String MyHost)
    {
	this.MyAgent = MyAgent;
	this.MyHost  = MyHost;
    }

    //
    //	Runnable::run -
    //
    //	Execution thread for the heartbeat. Send a package of
    //	information (a HeartbeatData object) to the Manager
    //	every HeartbeatTime milliseconds. The thread will
    //  stop when the Running flag in the Agent becomes false.
    //

    public void run()
    {
	while (MyAgent.Running())
	{
	    // Create and populate a HeartbeatData object
	    HeartbeatData Data = new HeartbeatData();
	    Data.Setup(MyAgent.GetAgentID(), MyAgent.GetStatus(), MyHost);
	    
	    // Send it to the manager 
	    MyAgent.SendObjectToManager(Data, MyAgent.GetAgentID());

	    // Sleep for a bit
	    try
	    {
		Thread.currentThread().sleep(Common.HeartbeatTime);
	    }	
	    catch (InterruptedException Ex)
	    {
		// This is ignorable 
	    }
	}
    }
}
