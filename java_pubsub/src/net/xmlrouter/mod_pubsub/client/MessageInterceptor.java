/*
 * Created on Mar 24, 2004
 *
 */
package net.xmlrouter.mod_pubsub.client;
import java.util.Map;

/**
 * Apply defaults to request messages.
 * 
 * @author msg
 *
 */
public interface MessageInterceptor
{
	public Map filterMessage(Map msg);

}