/*
 * Created on Mar 24, 2004
 *
 */
package org.mod_pubsub.client.command;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.mod_pubsub.client.Constants;

/**
 * @author rtl
 *
 */
public class CommandOptions implements Constants {
	
	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	/**
	 * 
	 */
	public String getMaxAge() {
		return get(KN_MAX_AGE_PARAM);
	}

	/**
	 * 
	 */
	public int getMaxCount() {
		String aValue = get(KN_MAX_COUNT_PARAM);
		if (null != aValue)
			return Integer.valueOf(aValue).intValue();
		else {
			return -1;
		}
	}

	/**
	 * 
	 */
	public String getCheckpoint() {
		return get(KN_SINCE_CHECKPOINT_PARAM);
	}
	
	public String get(String theName) {
		return (String) myElements.get(theName);
	}

	/* 
	 * 
	 */
	public Map getAllOptions() {
		return Collections.unmodifiableMap(myElements);
	}

	//===========================================================================
	// COMMAND METHODS
	//===========================================================================
	
	public void setMaxAge(String theMaxAge) {
		set(KN_MAX_AGE_PARAM, theMaxAge);
	}

	public void setMaxCount(int theMaxCount) {
		set(KN_MAX_COUNT_PARAM, Integer.toString(theMaxCount));
	}

	public void setCheckpoint(String theCheckpoint) {
		set(KN_SINCE_CHECKPOINT_PARAM, theCheckpoint);
	}
	
	public void set(String theName, String theValue) {
		myElements.put(theName, theValue);
	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================
	
	Map myElements = new HashMap();

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

}
