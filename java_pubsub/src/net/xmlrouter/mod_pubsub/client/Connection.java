package net.xmlrouter.mod_pubsub.client;
import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;

/**
 * @author msg
 * Manages connection and subscriptions to pubsub server.
 * 
 */
public class Connection implements Runnable
{
	EventServer server;
	String journal;
	URL url;
	Map msg;
	EventStream events;
	Listener listener;
	
	HashMap routes=new HashMap();
	HashMap optionsMap=new HashMap();
	
	public Connection(EventServer server,String journal,URL url,Map msg,Listener listener)
	{
		this.server = server;
		this.journal = journal;
		this.url = url;
		this.msg = msg;
		this.listener = listener;
	}
	public EventStream getEvents()
	{
		return events;		
	}
	
	/**
	 * Create routes between currently subscribed topics and the 'journal' topic
	 * for this connection.
	 * This is useful to re-connect to the pubsub server if the network 
	 * connection has failed.
	 */
	private void establishSubscriptions()
	{
		String topic;
		
		for (Iterator it = routes.keySet().iterator(); it.hasNext();)
		{
			topic = (String)it.next();
			subscribe(topic,(Map)optionsMap.get(topic));
		}
	}

	/**
	 * Open network connection to the pubsub server and attach an event stream to 
	 * the network connection.
	 * This will re-establish any existing subscriptions. 
	 * 
	 * @throws IOException
	 */
	public void open() throws IOException
	{
		HttpURLConnection conn;
		
		// make sure we are closed
		close();
		
		try
		{
		// send message
		conn = HTTPUtil.Post(url, msg);
		events = new EventStream(conn.getInputStream());
		}
		catch(IOException e)
		{
			System.err.println("open() : Unable to open connection to "+url+" : "+e.getMessage());
			throw e;		
		}
		// re-establish subscriptions
		establishSubscriptions();
	}

	public void close()
	{
		try
		{
			events.is.close();
		}
		catch(Exception ignoreForNow)
		{
		}
	}
	
	/**
	 * Establish route on pubsub server between specified topic and the
	 * 'journal' topic for this connection.
	 * Messages published to the specified topic will appear in the event stream
	 * for this connection (after traveling to the server and back again). 
	 * 
	 * @param topic
	 * @param options
	 */
	public void subscribe(String topic,Map options)
	{
		String route;
		route = server.subscribe(topic,journal,options);
		routes.put(topic,route);
		optionsMap.put(topic,options);		
	}
	
	public void unsubscribe(String topic)
	{
		server.unsubscribe((String)routes.get(topic));
		routes.remove(topic);
		optionsMap.remove(topic);
	}
	
	public void dispatchNextEvent() throws IOException
	{
		if (listener != null)
		{
			listener.onMessage(events.readEvent());
		}
	}
	public void run()
	{
		try
		{
			while (true)
				dispatchNextEvent();		
		}
		catch(Exception e)
		{			
			System.err.println("Connection stopped. "+e.getMessage());
		}
	}
}
