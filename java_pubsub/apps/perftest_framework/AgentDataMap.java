
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
 * $Id: AgentDataMap.java,v 1.2 2004/04/19 05:39:09 bsittler Exp $
 *
 **/

//
// AgentDataMap.java -
//
//	This class tracks the state of any number of Agents.
//	Each Agent is represented by an AgentData object. This
//	object can be interrogated to discover the status
//	of each agent, and the entire map can be printed.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;
import java.util.*;
import java.io.PrintStream;

public class AgentDataMap
{
    // Instance variables
    private HashMap	agentMap;
    private int		longestAgentID;
    private int		longestName;
    private Log		Logger;

    // Private definitions
    private static final int AGENT_STATUS_WIDTH = 8;

    //
    // AgentDataMap::AgentDataMap -
    //
    //		Constructor.
    //

    public AgentDataMap(Log Logger)
    {
	this.Logger    = Logger;
	agentMap       = new HashMap();
	longestAgentID = 0;
	longestName    = 0;
    }

    //
    //	AgentDataMap::UpdateAgentData -
    //
    //		Update the AgentMap to reflect the newest information
    //		from the given HeartbeatData.
    //

    public void UpdateAgentData(HeartbeatData heartbeatData)
    {
	String agentID = heartbeatData.AgentID;

	// First, see if agent is already known. Create it if not
	if (!agentMap.containsKey(agentID))
	{
	    AgentData agentData = new AgentData("", agentID, heartbeatData.Host, Logger);
	    agentData.Changed   = true;

	    agentMap.put(agentID, agentData);

	    if (Logger != null)
	    {
		Logger.Log("New Agent found: " + agentID);
	    }

	    // Track the longest agent ID and name
	    if (agentID.length() > longestAgentID)
	    {
		longestAgentID = agentID.length();
	    }

	    if (agentData.Name.length() > longestName)
	    {
		longestName = agentData.Name.length();
	    }
	}

	// Now get the agent
	AgentData agentData = (AgentData) agentMap.get(agentID);

	// Report on any changes
	if (Logger != null)
	{
	    if (!heartbeatData.Status.equals(agentData.Status))
	    {
		agentData.Changed = true;

		Logger.Log("Agent " +
			   agentData.AgentID +
			   ": " +
			   "Status change from " +
			   agentData.Status +
			   " to " +
			   heartbeatData.Status);
	    }
	}

	// And update it
	agentData.UpdateFromHeartbeat(heartbeatData);
    }

    //
    // AgentDataMap::keySet -
    //
    //		Return a Set for iteration over the contents
    //		of the AgentDataMap.
    //

    public Set keySet()
    {
	return agentMap.keySet();
    }

    //
    //	AgentDataMap::get -
    //
    //		Return the agent with the given AgentID.
    //

    public AgentData get(String agentID)
    {
	return (AgentData) agentMap.get(agentID);
    }

    //
    // AgentDataMap::GetAgentCount -
    //
    //		Return the number of agents in the map.
    //

    public int GetAgentCount()
    {
	return agentMap.size();
    }

    //
    // AgentDataMap::GetIdleAgentCount -
    //
    //		Return the number of idle agents in the map.
    //

    public int GetIdleAgentCount()
    {
	int idleAgentCount        = 0;
	Set agentSet              = agentMap.keySet();
	Iterator agentSetIterator = agentSet.iterator();

	while (agentSetIterator.hasNext())
	{
	    // Get the iterator key and then the AgentData
	    String agentID      = (String) agentSetIterator.next();
	    AgentData agentData = (AgentData) agentMap.get(agentID);
	    
	    if (agentData.Status.equals(AgentData.STATUS_IDLE))
	    {
		idleAgentCount++;
	    }
	}

	return idleAgentCount;
    }

    //
    //	AgentDataMap::AwaitIdle -
    //
    //		Wait until all of the agents in the map are idle,
    //		then return. Return true if all of the agents
    //		became idle, or false if the wait was interrupted.
    //

