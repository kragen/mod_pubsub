package net.xmlrouter.mod_pubsub.client;
import java.util.*;
import java.io.*;
import java.net.*;

/**
 * @author msg
 *
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
	
	public static HttpURLConnection Post(URL url,Map fields) throws IOException
	{
		String body;
		body = encodeToForm(fields);
		
		return Post(url,body);
	}
	
	public static HttpURLConnection Post(URL url,String body) throws IOException
	{
		OutputStream os;
		HttpURLConnection conn;
		String len;
		
		conn = (java.net.HttpURLConnection)url.openConnection();
		conn.setRequestMethod("POST");
		conn.setDoInput(true);
		conn.setDoOutput(true);
		conn.setRequestProperty("User-Agent","mod_pubsub.java/0.1");
		conn.setRequestProperty("Content-Type","application/x-www-form-urlencoded");
		len = Integer.toString(body.length());
		conn.setRequestProperty("Content-Length",len);
		conn.connect();
		os = conn.getOutputStream();

		// send content body
		for (int i=0; i < body.length(); i++)
		{
			os.write((byte)body.charAt(i));
		}

		return conn;
	}

	public static void dumpHeaders(HttpURLConnection conn)	
	{
		String name;
		String value;
		for(int i=0; i < 20; i++)
		{
			name = conn.getHeaderFieldKey(i);
			value = conn.getHeaderField(i);
			if (name== null && value==null)
				break;
			System.out.println(name+"="+value);
		}
		System.out.println();
	}
	
}
