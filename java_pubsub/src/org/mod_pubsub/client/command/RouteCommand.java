/*
 * Created on Mar 23, 2004
 *
 */
package org.mod_pubsub.client.command;

import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

/**
 * @author rtl
 *
 */
public class RouteCommand extends DestinationCommand {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	/**
	 * @param theCommandName
	 */
	public RouteCommand(URI theTopicURI, URL theJournalURL) {
		this(theTopicURI, theJournalURL, null);
	}
	
	/**
	 * @param theCommandName
	 */
	public RouteCommand(
		URI theTopicURI,
		URL theJournalURL,
		CommandOptions theCommandOptions) {

		super("route");
		setParamValue(KN_FROM_PARAM, theTopicURI.toString());
		setParamValue(KN_TO_PARAM, theJournalURL.toString());
		setParamValue(KN_ID_PARAM, createRandomString());
		String aRouteURI = getFrom() + KN_ROUTES_PATH + getId();
		setParamValue(KN_ROUTE_URI, aRouteURI);
		if (null != theCommandOptions)
			setParamValues(theCommandOptions.getAllOptions());
	}

	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	public URI getRouteURI() {
		URI retVal = null;
		try {
			String aRouteURI = getParamValue(KN_ROUTE_URI);
			if (null != aRouteURI)
				retVal = new URI(aRouteURI);
		} catch (URISyntaxException e) {
			// ignore - nothing we can do about it except dump message
			e.printStackTrace();
		}
		return retVal;
	}

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
