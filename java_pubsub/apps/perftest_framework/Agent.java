
/**
 * Copyright (c) 2000-2003 KnowNow, Inc.  All rights reserved.
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
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * 
 * 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
 * be used to endorse or promote any product without prior written
 * permission from KnowNow, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 *
 * $Id: Agent.java,v 1.1 2003/08/15 23:41:06 ifindkarma Exp $
 *
 **/

//
// Agent.java -
//
//	Implement the testing process. Use a heartbeat thread to
//	periodically tell the Master that this Agent is alive
//	and kicking. Respond to commands from the Master to 
//	do various testing-related actions.
//
//	A coordinating router is used to transfer information
//	between Agent and Manager.
//
//	The program is invoked as follows:
//
//	java Agent COORD_ROUTER_URI
//
//	The COORD_ROUTER_URI is a string (e.g. "http://coord:8000")
//
//	All of the Agent processes must be started before the
//	Manager is started.
//
//	Copyright (c) 2002 - KnowNow, Inc.
//

import java.lang.*;
import java.util.*;
import java.text.*;
import java.io.*;
import com.knownow.common.*;
import com.knownow.microserver.*;

public class Agent
    implements Log, KNEventListener, KNStatusHandler, PublisherReporter
{
    // Class variables
    private static String	Status;
    private static boolean	Running;
    
    public static void main(String[] args)
    {
	// Initalize
	String agentID        = null;
	String coordRouterURI = null;
	String host	      = null;
	Status                = AgentData.STATUS_IDLE;
	Running               = true;

	// Get the lone argument -- the URI of the coordinating router
	if (args.length == 1)
	{
	    coordRouterURI = args[0];
	}
	else
	{
	    System.err.println("Usage: java Agent COORD_ROUTER_URI");
	    System.exit(0);
	}

	// Create a unique ID for this Agent, and also get Host name
	try
	{
	    host    = Common.GetHostName();
	    agentID = Common.CreateUniqueID("Agent");
	    System.out.println("*** Agent " + agentID + " Started ***");
	    System.out.println("Using coordinating router at " + coordRouterURI);
	}
	catch (Exception Ex)
	{
	    System.err.println("Agent: Cannot start - " +
			       "No IP address/network connection");
	    System.exit(0);
	}

	// Create a connection to the server
	KNJServer coordRouter = new KNJServer(coordRouterURI);

	// Create the Agent object
	Agent ThisAgent = new Agent(coordRouter, agentID);

	// Create a heartbeat thread to keep the Manager informed
	Heartbeat HeartbeatThread = new Heartbeat(ThisAgent, host);
	new Thread(HeartbeatThread).start();

	// Start listening for commands
	KNOptions Options  = new KNOptions();
	KNRoute ThisRoute = coordRouter.subscribe(Common.CommandTopic,
						  ThisAgent,
						  Options, null);

	if (ThisRoute.getHttpStatus() >= 300)
	{
	    System.err.println("Agent: Could not listen for commands - " +
			       ThisRoute.getHttpStatus());
	    System.exit(0);
	}
    }

    // Instance variables
    private KNJServer	coordRouter;
    private CounterMap  counterMap;
    private String	agentID;
    private String	Architecture;
    private String	OperatingSystem;
    private String	TempDir;
    private int 	publisherSuccessCount;
    private Router	router;

    //
    //	Agent::Agent -
    //
    //		Constructor.
    //

    public Agent(KNJServer coordRouter, String agentID)
    {
	this.coordRouter  = coordRouter;
	this.agentID      = agentID;
	this.counterMap   = new CounterMap();
	try
	{
	    // Get architecture and operating system
	    Architecture    = System.getProperty("os.arch");
	    OperatingSystem = System.getProperty("os.name");

	    // Create a Router object to handle platform-specific operations
	    router = Router.GetRouter(Architecture, OperatingSystem);

	    if (router == null)
	    {
		Log("Could not create platform-specific Router object.");
		System.exit(0);
	    }

	    //  Form a directory name for the agent's files
	    String JavaTempDir = System.getProperty("java.io.tmpdir");
	    if (!JavaTempDir.endsWith(File.separator))
	    {
		JavaTempDir = JavaTempDir + File.separator;
	    }
	    TempDir = JavaTempDir + agentID + File.separator;

	    // Create the temporary directory if necessary
	    File fileTempDir = new File(TempDir);
	    if (!fileTempDir.exists())
	    {
		fileTempDir.mkdir();
		System.out.println("Created temporary directory: " + TempDir);
	    }
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Agent", Ex);
	    Log("Agent: Could not initialize");
	    System.exit(0);
	}
    }

    //
    //	Agent::Shutdown -
    //
    //		Arrange for an orderly shutdown of the agent.
    //

    public void Shutdown()
    {
	Running = false;
	
	counterMap.Dump(System.out);
	Log("Shutting down");
	Common.EmptyDirectory(TempDir, true);
	Log("Removed directory " + TempDir);

	coordRouter.shutdown();
	Log("Shut down router connection");

	//	Common.ListThreads();

	Log("Shutdown complete!");
    }

    //
    // Agent::SendObjectToManager
    //
    //		Send the given object to the Manager. It must
    //		be serializable as defined in KSerializer.
    //
   
    public void SendObjectToManager(Object TheObject)
    {
	SendObjectToManager(TheObject, null);
    }

    //
    // Agent::SendObjectToManager
    //
    //		Send the given object to the Manager with the
    //		given ID (if non-null). The object must be
    //		serializable as defined in KSerializer.
    //
   
    public void SendObjectToManager(Object TheObject, String ID)
    {
	// Serialize the object
	try
	{
	    String  TheSerial = KSerializer.Serialize(TheObject);
	    KNEvent TheEvent  = new KNEvent();
	    TheEvent.put("kn_payload", TheSerial);

	    if (ID != null)
	    {
		TheEvent.put("kn_id", ID);
	    }

	    coordRouter.publish(Common.StatusTopic, TheEvent, null);
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Agent", Ex);
	}
    }

    //
    //	Agent::Running -
    //
    //		Return true if child threads should keep running, or
    //		false otherwise.
    //

    public boolean Running()
    {
	return Running;
    }

    //
    // GetAgentID -
    //
    //		Return the ID of the agent.
    //

    public String GetAgentID()
    {
	return agentID;
    }

    //
    //	Agent::GetStatus -
    //
    //		Return the status of the Agent.
    //

    public String GetStatus()
    {
	return Status;
    }

    //
    //	Agent::SetStatus -
    //
    //		Set the status of the Agent.
    //

    public void SetStatus(String TheStatus)
    {
	Status = TheStatus;
    }

    //
    //	Agent::HandleExitCommand -
    //
    //		Process the given exit command.
    //

    private void HandleExitCommand(CommandExit command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    SetStatus("Exiting");
	    Log("Command: Exit");
	    Shutdown();
	}
    }

    //
    //	Agent::HandleDownloadCommand -
    //
    //		Process the given download command, and then
    //		return a StatusDownload object back to the 
    //		Manager.
    //

    private void HandleDownloadCommand(CommandDownload command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    boolean status = true;
	    String TheURL  = command.downloadURL;
	    String TheFile = command.localFile;

	    SetStatus(AgentData.STATUS_DOWNLOAD);
	    Log("Command: Download(" + TheURL + ") to " + TheFile);

	    try
	    {
		// Get the temporary directory, and compute full file name
		String ThePath = TempDir + TheFile;
		Log("About to download to " + ThePath);
		
		status = Common.DownloadFileFromURL(TheURL, ThePath, this);
		
		if (status)
		{
		    Log("File downloaded");
		}
		else
		{
		    Log("File not downloaded");
		}
	    }
	    catch (Exception Ex)
	    {
		Log("Exception during download");
		Common.DumpException("Agent", Ex);
		status = false;
	    }

	    SendObjectToManager(new StatusDownload(agentID, TheURL, status));
	    SetStatus(AgentData.STATUS_IDLE);
	}
    }

    //
    //	Agent::HandleRunCommand -
    //
    //		Process the given run command. Run it and then
    //		send a StatusRun object back to the Manager.
    //

    private void HandleRunCommand(CommandRun command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    boolean status     = true;
	    String  output     = new String();
	    String  TheCommand = command.command;

	    SetStatus(AgentData.STATUS_RUN);
	    Log("Command: Run(" + TheCommand + ")");

	    try
	    {
		output = Common.RunProcess(TheCommand);
		System.out.println("Returned:");
		System.out.println(output);
	    }
	    catch (Exception Ex)
	    {
		Log("Exception during run");
		Common.DumpException("Agent", Ex);
		output = "";
		status = false;
	    }

	    SendObjectToManager(new StatusRun(agentID, TheCommand, output, status));

	    SetStatus(AgentData.STATUS_IDLE);
	}
    }

    //
    //	Agent::HandleRouteCommand -
    //
    //		Process the given route command by creating
    //		the specified routes.
    //

    private void HandleRouteCommand(CommandRoute command)
    {
	if (command.agentID.equals(agentID))
	{
	    SetStatus(AgentData.STATUS_ROUTE);
	    String routerURI     = command.routerURI;
	    String sourceTopic   = command.sourceTopic;
	    String journalTopic  = command.journalTopic;
	    int    seqStart      = command.seqStart;
	    int    seqEnd        = command.seqEnd;
	    int    numPublishers = command.numPublishers;

	    Log("Command: Route(" +
		routerURI    + "," +
		sourceTopic  + "," +
		journalTopic + "," +
		seqStart     + "," +
		seqEnd       + "," +
		numPublishers + ")");

	    try
	    {
		boolean status =
		    Router.MakeRoutes(routerURI,
				      sourceTopic,
				      journalTopic,
				      seqStart,
				      seqEnd,
				      numPublishers,
				      this);

		System.out.println("Status: " + status);

		SendObjectToManager(new StatusRoute(agentID, routerURI, sourceTopic,
						    journalTopic, status));

	    }
	    catch (Exception Ex)
	    {
		Log("Exception during route");
		Common.DumpException("Agent", Ex);
	    }

	    SetStatus(AgentData.STATUS_IDLE);
	}
    }

    //
    //	Agent::HandlePublishCommand -
    //
    //		Process the given publish command by creating
    //		threads as specified, and then having each
    //		thread do the publishing. Return a StatusPublish
    //		object to the Manager on completion. The publisher
    //		threads report back to the Agent using the 
    //		PublisherReporter interface on the Agent.
    //

    private void HandlePublishCommand(CommandPublish command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    boolean status = true;

	    SetStatus(AgentData.STATUS_PUB);
	    String routerURI   = command.routerURI;
	    String topic       = command.topic;
	    int    threadCount = command.threadCount;
	    int    eventCount  = command.eventCount;
	    int    eventSize   = command.eventSize;
	    int    delay       = command.delay;

	    Log("Command: Publish(" +
		routerURI   + "," +
		topic       + "," +
		threadCount + "," +
		eventCount  + "," +
		eventSize   + "," +
		delay       + ")");

	    try
	    {
		// Reset count of successful publishes
		publisherSuccessCount = 0;

		// Create the threads
		ArrayList threadList = new ArrayList();

		for (int i = 0; i < threadCount; i++)
		{
		    PublisherThread publisherThread =
			new PublisherThread(routerURI, topic, eventSize,
					    eventCount, delay, this, this, i);

		    threadList.add(new Thread(publisherThread));
		}

		// Run the threads
		for (int i = 0; i < threadCount; i++)
		{
		    ((Thread) threadList.get(i)).start();
		}

		// Wait for the threads to complete
		for (int i = 0; i < threadCount; i++)
		{
		    ((Thread) threadList.get(i)).join();
		}

		// If all thread succeeded then the operation succeeded
		status = (threadCount == publisherSuccessCount);
	    }
	    catch (Exception Ex)
	    {
		Log("Exception during publish");
		Common.DumpException("Agent", Ex);
		status = false;
	    }

	    // Return status to Manager
	    SendObjectToManager(new StatusPublish(agentID, routerURI, topic, status));

	    Log("Publish complete");
	    SetStatus(AgentData.STATUS_IDLE);
	}
    }

    // 
    // PublisherReporter::ReportStatus -
    //
    //		Callback from the Publisher threads. Capture and store
    //		the status (success or failure) of each thread.
    //	

    public void ReportStatus(int index, boolean status)
    {
	// Count successful publishing operations
	if (status)
	{
	    publisherSuccessCount++;
	}
    }

    //
    //	Agent::HandleSubscribeCommand -
    //
    //		Process the given subscribe command by listening
    //		on the given journalTopic and arranging for the
    //		indicated counter to be incremented each time
    //		an event is received. Return a StatusSubcribe
    //		object to the Manager on completion.
    //

    private void HandleSubscribeCommand(CommandSubscribe command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    boolean status = true;

	    SetStatus(AgentData.STATUS_SUB);
	    String routerURI   = command.routerURI;
	    String topic       = command.topic;
	    String counterID   = command.counterID;

	    Log("Command: Subscribe(" +
		routerURI + "," +
		topic     + "," +
		counterID + ")");

	    // Do the subscription
	    try
	    {
		// Get a router for this routerURI
		KNJServer router  = RouterMap.getRouter(routerURI);

		// Subscribe
		KNOptions options = new KNOptions();
		KNRoute   route   = router.subscribe(topic, this, options, null);
	    }
	    catch (Exception Ex)
	    {
		Log("Exception during subscribe");
		Common.DumpException("Agent", Ex);
                status = false;
	    }

	    // Set up a counter
	    Counter counter = counterMap.GetCounter(topic, counterID);

	    // Return status to Manager
	    SendObjectToManager(new StatusSubscribe(agentID, routerURI, topic, status));

	    Log("Subscribe complete");
	    SetStatus(AgentData.STATUS_IDLE);
	}
    }

    //
    //	Agent::HandleResetCountersCommand -
    //
    //		Process the given reset counters command by
    //		resetting the counters. No status is returned.
    //

    private void HandleResetCountersCommand(CommandResetCounters command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    System.out.println("ResetCounters");
	    counterMap.ResetCounters();
	}
    }

    //
    //	Agent::HandleReportCountersCommand -
    //
    //		Process the given report counters command by
    //		returning a StatusCounter object to the Manager
    //		for each counter.
    //

    private void HandleReportCountersCommand(CommandReportCounters command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    System.out.println("ReportCounters");
	    Set counterSet              = counterMap.GetCounterKeySet();
	    Iterator counterSetIterator = counterSet.iterator();

	    // Return each counter to the Manager
	    while (counterSetIterator.hasNext())
	    {
		String  topic   = (String) counterSetIterator.next();
		Counter counter = counterMap.GetCounter(topic);

		SendObjectToManager(new StatusCounter(agentID, topic,
						      counter.GetCount()));
	    }
	}
    }

    //
    //	Agent::HandleDeleteCountersCommand -
    //
    //		Process the given delete counters command by
    //		deleting the counters. No status is returned.
    //

    private void HandleDeleteCountersCommand(CommandDeleteCounters command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    System.out.println("DeleteCounters");
	    counterMap.DeleteCounters();
	}
    }

    //
    //	Agent::HandleClearRouterCommand -
    //
    //		Process the given clear router command by
    //		clearing the router's persistent data.
    //		No status is returned.
    //

    private void HandleClearRouterCommand(CommandClearRouter command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    System.out.println("ClearRouter");
	    boolean status;

	    SetStatus(AgentData.STATUS_CLEAR);
	    status = router.Clear();
	    SendObjectToManager(new StatusClear(agentID, status));
	    SetStatus(AgentData.STATUS_IDLE);
	}
    }

    //
    //	Agent::HandleStartRouterCommand -
    //
    //		Process the given start router command by
    //		starting the router. No status is returned.
    //

    private void HandleStartRouterCommand(CommandStartRouter command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    System.out.println("StartRouter");
	    boolean status;

	    SetStatus(AgentData.STATUS_START);
	    status = router.Start();
	    SendObjectToManager(new StatusStart(agentID, status));
	    SetStatus(AgentData.STATUS_IDLE);
	}
    }

    //
    //	Agent::HandleStopRouterCommand -
    //
    //		Process the given stop router command by
    //		stopping the router. No status is returned.
    //

    private void HandleStopRouterCommand(CommandStopRouter command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    System.out.println("StopRouter");
	    boolean status;

	    SetStatus(AgentData.STATUS_STOP);
	    status = router.Stop();
	    SendObjectToManager(new StatusStop(agentID, status));
	    SetStatus(AgentData.STATUS_IDLE);
	}
    }

    //
    //	Agent::HandleConfigRouterCommand -
    //
    //		Process the given configure router command by
    //		configuring the router (downloading and then
    //		storing the given file). Return a StatusDownload
    //		object to the Manager.
    //

    private void HandleConfigRouterCommand(CommandConfigRouter command)
    {
	if (command.agentID.equals("") ||
	    command.agentID.equals(agentID))
	{
	    System.out.println("ConfigRouter");
	    SetStatus(AgentData.STATUS_CONFIG);

	    boolean status     = true;
	    String  configFile = new String();

	    // Download the configuration file
	    try
	    {
		configFile = TempDir + "knrouter.conf";
		status =
		    Common.DownloadFileFromURL(command.configURL, configFile, this);

		Log("About to download to " + configFile);

		if (status)
		{
		    Log("File downloaded");
		}
		else
		{
		    Log("File not downloaded");
		}
	    }
	    catch (Exception Ex)
	    {
		Log("Exception during download");
		Common.DumpException("Agent", Ex);
		status = false;
	    }

	    if (status)
	    {
		status = router.Config(configFile);
	    }

	    SendObjectToManager(new StatusConfig(agentID, command.configURL, status));
	    SetStatus(AgentData.STATUS_IDLE);
	}
    }

    //
    //	Agent::Log -
    //
    //		Time-stamp and then write the message to stderr.
    //

    public void Log(String Msg)
    {
	SimpleDateFormat MyFormat =
	    new SimpleDateFormat("dd-MMM-yyyy HH:mm:ss");
	String MyDate = MyFormat.format(new Date());

	System.err.println(MyDate + " " + Msg);
    }

    // KNEventListener::onEvent
    public void onEvent(KNEvent TheEvent)
    {
	String p = TheEvent.getPayload();
	
	if (p.startsWith(Common.PayloadChar))
	{
	    String routedFrom    = TheEvent.get("kn_routed_from");
	    Counter topicCounter = counterMap.GetCounterForRoute(routedFrom);

	    if (topicCounter != null)
	    {
		topicCounter.Increment();
		System.out.println("Increment counter: " + topicCounter.GetName());
	    }
	}
	else
	if (p.startsWith("object:"))
	{
	    try
	    {
		Object command   = KSerializer.Instantiate(p);
		String className = command.getClass().getName();

		if (className.equals("CommandExit"))
		{
		    HandleExitCommand((CommandExit) command);
		}
		else
		if (className.equals("CommandDownload"))
		{
		    HandleDownloadCommand((CommandDownload) command);
		}
		else
		if (className.equals("CommandRun"))
		{
		    HandleRunCommand((CommandRun) command);
		}
		else
		if (className.equals("CommandRoute"))
		{
		    HandleRouteCommand((CommandRoute) command);
		}
		else
		if (className.equals("CommandPublish"))
		{
		    HandlePublishCommand((CommandPublish) command);
		}
		else
		if (className.equals("CommandSubscribe"))
		{
		    HandleSubscribeCommand((CommandSubscribe) command);
		}
		else
		if (className.equals("CommandResetCounters"))
		{
		    HandleResetCountersCommand((CommandResetCounters) command);
		}
		else
		if (className.equals("CommandDeleteCounters"))
		{
		    HandleDeleteCountersCommand((CommandDeleteCounters) command);
		}
		else
		if (className.equals("CommandReportCounters"))
		{
		    HandleReportCountersCommand((CommandReportCounters) command);
		}
		else
		if (className.equals("CommandClearRouter"))
		{
		    HandleClearRouterCommand((CommandClearRouter) command);
		}
		else
		if (className.equals("CommandStartRouter"))
		{
		    HandleStartRouterCommand((CommandStartRouter) command);
		}
		else
		if (className.equals("CommandStopRouter"))
		{
		    HandleStopRouterCommand((CommandStopRouter) command);
		}
		else
		if (className.equals("CommandConfigRouter"))
		{
		    HandleConfigRouterCommand((CommandConfigRouter) command);
		}
		else
		{
		    Log("Agent: "        + agentID + ": " +
			"Unknown class " + className +
			" received & ignored");
		}
	    }
	    catch (Exception Ex)
	    {
		Common.DumpException("Agent", Ex);
	    }
	}
	else
	{
	    Log("onEvent: " + p);
	}
    }

    // KNStatusHandler::onStatusEvent
    public void onStatusEvent(KNEvent TheEvent)
    {
	Log("Agent: onStatusEvent: " + TheEvent.get("status"));
    }
}
