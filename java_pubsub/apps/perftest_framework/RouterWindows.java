
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
 * $Id: RouterWindows.java,v 1.2 2004/04/19 05:39:10 bsittler Exp $
 *
 **/

//
// RouterWindows.java -
//
//	Windows implementation of code to perform various operations
//	on a router. 
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;

public class RouterWindows extends Router
{
    //
    //	Router::GetPersistenceDirectory -
    //
    //		Virtual method to return the name of the directory
    //		used to store the router's persistence files when
    //		a standard install of the router is present.
    //

    public String GetPersistenceDirectory()
    {
	return "C:\\Program Files\\KnowNow\\Event_Router\\runtime\\knrouter.prs_db\\";
    }

    //
    //	Router::GetPersistenceFile -
    //
    //		Virtual method to return the name of the file used to
    //          store the router's persistence information when
    //		a standard install of the router is present.
    //

    public String GetPersistenceFile()
    {
	return "C:\\Program Files\\KnowNow\\Event_Router\\runtime\\knrouter.prs";
    } 

    //
    //	Router::GetConfigurationDirectory -
    //
    //		Virtual method to return the name of the directory
    //		used to store the router's configuration files when
    //		a standard install of the router is present.
    //

    public String GetConfigurationDirectory()
    {
	return "C:\\Program Files\\KnowNow\\Event_Router\\conf\\";
    }

    //
    //	Router::Clear -
    //
    //		Virtual method to clear the persistence for the
    //		router running on the current system. Return true
    //		on success, false on error.
    //

    public boolean Clear()
    {
	Common.RemoveFile(GetPersistenceFile());
	Common.EmptyDirectory(GetPersistenceDirectory(), false);
	return true;
    }

    //
    //	Router::Start -
    //
    //		Virtual method to start the router running on the
    //		current system.	Return true on success, false on error.
    //

    public boolean Start()
    {
	boolean status;

	System.out.println("RouterWindows: About to start router");

	try
	{
	    String output =
		Common.RunProcess("net start \"KnowNow Event Router-runtime\"");

	    status = (output.indexOf("was started successfully") != -1);
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Agent", Ex);
	    status = false;
	}

	return status;
    }

    //
    //	Router::Stop -
    //
    //		Virtual method to stop the router running on the
    //		current system.	Return true on success, false on error.
    //

    public boolean Stop()
    {
	boolean status;

	System.out.println("RouterWindows: About to stop router");

	try
	{
	    String output =
		Common.RunProcess("net stop \"KnowNow Event Router-runtime\"");

	    status =
		(output.indexOf("was stopped successfully") != -1) ||
		(output.indexOf("is not started")           != -1);
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Agent", Ex);
	    status = false;
	}

	return status;
    }

    //
    //	Router::Config -
    //
    //		Virtual method to configure the router running on the
    //		current system using the given file. Return true on
    //		success, false on error.
    //

    public boolean Config(String configFile)
    {
	boolean status;

	status = Common.CopyFile(configFile,
				 GetConfigurationDirectory() + "knrouter.conf");
				 
	return status;
    }
}


