
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
 * $Id: StatusUpdater.java,v 1.2 2004/04/19 05:39:10 bsittler Exp $
 *
 **/

//
// StatusUpdater.java -
//
//	Implement the status updater thread which periodically
//	writes an HTML page with the status of the agents.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;
import java.io.File;
import java.io.PrintStream;
import java.io.FileOutputStream;

public class StatusUpdater
    implements Runnable
{
    // Instance variables
    private Manager	MyManager;

    //
    //	StatusUpdater::StatusUpdater -
    //
    //		Constructor.
    //

    public StatusUpdater(Manager MyManager)
    {
	this.MyManager = MyManager;
    }

    //
    //	Runnable::run -
    //
    //		Execution thread for the status updater. Update the
    //		agent status web page every StatusUpdateTime 
    //		milliseconds. The thread will stop when the
    //		Running flag in the Manager becomes false.
    //

    public void run()
    {
	do
	{
	    // Create the temporary output file
	    FileOutputStream htmlFile   = null;
	    PrintStream htmlPrintStream = null;

	    try
	    {
		htmlFile        = new FileOutputStream(Common.StatusPageTempFile);
		htmlPrintStream = new PrintStream(htmlFile);

		// Get the Manager's AgentDataMap
		AgentDataMap agentDataMap = Manager.GetAgentDataMap();

		// Generate the web page to a temporary file
		htmlPrintStream.println("<html>");
		htmlPrintStream.println("<head>");
		htmlPrintStream.println("<title>Agent Status</title>");
		htmlPrintStream.println("<style>");
		htmlPrintStream.println(".header_cell  " +
		                        "{background-color: #c8c8c8; }");
		htmlPrintStream.println(".normal_cell  " +
		                        "{background-color: white; }");
		htmlPrintStream.println(".changed_cell " +
		                        "{background-color: yellow; }");
		htmlPrintStream.println("</style>");
		htmlPrintStream.println("<meta HTTP-EQUIV=Refresh " +
					"Content=\"5\" >");
		htmlPrintStream.println("</head>");
		htmlPrintStream.println("<body>");

		// The Agent Data Map
		htmlPrintStream.println("<hr>");
		agentDataMap.PrintHTML(htmlPrintStream, "", true);

		// Information about the Routers
		htmlPrintStream.println("<hr>");
		htmlPrintStream.println("<table border=0>");
		htmlPrintStream.println("<tr>");
		htmlPrintStream.println("<td><b>Coordinating Router:</b></td>");
		htmlPrintStream.println("<td>" + MyManager.GetCoordRouterURI() + "</td>");
		htmlPrintStream.println("</tr>");

		htmlPrintStream.println("</table>");
		
		htmlPrintStream.println("</body>");
		htmlPrintStream.println("</html>");
		htmlPrintStream.close();
		htmlFile.close();
	    
		// Rename the temporary file to the permanent file
		File TempFile = new File(Common.StatusPageTempFile);
		File PermFile = new File(Common.StatusPageFile);
		PermFile.delete();
		TempFile.renameTo(PermFile);
	    }
	    catch (Exception Ex)
	    {
		MyManager.Log("Could not create HTML file " +
			      Common.StatusPageTempFile);
	    }

	    // Sleep for a bit
	    try
	    {
		Thread.currentThread().sleep(Common.HeartbeatTime);
	    }	
	    catch (InterruptedException Ex)
	    {
		// This is ignorable 
	    }
	}
	while (MyManager.Running());
    }
}

