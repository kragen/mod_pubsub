
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
 * $Id: TestSpec.java,v 1.1 2003/08/15 23:41:07 ifindkarma Exp $
 *
 **/

//
// TestSpec.java -
//
//	Encode parsed information stored in a test specification
//	file.
//
//	The static Load method is used to parse a specification
//	and return an instance of the class.
//
//	The Run method is used to excute a TestSpec.
//
//	The syntax is as follows:
//
//		  # Comment
//			Lines beginning with '#' are comments and are ignored.
//
//		  agent AGENT_NAME ("OS" | *) [HOST]
//			Binds an actual agent (on a particular operating system
//			and optionally on a particular host) to an agent name. 
//			Operating system is any known operating system type:
//
//			Windows
//			Linux
//			Solaris
//
//		  download (AGENT_NAME | *) "URL" "FILE"
//			Downloads the information at the given URL into the
//			agent.
//
//		  run (AGENT_NAME | *) "COMMAND"
//			Run the given command on the agent.
//
//		  delete_counters (AGENT_NAME | *)
//			Delete the counters in the given agent or in all
//			agents.
//  
//		  reset_counters (AGENT_NAME | *)
//			Reset the counters in the given agent or in all
//			agents.
//
//		  send_counters (AGENT_NAME | *)
//			Get the counters from the given agent or from all
//			agents.
//
//		  clear_persistence (AGENT_NAME | *)
//			Clear the router persistence files in all agents
//			or the given agent.
//
//		  configure_router (AGENT_NAME | *) URL
//			Configure the router in all agents or the given
//			agent using the given URL.
//
//		  start_router (AGENT_NAME | *) 
//			Start the router on all agents or the given agent.
//
//		  stop_router (AGENT_NAME | *) 
//			Stop the router on all agents or the given agent.
//
//		  make_routes (AGENT_NAME | *) FROM_AGENT SRC DEST START END N
//			Create routes in all agents or the given agent
//			using the given parameters:
//			
//			FROM_AGENT is the agent to route from.
//			
//			SRC is the source topic.
//
//			DEST is the destination topic.
//
//			START is the starting sequence number.
//
//			END is the ending sequence number.
//
//			N is the number of publishers.
//
//		  subscribe AGENT_NAME TO_AGENT TOPIC COUNTER
//			Tell the agent to subscribe the given TO_AGENT to the given
//			TOPIC, and to tabulate incoming events in COUNTER.
//
//		  publish (AGENT_NAME | *) TO_AGENT THREADS DEST COUNT SIZE DELAY
//			Start publishing from all agents or the given agents
//			using the given parameters:
//
//			TO_AGENT is the agent to publish to.
//
//			THREADS is the number of threads to use.
//
//			DEST is the destination topic.
//
//			COUNT is the number of events per thread.
//
//			SIZE is the event payload size.
//
//			DELAY is the millisecond delay between events.
//
//		  wait_idle
//			Wait for all agents to become idle.
//
//		  sleep SECONDS
//			Sleep for the given number of seconds.
//
//		  get_counters (AGENT_NAME | *)
//			Retrieve the counters from the given agents or from
//			all agents.
//
//		  print MESSAGE
//			Print the given message.
//	
//	Copyright (c) 2002 - KnowNow, Inc.
//

import java.io.*;
import java.nio.*;
import java.lang.*;
import java.util.*;
import java.util.regex.*;

public class TestSpec
{
    // Class variables
    private static int	        lineNo       = 0;
    private static AgentDataMap agentDataMap = null;

    // Instance variables
    HashMap   agentMap = new HashMap();
    ArrayList stepList = new ArrayList();

    //
    // TestSpec::Load -
    //
    //		Factory method to load a configuration file, parse it, and
    //		return a TestSpec object. Raises an exception on loading and
    //		parsing errors. The AgentDataMap is used to assign real
    //		agents to symbolic agents.
    //

