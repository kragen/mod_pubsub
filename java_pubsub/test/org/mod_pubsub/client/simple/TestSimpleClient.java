/*
 * Created on Mar 19, 2004
 *
 */
package org.mod_pubsub.client.simple;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.mod_pubsub.client.Constants;
import org.mod_pubsub.client.IEvent;
import org.mod_pubsub.client.IEventHandler;
import org.mod_pubsub.client.IRequestStatusHandler;
import org.mod_pubsub.client.IStatusEvent;
import org.mod_pubsub.client.JournalTopic;
import org.mod_pubsub.client.command.CommandOptions;
import org.mod_pubsub.client.command.DeleteRouteCommand;
import org.mod_pubsub.client.command.NotifyCommand;
import org.mod_pubsub.client.command.RouteCommand;

/**
 * @author rtl
 *
 */
public class TestSimpleClient extends TestCase implements Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	public TestSimpleClient() throws MalformedURLException {
		DEFAULT_SERVER_URL = new URL(DEFAULT_SERVER_URL_STRING);
	}

	//===========================================================================
	// TEST METHODS
	//===========================================================================

	public void testConnectionFailureReportsErrorStatus()
		throws MalformedURLException {

		//	get a simple client that fails journal cmd
		SimpleClient aSimpleClient =
			createSimpleClientWithFailingJournalRouteCmd();

		assertFalse(
			"Connected after construction",
			aSimpleClient.isConnected());

		final List anErrorStatusReceivedList = new ArrayList();
		final List aSuccessStatusReceivedList = new ArrayList();
		IRequestStatusHandler aRequestStatusHandler =
			buildDebugRequestStatusHandler(
				anErrorStatusReceivedList,
				aSuccessStatusReceivedList);

		assertFalse(
			"Connected ok",
			aSimpleClient.connect(aRequestStatusHandler));
		assertFalse(
			"Is connected after connect failure",
			aSimpleClient.isConnected());
		assertEquals(
			"Error handler was not called correct no. of times",
			1,
			anErrorStatusReceivedList.size());
		assertEquals(
			"Success handler was called",
			0,
			aSuccessStatusReceivedList.size());
		assertFalse(
			"Successful status event reported",
			((IStatusEvent) anErrorStatusReceivedList.get(0)).wasSuccessful());
	}

	public void testConnectionSuccessReportsGoodStatus()
		throws MalformedURLException {

		// get a simple client that succeeds journal cmd
		SimpleClient aSimpleClient =
			createSimpleClientWithSuccessfulJournalRouteCmd();

		assertFalse(
			"Connected after construction",
			aSimpleClient.isConnected());

		final List anErrorStatusReceivedList = new ArrayList();
		final List aSuccessStatusReceivedList = new ArrayList();
		IRequestStatusHandler aRequestStatusHandler =
			buildDebugRequestStatusHandler(
				anErrorStatusReceivedList,
				aSuccessStatusReceivedList);

		assertTrue(
			"Connection failed",
			aSimpleClient.connect(aRequestStatusHandler));
		assertTrue(
			"Not connected after connect success",
			aSimpleClient.isConnected());
		assertEquals(
			"Success handler was not called correct no. of times",
			1,
			aSuccessStatusReceivedList.size());
		assertEquals(
			"Error handler was called",
			0,
			anErrorStatusReceivedList.size());
		assertTrue(
			"Failed to connect",
			((IStatusEvent) aSuccessStatusReceivedList.get(0)).wasSuccessful());

	}

	public void testEnsureConnectedWithFailedConnectionReportsErrorStatus()
		throws MalformedURLException, URISyntaxException {

		//	get a simple client that fails journal cmd
		SimpleClient aSimpleClient =
			createSimpleClientWithFailingJournalRouteCmd();

		IEvent anEvent = buildDummyEvent();

		final List anErrorStatusReceivedList = new ArrayList();
		final List aSuccessStatusReceivedList = new ArrayList();
		IRequestStatusHandler aRequestStatusHandler =
			buildDebugRequestStatusHandler(
				anErrorStatusReceivedList,
				aSuccessStatusReceivedList);

		assertFalse(
			"ensureConnected succeeded",
			aSimpleClient.ensureConnected(aRequestStatusHandler));
		assertFalse(
			"Is connected after connect failure",
			aSimpleClient.isConnected());
		assertEquals(
			"Error handler was not called correct no. of times",
			1,
			anErrorStatusReceivedList.size());
		assertEquals(
			"Success handler was called",
			0,
			aSuccessStatusReceivedList.size());
		assertFalse(
			"Should have failed to publish",
			((IStatusEvent) anErrorStatusReceivedList.get(0)).wasSuccessful());
	}

	public void testPublishWithGoodConnectionReportsFailedPublishStatus()
		throws MalformedURLException, URISyntaxException {

		//	get a simple client that succeeds journal cmd, but fails notify
		SimpleClient aSimpleClient =
			createSimpleClientWithSuccessfulJournalRouteCmdAndFailingNotifyCmd();

		IEvent anEvent = buildDummyEvent();

		final List anErrorStatusReceivedList = new ArrayList();
		final List aSuccessStatusReceivedList = new ArrayList();
		IRequestStatusHandler aRequestStatusHandler =
			buildDebugRequestStatusHandler(
				anErrorStatusReceivedList,
				aSuccessStatusReceivedList);

		assertFalse(
			"Publish succeeded",
			aSimpleClient.publish(
				new URI("/test/topic/"),
				anEvent,
				aRequestStatusHandler));
		assertTrue(
			"Not connected after publish failure",
			aSimpleClient.isConnected());
		assertEquals(
			"Error handler was not called correct no. of times",
			1,
			anErrorStatusReceivedList.size());
		assertEquals(
			"Success handler was called more than once",
			1,
			aSuccessStatusReceivedList.size());
		assertFalse(
			"Should have failed to publish",
			((IStatusEvent) anErrorStatusReceivedList.get(0)).wasSuccessful());

	}

	public void testPublishOnlyConnectsOnce()
		throws MalformedURLException, URISyntaxException {
		final List anExecuteCalledList = new ArrayList();

		// override journal route command factory method to return one 
		// we have control over i.e. that alwasy succeeds and preserves its calls
		SimpleClient aSimpleClient = new SimpleClient(DEFAULT_SERVER_URL) {

			protected SimpleJournalRouteCommand createJournalRouteCommand() {
				return new SimpleJournalRouteCommand(getJournalURL()) {
					public boolean doExecute(URL theServerURL) {
						anExecuteCalledList.add(theServerURL);
						return true;
					}
				};
			}
			// override notify command factory method 
			// to return one we have control over i.e. that alwasy succeeds
			protected NotifyCommand createNotifyCommand(
				URI theTopicURI,
				IEvent theEvent) {
				return new NotifyCommand(theTopicURI, theEvent) {
					public boolean doExecute(URL theServerURL) {
						return true;
					}
				};
			}
		};

		IEvent anEvent = buildDummyEvent();

		final List anErrorStatusReceivedList = new ArrayList();
		final List aSuccessStatusReceivedList = new ArrayList();
		IRequestStatusHandler aRequestStatusHandler =
			buildDebugRequestStatusHandler(
				anErrorStatusReceivedList,
				aSuccessStatusReceivedList);

		final int maxCnt = 5;
		URI aTopicURI = new URI("/test/topic/");
		for (int i = 0; i < maxCnt; i++) {
			assertTrue(
				"Publish failed",
				aSimpleClient.publish(
					aTopicURI,
					anEvent,
					aRequestStatusHandler));
		}

		assertEquals(
			"Should only have connected once",
			1,
			anExecuteCalledList.size());

		assertTrue(
			"Not connected after publish success",
			aSimpleClient.isConnected());
		assertEquals(
			"Error handler was called",
			0,
			anErrorStatusReceivedList.size());
		assertEquals(
			"Success handler was not called correct no. of times",
			maxCnt + 1,
			aSuccessStatusReceivedList.size());
	}

	public void testJournalURL() throws MalformedURLException {

		SimpleClient aSimpleClient = new SimpleClient(DEFAULT_SERVER_URL);

		URL anExpectedURLStart =
			new URL(
				DEFAULT_SERVER_URL,
				JournalTopic.ANONYMOUS_BASE_JOURNAL_URI);
		URL aJournalUrl = aSimpleClient.getJournalURL();
		assertNotNull(aJournalUrl);
		assertTrue(
			"Journal URL base path incorrect",
			aJournalUrl.toString().startsWith(anExpectedURLStart.toString()));
	}

	public void testSubscribeWithGoodConnectionReportsFailedSubscribeStatus()
		throws MalformedURLException, URISyntaxException {

		//	get a simple client that succeeds journal cmd, but fails subscribe
		SimpleClient aSimpleClient =
			createSimpleClientWithSuccessfulJournalRouteCmdAndFailingSubscribeCmd();

		IEventHandler anEventHandler = buildDummyEventHandler();

		final List anErrorStatusReceivedList = new ArrayList();
		final List aSuccessStatusReceivedList = new ArrayList();
		IRequestStatusHandler aRequestStatusHandler =
			buildDebugRequestStatusHandler(
				anErrorStatusReceivedList,
				aSuccessStatusReceivedList);

		assertNull(
			"Subscribe succeeded",
			aSimpleClient.subscribe(
				new URI("/test/topic/"),
				anEventHandler,
				aRequestStatusHandler));
		assertTrue(
			"Not connected after subscribe failure",
			aSimpleClient.isConnected());
		assertEquals(
			"Error handler was not called correct no. of times",
			1,
			anErrorStatusReceivedList.size());
		assertEquals(
			"Success handler was called more than once",
			1,
			aSuccessStatusReceivedList.size());
		assertFalse(
			"Should have failed to subscribe",
			((IStatusEvent) anErrorStatusReceivedList.get(0)).wasSuccessful());

	}

	public void testUnsubscribe()
		throws MalformedURLException, URISyntaxException {

		//get a simple client where all succeed
		SimpleClient aSimpleClient =
			createSimpleClientWithSuccessfulJournalRouteSubscribeAndDeleteCmd();

		IEventHandler anEventHandler = buildDummyEventHandler();

		final List anErrorStatusReceivedList = new ArrayList();
		final List aSuccessStatusReceivedList = new ArrayList();
		IRequestStatusHandler aRequestStatusHandler =
			buildDebugRequestStatusHandler(
				anErrorStatusReceivedList,
				aSuccessStatusReceivedList);

		URI aURI =
			aSimpleClient.subscribe(
				new URI("/test/topic/"),
				anEventHandler,
				aRequestStatusHandler);
		assertNotNull("Subscribe failed", aURI);

		assertTrue(
			"Unsubscribe failed",
			aSimpleClient.unsubscribe(aURI, aRequestStatusHandler));

		assertFalse(
			"Unsubscribe succeeded on retry",
			aSimpleClient.unsubscribe(aURI, aRequestStatusHandler));
	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	private IRequestStatusHandler buildDebugRequestStatusHandler(
		final List anErrorStatusReceivedList,
		final List aSuccessStatusReceivedList) {
		IRequestStatusHandler aRequestStatusHandler =
			new IRequestStatusHandler() {

			public void onError(IStatusEvent theStatusEvent) {
				anErrorStatusReceivedList.add(theStatusEvent);
			}

			public void onSuccess(IStatusEvent theStatusEvent) {
				aSuccessStatusReceivedList.add(theStatusEvent);
			}
		};
		return aRequestStatusHandler;
	}

	private IEvent buildDummyEvent() {
		IEvent anEvent = new IEvent() {

			public String getPayload() {
				return "payload";
			}

			public String getId() {
				return "dummy";
			}

			public String get(String theName) {
				return null;
			}

			public void set(String theName, String theParam) {
			}

			public void remove(String theName) {
			}

			public Map getEventElements() {
				return Collections.EMPTY_MAP;
			}

			public int getElementCount() {
				return 2;
			}
		};
		return anEvent;
	}

	private SimpleClient createSimpleClientWithFailingJournalRouteCmd()
		throws MalformedURLException {

		// override journal route command factory method 
		// to return one we have control over i.e. that alwasy fails
		SimpleClient aSimpleClient = new SimpleClient(DEFAULT_SERVER_URL) {
			protected SimpleJournalRouteCommand createJournalRouteCommand() {
				return new SimpleJournalRouteCommand(getJournalURL()) {
					public boolean doExecute(URL theServerURL) {
						return false;
					}
				};
			}
		};
		return aSimpleClient;
	}

	private SimpleClient createSimpleClientWithSuccessfulJournalRouteCmd()
		throws MalformedURLException {

		// override journal route command factory method 
		// to return one we have control over i.e. that alwasy succeeds
		SimpleClient aSimpleClient = new SimpleClient(DEFAULT_SERVER_URL) {
			protected SimpleJournalRouteCommand createJournalRouteCommand() {
				return new SimpleJournalRouteCommand(getJournalURL()) {
					public boolean doExecute(URL theServerURL) {
						return true;
					}
				};
			}
		};
		return aSimpleClient;
	}

	private SimpleClient createSimpleClientWithSuccessfulJournalRouteCmdAndFailingNotifyCmd()
		throws MalformedURLException {

		SimpleClient aSimpleClient = new SimpleClient(DEFAULT_SERVER_URL) {
				// override journal route command factory method 
		// to return one we have control over i.e. that alwasy succeeds
	protected SimpleJournalRouteCommand createJournalRouteCommand() {
				return new SimpleJournalRouteCommand(getJournalURL()) {
					public boolean doExecute(URL theServerURL) {
						return true;
					}
				};
			}
			// override notify command factory method 
			// to return one we have control over i.e. that alwasy fails
			protected NotifyCommand createNotifyCommand(
				URI theTopicURI,
				IEvent theEvent) {
				return new NotifyCommand(theTopicURI, theEvent) {
					public boolean doExecute(URL theServerURL) {
						return false;
					}
				};
			}
		};
		return aSimpleClient;
	}

	private SimpleClient createSimpleClientWithSuccessfulJournalRouteCmdAndFailingSubscribeCmd()
		throws MalformedURLException {

		SimpleClient aSimpleClient = new SimpleClient(DEFAULT_SERVER_URL) {
				// override journal route command factory method 
		// to return one we have control over i.e. that alwasy succeeds
	protected SimpleJournalRouteCommand createJournalRouteCommand() {
				return new SimpleJournalRouteCommand(getJournalURL()) {
					public boolean doExecute(URL theServerURL) {
						return true;
					}
				};
			}
			// override route command factory method 
			// to return one we have control over i.e. that alwasy fails
			protected RouteCommand createRouteCommand(
				URI theTopicURI,
				URL theJournalURL,
				CommandOptions theCommandOptions) {
				return new RouteCommand(theTopicURI, theJournalURL, theCommandOptions) {
					public boolean doExecute(URL theServerURL) {
						return false;
					}
				};
			}
		};
		return aSimpleClient;
	}

	private SimpleClient createSimpleClientWithSuccessfulJournalRouteSubscribeAndDeleteCmd()
		throws MalformedURLException {
		SimpleClient aSimpleClient = new SimpleClient(DEFAULT_SERVER_URL) {
			// override journal route command factory method 
			// to return one we have control over i.e. that alwasy succeeds
			protected SimpleJournalRouteCommand createJournalRouteCommand() {
				return new SimpleJournalRouteCommand(getJournalURL()) {
					public boolean doExecute(URL theServerURL) {
						return true;
					}
				};
			}
			
			// override route command factory method 
			// to return one we have control over i.e. that alwasy succeeds
			protected RouteCommand createRouteCommand(
				URI theTopicURI,
				URL theJournalURL,
				CommandOptions theCommandOptions) {
					return new RouteCommand(theTopicURI, theJournalURL, theCommandOptions) {
						public boolean doExecute(URL theServerURL) {
							return true;
						}
					};
			}

			// override delete route command factory method 
			// to return one we have control over i.e. that alwasy succeeds
			protected DeleteRouteCommand createDeleteRouteCommand(URI theRouteLocationURI) {
				return new DeleteRouteCommand(theRouteLocationURI) {
					public boolean doExecute(URL theServerURL) {
						return true;
					}
				};
			}

		};
		return aSimpleClient;
	}

	private IEventHandler buildDummyEventHandler() {
		IEventHandler anEventHandler = new IEventHandler() {
			public void onEvent(IEvent theEvent) {
					// do nothing
	}

		};
		return anEventHandler;
	}

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

	private final String DEFAULT_SERVER_URL_STRING =
		"http://127.0.0.1:4949/kn/";
	private final URL DEFAULT_SERVER_URL;

}
