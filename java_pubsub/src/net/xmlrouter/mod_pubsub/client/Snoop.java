/*
 * Created on Mar 24, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package net.xmlrouter.mod_pubsub.client;

/**
 * @author msg
 *
 */
public class Snoop
{

	public static void main(String[] args)
	{
		if (args.length < 2)
		{
			System.out.println("Usage: Snoop URL topic");			
			return;
		}
		
		try
		{
			SimpleRouter router = new SimpleRouter(args[0]);
			DebugListener listener = new DebugListener();
			router.subscribe(args[1],listener,null);
		}
		catch(Exception e)
		{
			System.err.println("ERROR: "+e.getMessage());		
		}
	}
}
