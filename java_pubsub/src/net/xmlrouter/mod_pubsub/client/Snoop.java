package net.xmlrouter.mod_pubsub.client;

/**
 * @author msg
 * Sample console app that outputs events from a single topic
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
