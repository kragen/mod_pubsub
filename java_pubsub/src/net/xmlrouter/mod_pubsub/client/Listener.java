package net.xmlrouter.mod_pubsub.client;
import java.util.Map;
/**
 * @author msg
 *
 * Message listener interface
 */
public interface Listener
{
	public void onMessage(Map msg);
}
