/*
 * Created on Mar 19, 2004
 *
 */
package org.mod_pubsub.client;

/**
 * @author rtl
 *
 */
public interface IRequestStatusHandler {

	/**
	 * This method gets called whenever there is an error from the server.
	 */
	public void onError(IStatusEvent theStatusEvent);

	/**
	 * This method gets called whenever the status is successful from the server.
	 */
	public void onSuccess(IStatusEvent theStatusEvent);
	
}
