/*
 * Created on Mar 20, 2004
 *
 */
package org.mod_pubsub.client.simple;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.httpclient.methods.PostMethod;
import org.mod_pubsub.client.Event;
import org.mod_pubsub.client.IEventHandler;
import org.mod_pubsub.client.JournalTopic;
import org.mod_pubsub.client.command.DestinationCommand;

/**
 * @author rtl
 *
 */
public class SimpleJournalRouteCommand extends DestinationCommand {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	public SimpleJournalRouteCommand(URL theJournalURL) {
		super("route");

		if (!theJournalURL
			.toString()
			.endsWith(JournalTopic.JOURNAL_TOPIC_NAME)) {
			throw new IllegalArgumentException(
				"Journal URL must end with " + JournalTopic.JOURNAL_TOPIC_NAME);
		}

		// TODO - ensure that the journal url is an absolute url, 
		// i.e. prepend the server url to any relative url's  

		setParamValue(KN_FROM_PARAM, theJournalURL.toString());
		setParamValue(KN_TO_PARAM, KN_TO_HTTP_STREAM);
		setParamValue(
			KN_STATUS_FROM_PARAM,
			"javascript://tempuri.org/" + createRandomString());
		setParamValue(KN_RESPONSE_FORMAT_PARAM, KN_SIMPLE_RESPONSE_FORMAT);
	}

	/**
	 * @return
	 */
	public String getStatusFrom() {
		return getParamValue(KN_STATUS_FROM_PARAM);
	}

	/**
	 * @return
	 */
	public String getResponseFormat() {
		return getParamValue(KN_RESPONSE_FORMAT_PARAM);
	}

	//===========================================================================
	// COMMAND METHODS
	//===========================================================================

	/**
	 * Register this event handler to receive events
	 * 
	 * @param anEventHandler
	 */
	public void attachEventHandler(
		IEventHandler anEventHandler,
		String theId) {
		myEventHandlers.put(theId, anEventHandler);
	}

	/**
	 * Stop this event handler from receiving events
	 * 
	 * @param anEventHandler
	 */
	public IEventHandler detachEventHandler(String theId) {
		return (IEventHandler) myEventHandlers.remove(theId);
	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	protected boolean doExecute(URL theServerURL) {
		boolean retVal = false;
		// setup debugging

		/*System.setProperty(
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
			"debug");*/

		// clean up any existing connection
		if (null != myPostMethod)
			myPostMethod.releaseConnection();

		myPostMethod = createPostMethod(theServerURL);
		retVal = executeMethod(myPostMethod);
		if (retVal) {
			try {
				InputStream anInputStream =
					myPostMethod.getResponseBodyAsStream();

				if (null != anInputStream) {
					// fire up a new thread to keep reading the stream
					EventReaderThread anEventReaderThread =
						new EventReaderThread(anInputStream, myEventHandlers);
					anEventReaderThread.start();
				}

			} catch (IOException e) {
				e.printStackTrace();
				retVal = false;
				myPostMethod.releaseConnection();
			}
		}

		return retVal;
	}

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	private PostMethod myPostMethod;
	private Map myEventHandlers = new HashMap();

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

	//===========================================================================
	// STATIC METHODS 
	//===========================================================================
	
	// a thread to receive events over the http connection
	private static class EventReaderThread extends Thread {

		private SimpleEventReader myEventReader;
		private Map myEventHandlers;

		public EventReaderThread(
			InputStream theEventSource,
			Map theEventHandlers) {
			myEventReader = new SimpleEventReader(theEventSource);
			myEventHandlers = theEventHandlers;
		}

		/* (non-Javadoc)
		 * @see java.lang.Thread#run()
		 */
		public void run() {
			while (true) {
				Event anEvent;
				try {
					anEvent = myEventReader.readEvent();
					if (null != anEvent) {
						IEventHandler anEventHandler =
							(IEventHandler) myEventHandlers.get(
								anEvent.get(KN_ROUTE_LOCATION));
						if (null != anEventHandler) {
							anEventHandler.onEvent(anEvent);
						} else {
							System.err.println(
								"EVENT received, but no handler available: \n"
									+ anEvent);
						}
					} else {
						break;
					}
				} catch (IOException e) {
					e.printStackTrace();
					break;
				}
			}
		}

	}

}
