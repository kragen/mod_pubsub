
/*
 * Copyright (c) 2003 Robert Leftwich.  All Rights Reserved.
 *
 */

package org.mod_pubsub.chat;

import java.net.MalformedURLException;

import org.wxwindows.wxApp;
import org.wxwindows.wxPoint;
import org.wxwindows.wxSize;


/**
 * @author Robert Leftwich
 */
public class ChatApp extends wxApp
{
	private String[] myArgs;
	
	public ChatApp(String args[]) {
		myArgs = args;	
	}
	
	public boolean OnInit() throws MalformedURLException
	{
		wxInitAllImageHandlers();
		
		// get the command line arguments, first is host, second is id
		String host = myArgs.length > 0 ? myArgs[0] : "http://127.0.0.1:9000/kn";
		String id = myArgs.length > 1 ? myArgs[1] : System.getProperty("user.name");

		ChatDialog dialog = new ChatDialog(host, id, "PubSub Chat wxWindows for Java App",
									new wxPoint(50, 50), new wxSize(650, 400));
		dialog.Show(true);
		
		return true;
	}

	public static void main(String args[])
	{
		wxApp app = new ChatApp(args);
		app.MainLoop();
	}
}
