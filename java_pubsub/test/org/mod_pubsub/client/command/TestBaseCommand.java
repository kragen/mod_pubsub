/*
 * Created on Mar 23, 2004
 *
 */
package org.mod_pubsub.client.command;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.apache.commons.httpclient.NameValuePair;
import org.mod_pubsub.client.Constants;
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
public class TestBaseCommand extends TestCase implements Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	/* (non-Javadoc)
	 * @see junit.framework.TestCase#setUp()
	 */
	protected void setUp() throws Exception {
		super.setUp();
		myServerURL = new URL("http", "localhost", theirTestPort, "/kn/");
		myValidCommand = new BaseCommand("testBaseCommand");
		myValidCommand.setParamValue(KN_ID_PARAM, "TestID");
		myValidCommand.setParamValue(KN_PAYLOAD_PARAM, "This is a test payload");
		myValidCommand.setParamValue("test1", "Test 1 value");
		myValidCommand.setParamValue("test2", "Test 2 value");
		myOnSuccessCalledCount = 0;
		myOnErrorCalledCount = 0;
	}

	//===========================================================================
	// TEST METHODS
	//===========================================================================

	public void testDefaultElementsSetupCorrectly() {

		final String aTestMethod = "testMethod";
		BaseCommand aCommand = new BaseCommand(aTestMethod);

		assertEquals(aTestMethod, aCommand.getMethod());
	}

	public void testConvenienceMethods() {

		final String aMethodName = "testConvenienceMethods";
		BaseCommand aCommand = new BaseCommand(aMethodName);
		final String aTestID = "testID";
		aCommand.setParamValue("kn_id", aTestID);
		assertEquals("Incorrect id", aTestID, aCommand.getId());

		NameValuePair[] aNvpArray = aCommand.convertElementsToNameValuePairs();
		assertEquals("Incorrect size", 2, aNvpArray.length);
		int checkedValuesCnt = 0;
		for (int i = 0; i < aNvpArray.length; i++) {
			NameValuePair aNameValuePair = aNvpArray[i];
			if (aNameValuePair.getName() == KN_METHOD_PARAM) {
				assertEquals(
					"Incorrect method in nvp",
					aMethodName,
					aNameValuePair.getValue());
				checkedValuesCnt++;
			} else if (aNameValuePair.getName() == KN_ID_PARAM) {
				assertEquals(
					"Incorrect id in nvp",
					aTestID,
					aNameValuePair.getValue());
				checkedValuesCnt++;
			}
		}
		assertEquals(
			"Did not find all default values in nvp array",
			2,
			checkedValuesCnt);

		// create a new map to populate the command with
		Map aNewElementMap = new HashMap();
		String aNewId = "a new id";
		aNewElementMap.put(KN_ID_PARAM, aNewId);
		String aNewMethod = "a new method";
		aNewElementMap.put(KN_METHOD_PARAM, aNewMethod);
		final int maxCnt = 7;
		for (int i = 0; i < maxCnt; i++) {
			aNewElementMap.put("name" + i, "value" + i);
		}
		aCommand.setParamValues(aNewElementMap);

		assertEquals("Incorrect new id", aNewId, aCommand.getId());
		assertEquals("Incorrect new method", aNewMethod, aCommand.getMethod());

		aNvpArray = aCommand.convertElementsToNameValuePairs();
		assertEquals("Incorrect new size", maxCnt + 2, aNvpArray.length);
		checkedValuesCnt = 0;
		for (int i = 0; i < aNvpArray.length; i++) {
			NameValuePair aNameValuePair = aNvpArray[i];
			assertEquals(
				"Incorrect value for:" + aNameValuePair.getName(),
				aNewElementMap.get(aNameValuePair.getName()),
				aNameValuePair.getValue());
			checkedValuesCnt++;
		}
		assertEquals(
			"Did not find all default values in new nvp array",
			maxCnt + 2,
			checkedValuesCnt);

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
					theResponse.setStatus(HttpResponse.__204_No_Content);
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
					theResponse.setStatus(HttpResponse.__204_No_Content);
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

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	public void validateRequest(HttpRequest theRequest) {
		// make sure command has some values
		assertNotNull("Method is null", myValidCommand.getMethod());
		assertNotNull("id is null", myValidCommand.getId());
		assertNotNull("payload is null", myValidCommand.getParamValue(KN_PAYLOAD_PARAM));
		assertNotNull("arbitrary param 1 is null", myValidCommand.getParamValue("test1"));
		assertNotNull("arbitrary param 2 is null", myValidCommand.getParamValue("test2"));
		
		assertEquals("UTF-8", theRequest.getCharacterEncoding());
		assertEquals(KN_COMMAND_CONTENT_TYPE, theRequest.getContentType());
		assertEquals(USER_AGENT, theRequest.getField("User-Agent"));
		assertEquals(
			"Incorrect method param",
			myValidCommand.getMethod(),
			theRequest.getParameter(KN_METHOD_PARAM));
		assertEquals(
			"Incorrect id",
			myValidCommand.getId(),
			theRequest.getParameter(KN_ID_PARAM));
		assertEquals(
			"Incorrect payload",
			myValidCommand.getParamValue(KN_PAYLOAD_PARAM),
			theRequest.getParameter(KN_PAYLOAD_PARAM));
		assertEquals(
			"Incorect arbitrary 1 param",
			myValidCommand.getParamValue("test1"),
			theRequest.getParameter("test1"));
		assertEquals(
			"Incorect arbitrary 2 param",
			myValidCommand.getParamValue("test2"),
			theRequest.getParameter("test2"));
	}

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	private BaseCommand myValidCommand;
	private URL myServerURL;
	private int myOnSuccessCalledCount;
	private int myOnErrorCalledCount;

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

	private static final int theirTestPort = 4949;

}
