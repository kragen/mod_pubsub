package net.xmlrouter.mod_pubsub.client;
import java.util.*;
import java.io.*;
import java.net.*;

/**
 * @author mike
 *
 * To change this generated comment edit the template variable "typecomment":
 * Window>Preferences>Java>Templates.
 * To enable and disable the creation of type comments go to
 * Window>Preferences>Java>Code Generation.
 */
public class HTTPUtil
{
	public static String encodeToForm(Map msg)
	{
		StringBuffer sb = new StringBuffer();
		
		String name;
		Object value;
		Iterator it = msg.keySet().iterator();
		while (it.hasNext())
		{
			name = (String)it.next();
			value = URLEncoder.encode((String)msg.get(name));
			sb.append(name+"="+value);
			if (it.hasNext())
				sb.append("&");
		}
		return sb.toString();
	}
	
	public static HttpURLConnection Post(URL url,String body) throws IOException
	{
		OutputStream os;
		HttpURLConnection conn;
		
		conn = (java.net.HttpURLConnection)url.openConnection();
		conn.setRequestMethod("POST");
		conn.setDoInput(true);
		conn.setDoOutput(true);
		conn.setRequestProperty("User-Agent","mod_pubsub.java/0.1");
		conn.setRequestProperty("Content-Type","application/x-www-form-urlencoded");
		conn.setRequestProperty("Content-Length",Integer.toString(body.length()*2));
		conn.connect();
		os = conn.getOutputStream();

		// send content body
		for (int i=0; i < body.length(); i++)
		{
			os.write((byte)body.charAt(i));
		}

		return conn;
	}
}
