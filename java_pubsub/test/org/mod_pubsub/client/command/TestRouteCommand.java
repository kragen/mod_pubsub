/*
 * Created on Mar 23, 2004
 *
 */
package org.mod_pubsub.client.command;

import java.net.URI;
import java.net.URL;

import junit.framework.TestCase;

import org.mod_pubsub.client.Constants;

/**
 * @author rtl
 *
 */
public class TestRouteCommand extends TestCase implements Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	/* (non-Javadoc)
	 * @see junit.framework.TestCase#setUp()
	 */
	protected void setUp() throws Exception {
		super.setUp();
		myServerURL = new URL("http", "localhost", theirTestPort, "/kn/");
		myValidTopicURI = new URI("/what/testTopic");
		myValidJournalUrl =
			new URL(myServerURL, "/who/anonymous/s/1234/kn_journal");
		myValidCommand = new RouteCommand(myValidTopicURI, myValidJournalUrl);
	}

	//===========================================================================
	// TEST METHODS
	//===========================================================================

	public void testDefaultElementsSetupCorrectly() {
		assertEquals(
			"Incorrect route to, should be journal path",
			myValidJournalUrl.toString(),
			myValidCommand.getTo());
		assertEquals(
			"Incorrect route from, should be topic uri",
			myValidTopicURI.toString(),
			myValidCommand.getFrom());
		assertEquals(
			"Incorrect method name",
			myValidCommand.getMethod(),
			"route");
		assertNotNull("Missing ID", myValidCommand.getId());
		URI aRouteURI = myValidCommand.getRouteURI();
		assertNotNull("Missing route uri", aRouteURI);
		assertTrue(
			"Route uri must start with from uri",
			aRouteURI.toString().startsWith(myValidCommand.getFrom()));
		assertTrue(
			"Route uri must contain kn_routes",
			aRouteURI.toString().indexOf(RouteCommand.KN_ROUTES_PATH) > -1);
		assertTrue(
			"Route uri must end with id",
			aRouteURI.toString().endsWith(myValidCommand.getId()));
	}

	public void testElementsSetupCorrectlyWithOptions() {

		CommandOptions someCommandOptions = new CommandOptions();
		final String aMaxAge = "99";
		someCommandOptions.setMaxAge(aMaxAge);
		final int aMaxCnt = 987;
		someCommandOptions.setMaxCount(aMaxCnt);
		String aCheckpoint = "cpt";
		someCommandOptions.setCheckpoint(aCheckpoint);
		final String aNewID = "fixedID";
		someCommandOptions.set(KN_ID_PARAM, aNewID);

		RouteCommand aRouteCommand =
			new RouteCommand(
				myValidTopicURI,
				myValidJournalUrl,
				someCommandOptions);
		assertEquals(
			"Incorrect max age",
			aRouteCommand.getParamValue(KN_MAX_AGE_PARAM),
			aMaxAge);
		assertEquals(
			"Incorrect max count",
			Integer.parseInt(aRouteCommand.getParamValue(KN_MAX_COUNT_PARAM)),
			aMaxCnt);
		assertEquals(
			"Incorrect cpt",
			aRouteCommand.getParamValue(KN_SINCE_CHECKPOINT_PARAM),
			aCheckpoint);
		assertEquals("Should have fixed id now", aRouteCommand.getId(), aNewID);
	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	private URL myServerURL;
	private URL myValidJournalUrl;
	private URI myValidTopicURI;
	private RouteCommand myValidCommand;

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

	private static final int theirTestPort = 4949;

}
