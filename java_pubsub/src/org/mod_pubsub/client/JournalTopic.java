/*
 * Created on Mar 20, 2004
 *
 */
package org.mod_pubsub.client;

import java.net.URI;
import java.net.URISyntaxException;



/**
 * @author rtl
 *
 */
public class JournalTopic implements Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	public JournalTopic() {
		this(DEFAULT_USERNAME);
	}

	public JournalTopic(String theUsername) {
		this(theUsername, new Double(Math.random()).toString().substring(2));
	}
	
	public JournalTopic(String theUsername, String theJournalSessionId) {
		myJournalTopic = "/" + DEFAULT_JOURNAL_URI_START
					+ "/"
					+ theUsername
					+ "/"
					+ DEFAULT_JOURNAL_URI_SESSION_PATH
					+ "/"
					+ theJournalSessionId
					+ "/"
					+ JOURNAL_TOPIC_NAME;
			try {
				URI aURI = new URI(myJournalTopic);
			} catch (URISyntaxException e) {
				throw new IllegalArgumentException(e.getLocalizedMessage());
			}	
	}
	
	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	public String getJournalTopic() {
		return myJournalTopic;
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

	private String myJournalTopic;

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

	public static final String JOURNAL_TOPIC_NAME = "kn_journal";
	
	protected static final String DEFAULT_JOURNAL_URI_START = "who";
	protected static final String DEFAULT_JOURNAL_URI_SESSION_PATH = "s";
	public static final String ANONYMOUS_BASE_JOURNAL_URI =
		"/"
			+ DEFAULT_JOURNAL_URI_START
			+ "/"
			+ DEFAULT_USERNAME
			+ "/"
			+ DEFAULT_JOURNAL_URI_SESSION_PATH;

}
