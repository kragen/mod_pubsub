package net.xmlrouter.mod_pubsub.client;

import java.util.*;
import java.io.*;

/**
 * @author mike
 */
public class EventStreamReader implements Runnable
{
	InputStream response;
	Listener listener;
	boolean bKeepGoing=true;
	
	public EventStreamReader(InputStream is,Listener listener)
	{
		this.response = is;
		this.listener = listener;
	}

	public void stop()
	{
		bKeepGoing=false;
	}
	
	/**
	 * @see java.lang.Runnable#run()
	 */
	public void run()
	{
		try
		{
			byte[] buffer = new byte[4096];
			int bytesRead;
			String line;
			BufferedReader reader = new BufferedReader(new InputStreamReader(response,"UTF-8"));
			HashMap msg = new HashMap();
			
			while (bKeepGoing)
			{
				int pos;
				// read a line
				line = reader.readLine();
				if (line == null)
					break;

				// convert to event object
				pos = line.indexOf(':');
				if (pos > 0)
				{
					msg.put(line.substring(0,pos),line.substring(pos+2));
				}
				// notify the listener
				if (line.length() == 0)		
				{	
					listener.onMessage(msg);
					msg.clear();
				}
			}
		}
		catch(IOException e)
		{
			System.err.println("EventStreamReader : "+e.getMessage());
		}
	}

}
