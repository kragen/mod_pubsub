/*
 * Created on Mar 19, 2004
 *
 */
package org.mod_pubsub.client.simple;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

import org.mod_pubsub.client.Constants;
import org.mod_pubsub.client.Event;
import org.mod_pubsub.client.IEvent;
import org.mod_pubsub.client.IEventHandler;
import org.mod_pubsub.client.IRequestStatusHandler;
import org.mod_pubsub.client.JournalTopic;
import org.mod_pubsub.client.command.*;

/**
 * @author rtl
 *
 */
public class SimpleClient implements Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	public SimpleClient(URL theServerUrl, String theClientID)
		throws MalformedURLException {
		myServerURL = theServerUrl;
		myJournalTopic = new JournalTopic(theClientID);
		myJournalURL = new URL(theServerUrl, myJournalTopic.getJournalTopic());
	}

	public SimpleClient(URL theServerUrl) throws MalformedURLException {
		this(theServerUrl, JournalTopic.DEFAULT_USERNAME);
	}

	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	/**
	 * @return
	 */
	protected URL getJournalURL() {
		return myJournalURL;
	}

	/**
	 * @return
	 */
	protected boolean isConnected() {
		return myConnectedFlag;
	}

	//===========================================================================
	// COMMAND METHODS
	//===========================================================================

	public boolean publish(
		URI theTopicURI,
		IEvent theEvent,
		IRequestStatusHandler theRequestStatusHandler) {

		// assume failure
		boolean retVal = false;

		// make sure we are connected to the server
		if (ensureConnected(theRequestStatusHandler)) {
			// create a notify command and execute it
			NotifyCommand aNotifyCommand =
				createNotifyCommand(theTopicURI, theEvent);
			retVal =
				aNotifyCommand.execute(myServerURL, theRequestStatusHandler);
		}

		return retVal;
	}

	public URI subscribe(
		URI theTopicURI,
		IEventHandler theEventHandler,
		IRequestStatusHandler theRequestStatusHandler) {

		return this.subscribe(
			theTopicURI,
			null,
			theEventHandler,
			theRequestStatusHandler);
	}

	public URI subscribe(
		URI theTopicURI,
		CommandOptions theCommandOptions,
		IEventHandler theEventHandler,
		IRequestStatusHandler theRequestStatusHandler) {

		// assume failure
		URI retVal = null;

		// make sure we are connected to the server
		if (ensureConnected(theRequestStatusHandler)) {
			// create a route command and execute it
			RouteCommand aRouteCommand =
				createRouteCommand(
					theTopicURI,
					getJournalURL(),
					theCommandOptions);
			boolean aSuccessFlag =
				aRouteCommand.execute(myServerURL, theRequestStatusHandler);

			// setup the URI to return if successful
			if (aSuccessFlag)
				retVal = aRouteCommand.getRouteURI();

			// if subscribed ok and an event handler supplied 
			// then attach it to the journal to receive events
			if (aSuccessFlag && null != theEventHandler) {
				myJournalCommand.attachEventHandler(
					theEventHandler,
					aRouteCommand.getRouteURI().toString());
			}
		}

		return retVal;
	}

	public boolean unsubscribe(
		URI theRouteLocationURI,
		IRequestStatusHandler theRequestStatusHandler) {
		// assume failure
		boolean retVal = false;

		// detach from the journal so as not to receive any events
		if (null != myJournalCommand) {
			// if was attached then we can unsubscribe from the server
			if (null != myJournalCommand.detachEventHandler(theRouteLocationURI.toString())) {
				retVal = true;
			}
		}

		// if was an active connection then make sure we are connected to the server
		if (retVal && ensureConnected(theRequestStatusHandler)) {
			// create a delete route command and execute it
			DeleteRouteCommand aDeleteRouteCommand =
				createDeleteRouteCommand(theRouteLocationURI);
			retVal =
				aDeleteRouteCommand.execute(
					myServerURL,
					theRequestStatusHandler);

		}

		return retVal;
	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	/**
	 * @param theRequestStatusHandler
	 * @return
	 */
	protected boolean connect(IRequestStatusHandler theRequestStatusHandler) {
		if (null == myJournalCommand) {
			// create a route command and execute it
			myJournalCommand = createJournalRouteCommand();
		}
		myConnectedFlag =
			myJournalCommand.execute(myServerURL, theRequestStatusHandler);
		return myConnectedFlag;
	}

	/**
	 * Factory method for creating a JournalRouteCommand
	 * 
	 * @return a JournalRouteCommand
	 */
	protected SimpleJournalRouteCommand createJournalRouteCommand() {
		return (new SimpleJournalRouteCommand(getJournalURL()));
	}

	/**
	 * Factory method for creating a NotifyCommand
	 * 
	 * @return a NotifyCommand
	 */
	protected NotifyCommand createNotifyCommand(
		URI theTopicURI,
		IEvent theEvent) {
		return (new NotifyCommand(theTopicURI, theEvent));
	}

	/**
	 * Factory method for creating a RouteCommand
	 * 
	 * @return a RouteCommand
	 */
	protected RouteCommand createRouteCommand(
		URI theTopicURI,
		URL theJournalURL,
		CommandOptions theCommandOptions) {
		return (
			new RouteCommand(theTopicURI, theJournalURL, theCommandOptions));
	}

	/**
	 * Factory method for creating a DeleteRouteCommand
	 * 
	 * @return a DeleteRouteCommand
	 */
	protected DeleteRouteCommand createDeleteRouteCommand(URI theRouteLocationURI) {
		return (new DeleteRouteCommand(theRouteLocationURI));
	}

	/**
	 * Helper method to make sure we are connected
	 * 
	 * @param theRequestStatusHandler - notifcation of status 
	 * @return - true if connected, false otherwise
	 */
	protected boolean ensureConnected(IRequestStatusHandler theRequestStatusHandler) {
		// assume we are connected
		boolean retVal = true;

		// make sure we are connected to the server
		// TODO - reconnection support
		if (!isConnected()) {
			// if cannot connect then fail out
			if (!connect(theRequestStatusHandler)) {
				retVal = false;
			}
		}

		return retVal;
	}

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	private URL myServerURL;
	private JournalTopic myJournalTopic;
	private URL myJournalURL;
	private boolean myConnectedFlag = false;
	private SimpleJournalRouteCommand myJournalCommand = null;

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

	//===========================================================================
	// STATIC METHODS 
	//===========================================================================

	public static void main(String args[])
		throws MalformedURLException, URISyntaxException {

		System.setProperty(
			"org.apache.commons.logging.Log",
			"org.apache.commons.logging.impl.SimpleLog");
		System.setProperty(
			"org.apache.commons.logging.simplelog.showdatetime",
			"true");
		System.setProperty(
			"org.apache.commons.logging.simplelog.log.httpclient.wire",
			"debug");
		System.setProperty(
			"org.apache.commons.logging.simplelog.log.org.apache.commons.httpclient",
			"debug");

		SimpleClient client =
			new SimpleClient(
			//new URL("http://www.mod-pubsub.org:9000/kn"),
			new URL("http://127.0.0.1:6789/kn"), 
			"SimpleClientTest");

		URI aTopicURL = new URI("/what/chat");

		// create an event handler that just dumps to system.out
		IEventHandler anEventHandler = new IEventHandler() {

			public void onEvent(IEvent theEvent) {
				System.out.println(theEvent);
			}
		};

		String id = "Simple Client Test Display Name";
		CommandOptions someCommandOptions = new CommandOptions();
		someCommandOptions.set("displayname", id);

		// subscribe
		URI aRouteLocation = client.subscribe(aTopicURL, someCommandOptions, anEventHandler, null);

		// publish
		try {

			long cnt = 0L;
			while (cnt < 3) {
				Event anEvent =
					new Event("Test from new java client library:" + cnt++);
				anEvent.set("displayname", id);
				//anEvent.set("userid", id);
				//anEvent.set("client_id", id);
				client.publish(aTopicURL, anEvent, null);
				Thread.sleep(10000);
			}
			
			client.unsubscribe(aRouteLocation, null);
			Event anEvent =
				new Event(
					"Test from new java client library:" + cnt++);
			anEvent.set("displayname", id);
			anEvent.set("userid", id);
			anEvent.set("client_id", id);
			client.publish(aTopicURL, anEvent, null);
			Thread.sleep(10000);
			
			while (true) {
			}

		} catch (Exception e) {
			System.err.println("ERROR : " + e.getMessage());
		}
	}

}
