/*
 * Created on Mar 19, 2004
 *
 */
package org.mod_pubsub.client;

import java.util.Map;

/**
 * @author rtl
 *
 */
public interface IEvent {
	
	//===========================================================================
	// QUERY METHODS
	//===========================================================================
	
	/**
	 * Convenience accessor for the payload of this event
	 * 
	 * @return the payload of this event
	 */
	public String getPayload();

	/**
	 * Convenience accessor for the id of this event
	 * 
	 * @return the id of this event
	 */
	public String getId();

	/**
	 * General purpose access to a named event element
	 * 
	 * @param theName - name of the event element to return
	 * @return the event element value if exists, null otherwise
	 */
	public String get(String theName);
	
	/**
	 * Access all the event elements as a (readonly) map
	 * 
	 * @return a readonly Map containing all the event elements
	 */
	public Map getEventElements();
	
	/**
	 * How many event elements are in the event?
	 * 
	 * @return - the count of the event elements
	 */
	public int getElementCount();

	
	//===========================================================================
	// COMMAND METHODS
	//===========================================================================

	/**
	 * Set the value of the named event element
	 * 
	 * @param theName - name of the event element to set
	 * @param theValue - the value to set
	 */
	public void set(String theName, String theParam);
	
	/**
	 * Remove the named event element
	 * @param theName - name of the event element to remove
	 */
	public void remove(String theName);

}
