
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
 * $Id: Common.java,v 1.1 2003/08/15 23:41:06 ifindkarma Exp $
 *
 **/

//
// Common.java -
//
//	Encapsulate common constants, definitions, and utility functions.
//
//	Copyright (c) 2002 - KnowNow, Inc.
//

import java.lang.*;
import java.util.*;
import java.text.*;
import java.io.*;
import java.net.*;

public class Common
{
    //
    //	CommandTopic -
    //		This is the topic used to send commands from the
    //		Manager to the Agents.
    //

    public static final String CommandTopic = "/ktest/agent_command";

    //
    // StatusTopic -
    //		This is the topic used to send status reports
    //		from Agents to the Manager.
    //

    public static final String StatusTopic  = "/ktest/agent_status";

    //
    //	HeartbeatTime -
    //
    //		This is the number of milliseconds between
    //		heartbeat reports from the Agents to the 
    //		Manager.

    public static final int HeartbeatTime = 1000;

    //
    //	StatusPageFile -
    //
    //		This is the file name where the Manager will
    //		write an HTML page with the status of each
    //		Agent.

    public static final String StatusPageFile = "agent.html";

    //
    //	StatusPageTempFile -
    //
    //		This is the temporary file name for the agent
    //		status HTML page. The HTML will be written
    //		to this file, which will then be renamed to
    //		StatusPageFile.

    public static final String StatusPageTempFile = "agent_temp.html";

    //
    //	StatusUpdateTime -
    //
    //		This is the number of milliseconds between
    //		updates to the StatusPageFile.
    //

    public static final int StatusUpdateTime = 1000;

    //
    //	BlockSize -
    //
    //		The block sized used when downloading from a URL
    //		or copying a file.
    //
    
    public static final int BlockSize = 1024;

    //
    //	ManagerLogFile -
    //
    //		The name of the log file written by the Manager.
    //

    public static final String ManagerLogFile = "manager.log";

    //
    //	PayloadChar -
    //
    //		The character that is replicated to form the payload
    //		for the test messages.
    //

    public static final String PayloadChar = new String("K");

    //
    // Common::DumpException -
    //	
    //		Produce a dump of the given Exception, with the
    //		given Label.
    //

    public static void DumpException(String Label,
				     Exception Ex)
    {
	if (Label != null)
	{
	    System.out.print("[" + Label + "] Exception: ");
	}
	else
	{
	    System.out.print("Exception: ");
	}

	String ExceptionMessage = Ex.getMessage();
	if (ExceptionMessage != null)
	{
	    System.out.print(ExceptionMessage);
	}
	else
	{
	    System.out.print("(No message supplied)");
	}
	System.out.println();

	System.out.println("Stack Trace:");
	Ex.printStackTrace();
    }

    //
    // Common::GetIPAddress -
    //
    //		Return an IP address for the machine running the code.
    //		If the machine has more than one address then this
    //		method could return any of them. Raise an exception
    //		if the machine has no IP addresses.
    //

    public static InetAddress GetIPAddress()
        throws Exception
    {
	Enumeration NetInterfaceEnum = NetworkInterface.getNetworkInterfaces();

	while (NetInterfaceEnum.hasMoreElements())
	{
	    NetworkInterface Interface =
		(NetworkInterface) NetInterfaceEnum.nextElement();
	    Enumeration NetAddressEnum = Interface.getInetAddresses();

	    while (NetAddressEnum.hasMoreElements())
	    {
		InetAddress Address =
		    (InetAddress) NetAddressEnum.nextElement();

		if (!Address.getHostAddress().equals("127.0.0.1"))
		{
		    return Address;
		}
	    }
	}

	throw new Exception("No IP addresses found");
    }

    //
    //	Common::CreateUniqueID -
    //
    //		Create a unique ID using the given prefix. Raises
    //		an exception if it cannot get an IP address.
    //

    public static String CreateUniqueID(String Prefix)
	throws Exception
    {
	SimpleDateFormat MyFormat =
	    new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss-S");
	String MyDate             = MyFormat.format(new Date());

	return Prefix + "_" + GetHostName() + "_" + MyDate;
    }

