package net.xmlrouter.mod_pubsub.client;
import java.util.HashMap;
import java.util.Map;
import java.util.*;

/**
 * @author msg
 * Simple one-to-many local router for publish/subscribe.
 * This delivers incoming messages to each registered listener
 */
public class Dispatcher implements Listener {
	/**
	 * map from topic to list of listeners
	 */
	Map topics = new HashMap();	
	/**
	 * map from route_id to listener
	 */
	Map routes = new HashMap();	
	String baseTopic;
	
	/**
	 * Local event router. Dispatches messages to all listeners of a topic.
	 *
	 */
	public Dispatcher(String baseTopic) {
		this.baseTopic=baseTopic;
	}

	/**
	 * Get list of listeners for a topic.
	 * @param topic
	 * @return list of listeners
	 */
	List getListeners(String topic)
	{
		return (List)topics.get(topic);
	}
	
	/**
	 * Generate random number
	 * @return random number suitable for a message ID
	 */
	public String getMessageId() {
		return Double.toString(Math.random()).substring(2, 10);
	}

	/**
	 * Generate unique sub-topic used for identifying a listener
	 * @param topic
	 * @param msg_id unique string
	 * @return formatted text that can be used to identify a single listener
	 */
	public String getRouteId(String topic, String msg_id) {
		return topic + "/" + msg_id;
	}
	private String getTopicFromRoute(String route_id)
	{
		return route_id.substring(0,route_id.lastIndexOf('/'));
	}
	
	public boolean hasRoute(String route_id)
	{
		return routes.containsKey(route_id);
	}

	/**
	 * Route message to destination specified within the message.
	 * @param msg the message to dispatch 
	 */
	public void onMessage(Map msg)
	{
		String from;
		String topic;
		
		// get destination		
		from = (String)msg.get("kn_routed_from");
		if (from == null)
			return;
		
		if (from.startsWith(baseTopic))
			topic = from.substring(baseTopic.length());
		else
			return;
			
		// route
		publish(topic,msg);
	}

	/**
	 * Delivers the message to all listeners of the specified topic
	 * 
	 * @param topic the destination
	 * @param msg the message
	 */
	public void publish(String topic, Map msg) {
		List listeners;
		Listener listener;
		
		listeners = getListeners(topic);
		if (listeners != null)
		{
			for (int i=0; i < listeners.size(); i++)
			{
				listener = (Listener)listeners.get(i);
				listener.onMessage(msg);	
			}	
		}
		else
		{
			System.out.println("no listeners for topic "+topic);
		}
	}

	/**
	 * Subscribe the listener to the specified topic.
	 * @param topic the source
	 * @param listener the listener
	 * @param options
	 * @return a route id which can be used with unsubscribe()
	 */
	public String subscribe(String topic, Listener listener, Map options) {
		String route_id = null;
		String random = getMessageId();
		List listeners;

		if (this == listener)
			throw new RuntimeException("Circular subscription");

		// generate magic route id
		route_id = getRouteId(topic, random);
		routes.put(route_id,listener);

		// add to list of listeners for this topic
		listeners=getListeners(topic);
		if (listeners == null)
		{
			listeners= new ArrayList();
			topics.put(topic,listeners);
		}	
		listeners.add(listener);

		return route_id;
	}
	
	
	/**
	 * Remove an individual listener from its associated topic
	 * @param route_id a route id provided by the subscribe() method
	 * @see #subscribe
	 */
	public void unsubscribe(String route_id) {
		Listener listener;
		String topic;
		List listeners;
		
		listener = (Listener)routes.get(route_id);
		routes.remove(listener);
		
		topic = getTopicFromRoute(route_id);
		listeners = getListeners(topic);
		if (listeners != null)
			listeners.remove(listener);
	}


	public static void main(String args[]) {
	}
}
