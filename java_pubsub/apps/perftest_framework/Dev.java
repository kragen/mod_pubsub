
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
 * $Id: Dev.java,v 1.2 2004/04/19 05:39:10 bsittler Exp $
 *
 **/

//
// Dev.java -
//
//	This is a scaffold program used to test out various new
//	ideas, functions, and methods. It is not a shipped part
//	of the test suite.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;
import java.util.*;
import java.net.InetAddress;
import java.io.InputStream;

public class Dev
    implements Log
{
    public static void main(String[] Args)
    {
	Dev DevObject = new Dev();
	DevObject.Exec();
    }

    public void Exec()
    {
	try
	{
	    if (false)
	    {
		// Test out fetching of IP addresses
		InetAddress MyAddress = Common.GetIPAddress();
		System.out.print("GetIPAddress returned ");
		System.out.print("address: " + MyAddress.getHostAddress());
		System.out.print(", host: " + MyAddress.getHostName());
		System.out.println();
	    }

	    if (false)
	    {
		// Test out downloading a non-trivial file
		Common.DownloadFileFromURL("http://www.vertexdev.com/~jeff/chv.exe",
				    "C:\\temp\\chv.exe", this);
	    }

	    if (false)
	    {
		// Test out running a child process
		String Output = Common.RunProcess("net statistics workstation");
		System.out.println("Process returned:");
		System.out.println(Output);
	    }

	    if (false)
	    {
		System.out.println("Making Routes");
		// Test out setting up routes
		Router.MakeRoutes("http://localhost:8000/kn",
				  "/test/fromTopic",
				  "/test/toTopic",
				  1, 1000, 10000, this);
	    }

	    if (false)
	    {
		System.out.println("Publishing");
		// Test out publishing to routes
		Router.Publish("http://localhost:8000/kn",
			       "/test/fromTopic0",
			       100,
			       100,
			       0,
			       this);

		System.out.println("Done!");
	    }

	    if (false)
	    {
		System.out.println("Before 1000 ms sleep");
		Thread.currentThread().sleep(1000);		
		System.out.println("After 1000 ms sleep");
	    }

	    if (false)
	    {
		// Test out starting and stopping the router on Windows
		boolean       status;
		RouterWindows rw = new RouterWindows();

		System.out.println("Starting router...");
		status = rw.Start();
		System.out.println("  Returned " + status);

		System.out.println("Sleeping...");
		Common.Sleep(5);

		System.out.println("Stopping router...");
		status = rw.Stop();
		System.out.println("  Returned " + status);
	    }

	    if (false)
	    {
		// Test out clearing router config on Windows
		boolean status;
		RouterWindows rw = new RouterWindows();
		System.out.println("Clearing router...");

		status = rw.Clear();
		System.out.println("  Returned " + status);
	    }
	    
	    if (false)
	    {
		// Test copying file
		boolean status;
		System.out.println("Copying file...");
		status = Common.CopyFile("C:\\temp\\a.txt", "C:\\temp\\a_copy.");
		System.out.println("  Returned " + status);
	    }

	    if (false)
	    {
		// Test getting host name
		String host = Common.GetHostName();
		System.out.println("Host: " + host);
	    }

	    if (false)
	    {
		// Test stripping quotes
		String s1 = "This has No Quotes!";
		String s2 = "\"This has Quotes!\"";
		String q1 = Common.StripQuotes(s1);
		String q2 = Common.StripQuotes(s2);

		System.out.println("From '" + s1 + "' to '" + q1 + "'");
		System.out.println("From '" + s2 + "' to '" + q2 + "'");
	    }

	    if (true)
	    {
		// Discover architecture and operating system
		String Architecture    = System.getProperty("os.arch");
		String OperatingSystem = System.getProperty("os.name");

		System.out.println("Arch: " + Architecture);
		System.out.println("OS  : " + OperatingSystem);
	    }
	}
	catch (Exception Ex)
	{
	    Common.DumpException("Dev", Ex);
	}
    }

    public void Log(String Msg)
    {
	System.out.println(Msg);
    }
}
