
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
 * $Id: Router.java,v 1.1 2003/08/15 23:41:06 ifindkarma Exp $
 *
 **/

//
// Router.java -
//
//	Perform various operations on a router. This class is subclassed
//	for each platform/architecture pair.
//
//	Copyright (c) 2002 - KnowNow, Inc.
//

import java.io.*;
import java.net.*;
import java.util.*;
import java.lang.*;

public abstract class Router
{
    //
    //	Router::GetRouter -
    //
    //		Given an architecture and an operating system, return
    //		a subclass of the Router object which has implementations
    //		of the abstract methods for that architecture and 
    //		oerating system.
    //

    public static Router GetRouter(String Architecture,
				   String OperatingSystem)
    {
	if (Architecture.equals("x86"))
	{
	    if (OperatingSystem.equals("Windows 2000") ||
		OperatingSystem.equals("Windows XP"))
	    {
		return new RouterWindows();
	    }
	}

	if (Architecture.equals("sparc"))
	{
	    if (OperatingSystem.equals("SunOS"))
	    {
		return new RouterSolaris();
	    }
	}

	if (OperatingSystem.equals("Linux"))
	{
	    return new RouterLinux();
	}

	System.out.println();
	System.out.println("=======================================================");
	System.out.println("There is no Router implementation for: ");
	System.out.println("  Operating system     : " + OperatingSystem);
	System.out.println("  Hardware architecture: " + Architecture);
	System.out.println();
	System.out.println("Please examine method GetRouter in source file ");
	System.out.println("Router.java for more information.");
	System.out.println("=======================================================");
	System.out.println();

	return null;
    }

    //
    //	Router::MakeRoutes -
    //
    //		Create routes from a series of starting points
    //		to a journal topic. The exact number of routes
    //		created is (seqEnd - seqStart + 1) * numPublishers.
    //		Return true on success, false on failure. Log
    //		failures to Logger, if given.
    //

    public static boolean MakeRoutes(String routerURI,
				     String sourceTopic,
				     String journalTopic,
				     int seqStart,
				     int seqEnd,
				     int numPublishers,
				     Log logger)
    {
	boolean success;

	// Generate a set of routes for each publisher
        for (int i = 0; i < numPublishers; i++)
	{
	    // Form the basic command
            StringBuffer baseCommand = new StringBuffer(routerURI);
            baseCommand.append("?do_method=route&kn_from=");
            baseCommand.append(sourceTopic);
            baseCommand.append(String.valueOf(i));
	    
	    // Generate routes for seqStart ... seqEnd
            for (int j = seqStart; j <= seqEnd; j++)
	    {
                StringBuffer fullCommand =
		    new StringBuffer(baseCommand.toString());
                fullCommand.append("&kn_to=");
                fullCommand.append(journalTopic);
                fullCommand.append(String.valueOf(j));

                try
		{
                    URL r = new URL(fullCommand.toString());
                    HttpURLConnection c =
			(HttpURLConnection) r.openConnection();

                    if (c.getResponseCode() != HttpURLConnection.HTTP_OK)
		    {
                        success = false;

			if (logger != null)
			{
			    logger.Log("MakeRoutes(" + fullCommand + ") " +
				       "failed with code " +
				       c.getResponseCode());
			}
                    }
                }
		catch(MalformedURLException e)
		{
		    return false;
                }
		catch(IOException e)
		{
		    return false;
                }
            }
        }

	if (logger != null)
	{
	    logger.Log("MakeRoutes() succeeded");
	}

	return true;
    }

    //
    //	Router::Publish -
    //
    //		Create and publish events of the given size
    //		to the given topic, with the given millisecond 
    //		delay between events. Return true on success,
    //		false on error. If delay is 0 then the
    //		events are blasted out at wire speed.
    //

    public static boolean Publish(String routerURI,
				  String topic,
				  int eventSize,
				  int eventCount,
				  int delay,
				  Log logger)
    {
	try
	{
	    // Create payload string
	    StringBuffer payload = new StringBuffer(eventSize);
	    for (int i = 0; i < eventSize; i++)
	    {
		payload.append(Common.PayloadChar);
	    }

	    // Create URL to router
	    String connURL = routerURI          +
		             "?"                +
		             "do_method=notify" +
		             "&kn_to="  + topic +
  		             "&kn_payload=" + payload;

	    logger.Log("About to publish " + eventCount + " events!");

	    // Send events
	    for (int e = 0; e < eventCount; e++)
	    {
		// Create the URL
		URL url = new URL(connURL);
		System.out.println("[" + e + "]: " + connURL);
		
		// Make the connection 
		HttpURLConnection urlConn = (HttpURLConnection) url.openConnection();

		// Keep the HTTP connection open
		urlConn.setRequestProperty("Connection", "Keep-Alive");
		
		// Connect 
		urlConn.connect();
		
		// Skip out on errors
		if (urlConn.getResponseCode() != HttpURLConnection.HTTP_OK)
		{
		    if (logger != null)
		    {
			logger.Log("Connection to " + routerURI + " refused with error code " +
				   urlConn.getResponseCode());
		    }
		    return false;
		}

		// Capture and ignore the response
		BufferedReader reader =
		    new BufferedReader(new InputStreamReader(urlConn.getInputStream()));

		while (reader.readLine() != null)
		{
		}
		reader.close();

		if (delay != 0)
		{
		    System.out.println("Sleeping for " + delay + " milliseconds");
		    Thread.currentThread().sleep(delay);
		}

		// At this point closing the connection is the right thing to do. 
		// There does not seem to be a way to do this in Java. I could run
		// the finalizer, I suppose.
	    }

	    if (logger != null)
	    {
		logger.Log("Published "       + eventCount +
			   " events of size " + eventSize  +
			   " to "             + routerURI);
	    }
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Agent", Ex);
	    return false;
	}

	return true;
    }

    //
    //	Router::GetPersistenceDirectory -
    //
    //		Virtual method to return the name of the directory
    //		used to store the router's persistence files when
    //		a standard install of the router is present.
    //

    public abstract String GetPersistenceDirectory();

    //
    //	Router::GetPersistenceFile -
    //
    //		Virtual method to return the name of the file used to
    //          store the router's persistence information when
    //		a standard install of the router is present.
    //

    public abstract String GetPersistenceFile();

    //
    //	Router::GetConfigurationDirectory -
    //
    //		Virtual method to return the name of the directory
    //		used to store the router's configuration files when
    //		a standard install of the router is present.
    //

    public abstract String GetConfigurationDirectory();

    //
    //	Router::Clear -
    //
    //		Virtual method to clear the persistence for the
    //		router running on the current system. Return true
    //		on success, false on error.
    //

    public abstract boolean Clear();

    //
    //	Router::Start -
    //
    //		Virtual method to start the router running on the
    //		current system.	Return true on success, false on error.
    //

    public abstract boolean Start();

    //
    //	Router::Stop -
    //
    //		Virtual method to stop the router running on the
    //		current system.	Return true on success, false on error.
    //

    public abstract boolean Stop();

    //
    //	Router::Config -
    //
    //		Virtual method to configure the router running on the
    //		current system using the given file. Return true on
    //		success, false on error.
    //

    public abstract boolean Config(String configFile);
}
