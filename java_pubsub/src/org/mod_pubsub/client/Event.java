/*
 * Created on Mar 21, 2004
 *
 */
package org.mod_pubsub.client;

import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * @author rtl
 *
 */
public class Event implements IEvent, Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	public Event() {
		this(null, null);
	}

	public Event(String thePayload) {
		this(null, thePayload);
	}

	public Event(String theId, String thePayload) {
		if (null != theId)
			set(KN_ID_PARAM, theId);
		else
			set(KN_ID_PARAM, createRandomString());
		if (null != thePayload)
			set(KN_PAYLOAD_PARAM, thePayload);
	}

	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	/* (non-Javadoc)
	 * @see org.mod_pubsub.client.IEvent#getPayload()
	 */
	public String getPayload() {
		return get(KN_PAYLOAD_PARAM);
	}

	/* (non-Javadoc)
	 * @see org.mod_pubsub.client.IEvent#getID()
	 */
	public String getId() {
		return get(KN_ID_PARAM);
	}

	/* (non-Javadoc)
	 * @see org.mod_pubsub.client.IEvent#get(java.lang.String)
	 */
	public String get(String theName) {
		return (String) myElements.get(theName);
	}

	/* (non-Javadoc)
	 * @see org.mod_pubsub.client.IEvent#getElementCount()
	 */
	public int getElementCount() {
		return myElements.size();
	}

	/* (non-Javadoc)
	 * @see org.mod_pubsub.client.IEvent#getEventElements()
	 */
	public Map getEventElements() {
		return Collections.unmodifiableMap(myElements);
	}

	//===========================================================================
	// COMMAND METHODS
	//===========================================================================

	/* (non-Javadoc)
	 * @see org.mod_pubsub.client.IEvent#set(java.lang.String, java.lang.String)
	 */
	public void set(String theName, String theParam) {
		myElements.put(theName, theParam);
	}

	/* (non-Javadoc)
	 * @see org.mod_pubsub.client.IEvent#remove(java.lang.String)
	 */
	public void remove(String theName) {
		myElements.remove(theName);
	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		StringBuffer aStringRep = new StringBuffer(super.toString());
		Iterator anIterator = myElements.keySet().iterator();
		while (anIterator.hasNext()) {
			String aName = (String) anIterator.next();
			aStringRep.append("\n").append(aName).append(":").append(
				(String) myElements.get(aName));
		}
		return aStringRep.toString();
	}

	protected String createRandomString() {
		return Double.toString(Math.random()).substring(2);
	}

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	private Map myElements = new HashMap();

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

}
