/*
 * Created on Mar 24, 2004
 *
 */
package org.mod_pubsub.client.command;

import java.net.URI;
import java.net.URISyntaxException;

import org.mod_pubsub.client.Constants;

import junit.framework.TestCase;

/**
 * @author rtl
 *
 */
public class TestDeleteRouteCommand extends TestCase implements Constants {
	
	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	//===========================================================================
	// TEST METHODS
	//===========================================================================
	
	public void testFailsWithRouteLocationURIMissingKn_Routes() throws URISyntaxException {
		URI anInvalidRouteLocation = new URI("/this/is/an/invalid/route/location/uri");
		try {
			DeleteRouteCommand aCommand = new DeleteRouteCommand(anInvalidRouteLocation);
			fail("Should throw IllegalArgumentException "); 
		} catch (IllegalArgumentException ex) {
			// expected
		}
	}

	public void testFailsWithRouteLocationURIMissingId() throws URISyntaxException {
		URI anInvalidRouteLocation = new URI("/this/is/an/invalid/kn_routes/");
		try {
			DeleteRouteCommand aCommand = new DeleteRouteCommand(anInvalidRouteLocation);
			fail("Should throw IllegalArgumentException "); 
		} catch (IllegalArgumentException ex) {
			// expected
		}
		
		anInvalidRouteLocation = new URI("/this/is/an/invalid/kn_routes");
		try {
			DeleteRouteCommand aCommand = new DeleteRouteCommand(anInvalidRouteLocation);
			fail("Should throw IllegalArgumentException "); 
		} catch (IllegalArgumentException ex) {
			// expected
		}
	}
	
	public void testSetupDefaultElements() throws URISyntaxException {
		final String anId = "location_uri_id";
		final URI aValidRouteLocation = new URI("/this/is/a/valid/kn_routes/" + anId);
		DeleteRouteCommand aCommand = new DeleteRouteCommand(aValidRouteLocation);
		assertEquals(
			"Incorrect route to, should be empty string",
			0,
			aCommand.getTo().length());
		assertEquals(
			"Incorrect route from, should be route location uri",
			aValidRouteLocation.toString(),
			aCommand.getFrom());
		assertEquals(
			"Incorrect method name",
			aCommand.getMethod(),
			"route");
		assertEquals("Incorrect ID", anId, aCommand.getId());
		assertEquals(
			"Incorrect expires",
			aCommand.getParamValue(KN_EXPIRES_PARAM),
			"+5");
	}

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
