 package net.xmlrouter.mod_pubsub.client;
import java.io.*;
import java.net.*;
import java.util.*;

import HTTPClient.*;

/**
 * @author msg
 * Simple router for publish/subscribe.
 */
public class SimpleRouter
{
	HTTPConnection server;
	String serverURI;
	String basePath;
	Map eventStreams = new HashMap();
	
	/**
	 * @param server URI of message server
	 */
	public SimpleRouter(String uri) throws MalformedURLException
	{
		this.serverURI = uri;
		
		try
		{
			URI url = new URI(uri);
			//basePath = url.getPathAndQuery();
			basePath="";
			server = createConnection(uri);
		}
		catch(Exception e)
		{
			throw new MalformedURLException(uri);
		}
	}
	
	HTTPConnection createConnection(String uri) throws Exception
	{
		URI url = new URI(uri);
		
		server= new HTTPConnection(url.getScheme(),url.getHost(),url.getPort());
		server.setAllowUserInteraction(false);
		return server;
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
			java.net.HttpURLConnection conn;
					
			// add some things
			msg.put("kn_method","notify");
			msg.put("kn_to",topic);

			// copy Map into namevalue pair array
			String body;
			{
				StringBuffer sb = new StringBuffer();
				
				String name;
				Object value;
				Iterator it = msg.keySet().iterator();
				while (it.hasNext())
				{
					name = (String)it.next();
					value = URLEncoder.encode((String)msg.get(name));
					sb.append(name+"="+value);
					if (it.hasNext())
						sb.append("&");
				}
				body = sb.toString();
			}
			
			// send message
			{
				OutputStream os;
				
				conn = (java.net.HttpURLConnection)url.openConnection();
				conn.setRequestMethod("POST");
				conn.setDoInput(true);
				conn.setDoOutput(true);
				conn.setRequestProperty("User-Agent","mod_pubsub.java/0.1");
				conn.setRequestProperty("Content-Type","application/x-www-form-urlencoded");
				conn.setRequestProperty("Content-Length",Integer.toString(body.length()*2));
				conn.connect();
				os = conn.getOutputStream();

				// send content body
				for (int i=0; i < body.length(); i++)
				{
					os.write((byte)body.charAt(i));
				}
				
				// get response
				if (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300)
				{
					// @todo: consume response entity
				}
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
		String msg_id = getMessageId();
		String random = getMessageId();
		
		// generate magic route id
		route_id = getRouteId(topic,msg_id);
		
		Map msg = new HashMap();
		try
		{
			URL url =new URL(serverURI+basePath);
			java.net.HttpURLConnection conn;
			
			msg_id = getMessageId();
			
			// add some things
			msg.put("kn_response_format","simple");
			msg.put("kn_method","route");
			msg.put("kn_from",topic);
			msg.put("do_max_age","3600");
			//msg.put("kn_uri",route_id);

			// copy Map into namevalue pair array
			String body;
			{
				StringBuffer sb = new StringBuffer();
				
				String name;
				Object value;
				Iterator it = msg.keySet().iterator();
				while (it.hasNext())
				{
					name = (String)it.next();
					value = URLEncoder.encode((String)msg.get(name));
					sb.append(name+"="+value);
					if (it.hasNext())
						sb.append("&");
				}
				body = sb.toString();
			}
			
			// send message
			{
				OutputStream os;
				
				conn = (java.net.HttpURLConnection)url.openConnection();
				conn.setRequestMethod("POST");
				conn.setDoInput(true);
				conn.setDoOutput(true);
				conn.setRequestProperty("User-Agent","mod_pubsub.java/0.1");
				conn.setRequestProperty("Content-Type","application/x-www-form-urlencoded");
				conn.setRequestProperty("Content-Length",Integer.toString(body.length()*2));
				conn.connect();
				os = conn.getOutputStream();

				// send content body
				for (int i=0; i < body.length(); i++)
				{
					os.write((byte)body.charAt(i));
				}
				
				// get response
				if (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300)
				{
					// add the listener
					EventStreamReader reader;
					System.out.println(conn.getResponseMessage());
					reader = new EventStreamReader(conn.getInputStream(),listener);
					eventStreams.put(route_id,reader);
	
					// start processing events
					reader.run();
					
					//new Thread(reader).run();
				}
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
		String random = getMessageId();
		
		// generate magic route id
		route_id = getRouteId(from,msg_id);
		
		Map msg = new HashMap();
		java.net.HttpURLConnection conn;
		try
		{
			String uri;
			uri = serverURI+basePath;
			URL url =new URL(uri);
			
			msg_id = getMessageId();
			
			// add some things
			msg.put("kn_method","route");
			msg.put("kn_from",from);
			msg.put("kn_to",to);
			//msg.put("do_max_age","3600");
			//msg.put("kn_uri",route_id);

			// copy Map into namevalue pair array
			String body;
			{
				StringBuffer sb = new StringBuffer();
				
				String name;
				Object value;
				Iterator it = msg.keySet().iterator();
				while (it.hasNext())
				{
					name = (String)it.next();
					value = URLEncoder.encode((String)msg.get(name));
					sb.append(name+"="+value);
					if (it.hasNext())
						sb.append("&");
				}
				body = sb.toString();
			}
			
			// send message
			{
				OutputStream os;
				
				conn = (java.net.HttpURLConnection)url.openConnection();
				conn.setRequestMethod("POST");
				conn.setDoInput(true);
				conn.setDoOutput(true);
				conn.setRequestProperty("User-Agent","mod_pubsub.java/0.1");
				conn.setRequestProperty("Content-Type","application/x-www-form-urlencoded");
				conn.setRequestProperty("Content-Length",Integer.toString(body.length()*2));
				conn.connect();
				os = conn.getOutputStream();

				// send content body
				for (int i=0; i < body.length(); i++)
				{
					os.write((byte)body.charAt(i));
				}
				
				// get response
				if (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300)
				{
					conn.getContentLength();
				}
			}
		}
		catch(Exception e)
		{
			System.err.println("subscribe() : ERROR : "+e.getMessage());
		}

		return route_id;
	}

// serverURI + "/" + random + "/kn_journal/"

	
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
			HTTPResponse response;
			
			// add some things
			msg.put("kn_method","route");
			msg.put("kn_from",getTopicFromRoute(route_id));
			msg.put("kn_id",getIdFromRoute(route_id));
			msg.put("kn_to","");
			msg.put("kn_response_format","simple");
			
			// copy Map into namevalue pair array
			int i=0;
			String name;
			Object value;
			Iterator it = msg.keySet().iterator();
			NVPair form_data[] = new NVPair[msg.size()];
			while (it.hasNext())
			{
				name = (String)it.next();
				value = msg.get(name);
				form_data[i] = new NVPair(name,value.toString());
				i++;
			}
			
			// send message
			response = server.Post(basePath,form_data);

			// check for errors
			if (response.getStatusCode() >= 300)
			{				
			}
			// read response
			System.out.println(response.getText());
		
		}
		catch(Exception e)
		{
			System.err.println("unsubscribe() : ERROR : "+e.getMessage());
		}
	}
	
	public static void main(String args[])
	{
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

		// subscribe
		try
		{
			SimpleRouter router = new SimpleRouter("http://localhost/kn/");
			Listener listener = new DebugListener();
			router.subscribe("/what/chat","/what/chat/kn_journal",null);
			router.subscribe("/what/chat/kn_journal",listener,null);
		}
		catch(Exception e)
		{
		}
		
	}
}
