
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
 * $Id: RouterSolaris.java,v 1.1 2003/08/15 23:41:07 ifindkarma Exp $
 *
 **/

//
// RouterSolaris.java -
//
//	Solaris implementation of code to perform various operations
//	on a router. 
//
//	Copyright (c) 2002 - KnowNow, Inc.
//

import java.lang.*;

public class RouterSolaris extends Router
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
	return "/usr/local/kn/runtime/knrouter.prs_db/";
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
	return "/usr/local/kn/runtime/knrouter.prs";
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
	return "/usr/local/kn/conf/";
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

	System.out.println("RouterSolaris: About to start router");

	try
	{
	    String output =
		Common.RunProcess("/usr/local/kn/bin/nsd -t /usr/local/kn/conf/knrouter.conf");

	    status = true;
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

	System.out.println("RouterSolaris: About to stop router");

	try
	{
	    String output =
		Common.RunProcess("/bin/pkill -9 -n -x nsd8x");

	    status = true;
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