    public static TestSpec Load(String configFile, Log logger, AgentDataMap theAgentDataMap)
	throws Exception
    {
	// Create empty test specification
	TestSpec testSpec = new TestSpec();

	// Store AgentDataMap
	agentDataMap = theAgentDataMap;

	// Reset error count
	int errorCount = 0;

	//
	// Read and parse the configuration file, and store new items
	// in testSpec as needed:
	//

	try
	{
	    // Form strings that are components of patterns
	    String patBegin         = "^";
	    String patEnd           = "$";
	    String patAny           = ".*";
	    String patWhiteSpace    = "[\\t ]{1,}";
	    String patOptWhiteSpace = "[\\t ]{0,}";
	    String patAgentGroup    = "(\\*|[a-zA-Z0-9_]{1,})";
	    String patCommandGroup  = "\"(.*)\"";
	    String patSystemGroup   = "(\\*|\"[a-zA-Z0-9 ]{1,}\")";
            String patHostGroup     = "([a-zA-Z0-9_./]{1,})";
	    String patURLGroup      = "\"(http://[^ \\t]*)\"";
	    String patMessageGroup  = "\"(.*)\"";
	    String patTopicGroup    = "(/[a-zA-Z0-9/]{1,})";
	    String patCounterGroup  = "([a-zA-Z0-9./]{1,})";
	    String patIntGroup      = "([0-9]{1,})";
	    String patFileGroup     = "\"(.*)\"";

	    // Form patterns 
	    Pattern emptyPattern   =
		Pattern.compile(patBegin + patOptWhiteSpace + patEnd);

	    Pattern commentPattern =
		Pattern.compile(patBegin + patOptWhiteSpace + "#" + patAny + patEnd);

	    Pattern runPattern =
		Pattern.compile(patBegin        + patOptWhiteSpace +
				"run"           + patWhiteSpace    +
				patAgentGroup   + patWhiteSpace    +
				patCommandGroup + patOptWhiteSpace +
				patEnd);

	    Pattern agentPattern =
		Pattern.compile(patBegin       + patOptWhiteSpace +
				"agent"        + patWhiteSpace    +
				patAgentGroup  + patWhiteSpace    +
				patSystemGroup + patWhiteSpace    + 
				patHostGroup   + patOptWhiteSpace +
				patEnd);

	    Pattern agentNoHostPattern =
		Pattern.compile(patBegin       + patOptWhiteSpace +
				"agent"        + patWhiteSpace    +
				patAgentGroup  + patWhiteSpace    +
				patSystemGroup + patOptWhiteSpace + 
				patEnd);

	    Pattern downloadPattern = 
		Pattern.compile(patBegin      + patOptWhiteSpace +
				"download"    + patWhiteSpace    +
				patAgentGroup + patWhiteSpace    +
				patURLGroup   + patWhiteSpace    +
				patFileGroup  + patOptWhiteSpace +
				patEnd);
				
	    Pattern deleteCountersPattern = 
		Pattern.compile(patBegin          + patOptWhiteSpace +
				"delete_counters" + patWhiteSpace    +
				patAgentGroup     + patOptWhiteSpace +
				patEnd);

	    Pattern resetCountersPattern = 
		Pattern.compile(patBegin         + patOptWhiteSpace +
				"reset_counters" + patWhiteSpace    +
				patAgentGroup    + patOptWhiteSpace +
				patEnd);

	    Pattern getCountersPattern = 
		Pattern.compile(patBegin       + patOptWhiteSpace +
				"get_counters" + patWhiteSpace    +
				patAgentGroup  + patOptWhiteSpace +
				patEnd);

	    Pattern clearPersistencePattern = 
		Pattern.compile(patBegin            + patOptWhiteSpace +
				"clear_persistence" + patWhiteSpace    +
				patAgentGroup       + patOptWhiteSpace +
				patEnd);

	    Pattern configureRouterPattern = 
		Pattern.compile(patBegin           + patOptWhiteSpace +
				"configure_router" + patWhiteSpace    +
				patAgentGroup      + patWhiteSpace    +
				patURLGroup        + patOptWhiteSpace +
				patEnd);
				
	    Pattern startRouterPattern = 
		Pattern.compile(patBegin       + patOptWhiteSpace +
				"start_router" + patWhiteSpace    +
				patAgentGroup  + patOptWhiteSpace +
				patEnd);

	    Pattern stopRouterPattern = 
		Pattern.compile(patBegin      + patOptWhiteSpace +
				"stop_router" + patWhiteSpace    +
				patAgentGroup + patOptWhiteSpace +
				patEnd);

	    Pattern waitIdlePattern = 
		Pattern.compile(patBegin      + patOptWhiteSpace +
				"wait_idle"   + patOptWhiteSpace +
				patEnd);

	    Pattern printPattern = 
		Pattern.compile(patBegin        + patOptWhiteSpace +
				"print"         + patWhiteSpace    +
				patMessageGroup + patOptWhiteSpace +
				patEnd);

	    Pattern makeRoutesPattern =
		Pattern.compile(patBegin       + patOptWhiteSpace +
				"make_routes"  + patWhiteSpace    +
				patAgentGroup  + patWhiteSpace    +
				patURLGroup    + patWhiteSpace    +
				patTopicGroup  + patWhiteSpace    +
				patTopicGroup  + patWhiteSpace    +
				patIntGroup    + patWhiteSpace    +
				patIntGroup    + patWhiteSpace    +
				patIntGroup    + patOptWhiteSpace +
				patEnd);

	    Pattern publishPattern =
		Pattern.compile(patBegin       + patOptWhiteSpace +
				"publish"      + patWhiteSpace    +
				patAgentGroup  + patWhiteSpace    +
				patURLGroup    + patWhiteSpace    +
				patIntGroup    + patWhiteSpace    +
				patTopicGroup  + patWhiteSpace    +
				patIntGroup    + patWhiteSpace    +
				patIntGroup    + patWhiteSpace    +
				patIntGroup    + patOptWhiteSpace +
				patEnd);

	    Pattern subscribePattern =
		Pattern.compile(patBegin       + patOptWhiteSpace  +
				"subscribe"    + patWhiteSpace     +
				patAgentGroup  + patWhiteSpace     +
				patURLGroup    + patWhiteSpace     +
				patTopicGroup  + patWhiteSpace     +
				patCounterGroup + patOptWhiteSpace +
				patEnd);
				
	    Pattern sleepPattern = 
		Pattern.compile(patBegin    + patOptWhiteSpace +
				"sleep"     + patWhiteSpace    +
				patIntGroup + patOptWhiteSpace +
				patEnd);

	    // Open and read
	    BufferedReader reader = new BufferedReader(new FileReader(configFile));
	    String         line   = "";
	    
	    lineNo = 0;
	    while ((line = reader.readLine()) != null)
	    {
		lineNo++;

		// Use a CharBuffer for efficiency
		CharBuffer buffer = CharBuffer.wrap(line.toCharArray());

		// Handle empty lines
		Matcher emptyMatcher = emptyPattern.matcher(buffer);
		if (emptyMatcher.matches())
		{
		    continue;
		}

		// Handle comments
		Matcher    commentMatcher = commentPattern.matcher(buffer);
		if (commentMatcher.matches())
		{
		    continue;
		}
		
		// Handle agent command (two variants)
		Matcher agentMatcher = agentPattern.matcher(buffer);
		if (agentMatcher.matches())
		{
		    String name   = agentMatcher.group(1);
		    String system = Common.StripQuotes(agentMatcher.group(2));
		    String host   = agentMatcher.group(3);

		    // Do the assignment
		    AgentData agent = null;

		    if (system.equals("*"))
		    {
			agent = theAgentDataMap.AssignAgent(name);
		    }
		    else
		    {
			agent = theAgentDataMap.AssignAgentByHostSystem(name, host, system);
		    }

		    if (agent == null)
		    {
			System.out.println("Manager: Could not assign agent " + name);
			errorCount++;
			continue;
		    }

		    if (!testSpec.AddAgent(agent, name, system, host))
		    {
			System.out.println("Manager: Agent " + name + " already declared.");
			errorCount++;
			continue;
		    }
		    continue;
		}

		Matcher agentNoHostMatcher = agentNoHostPattern.matcher(buffer);
		if (agentNoHostMatcher.matches())
		{
		    String name   = agentNoHostMatcher.group(1);
		    String system = Common.StripQuotes(agentNoHostMatcher.group(2));

		    // Do the assignment
		    AgentData agent = null;

		    if (system.equals("*"))
		    {
			agent = theAgentDataMap.AssignAgent(name);
		    }
		    else
		    {
			agent = theAgentDataMap.AssignAgentBySystem(name, system);
		    }

		    if (agent == null)
		    {
			System.out.println("Manager: Could not assign agent " + name);
			errorCount++;
			continue;
		    }

		    if (!testSpec.AddAgent(agent, name, "", ""))
		    {
			System.out.println("Manager: Agent " + name + " already declared.");
			errorCount++;
		    }
		    continue;
		}

		// Handle download command
		Matcher downloadMatcher = downloadPattern.matcher(buffer);
		if (downloadMatcher.matches())
		{
		    // Get arguments
		    String agent = downloadMatcher.group(1);
		    String url   = downloadMatcher.group(2);
		    String file  = downloadMatcher.group(3);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);
		    
		    if (agentID != null)
		    {
			// Store command
			testSpec.AddCommand(new CommandDownload(agentID, url, file));
		    }
		    continue;
		}

		// Handle delete_counters command
		Matcher deleteCountersMatcher = deleteCountersPattern.matcher(buffer);
		if (deleteCountersMatcher.matches())
		{
		    // Get arguments
		    String agent = deleteCountersMatcher.group(1);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			// Store command
			testSpec.AddCommand(new CommandDeleteCounters(agentID));
		    }
		    continue;
		}

		// Handle clear_counters command
		Matcher resetCountersMatcher = resetCountersPattern.matcher(buffer);
		if (resetCountersMatcher.matches())
		{
		    // Get arguments
		    String agent = resetCountersMatcher.group(1);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			// Store command
			testSpec.AddCommand(new CommandResetCounters(agentID));
		    }
		    continue;
		}

