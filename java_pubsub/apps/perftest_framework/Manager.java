
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
 * $Id: Manager.java,v 1.2 2004/04/19 05:39:10 bsittler Exp $
 *
 **/

//
// Manager.java -
//
//	Manage the testing process. Keep track of available Agent
//	processes running on the same network. Direct the Agents
//	to run a test (this is actually composed of a series of
//	smaller steps). Generate a comprehensive log file.
//
//	A coordinating router is used to transfer information
//	between this Manager and the Agents. 
//
//	The program is invoked as follows:
//
//	java Manager COORD_ROUTER_URI TEST_SPEC
//
//	The COORD_ROUTER_URI is a string (e.g. "http://coord:8000")
//
//	The TEST_SPEC is a file name. The file contains a specification
//	of the test to be run. The specification syntax is described
//	in file TestSpec.java.
//
//	The Agent processes must be running before the Manager process
//	is started.
//
//	Processing proceeds as follows:
//
//	1. The Manager creates a connection to the coordinating router.
//
//	2. The Manager starts listening to discover the set of available
//	   agents. This phase lasts 15 seconds.
//
//	3. The Manager parses, error checks, and stores the test 
//	   specification. The symbolic agent names in the specification
//	   are mapped to actual agent names.
//
//	4. The Manager creates a thread to periodically update the
//	   Agent Status Page.
//
//	5. The Manager executes the test specification. This typically
//	   involves sending commands to one agent or to all agents.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;
import java.util.*;
import java.text.*;
import java.io.PrintStream;
import java.io.FileOutputStream;
import com.knownow.common.*;
import com.knownow.microserver.*;