    //
    //	Common::GetHostName -
    //
    //		Return the host name. Raises an exception if
    //		it cannot get an IP address.
    //

    public static String GetHostName()
	throws Exception
    {
	InetAddress MyAddress = Common.GetIPAddress();
	return MyAddress.getHostName();
    }

    //
    // Common::StringRepeat -
    //
    //		Return a string formed by repeating the given
    //		string the given number of times.
    //

    public static String StringRepeat(int Count,
				      String TheString)
    {
	StringBuffer NewString =
	    new StringBuffer(TheString.length() * Count);

	for (int i = 0; i < Count; i++)
	{
	    NewString.append(TheString);
	}

	return NewString.toString();
    }

    //
    //	Common::RightJustify -
    //
    //		Right justify the given string by adding leading
    //		blanks to attain the given length.
    //

    public static String RightJustify(String TheString, int TheLength)
    {
	if (TheString.length() < TheLength)
	{
	    return StringRepeat(TheLength - TheString.length(), " ") + TheString;
	}
	else
	{
	    return TheString;
	}
    }

    //
    //	Common::Sleep -
    //
    //		Just sleep for the given number of seconds.
    //

    public static void Sleep(int Seconds)
    {
	try
	{
	    Thread.currentThread().sleep(Seconds * 1000);
	}
	catch (Exception Ex)
	{
	    System.err.println("Common::Sleep() - Who woke me up?");
	}
    }

    //
    //	DownloadFileFromURL -
    //
    //		Download the content from the given URL and store
    //		it in the given file, with logging to Logger if
    //		given. Return true on success, or false on error.
    //

    public static boolean DownloadFileFromURL(String FileURL,
					      String FileName,
					      Log Logger)
    {
	try
	{
	    URL           url           = new URL(FileURL);
	    URLConnection urlConn       = url.openConnection();

	    // Turn off local caching so we always get the latest bits
	    urlConn.setUseCaches(false);

	    String        contentType   = urlConn.getContentType();
	    int           contentLength = urlConn.getContentLength();

	    if (Logger != null)
	    {
		Logger.Log("DownloadFileFromURL(" + FileURL + ", " +
			   FileName + ")");
	    }

	    if (contentLength == -1)
	    {
		if (Logger != null)
		{
		    Logger.Log("File has unknown content length");
		}

		return false;
	    }

	    if (Logger != null)
	    {
		Logger.Log("Downloading " + contentLength + " bytes");
	    }

	    byte             buffer[]    = new byte[BlockSize];
	    InputStream      inputStream = urlConn.getInputStream();
	    File             outFile     = new File(FileName);
	    FileOutputStream outStream   = new FileOutputStream(outFile);

	    // Read from the connection, write to the file
	    int bytesRead = 0;
	    int offset    = 0;
	    while (bytesRead >= 0)
	    {
		bytesRead = inputStream.read(buffer);
		if (bytesRead == -1)
		{
		    break;
		}

		outStream.write(buffer, 0, bytesRead);
		offset += bytesRead;
	    }
	
	    if (Logger != null)
	    {
		Logger.Log("Download complete");
	    }

	    // Now write the file
	    outStream.close();

	    if (Logger != null)
	    {
		Logger.Log("File written");
	    }

	    return true;
	}
	catch (Exception Ex)
	{
	    DumpException(null, Ex);

	    Logger.Log("Download failed: " + Ex.getMessage());
	    return false;
	}
    }

    //
    //	Common::GetDownloadDir -
    //
    //		Based on the given operating system and architecture,
    //		return the system's preferred temporary directory.
    //		The values for OperatingSystem and SystemArchitecture
    //		are as returned from the getProperty function for
    //		"os.name" and "os.arch", respectively.
    //
    //		The returned name will always end with a path separator.
    //

    static String GetDownloadDir(String OperatingSystem,
				 String SystemArchitecture)
	throws Exception
    {
	System.out.println("GetDownloadDir(" + OperatingSystem + ", " +
			   SystemArchitecture + ")");

	return System.getProperty("java.io.tmpdir");
    }