		// Handle send_counters command
		Matcher getCountersMatcher = getCountersPattern.matcher(buffer);
		if (getCountersMatcher.matches())
		{
		    // Get arguments
		    String agent = getCountersMatcher.group(1);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			// Store command
			testSpec.AddCommand(new CommandReportCounters(agentID));
		    }
		    continue;
		}

		// Handle clear_persistence command
		Matcher clearPersistenceMatcher = clearPersistencePattern.matcher(buffer);
		if (clearPersistenceMatcher.matches())
		{
		    String agent = clearPersistenceMatcher.group(1);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			// Store command
			testSpec.AddCommand(new CommandClearRouter(agentID));
		    }
		    continue;
		}

		// Handle configure_router command
		Matcher configureRouterMatcher = configureRouterPattern.matcher(buffer);
		if (configureRouterMatcher.matches())
		{
		    // Get arguments
		    String agent = configureRouterMatcher.group(1);
		    String url   = configureRouterMatcher.group(2);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			// Store command
			testSpec.AddCommand(new CommandConfigRouter(agentID, url));
		    }
		    continue;
		}

		// Handle start_router command
		Matcher startRouterMatcher = startRouterPattern.matcher(buffer);
		if (startRouterMatcher.matches())
		{
		    // Get arguments
		    String agent = startRouterMatcher.group(1);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			// Store command
			testSpec.AddCommand(new CommandStartRouter(agentID));
		    }
		    continue;
		}

		// Handle stop_router command
		Matcher stopRouterMatcher = stopRouterPattern.matcher(buffer);
		if (stopRouterMatcher.matches())
		{
		    // Get arguments
		    String agent = stopRouterMatcher.group(1);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			// Store command
			testSpec.AddCommand(new CommandStopRouter(agentID));
		    }
		    continue;
		}

		// Handle run command
		Matcher runMatcher = runPattern.matcher(buffer);
		if (runMatcher.matches())
		{
		    // Get arguments
		    String agent   = runMatcher.group(1);
		    String command = runMatcher.group(2);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			// Store command
			testSpec.AddCommand(new CommandRun(agent, command));
		    }
		    continue;
		}

		// Handle wait_idle command
		Matcher waitIdleMatcher = waitIdlePattern.matcher(buffer);
		if (waitIdleMatcher.matches())
		{
		    // Store command
		    testSpec.AddCommand(new CommandWaitIdle());
		    continue;
		}

		// Handle print command
		Matcher printMatcher = printPattern.matcher(buffer);
		if (printMatcher.matches())
		{
		    // Get arguments
		    String message = printMatcher.group(1);

		    // Store command
		    testSpec.AddCommand(new CommandPrint(message));
		    continue;
		}

		// Handle make_routes command
		Matcher makeRoutesMatcher = makeRoutesPattern.matcher(buffer);
		if (makeRoutesMatcher.matches())
		{
		    // Get arguments
		    String agent       = makeRoutesMatcher.group(1);
		    String routerAgent = Common.StripQuotes(makeRoutesMatcher.group(2));
		    String srcTopic    = makeRoutesMatcher.group(3);
		    String destTopic   = makeRoutesMatcher.group(4);
		    String start       = makeRoutesMatcher.group(5);
		    String end	       = makeRoutesMatcher.group(6);
		    String n	       = makeRoutesMatcher.group(7);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			// NEED CODE : MAP routerAgent to routerURI
			String routerURI = routerAgent;

			// Store command
			testSpec.AddCommand(new CommandRoute(agentID, routerURI,
							     srcTopic, destTopic,
							     new Integer(start).intValue(),
							     new Integer(end).intValue(),
							     new Integer(n).intValue()));
		    }
		    continue;
		}

		// Handle subscribe command
		Matcher subscribeMatcher = subscribePattern.matcher(buffer);
		if (subscribeMatcher.matches())
		{
		    // Get arguments
		    String agent       = subscribeMatcher.group(1);
		    String routerAgent = Common.StripQuotes(subscribeMatcher.group(2));
		    String topic       = subscribeMatcher.group(3);
		    String counter     = subscribeMatcher.group(4);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			String routerURI = routerAgent;
			// NEED CODE -- MAP routerAgent to routerURI

			// Store command
			testSpec.AddCommand(new CommandSubscribe(agentID,
								 routerURI,
								 topic,
								 counter));
		    }	
		    continue;
		}

		// Handle publish command
		Matcher publishMatcher = publishPattern.matcher(buffer);
		if (publishMatcher.matches())
		{
		    // Get arguments
		    String agent       = publishMatcher.group(1);
		    String routerAgent = Common.StripQuotes(publishMatcher.group(2));
		    String threads     = publishMatcher.group(3);
		    String destTopic   = publishMatcher.group(4);
		    String count       = publishMatcher.group(5);
		    String size	       = publishMatcher.group(6);
		    String delay       = publishMatcher.group(7);

		    // Map arguments
		    String agentID = MapAgentNameToAgentID(agent);

		    if (agentID != null)
		    {
			String routerURI = routerAgent;
			// NEED CODE -- MAP routerAgent to routerURI
			// Store command
			testSpec.AddCommand(new CommandPublish(agentID,
							       routerURI,
							       new Integer(threads).intValue(),
							       destTopic,
							       new Integer(count).intValue(),
							       new Integer(size).intValue(),
							       new Integer(delay).intValue()));
		    }
		    continue;
		}

		// Handle sleep command
		Matcher sleepMatcher = sleepPattern.matcher(buffer);
		if (sleepMatcher.matches())
		{
		    // Get arguments
		    String seconds = sleepMatcher.group(1);

		    // Store command
		    testSpec.AddCommand(new CommandSleep(new Integer(seconds).intValue()));
		    continue;
		}

		errorCount++;
		System.out.println("Syntax error on line " + lineNo);
		continue;
	    }

	    // testSpec.Dump();

	    return testSpec;
	}
	catch (FileNotFoundException Ex)
	{
	    logger.Log("Could not find config file " + configFile);
	    throw Ex;
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Manager", Ex);
	    throw Ex;
	}
    }

    //
    //	TestSpec::GetStepCount -
    //
    //		Return the number of test steps.
    //

    public int GetStepCount()
    {
	return stepList.size();
    }
    
    //
    // TestSpec::DumpGroups -
    //
    //		Debugging function to list all of the groups
    //		in the given Matcher.
    //

    private static void DumpGroups(Matcher matcher)
    {
	int groupCount = matcher.groupCount();

	System.out.println("There are " + groupCount + " matches:");
	for (int i = 1; i <= groupCount; i++)
	{
	    String match = matcher.group(i);
	    System.out.println("  [" + i + "]: " + "'" + match  + "'");
	}
    }

    //
    //	TestSpec::AddAgent -
    //
    //		Add the given Agent. Returns true on success,
    //		false on error.
    //

    private boolean AddAgent(AgentData	agentData,
			     String	agentName,
			     String	system,
			     String	host)
    {
	// Make sure that the agent is not already known
	if (agentMap.containsKey(agentName))
	{
	    return false;
	}

	// Add the agent to the table
	agentMap.put(agentName, new AgentSpec(agentData, agentName, system, host));

	return true;
    }

    //
    // TestSpec::AddCommand -
    //
    //		Add the given command to the spec. No return value.
    //

    private void AddCommand(Object command)
    {
	stepList.add(command);
    }

    //
    //	MapAgentNameToAgentID -
    //
    //		Map the given Agent name to an AgentID:
    //
    //		If the name is "*" the AgentID is "".
    //		Otherwise, look up the name in the AgentDataMap.
    //
    //		Return the AgentID on success. Issue an error
    //		message,bump the error count, and return null
    //		on failure.

    private static String MapAgentNameToAgentID(String name)
    {
	if (name.equals("*"))
	{
	    return new String(" ");
	}
	else
	{
	    AgentData agentData = agentDataMap.GetAgentByName(name);

	    if (agentData != null)
	    {
		return agentData.AgentID;
	    }
	    else
	    {
		System.out.println("Unknown agent " + name + " on line " + lineNo);
		return null;
	    }
	}
    }

    // 
    // TestSpec::Run -
    //
    //		Execute the test specification by issuing commands one 
    //		after the other until getting to a wait_idle command. All
    //		commands are sent to the appropriate agent or agents
    //		except for the following:
    //
    //		wait_idle
    //		print
    //		sleep
    //
    //		Most of the command returns status information; this method
    //		does not track or interpret it. 
    //
    //		Send the commands through the given Manager.
    //
    //		Unfortunately it is necessary to look at the type of each
    //		command in order to distinguish Manager-side and Agent-side
    //		commands.
    //

    public void Run(Manager manager)
    {
	Class printClass    = null;
	Class waitIdleClass = null;
	Class sleepClass    = null;

	try
	{
	    // Get the class objects
	    printClass    = Class.forName("CommandPrint");
	    waitIdleClass = Class.forName("CommandWaitIdle");
	    sleepClass    = Class.forName("CommandSleep");
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Manager", Ex);
	    return;
	}

	int count = GetStepCount();
	for (int i = 0; i < count; i++)
	{
	    // Get the current step
	    Object step     = stepList.get(i);
	    Class stepClass = step.getClass();	    
	    
	    // Handle wait_idle, print, and sleep locally
	    if (stepClass == printClass)
	    {
		System.out.println(((CommandPrint) step).GetMessage());
	    }
	    else if (stepClass == waitIdleClass)
	    {
		agentDataMap.AwaitIdle();
	    }	
	    else if (stepClass == sleepClass)
	    {
		Common.Sleep(((CommandSleep) step).GetSeconds());
	    }	
	    else
	    {
		manager.SendObjectToAgents(step);
	    }
	}
    }

    //
    // TestSpec::Dump -
    //
    //		Print out the items in the spec.
    //

    private void Dump()
    {	
	System.out.println("Agents");
	System.out.println("======");

	Set agentSet              = agentMap.keySet();
	Iterator agentSetIterator = agentSet.iterator();

	while (agentSetIterator.hasNext())
	{
	    String agentName    = (String) agentSetIterator.next();
	    AgentSpec agentSpec = (AgentSpec) agentMap.get(agentName);
	    AgentData agentData = agentSpec.GetAgentData();
	    
	    System.out.println("[" + agentName + "]: " + agentData.AgentID);
	}
	System.out.println();

	System.out.println("Test Steps");
	System.out.println("==========");

	for (int i = 0; i < stepList.size(); i++)
	{
	    Object step      = stepList.get(i);
	    Class  stepClass = step.getClass();
	    String className = stepClass.getName();

	    System.out.println("[" + i + "]: " + className);
	}
   }
}

