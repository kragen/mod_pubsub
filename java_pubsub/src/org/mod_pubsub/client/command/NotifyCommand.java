/*
 * Created on Mar 21, 2004
 *
 */
package org.mod_pubsub.client.command;

import java.net.URI;

import org.mod_pubsub.client.IEvent;

/**
 * @author rtl
 *
 */
public class NotifyCommand extends DestinationCommand {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	public NotifyCommand(URI theTopicURI, IEvent theEvent) {
		super("notify");
		setParamValue(KN_TO_PARAM, theTopicURI.toString());
		setParamValue(KN_ID_PARAM, createRandomString());

		// copy everything from event to this command
		setParamValues(theEvent.getEventElements());
	}

	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	//===========================================================================
	// COMMAND METHODS
	//===========================================================================

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

}
