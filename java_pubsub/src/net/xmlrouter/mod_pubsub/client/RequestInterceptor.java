/*
 * Created on Mar 24, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package net.xmlrouter.mod_pubsub.client;
import java.util.*;

/**
 * @author Mike
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class RequestInterceptor implements MessageInterceptor
{
	Map context;
	
	public RequestInterceptor(Map context)
	{
		this.context = context;
	}
	
	/**
	 * Apply values from 'context' when missing from 'msg'
	 * @param msg the message to filter
	 * @return the updated message
	 */
	public Map filterMessage(Map msg)
	{
		String name;
		for (Iterator it=context.keySet().iterator(); it.hasNext(); )
		{
			name = (String)it.next();
			if (!msg.containsKey(name))
			{
				msg.put(name,context.get(name));
			}
		}
		return msg;
	}
}
