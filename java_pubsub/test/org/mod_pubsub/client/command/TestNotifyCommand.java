/*
 * Created on Mar 21, 2004
 *
 */
package org.mod_pubsub.client.command;

import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

import junit.framework.TestCase;

import org.mod_pubsub.client.Constants;
import org.mod_pubsub.client.Event;
import org.mod_pubsub.client.IEvent;

/**
 * @author rtl
 *
 */
public class TestNotifyCommand extends TestCase implements Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	/* (non-Javadoc)
	 * @see junit.framework.TestCase#setUp()
	 */
	protected void setUp() throws Exception {
		super.setUp();
		myServerURL = new URL("http", "localhost", myTestPort, "/kn/");
		myEvent = new Event("testID", "This is a test payload");
		myEvent.set("test1", "1");
		myEvent.set("test2", "2");
		myValidTopicURI = new URI("/what/test/notifyCommand");
		myValidCommand = new NotifyCommand(myValidTopicURI, myEvent);
	}

	//===========================================================================
	// TEST METHODS
	//===========================================================================

	public void testDefaultElementsSetupCorrectly() throws URISyntaxException {

		assertEquals("notify", myValidCommand.getMethod());
		assertEquals(myValidTopicURI.toString(), myValidCommand.getTo());
		assertEquals("testID", myValidCommand.getId());
		assertEquals(
			"This is a test payload",
			myValidCommand.getParamValue(KN_PAYLOAD_PARAM));
		assertEquals("1", myValidCommand.getParamValue("test1"));
		assertEquals("2", myValidCommand.getParamValue("test2"));
	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	private NotifyCommand myValidCommand;
	private URI myValidTopicURI;
	private URL myServerURL;
	private IEvent myEvent;
	private int myTestPort = 4949;

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================


}
