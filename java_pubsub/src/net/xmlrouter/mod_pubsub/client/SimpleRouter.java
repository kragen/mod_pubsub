package net.xmlrouter.mod_pubsub.client;
import java.io.*;
import java.net.*;
import java.util.*;

/**
 * @author msg
 * Simple router for publish/subscribe.
 */
public class SimpleRouter
{
	String serverURI;
	String basePath;
	Map eventStreams = new HashMap();
	
	/**
	 * @param server URI of message server (ex. http://localhost/kn/)
	 */
	public SimpleRouter(String uri) throws MalformedURLException
	{
		this.serverURI = uri;
		
		try
		{
			URL url = new URL(uri);
			basePath="";
		}
		catch(Exception e)
		{
			throw new MalformedURLException(uri);
		}
	}
	
	public String getMessageId()
	{
		return Double.toString(Math.random()).substring(2,10);
	}
	
	public String getRouteId(String topic,String msg_id)
	{
		return serverURI + topic + "/kn_routes/" + msg_id;
	}
	public String getTopicFromRoute(String route_id)
	{
		return null;
	}
	public String getIdFromRoute(String route_id)
	{
		return null;
	}
	

	/**
	 * Establish a route between a source and a destination.
	 * @param topic the source
	 * @param listener the listener
	 * @param options
	 * @return a route id
	 */
	public void publish(String topic,Map msg)
	{
		try
		{
			URL url =new URL(serverURI+basePath);
			HttpURLConnection conn;
					
			// add some things
			msg.put("do_method","notify");
			msg.put("kn_to",topic);

			// send message
			conn = HTTPUtil.Post(url,msg);
			
			// get response
			if (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300)
			{
				// @todo: consume response entity
			}
		}
		catch(Exception e)
		{
			System.err.println("subscribe() : ERROR : "+e.getMessage());
		}
	}

	/**
	 * Establish a route between a source and a destination.
	 * @param topic the source
	 * @param listener the listener
	 * @param options
	 * @return a route id
	 */
	public String subscribe(String topic,Listener listener,Map options)
	{
		String route_id=null;
		String random = getMessageId();
		
		// generate magic route id
		route_id = getRouteId(topic,random);
		
		Map msg = new HashMap();
		try
		{
			HttpURLConnection conn;
			EventStreamReader reader;
			URL url =new URL(serverURI+basePath);
			
			// add some things
			msg.put("kn_response_format","simple");
			msg.put("do_method","route");
			msg.put("kn_from",topic);
			msg.put("kn_id",route_id);
			// @todo: Add options

			// send message
			conn = HTTPUtil.Post(url,msg);
			
			// get response
			if (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300)
			{			
				// attach EventStream to the response
				reader = new EventStreamReader(conn.getInputStream(),listener);
				eventStreams.put(route_id,reader);
				
				// start processing events
				new Thread(reader).start();
			}
		}
		catch(Exception e)
		{
			System.err.println("subscribe() : ERROR : "+e.getMessage());
		}

		return route_id;
	}

	/**
	 * Establish a route between a source and a destination.
	 * @param topic the source
	 * @param listener the listener
	 * @param options
	 * @return a route id
	 */
	public String subscribe(String from,String to,Map options)
	{
		String route_id=null;
		String msg_id = getMessageId();
		
		// generate magic route id
		route_id = getRouteId(from,msg_id);
		
		Map msg = new HashMap();
		java.net.HttpURLConnection conn;
		try
		{
			String uri;
			uri = serverURI+basePath;
			URL url =new URL(uri);
				
			// add some things
			msg.put("do_method","route");
			msg.put("kn_from",from);
			msg.put("kn_to",to);
			//msg.put("kn_id",route_id);
			msg.put("do_max_age","3600");

			// send message
			conn = HTTPUtil.Post(url,msg);

			// get response
			if (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300)
			{				
			}
			else
			{
				System.err.println("subscribe() : ERROR : "+conn.getResponseMessage());
			}

			// consume response entity
			{
				byte buffer[] = new byte[1024];
				InputStream is = conn.getInputStream();
				while (is.read(buffer) > 0);
			}
		}
		catch(Exception e)
		{
			System.err.println("subscribe() : ERROR : "+e.getMessage());
		}

		return route_id;
	}
	
	/**
	 * Stop processing incoming events
	 * @param route_id a route id provided by the subscribe() method
	 * @see #subscribe
	 */	
	public void unsubscribe(String route_id)
	{
		EventStreamReader reader;

		reader = (EventStreamReader )eventStreams.remove(route_id);
		if (reader != null)
		{
			reader.stop();
		}
	}

	/**
	 * Remove a route between a source and a destination.
	 * @param route_id a route id from subscribe()
	 * @see subscribe
	 */	
	public void unsubscribe_session(String route_id)
	{
		Map msg = new HashMap();
		try
		{
			java.net.HttpURLConnection conn;
			String uri;
			uri = serverURI+basePath;
			URL url =new URL(uri);
			
			// add some things
			msg.put("do_method","route");
			msg.put("kn_from",getTopicFromRoute(route_id));
			msg.put("kn_id",getIdFromRoute(route_id));
			msg.put("kn_to","");
			
			conn=HTTPUtil.Post(url,msg);
			
			// check for errors
			if (conn.getResponseCode() >= 300)
			{				
			}
		}
		catch(Exception e)
		{
			System.err.println("unsubscribe() : ERROR : "+e.getMessage());
		}
	}
	
	public static void main(String args[])
	{
		// subscribe
		try
		{
			SimpleRouter router = new SimpleRouter("http://localhost/kn/");
			Listener listener = new DebugListener();
			router.subscribe("/what/chat/kn_journal",listener,null);
			router.subscribe("/what/chat","/what/chat/kn_journal",null);
		}
		catch(Exception e)
		{
		}
		
		// publish
		try
		{
			Map msg = new HashMap();
			SimpleRouter router = new SimpleRouter("http://localhost/kn/");

			msg.put("displayname","Snow White");			
			msg.put("kn_payload","Someday my prince will come");
			router.publish("/what/chat",msg);
		}
		catch(Exception e)
		{
			System.err.println("ERROR : " +e.getMessage());
		}
	}
}
