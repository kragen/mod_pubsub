package net.xmlrouter.mod_pubsub.client;

import java.util.*;

/**
 * @author mike
 *
 */
public class DebugListener implements Listener
{

	/**
	 * @see net.xmlrouter.mod_pubsub.client.Listener#onMessage(HashMap)
	 */
	public void onMessage(Map msg)
	{
		Iterator it = msg.keySet().iterator();
		String name;
		while (it.hasNext())
		{
			name = (String)it.next();
			System.out.print(name);
			System.out.print("=");
			System.out.print(msg.get(name));
			System.out.println();
		}
	}
}
