/*
 * Created on Mar 24, 2004
 *
 */
package org.mod_pubsub.client.command;

import java.net.URI;

/**
 * @author rtl
 *
 */
public class DeleteRouteCommand extends DestinationCommand {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	public DeleteRouteCommand(URI theRouteLocationURI) {
		super("route");
		// check that route location is valid, i.e. contains /kn_route/
		String aPath = theRouteLocationURI.getPath();
		int idx = aPath.indexOf(KN_ROUTES_PATH);
		if (idx == -1) {
			throw new IllegalArgumentException(
				"The route location uri must contain " + KN_ROUTES_PATH);
		}
		if (idx + KN_ROUTES_PATH.length() >= aPath.length()) {
			throw new IllegalArgumentException(
				"The route location uri must contain an id after " + KN_ROUTES_PATH);
		}
		
		String id = aPath.substring(idx + KN_ROUTES_PATH.length());

		setParamValue(KN_TO_PARAM, "");
		setParamValue(KN_EXPIRES_PARAM, "+5");
		setParamValue(KN_FROM_PARAM, theRouteLocationURI.toString());
		setParamValue(KN_ID_PARAM, id);

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
