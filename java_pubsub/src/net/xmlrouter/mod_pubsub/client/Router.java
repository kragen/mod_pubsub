/*
 * Router provide basic publish/subscribe capabilities.
 *
 */
package net.xmlrouter.mod_pubsub.client;

import java.util.Map;

/**
 * @author msg
 * 
 * Router provide basic publish/subscribe capabilities.
 *
 */
public interface Router {
	/**
	 * Post a message to the specified topic
	 * @param topic the destination
	 * @param msg the message
	 */	
	public abstract void publish(String topic, Map msg);
	
	/**
	 * Subscribe the specified listener to the specified topic.
	 * @param topic the source
	 * @param listener the listener
	 * @param options
	 * @return a route id
	 */
	public abstract String subscribe(
		String topic,
		Listener listener,
		Map options);
		
	/**
	 * Establish a route between a source and a destination.
	 * @param topic the source
	 * @param listener the listener
	 * @param options
	 * @return a route id
	 */
	public abstract String subscribe(String from, String to, Map options);
	
	/**
	 * Stop processing incoming events
	 * @param route_id a route id provided by the subscribe() method
	 * @see #subscribe
	 */
	public abstract void unsubscribe(String route_id);
}