    public boolean AwaitIdle()
    {
	// First, wait 2 seconds to let Agents transition out of idle
	try
	{
	    Thread.currentThread().sleep(2000);
	}
	catch (InterruptedException Ex)
	{
	    return false;
	}
	
	// Then wait for them to return to idle
	while (GetIdleAgentCount() != GetAgentCount())
	{
	    try
	    {
		Thread.currentThread().sleep(500);
	    }
	    catch (InterruptedException Ex)
	    {
		return false;
	    }
	}

	return true;
    }

    //
    // AgentDataMap::ClearChanged -
    //
    //		Reset the Changed flag for each AgentData object.
    //

    public void ClearChanged()
    {
	Set agentSet              = agentMap.keySet();
	Iterator agentSetIterator = agentSet.iterator();

	while (agentSetIterator.hasNext())
	{
	    // Get the iterator key and then the AgentData
	    String agentID      = (String) agentSetIterator.next();
	    AgentData agentData = (AgentData) agentMap.get(agentID);

	    agentData.Changed = false;
	}
    }

    //
    // AgentDataMap::Print -
    //
    //		Print the data map in a clean and readable format.
    //

    public void Print()
    {
	Set agentSet              = agentMap.keySet();
	Iterator agentSetIterator = agentSet.iterator();

	// Print the header
	System.out.print("Name");
	System.out.print(Common.StringRepeat(longestName - 4, " "));
	System.out.print("Agent ID");
	System.out.print(Common.StringRepeat(longestAgentID - 8, " "));
	System.out.print(" ");
	System.out.print("Status");
	System.out.println();
	System.out.print(Common.StringRepeat(longestAgentID, "="));
	System.out.print(" ");
	System.out.print(Common.StringRepeat(AGENT_STATUS_WIDTH, "="));
	System.out.println();

	// Print each agent
	while (agentSetIterator.hasNext())
	{
	    // Get the iterator key and then the AgentData
	    String agentID      = (String) agentSetIterator.next();
	    AgentData agentData = (AgentData) agentMap.get(agentID);

	    // Print Name, padded with spaces to the length of the
	    // longest Name
	    System.out.print(agentData.Name);
	    System.out.print(Common.StringRepeat(longestName -
						 agentData.Name.length(),
						 " "));
	    System.out.print(" ");

	    // Print AgentID, padded with spaces to the length of the
	    // longest ID
	    System.out.print(agentData.AgentID);
	    System.out.print(Common.StringRepeat(longestAgentID -
						 agentData.AgentID.length(),
						 " "));
	    System.out.print(" ");
	    
	    // Print Status, padded to AGENT_STATUS_WIDTH
	    System.out.print(agentData.Status);
	    System.out.print(Common.StringRepeat(AGENT_STATUS_WIDTH -
						 agentData.Status.length(),
						 " "));
	    System.out.println();
	}
    }

    //
    // AgentDataMap::PrintHTML -
    //
    //		Print the data map to the given PrintStream
    //		as an HTML table. If FlagChanged is true,
    //		then use a distinctive style for changed
    //		any Agent that has changed.
    //

    public void PrintHTML(PrintStream htmlStream, String TableAttributes,
			  boolean FlagChanged)
    {
	Set agentSet              = agentMap.keySet();
	Iterator agentSetIterator = agentSet.iterator();

	// Open the table
	htmlStream.println("<table + " + TableAttributes + ">");

	// Print the header
	htmlStream.println("<tr>");
	htmlStream.println("<td class=\"header_cell\">Agent Name</td>");
	htmlStream.println("<td class=\"header_cell\">Agent ID</td>");
	htmlStream.println("<td class=\"header_cell\">OS/Platform</td>");
	htmlStream.println("<td class=\"header_cell\">Status</td>");
	htmlStream.println("</tr>");

	// Print each agent
	String CellStyle = "";
	while (agentSetIterator.hasNext())
	{
	    htmlStream.println("<tr>");
	    // Get the iterator key and then the AgentData
	    String agentID      = (String) agentSetIterator.next();
	    AgentData agentData = (AgentData) agentMap.get(agentID);

	    // Decide on cell style to use
	    if (FlagChanged)
	    {
		if (agentData.Changed)
		{
		    CellStyle = "class=\"changed_cell\"";
		}
		else
		{
		    CellStyle = "class=\"normal_cell\"";
		}
	    }
	    else
	    {
		CellStyle = "class=\"normal_cell\"";
	    }

	    // Print Name, AgentID, Platform, and Status
	    htmlStream.print("<td " + CellStyle + ">" +
			     agentData.Name + "</td>");

	    htmlStream.print("<td " + CellStyle + ">" +
			     agentData.AgentID + "</td>");

	    htmlStream.print("<td " + CellStyle + ">" +
			     agentData.OperatingSystem + "/" + 
			     agentData.SystemArchitecture + "</td>");

	    htmlStream.print("<td " + CellStyle + ">" +
			     agentData.Status + "</td>");

	    htmlStream.println("</tr>");
	}

	// Close the table
	htmlStream.println("</table>");
    }

