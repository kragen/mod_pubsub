/*
 * Created on Mar 20, 2004
 *
 */
package org.mod_pubsub.client;

import java.net.URI;
import java.net.URISyntaxException;

import junit.framework.TestCase;

/**
 * @author rtl
 *
 */
public class TestJournalTopic extends TestCase {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	//===========================================================================
	// TEST METHODS
	//===========================================================================

	public void testJournalURIDefaultsToAnonymousAndRandomSessionID() {

		final String anExpectedJournalURIStart =
			JournalTopic.ANONYMOUS_BASE_JOURNAL_URI + "/";

		String aJournalURI1 = new JournalTopic().getJournalTopic();
		
		String aSessionID1 =
			validateJournalTopicURI(anExpectedJournalURIStart, aJournalURI1);

		// get a second journal uri to make sure session id is different
		String aJournalURI2 = new JournalTopic().getJournalTopic();
		assertNotNull(aJournalURI2);

		String aSessionID2 =
			validateJournalTopicURI(anExpectedJournalURIStart, aJournalURI2);
		assertFalse(
			"2 journal uri's have same session id's: "
				+ aSessionID1
				+ " and "
				+ aSessionID2,
			aSessionID1.equalsIgnoreCase(aSessionID2));
	}

	public void testJournalURIUsesClientIDAndRandomSessionId() {

		final String aUserID = "testUser";

		final String anExpectedJournalURIStart =
			"/"
				+ JournalTopic.DEFAULT_JOURNAL_URI_START
				+ "/"
				+ aUserID
				+ "/"
				+ JournalTopic.DEFAULT_JOURNAL_URI_SESSION_PATH
				+ "/";

		String aJournalURI1 = new JournalTopic(aUserID).getJournalTopic();
		String aSessionID1 =
			validateJournalTopicURI(anExpectedJournalURIStart, aJournalURI1);

		// get a second jounral uri to make sure session id is different
		String aJournalURI2 = new JournalTopic(aUserID).getJournalTopic();
		assertNotNull(aJournalURI2);

		String aSessionID2 =
			validateJournalTopicURI(anExpectedJournalURIStart, aJournalURI2);
		assertFalse(
			"2 journal uri's have same session id's: "
				+ aSessionID1
				+ " and "
				+ aSessionID2,
			aSessionID1.equalsIgnoreCase(aSessionID2));
	}

	public void testJournalURIUsesClientIDAndSpecifiedSessionId() {

		final String aUserID = "testUser";
		final String aSessionID = "testSessionID";

		final String anExpectedJournalURIStart =
			"/"
				+ JournalTopic.DEFAULT_JOURNAL_URI_START
				+ "/"
				+ aUserID
				+ "/"
				+ JournalTopic.DEFAULT_JOURNAL_URI_SESSION_PATH
				+ "/";

		String aJournalURI1 =
			new JournalTopic(aUserID, aSessionID).getJournalTopic();
		String aSessionID1 =
			validateJournalTopicURI(anExpectedJournalURIStart, aJournalURI1);

		// get a second jounral uri to make sure session id is the same
		String aJournalURI2 =
			new JournalTopic(aUserID, aSessionID).getJournalTopic();
		String aSessionID2 =
			validateJournalTopicURI(anExpectedJournalURIStart, aJournalURI2);
		assertTrue(
			"2 journal uri's have different session id's: "
				+ aSessionID1
				+ " and "
				+ aSessionID2,
			aSessionID1.equalsIgnoreCase(aSessionID2));
	}

	public void testJournalURIFailsIfClientIDRequiringEscaping() {

		final String aUserID = "test+user name";

		final String anExpectedJournalURIStart =
			"/"
				+ JournalTopic.DEFAULT_JOURNAL_URI_START
				+ "/"
				+ aUserID
				+ "/"
				+ JournalTopic.DEFAULT_JOURNAL_URI_SESSION_PATH
				+ "/";

		try {
			String aJournalURI1 = new JournalTopic(aUserID).getJournalTopic();
			fail("Should not permit an invalid user id");
		} catch (IllegalArgumentException ex) {
			// expected
		}

	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	private String validateJournalTopicURI(
		final String theExpectedJournalURIStart,
		final String theJournalURI) {

		//Logger.global.info(theExpectedJournalURIStart + "||" + theJournalURI);

		assertNotNull(theJournalURI);
		
		// make sure produces valid uri
		try {
			URI aURI = new URI(theJournalURI);
		} catch (URISyntaxException e) {
			e.printStackTrace();
			fail("Should produce a valid URI");
		}
		
		assertTrue(
			"Not a valid journal uri",
			theJournalURI.endsWith(JournalTopic.JOURNAL_TOPIC_NAME));
		assertTrue(
			"Not the expected journal start: " + theExpectedJournalURIStart,
			theJournalURI.startsWith(theExpectedJournalURIStart));

		String aSessionID =
			theJournalURI.substring(
				theExpectedJournalURIStart.length(),
				theJournalURI.length()
					- JournalTopic.JOURNAL_TOPIC_NAME.length()
					- 1);
		assertTrue(
			"Session ID is only a small no. of characters: " + aSessionID,
			aSessionID.length() > 3);
		return aSessionID;
	}

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

}
