
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
 * $Id: AgentData.java,v 1.2 2004/04/19 05:39:09 bsittler Exp $
 *
 **/

//
// AgentData.java -
//
//	This is the Agent status information that is tracked
//	within the Manager. A single record is stored for
//	each Agent.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;
import java.util.*;

public class AgentData
{
    // Public definitions
    public static final String STATUS_IDLE     = "Idle";
    public static final String STATUS_DOWNLOAD = "Download";
    public static final String STATUS_RUN      = "Run";
    public static final String STATUS_CONFIG   = "Configure Router";
    public static final String STATUS_ROUTE    = "Route";
    public static final String STATUS_CLEAR    = "Clear";
    public static final String STATUS_SUB      = "Subscribe";
    public static final String STATUS_PUB      = "Publish";
    public static final String STATUS_EXIT     = "Exit";
    public static final String STATUS_START    = "Start Router";
    public static final String STATUS_STOP     = "Stop Router";

    // Instance variables
    public String	Name;
    public boolean	Changed;
    public boolean	Assigned;
    public Date		LastUpdate;
    public String	AgentID;
    public String	JavaRuntimeVersion;
    public String	SystemArchitecture;
    public String	OperatingSystem;
    public String	OperatingSystemVersion;
    public String	JavaHome;
    public String	Status;
    public String	Host;

    private Log		Logger;

    //
    // AgentData::AgentData -
    //
    //		Constructor.
    //

    public AgentData(String Name, String AgentID, String Host, Log Logger)
    { 
	// Zap the Changed flag
	Changed = false;

	// Set the Name, AgentID, Host, and the Logger
	this.Name    = Name;
	this.AgentID = AgentID;
	this.Logger  = Logger;
	this.Host    = Host;

	// Set the date of the last update
	LastUpdate = new Date();

	// Initialize the other fields
	JavaRuntimeVersion     = "";
	SystemArchitecture     = "";
	OperatingSystem        = "";
	OperatingSystemVersion = "";
	JavaHome               = "";
	Status                 = "";
	Assigned               = false;
    }

    //
    // AgentData::UpdateFromHeartbeat -
    //
    //	Update the object from the given HeartbeatData object.
    //

    public void UpdateFromHeartbeat(HeartbeatData h)
    {
	JavaRuntimeVersion     = h.JavaRuntimeVersion;
	SystemArchitecture     = h.SystemArchitecture;
	OperatingSystem        = h.OperatingSystem;
	OperatingSystemVersion = h.OperatingSystemVersion;
	JavaHome               = h.JavaHome;
	Status 		       = h.Status;
    }

    //
    //	AgentData::SetName -
    //
    //		Set the agent's name.
    //

    public void SetName(String name)
    {
	this.Name = name;
    }

    //
    //	AgentData::SetAssigned -
    //
    //		Set the agent's assigned flag.
    //

    public void SetAssigned(boolean assigned)
    {
	this.Assigned = assigned;
    }
}



