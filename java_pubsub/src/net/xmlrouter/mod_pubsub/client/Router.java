 package net.xmlrouter.mod_pubsub.client;
import java.net.MalformedURLException;
import java.util.*;

import HTTPClient.*;

/**
 * @author msg
 * Simple router for publish/subscribe.
 */
public class Router
{
	HTTPConnection server;
	String serverURI;
	String basePath;
	Map eventStreams = new HashMap();
	
	/**
	 * @param server URI of message server
	 */
	public Router(String uri) throws MalformedURLException
	{
		this.serverURI = uri;
		
		try
		{
			URI url = new URI(uri);
			basePath = url.getPathAndQuery();
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
	 * Send a message to the specified topic.
	 * @param topic the destination
	 * @param msg the message
	 * @return route id
	 */
	public String publish(String topic,Map msg)
	{
		String msg_id=null;
		
		try
		{
			HTTPResponse response;
			msg_id = getMessageId();
			
			// add some things
			msg.put("do_method","notify");
			msg.put("kn_to",topic);
			msg.put("kn_id",msg_id);

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
			System.err.println("publish() : ERROR : "+e.getMessage());
		}
		return msg_id;
	};

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
			HTTPResponse response;
			msg_id = Double.toString(Math.random()).substring(0,8);
			
			// add some things
			msg.put("kn_method","route");
			msg.put("kn_from",topic);
			msg.put("kn_uri",route_id);
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
			HTTPConnection tunnel;
			tunnel= createConnection(serverURI);
			response = tunnel.Post(basePath,form_data);

			// check for success
			if (response.getStatusCode() >= 200 && response.getStatusCode() < 300)
			{
				// add the listener
				EventStreamReader reader;
				reader = new EventStreamReader(response.getInputStream(),listener);
				eventStreams.put(route_id,reader);

				// start processing events
				new Thread(reader).run();
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
		if (false)
		try
		{
			Map msg = new HashMap();
			Router router = new Router("http://topiczero.com:8000/kn/");

			msg.put("type","comment");
			msg.put("nickname","Snow White");
			msg.put("comment","Someday my prince will come");
			router.publish("/topiczero.com/news/",msg);
		}
		catch(Exception e)
		{
			System.err.println("ERROR : " +e.getMessage());
		}

		// subscribe
		try
		{
			Router router = new Router("http://topiczero.com:8000/kn/");
			Listener listener = new DebugListener();
			router.subscribe("http://topiczero.com:8000/kn/who/anonymous/s/25933979/kn_journal;kn_id=727538",listener,null);
		}
		catch(Exception e)
		{
		}
		
	}
}
