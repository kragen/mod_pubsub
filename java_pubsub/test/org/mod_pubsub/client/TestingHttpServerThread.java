/*
 * Created on Mar 20, 2004
 *
 */
package org.mod_pubsub.client;

import java.net.UnknownHostException;
import java.util.logging.Logger;

import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpHandler;
import org.mortbay.http.HttpServer;
import org.mortbay.http.SocketListener;
import org.mortbay.util.MultiException;

/**
 * @author rtl
 *
 */
public class TestingHttpServerThread extends Thread {
	
	private final HttpHandler myHttpHandler;
	private final int myPortNum;
	private final String myContextPath;
	private TestingHttpServer myHttpServer;
	
	public TestingHttpServerThread(
		HttpHandler theHttpHandler,
		int thePortNum,
		String theContextPath) {
		myHttpHandler = theHttpHandler;
		myPortNum = thePortNum;
		myContextPath = theContextPath;
	}
	
	public void run() {
		try {
			myHttpServer =
				new TestingHttpServer(myHttpHandler, myPortNum, myContextPath);
		} catch (Exception ex) {
			Logger.global.severe(
				"Could not start http server: "
					+ ex.getMessage());
		}
	}
	
	public void stopRun() {
		if (null != myHttpServer)
			myHttpServer.stop();
	}
	
	static class TestingHttpServer {
	
		public TestingHttpServer(
			HttpHandler theHttpHandler,
			int thePortNum,
			String theContextPath)
			throws MultiException, UnknownHostException {
	
			// Create the server
			myServer = new HttpServer();
	
			// Create a port listener
			SocketListener listener = new SocketListener();
			listener.setPort(thePortNum);
			listener.setHost("localhost");
			myServer.addListener(listener);
	
			// Create a context 
			HttpContext context = new HttpContext();
			context.setContextPath(theContextPath);
			myServer.addContext(context);
	
			context.addHandler(theHttpHandler);
	
			myServer.start();
		}
	
		public void stop() {
			try {
				myServer.stop();
			} catch (InterruptedException ex) {
				// ignore
			}
		}
	
		private HttpServer myServer;
	}
	
}
