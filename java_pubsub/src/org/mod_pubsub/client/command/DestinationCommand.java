/*
 * Created on Mar 21, 2004
 *
 */
package org.mod_pubsub.client.command;


/**
 * @author rtl
 *
 */
public abstract class DestinationCommand extends BaseCommand {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================
	
	protected DestinationCommand(String theCommandName) {
		super(theCommandName);
	}

	/**
	 * @return
	 */
	public String getTo() {
		return getParamValue(KN_TO_PARAM);
	}

	/**
	 * @return
	 */
	public String getFrom() {
		return getParamValue(KN_FROM_PARAM);
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