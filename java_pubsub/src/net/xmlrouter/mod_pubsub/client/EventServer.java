package net.xmlrouter.mod_pubsub.client;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

/**
 * @author msg
 * Event router that uses mod-pubsub compliant server for network messaging.
 * The openConnection() method opens a persistent connection that can deliver messages to the client 
 */
public class EventServer {
	private static String ROUTES_STR = "/kn_routes/";
	String serverURI;
	String basePath;

	/**
	 * @param server URI of message server (ex. http://localhost/kn/)
	 */
	public EventServer(String uri) throws MalformedURLException {
		this.serverURI = uri;

		try {
			new URL(uri);
			basePath = "";
		} catch (Exception e) {
			throw new MalformedURLException(uri);
		}
	}

	public String getMessageId() {
		return Double.toString(Math.random()).substring(2, 10);
	}

	public String getRouteId(String topic, String msg_id) {
		return serverURI + topic + ROUTES_STR + msg_id;
	}
	public String getJournalId(String username)
	{
		return "/who/"+username+"/s/"+getMessageId()+"/kn_journal";
	}

	public String getTopicFromRoute(String route_id) {
		String retVal = "";

		int topicsIdx = route_id.indexOf(ROUTES_STR);
		if (route_id.startsWith(serverURI) && topicsIdx != -1) {

			retVal = route_id.substring(serverURI.length(), topicsIdx);
		}

		return retVal;
	}
	public String getIdFromRoute(String route_id) {
		String retVal = "";

		int topicsIdx = route_id.indexOf(ROUTES_STR);
		if (topicsIdx != -1) {

			retVal = route_id.substring(topicsIdx + ROUTES_STR.length());
		}

		return retVal;
	}
	
	private void consumeResponse(java.net.HttpURLConnection conn)
		throws IOException
	{
		byte buffer[] = new byte[1024];
		InputStream is = conn.getInputStream();
		while (is.read(buffer) > 0);
	}

	/**
	 * Post a message to the specified topic on the pubsub server.
	 * @param topic the destination
	 * @param msg the message
	 */
	public void publish(String topic, Map msg) {
		try {
			URL url = new URL(serverURI + basePath);
			HttpURLConnection conn;

			// add some things
			msg.put("do_method", "notify");
			msg.put("kn_to", topic);

			// send message
			conn = HTTPUtil.Post(url, msg);

			// get response
			if (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300) {
				consumeResponse(conn);
			}
		} catch (Exception e) {
			System.err.println("publish() : ERROR : " + e.getMessage());
		}
	}

	/**
	 * Open a persistent connection with the pubsub server.
	 * This creates a 'journal' topic and starts listening to it.
	 * 
	 * @param topic the source
	 * @param listener the listener
	 * @param options
	 * @return a route id which can be used with the unsubscribe() method
	 */
	public Connection openConnection(String username,String password, Listener listener, Map options) 
	{
		Connection connection=null;
		String journal=null;
		String route_id = null;

		// generate an id for the route we will create so we can delete it later
		route_id = getRouteId("net.xmlrouter.mod_pubsub.client", getMessageId());
		
		// generate standard 'session tunnel' formatted URL
		journal= getJournalId(username);		
		
		Map msg = new HashMap();
		try {
			URL url = new URL(serverURI + basePath);

			// add some things
			msg.put("kn_response_format", "simple");
			msg.put("do_method", "route");
			msg.put("kn_from", journal);
			msg.put("kn_id", route_id);
			if (null != options) msg.putAll(options);

			connection = new Connection(this,journal,url,msg,listener);
			connection.open();
			
		} catch (Exception e) {
			e.printStackTrace();
			System.err.println("openConnection() : ERROR : " + e.getMessage());
		}

		return connection;
	}


	/**
	 * Establish a route between a source and a destination on the pubsub server.
	 * 
	 * @param topic the source
	 * @param listener the listener
	 * @param options
	 * @return a route id
	 */
	public String subscribe(String from, String to, Map options) {
		String route_id = null;
		String msg_id = getMessageId();

		// generate an id for the route we will create so we can delete it later
		route_id = getRouteId(from, msg_id);

		Map msg = new HashMap();
		java.net.HttpURLConnection conn;
		try {
			String uri;
			uri = serverURI + basePath;
			URL url = new URL(uri);

			// add some things
			msg.put("do_method", "route");
			msg.put("kn_from", from);
			msg.put("kn_to", to);
			//msg.put("kn_id",route_id);	// should we do this or not??
			//msg.put("do_max_age","3600");
			if (null != options) msg.putAll(options);

			// send message
			conn = HTTPUtil.Post(url, msg);

			// get response
			if (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300) 
			{
			} 
			else 
			{
				System.err.println("subscribe() : ERROR : " + conn.getResponseMessage());
			}

			// consume response entity
			consumeResponse(conn);
		} catch (Exception e) {
			e.printStackTrace();
			System.err.println("subscribe() : ERROR : " + e.getMessage());
		}

		return route_id;
	}

	/**
	 * Remove a route between a source and a destination on the pubsub server.
	 * 
	 * @param route_id a route id from subscribe()
	 * @see subscribe
	 */
	public void unsubscribe(String route_id) {
		Map msg = new HashMap();
		try {
			java.net.HttpURLConnection conn;
			String uri;
			
			uri = serverURI + basePath;
			URL url = new URL(uri);

			// add some things
			msg.put("do_method", "route");
			msg.put("kn_from", getTopicFromRoute(route_id));
			msg.put("kn_id", getIdFromRoute(route_id));
			msg.put("kn_to", "");

			conn = HTTPUtil.Post(url, msg);

			// check for errors
			if (conn.getResponseCode() >= 300) {
				consumeResponse(conn);
			}
		} catch (Exception e) {
			System.err.println("unsubscribe() : ERROR : " + e.getMessage());
		}
	}

	public static void main(String args[]) {
		// subscribe
		try {
			EventServer server = new EventServer("http://www.mod-pubsub.org:9000/kn");
			Connection connection;
			EventStreamReader reader;
			Listener listener = new DebugListener();
			
			connection=server.openConnection("snowwhite",null,listener,null);
			reader = new EventStreamReader(connection.getEvents(),listener);
			new Thread(reader).start();
			
			connection.subscribe("/what/chat",null);
		} 
		catch (Exception e) 
		{
			System.err.println("main() : ERROR : unable to subscribe : "+e.getMessage());
		}

		// publish
		try {
			Map msg = new HashMap();
			EventServer router = new EventServer("http://www.mod-pubsub.org:9000/kn");

			msg.put("displayname", "Snow White");
			msg.put("kn_payload", "Someday my prince will come");
			router.publish("/what/chat", msg);
		} catch (Exception e) {
			System.err.println("main() : ERROR : unable to publish : " + e.getMessage());
		}
	}
}
