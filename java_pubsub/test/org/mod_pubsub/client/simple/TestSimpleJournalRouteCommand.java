/*
 * Created on Mar 20, 2004
 *
 */
package org.mod_pubsub.client.simple;

import java.io.IOException;
import java.io.OutputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.mod_pubsub.client.Constants;
import org.mod_pubsub.client.IEvent;
import org.mod_pubsub.client.IEventHandler;
import org.mod_pubsub.client.IRequestStatusHandler;
import org.mod_pubsub.client.IStatusEvent;
import org.mod_pubsub.client.TestingHttpHandler;
import org.mod_pubsub.client.TestingHttpServerThread;
import org.mortbay.http.HttpRequest;
import org.mortbay.http.HttpResponse;

/**
 * @author rtl
 *
 */
public class TestSimpleJournalRouteCommand
	extends TestCase
	implements Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	/* (non-Javadoc)
	 * @see junit.framework.TestCase#setUp()
	 */
	protected void setUp() throws Exception {
		super.setUp();
		myServerURL = new URL("http", "localhost", theirTestPort, "/kn/");
		myValidJournalUrl =
			new URL(myServerURL, "/who/anonymous/s/1234/kn_journal");
		myValidCommand = new SimpleJournalRouteCommand(myValidJournalUrl);
		myOnSuccessCalledCount = 0;
		myOnErrorCalledCount = 0;
	}

	//===========================================================================
	// TEST METHODS
	//===========================================================================

	public void testNonJournalURLThrowsException()
		throws MalformedURLException {

		URL anInvalidJournalUrl =
			new URL("http://localhost:1234/this/is/an/invalid/journal/name");

		try {
			SimpleJournalRouteCommand aCommand =
				new SimpleJournalRouteCommand(anInvalidJournalUrl);
			fail("Invalid journal url not detected");
		} catch (IllegalArgumentException ex) {
			// expected
		}

	}

	public void testDefaultElementsSetupCorrectly()
		throws MalformedURLException {

		SimpleJournalRouteCommand aCommand =
			new SimpleJournalRouteCommand(myValidJournalUrl);
		assertEquals(aCommand.getFrom(), myValidJournalUrl.toString());
		assertEquals(aCommand.getTo(), "javascript:");
		assertEquals(aCommand.getMethod(), "route");
		assertNull(aCommand.getId());
		assertTrue(
			aCommand.getStatusFrom().startsWith("javascript://tempuri.org/"));
		assertEquals(aCommand.getResponseFormat(), "simple");
	}

	public void testExecuteServerSuccessNoStatusHandler()
		throws MalformedURLException {

		// create an http test handler
		TestingHttpHandler aHttpHandler = new TestingHttpHandler() {
			public void handleTestRequest(
				HttpRequest theRequest,
				HttpResponse theResponse) {

				validateRequest(theRequest);

				String method = theRequest.getMethod();
				// POST expected
				if (method.equalsIgnoreCase("POST")) {
					theResponse.setStatus(HttpResponse.__200_OK);
					theResponse.setContentType("text/plain");
					OutputStream anOutputStream = theResponse.getOutputStream();

					String aTest = "This is a test";
					try {
						anOutputStream.write(aTest.getBytes());
						Thread.sleep(1000);
						String anotherTest = "This is another test";
						anOutputStream.write(anotherTest.getBytes());
						anOutputStream.flush();
					} catch (IOException e) {
						e.printStackTrace();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}

				} else {
					theResponse.setStatus(
						HttpResponse.__405_Method_Not_Allowed);
				}
			}
		};

		// fire up a http test server
		TestingHttpServerThread aHttpServerThread =
			new TestingHttpServerThread(aHttpHandler, theirTestPort, "/kn");

		try {
			aHttpServerThread.run();
			assertTrue("Expected success", myValidCommand.execute(myServerURL));
			assertEquals(
				"No HTTP request received",
				1,
				aHttpHandler.myRequestList.size());
		} finally {
			aHttpServerThread.stopRun();
		}

	}

	public void testExecuteServerFailureNoStatusHandler()
		throws MalformedURLException {

		// create an http test handler
		TestingHttpHandler aHttpHandler = new TestingHttpHandler() {
			public void handleTestRequest(
				HttpRequest theRequest,
				HttpResponse theResponse) {

				validateRequest(theRequest);
				// fail everything
				theResponse.setStatus(HttpResponse.__400_Bad_Request);
			}
		};

		// fire up a http test server
		TestingHttpServerThread aHttpServerThread =
			new TestingHttpServerThread(aHttpHandler, theirTestPort, "/kn");

		try {
			aHttpServerThread.run();
			assertFalse(
				"Expected failure",
				myValidCommand.execute(myServerURL));
			assertEquals(
				"No HTTP request received",
				1,
				aHttpHandler.myRequestList.size());
		} finally {
			aHttpServerThread.stopRun();
		}

	}

	public void testExecuteServerSuccessWithStatusHandler()
		throws MalformedURLException {

		// create an http test handler
		TestingHttpHandler aHttpHandler = new TestingHttpHandler() {
			public void handleTestRequest(
				HttpRequest theRequest,
				HttpResponse theResponse) {

				validateRequest(theRequest);

				String method = theRequest.getMethod();
				// POST expected
				if (method.equalsIgnoreCase("POST")) {
					theResponse.setStatus(HttpResponse.__200_OK);
					theResponse.setContentType("text/plain");
				} else {
					theResponse.setStatus(
						HttpResponse.__405_Method_Not_Allowed);
				}
			}
		};

		// fire up a http test server
		TestingHttpServerThread aHttpServerThread =
			new TestingHttpServerThread(aHttpHandler, theirTestPort, "/kn");

		// create a status handler
		IRequestStatusHandler aRequestStatusHandler =
			new IRequestStatusHandler() {
			public void onError(IStatusEvent theStatusEvent) {
				myOnErrorCalledCount++;
			}

			public void onSuccess(IStatusEvent theStatusEvent) {
				myOnSuccessCalledCount++;
			}
		};

		try {
			aHttpServerThread.run();
			assertTrue(
				"Expected success",
				myValidCommand.execute(myServerURL, aRequestStatusHandler));
			assertEquals(
				"No HTTP request received",
				1,
				aHttpHandler.myRequestList.size());
			assertEquals(
				"Success status handler not called",
				1,
				myOnSuccessCalledCount);
			assertEquals(
				"Error status handler called",
				0,
				myOnErrorCalledCount);
		} finally {
			aHttpServerThread.stopRun();
		}

	}

	public void testEventHandlerCalledOnlyForMatchingRouteLocation() {

		final String aRouteLocation = "route_location";

		final List anEventList = new ArrayList();
		IEventHandler anEventHandler = new IEventHandler() {

			public void onEvent(IEvent theEvent) {
				anEventList.add(theEvent);
			}
		};

		// register event handler to the command		
		myValidCommand.attachEventHandler(anEventHandler, aRouteLocation);

		// create an event string to return
		String anElement =
			KN_ROUTE_LOCATION
				+ ": "
				+ aRouteLocation
				+ "\n"
				+ "element1: value1\n\nThis is a payload";
		final String aValidLocationEventString =
			anElement.length() + "\n" + anElement;
		anElement =
			KN_ROUTE_LOCATION
				+ ": "
				+ "invalid-location"
				+ "\n"
				+ "element1: value1\n\nThis is a payload";
		final String aInvalidLocationEventString =
			anElement.length() + "\n" + anElement;

		// create an http test handler
		TestingHttpHandler aHttpHandler = new TestingHttpHandler() {
			public void handleTestRequest(
				HttpRequest theRequest,
				HttpResponse theResponse) {

				validateRequest(theRequest);

				String method = theRequest.getMethod();
				// POST expected
				if (method.equalsIgnoreCase("POST")) {
					theResponse.setStatus(HttpResponse.__200_OK);
					theResponse.setContentType("text/plain");
					OutputStream anOutputStream = theResponse.getOutputStream();

					try {
						anOutputStream.write(aValidLocationEventString.getBytes());
						anOutputStream.write('\n');
						anOutputStream.write(aInvalidLocationEventString.getBytes());
						anOutputStream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}

				} else {
					theResponse.setStatus(
						HttpResponse.__405_Method_Not_Allowed);
				}
			}
		};

		// fire up a http test server
		TestingHttpServerThread aHttpServerThread =
			new TestingHttpServerThread(aHttpHandler, theirTestPort, "/kn");

		aHttpServerThread.run();
		assertTrue("Expected success", myValidCommand.execute(myServerURL));

		try {
			Thread.sleep(500);
		} catch (InterruptedException e) {
			// ignore
		}

		assertEquals(
			"No HTTP request received",
			1,
			aHttpHandler.myRequestList.size());
		assertEquals("No event received", 1, anEventList.size());

		// detach the event handler, make sure no events received
		assertSame(
			"Incorrect detached event handler",
			anEventHandler,
			myValidCommand.detachEventHandler(aRouteLocation));

		try {
			assertTrue("Expected success", myValidCommand.execute(myServerURL));
		} finally {
			aHttpServerThread.stopRun();
		}

		try {
			Thread.sleep(500);
		} catch (InterruptedException e) {
			// ignore
		}

		assertEquals(
			"No more HTTP request received",
			2,
			aHttpHandler.myRequestList.size());
		assertEquals("More events received", 1, anEventList.size());

	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	public void validateRequest(HttpRequest theRequest) {
		assertEquals("UTF-8", theRequest.getCharacterEncoding());
		assertEquals(KN_COMMAND_CONTENT_TYPE, theRequest.getContentType());
		assertEquals(USER_AGENT, theRequest.getField("User-Agent"));
		assertEquals(
			"Incorrect method param",
			myValidCommand.getMethod(),
			theRequest.getParameter(KN_METHOD_PARAM));
		assertEquals(
			"Incorrect to param",
			myValidCommand.getTo(),
			theRequest.getParameter(KN_TO_PARAM));
		assertEquals(
			"Incorrect from param",
			myValidCommand.getFrom(),
			theRequest.getParameter(KN_FROM_PARAM));
		assertEquals(
			"Incorrect status from param",
			myValidCommand.getStatusFrom(),
			theRequest.getParameter(KN_STATUS_FROM_PARAM));
		assertEquals(
			"Incorrect response format param",
			myValidCommand.getResponseFormat(),
			theRequest.getParameter(KN_RESPONSE_FORMAT_PARAM));
	}

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	private URL myServerURL;
	private URL myValidJournalUrl;
	private SimpleJournalRouteCommand myValidCommand;
	private int myOnSuccessCalledCount;
	private int myOnErrorCalledCount;

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

	private static final int theirTestPort = 4949;

}
