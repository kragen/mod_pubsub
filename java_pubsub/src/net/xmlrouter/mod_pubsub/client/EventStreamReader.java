package net.xmlrouter.mod_pubsub.client;

import java.util.*;
import java.io.*;

/**
 * @author mike
 */
public class EventStreamReader implements Runnable
{
	EventStream events;
	Listener listener;
	boolean bKeepGoing=true;
	
	public EventStreamReader(InputStream is,Listener listener)
	{
		events = new EventStream(is);
		this.listener = listener;
	}

	public void stop()
	{
		try {
			events.is.close();
		} catch (IOException e) {
			// ignore
			e.printStackTrace();
		}
		bKeepGoing=false;
	}
		
	/**
	 * Read events and dispatch to a lister.onMessage() method.
	 */
	public void run()
	{
		try
		{
			Map msg;
			while (bKeepGoing)
			{
				msg = events.readEvent();
				if (msg == null)
					break;
				
				// notify the listener
				listener.onMessage(msg);
			}
		}
		catch(IOException e)
		{
			System.err.println("EventStreamReader : "+e.getMessage());
		}
	}

}