    //
    //  Common::RemoveFile -
    //
    //		Remove the given file. No return value.
    //

    static void RemoveFile(String theFile)
    {
	try
	{
	    File fileFile = new File(theFile);
	    fileFile.delete();
	}
	catch (Exception Ex)
	{
	    System.err.println("Could not remove file " + theFile);
	    DumpException(null, Ex);
	}
    }

    //
    //	Common::EmptyDirectory -
    //
    //		Remove all files from the given directory, and
    //		(if deleteDirectory is true) the directory itself.
    //

    static void EmptyDirectory(String TheDir, boolean deleteDirectory)
    {
	try
	{
	    File fileDir     = new File(TheDir);
	    File[] fileFiles = fileDir.listFiles();

	    for (int i = 0; i < fileFiles.length; i++)
	    {
		if (fileFiles[i].isDirectory())
		{
		    EmptyDirectory(fileFiles[i].getPath(), true);
		}
		else
		{
		    fileFiles[i].delete();
		}
	    }

	    if (deleteDirectory)
	    {
		fileDir.delete();
	    }
	}
	catch (Exception Ex)
	{
	    System.err.println("Could not remove directory " +
			       TheDir +
			       " or one of its components");
	    DumpException(null, Ex);
	}
    }

    //
    //	Common::RunProcess -
    //
    //		Run the given executable and return its output
    //		as a String. Raise an exception if something
    //		goes wrong.

    public static String RunProcess(String executable)
	throws Exception
    {
	// Launch the program
	Process theProcess = Runtime.getRuntime().exec(executable);

	// Capture all output
	InputStream  theInput = theProcess.getInputStream();
	boolean      eof      = false;
	byte         buffer[] = new byte[1024];
	StringBuffer output   = new StringBuffer();

	while (!eof)
	{
	    int  numBytes = theInput.read(buffer);

	    if (numBytes == -1)
	    {
		eof = true;
	    }
	    else
	    {
		output.append(new String(buffer, 0, numBytes));
	    }
	}
		    
	// Wait for it to complete
	theProcess.waitFor();

	return new String(output);
    }

    //
    // Common::ListThreads -
    //
    //		Enumerate and display the set of active threads.
    //		No return value.
    //

    public static void ListThreads()
    {
	ThreadGroup group       = Thread.currentThread().getThreadGroup();
        int         threadCount = group.activeCount();
	Thread      threads[]   = new Thread[threadCount]; 

	System.out.println("There are " + threadCount + " active threads:");
	Thread.enumerate(threads); 
	for (int i = 0; i < threadCount; i++) 
	{
	    System.out.println("  [" + i + "]" + threads[i].getName());
	}
    }

    //
    //	Common::CopyFile -
    //
    //		Copy from the given source to the given destination. Return
    //		true on success, false or error.
    //

    public static boolean CopyFile(String fromFileName, String toFileName)
    {
	try
	{
	    File inputFile  = new File(fromFileName);
	    File outputFile = new File(toFileName);

	    FileReader input  = new FileReader(inputFile);
	    FileWriter output = new FileWriter(outputFile);

	    BufferedReader inputBuf  = new BufferedReader(input);
	    BufferedWriter outputBuf = new BufferedWriter(output);

	    int ch;
	    while ((ch = inputBuf.read()) != -1)
	    {
		outputBuf.write(ch);
	    }

	    inputBuf.close();
	    outputBuf.close();
	    input.close();
	    output.close();
	    
	    return true;
	}
	catch (Exception Ex)
	{
	    Common.DumpException("", Ex);
	    return false;
	}
    }

    //
    //	Common::StripQuotes -
    //
    //		If the given string contains leading and trailing quotation marks,
    //		then remove them. Return the original string or the stripped
    //		string, as appropriate.
    //

    public static String StripQuotes(String s)
    {
	if (s.startsWith("\"") && s.endsWith("\""))
	{
	    return new String(s.substring(1, s.length() - 1));
	}      
	else
	{
	    return s;
	}
    }
}

