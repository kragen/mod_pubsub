package net.xmlrouter.mod_pubsub.client;
import java.util.Map;
/**
 * @author mike
 *
 * To change this generated comment edit the template variable "typecomment":
 * Window>Preferences>Java>Templates.
 * To enable and disable the creation of type comments go to
 * Window>Preferences>Java>Code Generation.
 */
public interface Listener
{
	public void onMessage(Map msg);
}
