
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
 * $Id: Listen.java,v 1.2 2004/04/19 05:39:10 bsittler Exp $
 *
 **/

//
// Listen.java -
//
//	Listen to a router topic and print what comes in.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;
import com.knownow.common.*;
import com.knownow.microserver.*;

public class Listen
    implements KNEventListener, KNStatusHandler
{
    public static void main(String[] Args)
    {
	String ServerURL = "http://localhost:8000/kn";
	String Topic     = "/ktest/test";
    
	System.out.println("Listen");

	// Create a connection to the server
	KNJServer Server = new KNJServer(ServerURL);

	// Set server options
	KNOptions Options = new KNOptions();

	// Start listening
	Listen ThisListen = new Listen();
	KNRoute ThisRoute = Server.subscribe(Topic, ThisListen, Options, null);
	
	if (ThisRoute.getHttpStatus() >= 300)
	{
	    System.err.print("Failed to create route to callback: " +
			     ThisRoute.getHttpStatus());
	    System.exit(0);
	}
    }

    public Listen() {};

    // KNEventListener::onEvent
    public void onEvent(KNEvent TheEvent)
    {
	String p = TheEvent.getPayload();

	if (p.startsWith("object:"))
	{
	    try
	    {
		Object o = KSerializer.Instantiate(p);

		if (o.getClass().getName().equals("RandomObject"))
		{
		    System.out.println("Got a RandomObject:");
		    RandomObject r = (RandomObject) o;
		    r.Dump();
		    System.out.println();
		}
	    }
	    catch (Exception e)
	    {
		System.out.print("Exception: ");
		String es = e.getMessage();
		if (es != null)
		{
		    System.out.print(es);
		}
		else
		{
		    System.out.print("(No message supplied)");
		}
		System.out.println();
		System.out.println("Stack trace:");
		e.printStackTrace();
	    }
	}
	else
	{
	    System.out.print("onEvent: " + p);
	}
    }

    // KNStatusHandler::onStatusEvent
    public void onStatusEvent(KNEvent TheEvent)
    {
	System.out.print("onStatusEvent: " + TheEvent.get("status"));
    }
}
