/*
 * Created on Mar 21, 2004
 *
 */
package org.mod_pubsub.client;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.mortbay.http.HttpException;
import org.mortbay.http.HttpRequest;
import org.mortbay.http.HttpResponse;
import org.mortbay.http.handler.AbstractHttpHandler;

/**
 * @author rtl
 *
 */
public abstract class TestingHttpHandler extends AbstractHttpHandler {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	//===========================================================================
	// COMMAND METHODS
	//===========================================================================

	/* (non-Javadoc)
	 * @see org.mortbay.http.HttpHandler#handle(java.lang.String, java.lang.String, org.mortbay.http.HttpRequest, org.mortbay.http.HttpResponse)
	 */
	public void handle(
		String thePathInContext,
		String thePathParams,
		HttpRequest theRequest,
		HttpResponse theResponse)
		throws HttpException, IOException {

		// preserve the request for later				
		myRequestList.add(theRequest);

		// call the handler
		handleTestRequest(theRequest, theResponse);
		// tell Jetty we handled it		
		theRequest.setHandled(true);
		theResponse.commit();
	}

	/**
	 * Override this to do the actual testing
	 * 
	 * @param theRequest
	 * @param theResponse
	 * @return
	 */
	public abstract void handleTestRequest(
		HttpRequest theRequest,
		HttpResponse theResponse);

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	// keep track of the requests made to this handler
	public List myRequestList = new ArrayList();

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

}