public class Manager
    implements KNEventListener, KNStatusHandler, Log
{
    // Class variables
    private static AgentDataMap	agentDataMap;
    private static PrintStream	logPrintStream;
    private static boolean	Running;
    private static TestSpec     testSpec;

    public static void main(String[] args)
    {
	// Initialize
	String coordRouterURI = null;
	String testSpecFile   = null;
	Running               = true;

	//
	// Get the arguments:
	// 1 - The URI of the coordinating router
	// 2 - Name of the test spec
	//

	if (args.length == 2)
	{
	    coordRouterURI = args[0];
	    testSpecFile   = args[1];
	}
	else
	{
	    System.err.println("Usage: java Manager " +
			       "COORD_ROUTER_URI TEST_SPEC");
	    System.exit(0);
	}

	try
	{
	    // Open up the log file
	    FileOutputStream logFile = new FileOutputStream(Common.ManagerLogFile);
	    logPrintStream           = new PrintStream(logFile);
	}
	catch (Exception Ex)
	{
	    System.err.println("Manager: Could not create log file");
	    System.exit(0);
	}
	
	System.out.println("*** Manager Started ***");
	System.out.println("Using coordinating router at " + coordRouterURI);
	System.out.println("Using test spec at " + testSpecFile);
	System.out.println("Dynamic status at " + Common.StatusPageFile);
	System.out.println("Log file at " + Common.ManagerLogFile);

	// Create a connection to the server
	KNJServer coordRouter = new KNJServer(coordRouterURI);

	// Create the actual Manager object
	Manager ThisManager = new Manager(coordRouter, coordRouterURI);

	// Create an AgentDataMap to track the Agents
	agentDataMap = new AgentDataMap(ThisManager);

	// Create a thread to periodically update the Agent Status Page
	StatusUpdater StatusUpdaterThread = new StatusUpdater(ThisManager);
	new Thread(StatusUpdaterThread).start();

	// Listen for status messages
	KNOptions Options   = new KNOptions();
	KNRoute ThisRoute   = coordRouter.subscribe(Common.StatusTopic,
						    ThisManager, Options, null);

	if (ThisRoute.getHttpStatus() >= 300)
	{
	    System.err.print("Manager: Could not listen for status: " +
			     ThisRoute.getHttpStatus());
	    System.exit(0);
	}

	// For 15 seconds, just watch for Agents
	System.out.print("Waiting for agents");
	for (int i = 0; i < 15; i++)
	{	    
	    System.out.print(".");
	    Common.Sleep(1);
	}
	
	System.out.println();
	System.out.println(agentDataMap.GetAgentCount() + " agents found!");

	// Load and parse the test specification
	try
	{
	    testSpec = TestSpec.Load(testSpecFile, ThisManager, agentDataMap);
	    System.out.println("Test specification contains " +
			       testSpec.GetStepCount() + " commands.");
	}
	catch (Exception Ex)
	{
	    System.out.println("Manager: Could not load or parse configuration file.");
	    Common.DumpException("Manager", Ex);
	    System.exit(0);
	}

	// Now run the test per the specification
	System.out.println("========================================");
	testSpec.Run(ThisManager);
	System.out.println("========================================");

	// Tell other threads to shut down, then give them a chance to do so
	Running = false;
	Thread.currentThread().yield();

	// Write one final status page

	// Shutdown
	coordRouter.shutdown();
	System.out.println("Manager exiting");
	System.exit(0);
    }

    // Instance variables
    private KNJServer		coordRouter;
    private String		coordRouterURI;

    //
    // Manager::Manager -
    //
    //	Constructor for an instance of Manager.
    //

    public Manager(KNJServer coordRouter, String coordRouterURI)
    {
	this.coordRouter    = coordRouter;
	this.coordRouterURI = coordRouterURI;

	Log(Common.StringRepeat(60, "="));
	Log("Manager Started");
    }

    //
    //	Manager::Running -
    //
    //		Return true if child threads should keep running, or
    //		false otherwise.
    //

    public boolean Running()
    {
	return Running;
    }

    //
    // Manager::GetCoordRouter -
    //
    //		Accessor function for the coordinating router.
    //

    public KNJServer GetCoordRouter()
    {
	return coordRouter;
    }

    //
    // Manager::GetCoordRouterURI -
    //
    //		Accessor function for the URI of the coordinating router.
    //

    public String GetCoordRouterURI()
    {
	return coordRouterURI;
    }

    //
    //	Manager::GetAgentDataMap -
    //
    //		Return the Manager's AgentDataMap object.
    //

    public static AgentDataMap GetAgentDataMap()
    {
	return agentDataMap;
    }

    //
    // Manager::SendObjectToAgents -
    //
    //		Send the given object to all of the agents
    //		in the agent map.
    //

    public void SendObjectToAgents(Object TheObject)
    {
	try
	{
	    // Serialize and send the object
	    String  TheSerial = KSerializer.Serialize(TheObject);
	    KNEvent TheEvent  = new KNEvent();
	    TheEvent.put("kn_payload", TheSerial);
	    coordRouter.publish(Common.CommandTopic, TheEvent, null);
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Manager", Ex);
	    System.err.println("Manager: Could not send object to agents");
	}
    }

    //
    //	Manager::HandleHeartbeatData -
    //
    //		Process the given heartbeat data. Update the local
    //		status for the agent.
    //

    private void HandleHeartbeatData(HeartbeatData data)
    {
	agentDataMap.UpdateAgentData(data);
    }

    //
    //	Manager::HandleStatusRun -
    //
    //		Process the given command run status. 
    //

    private void HandleStatusRun(StatusRun status)
    {
	Log("Agent: "                + status.agentID +
	    " ran command: "         + status.command +
	    " and received output: " + status.output);
    }

    //
    //	Manager::HandleStatusRoute -
    //
    //		Process the given route status data.
    //

    private void HandleStatusRoute(StatusRoute status)
    {
	Log("Agent: "                   + status.agentID +
	    "created routes on router " + status.routerURI +
	    "from "                     + status.sourceTopic +
	    "to "                       + status.journalTopic);
    }
    
    //
    //	Manager::HandleStatusPublish -
    //
    //		Process the given publish status data.
    //

    private void HandleStatusPublish(StatusPublish status)
    {
	Log("Agent: "              + status.agentID   +
	    " published to topic " + status.topic     +
	    " on router "          + status.routerURI +
	    " with result "        + status.status);
    }

    //
    //	Manager::HandleStatusSubscribe -
    //
    //		Process the given subscribe status data.
    //

    private void HandleStatusSubscribe(StatusSubscribe status)
    {
	Log("Agent: "               + status.agentID   +
	    " subscribed to topic " + status.topic     +
	    " on router "           + status.routerURI +
	    " with result  "        + status.status);
    }

    //
    //	Manager::HandleStatusCounter -
    //
    //		Process the given counter status data.
    //

    private void HandleStatusCounter(StatusCounter status)
    {
	Log("Agent: "   + status.agentID +
	    " received " + status.count  +
	    " messages"                  +
	    " on topic " + status.topic);

	System.out.println("  " + status.topic + " " + status.count + " " +
			   status.agentID);
    }

    //
    //	Manager::HandleStatusStart -
    //
    //		Process the given start status data.
    //

    private void HandleStatusStart(StatusStart status)
    {
	Log("Agent: "           + status.agentID +
	    " returned status " + status.status  +
	    " when starting router");
    }

    //
    //	Manager::HandleStatusStop -
    //
    //		Process the given stop status data.
    //

    private void HandleStatusStop(StatusStop status)
    {
	Log("Agent: "           + status.agentID +
	    " returned status " + status.status  +
	    " when stopping router");
    }

    //
    //	Manager::HandleStatusConfig -
    //
    //		Process the given config status data.
    //

    private void HandleStatusConfig(StatusConfig status)
    {
	Log("Agent: "           + status.agentID +
	    " returned status " + status.status  +
	    " when configuring router");
    }

    //
    //	Manager::HandleStatusClear -
    //
    //		Process the given clear status data.
    //

    private void HandleStatusClear(StatusClear status)
    {
	Log("Agent: "           + status.agentID +
	    " returned status " + status.status  +
	    " when clearing router");
    }

    //
    //	Manager::HandleStatusDownloads-
    //
    //		Process the given clear download data.
    //

    private void HandleStatusDownload(StatusDownload status)
    {
	Log("Agent: "                 + status.agentID +
	    " returned status "       + status.status  +
	    " when downloading from " + status.downloadURL);
    }

    //
    //	Manager::Log -
    //
    //		Time-stamp and then write the given
    //		message to the log file.
    //

    public void Log(String Msg)
    {
	SimpleDateFormat MyFormat =
	    new SimpleDateFormat("dd-MMM-yyyy HH:mm:ss");
	String MyDate = MyFormat.format(new Date());

	logPrintStream.println(MyDate + " " + Msg);

	logPrintStream.flush();
    }

    // KNEventListener::onEvent
    public void onEvent(KNEvent TheEvent)
    {
	String p = TheEvent.getPayload();

	if (p.startsWith("object:"))
	{
	    try
	    {
		Object status    = KSerializer.Instantiate(p);
		String className = status.getClass().getName();

		if (className.equals("HeartbeatData"))
		{
		    HandleHeartbeatData((HeartbeatData) status);
		}
		else
		if (className.equals("StatusRun"))
		{
		    HandleStatusRun((StatusRun) status);
		}
		else
		if (className.equals("StatusRoute"))
		{
		    HandleStatusRoute((StatusRoute) status);
		}
		else
		if (className.equals("StatusPublish"))
		{
		    HandleStatusPublish((StatusPublish) status);
		}
		else
		if (className.equals("StatusSubscribe"))
		{
		    HandleStatusSubscribe((StatusSubscribe) status);
		}
		else
		if (className.equals("StatusCounter"))
		{
		    HandleStatusCounter((StatusCounter) status);
		}
		else
		if (className.equals("StatusStart"))
		{
		    HandleStatusStart((StatusStart) status);
		}
		else
		if (className.equals("StatusStop"))
		{
		    HandleStatusStop((StatusStop) status);
		}
		else
		if (className.equals("StatusConfig"))
		{
		    HandleStatusConfig((StatusConfig) status);
		}
		else
		if (className.equals("StatusClear"))
		{
		    HandleStatusClear((StatusClear) status);
		}
		else
		if (className.equals("StatusDownload"))
		{
		    HandleStatusDownload((StatusDownload) status);
		}

		status = null;
	    }
	    catch (Exception e)
	    {
		Common.DumpException("Manager", e);
	    }
	}
	else
	{
	    System.out.print("Manager: onEvent: " + p);
	}
   }

    // KNStatusHandler::onStatusEvent
    public void onStatusEvent(KNEvent TheEvent)
    {
	System.out.print("Manager: onStatusEvent: " + TheEvent.get("status"));
    }
}
