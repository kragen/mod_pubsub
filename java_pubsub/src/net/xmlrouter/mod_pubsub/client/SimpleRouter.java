package net.xmlrouter.mod_pubsub.client;
import java.net.MalformedURLException;
import java.util.HashMap;
import java.util.Map;

/**
 * @author msg
 * Event router that uses mod-pubsub compliant server for network messaging.
 * The subscribe() method opens a persistent connection and starts a thread which delivers
 * incoming messages to a message listener. 
 */
public class SimpleRouter implements Router {
	private static String ROUTES_STR = "/kn_routes/";
	String serverURI;
	String basePath;
	EventServer server;
	Connection connection;
	Dispatcher dispatcher;
	
	Map eventStreams = new HashMap();

	/**
	 * @param server URI of message server (ex. http://localhost/kn/)
	 */
	public SimpleRouter(String uri) throws MalformedURLException{
		this.serverURI = uri;
		this.server = new EventServer(uri);
		this.dispatcher = new Dispatcher(uri);
	}

	/**
	 * Get current connection or open new one.
	 * @return
	 */
	Connection acquireConnection()
	{
		if (connection == null)
		{
			connection=server.openConnection(null,null,dispatcher,null);
			new Thread(connection).start();
		}
		return connection;
	}
	
	/**
	 * Post a message to the specified topic on the pubsub server.
	 * @param topic the destination
	 * @param msg the message
	 */
	public void publish(String topic, Map msg) {
		server.publish(topic,msg);
	}

	/**
	 * Subscribe the specified listener to the specified topic.
	 * This opens a persistent connection with the pubsub server and starts a thread
	 * that delivers messages from the pubsub server to the specified listener.
	 * Each call to this method will create a new connection and thread.
	 * 
	 * @param topic the source
	 * @param listener the listener
	 * @param options
	 * @return a route id which can be used with the unsubscribe() method
	 */
	public String subscribe(String topic, Listener listener, Map options) {
		String routeId;
		Connection conn;

		// hook up local listener
		routeId=dispatcher.subscribe(topic,listener,options);
		
		// hook up remote topic to our connection
		conn = acquireConnection();
		conn.subscribe(topic,options);
		
		return routeId;
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
		return server.subscribe(from,to,options);
	}

	/**
	 * If the route is a local route, this will stop processing incoming events. 
	 * This closes the network connection and stops the thread for this particular route.
	 * 
	 * If the route is not a local route, this will unsubscribe the remote topics
	 * previosly connected via subscribe() method.
	 * 
	 * @param route_id a route id provided by the subscribe() method
	 * @see #subscribe
	 */
	public void unsubscribe(String route_id) {
		if (dispatcher.hasRoute(route_id))
			dispatcher.unsubscribe(route_id);
		else
			server.unsubscribe(route_id);
	}


	public static void main(String args[]) {
		// subscribe
		try {
			SimpleRouter router = new SimpleRouter("http://www.mod-pubsub.org:9000/kn");
			Listener listener = new DebugListener();
			router.subscribe("/what/chat", listener, null);
		} catch (Exception e) {
		}

		// publish
		try {
			Map msg = new HashMap();
			SimpleRouter router = new SimpleRouter("http://www.mod-pubsub.org:9000/kn");

			msg.put("displayname", "Snow White");
			msg.put("kn_payload", "Someday my prince will come");
			router.publish("/what/chat", msg);
		} catch (Exception e) {
			System.err.println("ERROR : " + e.getMessage());
		}
	}
}
