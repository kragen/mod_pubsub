/*
 * Created on Mar 21, 2004
 *
 */
package org.mod_pubsub.client.command;

import java.io.IOException;
import java.net.URL;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpException;
import org.apache.commons.httpclient.HttpMethodBase;
import org.apache.commons.httpclient.NameValuePair;
import org.apache.commons.httpclient.methods.PostMethod;
import org.mod_pubsub.client.Constants;
import org.mod_pubsub.client.IRequestStatusHandler;
import org.mod_pubsub.client.IStatusEvent;

/**
 * @author rtl
 *
 */
public class BaseCommand implements Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================
	
	protected BaseCommand(String theCommandName) {
		setParamValue(KN_METHOD_PARAM, theCommandName);
	}

	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	/**
	 * @return
	 */
	public String getMethod() {
		return getParamValue(KN_METHOD_PARAM);
	}

	public String getId() {
		return getParamValue(KN_ID_PARAM);
	}

	public String getParamValue(String theParamName) {
		return (String) myElements.get(theParamName);
	}
	
	public Integer getLastResponseCode() {
		return myResponseCode;
	}

	//===========================================================================
	// COMMAND METHODS
	//===========================================================================

	/**
	 * @param theServerURL
	 * @return
	 */
	public final boolean execute(URL theServerURL) {
		return execute(theServerURL, null);
	}

	public final boolean execute(
		URL theServerURL,
		IRequestStatusHandler theRequestStatusHandler) {

		myResponseCode = null;			
		boolean retVal = doExecute(theServerURL);

		// call the status handler, if required
		if (null != theRequestStatusHandler) {
			final boolean aStatusFlag = retVal;
			IStatusEvent aStatusEvent = new IStatusEvent() {
				public boolean wasSuccessful() {
					return aStatusFlag;
				}
			};
			if (aStatusFlag) {
				theRequestStatusHandler.onSuccess(aStatusEvent);
			} else {
				theRequestStatusHandler.onError(aStatusEvent);
			}
		}

		return retVal;

	}
	
	/* 
	 * 
	 */
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
		
		PostMethod aPostMethod = createPostMethod(theServerURL);
		retVal = executeMethod(aPostMethod);
		aPostMethod.releaseConnection();

		return retVal;
	}

	protected boolean executeMethod(HttpMethodBase theMethod) {
		boolean retVal = false;
		HttpClient aHttpClient = new HttpClient();
		// TODO - capture exception info, feed back to status handler
		try {
			int aResponseCode = aHttpClient.executeMethod(theMethod);
			retVal = (aResponseCode >= 200) && (aResponseCode < 300);
			myResponseCode = new Integer(aResponseCode);
		} catch (HttpException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return retVal;
	}

	protected PostMethod createPostMethod(URL theServerURL) {
		NameValuePair[] aNVPArray = convertElementsToNameValuePairs();
		
		// use HTTPClient to send the elements as form-encoded data
		PostMethod aPostMethod = new PostMethod(theServerURL.toString());
		aPostMethod.setRequestBody(aNVPArray);
		aPostMethod.setRequestHeader("Content-Type", KN_COMMAND_CONTENT_TYPE);
		aPostMethod.setRequestHeader("User-Agent", USER_AGENT);
		return aPostMethod;
	}

	protected void setParamValue(String theParamName, String theParamValue) {
		myElements.put(theParamName, theParamValue);
	}

	protected void setParamValues(Map theParamValues) {
		myElements.putAll(theParamValues);
	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	protected NameValuePair[] convertElementsToNameValuePairs() {
		// construct an array of name-value pairs for httpclient to use
		Iterator anIterator = myElements.keySet().iterator();
		NameValuePair[] aNVPArray =
			new NameValuePair[myElements.keySet().size()];

		int i = 0;
		while (anIterator.hasNext()) {
			String aName = (String) anIterator.next();
			String aValue = (String) myElements.get(aName);
			aNVPArray[i++] = new NameValuePair(aName, aValue);
		}
		return aNVPArray;
	}

	protected String createRandomString() {
		return Double.toString(Math.random()).substring(2);
	}

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	private Map myElements = new HashMap();
	private Integer myResponseCode = null;

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================
	
}
