/*
 * Created on Mar 19, 2004
 *
 */
package org.mod_pubsub.client;

/**
 * @author rtl
 * 
 * An interface for a status event
 *
 */
public interface IStatusEvent {

	/**
	 * Return true is the status event is a successful one, false otherwise
	 */
	public boolean wasSuccessful();
}
