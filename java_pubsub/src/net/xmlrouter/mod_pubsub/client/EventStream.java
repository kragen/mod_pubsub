package net.xmlrouter.mod_pubsub.client;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;

/**
 * Parse input stream and bundle data into a single message
 * @author msg
 */
public class EventStream
{
  static int MAX_EVENT_SIZE = 65535;
  InputStream is;
  byte[] buffer;
  ByteArrayInputStream bais;
  
  public EventStream(InputStream is)
  {
    this.is = is;
    buffer = new byte[4096];
    bais = new  ByteArrayInputStream(buffer);   
  }

  
  /**
   * readLine - read a line of ascii characters until a newline char.
   */
  String readLine(InputStream stream)
  {
    StringBuffer buffer=new StringBuffer();
    byte b[] = new byte[1];
    char c;
    
    try
    {
      while (stream.read(b) > 0)
      {
        if ((b[0] == '\r') ||
          (b[0] == '\n'))
        {
          break;
        }
        
        c = (char)b[0];
        buffer.append(c);
      }
    }
    catch(IOException e)
    {
      System.err.println("readLine() : "+e.getMessage());
    }

    return buffer.toString();
  }
  /**
   * readTrimmedLine - skip whitepace and read remaining line.
   */
  String readTrimmedLine(InputStream stream)
  {
    StringBuffer buffer=new StringBuffer();
    byte b[] = new byte[1];
    char c;
    boolean skipping=true;
    int count=0;
    
    try
    {
      while (stream.read(b) > 0)
      {
        if ((b[0] == '\r') ||
          (b[0] == '\n'))
        {
          break;
        }
        
        c = (char)b[0];
        if (!Character.isWhitespace(c))
          skipping = false;

        if (!skipping)
          buffer.append(c);
        else
          count++;
        b[0] = 0;
      }
    }
    catch(IOException e)
    {
      System.err.println("readLine() : "+e.getMessage());
    }

    return buffer.toString();
  }
  
  /**
   * Read the next event from an input stream
   */
  public Map readEvent() throws IOException
  {
    HashMap msg=null;
    
    try
    {
      String line;
      BufferedReader event;
      int nBytes;
      String name;
      String value;
      
      int bytesRead;
      
      {
        int pos;
        
        // read a line
        line = readTrimmedLine(is);
        if ((line == null) || (line.length()==0))
          return null;

        line = line.trim();

        // get size of message          
        nBytes = Integer.parseInt(line) + 1;
        if (nBytes > MAX_EVENT_SIZE)
          throw new RuntimeException("EventStream() : Event too large : "+line);
          
        // make sure buffer is correct size
        if (nBytes != buffer.length)
        {
          buffer = new byte[nBytes];
          bais = new ByteArrayInputStream(buffer);
        }
        bais.reset();
        event = new BufferedReader(new InputStreamReader(bais,"UTF-8"));

        // read full event
        msg = new HashMap();
        bytesRead = is.read(buffer,0,nBytes);
        if (bytesRead != nBytes) {
          // throw new RuntimeException("EventStream() : Didn't read enough bytes : " + bytesRead + " expected : " + nBytes);
          while (bytesRead < nBytes) {
            bytesRead += is.read(buffer,bytesRead,nBytes-bytesRead);
          }
        }
          

        bytesRead = 0;
        // read headers from byte buffer
        while (((line = event.readLine()) != null) &&
            (line.length() > 0))
        {
          bytesRead += line.length();
          // convert to event object
          pos = line.indexOf(':');
          if (pos > 0)
          {
            name = line.substring(0,pos);
            value = line.substring(pos+2);  // @todo: decode 'quoted-printable'
            msg.put(name,value);
          }
        }
        
        // read body
        name = "kn_payload";
        StringBuffer payload = new StringBuffer();
        while (((line = event.readLine()) != null) &&
            (line.length() > 0))
        {
          payload.append(line);
        }
        if (payload.length() > 0) msg.put(name,payload.toString());
        
        // read trailing newline??
        //line = readLine(is);
      }
    }
    catch(IOException e)
    {
      System.err.println("EventStream : "+e.getMessage());
      throw e;
    }
    
    return msg;
  }

}
