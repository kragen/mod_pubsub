
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
 * $Id: HeartbeatData.java,v 1.1 2003/08/15 23:41:06 ifindkarma Exp $
 *
 **/

//
// HeartbeatData.java -
//
//	This is the package of data that is periodically sent
//	from each Agent to the Manager to keep the Manager
//	apprised of the state of the Agent.
//
//	Note: All fields in this object must have types that can
//	      be serialized by KSerializer.
//
//	Copyright (c) 2002 - KnowNow, Inc.
//

import java.lang.*;

public class HeartbeatData
{
    public String	AgentID;
    public String	JavaRuntimeVersion;
    public String	SystemArchitecture;
    public String	OperatingSystem;
    public String	OperatingSystemVersion;
    public String	JavaHome;
    public String	Status;
    public String	Host;

    //
    // HeartbeatData::HeartbeatData -
    //
    //		Constructor.
    //

    public HeartbeatData()
    {
	AgentID		       = "";
	JavaRuntimeVersion     = "";
	SystemArchitecture     = "";
	OperatingSystem        = "";
	OperatingSystemVersion = "";
	JavaHome               = "";
	Status		       = "";
	Host		       = "";
    }

    //
    //	HeartbeatData::Setup -
    //
    //		Fill in the object based on system properties,
    //		other sources, and the given parameters.
    //

    public void Setup(String AgentID, String Status, String Host)
    {
	this.AgentID = AgentID;

	try
	{
	    JavaRuntimeVersion     = System.getProperty("java.version");
	    SystemArchitecture     = System.getProperty("os.arch");
	    OperatingSystem        = System.getProperty("os.name");
	    OperatingSystemVersion = System.getProperty("os.version");
	    JavaHome               = System.getProperty("java.home");
	    this.Status            = Status;
	    this.Host		   = Host;
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Agent", Ex);
	}
    }
}