    //
    //	AgentDataMap::AssignAgent -
    //
    //		Look for an unassigned agent on the given host.
    //		If one is found, mark it as assigned and set
    //		its name. Return the agent on success, or null
    //		on failure.
    //

    public AgentData AssignAgent(String name)
    {
	Set agentSet              = agentMap.keySet();
	Iterator agentSetIterator = agentSet.iterator();

	// Check each Agent
	while (agentSetIterator.hasNext())
	{
	    String agentID      = (String) agentSetIterator.next();
	    AgentData agentData = (AgentData) agentMap.get(agentID);

	    if (!agentData.Assigned)
	    {
		agentData.SetAssigned(true);
		agentData.SetName(name);
		return agentData;
	    }
	}

	return null;
    }

    //
    //	AgentDataMap::AssignAgentByHostSystem -
    //
    //		Look for an unassigned agent on the given host
    //		and operating system. If one is found, mark it as 
    //		assigned and set its name. Return the agent on 
    //		success, or null on failure.
    //

    public AgentData AssignAgentByHostSystem(String name,
					     String host,
					     String operatingSystem)
    {
	Set agentSet              = agentMap.keySet();
	Iterator agentSetIterator = agentSet.iterator();

	// Check each Agent
	while (agentSetIterator.hasNext())
	{
	    String agentID      = (String) agentSetIterator.next();
	    AgentData agentData = (AgentData) agentMap.get(agentID);

	    if (!agentData.Assigned)
	    {
		if (agentData.OperatingSystem.startsWith(operatingSystem) &&
		    agentData.Host.equals(host))
		{
		    agentData.SetAssigned(true);
		    agentData.SetName(name);
		    return agentData;
		}
	    }
	}

	return null;
    }

    //
    //	AgentDataMap::AssignAgentBySystem -
    //
    //		Look for an unassigned agent on a host running the 
    //		given operating system.	If one is found, mark it
    //		as assigned and set its	name. Return the agent on
    //		success, or null on failure.
    //

    public AgentData AssignAgentBySystem(String name, String operatingSystem)
    {
	Set agentSet              = agentMap.keySet();
	Iterator agentSetIterator = agentSet.iterator();

	// Check each Agent
	while (agentSetIterator.hasNext())
	{
	    String agentID      = (String) agentSetIterator.next();
	    AgentData agentData = (AgentData) agentMap.get(agentID);

	    if (!agentData.Assigned)
	    {
		if (agentData.OperatingSystem.startsWith(operatingSystem))
		{
		    System.out.println("Assigning " + name + " to " + agentData.AgentID);
		    agentData.SetAssigned(true);
		    agentData.SetName(name);
		    return agentData;
		}
	    }
	}

	return null;
    }

    //
    //	AgentDataMap::GetAgentByName -
    //
    //		Return the AgentData of the agent with the given
    //		name on success, null on error.
    //	

    public AgentData GetAgentByName(String name)
    {
	Set agentSet              = agentMap.keySet();
	Iterator agentSetIterator = agentSet.iterator();

	// Check each Agent
	while (agentSetIterator.hasNext())
	{
	    String agentID      = (String) agentSetIterator.next();
	    AgentData agentData = (AgentData) agentMap.get(agentID);

	    if (agentData.Name.equals(name))
	    {
		return agentData;
	    }
	}

	return null;
    }
}
