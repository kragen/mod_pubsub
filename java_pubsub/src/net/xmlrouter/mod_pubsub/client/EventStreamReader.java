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

	static int MAX_EVENT_SIZE = 65535;
		
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
	 * readLine - read a line of ascii characters until a newline char.
	 */
	String readLine(InputStream reader) 
	{
		StringBuffer buffer=new StringBuffer();
		byte b[] = new byte[1];
		char c;
		
		try
		{
			/*
			while (reader.available() == 0)
				Thread.sleep(100); 
			*/
			
			while ((reader.read(b) >= 0) && 
					(b[0] != '\r') && 
					(b[0] != '\n'))
			{
				c = (char)b[0];
				buffer.append(c);
			}
		}
		catch(IOException e)
		{
			System.err.println("readLine() : "+e.getMessage());
		}

		return buffer.toString();
	}
	
	/**
	 * Read events and dispatch to a lister.onMessage() method.
	 * This implementation fails if an event is larger than 64KB.
	 */
	public void run()
	{
		try
		{
			byte[] buffer = new byte[4096];
			int nBytes;
			ByteArrayInputStream bais = new  ByteArrayInputStream(buffer);
			BufferedReader event;
			String line;
			String name;
			String value;
			
			int bytesRead;
			HashMap msg;
			
			while (bKeepGoing)
			{
				int pos;
				
				// read a line
				line = readLine(response);
				if ((line == null) || (line.length()==0))
					break;
				line = line.trim();

				// get size of message					
				nBytes = Integer.parseInt(line);
				if (nBytes > MAX_EVENT_SIZE)
					throw new RuntimeException("EventStreamReader() : Event too large : "+line);

				// make sure buffer is big enough
				if (buffer.length < nBytes)
				{
					buffer = new byte[nBytes];
					bais = new  ByteArrayInputStream(buffer);
				}
				bais.reset();
				event = new BufferedReader(new InputStreamReader(bais,"UTF-8"));

				// read full event
				msg = new HashMap();
				bytesRead = response.read(buffer,0,nBytes);
				if (bytesRead != nBytes)
					throw new RuntimeException("EventStreamReader() : Didn't read enough bytes : " + bytesRead);

				// read headers from byte buffer
				while (((line = event.readLine()) != null) && 
						(line.length() > 0))
				{
					// convert to event object
					pos = line.indexOf(':');
					if (pos > 0)
					{
						name = line.substring(0,pos);
						value = line.substring(pos+2);	// @todo: decode 'quoted-printable'
						msg.put(name,value);
					}
				}
				
				// read body
				name = "kn_payload";
				nBytes = bais.available();
				if (nBytes > 0)
				{
					value = new String(buffer,buffer.length-nBytes,nBytes,"UTF-8");
					msg.put(name,value);
				}
				
				// notify the listener
				listener.onMessage(msg);
				line = readLine(response);
			}
		}
		catch(IOException e)
		{
			System.err.println("EventStreamReader : "+e.getMessage());
		}
	}

}
